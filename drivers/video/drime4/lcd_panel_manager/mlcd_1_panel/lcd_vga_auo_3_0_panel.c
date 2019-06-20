/**
 * @file lcd_vga_auo_3_0panel.c
 * @brief AUO VGA 3.0 AMOLED LCD panel driver.
 * @author Jo Yonghoon <yh0302.jo@samsung.com>
 * 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/wait.h>
#include <linux/fb.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/gpio.h>
#include <linux/d4_ht_pwm.h>
#include <linux/pinctrl/pinconf-drime4.h>
#include <linux/pinctrl/pinconf.h>
#include <linux/pinctrl/consumer.h>
#include <mach/version_information.h>

#include "../../lcd_panel_manager/d4_lcd_panel_manager_ctrl.h"

/* Private variable */

/**< define for LCD setting */
#define SLEEPMSEC		0x10
#define ENDDEF			0xFE
#define	DEFMASK			0xFF

/**< define for LCD brightness setting */

#define VGA_AUO_30_DARK         	2400000	// 60%
#define VGA_AUO_30_NORMAL_DARK   	1800000	// 70%
#define VGA_AUO_30_NORMAL        	1200000 // 80% 
#define VGA_AUO_30_BRIGHT_NORMAL	600000  // 90%
#define VGA_AUO_30_BRIGHT        	0       // 100%


/**< LCD panel control value */
static const unsigned char panel_display_ctrl_table[] = {
	0x0A, 0x03, 0x01, 0xF8, 0x02, 0x51, 0x03, 0x1B, 0x04, 0x40, 0x05, 0x40,
	0x06, 0x40, 0x07, 0x40, 0x08, 0x40, 0x09, 0x40, 0x0B, 0x77, 0x0C, 0xCC,
	0x0D, 0xCC, 0x0E, 0xAC, 0x0F, 0x8A, 0x10, 0x0A, 0x11, 0xCC, 0x12, 0xCC,
	0x13, 0xAC, 0x14, 0x8A, 0x15, 0x0A, 0x16, 0xCC, 0x17, 0xCC, 0x18, 0xAC,
	0x19, 0x8A, 0x1A, 0x0A, ENDDEF, 0x00
};

static const unsigned char panel_standby_off_table[] = {
	0x0A, 0x03, ENDDEF, 0x00
};

static const unsigned char panel_standby_on_table[] = {
	0x0A, 0x02, ENDDEF, 0x00
};

static const unsigned char panel_power_on_table[] = {
	0x0A, 0x03, ENDDEF, 0x00
};

static const unsigned char panel_power_off_table[] = {
	0x0A, 0x02, ENDDEF, 0x00
};


/* Private function */
static int mlcd_1_set_table_value(const unsigned char *wbuf);

/**< LCD panel control functions */
static void mlcd_1_ctrl_power_on(void);
static void mlcd_1_ctrl_reset(void);
static void mlcd_1_ctrl_init(void);
static void mlcd_1_ctrl_display_on(void);
static void mlcd_1_ctrl_display_off(void);
static void mlcd_1_ctrl_standby_on(void);
static void mlcd_1_ctrl_standby_off(void);
static void mlcd_1_ctrl_power_off(void);
static void mlcd_1_ctrl_brightness(int level);
static void mlcd_1_ctrl_extension(void *ext_info);

/******************************************************************************/
/*                     Main LCD 1 Panel Information                           */
/******************************************************************************/

static struct lcd_panel_hw_info mlcd_1_panel_hw_info = {
 /**< width/ height/   LCD clock/       LCD ratio/  dot array type */
		640,	480,	27000000,	LPM_RATIO_4_3,	LPM_STRIPE,
 /**< dot even seq/ dot odd seq/ output format/ output bit-width */
		LPM_BGR,	LPM_BGR,	LPM_RGB_OUT,	LPM_BIT_WIDTH_24
};

static struct lcd_panel_timing_info mlcd_1_panel_timing_info = {
 /**< total h/  total v/  h rising/ h falling/ v rising/ v falling */
		762,		525,		0,		40,			0,		9,
 /**< read h start/ enable h start/ enable h size/ enable v start/ enable v size */
		73,			81,			640,			27,			480,
 /**< inv_dot clk/ inv_enable/ inv_hsync/ inv_vsync */
	  LPM_ON,		LPM_OFF,	LPM_ON,		LPM_ON
};

static struct d4_hs_spi_config spi_config = {
	.speed_hz = 5000000,
	.bpw = 16,
	.mode = SH_SPI_MODE_3,
	.waittime = 100,
	.ss_inven = D4_SPI_TRAN_INVERT_OFF,
	.spi_ttype = D4_SPI_TRAN_BURST_OFF,
	.setup_select = SH_SPI_SPEED|SH_SPI_BPW|SH_SPI_WAVEMODE|SH_SPI_WAITTIME|SH_SPI_INVERT|SH_SPI_BURST
};

static struct lcd_panel_ctrl_func mlcd_1_panel_ctrl_func = {

		/* Basic Function */

		mlcd_1_ctrl_reset, 			/**< LCD reset function */

		mlcd_1_ctrl_power_on,		/**< LCD power on function */
		mlcd_1_ctrl_power_off,		/**< LCD power off function */
		mlcd_1_ctrl_init,			/**< LCD initialization function */

		mlcd_1_ctrl_display_on,		/**< LCD light on  function, including back light on */
		mlcd_1_ctrl_display_off,	/**< LCD light off function, including back light off */

		mlcd_1_ctrl_brightness,		/**< LCD panel brightness setting function */

		/* Optional Function */

		mlcd_1_ctrl_standby_on,     /**< LCD stand-by on Function */
		mlcd_1_ctrl_standby_off,   	/**< LCD stand-by off Function */

		NULL,	    				/**< LCD horizontal flip on function */
		NULL,	    				/**< LCD horizontal flip off function */

		NULL,	    				/**< LCD vertical flip on function */
		NULL,	    				/**< LCD vertical flip off function */

		NULL,       				/**< LCD Rotation control function */
		NULL,  						/**< LCD panel brightness gamma setting function */
		NULL,				    	/**< LCD CSC control function */
		NULL,	    				/**< LCD TCC control function */
		mlcd_1_ctrl_extension,		/**< LCD extension control function */

		NULL,						/**< suspend function for power management */
		NULL						/**< resume  function for power management */
};

static struct lcd_panel_manager_info mlcd_1_panel_info = {
    &mlcd_1_panel_hw_info,
    &mlcd_1_panel_timing_info,
    &mlcd_1_panel_ctrl_func,
    " VGA AUO 3.0 Inch Panel "
};

/******************************************************************************/
/*                  Main LCD 1 Panel Control Functions                        */
/******************************************************************************/

static void mlcd_1_ctrl_power_on(void)
{
	/*
	 * GPIO_DISP_3_3V_ON
	 */
	mlcd_1_panel_set_gpio(LPM_GPIO_POWER_1, LPM_ON);	
	mdelay(10);
}

struct ht_pwm_device *pwm13;

struct ht_pwm_conf_info pwm_setup = {
		.opmode = CONTINUE_PULSE,
		.trigtype = SW_TRIGGER,
		.freq1.period = 6000000,
		.freq1.duty = 0,
		.freq1.count = 0	
};

static bool pwm_request_state = false;

void mlcd_1_ctrl_bl_on(void)
{
	/*
	 * LCD_BL_ON
	 */
	
	if (pwm_request_state == false) {
		pwm13 = d4_ht_pwm_request(13, "pwmdev");
		if ((pwm13 == -16) || (pwm13 == -2))
			return;
		pwm_request_state = true;
   }

	d4_ht_pwm_config(pwm13, &pwm_setup);
	d4_ht_pwm_enable(pwm13);
}

static void mlcd_1_ctrl_reset(void)
{
	/*
	 * GPIO_LCD_nRESET
	 */
	mlcd_1_panel_set_gpio(LPM_GPIO_RESET, LPM_ON);
	mdelay(1);
	mlcd_1_panel_set_gpio(LPM_GPIO_RESET, LPM_OFF);
	mdelay(10);
	mlcd_1_panel_set_gpio(LPM_GPIO_RESET, LPM_ON);
	mdelay(10);

	/*
	 * LCD_BL_ON
	 */

	mlcd_1_ctrl_bl_on();
}




static void mlcd_1_ctrl_init(void)
{
	/**< Main LCD1 SPI init */
	mlcd_1_panel_spi_init();
	mlcd_1_set_table_value(panel_display_ctrl_table);
	mdelay(10);
}

static void mlcd_1_ctrl_standby_on(void)
{
	mlcd_1_set_table_value(panel_standby_on_table);
}

static void mlcd_1_ctrl_standby_off(void)
{
	mlcd_1_set_table_value(panel_standby_off_table);
}

static void mlcd_1_ctrl_display_on(void)
{
	d4_ht_pwm_enable(pwm13);
}

static void mlcd_1_ctrl_display_off(void)
{
	d4_ht_pwm_disable(pwm13);
}

static void mlcd_1_ctrl_power_off(void)
{
	mlcd_1_set_table_value(panel_power_off_table);
	mlcd_1_panel_set_gpio(LPM_GPIO_POWER_1, LPM_OFF);	
	mdelay(10);
}

static void mlcd_1_ctrl_brightness(int level)
{
	switch(level)
	{
		case LPM_DARK_LEVEL:
			pwm_setup.freq1.duty = VGA_AUO_30_DARK;
			break;
		case LPM_NORMAL_DARK_LEVEL:
			pwm_setup.freq1.duty = VGA_AUO_30_NORMAL_DARK;
			break;
		case LPM_NORMAL_LEVEL:
			pwm_setup.freq1.duty = VGA_AUO_30_NORMAL;
			break;
		case LPM_NORMAL_BRIGHT_LEVEL:
			pwm_setup.freq1.duty = VGA_AUO_30_BRIGHT_NORMAL;
			break;
		case LPM_BRIGHT_LEVEL:
			pwm_setup.freq1.duty = VGA_AUO_30_BRIGHT;
			break;
		default:
			break;
	}
	
	d4_ht_pwm_disable(pwm13);
	d4_ht_pwm_config(pwm13, &pwm_setup);
	d4_ht_pwm_enable(pwm13);	
}

static void mlcd_1_ctrl_extension(void *ext_info)
{
	struct lcd_panel_ext_ctrl_info *info;

	info = (struct lcd_panel_ext_ctrl_info *)ext_info;
	printk("Info1: %#010x, Info2: %#010x \n", info->ctrl_info_1, info->ctrl_info_2);
	printk("Comment: %s \n", info->comment);
}

/******************************************************************************/
/*                  		    Private Functions                        	  */
/******************************************************************************/

static int mlcd_1_set_table_value(const unsigned char *wbuf)
{
	int ret = 0, i = 0;
	unsigned short spi_buffer = 0;
	struct lcd_panel_spi_write_info spi_set_info;

	while ((wbuf[i] & DEFMASK) != ENDDEF) {
		if ((wbuf[i] & DEFMASK) != SLEEPMSEC) {
			spi_buffer = (unsigned short)((wbuf[i] << 8) | wbuf[i + 1]);

			spi_set_info.data_cnt = 1;
			spi_set_info.data_buf = (void *)&spi_buffer;

			ret = mlcd_1_panel_spi_write(&spi_set_info);
			udelay(10);
			if (ret < 0) {
				return ret;
			}

		} else {
			udelay(wbuf[i + 1] * 1000);

		}
		i += 2;

	}
	return ret;
}

static int mlcd_1_register(void)
{
	unsigned long pin_config = 0;
	int ret = -1;

	/**< Data strength�? ?�이�??�한 ?�시 코드, for NX1 proto board */
	pin_config = to_config_packed(PIN_CONFIG_DRIVE_STRENGTH,
			PIN_CONFIG_DRIVE_STRENGTH_X6);
	ret = pin_config_group_set("drime4-pinmux", "mlcdcon", pin_config);
	if (ret < 0) {
		printk("Data strength control: Fail \n");
		return ret;
	}

	mlcd_1_panel_set_spi_init_config(&spi_config);

	if (mlcd_1_panel_set_info(&mlcd_1_panel_info) < 0) {
		return -EINVAL;
	}

	printk("Main LCD 1 panel register \n");

#if defined(CONFIG_DRM_DRIME4)		/* for drm test */
	mlcd_panel_select(LPM_MLCD_1_SELECT);
	mlcd_panel_power_on();
	mlcd_panel_reset();
	mlcd_panel_init();
	mlcd_panel_light_on();
#endif
	
	return 0;
}

static void mlcd_1_unregister(void)
{
	printk("Main LCD 2 panel unregister \n");
}

#ifndef CONFIG_SCORE_FAST_RESUME
module_init(mlcd_1_register);
#else
fast_late_initcall(mlcd_1_register);
#endif
module_exit(mlcd_1_unregister);



