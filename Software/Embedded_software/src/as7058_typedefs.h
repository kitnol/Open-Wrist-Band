
#ifndef __AS7058_TYPEDEFS_H__
#define __AS7058_TYPEDEFS_H__

/*! Register definition of AS7058. */
enum as7058_reg_addresses {
    AS7058_REGADDR_OTP_BIOZ_REF_L = 0x0E,   /*!< Address of register OTP14. */
    AS7058_REGADDR_OTP_BIOZ_REF_H = 0x0F,   /*!< Address of register OTP15. */
    AS7058_REGADDR_OTP_GSR_REF_L = 0x13,    /*!< Address of register OTP19. */
    AS7058_REGADDR_OTP_GSR_REF_H = 0x14,    /*!< Address of register OTP20. */
    AS7058_REGADDR_OTP_TEMP_REF_L = 0x15,   /*!< Address of register OTP21. */
    AS7058_REGADDR_OTP_TEMP_REF_H = 0x16,   /*!< Address of register OTP22. */
    AS7058_REGADDR_CLK_CFG = 0x18,          /*!< Address of register CLK_CFG. */
    AS7058_REGADDR_REF_CFG1 = 0x19,         /*!< Address of register REF_CFG1. */
    AS7058_REGADDR_REF_CFG2 = 0x1A,         /*!< Address of register REF_CFG2. */
    AS7058_REGADDR_REF_CFG3 = 0x1B,         /*!< Address of register REF_CFG3. */
    AS7058_REGADDR_STANDBY_ON1 = 0x1C,      /*!< Address of register STANDBY_ON1. */
    AS7058_REGADDR_STANDBY_ON2 = 0x1D,      /*!< Address of register STANDBY_ON2. */
    AS7058_REGADDR_STANDBY_EN1 = 0x1E,      /*!< Address of register STANDBY_EN1. */
    AS7058_REGADDR_STANDBY_EN2 = 0x1F,      /*!< Address of register STANDBY_EN2. */
    AS7058_REGADDR_STANDBY_EN3 = 0x20,      /*!< Address of register STANDBY_EN3. */
    AS7058_REGADDR_STANDBY_EN4 = 0x21,      /*!< Address of register STANDBY_EN4. */
    AS7058_REGADDR_STANDBY_EN5 = 0x22,      /*!< Address of register STANDBY_EN5. */
    AS7058_REGADDR_STANDBY_EN6 = 0x23,      /*!< Address of register STANDBY_EN6. */
    AS7058_REGADDR_STANDBY_EN7 = 0x24,      /*!< Address of register STANDBY_EN7. */
    AS7058_REGADDR_STANDBY_EN8 = 0x25,      /*!< Address of register STANDBY_EN8. */
    AS7058_REGADDR_STANDBY_EN9 = 0x26,      /*!< Address of register STANDBY_EN9. */
    AS7058_REGADDR_STANDBY_EN10 = 0x27,     /*!< Address of register STANDBY_EN10. */
    AS7058_REGADDR_STANDBY_EN11 = 0x28,     /*!< Address of register STANDBY_EN11. */
    AS7058_REGADDR_STANDBY_EN12 = 0x29,     /*!< Address of register STANDBY_EN12. */
    AS7058_REGADDR_STANDBY_EN13 = 0x2A,     /*!< Address of register STANDBY_EN13. */
    AS7058_REGADDR_STANDBY_EN14 = 0x2B,     /*!< Address of register STANDBY_EN14. */
    AS7058_REGADDR_PWR_ON = 0x2D,           /*!< Address of register PWR_ON. */
    AS7058_REGADDR_PWR_ISO = 0x2E,          /*!< Address of register PWR_ISO. */
    AS7058_REGADDR_PWR_STAT = 0x2F,         /*!< Address of register PWR_STAT. */
    AS7058_REGADDR_I2C_MODE = 0x31,         /*!< Address of register I2C_MODE. */
    AS7058_REGADDR_INT_CFG = 0x32,          /*!< Address of register INT_CFG. */
    AS7058_REGADDR_IF_CFG = 0x33,           /*!< Address of register IF_CFG. */
    AS7058_REGADDR_GPIO_CFG1 = 0x34,        /*!< Address of register GPIO_CFG1. */
    AS7058_REGADDR_GPIO_CFG2 = 0x35,        /*!< Address of register GPIO_CFG2. */
    AS7058_REGADDR_IO_CFG = 0x36,           /*!< Address of register IO_CFG. */
    AS7058_REGADDR_PPGMOD_CFG1 = 0x37,      /*!< Address of register PPGMOD_CFG1. */
    AS7058_REGADDR_PPGMOD_CFG2 = 0x38,      /*!< Address of register PPGMOD_CFG2. */
    AS7058_REGADDR_PPGMOD_CFG3 = 0x39,      /*!< Address of register PPGMOD_CFG3. */
    AS7058_REGADDR_PPGMOD1_CFG1 = 0x3A,     /*!< Address of register PPGMOD1_CFG1. */
    AS7058_REGADDR_PPGMOD1_CFG2 = 0x3B,     /*!< Address of register PPGMOD1_CFG2. */
    AS7058_REGADDR_PPGMOD1_CFG3 = 0x3C,     /*!< Address of register PPGMOD1_CFG3. */
    AS7058_REGADDR_PPGMOD2_CFG1 = 0x3D,     /*!< Address of register PPGMOD2_CFG1. */
    AS7058_REGADDR_PPGMOD2_CFG2 = 0x3E,     /*!< Address of register PPGMOD2_CFG2. */
    AS7058_REGADDR_PPGMOD2_CFG3 = 0x3F,     /*!< Address of register PPGMOD2_CFG3. */
    AS7058_REGADDR_VCSEL_PASSWORD = 0x40,   /*!< Address of register VCSEL_PASSWORD. */
    AS7058_REGADDR_VCSEL_CFG = 0x41,        /*!< Address of register VCSEL_CFG. */
    AS7058_REGADDR_VCSEL_MODE = 0x42,       /*!< Address of register VCSEL_MODE. */
    AS7058_REGADDR_LED_CFG = 0x43,          /*!< Address of register LED_CFG. */
    AS7058_REGADDR_LED_DRV1 = 0x44,         /*!< Address of register LED_DRV1. */
    AS7058_REGADDR_LED_DRV2 = 0x45,         /*!< Address of register LED_DRV2. */
    AS7058_REGADDR_LED1_ICTRL = 0x46,       /*!< Address of register LED1_ICTRL. */
    AS7058_REGADDR_LED2_ICTRL = 0x47,       /*!< Address of register LED2_ICTRL. */
    AS7058_REGADDR_LED3_ICTRL = 0x48,       /*!< Address of register LED3_ICTRL. */
    AS7058_REGADDR_LED4_ICTRL = 0x49,       /*!< Address of register LED4_ICTRL. */
    AS7058_REGADDR_LED5_ICTRL = 0x4A,       /*!< Address of register LED5_ICTRL. */
    AS7058_REGADDR_LED6_ICTRL = 0x4B,       /*!< Address of register LED6_ICTRL. */
    AS7058_REGADDR_LED7_ICTRL = 0x4C,       /*!< Address of register LED7_ICTRL. */
    AS7058_REGADDR_LED8_ICTRL = 0x4D,       /*!< Address of register LED8_ICTRL. */
    AS7058_REGADDR_LED_IRNG1 = 0x4E,        /*!< Address of register LED_IRNG1. */
    AS7058_REGADDR_LED_IRNG2 = 0x4F,        /*!< Address of register LED_IRNG2. */
    AS7058_REGADDR_LED_SUB1 = 0x50,         /*!< Address of register LED_SUB1. */
    AS7058_REGADDR_LED_SUB2 = 0x51,         /*!< Address of register LED_SUB2. */
    AS7058_REGADDR_LED_SUB3 = 0x52,         /*!< Address of register LED_SUB3. */
    AS7058_REGADDR_LED_SUB4 = 0x53,         /*!< Address of register LED_SUB4. */
    AS7058_REGADDR_LED_SUB5 = 0x54,         /*!< Address of register LED_SUB5. */
    AS7058_REGADDR_LED_SUB6 = 0x55,         /*!< Address of register LED_SUB6. */
    AS7058_REGADDR_LED_SUB7 = 0x56,         /*!< Address of register LED_SUB7. */
    AS7058_REGADDR_LED_SUB8 = 0x57,         /*!< Address of register LED_SUB8. */
    AS7058_REGADDR_LOWVDS_WAIT = 0x58,      /*!< Address of register LOWVDS_WAIT. */
    AS7058_REGADDR_PDSEL_CFG = 0x59,        /*!< Address of register PDSEL_CFG. */
    AS7058_REGADDR_PPG1_PDSEL1 = 0x5A,      /*!< Address of register PPG1_PDSEL1. */
    AS7058_REGADDR_PPG1_PDSEL2 = 0x5B,      /*!< Address of register PPG1_PDSEL2. */
    AS7058_REGADDR_PPG1_PDSEL3 = 0x5C,      /*!< Address of register PPG1_PDSEL3. */
    AS7058_REGADDR_PPG1_PDSEL4 = 0x5D,      /*!< Address of register PPG1_PDSEL4. */
    AS7058_REGADDR_PPG1_PDSEL5 = 0x5E,      /*!< Address of register PPG1_PDSEL5. */
    AS7058_REGADDR_PPG1_PDSEL6 = 0x5F,      /*!< Address of register PPG1_PDSEL6. */
    AS7058_REGADDR_PPG1_PDSEL7 = 0x60,      /*!< Address of register PPG1_PDSEL7. */
    AS7058_REGADDR_PPG1_PDSEL8 = 0x61,      /*!< Address of register PPG1_PDSEL8. */
    AS7058_REGADDR_PPG2_PDSEL1 = 0x62,      /*!< Address of register PPG2_PDSEL1. */
    AS7058_REGADDR_PPG2_PDSEL2 = 0x63,      /*!< Address of register PPG2_PDSEL2. */
    AS7058_REGADDR_PPG2_PDSEL3 = 0x64,      /*!< Address of register PPG2_PDSEL3. */
    AS7058_REGADDR_PPG2_PDSEL4 = 0x65,      /*!< Address of register PPG2_PDSEL4. */
    AS7058_REGADDR_PPG2_PDSEL5 = 0x66,      /*!< Address of register PPG2_PDSEL2. */
    AS7058_REGADDR_PPG2_PDSEL6 = 0x67,      /*!< Address of register PPG2_PDSEL5. */
    AS7058_REGADDR_PPG2_PDSEL7 = 0x68,      /*!< Address of register PPG2_PDSEL6. */
    AS7058_REGADDR_PPG2_PDSEL8 = 0x69,      /*!< Address of register PPG2_PDSEL7. */
    AS7058_REGADDR_PPG2_AFESEL1 = 0x6A,     /*!< Address of register PPG2_AFESEL1. */
    AS7058_REGADDR_PPG2_AFESEL2 = 0x6B,     /*!< Address of register PPG2_AFESEL2. */
    AS7058_REGADDR_PPG2_AFESEL3 = 0x6C,     /*!< Address of register PPG2_AFESEL3. */
    AS7058_REGADDR_PPG2_AFESEL4 = 0x6D,     /*!< Address of register PPG2_AFESEL4. */
    AS7058_REGADDR_PPG2_AFEEN = 0x6E,       /*!< Address of register PPG2_AFEEN. */
    AS7058_REGADDR_PPG_SINC_CFGA = 0x6F,    /*!< Address of register PPG_SINC_CFGA. */
    AS7058_REGADDR_PPG_SINC_CFGB = 0x70,    /*!< Address of register PPG_SINC_CFGB. */
    AS7058_REGADDR_PPG_SINC_CFGC = 0x71,    /*!< Address of register PPG_SINC_CFGC. */
    AS7058_REGADDR_PPG_SINC_CFGD = 0x72,    /*!< Address of register PPG_SINC_CFGD. */
    AS7058_REGADDR_ECG1_SINC_CFGA = 0x73,   /*!< Address of register ECG1_SINC_CFGA. */
    AS7058_REGADDR_ECG1_SINC_CFGB = 0x74,   /*!< Address of register ECG1_SINC_CFGB. */
    AS7058_REGADDR_ECG1_SINC_CFGC = 0x75,   /*!< Address of register ECG1_SINC_CFGC. */
    AS7058_REGADDR_ECG2_SINC_CFGA = 0x76,   /*!< Address of register ECG2_SINC_CFGA. */
    AS7058_REGADDR_ECG2_SINC_CFGB = 0x77,   /*!< Address of register ECG2_SINC_CFGB. */
    AS7058_REGADDR_ECG2_SINC_CFGC = 0x78,   /*!< Address of register ECG2_SINC_CFGC. */
    AS7058_REGADDR_ECG_SINC_CFG = 0x79,     /*!< Address of register ECG_SINC_CFG. */
    AS7058_REGADDR_IOS_PPG1_SUB1 = 0x7A,    /*!< Address of register IOS_PPG1_SUB1. */
    AS7058_REGADDR_IOS_PPG1_SUB2 = 0x7B,    /*!< Address of register IOS_PPG1_SUB2. */
    AS7058_REGADDR_IOS_PPG1_SUB3 = 0x7C,    /*!< Address of register IOS_PPG1_SUB3. */
    AS7058_REGADDR_IOS_PPG1_SUB4 = 0x7D,    /*!< Address of register IOS_PPG1_SUB4. */
    AS7058_REGADDR_IOS_PPG1_SUB5 = 0x7E,    /*!< Address of register IOS_PPG1_SUB5. */
    AS7058_REGADDR_IOS_PPG1_SUB6 = 0x7F,    /*!< Address of register IOS_PPG1_SUB6. */
    AS7058_REGADDR_IOS_PPG1_SUB7 = 0x80,    /*!< Address of register IOS_PPG1_SUB7. */
    AS7058_REGADDR_IOS_PPG1_SUB8 = 0x81,    /*!< Address of register IOS_PPG1_SUB8. */
    AS7058_REGADDR_IOS_PPG2_SUB1 = 0x82,    /*!< Address of register IOS_PPG2_SUB1. */
    AS7058_REGADDR_IOS_PPG2_SUB2 = 0x83,    /*!< Address of register IOS_PPG2_SUB2. */
    AS7058_REGADDR_IOS_PPG2_SUB3 = 0x84,    /*!< Address of register IOS_PPG2_SUB3. */
    AS7058_REGADDR_IOS_PPG2_SUB4 = 0x85,    /*!< Address of register IOS_PPG2_SUB4. */
    AS7058_REGADDR_IOS_PPG2_SUB5 = 0x86,    /*!< Address of register IOS_PPG2_SUB5. */
    AS7058_REGADDR_IOS_PPG2_SUB6 = 0x87,    /*!< Address of register IOS_PPG2_SUB6. */
    AS7058_REGADDR_IOS_PPG2_SUB7 = 0x88,    /*!< Address of register IOS_PPG2_SUB7. */
    AS7058_REGADDR_IOS_PPG2_SUB8 = 0x89,    /*!< Address of register IOS_PPG2_SUB8. */
    AS7058_REGADDR_IOS_LEDOFF = 0x8A,       /*!< Address of register IOS_LEDOFF. */
    AS7058_REGADDR_IOS_CFG = 0x8B,          /*!< Address of register IOS_CFG. */
    AS7058_REGADDR_AOC_SAR_THRES = 0x8C,    /*!< Address of register AOC_SAR_THRES. */
    AS7058_REGADDR_AOC_SAR_RANGE = 0x8D,    /*!< Address of register AOC_SAR_RANGE. */
    AS7058_REGADDR_AOC_SAR_PPG1 = 0x8E,     /*!< Address of register AOC_SAR_PPG1. */
    AS7058_REGADDR_AOC_SAR_PPG2 = 0x8F,     /*!< Address of register AOC_SAR_PPG2. */
    AS7058_REGADDR_PP_CFG = 0x90,           /*!< Address of register PP_CFG. */
    AS7058_REGADDR_PPG1_PP1 = 0x91,         /*!< Address of register PPG1_PP1. */
    AS7058_REGADDR_PPG1_PP2 = 0x92,         /*!< Address of register PPG1_PP2. */
    AS7058_REGADDR_PPG2_PP1 = 0x93,         /*!< Address of register PPG2_PP1. */
    AS7058_REGADDR_PPG2_PP2 = 0x94,         /*!< Address of register PPG2_PP2. */
    AS7058_REGADDR_IRQ_ENABLE = 0x95,       /*!< Address of register IRQ_ENABLE. */
    AS7058_REGADDR_PPG_SUB_WAIT = 0x96,     /*!< Address of register PPG_SUB_WAIT. */
    AS7058_REGADDR_PPG_SAR_WAIT = 0x97,     /*!< Address of register AOC_SAR_PPG2. */
    AS7058_REGADDR_PPG_LED_INIT = 0x98,     /*!< Address of register PPG_LED_INIT. */
    AS7058_REGADDR_PPG_FREQL = 0x99,        /*!< Address of register PPG_FREQL. */
    AS7058_REGADDR_PPG_FREQH = 0x9A,        /*!< Address of register PPG_FREQH. */
    AS7058_REGADDR_PPG1_SUB_EN = 0x9B,      /*!< Address of register PPG1_SUB_EN. */
    AS7058_REGADDR_PPG2_SUB_EN = 0x9C,      /*!< Address of register PPG2_SUB_EN. */
    AS7058_REGADDR_PPG_MODE1 = 0x9D,        /*!< Address of register PPG_MODE1. */
    AS7058_REGADDR_PPG_MODE2 = 0x9E,        /*!< Address of register PPG_MODE2. */
    AS7058_REGADDR_PPG_MODE3 = 0x9F,        /*!< Address of register PPG_MODE3. */
    AS7058_REGADDR_PPG_MODE4 = 0xA0,        /*!< Address of register PPG_MODE4. */
    AS7058_REGADDR_PPG_MODE5 = 0xA1,        /*!< Address of register PPG_MODE5. */
    AS7058_REGADDR_PPG_MODE6 = 0xA2,        /*!< Address of register PPG_MODE6. */
    AS7058_REGADDR_PPG_MODE7 = 0xA3,        /*!< Address of register PPG_MODE7. */
    AS7058_REGADDR_PPG_MODE8 = 0xA4,        /*!< Address of register PPG_MODE8. */
    AS7058_REGADDR_PPG_CFG = 0xA5,          /*!< Address of register PPG_CFG. */
    AS7058_REGADDR_ECG_FREQL = 0xA6,        /*!< Address of register ECG_FREQL. */
    AS7058_REGADDR_ECG_FREQH = 0xA7,        /*!< Address of register ECG_FREQH. */
    AS7058_REGADDR_ECG1_FREQDIVL = 0xA8,    /*!< Address of register ECG1_FREQDIVL. */
    AS7058_REGADDR_ECG1_FREQDIVH = 0xA9,    /*!< Address of register ECG1_FREQDIVH. */
    AS7058_REGADDR_ECG2_FREQDIVL = 0xAA,    /*!< Address of register ECG2_FREQDIVL. */
    AS7058_REGADDR_ECG2_FREQDIVH = 0xAB,    /*!< Address of register ECG2_FREQDIVH. */
    AS7058_REGADDR_ECG_SUBS = 0xAC,         /*!< Address of register ECG_SUBS. */
    AS7058_REGADDR_LEADOFF_INITL = 0xAD,    /*!< Address of register LEADOFF_INITL. */
    AS7058_REGADDR_LEADOFF_INITH = 0xAE,    /*!< Address of register LEADOFF_INITH. */
    AS7058_REGADDR_ECG_INITL = 0xAF,        /*!< Address of register ECG_INITL. */
    AS7058_REGADDR_ECG_INITH = 0xB0,        /*!< Address of register ECG_INITH. */
    AS7058_REGADDR_SAMPLE_NUM = 0xB1,       /*!< Address of register SAMPLE_NUM. */
    AS7058_REGADDR_BIOZ_CFG = 0xB2,         /*!< Address of register BIOZ_CFG. */
    AS7058_REGADDR_BIOZ_EXCIT = 0xB3,       /*!< Address of register BIOZ_EXCIT. */
    AS7058_REGADDR_BIOZ_MIXER = 0xB4,       /*!< Address of register BIOZ_MIXER. */
    AS7058_REGADDR_BIOZ_SELECT = 0xB5,      /*!< Address of register BIOZ_SELECT. */
    AS7058_REGADDR_BIOZ_GAIN = 0xB6,        /*!< Address of register BIOZ_GAIN. */
    AS7058_REGADDR_ECGMOD_CFG1 = 0xB7,      /*!< Address of register ECGMOD_CFG1. */
    AS7058_REGADDR_ECGMOD_CFG2 = 0xB8,      /*!< Address of register ECGMOD_CFG2. */
    AS7058_REGADDR_ECGIMUX_CFG1 = 0xB9,     /*!< Address of register ECGIMUX_CFG1. */
    AS7058_REGADDR_ECGIMUX_CFG2 = 0xBA,     /*!< Address of register ECGIMUX_CFG2. */
    AS7058_REGADDR_ECGIMUX_CFG3 = 0xBB,     /*!< Address of register ECGIMUX_CFG3. */
    AS7058_REGADDR_ECGAMP_CFG1 = 0xBC,      /*!< Address of register ECGAMP_CFG1. */
    AS7058_REGADDR_ECGAMP_CFG2 = 0xBD,      /*!< Address of register ECGAMP_CFG2. */
    AS7058_REGADDR_ECGAMP_CFG3 = 0xBE,      /*!< Address of register ECGAMP_CFG3. */
    AS7058_REGADDR_ECGAMP_CFG4 = 0xBF,      /*!< Address of register ECGAMP_CFG4. */
    AS7058_REGADDR_ECGAMP_CFG5 = 0xC0,      /*!< Address of register ECGAMP_CFG5. */
    AS7058_REGADDR_ECGAMP_CFG6 = 0xC1,      /*!< Address of register ECGAMP_CFG6. */
    AS7058_REGADDR_ECGAMP_CFG7 = 0xC2,      /*!< Address of register ECGAMP_CFG7. */
    AS7058_REGADDR_ECG_BIOZ = 0xC3,         /*!< Address of register ECG_BIOZ. */
    AS7058_REGADDR_LEADOFF_CFG = 0xC4,      /*!< Address of register LEADOFF_CFG. */
    AS7058_REGADDR_LEADOFF_THRESL = 0xC5,   /*!< Address of register LEADOFF_THRESL. */
    AS7058_REGADDR_LEADOFF_THRESH = 0xC6,   /*!< Address of register LEADOFF_THRESH. */
    AS7058_REGADDR_IIR_CFG = 0xC7,          /*!< Address of register IIR_CFG. */
    AS7058_REGADDR_IIR_COEFF_ADDR = 0xC8,   /*!< Address of register IIR_COEFF_ADDR. */
    AS7058_REGADDR_IIR_COEFF_DATA = 0xC9,   /*!< Address of register IIR_COEFF_DATA. */
    AS7058_REGADDR_FIFO_THRESHOLD = 0xCA,   /*!< Address of register FIFO_THRESHOLD. */
    AS7058_REGADDR_FIFO_CTRL = 0xCB,        /*!< Address of register FIFO_CTRL. */
    AS7058_REGADDR_PRODUCT_ID = 0xEB,       /*!< Address of register PRODUCT_ID. */
    AS7058_REGADDR_SILICON_ID = 0xEC,       /*!< Address of register SILICON_ID. */
    AS7058_REGADDR_REVISION = 0xED,         /*!< Address of register REVISION. */
    AS7058_REGADDR_GPIO_CTRL = 0xEE,        /*!< Address of register GPIO_CTRL. */
    AS7058_REGADDR_CHIP_CTRL = 0xEF,        /*!< Address of register CHIP_CTRL. */
    AS7058_REGADDR_SEQ_START = 0xF0,        /*!< Address of register SEQ_START. */
    AS7058_REGADDR_STATUS_CGB = 0xF1,       /*!< Address of register STATUS_CGBA. */
    AS7058_REGADDR_STATUS_SEQ = 0xF2,       /*!< Address of register STATUS_SEQ. */
    AS7058_REGADDR_STATUS_LED = 0xF3,       /*!< Address of register STATUS_LED. */
    AS7058_REGADDR_STATUS_ASATA = 0xF4,     /*!< Address of register STATUS_ASATA. */
    AS7058_REGADDR_STATUS_ASATB = 0xF5,     /*!< Address of register STATUS_ASATB. */
    AS7058_REGADDR_STATUS_VCSEL = 0xF6,     /*!< Address of register STATUS_VCSEL. */
    AS7058_REGADDR_STATUS_VCSEL_VSS = 0xF7, /*!< Address of register STATUS_VCSEL_VSS. */
    AS7058_REGADDR_STATUS_VCSEL_VDD = 0xF8, /*!< Address of register STATUS_VCSEL_VDD. */
    AS7058_REGADDR_STATUS_LEADOFF = 0xF9,   /*!< Address of register STATUS_LEADOFF. */
    AS7058_REGADDR_STATUS = 0xFA,           /*!< Address of register STATUS. */
    AS7058_REGADDR_FIFO_LEVEL0 = 0xFB,      /*!< Address of register FIFO_LEVEL0. */
    AS7058_REGADDR_FIFO_LEVEL1 = 0xFC,      /*!< Address of register FIFO_LEVEL1. */
    AS7058_REGADDR_FIFOL = 0xFD,            /*!< Address of register FIFOL. */
    AS7058_REGADDR_FIFOM = 0xFE,            /*!< Address of register FIFOM. */
    AS7058_REGADDR_FIFOH = 0xFF,            /*!< Address of register FIFOH. */
};

enum as7058_fifo_data_marker {
    MARKER_PPG1_FIRST = 0,    /* Marker for first sub-sample of modulator 1 */
    MARKER_PPG1_OTHER = 1,    /* Marker for other sub-samples of modulator 1 */
    MARKER_PPG2_FIRST = 2,    /* Marker for first sub-sample of modulator 2 */
    MARKER_PPG2_OTHER = 3,    /* Marker for other sub-samples of modulator 2 */
    MARKER_ECG_SEQ1_SUB1 = 4, /* Marker for sub-sample 1 of modulator 3 and sequence 1 (ECG) */
    MARKER_ECG_SEQ1_SUB2 = 5, /* Marker for sub-sample 2 of modulator 3 and sequence 1 (ECG) */
    MARKER_ECG_SEQ2 = 6, /* Marker for sub-sample 1 of modulator 3 and sequence 2. */
    MARKER_STATUS = 7,        /* Marker for information. */
};

#define FIFO_BUFFER_MAX_SIZE 512  /* Maximum number of samples that can be stored in FIFO */

/*! Type of ::as7058_reg_addresses. */
typedef uint8_t as7058_reg_addresses_t;

typedef struct {
    uint32_t raw;       /* 24-bit raw sample data, right-aligned in 32 bits */
    uint32_t adc;       /* 20-bit ADC value, sign-extended to 32 bits  */
    uint8_t  marker;    /* 3-bit data marker                           */
} as7058_sample_t;

struct as7058_fifo_t{
    uint8_t fifol;
    uint8_t fifom;
    uint8_t fifoh;
};

#endif /* __AS7058_TYPEDEFS_H__ */