/**
 * @file slcd_720p_bridge_ic.c
 * @brief Sub LCD 720p YCbCr output for bridge IC.
 * @author Kim Sunghoon <bluesay.kim@samsung.com>
 * 2012 Samsung Electronics
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
#include "../../lcd/d4_dp_lcd_dd.h"

/* Private variable */

/* Private function */

/**< LCD panel control functions */
static void slcd_ctrl_power_on(void);
static void slcd_ctrl_reset(void);
static void slcd_ctrl_init(void);
static void slcd_ctrl_display_on(void);
static void slcd_ctrl_display_off(void);
static void slcd_ctrl_standby_on(void);
static void slcd_ctrl_standby_off(void);
static void slcd_ctrl_power_off(void);
static void slcd_ctrl_brightness(int level);

/******************************************************************************/
/*                     Main LCD 1 Panel Information                           */
/******************************************************************************/

struct lcd_panel_hw_info slcd_panel_hw_info = {
 /**< width/ height/   LCD clock/       LCD ratio/  dot array type */
	   //1288,	720,	36000000,	LPM_RATIO_16_9,	LPM_STRIPE,
		1300,	719,	72000000,	LPM_RATIO_16_9,	LPM_STRIPE,
 /**< dot even seq/ dot odd seq/ output format/ output bit-width */
		LPM_BGR,	LPM_BGR,	LPM_CCIR656_OUT,	LPM_BIT_WIDTH_16
};

struct lcd_panel_timing_info slcd_panel_timing_info = {
 /**< total h/  total v/  h rising/ h falling/ v rising/ v falling */
		1599,		749,		0,		96,			0,		10,

 /**< read h start/ enable h start/ enable h size/ enable v start/ enable v size */
		//128,			136,			1280,			12,			720,
		128,			136,			1300,			12,			719,

 /**< inv_dot clk/ inv_enable/ inv_hsync/ inv_vsync */
	  LPM_OFF,		  LPM_OFF,	    LPM_OFF,		LPM_OFF
};

struct lcd_panel_ctrl_func slcd_panel_ctrl_func = {

		/* Basic Function */

		slcd_ctrl_reset, 			/**< LCD reset function */

		slcd_ctrl_power_on,			/**< LCD power on function */
		slcd_ctrl_power_off,		/**< LCD power off function */
		slcd_ctrl_init,				/**< LCD initialization function */

		slcd_ctrl_display_on,		/**< LCD light on  function, including back light on */
		slcd_ctrl_display_off,		/**< LCD light off function, including back light off */

		slcd_ctrl_brightness,		/**< LCD panel brightness setting function */

		/* Optional Function */

		slcd_ctrl_standby_on,     	/**< LCD stand-by on Function */
		slcd_ctrl_standby_off,   	/**< LCD stand-by off Function */

		NULL,	    				/**< LCD horizontal flip on function */
		NULL,	    				/**< LCD horizontal flip off function */

		NULL,	    				/**< LCD vertical flip on function */
		NULL,	    				/**< LCD vertical flip off function */

		NULL,       				/**< LCD Rotation control function */
		NULL,  						/**< LCD panel brightness gamma setting function */
		NULL,	    				/**< LCD CSC control function */
		NULL,						/**< LCD TCC control function */
		NULL,						/**< LCD extension control function */

		NULL,						/**< suspend function for power management */
		NULL						/**< resume  function for power management */
};

struct lcd_panel_manager_info slcd_panel_info = {
    &slcd_panel_hw_info,
    &slcd_panel_timing_info,
    &slcd_panel_ctrl_func,
    "720P YCC output for bridge IC"
};

/******************************************************************************/
/*                  Main LCD 1 Panel Control Functions                        */
/******************************************************************************/

static void slcd_ctrl_power_on(void)
{

}

static void slcd_ctrl_reset(void)
{

}

static void slcd_ctrl_init(void)
{
	struct stlcd_bt656 slcd_yc_out;

	slcd_yc_out.path 		= DP_SLCD;
	slcd_yc_out.bit 		= _16bit;
	slcd_yc_out.ycbcr_order = CB_Y_CR_Y;
	slcd_yc_out.onoff 		= DP_ON;

	d4_dp_bt656_onoff(slcd_yc_out);

	printk("Bridgh IC sync start\n");
}

static void slcd_ctrl_standby_on(void)
{
}

static void slcd_ctrl_standby_off(void)
{
	mdelay(120);
}

static void slcd_ctrl_display_on(void)
{
}

static void slcd_ctrl_display_off(void)
{

}

static void slcd_ctrl_power_off(void)
{
	slcd_ctrl_standby_on();
	slcd_ctrl_display_off();
}

static void slcd_ctrl_brightness(int level)
{
	printk("Sub LCD Brightness: %d \n", level);
}

/******************************************************************************/
/*                  		    Private Functions                        	  */
/******************************************************************************/

static int slcd_register(void)
{

	if (slcd_panel_set_info(&slcd_panel_info) < 0) {
		return -EINVAL;
	}
	printk("Sub LCD panel register \n");
	return 0;
}

static void slcd_unregister(void)
{
	printk("Sub LCD 1 panel unregister \n");
}

module_init(slcd_register);
module_exit(slcd_unregister);
