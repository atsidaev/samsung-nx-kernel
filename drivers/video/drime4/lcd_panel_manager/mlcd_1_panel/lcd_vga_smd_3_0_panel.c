/**
 * @file lcd_vga_cmd_3_0panel.c
 * @brief SMD VGA 3.0 AMOLED LCD panel driver.
 * @author Kim Sunghoon <bluesay.kim@samsung.com>
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
#include <mach/version_information.h>

#include "../../lcd_panel_manager/d4_lcd_panel_manager_ctrl.h"

/* Private variable */

/**< define for LCD setting */
#define SLEEPMSEC		0x10
#define ENDDEF			0x20
#define	DEFMASK			0xFF

/**< LCD panel control value */
static const unsigned char panel_display_ctrl_table[] = {
		0x00, 0xA4, 0x01, 0xA0, 0x02, 0xA0, 0x03, 0xA0, 0x04, 0xA1, 0x05, /*0x7A*/0x96,
		0x06, 0x26, 0x07, 0x07, 0x08, 0xA0, 0x09, 0xA0, /* CM0,CM1 :0,0 24bit- RGB Interface */
		0xA5, 0x19, 0xA6, 0x14, ENDDEF, 0x00
};

static const unsigned char panel_power_ctrl_table[] = {
		0x0A, 0xA1, 0x0B, 0x22, 0x0C, 0x33, 0x0D, 0xA3, 0x0E, 0xA1, 0x0F, 0x6A,
		0x10, 0x72, 0x11, 0xA3, 0x12, 0xA3, 0x13, 0x77, ENDDEF, 0x00
};

static const unsigned char panel_timing_ctrl_table[] = {
		0x14, 0xA0, 0x15, 0xA1, 0x16, 0xE2, 0x17, 0xB8, 0x18, 0xEF, 0x19, 0x9D,
		0x1A, 0x36, 0x1B, 0x51, 0x1C, 0x00, 0x1D, 0x00, 0x1E, 0x0E, 0x1F, 0x07,
		0x20, 0x0B, 0x21, 0x2F, 0x22, 0x2F, 0x23, 0x2F, 0x24, 0x2F, ENDDEF, 0x00
};

static const unsigned char panel_gamma_table[] = {
		0x25, 0x44, 0x26, 0x35, 0x27, 0x6D, 0x28, 0x3C, 0x29, 0x35, 0x2A, 0x47,
		0x2B, 0x61, 0x2C, 0x35, 0x2D, 0x5D, 0x2E, 0x39, 0x2F, 0x35, 0x30, 0x46,
		0x31, 0x5D, 0x32, 0x35, 0x33, 0x69, 0x34, 0x3A, 0x35, 0x33, 0x36, 0x44,
		0x37, 0x7D, ENDDEF, 0x00
};

static const unsigned char panel_standby_off_table[] = {
		0x9A, 0x80, 0x00, 0xA0, SLEEPMSEC, 160, 0x01, 0xA1, SLEEPMSEC, 20, 0x00,
		0xA3, ENDDEF, 0x00
};

static const unsigned char panel_standby_on_table[] = {
		0x00, 0xA0, SLEEPMSEC, 20, 0x01, 0xA0, SLEEPMSEC, 20, 0x00, 0xA4,
		ENDDEF, 0x00
};

static const unsigned char panel_display_on_table[] = {
		0x01, 0xA1, SLEEPMSEC, 20, 0x00, 0xA3, ENDDEF, 0x00
};

static const unsigned char panel_display_off_table[] = {
		0x00, 0xA0, SLEEPMSEC, 20, 0x01, 0xA0, ENDDEF, 0x00
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
		857,		524,		0,		60,			0,		10,
 /**< read h start/ enable h start/ enable h size/ enable v start/ enable v size */
		144,			150,			640,			38,			480,
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
    " VGA SMD 3.0 Inch AM-OLED Panel "
};

/******************************************************************************/
/*                  Main LCD 1 Panel Control Functions                        */
/******************************************************************************/

static void mlcd_1_ctrl_power_on(void)
{
	/**< power on */
	mlcd_1_panel_set_gpio(LPM_GPIO_POWER_1, LPM_ON);
	mdelay(10);
}

static void mlcd_1_ctrl_reset(void)
{
	/**< reset */
	mlcd_1_panel_set_gpio(LPM_GPIO_RESET, LPM_ON);
	mdelay(1);
	mlcd_1_panel_set_gpio(LPM_GPIO_RESET, LPM_OFF);
	mdelay(10);

	mlcd_1_panel_set_gpio(LPM_GPIO_RESET, LPM_ON);
	mdelay(10);
}

static void mlcd_1_ctrl_init(void)
{
	mlcd_1_panel_spi_init();

	mlcd_1_set_table_value(panel_display_ctrl_table);
	mlcd_1_set_table_value(panel_power_ctrl_table);
	mlcd_1_set_table_value(panel_timing_ctrl_table);
	mlcd_1_set_table_value(panel_gamma_table);
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
	mlcd_1_set_table_value(panel_display_on_table);
}

static void mlcd_1_ctrl_display_off(void)
{
	mlcd_1_set_table_value(panel_display_off_table);
}

static void mlcd_1_ctrl_power_off(void)
{
	mlcd_1_ctrl_standby_on();
	mlcd_1_ctrl_display_off();
}

static void mlcd_1_ctrl_brightness(int level)
{
	printk("Main LCD Brightness: %d \n", level);
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
	printk("Main LCD 1 panel unregister \n");
}

#ifndef CONFIG_SCORE_FAST_RESUME
module_init(mlcd_1_register);
#else
fast_late_initcall(mlcd_1_register);
#endif
module_exit(mlcd_1_unregister);
