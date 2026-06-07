#include <zephyr/sys/poweroff.h>
#include <zephyr/usb/usbd.h>
#include <zephyr/drivers/uart.h>
#include "usb_drv.h"
#include <zephyr/usb/usb_device.h>
#include <zephyr/sys/ring_buffer.h>
#include <zephyr/usb/usbd.h>
#include <sample_usbd.h>

LOG_MODULE_REGISTER(usb_drv, LOG_LEVEL_DBG);

/* ── Global State ──────────────────────────────────────────── */
volatile bool host_ready = false;   /* DTR set by host    */
volatile bool tx_busy    = false;   /* TX in progress     */

/* ── Ring Buffers ──────────────────────────────────────────── */
RING_BUF_DECLARE(tx_ring_buf, TX_RING_BUF_SIZE);
RING_BUF_DECLARE(rx_ring_buf, RX_RING_BUF_SIZE);


/* ── CRC-16/CCITT ──────────────────────────────────────────── */
uint16_t crc16_ccitt_drv(const uint8_t *data, size_t len)
{
    uint16_t crc = 0xFFFF;
    for (size_t i = 0; i < len; i++) {
        crc ^= (uint16_t)data[i] << 8;
        for (int b = 0; b < 8; b++) {
            crc = (crc & 0x8000) ? (crc << 1) ^ 0x1021 : (crc << 1);
        }
    }
    return crc;
}

/* ── Packet Builder ────────────────────────────────────────── */
/*
 * Frame layout:
 *   [0]      SYNC  (0xAA)
 *   [1]      LEN_HI
 *   [2]      LEN_LO
 *   [3..N]   payload
 *   [N+1]    CRC_HI
 *   [N+2]    CRC_LO
 */
int build_packet(const uint8_t *payload, uint16_t len,
                        uint8_t *out, size_t out_size)
{
    size_t frame_len = 3 + len + 2;
    if (frame_len > out_size || len > MAX_PAYLOAD) {
        return -ENOMEM;
    }

    out[0] = PACKET_SYNC;
    out[1] = (len >> 8) & 0xFF;
    out[2] =  len       & 0xFF;
    memcpy(&out[3], payload, len);

    uint16_t crc = crc16_ccitt_drv(payload, len);
    out[3 + len]     = (crc >> 8) & 0xFF;
    out[3 + len + 1] =  crc       & 0xFF;

    return (int)frame_len;
}

/* ── UART/CDC Interrupt Handler ────────────────────────────── */
void cdc_irq_handler(const struct device *dev, void *user_data)
{
    ARG_UNUSED(user_data);

    uart_irq_update(dev);

    /* ── RX: host → device ── */
    if (uart_irq_rx_ready(dev)) {
        uint8_t buf[64];
        int rx_len = uart_fifo_read(dev, buf, sizeof(buf));
        if (rx_len > 0) {
            uint32_t written = ring_buf_put(&rx_ring_buf, buf, rx_len);
            if (written < (uint32_t)rx_len) {
                LOG_WRN("RX ring buf overflow! Dropped %d bytes",
                        rx_len - (int)written);
            }
        }
    }

    /* ── TX: device → host ── */
    if (uart_irq_tx_ready(dev)) {
        uint8_t chunk[64];
        uint32_t claim_len = ring_buf_get(&tx_ring_buf, chunk, sizeof(chunk));

        if (claim_len == 0) {
            /* Nothing left — disable TX interrupt */
            uart_irq_tx_disable(dev);
            tx_busy = false;
        } else {
            int sent = uart_fifo_fill(dev, chunk, claim_len);
            if (sent < (int)claim_len) {
                /* Re-push unsent bytes (best-effort; ring buf may be full) */
                ring_buf_put(&tx_ring_buf,
                             chunk + sent, claim_len - sent);
            }
        }
    }
}

void tx_kick(const struct device *cdc_dev)
{
    if (!tx_busy && !ring_buf_is_empty(&tx_ring_buf)) {
        tx_busy = true;
        uart_irq_tx_enable(cdc_dev);
    }
}

/* ── Line Control (DTR/RTS) ────────────────────────────────── */
void check_line_ctrl(const struct device *cdc_dev)
{
    uint32_t dtr = 0;
    uart_line_ctrl_get(cdc_dev, UART_LINE_CTRL_DTR, &dtr);
    if ((bool)dtr != host_ready) {
        host_ready = (bool)dtr;
        LOG_INF("Host %s", host_ready ? "connected" : "disconnected");

        if (host_ready) {
            /* Signal RTS to host */
            uart_line_ctrl_set(cdc_dev, UART_LINE_CTRL_RTS, 1);
        }
    }
}

/* ── Enqueue a framed packet into TX ring buffer ───────────── */
int enqueue_packet(const uint8_t *payload, uint16_t len, const struct device *cdc_dev)
{
    uint8_t frame[MAX_PAYLOAD + 5];
    int frame_len = build_packet(payload, len, frame, sizeof(frame));
    if (frame_len < 0) {
        return frame_len;
    }

    /* Block until there's space (avoids data loss under back-pressure) */
    uint32_t remaining = frame_len;
    uint8_t *ptr = frame;
    while (remaining > 0) {
        uint32_t put = ring_buf_put(&tx_ring_buf, ptr, remaining);
        remaining -= put;
        ptr       += put;
        if (remaining > 0) {
            /* Ring buffer full — yield and retry */
            k_yield();
        }
    }

    tx_kick(cdc_dev);
    return frame_len;
}

/* ── Process Received Packets (main loop) ──────────────────── */
uint8_t rx_parse_buf[MAX_PAYLOAD + 5];
size_t  rx_parse_pos = 0;

void process_rx(const struct device *cdc_dev)
{
    uint8_t byte;
    while (ring_buf_get(&rx_ring_buf, &byte, 1) == 1) {

        /* Simple state machine: hunt for SYNC, then accumulate frame */
        if (rx_parse_pos == 0 && byte != PACKET_SYNC) {
            continue; /* skip garbage before sync */
        }

        rx_parse_buf[rx_parse_pos++] = byte;

        /* Need at least 3 bytes to know length */
        if (rx_parse_pos < 3) continue;

        uint16_t payload_len = ((uint16_t)rx_parse_buf[1] << 8)
                             |  rx_parse_buf[2];

        if (payload_len > MAX_PAYLOAD) {
            /* Bad frame — reset */
            rx_parse_pos = 0;
            continue;
        }

        size_t expected = 3 + payload_len + 2;
        if (rx_parse_pos < expected) continue;

        /* Full frame received — verify CRC */
        uint16_t rx_crc = ((uint16_t)rx_parse_buf[3 + payload_len] << 8)
                        |  rx_parse_buf[3 + payload_len + 1];
        uint16_t calc_crc = crc16_ccitt_drv(&rx_parse_buf[3], payload_len);

        if (rx_crc == calc_crc) {
            /* Valid packet — handle it */
            LOG_INF("RX packet: %u bytes", payload_len);

            /* Echo back as ACK (optional) */
            enqueue_packet(&rx_parse_buf[3], payload_len, cdc_dev);
        } else {
            LOG_WRN("CRC mismatch! Got 0x%04X expected 0x%04X",
                    rx_crc, calc_crc);
            /* Send NACK (0xFF byte payload) */
            uint8_t nack = 0xFF;
            enqueue_packet(&nack, 1, cdc_dev);
        }

        rx_parse_pos = 0;
    }
}

/* ── USB Event Callback ────────────────────────────────────── */
void usb_status_cb(enum usb_dc_status_code status, const uint8_t *param)
{
    switch (status) {
    case USB_DC_CONFIGURED:
        LOG_INF("USB configured");
        break;
    case USB_DC_DISCONNECTED:
        host_ready = false;
        tx_busy    = false;
        LOG_INF("USB disconnected");
        break;
    default:
        break;
    }
}

int recv_packet(uint8_t *out, size_t out_max, k_timeout_t timeout)
{
    int64_t deadline = k_uptime_get() + k_ticks_to_ms_floor64(timeout.ticks);
    uint8_t frame[MAX_PAYLOAD + 5];
    size_t  pos = 0;
 
    while (k_uptime_get() < deadline) {
        uint8_t b;
        if (ring_buf_get(&rx_ring_buf, &b, 1) != 1) {
            k_sleep(K_MSEC(2));
            continue;
        }
 
        /* Hunt for SYNC */
        if (pos == 0 && b != PACKET_SYNC) continue;
 
        frame[pos++] = b;
 
        /* Need 3 bytes to know length */
        if (pos < 3) continue;
 
        unsigned int plen = ((uint16_t)frame[1] << 8) | frame[2];
        if (plen > MAX_PAYLOAD) { pos = 0; continue; }  /* bad frame */
 
        size_t total = 3 + plen + 2;
        if (pos < total) continue;
 
        /* Full frame – verify CRC */
        uint16_t rx_crc   = ((uint16_t)frame[3 + plen] << 8)
                          |  frame[3 + plen + 1];
        uint16_t calc_crc = crc16_ccitt_drv(&frame[3], plen);
 
        if (rx_crc != calc_crc) {
            LOG_WRN("xfer: recv_packet CRC mismatch");
            pos = 0;
            return -EIO;
        }
 
        size_t copy = (plen < out_max) ? plen : out_max;
        memcpy(out, &frame[3], copy);
        return (int)copy;
    }
    return -ETIMEDOUT;
}