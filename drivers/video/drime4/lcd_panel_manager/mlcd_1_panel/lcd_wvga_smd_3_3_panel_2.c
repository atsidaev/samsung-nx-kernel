/**
 * @file lcd_wvga_smd_3_3_panel_2.c
 * @brief SMD WVGA 3.3 AMOLED panel driver(Portrait type).
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

/* Private variable */

#define CALC_PARAM_CNT(x)	((sizeof(x) / 2) - 1)

/**< LCD panel control value */

static const unsigned short panel_condition_set_table[] = {
		0x0F8,
		0x101, 0x127, 0x127, 0x107, 0x107, 0x154, 0x19F, 0x163, 0x186, 0x11A,
		0x133, 0x10D, 0x100, 0x100
};

static const unsigned short display_condition_1_set_table[] = {
		0x0F2,
		0x102, 0x103, 0x137, 0x11C, 0x111 /**< VBP:3, VHB:55, HBP:28, HFP:17 */
};

static const unsigned short display_condition_2_set_table[] = {
		0x0F7,
		0x100, 0x100, 0x120
};

static const unsigned short gamma_set_table[] = {
		0x0FA,
		0x102, 0x177, 0x168, 0x177, 0x180, 0x190, 0x17D, 0x18F, 0x19C, 0x18C,
		0x190, 0x19B, 0x18F, 0x1AD, 0x1B3, 0x1AC, 0x100, 0x1BC, 0x100, 0x1B5,
		0x100, 0x1DF
};

static const unsigned short gamma_update_table[] = {
		0x0FA,
		0x103
};

static const unsigned short etc_condition_1_table[] = {
		0x0F6,
		0x100, 0x18E, 0x10F
};

static const unsigned short etc_condition_2_table[] = {
		0x0B3,
		0x10C
};

static const unsigned short etc_condition_3_table[] = {
		0x0B5,
		0x108, 0x108, 0x108, 0x108, 0x110, 0x110, 0x120, 0x120, 0x12E, 0x119,
		0x12E, 0x127, 0x121, 0x11E, 0x11A, 0x119, 0x12C, 0x127, 0x124, 0x121,
		0x13B, 0x134, 0x130, 0x12C, 0x128, 0x126, 0x124, 0x122, 0x121, 0x11F,
		0x11E, 0x11C
};

static const unsigned short etc_condition_4_table[] = {
		0x0B6,
		0x100, 0x100, 0x111, 0x122, 0x133, 0x144, 0x144, 0x144, 0x155, 0x155,
		0x166, 0x166, 0x166, 0x166, 0x166, 0x166
};

static const unsigned short etc_condition_5_table[] = {
		0x0B7,
		0x108, 0x108, 0x108, 0x108, 0x110, 0x110, 0x120, 0x120, 0x12E, 0x119,
		0x12E, 0x127, 0x121, 0x11E, 0x11A, 0x119, 0x12C, 0x127, 0x124, 0x121,
		0x13B, 0x134, 0x130, 0x12C, 0x128, 0x126, 0x124, 0x122, 0x121, 0x11F,
		0x11E, 0x11C
};

static const unsigned short etc_condition_6_table[] = {
		0x0B8,
		0x100, 0x100, 0x111, 0x122, 0x133, 0x144, 0x144, 0x144, 0x155, 0x155,
		0x166, 0x166, 0x166, 0x166, 0x166, 0x166
};

static const unsigned short etc_condition_7_table[] = {
		0x0B9,
		0x108, 0x108, 0x108, 0x108, 0x110, 0x110, 0x120, 0x120, 0x12E, 0x119,
		0x12E, 0x127, 0x121, 0x11E, 0x11A, 0x119, 0x12C, 0x127, 0x124, 0x121,
		0x13B, 0x134, 0x130, 0x12C, 0x128, 0x126, 0x124, 0x122, 0x121, 0x11F,
		0x11E, 0x11C
};

static const unsigned short etc_condition_8_table[] = {
		0x0BA,
		0x100, 0x100, 0x111, 0x122, 0x133, 0x144, 0x144, 0x144, 0x155, 0x155,
		0x166, 0x166, 0x166, 0x166, 0x166, 0x166
};

static const unsigned short dynamic_elvss_on[] = {
		0x0B1,
		0x10D
};

static const unsigned short dynamic_elvss_off[] = {
		0x0B1,
		0x10A
};

/* Private function */
static int mlcd_1_set_table_value(const unsigned short *wbuf, unsigned int param_cnt);

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
static void mlcd_1_ctrl_csc(void *info);
static void mlcd_1_ctrl_tcc(void *info);
static void mlcd_1_ctrl_extension(void *ext_info);

/******************************************************************************/
/*                     Main LCD 1 Panel Information                           */
/******************************************************************************/

struct lcd_panel_hw_info mlcd_1_panel_hw_info = {
 /**< width/ height/   LCD clock/       LCD ratio/  dot array type */
		480,	800,	27000000,	LPM_RATIO_16_9,	LPM_STRIPE,
 /**< dot even seq/ dot odd seq/ output format/ output bit-width */
		LPM_BGR,	LPM_BGR,	LPM_RGB_OUT,	LPM_BIT_WIDTH_24
};

struct lcd_panel_timing_info mlcd_1_panel_timing_info = {
 /**< total h/  total v/  h rising/ h falling/ v rising/ v falling */
		524,		857,		0,		10,			0,		2,

 /**< read h start/ enable h start/ enable h size/ enable v start/ enable v size */
		20,			28,			480,			3,			800,

 /**< inv_dot clk/ inv_enable/ inv_hsync/ inv_vsync */
	  LPM_OFF,		  LPM_ON,	    LPM_ON,		LPM_ON
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
		mlcd_1_ctrl_csc,	    	/**< LCD CSC control function */
		mlcd_1_ctrl_tcc,			/**< LCD TCC control function */
		mlcd_1_ctrl_extension,		/**< LCD extension control function */

		NULL,						/**< suspend function for power management */
		NULL						/**< resume  function for power management */
};

struct lcd_panel_manager_info mlcd_1_panel_info = {
    &mlcd_1_panel_hw_info,
    &mlcd_1_panel_timing_info,
    &mlcd_1_panel_ctrl_func,
    " VGA SMD 3.31 Inch AM-OLED Panel "
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
	mlcd_1_panel_spi_init();

	mlcd_1_set_table_value(panel_condition_set_table, CALC_PARAM_CNT(panel_condition_set_table));

	mlcd_1_set_table_value(display_condition_1_set_table, CALC_PARAM_CNT(display_condition_1_set_table));
	mlcd_1_set_table_value(display_condition_2_set_table, CALC_PARAM_CNT(display_condition_2_set_table));

	/**< Gamma Setting */
	mlcd_1_set_table_value(gamma_set_table, CALC_PARAM_CNT(gamma_set_table));
	mlcd_1_set_table_value(gamma_update_table, CALC_PARAM_CNT(gamma_update_table));

	mlcd_1_set_table_value(etc_condition_1_table, CALC_PARAM_CNT(etc_condition_1_table));
	mlcd_1_set_table_value(etc_condition_2_table, CALC_PARAM_CNT(etc_condition_2_table));
	mlcd_1_set_table_value(etc_condition_3_table, CALC_PARAM_CNT(etc_condition_3_table));
	mlcd_1_set_table_value(etc_condition_4_table, CALC_PARAM_CNT(etc_condition_4_table));
	mlcd_1_set_table_value(etc_condition_5_table, CALC_PARAM_CNT(etc_condition_5_table));
	mlcd_1_set_table_value(etc_condition_6_table, CALC_PARAM_CNT(etc_condition_6_table));
	mlcd_1_set_table_value(etc_condition_7_table, CALC_PARAM_CNT(etc_condition_7_table));
	mlcd_1_set_table_value(etc_condition_8_table, CALC_PARAM_CNT(etc_condition_8_table));

	/**< ELVSS Setting */
	#if 1
		/**< ELVSS OFF */
	mlcd_1_set_table_value(dynamic_elvss_off, CALC_PARAM_CNT(dynamic_elvss_off));

	#else
		/**< ELVSS ON */
	mlcd_1_set_table_value(dynamic_elvss_210_300_table, CALC_PARAM_CNT(dynamic_elvss_210_300_table));
	mlcd_1_set_table_value(dynamic_elvss_on, CALC_PARAM_CNT(dynamic_elvss_on));
	#endif

	mdelay(10);
	mlcd_1_ctrl_standby_off();
}

static void mlcd_1_ctrl_standby_on(void)
{
	unsigned short set_value = 0;

	set_value = 0x010;
	mlcd_1_set_table_value(&set_value, 0);
	mdelay(120);
}

static void mlcd_1_ctrl_standby_off(void)
{
	unsigned short set_value = 0;

	set_value = 0x011;
	mlcd_1_set_table_value(&set_value, 0);
	mdelay(120);
}

static void mlcd_1_ctrl_display_on(void)
{
	unsigned short set_value = 0;

	set_value = 0x029;
	mlcd_1_set_table_value(&set_value, 0);
}

static void mlcd_1_ctrl_display_off(void)
{

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

static void mlcd_1_ctrl_csc(void *info)
{
	int *set_value = NULL;

	set_value = (int *)info;
	pr_emerg("set_value: %d \n", *set_value);
}

static void mlcd_1_ctrl_tcc(void *info)
{
	int *set_value = NULL;

	set_value = (int *)info;
	pr_emerg("set_value: %d \n", *set_value);
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

static int mlcd_1_set_table_value(const unsigned short *wbuf, unsigned int param_cnt)
{
	int ret = 0;
	struct lcd_panel_spi_write_info spi_set_info;

	spi_set_info.data_cnt = (param_cnt + 1);
	spi_set_info.data_buf = (void *)wbuf;

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
