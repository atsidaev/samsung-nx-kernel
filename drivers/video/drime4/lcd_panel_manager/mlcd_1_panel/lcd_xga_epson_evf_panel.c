/**
 * @file lcd_xga_epson_evf_panel.c
 * @brief Epson XVG EVF panel driver.
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
#include <linux/pinctrl/pinconf-drime4.h>
#include <linux/pinctrl/pinconf.h>
#include <linux/pinctrl/consumer.h>

#include "../../lcd_panel_manager/d4_lcd_panel_manager_ctrl.h"

/* Private variable */

/**< LCD panel control value */

const unsigned char init_value[][2] = {
	{0x01, 0x00}, {0x02, 0x00}, {0x03, 0x00}, {0x05, 0x00}, {0x07, 0x00},
	{0x0A, 0x00}, {0x10, 0x03}, {0x11, 0x44}, {0x12, 0x44}, {0x13, 0x55},
	{0x14, 0x03}, {0x15, 0x00}, {0x16, 0x2A}, {0x17, 0x2A}, {0x18, 0x00},
	{0x19, 0x15}, {0x1A, 0x18}, {0x1B, 0x13}, {0x1C, 0x14}, {0x1D, 0x15},
	{0x1E, 0x14}, {0x1F, 0x24}, {0x20, 0x14}, {0x21, 0x00}, {0x22, 0x00},
	{0x23, 0x00}, {0x24, 0x00}, {0x28, 0x14}, {0x29, 0x19}, {0x2A, 0x17},
	{0x2B, 0x2B}, {0x2C, 0x9C}, {0x2D, 0x13}, {0x2E, 0x2A}, {0x30, 0x0B},
	{0x31, 0x00}, {0x32, 0x00}, {0x33, 0x01}, {0x34, 0x00}, {0x35, 0x1D},
	{0x36, 0x04}, {0x37, 0xF5}, {0x38, 0x00}, {0x39, 0x64}, {0x3A, 0x01},
	{0x3B, 0x0C}, {0x3C, 0x00}, {0x3D, 0x00}, {0x3E, 0x0E}, {0x3F, 0x08},

	{0x40, 0x00}, {0x41, 0x22}, {0x42, 0x00}, {0x43, 0x33}, {0x45, 0x10},
	{0x46, 0x00}, {0x47, 0x01}, {0x48, 0x00}, {0x49, 0x00}, {0x4A, 0x12},
	{0x4B, 0x44}, {0x4C, 0x0E}, {0x4D, 0x08}, {0x50, 0x0F}, {0x51, 0x04},

	{0x52, 0x01}, {0x53, 0x80}, {0x54, 0x11}, {0x55, 0x55}, {0x56, 0x08},
				   /**< Auto De:Enable */
	{0x57, 0x00}, {0x58, 0x58}, {0x59, 0x02}, {0x5A, 0x00}, {0x5B, 0x00},
	{0x5C, 0x00}, {0x5D, 0x00}, {0x5E, 0x68}, {0x5F, 0x0B}, {0x60, 0x02},
	{0x61, 0x04}, {0x62, 0x12}, {0x63, 0x00}, {0x64, 0x80}, {0x65, 0x00},

	{0x66, 0x80}, {0x67, 0x1D}, {0x68, 0x03}, {0x69, 0x3F}, {0x6A, 0x00},
	{0x6B, 0x00}, {0x6C, 0x00}, {0x70, 0x00}, {0x71, 0x44}, {0x72, 0x22},
	{0x73, 0x11}, {0x74, 0x11}, {0x75, 0x22}, {0x76, 0x11}, {0x77, 0x22},

	{0x78, 0x22}, {0x79, 0x33}, {0x7A, 0x44}, {0x7B, 0x44}, {0x7C, 0x00},
	{0x80, 0x00}, {0x81, 0x55}, {0x82, 0x22}, {0x83, 0x11}, {0x84, 0x11},
	{0x85, 0x22}, {0x86, 0x11}, {0x87, 0x22}, {0x88, 0x33}, {0x89, 0x33},
	{0x8A, 0x33}, {0x8B, 0x44}, {0x8C, 0x00}, {0x90, 0x00}, {0x91, 0x77},
	{0x92, 0x00}, {0x93, 0x00}, {0x94, 0x00}, {0x95, 0x22}, {0x96, 0x11},
	{0x97, 0x22}, {0x98, 0x33}, {0x99, 0x44}, {0x9A, 0x22}, {0x9B, 0x33},
	{0x9C, 0x00}, {0xA0, 0x88}, {0xA1, 0x88}, {0xA2, 0x88}, {0xA3, 0x88},
	{0xA4, 0x88}, {0xA5, 0x88}, {0xA6, 0x88}, {0xA7, 0x88}, {0xA8, 0x88},
	{0xA9, 0x88}, {0xAA, 0x88}, {0xAB, 0x88}, {0xAC, 0x88}, {0xAD, 0x88},
	{0xAE, 0x88}, {0xAF, 0x88}, {0xB0, 0x88}, {0xB1, 0x88}, {0xB2, 0x88},

	{0xB3, 0x88}, {0xB4, 0x88}, {0xB5, 0x88}, {0xB6, 0x88}, {0xB7, 0x88},
	{0xB8, 0x88}, {0xB9, 0x88}, {0xBA, 0x88}, {0xBB, 0xA6}, {0xBC, 0x88},
	{0xBD, 0x88}, {0xBE, 0x88}, {0xBF, 0x88}, {0xC0, 0x88}, {0xC1, 0x88},
	{0xC2, 0x88}, {0xC3, 0x88}, {0xC4, 0x88}, {0xC5, 0x88}, {0xC6, 0x88},
	{0xC7, 0x88}, {0xC8, 0x88}, {0xC9, 0x88}, {0xD0, 0x3A}, {0xD1, 0x2A},
	{0xD2, 0x23}, {0xD3, 0x1F}, {0xD4, 0x1F}, {0xD5, 0x1F}, {0xD6, 0x1F},
	{0xD7, 0x1F}, {0xD8, 0x1F}, {0xD9, 0x04}, {0xDA, 0x14}, {0xDB, 0x1B},
	{0xDC, 0x1F}, {0xDD, 0x1F}, {0xDE, 0x1F}, {0xDF, 0x1F}, {0xE0, 0x1F},
	{0xE1, 0x1F}, {0xE6, 0x00}, {0xF1, 0x00}, {0xF2, 0x00}, {0xF3, 0x00}
};


/* Private function */
static int mlcd_1_set_table_value(const unsigned char addr, const unsigned char data);

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

struct lcd_panel_hw_info mlcd_1_panel_hw_info = {
 /**< width/ height/   LCD clock/       LCD ratio/  dot array type */
	   1024,	768,	64800000,	LPM_RATIO_16_9,	LPM_STRIPE,
 /**< dot even seq/ dot odd seq/ output format/ output bit-width */
		LPM_BGR,	LPM_BGR,	LPM_RGB_OUT,	LPM_BIT_WIDTH_24
};

struct lcd_panel_timing_info mlcd_1_panel_timing_info = {
 /**< total h/  total v/  h rising/ h falling/ v rising/ v falling */
		1349,		799,		0,		   30,			0,		2,

 /**< read h start/ enable h start/ enable h size/ enable v start/ enable v size */
		82,			90,				1024,				15,			768,

 /**< inv_dot clk/ inv_enable/ inv_hsync/ inv_vsync */
	  LPM_ON,		 LPM_OFF,    LPM_ON, 	LPM_ON
};

struct lcd_panel_ctrl_func mlcd_1_panel_ctrl_func = {

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

struct lcd_panel_manager_info mlcd_1_panel_info = {
    &mlcd_1_panel_hw_info,
    &mlcd_1_panel_timing_info,
    &mlcd_1_panel_ctrl_func,
    " Epson XGA EVF Panel "
};

/******************************************************************************/
/*                  Main LCD 1 Panel Control Functions                        */
/******************************************************************************/

static void mlcd_1_ctrl_power_on(void)
{
	/**< for test */
	mlcd_1_panel_set_gpio(LPM_GPIO_POWER_1, LPM_ON);
	mdelay(25);
}

static void mlcd_1_ctrl_reset(void)
{
	/**< reset */
	mlcd_1_panel_set_gpio(LPM_GPIO_RESET, LPM_ON);
	mdelay(10);
	mlcd_1_panel_set_gpio(LPM_GPIO_RESET, LPM_OFF);
	mdelay(10);
	mlcd_1_panel_set_gpio(LPM_GPIO_RESET, LPM_ON);
	mdelay(10);
}

static void mlcd_1_ctrl_init(void)
{
	int i = 0, set_count = 0;

	mlcd_1_panel_spi_init();

	set_count = (sizeof(init_value) / 2);

	for (i = 0; i < set_count; i++) {
		mlcd_1_set_table_value(init_value[i][0], init_value[i][1]);
	}

	mdelay(10);
	mlcd_1_ctrl_standby_off();
}

static void mlcd_1_ctrl_standby_on(void)
{

}

static void mlcd_1_ctrl_standby_off(void)
{

}

static void mlcd_1_ctrl_display_on(void)
{
	mlcd_1_set_table_value(0xA, 0x1);
	mdelay(10);

	/**< backlight ON */
	mlcd_1_panel_set_gpio(LPM_GPIO_POWER_3, LPM_ON);
}

static void mlcd_1_ctrl_display_off(void)
{
	/**< backlight OFF */
	mlcd_1_panel_set_gpio(LPM_GPIO_POWER_3, LPM_OFF);
	mdelay(10);

	mlcd_1_set_table_value(0xA, 0x0);
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

static int mlcd_1_set_table_value(const unsigned char addr, const unsigned char data)
{
	int ret = 0;
	unsigned short spi_buffer = 0;
	struct lcd_panel_spi_write_info spi_set_info;

	spi_buffer = (unsigned short)((addr << 8) | data);
	LPM_PRINTK("SPI set data: %#04x \n", spi_buffer);

	spi_set_info.data_cnt = 1;
	spi_set_info.data_buf = (void *)&spi_buffer;

	ret = mlcd_1_panel_spi_write(&spi_set_info);
	udelay(10);

	return ret;
}

static int mlcd_1_register(void)
{
#if 1
	unsigned long pin_config = 0;
	int ret = -1;

	/**< Data strength를  높이기 위한 임시 코드, for NX1 proto board */
	pin_config = to_config_packed(PIN_CONFIG_DRIVE_STRENGTH,
			PIN_CONFIG_DRIVE_STRENGTH_X6);
	ret = pin_config_group_set("drime4-pinmux", "mlcdcon", pin_config);
	if (ret < 0) {
		printk("Data strength control: Fail \n");
		return ret;
	}
#endif

	if (mlcd_1_panel_set_info(&mlcd_1_panel_info) < 0) {
		return -EINVAL;
	}
	printk("Main LCD 1 panel register \n");
	return 0;
}

static void mlcd_1_unregister(void)
{
	printk("Main LCD 1 panel unregister \n");
}

module_init(mlcd_1_register);
module_exit(mlcd_1_unregister);
