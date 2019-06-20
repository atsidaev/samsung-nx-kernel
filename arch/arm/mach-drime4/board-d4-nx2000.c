/* linux/arch/arm/mach-drime4/board-d4-nx2000.c
 *
 * Copyright (c) 2011 Samsung Electronics Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/platform_device.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/spi/spi.h>
#include <linux/lcd.h>
#include <linux/io.h>
#include <linux/i2c.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/nand_ecc.h>
#include <linux/mtd/partitions.h>
#include <linux/pinctrl/machine.h>
#include <linux/pinctrl/consumer.h>
#include <linux/jack.h>
#include <linux/dma-contiguous.h>
#include <linux/drime_adc_battery.h>
#include <linux/input.h>
#include <linux/d4_rmu.h>
#include <asm/hardware/vic.h>
#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <mach/common.h>
#include <mach/i2c.h>
#include <mach/nand.h>
#include <mach/map.h>
#include <mach/devs.h>
#include <mach/gpio.h>
#include <mach/d4_cma.h>
#include <mach/dp/d4_dp.h>
#include <drm/drime4_drm.h>
#include <mach/version_information.h>
#include <mach/d4keys.h>
#include <mach/adc.h>
#include <mach/cpuidle.h>

#ifdef CONFIG_DRIME4_PP_SSIF
#include <mach/pp/pp_ssif/d4_pp_ssif.h>
#endif

#ifdef CONFIG_DRIME4_LCD_PANEL_MANAGER
#include <mach/lcd_panel_manager/d4_lcd_panel_manager.h>
#endif

#ifdef CONFIG_PM
#include <mach/pm.h>
#endif

#ifdef CONFIG_HT_PWM_UMODE
#include <linux/pwmdev_conf.h>
#endif

#include <linux/musb_sw.h>
#include <linux/tsu6712.h>
#include <linux/tsu6111a.h>
#include <linux/bd9361.h>


#ifdef CONFIG_TOUCHSCREEN_MELFAS
#include <linux/melfas_ts.h>
#endif

#ifdef CONFIG_LEDS_GPIO
#include <linux/leds.h>
#endif

#ifdef CONFIG_SND_SOC_D4_WM8994
#include <linux/mfd/wm8994/pdata.h>
#endif

#if defined (CONFIG_RTC_DRV_R2051K02)
#include <linux/r2051k02.h>
#endif

/* DA8XX devices support DDR2 power down */
static struct drime4_cpuidle_config drime4_cpuidle_pdata = {
	.ddr2_pdown	= 1,
};


static struct platform_device drime4_cpuidle_device = {
	.name			= "cpuidle-drime4",
	.id				= -1,
	.dev = {
		.platform_data	= &drime4_cpuidle_pdata,
	},
};

/*********** INTERRUPT KEY  ************/
static struct d4_keys_button drime4_gpio_key_buttons[] = {
	/* Gpio Key */
	{
		.keytype		= PORT1_TYPE,
		.code		= KEY_POWER,
		.code2nd		= KEY_VOLUMEUP,
		.gpio		= (unsigned)-2,
		.desc		= "power_key",
		.debounce_interval = 10,
		.type		= EV_KEY,
		.active_low = 1,
		.mask_id = MASK_POWER,
	},{
		.keytype		= PORT1_TYPE,
		.code		= KEY_LEFTMETA,
		.gpio		= (unsigned)-2, // GPIO_SHUTTER_KEY1_DSP,
		.desc		= "shutter1_key",
		.debounce_interval = 10,
		.type		= EV_KEY,
		.active_low = 1,
		.mask_id = MASK_SH1,
	},{
		.keytype		= PORT1_TYPE,
		.code		= KEY_RIGHTMETA,
		.gpio		= (unsigned)-2, // GPIO_SHUTTER_KEY2,
		.desc		= "shutter2_key",
		.debounce_interval = 10,
		.type		= EV_KEY,
		.active_low = 1,
		.mask_id = MASK_SH2,
	},{
		.keytype		= PORT2_TYPE,
		.code		= KEY_SCROLLUP,
		.code2nd		= KEY_SCROLLDOWN,
		.gpio		= (unsigned)-2, // GPIO_JOG_L,/*interrupt*/
		.gpio2nd		= (unsigned)-2, // GPIO_JOG_R,/*no interrupt*/
		.desc		= "jog_ccw",
		.debounce_interval = 5,
		.type		= EV_KEY,
		.active_low = 0,
		.mask_id = MASK_JOG,
	},{
		.keytype		= PORT1_TYPE,
		.code		= KEY_HANGEUL,
		.gpio		= (unsigned)-2, // GPIO_SH_KEY1_MUIC
		.desc		= "sh1_muic",
		.debounce_interval = 10,
		.type		= EV_KEY,
		.active_low = 1,
		.mask_id = MASK_SH1,
	},{
		.keytype		= PORT1_TYPE,
		.code		= KEY_HANJA,
		.gpio		= (unsigned)-2, // GPIO_SH_KEY2_MUIC
		.desc		= "sh1_muic",
		.debounce_interval = 10,
		.type		= EV_KEY,
		.active_low = 1,
		.mask_id = MASK_SH2,
	},{
		.keytype		= PORT1_TYPE,
		.code		= KEY_EMAIL,
		.gpio		= (unsigned)-2, // WIFI_PWR_CNT
		.desc		= "wifi_key",
		.debounce_interval = 10,
		.type		= EV_KEY,
		.active_low = 1,
		.mask_id = MASK_WIFI,
	},{
		.keytype		= PORT1_TYPE,
		.code		= KEY_KPLEFTPAREN,
		.gpio		= (unsigned)-2, // GPIO_JOG_PUSH,
		.desc		= "jog_push_key",
		.debounce_interval = 10,
		.type		= EV_KEY,
		.active_low = 1,
		.mask_id = MASK_JOGPUSH,
	},
};

static struct d4_keys_platform_data drime4_gpio_keys_data = {
	.buttons	= drime4_gpio_key_buttons,
	.nbuttons	= ARRAY_SIZE(drime4_gpio_key_buttons),
	.rep		= 0,
};

struct platform_device drime4_device_key = {
	.name			= "d4-keys",
	.id				= -1,
	.dev = {
		.platform_data = &drime4_gpio_keys_data,
	},
};

static void d4keys_gpio_init(void){
	drime4_gpio_key_buttons[0].gpio = GPIO_POWER_ON;
	drime4_gpio_key_buttons[1].gpio = GPIO_SHUTTER_KEY1_DSP;
	drime4_gpio_key_buttons[2].gpio = GPIO_SHUTTER_KEY2;
	drime4_gpio_key_buttons[3].gpio = GPIO_JOG_L;
	drime4_gpio_key_buttons[3].gpio2nd = GPIO_JOG_R;
	drime4_gpio_key_buttons[4].gpio = GPIO_SH_KEY1_MUIC;
	drime4_gpio_key_buttons[5].gpio = GPIO_SH_KEY2_MUIC;
	drime4_gpio_key_buttons[6].gpio = WIFI_PWR_CNT;
	drime4_gpio_key_buttons[7].gpio = GPIO_JOG_PUSH;
}

/*********** POLLED KEY  ************/
static struct d4_keys_button_polled drime4_key_buttons_polled[] = {
	{	/* ADC Key 1 */
		.keytype		= ADCKEY_TYPE,
		.ref_mvolt	= 10,
		.time_margin	= 2500,
		.data_diff		= 55,
		.adc_useint	= ADC_INT_OFF,
		.adc_ch		= 1,
		.adc_input	= true,
	},
};

static struct d4_keys_platform_data_polled drime4_keys_data_polled = {
	.buttons	= drime4_key_buttons_polled,
	.nbuttons	= ARRAY_SIZE(drime4_key_buttons_polled),
	.poll_interval = 33,
	.rep		= 0,
};

struct platform_device drime4_device_key_polled = {
	.name			= "d4keys-polled",
	.id				= -1,
	.dev = {
		.platform_data = &drime4_keys_data_polled,
	},
};

static struct adc_info_key adc1_key[3]={
	{.code = KEY_CONFIG, 	.adc_level = 103, 	.mask_id = MASK_PB, }, 		//pb
	{.code = KEY_SEARCH, .adc_level = 56, 	.mask_id = MASK_MODE, }, 		//func
	{.code = KEY_CAMERA, .adc_level = 0, 	.mask_id = MASK_REC,}, 		//rec
};

static void d4keys_polled_init(void)
{
	drime4_key_buttons_polled[0].key_info = adc1_key;
	drime4_key_buttons_polled[0].key_cnt = ARRAY_SIZE(adc1_key);
}

/*********** LED  ************/
static int drime4_gpio_blink_set(unsigned gpio, int state,
		unsigned long *delay_on, unsigned long *delay_off)
{
	/*
		if (delay_on && delay_off && !*delay_on && !*delay_off)
			*delay_on = *delay_off = ORION_BLINK_HALF_PERIOD;
	*/
	switch (state) {
	case GPIO_LED_NO_BLINK_LOW:
	case GPIO_LED_NO_BLINK_HIGH:
		gpio_set_value(gpio, state);
		break;
	case GPIO_LED_BLINK:
		gpio_set_value(gpio, state);
		break;
	}
	return 0;
}

static struct gpio_led drime4_leds[] = {
	{
		.name = "af",
		.gpio = (unsigned)-1, // GPIO_AF_LED,
		.active_low = 0,
	}, {
		.name = "card",
		.gpio = (unsigned)-1, // GPIO_CARD_LED,
		.active_low = 1,
	},
};

static struct gpio_led_platform_data drime4_led_data = {
	.num_leds = ARRAY_SIZE(drime4_leds),
	.leds = drime4_leds,
	.gpio_blink_set = drime4_gpio_blink_set,
};

static struct platform_device drime4_gpio_leds = {
	.name = "leds-gpio",
	.id = -1,
	.dev = {
		.platform_data = &drime4_led_data,
	},
};

static void d4_led_init_gpio(void)
{
	drime4_leds[0].gpio = GPIO_AF_LED;
	drime4_leds[1].gpio = GPIO_CARD_LED;
}

/*********** BATTERY  ************/
static struct drime_adc_bat_pdata drime4_batchk_pdata = {
	.init = 0,
	.exit = 0,
	.enable_charger = 0,
	.disable_charger = 0,
	.gpio_charge_finished = 0,
	.lut_noac = 0,
	.lut_noac_cnt = 0,
	.lut_acin = 0,
	.lut_acin_cnt = 0,
	.volt_channel = 5,
	.current_channel = 0,
	.volt_mult = 10,
	.current_mult = 0,
	.internal_impedance = 0,
};

static struct platform_device drime4_batchk_pdev = {
	.name = "d4-adc-battery",
	.id = -1,
	.dev = {
		.platform_data = &drime4_batchk_pdata,
	},
};

/*********** MUIC  ************/
#if defined(CONFIG_TSU6111A)
static int tsu6111a_attach_callback(int dev_type)
{
	int ret;
	
	switch (dev_type) {
	case MUSB_DEV_USB:
#ifndef CONFIG_USBSTARTSTOP_SYSFS
		pinctrl_request_gpio(GPIO_USB_DET);
		ret = gpio_request(GPIO_USB_DET, "USB Detect");
		if (ret != 0)
		return -1;

		gpio_direction_output(GPIO_USB_DET, 1);

		gpio_free(GPIO_USB_DET);
		pinctrl_free_gpio(GPIO_USB_DET);
		jack_event_handler("usb", 1);
#else
		printk(KERN_EMERG"usb 1\n");
		jack_event_handler("usb", 1);
#endif	/* HSW TEST 20120920 M */
		break;
	case MUSB_DEV_ETC:
		printk(KERN_EMERG"release 1\n");
//		jack_event_handler("release", 1);
		break;
	default:
		printk("not surport\n");
		break;
	}

	return 0;
}

static int tsu6111a_detach_callback(int dev_type)
{
	int ret;
	switch (dev_type) {
	case MUSB_DEV_USB:
#ifndef CONFIG_USBSTARTSTOP_SYSFS
		pinctrl_request_gpio(GPIO_USB_DET);
		ret = gpio_request(GPIO_USB_DET, "USB Detect");
		if (ret != 0)
		return -1;

		gpio_direction_output(GPIO_USB_DET, 0);

		gpio_free(GPIO_USB_DET);
		pinctrl_free_gpio(GPIO_USB_DET);
		jack_event_handler("usb", 0);
#else
		printk(KERN_EMERG"usb 0\n");
		jack_event_handler("usb", 0);
#endif	/* HSW TEST 20120920 M */
		break;
	case MUSB_DEV_ETC:
		printk(KERN_EMERG"release 0\n");
//		jack_event_handler("release", 0);
		break;

	default:
		printk("not surport\n");
		break;
	}
	return 0;
}

static struct tsu6111a_platform_data tsu6111a_data = {
	.pinmux_name = "I2C2",
	.sda_gpio = GPIO_USB_SDA,
	.scl_gpio = GPIO_USB_SCL,
	.intb = (unsigned)-1, // GPIO_JACK_INT,
	.attach_callback = tsu6111a_attach_callback,
	.detach_callback = tsu6111a_detach_callback,
};
#endif

/************ PMIC**************/
#ifdef CONFIG_BD9361
static struct bd9361_platform_data bd9361_data = {
	.pinmux_name = "I2C2",
	.sda_gpio = GPIO_USB_SDA,
	.scl_gpio = GPIO_USB_SCL,
};
#endif

/*********** TOUCH  ************/
#ifdef CONFIG_TOUCHSCREEN_MELFAS
static int mel_ts_int_set(int pin_num)
{
	int ret;
	pinctrl_request_gpio(pin_num);
	gpio_request(pin_num, "GPIO_TSP_INT");
	gpio_direction_input(pin_num);
	ret = gpio_to_irq(pin_num);
	return ret;
}

static void mel_ts_power(int on)
{

	pinctrl_request_gpio(on);
	gpio_request(on, "GPIO_TSP_3_3V_ON");

}

static void mel_ts_power_en(int en, int pin_num)
{

	gpio_direction_output(pin_num, en);

}

static struct melfas_tsi_platform_data ts_config = {
	.max_x = 640,
	.max_y = 480,
	.max_area = 30,
	.max_pressure = 255,
	.power = mel_ts_power,
	.power_enable = mel_ts_power_en,
	.int_enable = mel_ts_int_set,
	.pin_num = -1, // GPIO_TSP_INT,
	.pwr_num = -1, // GPIO_TSP_3_3V_ON

};
#endif

#ifdef CONFIG_SND_SOC_D4_WM8994
static void wm8994_power_enable(void)
{
	int pin_num = GPIO_AUDIO_EN;

	pinctrl_request_gpio(pin_num);
	gpio_request(pin_num, "");
	gpio_direction_output(pin_num, 1);
}

static struct wm8994_retune_mobile_cfg wm8994_retune_eq[] = {
	{
		.name = "NX2000 EQ2050 XL1206",
		.rate = 48000,
		.regs = {
				0x4004, 0x2b00, 0x0f2f, 0x0400, 0x0344, 
				0x1c4f, 0xf103,	0x040b, 0x0402, 0x1523,
				0xf069, 0x040a, 0x01a0, 0x09c4,	0xf066,
				0x0407, 0x0194, 0x0000, 0x0400, 0x4000
		},
	},
	{
		.name = "NX2000 EQ2050 XL1015",
		.rate = 48000,
		.regs = {
				0x4804, 0x2A80, 0x0F2F, 0x0400, 0x0344, 
				0x1EB3, 0xF09E,	0x0408, 0x0272, 0x1CDB,
				0xF06A, 0x0409, 0x01A4, 0x198F,	0xF068,
				0x040B, 0x019C, 0x06A1, 0x05A8, 0x4000
		},
	},
	{
		.name = "NX2000 EQ1855",
		.rate = 48000,
		.regs = {
			    0x4252, 0x32c0, 0x0F2F, 0x0400, 0x0344,
			    0x1F92, 0xF035, 0x040e, 0x00D2, 0x1EB7,
			    0xF06A, 0x0407, 0x01A4, 0x1Dd9, 0xF09e,
			    0x0409, 0x0272, 0x021b, 0x0487, 0x4000
		},
	},
	{
		.name = "NX2000 EQ for external mic",
		.rate = 48000,
		.regs = {
				0x5210, 0x4B00, 0x0F2F, 0x0400, 0x0344,
				0x1F92, 0xF035, 0x040D, 0x00D2, 0x1EB7,
				0xF06A, 0x0407, 0x01A4, 0x1DA2,	0xF06A,
				0x0406, 0x01A4, 0x07E4, 0x05F9, 0x4000
		},
	},
	{
		.name = "NX2000 general EQ",
		.rate = 48000,
		.regs = {
				0x6318, 0x6300, 0x0FC8, 0x03FE, 0x00E0,
				0x1EC4, 0xF136, 0x0409, 0x04CC, 0x1C9B,
				0xF337, 0x040B, 0x0CBB, 0x16F8,	0xF7D9,
				0x040A, 0x1F14, 0x058C, 0x0563, 0x4000
		},
	},
	{
		.name = "NX2000 EQ30",
		.rate = 48000,
		.regs = {
				0x1999, 0x18c0, 0x0f62, 0x03ff, 0x0278,
				0x1fee, 0xf011, 0x03e6, 0x0044, 0x1fe3,
				0xF01b, 0x03eb, 0x006d, 0xe857, 0xf3b7,
				0x040b, 0x0eb6, 0xf2de, 0x0401, 0x0b78
		},
	},
	{
		.name = "NX2000 EQ60",
		.rate = 48000,
		.regs = {
				0x39cd, 0x30c0, 0x0f7c, 0x03ff, 0x0210,
				0x1f0b, 0xf0ea, 0x040c, 0x039f, 0x0cee,
				0xF1f8, 0x040a, 0x07cc, 0xee34, 0xf3b9,
				0x040a, 0x0ebe, 0xf2de, 0x0401, 0x0b78
		},
	},
	{
		.name = "NX2000 EQ1855 for external mic",
		.rate = 32000,
		.regs = {
				0x4252, 0x32c0, 0x0ECA, 0x03FF, 0x04D8,
				0x1F32, 0xF050, 0x0406, 0x013D, 0x1D72,
				0xF09E, 0x0409, 0x0272, 0x1BB2, 0xF0EA,
				0x040C, 0x039F, 0xFE6D, 0x0400, 0x39B4
		},
    },

};


static struct wm8994_drc_cfg wm8994_retune_drc[] = {
	{
		.name = "NX2000 DRC params for internal mic: lens 20-50",
		.regs = {
                0x0710, 0x0453, 0x0818, 0x037c, 0x0160
		},
	},
	{
		.name = "NX2000 DRC2050 XL1015",  /* slope 4 under -52.5 dB */
		.regs = {
				0x0710, 0x0e53, 0x0808, 0x0232, 0x0160
		},
	},
	{
		.name = "NX2000 DRC params for internal mic: lens 18-55",
		.regs = {
                 0x0710, 0x0453, 0x0818, 0x037c, 0x0180
		},
	},
	{
		.name = "NX2000 DRC params for external mic: lens 20-50",
		.regs = {
                0x0710, 0x0453, 0x0818, 0x035b, 0x0120
		},
	},
	{
		.name = "NX2000 general DRC for internal mic",
		.regs = {
				0x0710, 0x0e53, 0x0808, 0x0232, 0x0180
		},
	},
	{
		.name = "NX2000 DRC params for external mic: lens 18-55",
		.regs = {
                0x710, 0x453, 0x818, 0x035b, 0x120
		},
	},
}; 

static struct wm8994_pdata wm8994_platform_data = {
	.power_enable	= wm8994_power_enable,

	.retune_mobile_cfgs = wm8994_retune_eq,
	.num_retune_mobile_cfgs = ARRAY_SIZE(wm8994_retune_eq), 
	.drc_cfgs = wm8994_retune_drc,
	.num_drc_cfgs = ARRAY_SIZE(wm8994_retune_drc),
	.gpio_defaults[0] = 0x8101,
	.gpio_defaults[2] = 0x0100,
	.gpio_defaults[3] = 0x0100,
	.gpio_defaults[4] = 0x8100,
	.gpio_defaults[6] = 0x0100,
	.gpio_defaults[7] = 0x8100,
	.gpio_defaults[8] = 0x0100,
};

#endif

/*********** LCD PANEL ************/
#ifdef CONFIG_DRIME4_LCD_PANEL_MANAGER

	#ifdef CONFIG_MLCD_1_PANEL_1
	struct d4_hs_spi_config mlcd_1_config = {
		.speed_hz = 5000000,
		.bpw = 16,
		.mode = SH_SPI_MODE_3,
		.waittime = 100,
		.ss_inven = D4_SPI_TRAN_INVERT_OFF,
		.spi_ttype = D4_SPI_TRAN_BURST_OFF,
		.setup_select = SH_SPI_SPEED|SH_SPI_BPW|SH_SPI_WAVEMODE|SH_SPI_WAITTIME|SH_SPI_INVERT|SH_SPI_BURST
	};

	#elif CONFIG_MLCD_1_PANEL_2
	struct d4_hs_spi_config mlcd_1_config = {
		.speed_hz = 781300,
		.bpw = 16,
		.mode = SH_SPI_MODE_3,
		.waittime = 100,
		.ss_inven = D4_SPI_TRAN_INVERT_OFF,
		.spi_ttype = D4_SPI_TRAN_BURST_OFF,
		.setup_select = SH_SPI_SPEED|SH_SPI_BPW|SH_SPI_WAVEMODE|SH_SPI_WAITTIME|SH_SPI_INVERT|SH_SPI_BURST
	};

	#elif CONFIG_MLCD_1_PANEL_5
	struct d4_hs_spi_config mlcd_1_config = {
		.speed_hz = 1000000,
		.bpw = 16,
		.mode = SH_SPI_MODE_3,
		.waittime = 100,
		.ss_inven = D4_SPI_TRAN_INVERT_OFF,
		.spi_ttype = D4_SPI_TRAN_BURST_OFF,
		.setup_select = SH_SPI_SPEED|SH_SPI_BPW|SH_SPI_WAVEMODE|SH_SPI_WAITTIME|SH_SPI_INVERT|SH_SPI_BURST
	};

	#elif CONFIG_MLCD_1_PANEL_6
	struct d4_hs_spi_config mlcd_1_config = {
		.speed_hz = 1000000,
		.bpw = 8,
		.mode = SH_SPI_MODE_1,
		.waittime = 100,
		.ss_inven = D4_SPI_TRAN_INVERT_OFF,
		.spi_ttype = D4_SPI_TRAN_BURST_ON,
		.setup_select = SH_SPI_SPEED|SH_SPI_BPW|SH_SPI_WAVEMODE|SH_SPI_WAITTIME|SH_SPI_INVERT|SH_SPI_BURST
	};

	#else
	struct d4_hs_spi_config mlcd_1_config = {
		.speed_hz = 781300,
		.bpw = 9,
		.mode = SH_SPI_MODE_3,
		.waittime = 100,
		.ss_inven = D4_SPI_TRAN_INVERT_OFF,
		.spi_ttype = D4_SPI_TRAN_BURST_ON,
		.setup_select = SH_SPI_SPEED|SH_SPI_BPW|SH_SPI_WAVEMODE|SH_SPI_WAITTIME|SH_SPI_INVERT|SH_SPI_BURST
	};

	#endif

#ifdef CONFIG_MLCD_1_PANEL_5
	struct d4_lcd_hw_connect_info mlcd_1_hw_info = {
		.gpio_reset   	= DRIME4_GPIO24(0),
		.gpio_power_1 	= DRIME4_GPIO1(1),
		.gpio_power_2 	= LPM_HW_CONNECT_NO_USE,
		.gpio_power_3 	= DRIME4_GPIO0(2),
		.spi_ch			= 0,
		.spi_config		= &mlcd_1_config,
	};

#elif CONFIG_MLCD_1_PANEL_6
	struct d4_lcd_hw_connect_info mlcd_1_hw_info = {
		.gpio_reset   	= GPIO_LCD_nRESET,
		.gpio_power_1 	= GPIO_DISP_3_3V_ON,
		.gpio_power_2 	= LPM_HW_CONNECT_NO_USE,
		.gpio_power_3 	= LPM_HW_CONNECT_NO_USE,
		.spi_ch			= 0,
		.spi_config		= &mlcd_1_config,
	};

#else
	struct d4_lcd_hw_connect_info mlcd_1_hw_info = {
		.gpio_reset   	= GPIO_LCD_nRESET,
		.gpio_power_1 	= GPIO_DISP_3_3V_ON,
		.gpio_power_2 	= LPM_HW_CONNECT_NO_USE,
		.gpio_power_3 	= LPM_HW_CONNECT_NO_USE,
		.spi_ch			= 0,
		.spi_config		= &mlcd_1_config,
	};

#endif

	struct d4_lcd_panel_manager_data lcd_hw_conect_info = {
		.mlcd_1_hw_connect_info = &mlcd_1_hw_info,
		.mlcd_2_hw_connect_info = NULL,
		.slcd_hw_connect_info 	= NULL,
	};

	struct platform_device drime4_device_lcd_panel_manager = {
		.name		    = LCD_PANEL_MANAGER_MODULE_NAME,
		.id		        = -1,
		.num_resources  = 0,
		.dev		    = {
				.platform_data = &lcd_hw_conect_info,
		},
	};
#endif

#ifdef CONFIG_DRM_DRIME4_DP_LCD
static struct drime4_drm_dp_lcd_pdata drm_dp_lcd_pdata = {
	.panel = {
		.timing	= {
			.xres		= 640,
			.yres		= 480,
			.hsync_len	= 60,
			.left_margin	= 150,
			.right_margin	= 7,
			.vsync_len	= 10,
			.upper_margin	= 38,
			.lower_margin	= 6,
			.refresh	= 60,
			.pixclock       = 27000000
		},
		.width_mm = 640,
		.height_mm = 480,
	},
	.pannel_buf_h_start = 142,
	.pannel_inv_dot_clk = 1,
	.pannel_inv_clk = 0,
	.pannel_inv_h_sync = 1,
	.pannel_inv_v_sync = 1,
	.data_width = D4_DATA_24,
	.panel_type = D4_STRIPE,
	.even_seq = D4_BGR,
	.odd_seq = D4_BGR,
	.default_vid_win = 0,
	.default_grp_win = 4,
};

static struct resource drime4_dp_resource[] = { [0] = {
	.start = DRIME4_PA_DP,
	.end = DRIME4_PA_DP + 0x1500 - 1,
	.flags = IORESOURCE_MEM,
},
[1] = {
	.start = IRQ_DP_CORE,
	.end = IRQ_DP_CORE,
	.flags = IORESOURCE_IRQ,
},
[2] = {
	.start = IRQ_DP_DMA,
	.end = IRQ_DP_DMA,
	.flags = IORESOURCE_IRQ,
}, };

static u64 dp_dma_mask = 0xffffffffUL;

	struct platform_device drime4_device_lcd = {
		.name		    = LCD_MODULE_NAME,
		.id		        = -1,
		.num_resources  = ARRAY_SIZE(drime4_dp_resource),
		.resource		= drime4_dp_resource,
		.dev = {
				.platform_data = &drm_dp_lcd_pdata,
				.dma_mask = &dp_dma_mask,
				.coherent_dma_mask = 0xffffffffUL,
		},
	};
#endif

#ifdef CONFIG_DRM_DRIME4_DP_SUBLCD

static struct drime4_drm_dp_lcd_pdata drm_dp_sublcd_pdata = {
	.panel = {
		.timing	= {
			.xres		= 660/3,
			.yres		= 175,
			.hsync_len	= 21/3,
			.left_margin	= 84/3,
			.right_margin	= 410/3,
			.vsync_len	= 6,
			.upper_margin	= 15,
			.lower_margin	= 0,
			.refresh	= 60,
			.pixclock       = 13500000,
		},
	},
	.pannel_buf_h_start = 59,
	.pannel_inv_dot_clk = 1,
	.pannel_inv_clk = 0,
	.pannel_inv_h_sync = 1,
	.pannel_inv_v_sync = 1,
	.data_width = D4_DATA_8,
	.panel_type = D4_DELTA,
	.even_seq = D4_RGB,
	.odd_seq = D4_RGB,
	.default_vid_win = 0,
	.default_grp_win = 4,
};

static struct resource drime4_dp_sublcd_resource[] = { [0] = {
	.start = DRIME4_PA_DP+0xa00,
	.end = DRIME4_PA_DP + SZ_64K - 1,
	.flags = IORESOURCE_MEM,
},
[1] = {
	.start = IRQ_DP_CORE,
	.end = IRQ_DP_CORE,
	.flags = IORESOURCE_IRQ,
},
[2] = {
	.start = IRQ_DP_DMA,
	.end = IRQ_DP_DMA,
	.flags = IORESOURCE_IRQ,
}, };

static u64 dp_sublcd_dma_mask = 0xffffffffUL;


struct platform_device drime4_device_sublcd = {
	.name		    = SUBLCD_MODULE_NAME,
	.id		        = -1,
	.num_resources  = ARRAY_SIZE(drime4_dp_sublcd_resource),
	.resource		= drime4_dp_sublcd_resource,
	.dev = {
			.platform_data = &drm_dp_sublcd_pdata,
			.dma_mask = &dp_sublcd_dma_mask,
			.coherent_dma_mask = 0xffffffffUL,
	},
};

#endif

#ifdef CONFIG_DRM_DRIME4_DP_TV
static struct drime4_drm_common_hdmi_pd drm_common_hdmi_pd = {
	.hdmi_dev = &drime4_device_hdmi.dev,
	.tv_dev = &drime4_device_tv.dev,
    .cec_dev = &drime4_device_cec.dev,
};
struct platform_device drime4_drm_hdmi_device = {
	.name = "drime4-drm-hdmi",
	.dev = {
		.platform_data = &drm_common_hdmi_pd,
	},
};
#endif

// I2C (CH1 ~ CH6) ----------------------------------------------------------------------------------------------------------------------------------------------------
static struct i2c_board_info i2c_devs0[] __initdata = {
#if defined (CONFIG_RTC_DRV_R2051K02)
 	{
		I2C_BOARD_INFO(R2051K02_I2C_DEV_NAME, R2051K02_SLAVE_ADDR),
	},
#endif
};

static struct i2c_board_info i2c_devs1[] __initdata = { 
	{
		I2C_BOARD_INFO("d4_ddc", 0x50), /* a0 >>1 */
	},
};

static struct i2c_board_info i2c_devs2[] __initdata = {
#ifdef CONFIG_TSU6111A
 	{
		I2C_BOARD_INFO(TSU6111A_I2C_DEV_NAME, TSU6111A_SLAVE_ADDR),
		.platform_data = &tsu6111a_data,
	},
#endif
};

static struct i2c_board_info i2c_devs3[] __initdata = {
#ifdef CONFIG_SND_SOC_D4_WM8994
	{
		I2C_BOARD_INFO("wm8994", 0x1a),
		.platform_data = &wm8994_platform_data,
	},
#endif
};

static struct i2c_board_info i2c_devs4[] __initdata = {
#ifdef CONFIG_TOUCHSCREEN_MELFAS
	{
		I2C_BOARD_INFO(MELFAS_TS_NAME, 0x48),
		.platform_data = &ts_config,
	},
#endif
};

static struct i2c_board_info i2c_devs6[] __initdata = { 
	{
		I2C_BOARD_INFO("hdmi-phy", 0x38), /* 0x70>>1 */
	}, 
};

// PWM (CH0 ~ CH17) ----------------------------------------------------------------------------------------------------------------------------------------------------

static struct platform_pwm_dev_data pwmdev_data0 = {
	.pwm_sch = PWMDEV_AUTO,
	.pin_num = DRIME4_GPIO_END,
};

static struct platform_device pwmdev_device0 = {
	.name = "pwmdev",
	.id = 0,
	.dev = {
		.platform_data = &pwmdev_data0,
	},
};

static struct platform_pwm_dev_data pwmdev_data1 = {
	.pwm_sch = PWMDEV_AUTO,
	.pin_num = DRIME4_GPIO_END,
};

static struct platform_device pwmdev_device1 = {
	.name = "pwmdev",
	.id = 1,
	.dev = {
		.platform_data = &pwmdev_data1,
	},
};


static struct platform_pwm_dev_data pwmdev_data2 = {
	.pwm_sch = PWMDEV_AUTO,
	.pin_num = DRIME4_GPIO_END,
};

static struct platform_device pwmdev_device2 = {
	.name = "pwmdev",
	.id = 2,
	.dev = {
		.platform_data = &pwmdev_data2,
	},
};


static struct platform_pwm_dev_data pwmdev_data3 = {

		.pwm_sch = PWMDEV_AUTO,
		.pin_num = DRIME4_GPIO_END,
};

static struct platform_device pwmdev_device3 = {
	.name = "pwmdev",
	.id = 3,
	.dev = {
		.platform_data = &pwmdev_data3,
	},
};

static struct platform_pwm_dev_data pwmdev_data4 = {
		.pwm_sch = PWMDEV_NULL,
		.pin_num = DRIME4_GPIO_END,
};

static struct platform_device pwmdev_device4 = {
	.name = "pwmdev",
	.id = 4,
	.dev = {
		.platform_data = &pwmdev_data4,
	},
};



static struct platform_pwm_dev_data pwmdev_data5 = {
		.pwm_sch = PWMDEV_NULL,
		.pin_num = DRIME4_GPIO_END,
};

static struct platform_device pwmdev_device5 = {
	.name = "pwmdev",
	.id = 5,
	.dev = {
		.platform_data = &pwmdev_data5,
	},
};


static struct platform_pwm_dev_data pwmdev_data6 = {
		.pwm_sch = PWMDEV_NULL,
		.pin_num = DRIME4_GPIO_END,
};

static struct platform_device pwmdev_device6 = {
	.name = "pwmdev",
	.id = 6,
	.dev = {
		.platform_data = &pwmdev_data6,
	},
};


static struct platform_pwm_dev_data pwmdev_data7 = {
	.pwm_sch = PWMDEV_NULL,
	.pin_num = DRIME4_GPIO_END,
};

static struct platform_device pwmdev_device7 = {
	.name = "pwmdev",
	.id = 7,
	.dev = {
		.platform_data = &pwmdev_data7,
	},
};

static struct platform_pwm_dev_data pwmdev_data8 = {
	.pwm_sch = PWMDEV_NULL,
	.pin_num = DRIME4_GPIO_END,
};

static struct platform_device pwmdev_device8 = {
	.name = "pwmdev",
	.id = 8,
	.dev = {
		.platform_data = &pwmdev_data8,
	},
};

static struct platform_pwm_dev_data pwmdev_data9 = {
	.pwm_sch = PWMDEV_NULL,
	.pin_num = DRIME4_GPIO_END,
};

static struct platform_device pwmdev_device9 = {
	.name = "pwmdev",
	.id = 9,
	.dev = {
		.platform_data = &pwmdev_data9,
	},
};

static struct platform_pwm_dev_data pwmdev_data10 = {
	.pwm_sch = PWMDEV_NULL,
	.pin_num = DRIME4_GPIO_END,
};

static struct platform_device pwmdev_device10 = {
	.name = "pwmdev",
	.id = 10,
	.dev = {
		.platform_data = &pwmdev_data10,
	},
};


static struct platform_pwm_dev_data pwmdev_data11 = {
	.pwm_sch = PWMDEV_NULL,
	.pin_num = DRIME4_GPIO_END,
};

static struct platform_device pwmdev_device11 = {
	.name = "pwmdev",
	.id = 11,
	.dev = {
		.platform_data = &pwmdev_data11,
	},
};

static struct platform_pwm_dev_data pwmdev_data12 = {
	.pwm_sch = PWMDEV_NULL,
	.pin_num = DRIME4_GPIO_END,
};

static struct platform_device pwmdev_device12 = {
	.name = "pwmdev",
	.id = 12,
	.dev = {
		.platform_data = &pwmdev_data12,
	},
};

static struct platform_pwm_dev_data pwmdev_data13 = {
	.pwm_sch = PWMDEV_NULL,
	.pin_num = DRIME4_GPIO_END,
};

static struct platform_device pwmdev_device13 = {
	.name = "pwmdev",
	.id = 13,
	.dev = {
		.platform_data = &pwmdev_data13,
	},
};

static struct platform_pwm_dev_data pwmdev_data14 = {
	.pwm_sch = PWMDEV_NULL,
	.pin_num = DRIME4_GPIO_END,
};

static struct platform_device pwmdev_device14 = {
	.name = "pwmdev",
	.id = 14,
	.dev = {
		.platform_data = &pwmdev_data14,
	},
};

static struct platform_pwm_dev_data pwmdev_data15 = {
	.pwm_sch = PWMDEV_NULL,
	.pin_num = DRIME4_GPIO_END,
};

static struct platform_device pwmdev_device15 = {
	.name = "pwmdev",
	.id = 15,
	.dev = {
		.platform_data = &pwmdev_data15,
	},
};

static struct platform_pwm_dev_data pwmdev_data16 = {
	.pwm_sch = PWMDEV_NULL,
	.pin_num = DRIME4_GPIO_END,
};

static struct platform_device pwmdev_device16 = {
	.name = "pwmdev",
	.id = 16,
	.dev = {
		.platform_data = &pwmdev_data16,
	},
};

static struct platform_pwm_dev_data pwmdev_data17 = {
	.pwm_sch = PWMDEV_NULL,
	.pin_num = DRIME4_GPIO_END,
};

static struct platform_device pwmdev_device17 = {
	.name = "pwmdev",
	.id = 17,
	.dev = {
		.platform_data = &pwmdev_data17,
	},
};

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------
#ifdef CONFIG_PTC_0
static struct platform_device ptc_device0 = {.name = "ptcdev", .id = 0,};
#endif

#ifdef CONFIG_PTC_1
static struct platform_device ptc_device1 = {.name = "ptcdev", .id = 1,};
#endif

static struct platform_device adc_device = {.name = "adcdev",};

//SPI (CH0 ~ CH7) ----------------------------------------------------------------------------------------------------------------------------------------------------

static struct platform_device spi_udd_device0 = {.name = "hs_spidev", .id = 0,};
static struct platform_device spi_udd_device1 = {.name = "hs_spidev", .id = 1,};
static struct platform_device spi_udd_device2 = {.name = "hs_spidev", .id = 2,};
static struct platform_device spi_udd_device3 = {.name = "hs_spidev", .id = 3,};
static struct platform_device spi_udd_device4 = {.name = "hs_spidev", .id = 4,};
static struct platform_device spi_udd_device5 = {.name = "hs_spidev", .id = 5,};
static struct platform_device spi_udd_device6 = {.name = "hs_spidev", .id = 6,};
static struct platform_device spi_udd_device7 = {.name = "hs_spidev", .id = 7,};

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------


static struct drime4_platform_fb tv_pd = {
	.buffer_cnt = 0,
	.vid_wins = 1,
	.grp_wins = 2,
	.layer = {
		.video_stride = 1920*4,
		.graphic_stride = 7680,
		.graphic_scale = GRP_SCL_X1,
		.vid_win[0] = {
			.format = DP_YUV_420,
			.vid_image = {
				.img_width = 1920,
				.img_height = 1080,
			},
			.vid_display = {
				.Display_H_Start = 0,
				.Display_H_Size = 1920,
				.Display_V_Start = 0,
				.Display_V_Size = 1080,
			},
		},
		.vid_win[1] = {
			.format = DP_YUV_420,
			.vid_image = {
				.img_width = 1920,
				.img_height = 1080,
			},
			.vid_display = {
				.Display_H_Start = 0,
				.Display_H_Size = 1920,
				.Display_V_Start = 0,
				.Display_V_Size = 1080,
			},
		},
		.vid_win[2] = {
			.format = DP_YUV_420,
			.vid_image = {
				.img_width = 1920,
				.img_height = 1080,
			},
			.vid_display = {
				.Display_H_Start = 0,
				.Display_H_Size = 1920,
				.Display_V_Start = 0,
				.Display_V_Size = 1080,
			},
		},
		.vid_win[3] = {
			.format = DP_YUV_420,
			.vid_image = {
				.img_width = 1920,
				.img_height = 1080,
			},
			.vid_display = {
				.Display_H_Start = 0,
				.Display_H_Size = 1920,
				.Display_V_Start = 0,
				.Display_V_Size = 1080,
			},
		},
		.grp_win[0] = {
			.vid_image = {
				.img_width = 1920,
				.img_height = 1080,
			},
			.vid_display = {
				.Display_H_Start = 0,
				.Display_H_Size = 1920,
				.Display_V_Start = 0,
				.Display_V_Size = 1080,
			},
		},
		.grp_win[1] = {
			.vid_image = {
				.img_width = 1920,
				.img_height = 1080,
			},
			.vid_display = {
				.Display_H_Start = 0,
				.Display_H_Size = 1920,
				.Display_V_Start = 0,
				.Display_V_Size = 1080,
			},
		},
		.grp_win[2] = {
			.vid_image = {
				.img_width = 1920,
				.img_height = 1080,
			},
			.vid_display = {
				.Display_H_Start = 0,
				.Display_H_Size = 1920,
				.Display_V_Start = 0,
				.Display_V_Size = 1080,
			},
		},
		.grp_win[3] = {
			.vid_image = {
				.img_width = 1920,
				.img_height = 1080,
			},
			.vid_display = {
				.Display_H_Start = 0,
				.Display_H_Size = 1920,
				.Display_V_Start = 0,
				.Display_V_Size = 1080,
			},
		},
	},

};


#ifdef CONFIG_FB_DRIME4

static struct drime4_platform_tvfb fb_pd = {
	.buffer_cnt = 0,
	.vid_wins = 1,
	.grp_wins = 2,
	.layer = {
		.video_stride = 2560,
		.graphic_stride = 2560,
		.graphic_scale = GRP_SCL_X1,
		.vid_win[0] = {
			.format = DP_YUV_420,
			.vid_image = {
				.img_width = 640,
				.img_height = 480,
			},
			.vid_display = {
				.Display_H_Start = 0,
				.Display_H_Size = 640,
				.Display_V_Start = 0,
				.Display_V_Size = 480,
			},
		},
		.vid_win[1] = {
			.format = DP_YUV_420,
			.vid_image = {
				.img_width = 640,
				.img_height = 480,
			},
			.vid_display = {
				.Display_H_Start = 0,
				.Display_H_Size = 640,
				.Display_V_Start = 0,
				.Display_V_Size = 480,
			},
		},
		.vid_win[2] = {
			.format = DP_YUV_420,
			.vid_image = {
				.img_width = 640,
				.img_height = 480,
			},
			.vid_display = {
				.Display_H_Start = 0,
				.Display_H_Size = 640,
				.Display_V_Start = 0,
				.Display_V_Size = 480,
			},
		},
		.vid_win[3] = {
			.format = DP_YUV_420,
			.vid_image = {
				.img_width = 640,
				.img_height = 480,
			},
			.vid_display = {
				.Display_H_Start = 0,
				.Display_H_Size = 640,
				.Display_V_Start = 0,
				.Display_V_Size = 480,
			},
		},
		.grp_win[0] = {
			.vid_image = {
				.img_width = 640,
				.img_height = 480,
			},
			.vid_display = {
				.Display_H_Start = 0,
				.Display_H_Size = 640,
				.Display_V_Start = 0,
				.Display_V_Size = 480,
			},
		},
		.grp_win[1] = {
			.vid_image = {
				.img_width = 640,
				.img_height = 480,
			},
			.vid_display = {
				.Display_H_Start = 0,
				.Display_H_Size = 640,
				.Display_V_Start = 0,
				.Display_V_Size = 480,
			},
		},
		.grp_win[2] = {
			.vid_image = {
				.img_width = 640,
				.img_height = 480,
			},
			.vid_display = {
				.Display_H_Start = 0,
				.Display_H_Size = 640,
				.Display_V_Start = 0,
				.Display_V_Size = 480,
			},
		},
		.grp_win[3] = {
			.vid_image = {
				.img_width = 640,
				.img_height = 480,
			},
			.vid_display = {
				.Display_H_Start = 0,
				.Display_H_Size = 640,
				.Display_V_Start = 0,
				.Display_V_Size = 480,
			},
		},
	},

};
#endif


#ifdef CONFIG_NXLENS_COMM_MAN
static struct platform_device lens_device = {.name = "lens_comm", .id = -1,};
#endif


#ifdef CONFIG_DRIME4_PP_SSIF
static struct drimet_pp_ssif_ext_tg_data pp_ssif_clk_data = {
		.ext_tg_clk = 27000000,
		.sg_cksel = "ext_tg",
		.s0_cksel = "byte_clk2",
		.s1_cksel = "byte_clk2",
		.s2_cksel = "byte_clk2",
		.s3_cksel = "byte_clk2",
};

static struct platform_device ssif_iodev = {
		.name = "pp_ssif_io",
		.id = -1,
		.dev = {
		.platform_data = &pp_ssif_clk_data,
	},
};
#endif
/* NAND Flash on DRIME4 board */
#ifdef CONFIG_MTD_NAND_DRIME4
static struct drime4_nand_set drime4_nand_sets[] __initdata = {
	[0] = {
		.name = "drime4-nand",
		.nr_chips = 1,
		.flash_bbt = 0, /* we use u-boot to create a BBT */
	},
};

static struct drime4_platform_nand drime4_nand_info __initdata = {
	.tacls = 1,
	.twrph0 = 4,
	.twrph1 = 2,
	.nr_sets = ARRAY_SIZE(drime4_nand_sets),
	.sets = drime4_nand_sets,
	.ignore_unset_ecc = 1,
};
#endif
#define PINCTRL_DEV	"drime4-pinmux"

/* Padmux setting */
static struct pinctrl_map drime4_padmux_map[] = {
#if defined(CONFIG_DRIME4_SDIO) || defined(CONFIG_DRIME4_EMMC)
	PIN_MAP_MUX_GROUP_DEFAULT("dw_mmc_sdmmc.1", PINCTRL_DEV, NULL, "sd0"),
#else
	PIN_MAP_MUX_GROUP_DEFAULT("dw_mmc_sdmmc.0", PINCTRL_DEV, NULL, "sd0"),
#endif
	PIN_MAP_MUX_GROUP_DEFAULT("drime4-i2c.0", PINCTRL_DEV, NULL, "i2c0"),
	PIN_MAP_MUX_GROUP_DEFAULT("drime4-i2c.1", PINCTRL_DEV, NULL, "i2c1"),
	PIN_MAP_MUX_GROUP_DEFAULT("drime4-i2c.2", PINCTRL_DEV, NULL, "i2c2"),
	PIN_MAP_MUX_GROUP_DEFAULT("drime4-i2c.3", PINCTRL_DEV, NULL, "i2c3"),
	PIN_MAP_MUX_GROUP_DEFAULT("drime4-i2c.4", PINCTRL_DEV, NULL, "i2c4"),
	//PIN_MAP_MUX_GROUP_DEFAULT("drime4-i2c.5", PINCTRL_DEV, NULL, "i2c5"),
	PIN_MAP_MUX_GROUP_DEFAULT("drime4-i2c.6", PINCTRL_DEV, NULL, "i2c6"),
	//PIN_MAP_MUX_GROUP_DEFAULT("drime4-i2c.7", PINCTRL_DEV, NULL, "i2c7"),
	PIN_MAP_MUX_GROUP_DEFAULT("drime4-ccd.0", PINCTRL_DEV, NULL, "ccd0"),
	PIN_MAP_MUX_GROUP_DEFAULT("drime4-ccd.1", PINCTRL_DEV, NULL, "ccd1"),
	PIN_MAP_MUX_GROUP_DEFAULT("drime4-i2s.1", PINCTRL_DEV, NULL, "i2s1"),
	PIN_MAP_MUX_GROUP_DEFAULT("drime4-i2s.2", PINCTRL_DEV, NULL, "i2s2"),
	PIN_MAP_MUX_GROUP_DEFAULT("drime4-i2s.3", PINCTRL_DEV, NULL, "i2s3"),
	PIN_MAP_MUX_GROUP_DEFAULT("drime4-i2s.4", PINCTRL_DEV, NULL, "i2s4"),
	PIN_MAP_MUX_GROUP_DEFAULT("drime4-ata", PINCTRL_DEV, NULL, "ata"),
	PIN_MAP_MUX_GROUP_DEFAULT("drime4-efs.0", PINCTRL_DEV, NULL, "efs0"),
	PIN_MAP_MUX_GROUP_DEFAULT("drime4-efs.1", PINCTRL_DEV, NULL, "efs1"),
#if defined(CONFIG_DRIME4_SDIO) || defined(CONFIG_DRIME4_EMMC)
	PIN_MAP_MUX_GROUP_DEFAULT("dw_mmc_sdio.0", PINCTRL_DEV, NULL, "sd1"),
#endif
	PIN_MAP_MUX_GROUP_DEFAULT("drime4-ptc.0", PINCTRL_DEV, NULL, "ptc0"),
	PIN_MAP_MUX_GROUP_DEFAULT("drime4-ptc.1", PINCTRL_DEV, NULL, "ptc1"),
	PIN_MAP_MUX_GROUP_DEFAULT("drime4_spi.0", PINCTRL_DEV, NULL, "spi0"),
	PIN_MAP_MUX_GROUP_DEFAULT("drime4_spi.1", PINCTRL_DEV, NULL, "spi1"),
	PIN_MAP_MUX_GROUP_DEFAULT("drime4_spi.2", PINCTRL_DEV, NULL, "spi2"),
	PIN_MAP_MUX_GROUP_DEFAULT("drime4_spi.3", PINCTRL_DEV, NULL, "spi3"),
	PIN_MAP_MUX_GROUP_DEFAULT("drime4_spi.4", PINCTRL_DEV, NULL, "spi4"),
	PIN_MAP_MUX_GROUP_DEFAULT("drime4_spi.5", PINCTRL_DEV, NULL, "spi5"),
	PIN_MAP_MUX_GROUP_DEFAULT("drime4_spi.6", PINCTRL_DEV, NULL, "spi6"),
	PIN_MAP_MUX_GROUP_DEFAULT("drime4_spi.7", PINCTRL_DEV, NULL, "spi7"),

	PIN_MAP_MUX_GROUP_DEFAULT("drime4-pwm.0", PINCTRL_DEV, NULL, "pwm0"),
	PIN_MAP_MUX_GROUP_DEFAULT("drime4-pwm.1", PINCTRL_DEV, NULL, "pwm1"),
	PIN_MAP_MUX_GROUP_DEFAULT("drime4-pwm.2", PINCTRL_DEV, NULL, "pwm2"),
	PIN_MAP_MUX_GROUP_DEFAULT("drime4-pwm.3", PINCTRL_DEV, NULL, "pwm3"),
	PIN_MAP_MUX_GROUP_DEFAULT("drime4-pwm.4", PINCTRL_DEV, NULL, "pwm4"),
	PIN_MAP_MUX_GROUP_DEFAULT("drime4-pwm.5", PINCTRL_DEV, NULL, "pwm5"),
	PIN_MAP_MUX_GROUP_DEFAULT("drime4-pwm.6", PINCTRL_DEV, NULL, "pwm6"),
	PIN_MAP_MUX_GROUP_DEFAULT("drime4-pwm.7", PINCTRL_DEV, NULL, "pwm7"),
	PIN_MAP_MUX_GROUP_DEFAULT("drime4-pwm.8", PINCTRL_DEV, NULL, "pwm8"),
	PIN_MAP_MUX_GROUP_DEFAULT("drime4-pwm.9", PINCTRL_DEV, NULL, "pwm9"),
	PIN_MAP_MUX_GROUP_DEFAULT("drime4-pwm.10", PINCTRL_DEV, NULL, "pwm10"),
	PIN_MAP_MUX_GROUP_DEFAULT("drime4-pwm.11", PINCTRL_DEV, NULL, "pwm11"),
	PIN_MAP_MUX_GROUP_DEFAULT("drime4-pwm.12", PINCTRL_DEV, NULL, "pwm12"),
	PIN_MAP_MUX_GROUP_DEFAULT("drime4-pwm.13", PINCTRL_DEV, NULL, "pwm13"),
	PIN_MAP_MUX_GROUP_DEFAULT("drime4-pwm.14", PINCTRL_DEV, NULL, "pwm14"),
	PIN_MAP_MUX_GROUP_DEFAULT("drime4-pwm.15", PINCTRL_DEV, NULL, "pwm15"),
	PIN_MAP_MUX_GROUP_DEFAULT("drime4-pwm.16", PINCTRL_DEV, NULL, "pwm16"),
	PIN_MAP_MUX_GROUP_DEFAULT("drime4-pwm.17", PINCTRL_DEV, NULL, "pwm17"),
	PIN_MAP_MUX_GROUP_DEFAULT("uart1", PINCTRL_DEV, NULL, "uart1"),
	PIN_MAP_MUX_GROUP_DEFAULT("d4_pp_ssif.0", PINCTRL_DEV, NULL, "slvds"),
#ifdef CONFIG_DRIME4_SRP
	PIN_MAP_MUX_GROUP_DEFAULT("d4_srp", PINCTRL_DEV, NULL, "srp1"),
#endif
};

void __init drime4_fb_init(void)
{
#ifdef CONFIG_FB_DRIME4
	drime4_device_fb.dev.platform_data = (void *)&fb_pd;
#endif
}

void __init drime4_tv_init(void)
{
#ifdef CONFIG_DRM_DRIME4_DP_TV
	drime4_device_tv.dev.platform_data = (void *)&tv_pd;
#endif
}
static void __init drime4_init_i2c(void)
{
	int size;

#ifdef CONFIG_DRIME4_I2C0
	if(GetBoardVersion() >= eBdVer_PV1){
		i2c_register_board_info(0, i2c_devs0, ARRAY_SIZE(i2c_devs0));
	}
	drime4_register_i2c(0, NULL);
#endif

#ifdef CONFIG_DRIME4_I2C1
	i2c_register_board_info(1, i2c_devs1, ARRAY_SIZE(i2c_devs1));
	drime4_register_i2c(1, NULL);
#endif

	size = 0;
#ifdef CONFIG_DRIME4_I2C2
	size = 1;
	drime4_register_i2c(2, NULL);
	i2c_register_board_info(2, i2c_devs2, ARRAY_SIZE(i2c_devs2));
#endif

	size = 0;
#ifdef CONFIG_DRIME4_I2C3
	size = 1;
	i2c_register_board_info(3, i2c_devs3, size);
	drime4_register_i2c(3, NULL);
#endif

	size = 0;
#ifdef CONFIG_DRIME4_I2C4
	size = 1;
	i2c_register_board_info(4, i2c_devs4, size);
	drime4_register_i2c(4, NULL);
#endif

#ifdef CONFIG_DRIME4_I2C6
	i2c_register_board_info(6, i2c_devs6, ARRAY_SIZE(i2c_devs6));
	drime4_register_i2c(6, NULL);
#endif
}

static void __init drime4_reserve(void)
{
#ifdef CONFIG_DRIME4_SMA
	if (dma_declare_contiguous(&drime4_device_sma.dev, CMA_REGION1_SIZE,	CMA_REGION1_START, 0))
		printk("sma region declare is failed \n");
#endif
#ifdef CONFIG_SCORE_SUSPEND
	{
		extern phys_addr_t memblock_alloc_base(phys_addr_t size, phys_addr_t align, phys_addr_t max_addr);
		extern int reserve_bootmem(unsigned long addr, unsigned long size, int flags);
		extern void drime4_cpu_restore(void);
		extern unsigned int reserved_for_resume_point;

#define BOOTMEM_EXCLUSIVE	(1<<0)
		phys_addr_t found;
		int ret;

		/* Reserve MEM for resume-point */
#if 0
		found = memblock_alloc_base(PAGE_SIZE, PAGE_SIZE, D4_CMA_REG1_START);
		ret = reserve_bootmem(found, PAGE_SIZE, BOOTMEM_EXCLUSIVE);
		if (ret && ret != -EBUSY) {
			reserved_for_resume_point = ret;
			printk(KERN_INFO "Reserving a page(0x%x) for resume point, ret(0x%x)\n", found, ret);
		}
#else
		reserved_for_resume_point = CMA_DDR_END - POWER_CONF_SZ;
#endif
	}
#endif
#ifdef CONFIG_SCORE_FAST_BOOT
	extern void mark_cma_nosave_region(unsigned long start_pfn, unsigned long end_pfn);
	mark_cma_nosave_region(CMA_REGION1_START >> PAGE_SHIFT, (CMA_REGION1_START + CMA_REGION1_SMA_SIZE) >> PAGE_SHIFT);
#endif
}


extern int oneshot_direct;

 static void drime4_poweroff(void)
{
	struct d4_rmu_device * rmu;

	rmu = d4_rmu_request();
	if (rmu == -12)
		return;
	if(oneshot_direct == 1){
		printk(KERN_EMERG"drime4_poweroff oneshot reboot\n");
		pinctrl_request_gpio(GPIO_1SHOT_HS);
		gpio_request(GPIO_1SHOT_HS,"1shot_hs");
		gpio_direction_output(GPIO_1SHOT_HS,1);	/* Disable interrupts first */
	}
 	d4_pcu_intr_mask_set(NULL);
	d4_pcu_hold_set(rmu);
	d4_pcu_ddroff_set(rmu);
	d4_pcu_off_set(rmu);

	d4_rmu_release(rmu);
}

#define GPIO_DATA	0x3FC
#define GPIO_DIR	0x400
#define GPIO_SET_OUT0(group_num, pin_num) \
{ \
	*((volatile unsigned long *)(DRIME4_VA_GPIO_BASE(group_num) + GPIO_DIR))  |=  (1 << (pin_num)); \
	*((volatile unsigned long *)(DRIME4_VA_GPIO_BASE(group_num) + GPIO_DATA)) &= ~(1 << (pin_num)); \
}

static void __init drime4_d4es_init(void)
{
	/*
	 * initialize gpio
	 */
	EBdVersion bd_ver = GetBoardVersion();		// BVC Initialize!

#ifdef CONFIG_TOUCHSCREEN_MELFAS
	/* redefine power and interrupt pin for touch screen */
	ts_config.pin_num = GPIO_TSP_INT;
	ts_config.pwr_num = GPIO_TSP_3_3V_ON;
	ts_config.max_x = 480;
	ts_config.max_y = 800;
#endif

	d4keys_gpio_init();
	d4keys_polled_init();
	d4_led_init_gpio();
	pinctrl_register_mappings(drime4_padmux_map, ARRAY_SIZE(drime4_padmux_map));

#ifdef CONFIG_PM
	drime4_pm_init();
#endif
	drime4_init_common_devices();
	drime4_init_i2c();
	drime4_fb_init();									// CONFIG_FB_DRIME4
#ifdef CONFIG_DRM_DRIME4_DP_TV
	drime4_tv_init();									// CONFIG_DRM_DRIME4_DP_TV
#endif
	drime4_nand_set_platdata(&drime4_nand_info);		// CONFIG_MTD_NAND_DRIME4


#ifdef CONFIG_PTC_0
	platform_device_register(&ptc_device0);
#endif
#ifdef CONFIG_PTC_1
	platform_device_register(&ptc_device1);
#endif
	platform_device_register(&adc_device);


	// Register PWM (CH0 ~ CH17) -------------------------------------------------------------------------------------------------------
	platform_device_register(&d4_device_pwm[0]);	// CONFIG_HT_PWM0
	platform_device_register(&d4_device_pwm[1]);	// CONFIG_HT_PWM1
	platform_device_register(&d4_device_pwm[2]);	// CONFIG_HT_PWM2
	platform_device_register(&d4_device_pwm[3]);	// CONFIG_HT_PWM3
	platform_device_register(&d4_device_pwm[4]);	// CONFIG_HT_PWM4
	platform_device_register(&d4_device_pwm[5]);	// CONFIG_HT_PWM5
	platform_device_register(&d4_device_pwm[6]);	// CONFIG_HT_PWM6
	platform_device_register(&d4_device_pwm[7]);	// CONFIG_HT_PWM7
	//platform_device_register(&d4_device_pwm[8]);		// CONFIG_HT_PWM8
	platform_device_register(&d4_device_pwm[9]);		// CONFIG_HT_PWM9
	//platform_device_register(&d4_device_pwm[10]);		// CONFIG_HT_PWM10
	//platform_device_register(&d4_device_pwm[11]);		// CONFIG_HT_PWM11
	//platform_device_register(&d4_device_pwm[12]);		// CONFIG_HT_PWM12
#if defined(CONFIG_HT_PWM13)
	platform_device_register(&d4_device_pwm[13]);	// CONFIG_HT_PWM13
#endif	
	//platform_device_register(&d4_device_pwm[14]);		// CONFIG_HT_PWM14
	//platform_device_register(&d4_device_pwm[15]);		// CONFIG_HT_PWM15
	//platform_device_register(&d4_device_pwm[16]);		// CONFIG_HT_PWM16
	//platform_device_register(&d4_device_pwm[17]);		// CONFIG_HT_PWM17

	//platform_device_register(&pwmdev_device0);		// CONFIG_UDD_PWM0	On
	//platform_device_register(&pwmdev_device1);		// CONFIG_UDD_PWM1	On
	platform_device_register(&pwmdev_device2);		// CONFIG_UDD_PWM2	On
	platform_device_register(&pwmdev_device3);		// CONFIG_UDD_PWM3	On
	platform_device_register(&pwmdev_device4);		// CONFIG_UDD_PWM4	On
	platform_device_register(&pwmdev_device5);		// CONFIG_UDD_PWM5	On
	//platform_device_register(&pwmdev_device6);		// CONFIG_UDD_PWM6
	platform_device_register(&pwmdev_device7);		// CONFIG_UDD_PWM7	On
	//platform_device_register(&pwmdev_device8);		// CONFIG_UDD_PWM8
	//platform_device_register(&pwmdev_device9);		// CONFIG_UDD_PWM9
	//platform_device_register(&pwmdev_device10);		// CONFIG_UDD_PWM10
	//platform_device_register(&pwmdev_device11);		// CONFIG_UDD_PWM11
	//platform_device_register(&pwmdev_device12);		// CONFIG_UDD_PWM12
	//platform_device_register(&pwmdev_device13);		// CONFIG_UDD_PWM13
	//platform_device_register(&pwmdev_device14);		// CONFIG_UDD_PWM14
	//platform_device_register(&pwmdev_device15);		// CONFIG_UDD_PWM15
	//platform_device_register(&pwmdev_device16);		// CONFIG_UDD_PWM16
	//platform_device_register(&pwmdev_device17);		// CONFIG_UDD_PWM17
	// ------------------------------------------------------------------------------------------------------------------------------

	// Register SPI (CH0 ~ CH7) ---------------------------------------------------------------------------------------------------------
	//platform_device_register(&spi_udd_device0);		// CONFIG_UDD_SPI0
	platform_device_register(&spi_udd_device1);		// CONFIG_UDD_SPI1	On
	platform_device_register(&spi_udd_device2);		// CONFIG_UDD_SPI2	On
	//platform_device_register(&spi_udd_device3);		// CONFIG_UDD_SPI3
	platform_device_register(&spi_udd_device4);		// CONFIG_UDD_SPI4
	platform_device_register(&spi_udd_device5);		// CONFIG_UDD_SPI5
	//platform_device_register(&spi_udd_device6);		// CONFIG_UDD_SPI6
	//platform_device_register(&spi_udd_device7);		// CONFIG_UDD_SPI7
	// ------------------------------------------------------------------------------------------------------------------------------

#if defined(CONFIG_DRIME4_PP_SSIF)
	platform_device_register(&ssif_iodev);				// CONFIG_DRIME4_PP_SSIF
#endif
#if defined(CONFIG_NXLENS_COMM_MAN)
	platform_device_register(&lens_device);				// CONFIG_NXLENS_COMM_MAN
#endif
	platform_device_register(&drime4_device_padmux);
	platform_device_register(&drime4_asoc_dma);
#if defined(CONFIG_MMC_DW)
	platform_device_register(&drime4_device_sdmmc);				// CONFIG_MMC_DW
#endif
	//platform_device_register(&drime4_device_emmc);							// CONFIG_DRIME4_EMMC
#if defined(CONFIG_DRIME4_SDIO)
	platform_device_register(&drime4_device_sdio);					// CONFIG_DRIME4_SDIO
#endif
#if defined(CONFIG_MTD_NAND_DRIME4)
	platform_device_register(&drime4_device_nand);					// CONFIG_MTD_NAND_DRIME4
#endif
	platform_device_register(&drime4_device_i2s0);
	platform_device_register(&drime4_device_i2s2);
#if defined(CONFIG_KEYBOARD_DRIME4ES)
	platform_device_register(&drime4_device_key);					// CONFIG_KEYBOARD_DRIME4ES
	platform_device_register(&drime4_device_key_polled);			// CONFIG_KEYBOARD_DRIME4ES
#endif
#if defined(CONFIG_DRIME4_BMA)
	platform_device_register(&drime4_device_bma);					// CONFIG_DRIME4_BMA
#endif
#if defined(CONFIG_DRIME4_SMA)
	platform_device_register(&drime4_device_sma);					// CONFIG_DRIME4_SMA
#endif
#if defined(CONFIG_DRIME4_PP_CORE)
	platform_device_register(&drime4_device_pp_core);				// CONFIG_DRIME4_PP_CORE
#endif
#if defined(CONFIG_DRIME4_MIPI)
	platform_device_register(&drime4_device_mipi);								// CONFIG_DRIME4_MIPI
#endif
#if defined(CONFIG_DRIME4_PP_SSIF)
	platform_device_register(&drime4_device_pp_ssif);				// CONFIG_DRIME4_PP_SSIF
#endif
#if defined(CONFIG_DRIME4_PP_3A)
	platform_device_register(&drime4_device_pp_3a);							// CONFIG_DRIME4_PP_3A
#endif	
#if defined(CONFIG_DRIME4_IPCM)
	platform_device_register(&drime4_device_ipcm);					// CONFIG_DRIME4_IPCM
#endif
#if defined(CONFIG_DRIME4_IPCS)
	platform_device_register(&drime4_device_ipcs);					// CONFIG_DRIME4_IPCS
#endif
	//platform_device_register(&drime4_device_jxr);								// CONFIG_DRIME4_JPEG_XR
#if defined(CONFIG_DRIME4_JPEG)	
	platform_device_register(&drime4_device_jpeg);					// CONFIG_DRIME4_JPEG
#endif
#if defined(CONFIG_DRIME4_EP)
	platform_device_register(&drime4_device_ep);					// CONFIG_DRIME4_EP
#endif	

	platform_device_register(&drime4_device_opener);							// CONFIG_DRIME4_OPENER
#if defined(CONFIG_DRIME4_BE)
	platform_device_register(&drime4_device_be);					// CONFIG_DRIME4_BE
#endif
#if defined(CONFIG_VIDEO_MFC5X)
	platform_device_register(&drime4_device_mfc);								// CONFIG_VIDEO_MFC5X
#endif	
#ifdef CONFIG_DRIME4_LCD_PANEL_MANAGER
	platform_device_register(&drime4_device_lcd_panel_manager);		/* CONFIG_DRIME4_LCD_PANEL_MANAGER */
#endif
#if defined(CONFIG_DRIME4_BWM)
	platform_device_register(&drime4_device_bwm);					// CONFIG_DRIME4_BWM
#endif
#if defined(CONFIG_DRIME4_SRP)
	platform_device_register(&drime4_device_srp);					// CONFIG_DRIME4_SRP
#endif	
	platform_device_register(&drime4_device_spi0);					// CONFIG_DRIME4_SPI0
	platform_device_register(&drime4_device_spi1);					// CONFIG_DRIME4_SPI1
	platform_device_register(&drime4_device_spi2);					// CONFIG_DRIME4_SPI2
	platform_device_register(&drime4_device_spi3);					// CONFIG_DRIME4_SPI3
	platform_device_register(&drime4_device_spi4);					// CONFIG_DRIME4_SPI4
	platform_device_register(&drime4_device_spi5);					// CONFIG_DRIME4_SPI5
#if defined(CONFIG_DRIME4_SPI6)
	platform_device_register(&drime4_device_spi6);								//CONFIG_DRIME4_SPI7 && CONFIG_CMOS_AQUILA_SENSOR
#endif	

#ifdef CONFIG_DRIME4_GPU
	platform_device_register(&powervr_device);					// CONFIG_DRIME4_GPU
#endif
#ifdef CONFIG_FB_DRIME4
	platform_device_register(&drime4_device_fb);					/* CONFIG_FB_DRIME4 */
#endif
	platform_device_register(&drime4_device_rmu);					// CONFIG_DRIME4_DEV_RMU

#ifdef CONFIG_DRM_DRIME4
	platform_device_register(&drime4_device_drm);						// CONFIG_DRM_DRIME4
#endif
#ifdef CONFIG_DRM_DRIME4_DP_LCD
	platform_device_register(&drime4_device_lcd);						// CONFIG_DRM_DRIME4_DP_LCD
#endif
#ifdef CONFIG_DRM_DRIME4_DP_SUBLCD
	platform_device_register(&drime4_device_sublcd);					// CONFIG_DRM_DRIME4_DP_SUBLCD
#endif

	platform_device_register(&drime4_device_adc);						// CONFIG_DRIME4_ADC
#if defined(CONFIG_STROBE_CONTROLLER)
	platform_device_register(&drime4_str_device); 						// CONFIG_STROBE_CONTROLLER
#endif	
#if defined(CONFIG_JACK_MON)
	platform_device_register(&d4_jack); 									// CONFIG_JACK_MON
#endif
#if defined(CONFIG_HT_PWM4)
	platform_device_register(&d4_device_dr_ioctl); 						// CONFIG_HT_PWM4
#endif	
	platform_device_register(&drime4_gpio_leds); 						// CONFIG_LEDS_GPIO
	platform_device_register(&d4_device_pmu);							// CONFIG_DRIME4_PMU

#ifdef CONFIG_DRIME4_USB3_DRD
	platform_device_register(&drime4_device_usb_dwc3);
	platform_device_register(&s3c_device_android_usb);
	platform_device_register(&s3c_device_usb_mass_storage);
#endif

	// ------------------------------------------------------------------------------------------------------------------------------
#ifdef CONFIG_PTC_0
	platform_device_register(&d4_device_ptc[0]);
#endif
#ifdef CONFIG_PTC_0
	platform_device_register(&d4_device_ptc[1]);
#endif
	// GPIO (G0 ~ G27) ----------------------------------------------------------------------------------------------------------------
	platform_device_register(&gpio_udevice[0]);							// CONFIG_DRIME4_UGPIO0
	platform_device_register(&gpio_udevice[1]);							// CONFIG_DRIME4_UGPIO1
	platform_device_register(&gpio_udevice[2]);							// CONFIG_DRIME4_UGPIO2
	platform_device_register(&gpio_udevice[3]);							// CONFIG_DRIME4_UGPIO3
	platform_device_register(&gpio_udevice[4]);							// CONFIG_DRIME4_UGPIO4
	platform_device_register(&gpio_udevice[5]);							// CONFIG_DRIME4_UGPIO5
	platform_device_register(&gpio_udevice[6]);							// CONFIG_DRIME4_UGPIO6
	platform_device_register(&gpio_udevice[7]);							// CONFIG_DRIME4_UGPIO7
	platform_device_register(&gpio_udevice[8]);							// CONFIG_DRIME4_UGPIO8
	platform_device_register(&gpio_udevice[9]);							// CONFIG_DRIME4_UGPIO9
	platform_device_register(&gpio_udevice[10]);							// CONFIG_DRIME4_UGPIO10
	platform_device_register(&gpio_udevice[11]);							// CONFIG_DRIME4_UGPIO11
	platform_device_register(&gpio_udevice[12]);							// CONFIG_DRIME4_UGPIO12
	platform_device_register(&gpio_udevice[13]);							// CONFIG_DRIME4_UGPIO13
	platform_device_register(&gpio_udevice[14]);							// CONFIG_DRIME4_UGPIO14
	platform_device_register(&gpio_udevice[15]);							// CONFIG_DRIME4_UGPIO15
	platform_device_register(&gpio_udevice[16]);							// CONFIG_DRIME4_UGPIO16
	platform_device_register(&gpio_udevice[17]);							// CONFIG_DRIME4_UGPIO17
	platform_device_register(&gpio_udevice[18]);							// CONFIG_DRIME4_UGPIO18
	platform_device_register(&gpio_udevice[19]);							// CONFIG_DRIME4_UGPIO19
	platform_device_register(&gpio_udevice[20]); 							// CONFIG_DRIME4_UGPIO20
	platform_device_register(&gpio_udevice[21]); 							// CONFIG_DRIME4_UGPIO21
	platform_device_register(&gpio_udevice[22]); 							// CONFIG_DRIME4_UGPIO22
	platform_device_register(&gpio_udevice[23]); 							// CONFIG_DRIME4_UGPIO23
	platform_device_register(&gpio_udevice[24]); 							// CONFIG_DRIME4_UGPIO24
	platform_device_register(&gpio_udevice[25]); 							// CONFIG_DRIME4_UGPIO25
	platform_device_register(&gpio_udevice[26]); 							// CONFIG_DRIME4_UGPIO26
	platform_device_register(&gpio_udevice[27]); 							// CONFIG_DRIME4_UGPIO27
	// ------------------------------------------------------------------------------------------------------------------------------
	if(GetBoardVersion() < eBdVer_PV1){
		platform_device_register(&drime4_device_rtc);						// CONFIG_DRIME4_RTC
	}
	platform_device_register(&drime4_batchk_pdev);						// CONFIG_BATTERY_DRIME_ADC
#ifdef CONFIG_DRM_DRIME4_DP_TV
    platform_device_register(&drime4_device_tv);
	platform_device_register(&drime4_device_hdmi);
    platform_device_register(&drime4_device_cec);
	platform_device_register(&drime4_drm_hdmi_device);
	platform_device_register(&drime4_device_alsa_hdmi);
#endif

#ifdef CONFIG_DRIME4_CSM
	platform_device_register(&drime4_device_csm);							/* CONFIG_DRIME4_CSM */
#endif

//	platform_device_register(&drime4_cpuidle_device);

  	pm_power_off = drime4_poweroff;
	
	/**
	 *
	 * Initialize unused i/o as GPIO to avoid outside hardware signal effect.
	 *
	 */

	*((volatile unsigned long *)(DRIME4_VA_GLOBAL_CTRL + 0x0010)) = 0x30843; // DRIME4_GPIO0(4)
	GPIO_SET_OUT0(0, 4);
	*((volatile unsigned long *)(DRIME4_VA_GLOBAL_CTRL + 0x00E8)) = 0x30843; // DRIME4_GPIO7(2)
	GPIO_SET_OUT0(7, 2);
	*((volatile unsigned long *)(DRIME4_VA_GLOBAL_CTRL + 0x00EC)) = 0x30843; // DRIME4_GPIO7(2)
	GPIO_SET_OUT0(7, 3);

	*((volatile unsigned long *)(DRIME4_VA_GLOBAL_CTRL + 0x0120)) = 0x30843; // DRIME4_GPIO9(0)
	GPIO_SET_OUT0(9, 0);

	*((volatile unsigned long *)(DRIME4_VA_GLOBAL_CTRL + 0x0124)) = 0x30843; // DRIME4_GPIO9(1)
	GPIO_SET_OUT0(9, 1);
	*((volatile unsigned long *)(DRIME4_VA_GLOBAL_CTRL + 0x0128)) = 0x30843; // DRIME4_GPIO9(2)
	GPIO_SET_OUT0(9, 2);
	*((volatile unsigned long *)(DRIME4_VA_GLOBAL_CTRL + 0x012C)) = 0x30843; // DRIME4_GPIO9(3)
	GPIO_SET_OUT0(9, 3);

	*((volatile unsigned long *)(DRIME4_VA_GLOBAL_CTRL + 0x01D8)) = 0x30843; // DRIME4_GPIO12(2)
	GPIO_SET_OUT0(12, 2);
	*((volatile unsigned long *)(DRIME4_VA_GLOBAL_CTRL + 0x01DC)) = 0x30843; // DRIME4_GPIO12(3)
	GPIO_SET_OUT0(12, 3);
	*((volatile unsigned long *)(DRIME4_VA_GLOBAL_CTRL + 0x01E0)) = 0x30843; // DRIME4_GPIO12(4)
	GPIO_SET_OUT0(12, 4);

	*((volatile unsigned long *)(DRIME4_VA_GLOBAL_CTRL + 0x03A4)) = 0x30843; // DRIME4_GPIO22(2)
	GPIO_SET_OUT0(22, 2);
	*((volatile unsigned long *)(DRIME4_VA_GLOBAL_CTRL + 0x03A8)) = 0x30843; // DRIME4_GPIO22(3)
	GPIO_SET_OUT0(22, 3);
	*((volatile unsigned long *)(DRIME4_VA_GLOBAL_CTRL + 0x03AC)) = 0x30843; // DRIME4_GPIO22(4)
	GPIO_SET_OUT0(22, 4);
	*((volatile unsigned long *)(DRIME4_VA_GLOBAL_CTRL + 0x03B0)) = 0x30843; // DRIME4_GPIO22(5)
	GPIO_SET_OUT0(22, 5);
	*((volatile unsigned long *)(DRIME4_VA_GLOBAL_CTRL + 0x03B4)) = 0x30843; // DRIME4_GPIO22(6)
	GPIO_SET_OUT0(22, 6);
	*((volatile unsigned long *)(DRIME4_VA_GLOBAL_CTRL + 0x03B8)) = 0x30843; // DRIME4_GPIO22(7)
	GPIO_SET_OUT0(22, 7);

}

MACHINE_START(D4ES, "Samsung-DRIMeIV-NX2000")
/* Maintainer: */
	.atag_offset	= 0x100,
	.map_io		= drime4_map_io,
	.init_early	= drime4_init_early,
	.init_irq	= drime4_init_irq,
	.handle_irq	= vic_handle_irq,
	.timer		= &drime4_timer,
	.init_machine	= drime4_d4es_init,
	.reserve	= &drime4_reserve,
//	.restart	= drime4_init_restart,

MACHINE_END
