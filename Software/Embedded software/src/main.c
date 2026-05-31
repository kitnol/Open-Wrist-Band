/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/logging/log.h>
#include <zephyr/logging/log_ctrl.h> /* add this for LOG_PANIC */

#include <zephyr/drivers/rtc.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/flash.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/pm/device.h>
#include <zephyr/sys/poweroff.h>

#include <zephyr/sys/util.h>
#include <zephyr/fs/fs.h>
#include <zephyr/fs/littlefs.h>
#include <zephyr/storage/flash_map.h>
#include <zephyr/sys/timeutil.h>

// USB includes
#include <zephyr/usb/usb_device.h>
#include <zephyr/sys/ring_buffer.h>
#include <zephyr/usb/usbd.h>
#include <sample_usbd.h>

// Shell includes
#include <zephyr/shell/shell.h>
#include <zephyr/shell/shell_uart.h>   /* shell_backend_uart_get_ptr() */

#include "as7058_typedefs.h"
#include "as7058_drv.h"
#include "flash_drv.h"
#include "usb_drv.h"

LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

/*
 *      DEFINITIONS
 */

// Flags
bool usb_connected = false;
bool usb_initalised = false;
bool shell_initialised = false;
bool flash_mounted = false;
bool rtc_initialised = false;
bool imu_initialised = false;
bool as7058_started = false;
bool measurments_en = true;

// Shell definitions
const struct device *i2c_dev;
const struct device *cdc_dev_usb;

// Flash
#define FLASH_NODE DT_NODE_BY_FIXED_PARTITION_LABEL(DT_N_NODELABEL_is25wp01g)
#define SPI_NODE DT_BUS(DT_PARENT(FLASH_NODE))

// AS7058 setup
#define I2C1_NODE DT_NODELABEL(as7058)
#define LIS2DS12_NODE DT_NODELABEL(lis2ds12)  

// DEVICES initialisation
const struct i2c_dt_spec dev_i2c = I2C_DT_SPEC_GET(I2C1_NODE);
const struct device *dev_rtc = DEVICE_DT_GET_ANY(microcrystal_rv3028);
const struct device *flash_dev = DEVICE_DT_GET_ANY(jedec_spi_nor);
//const struct device *dev_imu = DEVICE_DT_GET_ANY(st_lis2ds12);
const struct i2c_dt_spec imu_i2c = I2C_DT_SPEC_GET(LIS2DS12_NODE);

// Data blocks structure
#define rC_fifo_amount 423

struct readingContainer
{
    time_t time_sec;
    struct as7058_fifo_t data[rC_fifo_amount];
    uint8_t flags;
    uint16_t steps;
}; // 1280 bytes container - multiple of 256

// Time structure
struct rtc_time set_time = {
    .tm_year = 126, /* years since 1900 */
    .tm_mon = 5,    /* 0-indexed, January = 0 */
    .tm_mday = 1,
    .tm_hour = 12,
    .tm_min = 0,
    .tm_sec = 0,
};

/*
 *      Functions
 */

// Function to initialise step counter on IMU
int IMU_step_count_init(void)
{
	uint8_t reg;

	if(i2c_reg_read_byte_dt(&imu_i2c, 0x3F, &reg) < 0) {
		LOG_ERR("Failed to read step counter mode");
		return -1;
	}

	LOG_INF("Current 0x3F register: %x", reg);
	reg |= 0x01; // Set bit 0 to enable step counter mode

	if(i2c_reg_write_byte_dt(&imu_i2c, 0x3F, reg) < 0) {
		LOG_ERR("Failed to set step counter mode");
		return -1;
	}

	// Reset the step counter to 0
	if(i2c_reg_read_byte_dt(&imu_i2c, 0x3A, &reg) < 0) {
		LOG_ERR("Failed to read step counter reset register");
		return -1;
	}

	LOG_INF("Current 0x3A register: %x", reg);
	reg |= 0x80;  // Set bit 7 to reset step counter

	if(i2c_reg_write_byte_dt(&imu_i2c, 0x3A, reg) < 0) {
		LOG_ERR("Failed to reset step counter");
		return -1;
	}

	k_msleep(10); // Wait for the reset to take effect

	if(i2c_reg_read_byte_dt(&imu_i2c, 0x3D, &reg) < 0) {
		LOG_ERR("Failed to read FUNC_CK_GATE register after reset");
		return -1;
	}

	reg &= 0x04; // Clear bit 7 to enable step counter output
	LOG_INF("Current 0x3D register: %x", reg);
	
	reg = reg>>2; // Shift right to get the step counter output value
	if(reg != 0) {
		LOG_ERR("Step counter reset not finished, RST_PEDO bit: %x", reg);
	}

	return 0;
}

// Set up time
int rtc_driver_set_time(const struct device *dev_rtc, const struct rtc_time *timeptr)
{

    int ret = rtc_set_time(dev_rtc, timeptr);
    if (ret < 0)
    {
        LOG_ERR("Failed to set RTC time: %d", ret);
        return ret;
    }
    LOG_INF("Time set successfully");
    return 0;
}

// Get current time
int rtc_driver_get_time(const struct device *dev_rtc)
{
    struct rtc_time now;

    int ret = rtc_get_time(dev_rtc, &now);
    if (ret < 0)
    {
        LOG_ERR("Failed to get RTC time: %d", ret);
        return ret;
    }

    LOG_INF("Current time: %04d-%02d-%02d %02d:%02d:%02d",
            now.tm_year + 1900,
            now.tm_mon + 1, /* back to 1-indexed for display */
            now.tm_mday,
            now.tm_hour,
            now.tm_min,
            now.tm_sec);

    return 0;
}

/*
 *      Threads setup
 */

// Thread for saving measurments from as7058
static const struct gpio_dt_spec as7058_irq = GPIO_DT_SPEC_GET(DT_PATH(zephyr_user), ppg_irq_gpios);
static struct gpio_callback as7058_irq_cb;

// Struct for holding the workqueue
static struct k_work as7058_work;

#define WORK_STACK_SIZE 8192
#define WORK_PRIORITY 5

/* Actual processing — runs in work queue thread, not ISR context */
static void as7058_work_handler(struct k_work *work)
{
    ARG_UNUSED(work);

    int ret;
    uint16_t fifo_data_size = 0;
    static struct readingContainer rC;
    uint8_t step_count_h, step_count_l;

    rtc_get_time(dev_rtc, &set_time);
    rC.time_sec = timeutil_timegm(rtc_time_to_tm(&set_time)); //current time in seconds
	rC.steps = 0; //placeholder for step count, can be updated with actual logic
	rC.flags = 0; //placeholder for any flags, can be updated with actual logic

    ret = as7058_fifo_read_level(&dev_i2c, &fifo_data_size);
    if (ret < 0) {
        LOG_ERR("Failed to read FIFO level: %d", ret);
    } else {
        LOG_INF("FIFO level: %u samples", fifo_data_size);
    }

    for (int i = 0; i < fifo_data_size; i++) {
        ret = as7058_read_fifo_raw(&dev_i2c, (uint8_t *)&rC.data[i]);
        if (ret < 0) {
            LOG_ERR("Failed to read FIFO sample: %d", ret);
        }
    }

    if(i2c_reg_read_byte_dt(&imu_i2c, 0x3B, &step_count_l) < 0) {
		LOG_ERR("Failed to read step_count_l data");
	}
	if (i2c_reg_read_byte_dt(&imu_i2c, 0x3C, &step_count_h) < 0) {
		LOG_ERR("Failed to read step_count_h data");
	}

    rC.steps = (step_count_h << 8) | step_count_l;
	
    lfs_append_file("/lfs/readings.bin", &rC, sizeof(rC));

    /*
    
    TODO: Proper flash init
    
    */
}

static void gpio_isr(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
    k_work_submit(&as7058_work);
}

// Shell init thread
struct k_work          shell_toggle_work;
static void shell_toggle_handler(struct k_work *work)
{
    ARG_UNUSED(work);
 
    const struct shell *sh = shell_backend_uart_get_ptr();
 
    if (!shell_initialised) {
        /* ── Enable ── */
        shell_start(sh);
        shell_initialised = true;
        LOG_INF("Shell ENABLED - use 'flash read' command to transmit readings");
    } else {
        /* ── Disable ── */
        shell_stop(sh);
        shell_initialised = false;
        LOG_INF("Shell DISABLED");
    }
}

// Data transmition thread
struct k_work   data_transmit_work;
static void data_transmit_handler(struct k_work *work)
{
    ARG_UNUSED(work);

    printk("Thread started\n");

    if(shell_initialised)
    {
        k_work_submit(&shell_toggle_work);
        k_msleep(100);
    }

    printk("Shell toggled\n");

    if (!host_ready) {
        LOG_WRN("Host not ready, cannot transmit");
        return;
    }

    LOG_INF("Starting data transmission...");

    int ret;
    static struct readingContainer onePack;
	struct fs_file_t file;
	struct fs_dirent entry;
	
	ret = fs_stat("/lfs/readings.bin", &entry);
	if (ret < 0) {
		LOG_ERR("Failed to stat file: %d", ret);
		return;
	}

	fs_file_t_init(&file);
	ret = fs_open(&file, "/lfs/readings.bin", FS_O_READ);
	if (ret < 0) {
		LOG_ERR("Failed to open /lfs/readings.bin: %d", ret);
		return;
	}

	LOG_INF("File opened, size: %zu bytes, %zu records", entry.size, entry.size / sizeof(onePack));

    char payload[1280];
	uint32_t records_sent = 0;
	uint32_t total_records = entry.size / sizeof(onePack);

	for (uint32_t i = 0; i < total_records; i++) {
	    ret = lfs_read_file_partial(&file, &onePack, sizeof(onePack));
		if (ret < 0) {
			LOG_ERR("Failed to read record %u: %d", i, ret);
			break;
		}
		
        memcpy(payload, &onePack, sizeof(payload));
		ret = enqueue_packet((uint8_t *)payload, (uint16_t)sizeof(payload), &cdc_dev_usb);
		if (ret < 0) {
            LOG_ERR("enqueue_packet failed for record %u: %d", i, ret);
			break;
    	}
		
		records_sent++;
        k_sleep(K_MSEC(5));
	}

    LOG_INF("Transmission complete: %u/%u records sent", records_sent, total_records);
    fs_close(&file);

    if(!shell_initialised)
    {
        k_work_submit(&shell_toggle_work);
        k_msleep(100);
    }
}

// System init function
static int system_init(void)
{
    int ret;

    // Step counter initilisation
    const struct device *dev_imu = DEVICE_DT_GET_ANY(st_lis2ds12);
	if (dev_imu == NULL || !device_is_ready(dev_imu)) {
        LOG_ERR("IMU Device not ready");
    }

    if (!device_is_ready(imu_i2c.bus)) {
		LOG_ERR("I2C bus %s is not ready!\n\r",imu_i2c.bus->name);
	}

    if(IMU_step_count_init() < 0) {
		LOG_ERR("Failed to initialize IMU step counter");
	}

    // Attaching work handlers   
    k_work_init(&as7058_work, as7058_work_handler);
    k_work_init(&shell_toggle_work, shell_toggle_handler);

    // Cheking if GPIO for interrupts is ready
    ret = gpio_is_ready_dt(&as7058_irq); 
    if (ret < 0) {
        LOG_ERR("init: GPIO device not ready");
        return -ENODEV;
    }

    // Configuring GPIO
    ret = gpio_pin_configure_dt(&as7058_irq, GPIO_INPUT);
    if (ret < 0) return ret;

    // Attaching interrupt to GPIO
    ret = gpio_pin_interrupt_configure_dt(&as7058_irq, GPIO_INT_EDGE_TO_INACTIVE);
    if (ret < 0) return ret;

    // Initialising callback for interrupt
    gpio_init_callback(&as7058_irq_cb, gpio_isr, BIT(as7058_irq.pin));
    ret = gpio_add_callback(as7058_irq.port, &as7058_irq_cb);
    if (ret < 0) return ret;

    // Checking if i2c node for as7058 intialised
    if (!device_is_ready(dev_i2c.bus)) {
        LOG_ERR("I2C bus not ready");
        return -1;
    }

    // Checking if falsh intialised
    if (!device_is_ready(flash_dev)) {
        LOG_ERR("Flash device not ready");
        return -1;
    }
    // Initialising file system
    uint8_t jedec_id[3];
	if (flash_read_jedec_id(flash_dev, (uint8_t *)jedec_id) < 0) {
		LOG_ERR("Failed to read JEDEC ID\n");
		return -1;
	}
	LOG_INF("Flash JEDEC ID: 0x%02X %02X %02X", jedec_id[0], jedec_id[1], jedec_id[2]);
	uint8_t fs_initialized = 0;
	if (init_fs() < 0) {
		LOG_ERR("Filesystem init failed");
		log_flush();
		return -1;
	}
	else {
		fs_initialized = 1;
	}

    // RTC initialising 
	if (dev_rtc == NULL || !device_is_ready(dev_rtc)) {
		LOG_ERR("RTC Device not ready");
	}

    // Setting time 
    rtc_driver_set_time(dev_rtc, &set_time); 
	int rc = lfs_read_file("/lfs/time.bin", &set_time, sizeof(set_time));
	if	( rc == sizeof(set_time)) {
		rtc_driver_set_time(dev_rtc, &set_time);
	}
	else{
		lfs_write_file("/lfs/time.bin", &set_time, sizeof(set_time));
		LOG_ERR("Failed to read time from file, using default error: %d", rc);
	}

    // Initialising AS7058
    ret = as7058_init(&dev_i2c);
    if (ret < 0) return ret;

    k_msleep(10);

    // Starting measurments
    ret = as7058_start(&dev_i2c);
    if (ret < 0) return ret;

    return 0;
}

// USB interrupt
static void usb_status_cb_2(enum usb_dc_status_code status, const uint8_t *param)
{
    switch (status) {
    case USB_DC_CONNECTED:
        /* Stop AS7058 measurements when USB connects */
        k_work_cancel_delayable(&as7058_work);
        as7058_stop(&dev_i2c);
        as7058_started = false;
        
        /* Enable shell for user interaction */
        if (!shell_initialised) {
            k_work_submit(&shell_toggle_work);
        }
        host_ready = true;
        LOG_INF("USB connected - measurements stopped, shell enabled");
        break;
        
    case USB_DC_DISCONNECTED:
        /* Disable shell and stop any pending operations */
        if (shell_initialised) {
            k_work_submit(&shell_toggle_work);
        }

        /* Resume AS7058 measurements */
        if(measurments_en)
        {
            as7058_start(&dev_i2c);
            as7058_started = true;  
        }
        host_ready = false;
        tx_busy    = false;
        LOG_INF("USB disconnected - shell disabled, measurements resumed");
        break;
        
    case USB_DC_CONFIGURED:
        LOG_INF("USB configured (enumeration complete)");
        break;
        
    case USB_DC_SUSPEND:
        LOG_INF("USB suspended");
        break;
        
    case USB_DC_RESUME:
        LOG_INF("USB resumed");
        break;
        
    default:
        break;
    }
}

static struct k_work_delayable line_ctrl_work;
 
static void line_ctrl_handler(struct k_work *work)
{
    check_line_ctrl(cdc_dev_usb);
    k_work_reschedule(&line_ctrl_work, K_MSEC(100));
}

int main(void)
{
    int ret;
    ret = system_init();
    if(ret<0){
        LOG_ERR("System init failed");
        return ret;
    }
    
    ret = usb_enable(usb_status_cb_2);
    if (ret != 0) {
        LOG_ERR("Failed to enable USB: %d", ret);
        return ret;
    }

     if (!device_is_ready(cdc_dev_usb)) {
        LOG_ERR("CDC-ACM device not ready after USB enable");
        return -ENODEV;
    }

    /* ── Wire up the IRQ handler from usb_drv.c ──────────────────── */
    uart_irq_callback_user_data_set(cdc_dev_usb, cdc_irq_handler, NULL);
    uart_irq_rx_enable(cdc_dev_usb);
 
    /* ── Start line-control polling (DTR detection) ──────────────── */
    k_work_init_delayable(&line_ctrl_work, line_ctrl_handler);
    k_work_reschedule(&line_ctrl_work, K_MSEC(100));

    LOG_INF("System ready - measuring continuously until USB connects");
    k_sleep(K_FOREVER);
    return 0;
}
