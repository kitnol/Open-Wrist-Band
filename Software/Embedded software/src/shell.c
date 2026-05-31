#include <zephyr/kernel.h>
#include <zephyr/shell/shell.h>
#include <zephyr/sys/ring_buffer.h>
#include <zephyr/logging/log.h>
#include <zephyr/usb/usb_device.h>
#include <zephyr/drivers/uart.h>
#include <string.h>
#include <stdlib.h>

#include <zephyr/fs/fs.h>
#include <zephyr/fs/littlefs.h>
#include "flash_drv.h"
#include "as7058_typedefs.h"

LOG_MODULE_REGISTER(data_tx, LOG_LEVEL_INF);

#define PACKET_SIZE        1280
#define TX_THREAD_PRIORITY 2
#define TX_STACK_SIZE      4096
#define RING_BUF_SIZE      (PACKET_SIZE * 16)

RING_BUF_DECLARE(tx_ring_buf_2, RING_BUF_SIZE);

static K_THREAD_STACK_DEFINE(tx_stack, TX_STACK_SIZE);
static struct k_thread tx_thread_data;
static K_SEM_DEFINE(tx_sem, 0, 1);

/* CDC ACM UART device — second CDC instance so shell keeps its own port */
static const struct device *cdc_dev;

static volatile uint32_t packets_total;
static volatile uint32_t packets_sent;
static volatile bool      tx_running;

struct readingContainer
{
    time_t time_sec;
    struct as7058_fifo_t data[423];
    uint8_t flags;
    uint16_t steps;
}; // 1280 bytes container - multiple of 256

/* ------------------------------------------------------------------ */
/*  Blocking CDC ACM write — retries until all bytes are sent         */
/* ------------------------------------------------------------------ */
static int cdc_write_all(const struct device *dev,
                         const uint8_t *buf, size_t len)
{
    size_t written = 0;

    while (written < len) {
        /* uart_fifo_fill returns number of bytes actually written */
        int ret = uart_fifo_fill(dev, buf + written, len - written);
        if (ret < 0) {
            return ret;
        }
        written += ret;
        if (written < len) {
            k_yield(); /* Let USB stack flush before retrying */
        }
    }
    return 0;
}

/* ------------------------------------------------------------------ */
/*  TX thread: drains ring buffer over CDC ACM                        */
/* ------------------------------------------------------------------ */
static void tx_thread_entry(void *p1, void *p2, void *p3)
{
    ARG_UNUSED(p1); ARG_UNUSED(p2); ARG_UNUSED(p3);



    uint8_t buf[PACKET_SIZE];

    while (tx_running || !ring_buf_is_empty(&tx_ring_buf_2)) {
        k_sem_take(&tx_sem, K_MSEC(10));

        while (!ring_buf_is_empty(&tx_ring_buf_2)) {
            uint32_t got = ring_buf_get(&tx_ring_buf_2, buf, PACKET_SIZE);
            if (got < PACKET_SIZE) {
                continue;
            }

            int ret = cdc_write_all(cdc_dev, buf, PACKET_SIZE);
            if (ret < 0) {
                LOG_ERR("CDC write failed: %d", ret);
                continue;
            }

            packets_sent++;

            if (packets_sent % 10 == 0 || packets_sent == packets_total) {
                LOG_INF("Sent %u / %u packets", packets_sent, packets_total);
            }
        }
    }

    LOG_INF("TX done: %u packets sent", packets_sent);
}

/* ------------------------------------------------------------------ */
/*  Shell command                                                      */
/* ------------------------------------------------------------------ */
static int cmd_data_tx(const struct shell *sh, size_t argc, char **argv)
{

    if (argc < 2) { shell_error(sh, "Usage: flash info <path>"); return -EINVAL; }

    struct fs_dirent e;
    int rc = fs_stat(argv[1], &e);
    
    if (!device_is_ready(cdc_dev)) {
        shell_error(sh, "CDC ACM device not ready");
        return -ENODEV;
    }

    if (tx_running) {
        shell_error(sh, "TX already running");
        return -EBUSY;
    }

    ring_buf_reset(&tx_ring_buf_2);
    packets_total = e.size/1280;
    packets_sent  = 0;
    tx_running    = true;

    shell_print(sh, "TX start: %u packets x %d bytes = %u bytes",
                packets_total, PACKET_SIZE, packets_total * PACKET_SIZE);

    k_tid_t tid = k_thread_create(
        &tx_thread_data,
        tx_stack,
        K_THREAD_STACK_SIZEOF(tx_stack),
        tx_thread_entry,
        NULL, NULL, NULL,
        TX_THREAD_PRIORITY,
        0,
        K_NO_WAIT
    );

    if (!tid) {
        shell_error(sh, "Failed to create TX thread");
        tx_running = false;
        return -ENOMEM;
    }

    /* Producer: fill ring buffer */
    uint8_t packet[PACKET_SIZE];


    static struct readingContainer onePack;
	struct fs_file_t file;
	struct fs_dirent entry;
	fs_stat(argv[1], &entry);

    fs_file_t_init(&file);
	rc = fs_open(&file,argv[1], FS_O_READ);
	if (rc < 0) {
		LOG_ERR("Failed to open %s: %d", argv[1], rc);
	}

    uint32_t produced = 0;
    while (produced < entry.size/sizeof(onePack)) {

        rc = lfs_read_file_partial(&file, &onePack, sizeof(onePack));
		if (rc < 0) {
			LOG_ERR("Failed to read /lfs/readings.bin: %d", rc);
		}

        memcpy(packet, &onePack, sizeof(packet));

        uint32_t written = ring_buf_put(&tx_ring_buf_2, packet, PACKET_SIZE);
        if (written == PACKET_SIZE) {
            produced++;
            k_sem_give(&tx_sem);
        } else {
            k_yield();
        }
    }

    tx_running = false;
    k_sem_give(&tx_sem);

    k_thread_join(&tx_thread_data, K_FOREVER);

    shell_print(sh, "TX complete: %u packets sent", packets_sent);
    return 0;
}

/* ------------------------------------------------------------------ */
/*  Init: get CDC device handle                                        */
/* ------------------------------------------------------------------ */
static int data_tx_init(void)
{
    /* CDC ACM instance 1 — instance 0 is used by the shell */
    cdc_dev = DEVICE_DT_GET(DT_NODELABEL(cdc_acm_uart1));
    if (!device_is_ready(cdc_dev)) {
        LOG_ERR("CDC ACM UART1 not ready");
        return -ENODEV;
    }

    LOG_INF("data_tx ready on CDC ACM UART1");
    return 0;
}

SYS_INIT(data_tx_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);

 
/* ------------------------------------------------------------------ */
/*  Helpers                                                           */
/* ------------------------------------------------------------------ */
#define CHUNK_SIZE      4096u           /* bytes per chunk              */


static int file_size(const char *path, size_t *out)
{
    struct fs_dirent e;
    int rc = fs_stat(path, &e);
    if (rc == 0) *out = e.size;
    return rc;
}
 
/* ------------------------------------------------------------------ */
/*  cmd: flash ls                                                      */
/* ------------------------------------------------------------------ */
static int cmd_flash_ls(const struct shell *sh, size_t argc, char **argv)
{
    const char *path = (argc >= 2) ? argv[1] : "/lfs";
    struct fs_dir_t dir;
    struct fs_dirent entry;
    fs_dir_t_init(&dir);
 
    int rc = fs_opendir(&dir, path);
    if (rc < 0) { shell_error(sh, "opendir '%s': %d", path, rc); return rc; }
 
    shell_print(sh, "Contents of %s:", path);
    int n = 0;
    for (;;) {
        rc = fs_readdir(&dir, &entry);
        if (rc < 0 || entry.name[0] == '\0') break;
        if (entry.type == FS_DIR_ENTRY_DIR)
            shell_print(sh, "  [DIR]  %s", entry.name);
        else
            shell_print(sh, "  [FILE] %-32s  %6zu bytes", entry.name, entry.size);
        n++;
    }
    fs_closedir(&dir);
    shell_print(sh, "  (%d entr%s)", n, n == 1 ? "y" : "ies");
    return 0;
}
 
/* ------------------------------------------------------------------ */
/*  cmd: flash info                                                    */
/* ------------------------------------------------------------------ */
static int cmd_flash_info(const struct shell *sh, size_t argc, char **argv)
{
    if (argc < 2) { shell_error(sh, "Usage: flash info <path>"); return -EINVAL; }
    struct fs_dirent e;
    int rc = fs_stat(argv[1], &e);
    if (rc < 0) { shell_error(sh, "stat '%s': %d", argv[1], rc); return rc; }
    shell_print(sh, "Path : %s", argv[1]);
    shell_print(sh, "Type : %s", e.type == FS_DIR_ENTRY_DIR ? "directory" : "file");
    shell_print(sh, "Size : %zu bytes", e.size);
    return 0;
}
 
/* ------------------------------------------------------------------ */
/*  cmd: flash rm                                                      */
/* ------------------------------------------------------------------ */
static int cmd_flash_rm(const struct shell *sh, size_t argc, char **argv)
{
    if (argc < 2) { shell_error(sh, "Usage: flash rm <path>"); return -EINVAL; }
    int rc = fs_unlink(argv[1]);
    if (rc < 0) { shell_error(sh, "unlink '%s': %d", argv[1], rc); return rc; }
    shell_print(sh, "Deleted '%s'", argv[1]);
    return 0;
}
 
/* ------------------------------------------------------------------ */
/*  Registration                                                       */
/* ------------------------------------------------------------------ */
SHELL_CMD_ARG_REGISTER(data_tx, NULL,
    "Send N x 1280 byte packets over USB CDC ACM. Usage: data_tx <num_packets>",
    cmd_data_tx, 2, 0);

SHELL_STATIC_SUBCMD_SET_CREATE(flash_cmds,
    SHELL_CMD_ARG(ls,   NULL,
        "List LittleFS directory\nUsage: flash ls [path]",
        cmd_flash_ls,   1, 1),
    SHELL_CMD_ARG(info, NULL,
        "Show file info\nUsage: flash info <path>",
        cmd_flash_info, 2, 0),
    SHELL_CMD_ARG(dump, NULL,
        "Show file info\nUsage: flash info <path>",
        cmd_data_tx, 2, 0),
    SHELL_CMD_ARG(rm,   NULL,
        "Delete a file\nUsage: flash rm <path>",
        cmd_flash_rm,   2, 0),
    SHELL_SUBCMD_SET_END
);
 
SHELL_CMD_REGISTER(flash, &flash_cmds,
    "LittleFS file commands (ls / info / dump / hex / rm)", NULL);


#include <zephyr/drivers/rtc.h>

extern const struct device *dev_rtc;

/* ------------------------------------------------------------------ */
/*  cmd: rtc set time                                                 */
/* ------------------------------------------------------------------ */
static int cmd_rtc_set(const struct shell *sh, size_t argc, char **argv)
{   
    if (argc < 7) {
        shell_error(sh, "Usage: rtc set <year> <month> <day> <hour> <minute>");
        return -EINVAL;
    }
    struct rtc_time set_time = {
        .tm_year = atoi(argv[1]) - 1900, /* years since 1900 */
        .tm_mon = atoi(argv[2]) - 1,     /* 0-indexed, January = 0 */
        .tm_mday = atoi(argv[3]),
        .tm_hour = atoi(argv[4]),
        .tm_min = atoi(argv[5]),
        .tm_sec = atoi(argv[6]),
    };

    shell_print(sh, "Setting time to: %04d-%02d-%02d %02d:%02d:%02d",
            set_time.tm_year + 1900,
            set_time.tm_mon + 1, /* back to 1-indexed for display */
            set_time.tm_mday,
            set_time.tm_hour,
            set_time.tm_min,
            set_time.tm_sec);

    int ret = rtc_set_time(dev_rtc, &set_time);
    if (ret < 0)
    {
        LOG_ERR("Failed to get RTC time: %d", ret);
        return ret;
    }

    return 0;
}

/* ------------------------------------------------------------------ */
/*  cmd: rtc get time                                                 */
/* ------------------------------------------------------------------ */
static int cmd_rtc_get(const struct shell *sh, size_t argc, char **argv)
{   
    struct rtc_time now;

    int ret = rtc_get_time(dev_rtc, &now);
    if (ret < 0)
    {
        LOG_ERR("Failed to get RTC time: %d", ret);
        return ret;
    }

    shell_print(sh, "Current time: %04d-%02d-%02d %02d:%02d:%02d",
            now.tm_year + 1900,
            now.tm_mon + 1, /* back to 1-indexed for display */
            now.tm_mday,
            now.tm_hour,
            now.tm_min,
            now.tm_sec);

    return 0;
}

/* ------------------------------------------------------------------ */
/*  Registration                                                       */
/* ------------------------------------------------------------------ */

SHELL_STATIC_SUBCMD_SET_CREATE(rtc_cmds,
    SHELL_CMD_ARG(get,   NULL,
        "Print current time on device\n"
        "Usage: rtc get",
        cmd_rtc_get,   1, 1),

    SHELL_CMD_ARG(set, NULL,
        "Set time on device\n"
        "Usage: rtc set <year> <month> <day> <hour> <minute>",
        cmd_rtc_set, 7, 0),

    SHELL_SUBCMD_SET_END
);

SHELL_CMD_REGISTER(rtc, &rtc_cmds,
    "RTC commands (get / set)",
    NULL);

#include "as7058_drv.h"

extern const struct i2c_dt_spec dev_i2c;
extern bool measurments_en;

static int cmd_as7058_start(const struct shell *sh, size_t argc, char **argv)
{   
    int ret = as7058_start(&dev_i2c);

    if(ret<0)
    {
        shell_print(sh, "Failed to start measurments\n");
    }
    else{
        shell_print(sh, "Measurments started\n");
    }

    return 0;
}

static int cmd_as7058_enable(const struct shell *sh, size_t argc, char **argv)
{   
    measurments_en = true;
    shell_print(sh, "Measurments enabeled\n");

    return 0;
}

static int cmd_as7058_disable(const struct shell *sh, size_t argc, char **argv)
{   
    measurments_en = false;
    shell_print(sh, "Measurments enabeled\n");

    return 0;
}

static int cmd_as7058_stop(const struct shell *sh, size_t argc, char **argv)
{   
    int ret = as7058_stop(&dev_i2c);

    if(ret<0)
    {
        shell_print(sh, "Failed to start measurments\n");
    }
    else{
        shell_print(sh, "Measurments stopeped\n");
    }

    return 0;
}

static int cmd_as7058_read(const struct shell *sh, size_t argc, char **argv)
{   
    uint16_t fifo_data_size;

    struct as7058_fifo_t data[511];

    int ret = as7058_fifo_read_level(&dev_i2c, &fifo_data_size);
    if (ret < 0) {
        LOG_ERR("Failed to read FIFO level: %d", ret);
    } else {
        LOG_INF("FIFO level: %u samples", fifo_data_size);
    }

    for (int i = 0; i < fifo_data_size; i++) {
        ret = as7058_read_fifo_raw(&dev_i2c, (uint8_t *)&data[i]);
        if (ret < 0) {
            LOG_ERR("Failed to read FIFO sample: %d", ret);
        }
    }

    shell_print(sh, "Printing read FIFO:\n");
    for(int i = 0; i<fifo_data_size; i++)
    {
        shell_print(sh, "%d", ((data[i].fifoh<<16)|(data[i].fifom<<8)|data[i].fifol));
    }

    return 0;
}


/* ------------------------------------------------------------------ */
/*  Registration                                                       */
/* ------------------------------------------------------------------ */

SHELL_STATIC_SUBCMD_SET_CREATE(as7058_cmds,
    SHELL_CMD_ARG(start,   NULL,
        "Start AS7058 measurments\n"
        "Usage: as7058 start",
        cmd_as7058_start,   1, 1),

    SHELL_CMD_ARG(stop, NULL,
        "Stop AS7058 measurments\n"
        "Usage: as7058 stop",
        cmd_as7058_stop, 1,1),

    SHELL_CMD_ARG(read, NULL,
        "Read AS7058 FIFO\n"
        "Usage: as7058 read",
        cmd_as7058_read, 1, 1),

    SHELL_CMD_ARG(enable, NULL,
        "Enabling AS7058 measurments\n"
        "Usage: as7058 enable",
        cmd_as7058_enable, 1, 1),

    SHELL_CMD_ARG(disable, NULL,
        "Disabling AS7058 measurments\n"
        "Usage: as7058 disable",
        cmd_as7058_disable, 1, 1),
    SHELL_SUBCMD_SET_END
);

SHELL_CMD_REGISTER(as7058, &as7058_cmds,
    "RTC commands (get / set)",
    NULL);