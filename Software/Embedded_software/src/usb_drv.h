#include <zephyr/usb/usbd.h>   /* struct usbd_context, struct usbd_msg */
#include <zephyr/kernel.h>     /* struct k_work */
#include <zephyr/usb/usb_device.h>
#include <zephyr/sys/ring_buffer.h>
#include <zephyr/usb/usb_device.h>
#include <zephyr/sys/ring_buffer.h>

/* ── Configuration ─────────────────────────────────────────── */
#define TX_RING_BUF_SIZE   4096   /* TX ring buffer (bytes)     */
#define RX_RING_BUF_SIZE   4096   /* RX ring buffer (bytes)     */
#define PACKET_SYNC        0xAA   /* Framing sync byte          */
#define MAX_PAYLOAD        1280   /* Max payload per packet     */
#define SEND_INTERVAL_MS   10     /* How often to send data     */


extern volatile bool host_ready;   /* DTR set by host    */
extern volatile bool tx_busy;      /* TX in progress     */

uint16_t crc16_ccitt_drv(const uint8_t *data, size_t len);

int build_packet(const uint8_t *payload, uint16_t len, uint8_t *out, size_t out_size);

void tx_kick(const struct device *cdc_dev);

void cdc_irq_handler(const struct device *dev, void *user_data);

void check_line_ctrl(const struct device *cdc_dev);

int enqueue_packet(const uint8_t *payload, uint16_t len, const struct device *cdc_dev);

void process_rx(const struct device *cdc_dev);

void usb_status_cb(enum usb_dc_status_code status, const uint8_t *param);

int recv_packet(uint8_t *out, size_t out_max, k_timeout_t timeout);

