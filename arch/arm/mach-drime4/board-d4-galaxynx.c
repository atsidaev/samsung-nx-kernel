/* linux/arch/arm/mach-drime4/board-d4-nx300.c
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
#include <linux/dma-contiguous.h>
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
#include <mach/adc.h>

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

#ifdef CONFIG_LEDS_GPIO
#include <linux/leds.h>
#endif

#ifdef CONFIG_SLAVE_PROTOCOL
#include <linux/spi/protocol_config.h>
#endif

/*------- LED --------*/
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
}


/**< 			D4 board 에서 Main LCD dislpay 를 위한 임시 코드 					   */
#ifdef CONFIG_DRIME4_LCD_PANEL_MANAGER
	#ifdef CONFIG_MLCD_1_PANEL_8
		struct d4_hs_spi_config mlcd_1_config = {
			.speed_hz = 20000000,
			.bpw = 9,
			.mode = SH_SPI_MODE_3,
			.waittime = 100,
			.ss_inven = D4_SPI_TRAN_INVERT_OFF,
			.spi_ttype = D4_SPI_TRAN_BURST_ON,
			.setup_select = SH_SPI_SPEED|SH_SPI_BPW|SH_SPI_WAVEMODE|SH_SPI_WAITTIME|SH_SPI_INVERT|SH_SPI_BURST
		};

		struct d4_lcd_hw_connect_info mlcd_1_hw_info = {
			.gpio_reset   	= DRIME4_GPIO0(7),
			.gpio_power_1 	= LPM_HW_CONNECT_NO_USE,
			.gpio_power_2 	= LPM_HW_CONNECT_NO_USE,
			.gpio_power_3 	= LPM_HW_CONNECT_NO_USE,
			.spi_ch			= 0,
			.spi_config		= &mlcd_1_config,
		};

		struct d4_lcd_panel_manager_data lcd_hw_conect_info = {
			.mlcd_1_hw_connect_info = &mlcd_1_hw_info,
			.mlcd_2_hw_connect_info = NULL,
			.slcd_hw_connect_info 	= NULL,
		};
	#endif

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

static struct resource drime4_dp_resource[] = {
	[0] = {
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
	},
};

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
/******************************************************************************/

/************ PMIC **************/
#ifdef CONFIG_BD9361
static struct bd9361_platform_data bd9361_data = {
	.pinmux_name = "I2C2",
	.sda_gpio = GPIO_EYE_SDA_I2C,
	.scl_gpio = GPIO_EYE_SCK_I2C,
};
#endif

#ifdef CONFIG_DRIME4_I2C2
static struct i2c_board_info i2c_devs2[] __initdata = {
	{
		I2C_BOARD_INFO("bd9361", 0x29),
		//.platform_data = &bd9361_data,
	},
};
#endif


#ifdef CONFIG_I2C_D4_AP2ISP_MASTER
static struct i2c_board_info i2c_devs0[] __initdata = {
	{
		I2C_BOARD_INFO("i2c-ap2isp-master", 0x1f),
	},
};
#endif

static struct i2c_board_info i2c_devs6[] __initdata = {
	{
		I2C_BOARD_INFO("bridgeic", 0x07),
	},
};


#if 0

static struct platform_device *d4es_devices[] __initdata = {
/* drime4 es board platform devices */
	&drime4_device_padmux,
	&drime4_asoc_dma,

#ifdef CONFIG_MMC_DW
#ifdef CONFIG_DRIME4_EMMC
	&drime4_device_emmc,
#endif
	&drime4_device_sdmmc,
#endif
#ifdef CONFIG_DRIME4_SDIO
	&drime4_device_sdio,
#endif
#ifdef CONFIG_MTD_NAND_DRIME4
	&drime4_device_nand,
#endif

	&drime4_device_i2s0,
	&drime4_device_i2s2,

#ifdef CONFIG_DRIME4_BMA
	&drime4_device_bma,
#endif

#ifdef CONFIG_DRIME4_SMA
	&drime4_device_sma,
#endif

#ifdef CONFIG_DRIME4_PP_CORE
	&drime4_device_pp_core,
#endif

#ifdef CONFIG_DRIME4_MIPI
		&drime4_device_mipi,
#endif

#ifdef CONFIG_DRIME4_PP_SSIF
	&drime4_device_pp_ssif,
#endif

#ifdef CONFIG_DRIME4_PP_3A
	&drime4_device_pp_3a,
#endif

#ifdef CONFIG_DRIME4_IPCM
	&drime4_device_ipcm,
#endif

#ifdef CONFIG_DRIME4_IPCS
	&drime4_device_ipcs,
#endif

#ifdef CONFIG_DRIME4_JPEG_XR
	&drime4_device_jxr,
#endif

#ifdef CONFIG_DRIME4_JPEG
	&drime4_device_jpeg,
#endif


#ifdef CONFIG_DRIME4_EP
	&drime4_device_ep,
#endif

#ifdef CONFIG_PMU_SELECT
	&drime4_device_opener,
#endif

#ifdef CONFIG_DRIME4_BE
	&drime4_device_be,
#endif

#ifdef CONFIG_VIDEO_MFC5X
	&drime4_device_mfc,
#endif

#ifdef CONFIG_DRIME4_BWM
	&drime4_device_bwm,
#endif

#ifdef CONFIG_DRIME4_SRP
		&drime4_device_srp,
#endif

#ifdef	CONFIG_DRIME4_SPI0
		&drime4_device_spi0,
#endif
#ifdef	CONFIG_DRIME4_SPI1
		&drime4_device_spi1,
#endif
#ifdef	CONFIG_DRIME4_SPI2
		&drime4_device_spi2,
#endif
#ifdef	CONFIG_DRIME4_SPI3
		&drime4_device_spi3,
#endif
#ifdef	CONFIG_DRIME4_SPI4
		&drime4_device_spi4,
#endif

#ifdef	CONFIG_DRIME4_SPI5
		&drime4_device_spi5,
#endif

#ifdef	CONFIG_DRIME4_SPI7
#if defined(CONFIG_CMOS_AQUILA_SENSOR)
		&drime4_device_spi7,
#endif
#endif

#ifdef CONFIG_PTC_0
	&d4_device_ptc[0],
#endif

#ifdef CONFIG_PTC_1
	&d4_device_ptc[1],
#endif

#ifdef CONFIG_HT_PWM0
		&d4_device_pwm[0],
#endif
#ifdef CONFIG_HT_PWM1
		&d4_device_pwm[1],
#endif
#ifdef CONFIG_HT_PWM2
		&d4_device_pwm[2],
#endif
#ifdef CONFIG_HT_PWM3
		&d4_device_pwm[3],
#endif
#ifdef CONFIG_HT_PWM4
		&d4_device_pwm[4],
#endif
#ifdef CONFIG_HT_PWM5
		&d4_device_pwm[5],
#endif
#ifdef CONFIG_HT_PWM6
		&d4_device_pwm[6],
#endif

#ifdef CONFIG_HT_PWM7
		&d4_device_pwm[7],
#endif

#ifdef CONFIG_HT_PWM8
		&d4_device_pwm[8],
#endif

#ifdef CONFIG_HT_PWM9
		&d4_device_pwm[9],
#endif
#ifdef CONFIG_HT_PWM10
		&d4_device_pwm[10],
#endif
#ifdef CONFIG_HT_PWM11
		&d4_device_pwm[11],
#endif
#ifdef CONFIG_HT_PWM12
		&d4_device_pwm[12],
#endif
#ifdef CONFIG_HT_PWM13
		&d4_device_pwm[13],
#endif
#ifdef CONFIG_HT_PWM14
		&d4_device_pwm[14],
#endif
#ifdef CONFIG_HT_PWM15
		&d4_device_pwm[15],
#endif

#ifdef CONFIG_HT_PWM16
		&d4_device_pwm[16],
#endif

#ifdef CONFIG_HT_PWM17
		&d4_device_pwm[17],
#endif

#ifdef CONFIG_DRIME4_DEV_RMU
		&drime4_device_rmu,
#endif

#ifdef CONFIG_DRIME4_ADC
		&drime4_device_adc,
#endif

#if defined(CONFIG_HT_PWM4)
		&d4_device_dr_ioctl,
#endif
#if defined(CONFIG_LEDS_GPIO)
		&drime4_gpio_leds,
#endif

#ifdef CONFIG_DRIME4_PMU
		&d4_device_pmu,
#endif

#ifdef CONFIG_DRIME4_UGPIO0
		&gpio_udevice[0],
#endif
#ifdef CONFIG_DRIME4_UGPIO1
		&gpio_udevice[1],
#endif
#ifdef CONFIG_DRIME4_UGPIO2
		&gpio_udevice[2],
#endif
#ifdef CONFIG_DRIME4_UGPIO3
		&gpio_udevice[3],
#endif
#ifdef CONFIG_DRIME4_UGPIO4
		&gpio_udevice[4],
#endif
#ifdef CONFIG_DRIME4_UGPIO5
		&gpio_udevice[5],
#endif
#ifdef CONFIG_DRIME4_UGPIO6
		&gpio_udevice[6],
#endif
#ifdef CONFIG_DRIME4_UGPIO7
		&gpio_udevice[7],
#endif
#ifdef CONFIG_DRIME4_UGPIO8
		&gpio_udevice[8],
#endif
#ifdef CONFIG_DRIME4_UGPIO9
		&gpio_udevice[9],
#endif
#ifdef CONFIG_DRIME4_UGPIO10
		&gpio_udevice[10],
#endif
#ifdef CONFIG_DRIME4_UGPIO11
		&gpio_udevice[11],
#endif
#ifdef CONFIG_DRIME4_UGPIO12
		&gpio_udevice[12],
#endif
#ifdef CONFIG_DRIME4_UGPIO13
		&gpio_udevice[13],
#endif
#ifdef CONFIG_DRIME4_UGPIO14
		&gpio_udevice[14],
#endif
#ifdef CONFIG_DRIME4_UGPIO15
		&gpio_udevice[15],
#endif
#ifdef CONFIG_DRIME4_UGPIO16
		&gpio_udevice[16],
#endif
#ifdef CONFIG_DRIME4_UGPIO17
		&gpio_udevice[17],
#endif
#ifdef CONFIG_DRIME4_UGPIO18
		&gpio_udevice[18],
#endif
#ifdef CONFIG_DRIME4_UGPIO19
		&gpio_udevice[19],
#endif
#ifdef CONFIG_DRIME4_UGPIO20
		&gpio_udevice[20],
#endif
#ifdef CONFIG_DRIME4_UGPIO21
		&gpio_udevice[21],
#endif
#ifdef CONFIG_DRIME4_UGPIO22
		&gpio_udevice[22],
#endif
#ifdef CONFIG_DRIME4_UGPIO23
		&gpio_udevice[23],
#endif
#ifdef CONFIG_DRIME4_UGPIO24
		&gpio_udevice[24],
#endif
#ifdef CONFIG_DRIME4_UGPIO25
		&gpio_udevice[25],
#endif
#ifdef CONFIG_DRIME4_UGPIO26
		&gpio_udevice[26],
#endif
#ifdef CONFIG_DRIME4_UGPIO27
		&gpio_udevice[27],
#endif
#ifdef CONFIG_DRIME4_RTC
		&drime4_device_rtc,
#endif

#ifdef CONFIG_DRIME4_SI2C0
		&drime4_device_si2c[0],
#endif
#ifdef CONFIG_DRIME4_SI2C1
		&drime4_device_si2c[1],
#endif
#ifdef CONFIG_DRIME4_SI2C2
		&drime4_device_si2c[2],
#endif
#ifdef CONFIG_DRIME4_SI2C3
		&drime4_device_si2c[3],
#endif
#ifdef CONFIG_DRIME4_SI2C4
		&drime4_device_si2c[4],
#endif
#ifdef CONFIG_DRIME4_SI2C5
		&drime4_device_si2c[5],
#endif
#ifdef CONFIG_DRIME4_SI2C6
		&drime4_device_si2c[6],
#endif
};
#endif

/* PWM (CH0 ~ CH17) */

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


static struct platform_device si2c_device0 = {.name = "si2cdev", .id = 0,};
static struct platform_device si2c_device4 = {.name = "si2cdev", .id = 4,};

/* SPI (CH0 ~ CH7) */

static struct platform_device spi_udd_device0 = {.name = "hs_spidev", .id = 0,};
static struct platform_device spi_udd_device1 = {.name = "hs_spidev", .id = 1,};
static struct platform_device spi_udd_device2 = {.name = "hs_spidev", .id = 2,};
static struct platform_device spi_udd_device3 = {.name = "hs_spidev", .id = 3,};
static struct platform_device spi_udd_device4 = {.name = "hs_spidev", .id = 4,};
static struct platform_device spi_udd_device5 = {.name = "hs_spidev", .id = 5,};
static struct platform_device spi_udd_device6 = {.name = "hs_spidev", .id = 6,};
static struct platform_device spi_udd_device7 = {.name = "hs_spidev", .id = 7,};


#ifdef CONFIG_SLAVE_PROTOCOL

static struct protocol_platform_data pt_config = {
	.int_pin_num = -1,
	.protocol_ch = 1,

};
#endif

#ifdef CONFIG_NXLENS_COMM_MAN
static struct platform_device lens_device = {.name = "lens_comm", .id = -1,};
#endif

#ifdef CONFIG_INT_STR_DEV
static struct platform_device int_str_device = {.name = "intstr", .id = -1,};
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
	PIN_MAP_MUX_GROUP_DEFAULT("dw_mmc_sdcard.1", PINCTRL_DEV, NULL, "sd0"),
#else
	PIN_MAP_MUX_GROUP_DEFAULT("dw_mmc_sdcard.0", PINCTRL_DEV, NULL, "sd0"),
#endif
#ifdef CONFIG_DRIME4_SI2C0
	PIN_MAP_MUX_GROUP_DEFAULT("drime4-si2c.0", PINCTRL_DEV, NULL, "i2c0"),
#else
	PIN_MAP_MUX_GROUP_DEFAULT("drime4-i2c.0", PINCTRL_DEV, NULL, "i2c0"),
#endif
	PIN_MAP_MUX_GROUP_DEFAULT("drime4-i2c.1", PINCTRL_DEV, NULL, "i2c1"),
	PIN_MAP_MUX_GROUP_DEFAULT("drime4-i2c.2", PINCTRL_DEV, NULL, "i2c2"),
	PIN_MAP_MUX_GROUP_DEFAULT("drime4-i2c.3", PINCTRL_DEV, NULL, "i2c3"),
#ifdef CONFIG_DRIME4_SI2C4
	PIN_MAP_MUX_GROUP_DEFAULT("drime4-si2c.4", PINCTRL_DEV, NULL, "i2c4"),
#else
	PIN_MAP_MUX_GROUP_DEFAULT("drime4-i2c.4", PINCTRL_DEV, NULL, "i2c6"),
#endif
	/*PIN_MAP_MUX_GROUP_DEFAULT("drime4-i2c.5", PINCTRL_DEV, NULL, "i2c5"), */
	/*PIN_MAP_MUX_GROUP_DEFAULT("drime4-i2c.6", PINCTRL_DEV, NULL, "i2c6"),*/
	/*PIN_MAP_MUX_GROUP_DEFAULT("drime4-i2c.7", PINCTRL_DEV, NULL, "i2c7"), */
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
//	PIN_MAP_MUX_GROUP_DEFAULT("dw_mmc_sdio.0", PINCTRL_DEV, NULL, "sd1"),
#endif

	PIN_MAP_MUX_GROUP_DEFAULT("drime4-ptc.0", PINCTRL_DEV, NULL, "ptc0"),
	PIN_MAP_MUX_GROUP_DEFAULT("drime4-ptc.1", PINCTRL_DEV, NULL, "ptc1"),

	PIN_MAP_MUX_GROUP_DEFAULT("drime4_spi.0", PINCTRL_DEV, NULL, "spi0"),
	PIN_MAP_MUX_GROUP_DEFAULT("drime4_spi.1", PINCTRL_DEV, NULL, "spi1"),
	PIN_MAP_MUX_GROUP_DEFAULT("drime4_spi.2", PINCTRL_DEV, NULL, "spi2"),
	PIN_MAP_MUX_GROUP_DEFAULT("drime4_spi.3", PINCTRL_DEV, NULL, "spi3"),
	PIN_MAP_MUX_GROUP_DEFAULT("drime4_spi.4", PINCTRL_DEV, NULL, "spi4"),
	PIN_MAP_MUX_GROUP_DEFAULT("drime4_spi.5", PINCTRL_DEV, NULL, "spi5"),
/*

	PIN_MAP_MUX_GROUP_DEFAULT("drime4_spi.6", PINCTRL_DEV, NULL, "spi6"),
	PIN_MAP_MUX_GROUP_DEFAULT("drime4_spi.7", PINCTRL_DEV, NULL, "spi7"),

*/
	PIN_MAP_MUX_GROUP_DEFAULT("drime4_spi.7", PINCTRL_DEV, NULL, "spi9"),
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

#ifdef CONFIG_DRM_DRIME4_DP_SUBLCD
	PIN_MAP_MUX_GROUP_DEFAULT("drime4_lcd", PINCTRL_DEV, NULL, "slcd"),
#endif
	/*
	PIN_MAP_MUX_GROUP_DEFAULT("uart1", PINCTRL_DEV, NULL, "uart1"),
	*/
	PIN_MAP_MUX_GROUP_DEFAULT("d4_pp_ssif.0", PINCTRL_DEV, NULL, "slvds"),
#ifdef CONFIG_DRIME4_SRP
	PIN_MAP_MUX_GROUP_DEFAULT("d4_srp", PINCTRL_DEV, NULL, "srp1"),
#endif
};

static void __init drime4_init_i2c(void)
{
#ifdef CONFIG_DRIME4_I2C0
	i2c_register_board_info(0, i2c_devs0, ARRAY_SIZE(i2c_devs0));
	drime4_register_i2c(0, NULL);
#endif

#ifdef CONFIG_DRIME4_I2C1
	i2c_register_board_info(1, i2c_devs1, ARRAY_SIZE(i2c_devs1));
	drime4_register_i2c(1, NULL);
#endif

#ifdef CONFIG_DRIME4_I2C2
	i2c_register_board_info(2, i2c_devs2, ARRAY_SIZE(i2c_devs2));
	drime4_register_i2c(2, NULL);
#endif

#ifdef CONFIG_DRIME4_I2C6
	i2c_register_board_info(4, i2c_devs6, ARRAY_SIZE(i2c_devs6));
	drime4_register_i2c(4, NULL);
#endif

#ifdef CONFIG_DRIME4_SI2C0
	platform_device_register(&si2c_device0);
	platform_device_register(&drime4_device_si2c[0]);
#endif

#ifdef CONFIG_DRIME4_SI2C4
	platform_device_register(&si2c_device4);
	platform_device_register(&drime4_device_si2c[4]);
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

/**< 			D4 board 에서 Main LCD dislpay 를 위한 임시 코드 					   */
void __init drime4_fb_init(void)
{
#ifdef CONFIG_FB_DRIME4
	drime4_device_fb.dev.platform_data = (void *)&fb_pd;
#endif
}
/******************************************************************************/

 static void drime4_poweroff(void)
{
	struct d4_rmu_device * rmu;

	rmu = d4_rmu_request();
	d4_pcu_hold_set(rmu);
	d4_pcu_off_set(rmu);

	d4_rmu_release(rmu);

/*	arm_machine_restart('s', NULL); */
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
	EBdVersion bd_ver;
	VersionInfo_Initialize();
	bd_ver = GetBoardVersion();

	d4_led_init_gpio();


	pinctrl_register_mappings(drime4_padmux_map, ARRAY_SIZE(drime4_padmux_map));

#ifdef CONFIG_PM
	drime4_pm_init();
#endif
	drime4_init_common_devices();
	drime4_init_i2c();

/**< 			D4 board 에서 Main LCD dislpay 를 위한 임시 코드 					   */
#ifdef CONFIG_FB_DRIME4
	drime4_fb_init();
#endif
/******************************************************************************/

	drime4_nand_set_platdata(&drime4_nand_info);		/* CONFIG_MTD_NAND_DRIME4 */

	/*   Register PWM (CH0 ~ CH17)
	 *   User DD
	 */
	platform_device_register(&pwmdev_device0);
	platform_device_register(&pwmdev_device1);
	platform_device_register(&pwmdev_device2);
	platform_device_register(&pwmdev_device3);
	platform_device_register(&pwmdev_device4);

	if (g_eBoardVersion >= eBdVer_ES2) {
		platform_device_register(&pwmdev_device5);
	}

	platform_device_register(&pwmdev_device7);
	platform_device_register(&pwmdev_device8);

	/*   Register SPI (CH0 ~ CH7)
	 *   User DD
	 */

	if (g_eBoardVersion == eBdVer_ES2) {
		platform_device_register(&spi_udd_device0);
		platform_device_register(&spi_udd_device3);
	}
	platform_device_register(&spi_udd_device1);
	platform_device_register(&spi_udd_device2);
	platform_device_register(&spi_udd_device4);
	platform_device_register(&spi_udd_device5);
	platform_device_register(&spi_udd_device7);

	platform_device_register(&ssif_iodev);
#ifdef CONFIG_NXLENS_COMM_MAN
	platform_device_register(&lens_device);
#endif
#ifdef CONFIG_INT_STR_DEV
	platform_device_register(&int_str_device);
#endif
	platform_device_register(&drime4_device_padmux);
	platform_device_register(&drime4_asoc_dma);
	platform_device_register(&drime4_device_sdmmc);
	/*
	platform_device_register(&drime4_device_emmc);
	platform_device_register(&drime4_device_sdio);
	*/
	platform_device_register(&drime4_device_nand);
	platform_device_register(&drime4_device_i2s0);
	platform_device_register(&drime4_device_i2s2);
	platform_device_register(&drime4_device_bma);
	platform_device_register(&drime4_device_sma);
	platform_device_register(&drime4_device_pp_core);
	platform_device_register(&drime4_device_mipi);
	platform_device_register(&drime4_device_pp_ssif);
	platform_device_register(&drime4_device_pp_3a);
	platform_device_register(&drime4_device_ipcm);
	platform_device_register(&drime4_device_ipcs);
	/*
	platform_device_register(&drime4_device_jxr);
	*/
	platform_device_register(&drime4_device_jpeg);
	platform_device_register(&drime4_device_ep);

#ifdef CONFIG_PMU_SELECT
	platform_device_register(&drime4_device_opener);
#endif
	platform_device_register(&drime4_device_be);
	platform_device_register(&drime4_device_mfc);
	platform_device_register(&drime4_device_bwm);
	platform_device_register(&drime4_device_srp);

	if (g_eBoardVersion >= eBdVer_ES2) {
		platform_device_register(&drime4_device_spi0);
		platform_device_register(&drime4_device_spi3);
	}
	platform_device_register(&drime4_device_spi1);
	platform_device_register(&drime4_device_spi2);
	platform_device_register(&drime4_device_spi4);
	platform_device_register(&drime4_device_spi5);

	/*
	platform_device_register(&drime4_device_spi6);
	platform_device_register(&drime4_device_spi7);
	*/


	/*
	platform_device_register(&powervr_device);
	*/

	platform_device_register(&d4_device_pwm[0]);
	platform_device_register(&d4_device_pwm[1]);
	platform_device_register(&d4_device_pwm[2]);
	platform_device_register(&d4_device_pwm[3]);
	platform_device_register(&d4_device_pwm[4]);

	if (g_eBoardVersion >= eBdVer_ES2) {
		platform_device_register(&d4_device_pwm[5]);
	}


	platform_device_register(&d4_device_pwm[7]);
	platform_device_register(&d4_device_pwm[12]);

	platform_device_register(&d4_device_pwm[8]);
	/*
	platform_device_register(&d4_device_pwm[9]);
	platform_device_register(&d4_device_pwm[10]);
	platform_device_register(&d4_device_pwm[11]);

	platform_device_register(&d4_device_pwm[13]);
	platform_device_register(&d4_device_pwm[14]);
	platform_device_register(&d4_device_pwm[15]);
	platform_device_register(&d4_device_pwm[16]);
	platform_device_register(&d4_device_pwm[17]);
	 */

	platform_device_register(&drime4_device_rmu);
	platform_device_register(&drime4_device_adc);
	platform_device_register(&d4_device_dr_ioctl);
	platform_device_register(&drime4_gpio_leds);
	platform_device_register(&d4_device_pmu);


	/*   Register GPIO (G0 ~ G27)
	 *   User DD
	 */
	platform_device_register(&gpio_udevice[0]);
	platform_device_register(&gpio_udevice[1]);
	platform_device_register(&gpio_udevice[2]);
	platform_device_register(&gpio_udevice[3]);
	platform_device_register(&gpio_udevice[4]);
	platform_device_register(&gpio_udevice[5]);
	platform_device_register(&gpio_udevice[6]);
	platform_device_register(&gpio_udevice[7]);
	platform_device_register(&gpio_udevice[8]);
	platform_device_register(&gpio_udevice[9]);
	platform_device_register(&gpio_udevice[10]);
	platform_device_register(&gpio_udevice[11]);
	platform_device_register(&gpio_udevice[12]);
	platform_device_register(&gpio_udevice[13]);
	platform_device_register(&gpio_udevice[14]);
	platform_device_register(&gpio_udevice[15]);
	platform_device_register(&gpio_udevice[16]);
	platform_device_register(&gpio_udevice[17]);
	platform_device_register(&gpio_udevice[18]);
	platform_device_register(&gpio_udevice[19]);
	platform_device_register(&gpio_udevice[20]);
	platform_device_register(&gpio_udevice[21]);
	platform_device_register(&gpio_udevice[22]);
	platform_device_register(&gpio_udevice[23]);
	platform_device_register(&gpio_udevice[24]);
	platform_device_register(&gpio_udevice[25]);
	platform_device_register(&gpio_udevice[26]);
	platform_device_register(&gpio_udevice[27]);

	platform_device_register(&drime4_device_rtc);

/**< 			D4 board 에서 Main LCD dislpay 를 위한 임시 코드 					   */
#ifdef CONFIG_DRIME4_LCD_PANEL_MANAGER
	platform_device_register(&drime4_device_lcd_panel_manager);
#endif

#ifdef CONFIG_DRM_DRIME4
	platform_device_register(&drime4_device_drm);
#endif

#ifdef CONFIG_DRM_DRIME4_DP_LCD
	platform_device_register(&drime4_device_lcd);
#endif
#ifdef CONFIG_DRM_DRIME4_DP_SUBLCD
	platform_device_register(&drime4_device_sublcd);
#endif

#ifdef CONFIG_DRIME4_CSM
	platform_device_register(&drime4_device_csm);							/* CONFIG_DRIME4_CSM */
#endif

/******************************************************************************/
	pm_power_off = drime4_poweroff;
	
	/*
	Initialize unused i/o as GPIO to avoid outside hardware signal effect.
	 */

	/*
	pinmux_request_gpio(DRIME4_GPIO15(5));
	pinmux_request_gpio(DRIME4_GPIO15(6));
	 */

	*((volatile unsigned long *)(DRIME4_VA_GLOBAL_CTRL + 0x0010)) = 0x30843; /* DRIME4_GPIO0(4) */
	GPIO_SET_OUT0(0, 4);
	*((volatile unsigned long *)(DRIME4_VA_GLOBAL_CTRL + 0x00E8)) = 0x30843; /* DRIME4_GPIO7(2) */
	GPIO_SET_OUT0(7, 2);
	*((volatile unsigned long *)(DRIME4_VA_GLOBAL_CTRL + 0x00EC)) = 0x30843; /* DRIME4_GPIO7(2)*/
	GPIO_SET_OUT0(7, 3);

	*((volatile unsigned long *)(DRIME4_VA_GLOBAL_CTRL + 0x0120)) = 0x30843; /* DRIME4_GPIO9(0)*/
	GPIO_SET_OUT0(9, 0);

	*((volatile unsigned long *)(DRIME4_VA_GLOBAL_CTRL + 0x0124)) = 0x30843; /* DRIME4_GPIO9(1)*/
	GPIO_SET_OUT0(9, 1);
	*((volatile unsigned long *)(DRIME4_VA_GLOBAL_CTRL + 0x0128)) = 0x30843; /* DRIME4_GPIO9(2)*/
	GPIO_SET_OUT0(9, 2);
	*((volatile unsigned long *)(DRIME4_VA_GLOBAL_CTRL + 0x012C)) = 0x30843; /* DRIME4_GPIO9(3)*/
	GPIO_SET_OUT0(9, 3);

	if (bd_ver > eBdVer_ES1)
{
		*((volatile unsigned long *)(DRIME4_VA_GLOBAL_CTRL + 0x01D8)) = 0x30843; /* DRIME4_GPIO12(2)*/
		GPIO_SET_OUT0(12, 2);
		*((volatile unsigned long *)(DRIME4_VA_GLOBAL_CTRL + 0x01DC)) = 0x30843; /* DRIME4_GPIO12(3)*/
		GPIO_SET_OUT0(12, 3);
	}
	*((volatile unsigned long *)(DRIME4_VA_GLOBAL_CTRL + 0x01E0)) = 0x30843; /* DRIME4_GPIO12(4)*/
	GPIO_SET_OUT0(12, 4);


	*((volatile unsigned long *)(DRIME4_VA_GLOBAL_CTRL + 0x0244)) = 0x30843; /* DRIME4_GPIO15(5)*/
	GPIO_SET_OUT0(15, 5);
	*((volatile unsigned long *)(DRIME4_VA_GLOBAL_CTRL + 0x0248)) = 0x30843; /* DRIME4_GPIO15(6)*/
	GPIO_SET_OUT0(15, 6);

	*((volatile unsigned long *)(DRIME4_VA_GLOBAL_CTRL + 0x03A4)) = 0x30843; /* DRIME4_GPIO22(2)*/
	GPIO_SET_OUT0(22, 2);
	*((volatile unsigned long *)(DRIME4_VA_GLOBAL_CTRL + 0x03A8)) = 0x30843; /* DRIME4_GPIO22(3)*/
	GPIO_SET_OUT0(22, 3);
	*((volatile unsigned long *)(DRIME4_VA_GLOBAL_CTRL + 0x03AC)) = 0x30843; /* DRIME4_GPIO22(4)*/
	GPIO_SET_OUT0(22, 4);
	*((volatile unsigned long *)(DRIME4_VA_GLOBAL_CTRL + 0x03B0)) = 0x30843; /* DRIME4_GPIO22(5)*/
	GPIO_SET_OUT0(22, 5);
	*((volatile unsigned long *)(DRIME4_VA_GLOBAL_CTRL + 0x03B4)) = 0x30843; /* DRIME4_GPIO22(6)*/
	GPIO_SET_OUT0(22, 6);
	*((volatile unsigned long *)(DRIME4_VA_GLOBAL_CTRL + 0x03B8)) = 0x30843; /* DRIME4_GPIO22(7)*/
	GPIO_SET_OUT0(22, 7);

#ifdef CONFIG_SLAVE_PROTOCOL
	protocol_data_register(&pt_config);
#endif
	/*card detection pull up*/
	*(volatile unsigned int *)(DRIME4_VA_GLOBAL_CTRL + 0x014C) = 0x1014F;
}





MACHINE_START(D4ES, "Samsung-DRIMeIV-GALAXYNX")
/* Maintainer: */
	.atag_offset	= 0x100,
	.map_io		= drime4_map_io,
	.init_early	= drime4_init_early,
	.init_irq	= drime4_init_irq,
	.handle_irq	= vic_handle_irq,
	.timer		= &drime4_timer,
	.init_machine	= drime4_d4es_init,
	.reserve	= &drime4_reserve,
/*	.restart	= drime4_init_restart,*/

MACHINE_END
