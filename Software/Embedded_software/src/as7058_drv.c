#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/i2c.h>

#include "as7058_typedefs.h"
#include "as7058_drv.h"

LOG_MODULE_REGISTER(as7058, LOG_LEVEL_DBG);

/* User defined measurement variables */

// FIFO definitions
#define FIFO_MAX_LEVEL 511
#define FIFO_SAMPLE_COUNT 423 // Maaximum: 511

// PPG frequency and sample count
#define PPG_FREQ 10 // Hz

// ECG frequency
#define ECG_FREQ 10 // Hz

// PPG Modulator eneabling
#define PPG_MOD_1 1 // Enable Modulator1 0 = off, 1 = on
#define PPG_MOD_2 0 // Enable Modulator2 0 = off, 1 = on

// PPG LED current settings
#define PPG_LED_INIT (uint8_t)8 		// us; Max:255 - time before measurement for LED to reach stable brightness after being turned on
#define LED_1_max_current 1 // 0 = 25mA, 1 = 150mA, 2 = 225mA, 3 = 300mA (RED center)
#define LED_2_max_current 1 // 0 = 25mA, 1 = 150mA, 2 = 225mA, 3 = 300mA (GREEN center)
#define LED_3_max_current 0 // 0 = 25mA, 1 = 150mA, 2 = 225mA, 3 = 300mA
#define LED_4_max_current 0 // 0 = 25mA, 1 = 150mA, 2 = 225mA, 3 = 300mA
#define LED_5_max_current 1 // 0 = 25mA, 1 = 150mA, 2 = 225mA, 3 = 300mA
#define LED_6_max_current 1 // 0 = 25mA, 1 = 150mA, 2 = 225mA, 3 = 300mA 
#define LED_7_max_current 0 // 0 = 25mA, 1 = 150mA, 2 = 225mA, 3 = 300mA (IR center)
#define LED_8_max_current 0 // 0 = 25mA, 1 = 150mA, 2 = 225mA, 3 = 300mA

#define LED_1_current 20 	// mA
#define LED_2_current 20 	// mA
#define LED_3_current 0 	// mA
#define LED_4_current 0 	// mA
#define LED_5_current 20     // mA
#define LED_6_current 0 	// mA
#define LED_7_current 0  	// mA 
#define LED_8_current 0 	// mA

// PPG slecting which LEDs are on for which measurements
/*
 * Explanation on how to set up register LED_SUBx
 *
 * Each register devided into two 4-bit fields: subx_drv1_sel (bits 0-3) and subx_drv2_sel (bits 4-7)
 * Only one LED can be selected from each driver for each sub sample, 
 * so only one bit can be set in each 4-bit field. 
 * 
 * subx_drv1_sel selects which LED from 1-4 where bit 0 corresponds to LED1 and bit 3 corresponds to LED4.
 * subx_drv2_sel selects which LED from 5-8 where bit 0 corresponds to LED5 and bit 3 corresponds to LED8.
 * 0: Disabled
 * 1: Enabled
*/
//On wwdevice PCB LED1 - IR_1, LED2 - green_1, LED3 - IR_2, LED5 - RED_2, LED6 - green_2, LED7 - RED_1
#define LED_SUB1 0b00000010 // LED_SUB1: Selecting: LED2 as the source for the first sub sample (sub1_drv1_sel=2)
#define LED_SUB2 0b00000001 // LED_SUB2: LEFT DEFAULT
#define LED_SUB3 0b00010000 // LED_SUB3: LEFT DEFAULT
#define LED_SUB4 0b00000000 // LED_SUB4: LEFT DEFAULT
#define LED_SUB5 0b00000000 // LED_SUB5: LEFT DEFAULT
#define LED_SUB6 0b00000000 // LED_SUB6: LEFT DEFAULT
#define LED_SUB7 0b00000000 // LED_SUB7: LEFT DEFAULT
#define LED_SUB8 0b00000000 // LED_SUB8: LEFT DEFAULT

// PPG slecting which Photo diodes (PD) are on for which measurements
/*
 * Explanation on how to set up register PPGx_PDSELx
 *
 * Each register devided consists of 8 bits, where each bit corresponds to one of the 8 photodiodes (PD) in the system.
 * Bit 0 corresponds to PD1 and bit 7 corresponds to PD8.
 * PPGx means to which PPG driver the PD selection applies, where x is 1 or 2.
 * 
 * 0: Disabled
 * 1: Enabled
*/

#define PPG1_PDSEL1 0b00000010 // PPG1_PDSEL1: Selecting: photodiode 2 for sub sample 1 (ppg1_pdsel_sub1=2)
#define PPG1_PDSEL2 0b00000001 // PPG1_PDSEL2: LEFT DEFAULT
#define PPG1_PDSEL3 0b00000001 // PPG1_PDSEL3: LEFT DEFAULT
#define PPG1_PDSEL4 0b00000000 // PPG1_PDSEL4: LEFT DEFAULT
#define PPG1_PDSEL5 0b00000000 // PPG1_PDSEL5: LEFT DEFAULT
#define PPG1_PDSEL6 0b00000000 // PPG1_PDSEL6: LEFT DEFAULT
#define PPG1_PDSEL7 0b00000000 // PPG1_PDSEL7: LEFT DEFAULT
#define PPG1_PDSEL8 0b00000000 // PPG1_PDSEL8: LEFT DEFAULT
#define PPG2_PDSEL1 0b00000000 // PPG2_PDSEL1: LEFT DEFAULT
#define PPG2_PDSEL2 0b00000000 // PPG2_PDSEL2: LEFT DEFAULT
#define PPG2_PDSEL3 0b00000000 // PPG2_PDSEL3: LEFT DEFAULT
#define PPG2_PDSEL4 0b00000000 // PPG2_PDSEL4: LEFT DEFAULT
#define PPG2_PDSEL5 0b00000000 // PPG2_PDSEL5: LEFT DEFAULT
#define PPG2_PDSEL6 0b00000000 // PPG2_PDSEL6: LEFT DEFAULT
#define PPG2_PDSEL7 0b00000000 // PPG2_PDSEL7: LEFT DEFAULT
#define PPG2_PDSEL8 0b00000000 // PPG2_PDSEL8: LEFT DEFAULT


/* Calculations for registers setup */

// Calculations for LED 1
#if defined(LED_1_current) && defined(LED_1_max_current)
	#if LED_1_max_current == 0
		#if LED_1_current >= 25
			#define LED1_ICTRL 255
		#elif LED_1_current <= 0
			#define LED1_ICTRL 0
		#else
			#define LED1_ICTRL (uint8_t)((LED_1_current * 255) / 25)
		#endif
	#elif LED_1_max_current == 1
		#if LED_1_current > 150
			#define LED1_ICTRL 255
		#elif LED_1_current <= 0
			#define LED1_ICTRL 0
		#else
			#define LED1_ICTRL (uint8_t)((LED_1_current * 255) / 150)
		#endif
	#elif LED_1_max_current == 2
		#if LED_1_current >= 225
			#define LED1_ICTRL 255
		#elif LED_1_current <= 0
			#define LED1_ICTRL 0
		#else
			#define LED1_ICTRL (uint8_t)((LED_1_current * 255) / 225)
		#endif
	#elif LED_1_max_current == 3
		#if LED_1_current >= 300
			#define LED1_ICTRL 255
		#elif LED_1_current <= 0	
			#define LED1_ICTRL 0
		#else
			#define LED1_ICTRL (uint8_t)((LED_1_current * 255) / 300)
		#endif
	#else
		#error "Invalid LED_1_max_current value, should be 0, 1, 2, or 3"
		#define LED1_ICTRL 0
	#endif
#endif

// Calculations for LED 2
#if defined(LED_2_current) && defined(LED_2_max_current)
	#if LED_2_max_current == 0
		#if LED_2_current > 25
			#define LED2_ICTRL 255
		#elif LED_2_current <= 0
			#define LED2_ICTRL 0
		#else
			#define LED2_ICTRL (uint8_t)((LED_2_current * 255) / 25)
		#endif
	#elif LED_2_max_current == 1
		#if LED_2_current > 150
			#define LED2_ICTRL 255
		#elif LED_2_current <= 0
			#define LED2_ICTRL 0
		#else
			#define LED2_ICTRL (uint8_t)((LED_2_current * 255) / 150)
		#endif
	#elif LED_2_max_current == 2
		#if LED_2_current > 225
			#define LED2_ICTRL 255
		#elif LED_2_current <= 0
			#define LED2_ICTRL 0
		#else
			#define LED2_ICTRL (uint8_t)((LED_2_current * 255) / 225)
		#endif
	#elif LED_2_max_current == 3
		#if LED_2_current > 300
			#define LED2_ICTRL 255
		#elif LED_2_current <= 0	
			#define LED2_ICTRL 0
		#else
			#define LED2_ICTRL (uint8_t)((LED_2_current * 255) / 300)
		#endif
	#else
		#error "Invalid LED_2_max_current value, should be 0, 1, 2, or 3"
		#define LED2_ICTRL 0
	#endif
#endif

// Calculations for LED 2
#if defined(LED_3_current) && defined(LED_3_max_current)
	#if LED_3_max_current == 0
		#if LED_3_current >= 25
			#define LED3_ICTRL 255
		#elif LED_3_current <= 0
			#define LED3_ICTRL 0
		#else
			#define LED3_ICTRL (uint8_t)((LED_3_current * 255) / 25)
		#endif
	#elif LED_3_max_current == 1
		#if LED_3_current >= 150
			#define LED3_ICTRL 255
		#elif LED_3_current <= 0
			#define LED3_ICTRL 0
		#else
			#define LED3_ICTRL (uint8_t)((LED_3_current * 255) / 150)
		#endif
	#elif LED_3_max_current == 2
		#if LED_3_current >= 225
			#define LED3_ICTRL 255
		#elif LED_3_current <= 0
			#define LED3_ICTRL 0
		#else
			#define LED3_ICTRL (uint8_t)((LED_3_current * 255) / 225)
		#endif
	#elif LED_3_max_current == 3
		#if LED_3_current >= 300
			#define LED3_ICTRL 255
		#elif LED_3_current <= 0	
			#define LED3_ICTRL 0
		#else
			#define LED3_ICTRL (uint8_t)((LED_3_current * 255) / 300)
		#endif
	#else
		#error "Invalid LED_3_max_current value, should be 0, 1, 2, or 3"
		#define LED3_ICTRL 0
	#endif
#endif

// Calculations for LED 4
#if defined(LED_4_current) && defined(LED_4_max_current)
	#if LED_4_max_current == 0
		#if LED_4_current >= 25
			#define LED4_ICTRL 255
		#elif LED_4_current <= 0
			#define LED4_ICTRL 0
		#else
			#define LED4_ICTRL (uint8_t)((LED_4_current * 255) / 25)
		#endif
	#elif LED_4_max_current == 1
		#if LED_4_current >= 150
			#define LED4_ICTRL 255
		#elif LED_4_current <= 0
			#define LED4_ICTRL 0
		#else
			#define LED4_ICTRL (uint8_t)((LED_4_current * 255) / 150)
		#endif
	#elif LED_4_max_current == 2
		#if LED_4_current >= 225
			#define LED4_ICTRL 255
		#elif LED_4_current <= 0
			#define LED4_ICTRL 0
		#else
			#define LED4_ICTRL (uint8_t)((LED_4_current * 255) / 225)
		#endif
	#elif LED_4_max_current == 3
		#if LED_4_current >= 300
			#define LED4_ICTRL 255
		#elif LED_4_current <= 0	
			#define LED4_ICTRL 0
		#else
			#define LED4_ICTRL (uint8_t)((LED_4_current * 255) / 300)
		#endif
	#else
		#error "Invalid LED_4_max_current value, should be 0, 1, 2, or 3"
		#define LED4_ICTRL 0
	#endif
#endif

// Calculations for LED 5
#if defined(LED_5_current) && defined(LED_5_max_current)
	#if LED_5_max_current == 0
		#if LED_5_current >= 25
			#define LED5_ICTRL 255
		#elif LED_5_current <= 0
			#define LED5_ICTRL 0
		#else
			#define LED5_ICTRL (uint8_t)((LED_5_current * 255) / 25)
		#endif
	#elif LED_5_max_current == 1
		#if LED_5_current >= 150
			#define LED5_ICTRL 255
		#elif LED_5_current <= 0
			#define LED5_ICTRL 0
		#else
			#define LED5_ICTRL (uint8_t)((LED_5_current * 255) / 150)
		#endif
	#elif LED_5_max_current == 2
		#if LED_5_current >= 225
			#define LED5_ICTRL 255
		#elif LED_5_current <= 0
			#define LED5_ICTRL 0
		#else
			#define LED5_ICTRL (uint8_t)((LED_5_current * 255) / 225)
		#endif
	#elif LED_5_max_current == 3
		#if LED_5_current >= 300
			#define LED5_ICTRL 255
		#elif LED_5_current <= 0	
			#define LED5_ICTRL 0
		#else
			#define LED5_ICTRL (uint8_t)((LED_5_current * 255) / 300)
		#endif
	#else
		#error "Invalid LED_5_max_current value, should be 0, 1, 2, or 3"
		#define LED5_ICTRL 0
	#endif
#endif

// Calculations for LED 6
#if defined(LED_6_current) && defined(LED_6_max_current)
	#if LED_6_max_current == 0
		#if LED_6_current >= 25
			#define LED6_ICTRL 255
		#elif LED_6_current <= 0
			#define LED6_ICTRL 0
		#else
			#define LED6_ICTRL (uint8_t)((LED_6_current * 255) / 25)
		#endif
	#elif LED_6_max_current == 1
		#if LED_6_current >= 150
			#define LED6_ICTRL 255
		#elif LED_6_current <= 0
			#define LED6_ICTRL 0
		#else
			#define LED6_ICTRL (uint8_t)((LED_6_current * 255) / 150)
		#endif
	#elif LED_6_max_current == 2
		#if LED_6_current >= 225
			#define LED6_ICTRL 255
		#elif LED_6_current <= 0
			#define LED6_ICTRL 0
		#else
			#define LED6_ICTRL (uint8_t)((LED_6_current * 255) / 225)
		#endif
	#elif LED_6_max_current == 3
		#if LED_6_current >= 300
			#define LED6_ICTRL 255
		#elif LED_6_current <= 0	
			#define LED6_ICTRL 0
		#else
			#define LED6_ICTRL (uint8_t)((LED_6_current * 255) / 300)
		#endif
	#else
		#error "Invalid LED_6_max_current value, should be 0, 1, 2, or 3"
		#define LED6_ICTRL 0
	#endif
#endif

// Calculations for LED 7
#if defined(LED_7_current) && defined(LED_7_max_current)
	#if LED_7_max_current == 0
		#if LED_7_current >= 25
			#define LED7_ICTRL 255
		#elif LED_7_current <= 0
			#define LED7_ICTRL 0
		#else
			#define LED7_ICTRL (uint8_t)((LED_7_current * 255) / 25)
		#endif
	#elif LED_7_max_current == 1
		#if LED_7_current >= 150
			#define LED7_ICTRL 255
		#elif LED_7_current <= 0
			#define LED7_ICTRL 0
		#else
			#define LED7_ICTRL (uint8_t)((LED_7_current * 255) / 150)
		#endif
	#elif LED_7_max_current == 2
		#if LED_7_current >= 225
			#define LED7_ICTRL 255
		#elif LED_7_current <= 0
			#define LED7_ICTRL 0
		#else
			#define LED7_ICTRL (uint8_t)((LED_7_current * 255) / 225)
		#endif
	#elif LED_7_max_current == 3
		#if LED_7_current >= 300
			#define LED7_ICTRL 255
		#elif LED_7_current <= 0	
			#define LED7_ICTRL 0
		#else
			#define LED7_ICTRL (uint8_t)((LED_7_current * 255) / 300)
		#endif
	#else
		#error "Invalid LED_7_max_current value, should be 0, 1, 2, or 3"
		#define LED7_ICTRL 0
	#endif
#endif

// Calculations for LED 8
#if defined(LED_8_current) && defined(LED_8_max_current)
	#if LED_8_max_current == 0
		#if LED_8_current >= 25
			#define LED8_ICTRL 255
		#elif LED_8_current <= 0
			#define LED8_ICTRL 0
		#else
			#define LED8_ICTRL (uint8_t)((LED_8_current * 255) / 25)
		#endif
	#elif LED_8_max_current == 1
		#if LED_8_current >= 150
			#define LED8_ICTRL 255
		#elif LED_8_current <= 0
			#define LED8_ICTRL 0
		#else
			#define LED8_ICTRL (uint8_t)((LED_8_current * 255) / 150)
		#endif
	#elif LED_8_max_current == 2
		#if LED_8_current >= 225
			#define LED8_ICTRL 255
		#elif LED_8_current <= 0
			#define LED8_ICTRL 0
		#else
			#define LED8_ICTRL (uint8_t)((LED_8_current * 255) / 225)
		#endif
	#elif LED_8_max_current == 3
		#if LED_8_current >= 300
			#define LED8_ICTRL 255
		#elif LED_8_current <= 0	
			#define LED8_ICTRL 0
		#else
			#define LED8_ICTRL (uint8_t)((LED_8_current * 255) / 300)
		#endif
	#else
		#error "Invalid LED_8_max_current value, should be 0, 1, 2, or 3"
		#define LED8_ICTRL 0
	#endif
#endif

// Calculations for PPG frequency
#define PPG_FREQL (uint8_t)(32000/(PPG_FREQ)-1) 		// n = 1/(f*31,25us) -1 low byte of PPG frequency setting
#define PPG_FREQH (uint8_t)((32000/(PPG_FREQ)-1) >> 8) 	// high byte of PPG frequency setting

// Calculations for ECG frequency
#if ECG_FREQ <= 0
	#define ECG_FREQL 0
	#define ECG_FREQH 0
#else 
	#define ECG_FREQL (uint8_t)(32000/(ECG_FREQ)-1)			// n = 1/(f*31,25us) -1 low byte of PPG frequency setting
	#define ECG_FREQH (uint8_t)((32000/(ECG_FREQ)-1) >> 8) 	// high byte of PPG frequency setting
#endif

// Checking for FIFO threshold	
#if FIFO_SAMPLE_COUNT > FIFO_MAX_LEVEL
	#define FIFO_THRESHOLD 511
	#error "FIFO_SAMPLE_COUNT cannot exceed FIFO_MAX_LEVEL (511)"
#else
	#define FIFO_THRESHOLD FIFO_SAMPLE_COUNT
#endif

// Checking PPG Modulator 1 enabling
#if PPG_MOD_1 >= 1
	#define PPGMOD1_EN 1
#elif PPG_MOD_1 <= 0
	#define PPGMOD1_EN 0
#endif

// Checking PPG Modulator 2 enabling
#if PPG_MOD_2 >= 1
	#define PPGMOD2_EN 1
#elif PPG_MOD_2 <= 0
	#define PPGMOD2_EN 0
#endif

/**
 * @brief Write multiple bytes to AS7058 registers starting at reg_addr
 *
 * @param dev is the I2C bus
 * @param reg_addr is the first register address to write to
 * @param number is the number of bytes to write
 * @param values is the pointer to the bytes to write
 */
int as7058_write_reg(const struct i2c_dt_spec *dev, uint8_t reg_addr, uint8_t number, uint8_t* values) {

	if(values == NULL){
		return -1;
	}

	int ret = i2c_burst_write_dt(dev, reg_addr, values, number); // write multiple bytes starting at reg_addr

    return ret;
}

/**
 * @brief Read multiple bytes from AS7058 registers starting at reg_addr
 *
 * @param dev is the I2C bus
 * @param reg_addr is the first register address to read from
 * @param number is the number of bytes to read
 * @param values is the pointer to the bytes to read
 */
int as7058_read_reg(const struct i2c_dt_spec *dev, uint8_t reg_addr, uint8_t number, uint8_t* values) {

	int ret = i2c_burst_read_dt(dev, reg_addr, values, number); // read multiple bytes starting at reg_addr
	
	if(ret<0){
		LOG_ERR("Reading from a register group starting at reg_addr %u returned %d", reg_addr, ret);
	}

	return ret;
}

/**
 * @brief Initialize AS7058 sensor
 *
 * @param dev is the I2C bus
 * @return 0 if successful, negative error code if unsuccessful
 */
int as7058_init(const struct i2c_dt_spec *dev){

	/*
		Comment for registers configs writen starting from lower bit/bits
	*/

	int ret;

	// Power registers setup
	uint8_t reg_pwr_cfg[2] = 
	{
		0b00000111, // PWR_ON: Enables: CONF CTRL MOD1 MOD2
		0b00000000, // PWR_ISO: Disables: CONF
	};
	ret = as7058_write_reg(dev, AS7058_REGADDR_PWR_ON, sizeof(reg_pwr_cfg), reg_pwr_cfg);
	if(ret<0){
		LOG_ERR("Writing to a register group starting at AS7058_REGADDR_PWR_ON returned %d", ret);
	}

	// Clock and standby registers setup
	uint8_t reg_clk_standby_cfg[20] =
	{
		0b00000111, // CLK_CFG: Enables: 32kHz (lf_osc_on), 2MHz (hf_osc_on), 20MHz PLL (pll_on)
		0b00111111, /*  REF_CFG1: 
						Enables: IOS DAC and IREF DAC (en_bias_ppg_idac), current reference (en_bias_iref), 
						LED driver voltage (en_vr_led), PPG common-mode voltage (en_vcm_ppg), Bandgap (en_bg);
						Selecting: Common mode volatge = 0.75V (sel_vcm)
					*/ 
		0b00000000, // REF_CFG2: LEFT DEFAULT
		0b00000000, // REF_CFG3: LEFT DEFAULT
		0b01111111, // STANDBY_ON1: LEFT DEFAULT
		0b00000000, // STANDBY_ON2: LEFT DEFAULT
		0b11111111, // STANDBY_EN1: LEFT DEFAULT
		0b11111111, // STANDBY_EN2: LEFT DEFAULT
		0b11111111, // STANDBY_EN3: LEFT DEFAULT
		0b11111111, // STANDBY_EN4: LEFT DEFAULT
		0b11111111, // STANDBY_EN5: LEFT DEFAULT
		0b11111111, // STANDBY_EN6: LEFT DEFAULT
		0b11111111, // STANDBY_EN7: LEFT DEFAULT
		0b11111111, // STANDBY_EN8: LEFT DEFAULT
		0b11111111, // STANDBY_EN9: LEFT DEFAULT
		0b11111111, // STANDBY_EN10: LEFT DEFAULT
		0b11111111, // STANDBY_EN11: LEFT DEFAULT
		0b11111111, // STANDBY_EN12: LEFT DEFAULT
		0b11111111, // STANDBY_EN13: LEFT DEFAULT
		0b11111111, // STANDBY_EN13: LEFT DEFAULT
	};
	ret = as7058_write_reg(dev, AS7058_REGADDR_CLK_CFG, sizeof(reg_clk_standby_cfg), reg_clk_standby_cfg);
	if(ret<0){
		LOG_ERR("Writing to a register group starting at AS7058_REGADDR_CLK_CFG returned %d", ret);
	}

	// I2C, interrupt, and GPIO registers setup
	uint8_t reg_ctrl_cfg[6] = 
	{
		0b00000000, // I2C_MODE: LEFT DEFAULT
		0b00000000, // INT_CFG: LEFT DEFAULT
		0b01001000, // IF_CFG: LEFT DEFAULT
		0b00000000, // GPIO_CFG1: LEFT DEFAULT
		0b00000000, // GPIO_CFG2: LEFT DEFAULT
		0b00000000, // IO_CFG: LEFT DEFAULT
	};
	ret = as7058_write_reg(dev, AS7058_REGADDR_I2C_MODE, sizeof(reg_ctrl_cfg), reg_ctrl_cfg);
	if(ret<0){
		LOG_ERR("Writing to a register group starting at AS7058_REGADDR_I2C_MODE returned %d", ret);
	}

	// PPG MOD register group
	uint8_t reg_ppg_cfg[9] = {
        0, 	 // PPGMOD_CFG1
        0, 	 // PPGMOD_CFG2: DOES NOT EXIST IN DATASHEET
        0, 	 // PPGMOD_CFG3
        0 | (PPGMOD1_EN << 7),	// PPGMOD1_CFG1
        4,	 // PPGMOD1_CFG2
        0,	 // PPGMOD1_CFG3
        0 | (PPGMOD2_EN << 7),	// PPGMOD2_CFG1
        4, 	 // PPGMOD2_CFG2
        0, 	 // PPGMOD2_CFG3
    };
	ret = as7058_write_reg(dev, AS7058_REGADDR_PPGMOD_CFG1, sizeof(reg_ppg_cfg), reg_ppg_cfg);
	if(ret<0){
		LOG_ERR("Writing to a register group starting at AS7058_REGADDR_PPGMOD_CFG1 returned %d", ret);
	}

	// LED registers setup
	uint8_t reg_led_cfg[25] = 
	{
		0x57, 		// VCSEL_PASSWORD: 0x57 unlocks LED driver registers
		0b11000000, // VCSEL_CFG: Disable: VCSEL watchdog, safety control logic evaluation of the short to the VSS/VDD signals.
		0b00000000, // VCSEL_MODE: Selecting: LED1-8 are used as LED drivers (not VCSEL)
		0b00000001, // LED_CFG: Disable: the LED Watchdog (led_wd_disable=1)
		0b00000000, // LED_DRV1: LEFT DEFAULT
		0b00000000, // LED_DRV2: LEFT DEFAULT
		LED1_ICTRL, 			// LED1_ICTRL: LEFT DEFAULT
		LED2_ICTRL, 			// LED2_ICTRL: set LED2 current to ~15mA, calculated as (current range/255)*led2_ictrl = (150mA/255)*26 = ~15mA
		LED3_ICTRL, 			// LED3_ICTRL: LEFT DEFAULT
		LED4_ICTRL, 			// LED4_ICTRL: LEFT DEFAULT
		LED5_ICTRL, 			// LED5_ICTRL: LEFT DEFAULT
		LED6_ICTRL, 			// LED6_ICTRL: LEFT DEFAULT
		LED7_ICTRL, 			// LED7_ICTRL: LEFT DEFAULT
		LED8_ICTRL,				// LED8_ICTRL: LEFT DEFAULT
		((LED_4_max_current << 6) | (LED_3_max_current << 4) | (LED_2_max_current << 2) | LED_1_max_current), 
					// LED_IRNG1: Selecting: LED1-4 current range
		((LED_8_max_current << 6) | (LED_7_max_current << 4) | (LED_6_max_current << 2) | LED_5_max_current), 
					// LED_IRNG2: Selecting: LED5-8 current range
		LED_SUB1, // LED_SUB1
		LED_SUB2, // LED_SUB2
		LED_SUB3, // LED_SUB3
		LED_SUB4, // LED_SUB4: LEFT DEFAULT
		LED_SUB5, // LED_SUB5: LEFT DEFAULT
		LED_SUB6, // LED_SUB6: LEFT DEFAULT
		LED_SUB7, // LED_SUB7: LEFT DEFAULT
		LED_SUB8, // LED_SUB8: LEFT DEFAULT
		0b00000000, // LOWVDS_WAIT: LEFT DEFAULT
	};
	ret = as7058_write_reg(dev, AS7058_REGADDR_VCSEL_PASSWORD, sizeof(reg_led_cfg), reg_led_cfg);
	if(ret<0){
		LOG_ERR("Writing to a register group starting at AS7058_REGADDR_VCSEL_PASSWORD returned %d", ret);
	}

	// Photodiodes registers setup
	uint8_t reg_pd_cfg[22] ={
		0b00000000,  // PDSEL_CFG: LEFT DEFAULT
		PPG1_PDSEL1, // PPG1_PDSEL1
		PPG1_PDSEL2, // PPG1_PDSEL2
		PPG1_PDSEL3, // PPG1_PDSEL3
		PPG1_PDSEL4, // PPG1_PDSEL4
		PPG1_PDSEL5, // PPG1_PDSEL5
		PPG1_PDSEL6, // PPG1_PDSEL6
		PPG1_PDSEL7, // PPG1_PDSEL7
		PPG1_PDSEL8, // PPG1_PDSEL8
		PPG2_PDSEL1, // PPG2_PDSEL1
		PPG2_PDSEL2, // PPG2_PDSEL2
		PPG2_PDSEL3, // PPG2_PDSEL3
		PPG2_PDSEL4, // PPG2_PDSEL4
		PPG2_PDSEL5, // PPG2_PDSEL5
		PPG2_PDSEL6, // PPG2_PDSEL6
		PPG2_PDSEL7, // PPG2_PDSEL7
		PPG2_PDSEL8, // PPG2_PDSEL8
		0b00000000,  // PPG2_AFESEL1: LEFT DEFAULT
		0b00000000,  // PPG2_AFESEL2: LEFT DEFAULT
		0b00000000,  // PPG2_AFESEL3: LEFT DEFAULT
		0b00000000,  // PPG2_AFESEL4: LEFT DEFAULT
		0b00000000,  // PPG2_AFEEN: LEFT DEFAULT
	};
	ret = as7058_write_reg(dev, AS7058_REGADDR_PDSEL_CFG, sizeof(reg_pd_cfg), reg_pd_cfg);
	if(ret<0){
		LOG_ERR("Writing to a register group starting at AS7058_REGADDR_PDSEL_CFG returned %d", ret);
	}

	// SINC filter registers setup
	uint8_t reg_sinc_cfg[11] = {
		0b00010011, // PPG_SINC_CFGA: Selecting: Value for decimation = 128 (ppg_sinc_dec=3)
		0b00000011, // PPG_SINC_CFGB: Selecting: CIC filter (ppg_filter_mode=1), Filter order = 5 (ppg_sel_order=1)
		0b00000000, // PPG_SINC_CFGC: LEFT DEFAULT
		0b00000000, // PPG_SINC_CFGD: LEFT DEFAULT
		0b00000000, // ECG1_SINC_CFGA: LEFT DEFAULT
		0b00000001, // ECG1_SINC_CFGB: LEFT DEFAULT
		0b00000000, // ECG1_SINC_CFGC: LEFT DEFAULT
		0b00000000, // ECG2_SINC_CFGA: LEFT DEFAULT
		0b00000001, // ECG2_SINC_CFGB: LEFT DEFAULT
		0b00000000, // ECG2_SINC_CFGC: LEFT DEFAULT
		0b00000000, // ECG_SINC_CFG: LEFT DEFAULT
	};
	ret = as7058_write_reg(dev, AS7058_REGADDR_PPG_SINC_CFGA, sizeof(reg_sinc_cfg), reg_sinc_cfg);
	if(ret<0){
		LOG_ERR("Writing to a register group starting at AS7058_REGADDR_PPG_SINC_CFGA returned %d", ret);
	}

	// Photodiode offset (IOS) registers setup
	uint8_t reg_ios_cfg[18] = {
		0b00000000, // IOS_PPG1_SUB1: LEFT DEFAULT
		0b00000000, // IOS_PPG1_SUB2: LEFT DEFAULT
		0b00000000, // IOS_PPG1_SUB3: LEFT DEFAULT
		0b00000000, // IOS_PPG1_SUB4: LEFT DEFAULT
		0b00000000, // IOS_PPG1_SUB5: LEFT DEFAULT
		0b00000000, // IOS_PPG1_SUB6: LEFT DEFAULT
		0b00000000, // IOS_PPG1_SUB7: LEFT DEFAULT
		0b00000000, // IOS_PPG1_SUB8: LEFT DEFAULT
		0b00000000, // IOS_PPG2_SUB1: LEFT DEFAULT
		0b00000000, // IOS_PPG2_SUB2: LEFT DEFAULT
		0b00000000, // IOS_PPG2_SUB3: LEFT DEFAULT
		0b00000000, // IOS_PPG2_SUB4: LEFT DEFAULT
		0b00000000, // IOS_PPG2_SUB5: LEFT DEFAULT
		0b00000000, // IOS_PPG2_SUB6: LEFT DEFAULT
		0b00000000, // IOS_PPG2_SUB7: LEFT DEFAULT
		0b00000000, // IOS_PPG2_SUB8: LEFT DEFAULT
		0b00000000, // IOS_LEDOFF: LEFT DEFAULT
		0b00000000, // IOS_CFG: LEFT DEFAULT
	};
	ret = as7058_write_reg(dev, AS7058_REGADDR_IOS_PPG1_SUB1, sizeof(reg_ios_cfg), reg_ios_cfg);
	if(ret<0){
		LOG_ERR("Writing to a register group starting at AS7058_REGADDR_IOS_PPG1_SUB1 returned %d", ret);
	}

	// AAOC registers setup
	uint8_t reg_aaoc_cfg[4] = {
		0b00000000, // AOC_SAR_THRES: LEFT DEFAULTs
		0b00000000, // AOC_SAR_RANGE: LEFT DEFAULT
		0b00000000, // AOC_SAR_PPG1: LEFT DEFAULT
		0b00000000, // AOC_SAR_PPG2: LEFT DEFAULT
	};
	ret = as7058_write_reg(dev, AS7058_REGADDR_AOC_SAR_THRES, sizeof(reg_aaoc_cfg), reg_aaoc_cfg);
	if(ret<0){
		LOG_ERR("Writing to a register group starting at AS7058_REGADDR_AOC_SAR_THRES returned %d", ret);
	}

	// PP(Post processing) registers setup
	uint8_t reg_pp_cfg[5] = {
		0b00000000, // PP_CFG: LEFT DEFAULT
		0b00000000, // PPG1_PP1: LEFT DEFAULT
		0b00000000, // PPG1_PP2: LEFT DEFAULT
		0b00000000, // PPG2_PP1: LEFT DEFAULT
		0b00000000, // PPG2_PP2: LEFT DEFAULT
	};
	ret = as7058_write_reg(dev, AS7058_REGADDR_PP_CFG, sizeof(reg_pp_cfg), reg_pp_cfg);
	if(ret<0){
		LOG_ERR("Writing to a register group starting at AS7058_REGADDR_PP_CFG returned %d", ret);
	}

	// Sequencer registers setup
	uint8_t reg_seq_cfg[29] = {
		0b00000111, // IRQ_ENABLE: Enabels interrupt: Sequencer (irq_en_sequencer), FIFO is almost full (irq_en_fifothreshold), FIFO overflow (irq_en_fifooverflow)
		0b00000000, // PPG_SUBWAIT: LEFT DEFAULT 
		0b00000000, // PPG_SAR_WAIT: LEFT DEFAULT
		PPG_LED_INIT, 		// PPG_LED_INIT: t_LED_INIT = N * 1us = 10 * 1us = 10us
		PPG_FREQL,		// PPG_FREQL: 
		PPG_FREQH,			// PPG_FREQH: 
		0b00000111, // PPG1_SUB_EN: Enabling: sub sample 1 for ppg1
		0b00000000, // PPG2_SUB_EN: LEFT DEFAULT
		0b00000001, // PPG_MODE1: LEFT DEFAULT
		0b00000001, // PPG_MODE2: LEFT DEFAULT
		0b00000001, // PPG_MODE3: LEFT DEFAULT
		0b00000000, // PPG_MODE4: LEFT DEFAULT
		0b00000000, // PPG_MODE5: LEFT DEFAULT
		0b00000000, // PPG_MODE6: LEFT DEFAULT
		0b00000000, // PPG_MODE7: LEFT DEFAULT
		0b00000000, // PPG_MODE8: LEFT DEFAULT
		0b00000000, // PPG_CFG: LEFT DEFAULT
		ECG_FREQL,  // ECG_FREQL: Disabling ECG frequency
		ECG_FREQH,  // ECG_FREQH: Disabling ECG frequency
		0b00000000, // ECG1_FREQDIVL: LEFT DEFAULT
		0b00000000, // ECG1_FREQDIVH: LEFT DEFAULT
		0b00000000, // ECG2_FREQDIVL: LEFT DEFAULT
		0b00000000, // ECG2_FREQDIVH: LEFT DEFAULT
		0b00000000, // ECG_SUBS: LEFT DEFAULT
		0b00000001, // LEADOFF_INITL: LEFT DEFAULT
		0b00000000, // LEADOFF_INITH: LEFT DEFAULT
		0b00000001, // ECG_INITL: LEFT DEFAULT
		0b00000001, // ECG_INITH: LEFT DEFAULT
		0b00000000, // SAMPLE_NUM: Sequencer runs continuously, since register is set to 0
	};
	ret = as7058_write_reg(dev, AS7058_REGADDR_IRQ_ENABLE, sizeof(reg_seq_cfg), reg_seq_cfg);
	if(ret<0){
		LOG_ERR("Writing to a register group starting at AS7058_REGADDR_IRQ_ENABLE returned %d", ret);
	}
	// uint8_t reg_ecg_cfg[18] = {
	// 		0b00000010, // BIOZ_CFG: LEFT DEFAULT
	// 		0b01100000, // BIOZ_EXCIT: LEFT DEFAULT
	// 		0b00000000, // BIOZ_MIXER: LEFT DEFAULT
	// 		0b00000110, // BIOZ_SELECT: LEFT DEFAULT
	// 		0b00000000, // BIOZ_GAIN: LEFT DEFAULT
	// 		0b00001000, // ECGMOD_CFG1: LEFT DEFAULT
	// 		0b00000000, // ECGMOD_CFG2: LEFT DEFAULT
	// 		0b01000000, // ECGIMUX_CFG1: LEFT DEFAULT
	// 		0b00110011, // ECGIMUX_CFG2: LEFT DEFAULT
	// 		0b00000000, // ECGIMUX_CFG3: LEFT DEFAULT
	// 		0b01100010, // ECGAMP_CFG1: 
	// 		0b00000000, // ECGAMP_CFG2: LEFT DEFAULT
	// 		0b00011001, // ECGAMP_CFG3: 
	// 		0b00000000, // ECGAMP_CFG4: LEFT DEFAULT
	// 		0b00100011, // ECGAMP_CFG5: 
	// 		0b00000101, // ECGAMP_CFG6: 
	// 		0b00100010, // ECGAMP_CFG7: LEFT DEFAULT
	// 		0b00000011, // ECG_BIOZ: LEFT DEFAULT
	// 	};
	// ECG/BioZ registers setup
	uint8_t reg_ecg_cfg[18] = {
		0b00000010, // BIOZ_CFG: LEFT DEFAULT
		0b00110000, // BIOZ_EXCIT: LEFT DEFAULT
		0b00000000, // BIOZ_MIXER: LEFT DEFAULT
		0b00000110, // BIOZ_SELECT: LEFT DEFAULT
		0b00000000, // BIOZ_GAIN: LEFT DEFAULT
		0b00001000, // ECGMOD_CFG1: LEFT DEFAULT
		0b00000000, // ECGMOD_CFG2: LEFT DEFAULT
		0b01000000, // ECGIMUX_CFG1: LEFT DEFAULT
		0b00110011, // ECGIMUX_CFG2: LEFT DEFAULT
		0b00000000, // ECGIMUX_CFG3: LEFT DEFAULT
		0b01100010, // ECGAMP_CFG1: 
		0b00000000, // ECGAMP_CFG2: LEFT DEFAULT
		0b00011001, // ECGAMP_CFG3: 
		0b00000000, // ECGAMP_CFG4: LEFT DEFAULT
		0b00100011, // ECGAMP_CFG5: 
		0b00000101, // ECGAMP_CFG6: 
		0b00100010, // ECGAMP_CFG7: 
		0b00000011, // ECG_BIOZ: 
	};
	ret = as7058_write_reg(dev, AS7058_REGADDR_BIOZ_CFG, sizeof(reg_ecg_cfg), reg_ecg_cfg);
	if(ret<0){
		LOG_ERR("Writing to a register group starting at AS7058_REGADDR_BIOZ_CFG returned %d", ret);
	}

	// Lead-off registers setup
	uint8_t reg_leadoff_cfg[3] = {
		0b00000000, // LEADOFF_CFG: LEFT DEFAULT
		0b00000000, // LEADOFF_THRESL: LEFT DEFAULT
		0b00000000, // LEADOFF_THRESH: LEFT DEFAULT
	};
	ret = as7058_write_reg(dev, AS7058_REGADDR_LEADOFF_CFG, sizeof(reg_leadoff_cfg), reg_leadoff_cfg);
	if(ret<0){
		LOG_ERR("Writing to a register group starting at AS7058_REGADDR_LEADOFF_CFG returned %d", ret);
	}

	// FIFO registers setup
	uint8_t reg_fifo_cfg[2] = {
		(uint8_t)(FIFO_THRESHOLD & 0xFF), 	// FIFO_THRESHOLD: LEFT DEFAULT
		(uint8_t)(FIFO_THRESHOLD >> 8), 	// FIFO_CTRL: LEFT DEFAULT
	};
	ret = as7058_write_reg(dev, AS7058_REGADDR_FIFO_THRESHOLD, sizeof(reg_fifo_cfg), reg_fifo_cfg);
	if(ret<0){
		LOG_ERR("Writing to a register group starting at AS7058_REGADDR_FIFO_THRESHOLD returned %d", ret);
	}

	return ret;
}

/**
 * @brief Read the number of samples currently in the AS7058 FIFO, up to 512
 *
 * @param dev is the I2C bus
 * @param level is a pointer to store the FIFO level
 * @return 0 if successful, negative error code if unsuccessful
 */
int as7058_fifo_read_level(const struct i2c_dt_spec *dev, uint16_t *level)
{
    uint8_t fifo_level[2];
	int ret = as7058_read_reg(dev, AS7058_REGADDR_FIFO_LEVEL0, sizeof(fifo_level), fifo_level);
	if(ret<0){
		LOG_ERR("Reading from AS7058_REGADDR_FIFO_LEVEL0 returned %d", ret);
		return ret;
	}
    *level = ((uint16_t)(fifo_level[1] & 0x03) << 8) | fifo_level[0];
    return ret;
}

/**
 * @brief Read the fifo register raw
 *
 * @param dev is the I2C bus
 * @param raw is a pointer to store the FIFO level
 * @return 0 if successful, negative error code if unsuccessful
 */
int as7058_read_fifo_raw(const struct i2c_dt_spec *dev, struct as7058_fifo_t *raw)
{
	int ret = as7058_read_reg(dev, AS7058_REGADDR_FIFOL, 3, (uint8_t *)raw);
	if(ret<0)
	{
		LOG_ERR("Reading from AS7058_REGADDR_FIFOL returned %d", ret);
	}

	return ret;
}	

/**
 * @brief Read one sample from the AS7058 FIFO and store it in the provided as7058_sample_t struct
 *
 * @param dev is the I2C bus
 * @param sample is a pointer to the sample struct to store the read data
 * @return 0 if successful, negative error code if unsuccessful
**/
int as7058_fifo_read_word(const struct i2c_dt_spec *dev, as7058_sample_t *sample)
{
	uint8_t fifo_data[3] = {0};

	int ret = as7058_read_reg(dev, AS7058_REGADDR_FIFOL, sizeof(fifo_data), fifo_data);
	if(ret<0){
		LOG_ERR("Reading from AS7058_REGADDR_FIFOL returned %d", ret);
    	return ret;
	}

	sample->raw = ((uint32_t)fifo_data[2] << 16) | ((uint32_t)fifo_data[1] << 8) | fifo_data[0]; // Combine the three bytes into a 24-bit raw sample, right-aligned in 32 bits
	sample->adc = ((uint32_t)(sample->raw & 0xFFFFF0)) >> 4; // Extract the 20-bit ADC value and sign-extend it to 32 bits
	sample->marker = fifo_data[0] & 0x07; // Extract the 3-bit data marker from the least significant bits of the first byte

    return ret;
}

/**
 * @brief Read one sample from the AS7058 FIFO and store it in the provided as7058_sample_t struct
 *
 * @param dev is the I2C bus
 * @param samples is a pointer to the array of sample structs to store the read data, should be an array of at least num_samples size
 * @param num_samples is the number of samples to read from the FIFO and store in the samples array
 * @return 0 if successful, negative error code if unsuccessful
**/
int as7058_read_fifo(const struct i2c_dt_spec *dev, as7058_sample_t *samples, uint16_t num_samples)
{

	int ret = 0;
	as7058_sample_t sample;

	for(uint16_t i = 0; i < num_samples; i++){
		ret = as7058_fifo_read_word(dev, &sample);
		samples[i] = sample;
		//memcpy(&samples[i], &sample, sizeof(as7058_sample_t));

		if(ret<0){
			LOG_ERR("Reading sample %d from FIFO returned %d", i, ret);
			return ret;
		}
	}
	return ret;
}

/**
 * @brief Write multiple bytes to AS7058 registers starting at AS7058_REGADDR_STATUS_SEQ
 *
 * @param dev is the I2C bus
 * @param status_out is a pointer to store the read status, to read full status, should be an array of at least 9 bytes to store all status registers
 * @return 0 if successful, negative error code if unsuccessful
 */
int as7058_check_status(const struct i2c_dt_spec *dev, uint8_t *status_out) {
	
	int ret;
	if(status_out == NULL){
		return -1;
	}
	ret = as7058_read_reg(dev, AS7058_REGADDR_STATUS_SEQ, sizeof(&status_out), status_out);
	return ret;
}

/**
 * @brief Print the AS7058 status registers in a human-readable format, interpreting the main status register and printing relevant information based on which bits are set. Also reads and prints specific status registers related to any active interrupts.
 *
 * @param dev is the I2C bus
 */
int as7058_print_status(const struct i2c_dt_spec *dev) {

	uint8_t status[9];
	int ret = as7058_check_status(dev, status);
	if (ret < 0) {
		LOG_ERR("Failed to read status: %d", ret);
		return ret;
	}

	if(status[8] & 0b00000001){ // Check the irq_sequencer bit in the main status register
		LOG_INF("Sequencer Interrupt");
		LOG_INF("STATUS_SEQ: %x ", status[0]);
		if((status[0] & 0b00000001)){ // Check if the error bit in the main status register is set
			LOG_ERR("(seq_error): Measurement of a sample was not started. Sample frequency is too high.");
		}
		if(((status[0]>>1) & 0b00000001) ){ // Check if the error bit in the main status register is set
			LOG_ERR("(seq_end): Measurement was stopped.");
		}
	}

	if(status[8] & 0b00000010){ // Check the irq_fifothreshold bit in the main status register
		LOG_INF("FIFO Almost Full Interrupt");
	}

	if(status[8] & 0b00000100){ // Check the irq_fifooverflow bit in the main status register
		LOG_INF("FIFO Overflow Interrupt");
	}

	if(status[8] & 0b00001000){ // Check the irq_led_lowvds bit in the main status register
		LOG_INF("LED lowvds Interrupt.");
		LOG_INF("STATUS_LED: %x; If active, the LED current does not reach the expected value.", status[1]);
	}

	if(status[8] & 0b00010000){ // Check the irq_asat bit in the main status register
		LOG_INF("Analog Saturation Interrupt.");
		LOG_INF("Analog Saturation Modulator1: %x; If active, the photodiode signal is saturated and the ADC value exceeds the maximum value.", (status[2] & 0xF0) >> 4);
		LOG_INF("Analog Saturation Modulator2: %x; If active, the photodiode signal is saturated and the ADC value exceeds the maximum value.", (status[2] & 0x0F));
		LOG_INF("Analog Saturation Modulator3: %x; If active, the photodiode signal is saturated and the ADC value exceeds the maximum value.", (status[3] & 0x0F));
	}

	if(status[8] & 0b00100000){ // Check the irq_vcsel bit in the main status register
		LOG_INF("VCSEL Error Interrupt.");
		LOG_INF("STATUS_VCSEL: %x;", status[4]);
		LOG_INF("STATUS_VCSEL_VSS: %x; If active detected short circuit to the VSS; LED1 -> LED8 = Bit 0 -> Bit 7", status[5]);
		LOG_INF("STATUS_VCSEL_VDD: %x; If active detected short circuit to the VDD; LED1 -> LED8 = Bit 0 -> Bit 7", status[6]);
		if((status[4] & 0b00000001)){ // Check vcsel_wd bit 0 in the STATUS_VCSEL register
			LOG_ERR("(vcsel_wd): VCSEL LED analog Watchdog occurred t > 3.2 ms; LED Driver 1");
		}
		if((status[4] & 0b00000010)){ // Check vcsel_wd bit 1 in the STATUS_VCSEL register
			LOG_ERR("(vcsel_wd): VCSEL LED analog Watchdog occurred t > 3.2 ms; LED Driver 2");
		}
		if((status[4] & 0b00000100)){ // Check vcsel_vdd bit in the STATUS_VCSEL register
			LOG_ERR("(vcsel_vdd): VCSEL short to VCSELS detected");
		}
		if((status[4] & 0b00001000)){ // Check vcsel_vss bit in the STATUS_VCSEL register
			LOG_ERR("(vcsel_vss): VCSEL short to GND detected");
		}
		if((status[4] & 0b00010000)){ // Check led_wd bit in the STATUS_VCSEL register
			LOG_ERR("(led_wd): VCSEL LED digital Watchdog occurred t > 1.0 ms");
		}
	}

	if (status[8] & 0b01000000){ // Check the irq_leadoff bit in the main status register
		LOG_INF("Lead-Off Interrupt.");
		LOG_INF("STATUS_LEADOFF: %x", status[7]);
		if(status[7] & 0b00000001){ // Check leadoff_off 0 in the STATUS_LEADOFF register
			LOG_ERR("(leadoff_off): Lead-Off has been deactivated. Automatic Reset by reading.");
		}
		if(status[7] & 0b00000010){ // Check leadoff_on in the STATUS_LEADOFF register
			LOG_ERR("(leadoff_on): Lead-Off has been activated. Automatic Reset by reading");
		}
		if(status[7] & 0b00000100){ // Check leadoff in the STATUS_LEADOFF register
			LOG_ERR("(leadoff): Internal Lead-Off");
		}
	}
	
	if (status[8] & 0b10000000){ // Check the irq_iir_overflow bit in the main status register
		LOG_INF("Interrupt status bit if the IIR filter block has an overflow condition.");
	}
	
	return ret;
}

/**
 * @brief Start the AS7058 measurement by first clearing the FIFO, then resetting the interrupt pin by reading the status registers, and finally writing to the SEQ_START register to start the measurement
 *
 * @param dev is the I2C bus
 * @return 0 if successful, negative error code if unsuccessful
 */
int as7058_start(const struct i2c_dt_spec *dev){

	int ret;
    uint8_t irq_status[9] = {0}, reg_value;

    // Ensuring clear FIFO before starting the measurement to avoid reading old samples from previous measurements
	ret = as7058_read_reg(dev, AS7058_REGADDR_FIFO_CTRL, 1, &reg_value);
	if(ret == 0){
		reg_value |= 0b10000000; // Setting FIFO clear bit to clear FIFO (fifo_clear=1)
		ret = as7058_write_reg(dev, AS7058_REGADDR_FIFO_CTRL, 1, &reg_value);
		if(ret<0){
			LOG_ERR("Writing to AS7058_REGADDR_FIFO_CTRL returned %d", ret);
			return ret;
		}
	}

    // Read interrupt status register to reset interrupt pin
	ret = as7058_check_status(dev, irq_status);
    if (ret<0){
		LOG_ERR("Reading from AS7058_REGADDR_STATUS_SEQ returned %d", ret);
	}
    // Read the main status register again to ensure that the interrupt pin is properly reset
	ret = as7058_read_reg(dev, AS7058_REGADDR_STATUS, 1, irq_status);
	if (ret<0){
		LOG_ERR("Reading from AS7058_REGADDR_STATUS returned %d", ret);
		return ret;
	}

	// Check if status register reseted properly, if not, return an error
	if ((ret == 0) && (0 != irq_status[0])) {
		LOG_ERR("AS7058 status register indicates an error, status: 0x%02x", irq_status[0]);
		return -1;
	}

	// Start the measurement by writing to the SEQ_START register
	ret = as7058_write_reg(dev, AS7058_REGADDR_SEQ_START, 1, (uint8_t[]){0b00000001}); // SEQ_START start measurement
	if(ret<0){
		LOG_ERR("Writing to AS7058_REGADDR_SEQ_START returned %d", ret);
	}

    return ret;
}

/**
 * @brief Stop the AS7058 measurement by writing to the SEQ_START register
 *
 * @param dev is the I2C bus
 * @return 0 if successful, negative error code if unsuccessful
 */
int as7058_stop(const struct i2c_dt_spec *dev){
	return as7058_write_reg(dev, AS7058_REGADDR_SEQ_START, 1, (uint8_t[]){0b00000000}); // SEQ_START stop measurement
}


/* TODO 
- Implement changing setting for LED current, frequency, etc. through writing it in the defines
*/