/**
 * @file lcd_xga_emagin_evf_panel.c
 * @brief eMagin XVG EVF panel driver.
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
	{0x00, 0x00}, {0x01, 0x03}, {0x02, 0x80}, {0x03, 0x06}, {0x04, 0x06},
	/**< HBP(0x08): 82(0x52), VBP(0x07): 19(0x13)  */
	{0x05, 0x06}, {0x06, 0x06}, {0x07, 0x13}, {0x08, 0x52}, {0x09, 0x00},
	{0x0A, 0x00}, {0x0B, 0x00}, {0x0C, 0x80}, {0x0D, 0x80}, {0x0E, 0x80},
	{0x0F, 0x80}, {0x10, 0x80}, {0x11, 0x01}, {0x12, 0x22}, {0x13, 0x70},
	{0x14, 0x05}, {0x15, 0x00}, {0x16, 0x00}, {0x17, 0x0D}, {0x18, 0x0D},
	{0x19, 0x90}, {0x1A, 0x20}, {0x1B, 0x64}, {0x1C, 0x18}, {0x1D, 0x35},
	{0x1E, 0xFF}, {0x1F, 0x00}, {0x20, 0x00}, {0x21, 0x03}, {0x22, 0x00},
	{0x23, 0x00}, {0x24, 0x00}, {0x25, 0x00}, {0x26, 0x07}, {0x27, 0x0F},
	{0x28, 0x00}, {0x29, 0x00}, {0x2A, 0x00}, {0x2B, 0x10}, {0x2C, 0x2A},
	{0x2D, 0x01}, {0x2E, 0x99}, {0x2F, 0x01}, {0x30, 0xD0}, {0x31, 0x00},
	{0x32, 0x64}, {0x33, 0x00}, {0x34, 0x04}, {0x35, 0x02}, {0x36, 0x00},
	{0x37, 0x00}, {0x38, 0x00}, {0x39, 0xFF}, {0x3A, 0x00}, {0x3B, 0x00},
	{0x40, 0x00}, {0x41, 0x30}, {0x42, 0x64},
};

const unsigned char init_burst_table_value_1[] = {
	0x00,
	0x00, 0x03, 0x80, 0x06, 0x06, 0x06, 0x06, 0x13, 0x52, 0x00,
	0x00, 0x00, 0x80, 0x80, 0x80, 0x80, 0x80, 0x01, 0x22, 0x70,
	0x05, 0x00, 0x00, 0x0D, 0x0D, 0x90, 0x20, 0x64, 0x18, 0x35,
	0xFF, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x07, 0x0F,
	0x00, 0x00, 0x00, 0x10, 0x2A, 0x01, 0x99, 0x01, 0xD0, 0x00,
	0x64, 0x00, 0x04, 0x02, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00,
};

const unsigned char init_burst_table_value_2[] = {
	0x40,
	0x00, 0x30, 0x64
};



/* Private function */
static int mlcd_1_set_single_table_value(const unsigned char addr, const unsigned char data);
static int mlcd_1_set_burst_table_value(const unsigned char *wbuf, unsigned int param_cnt);
static int mlcd_1_read_single_table_value(const unsigned char addr, unsigned char *data);

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

/**< #define SPI_READ_ENABLE */
/**< #define SPI_TABLE_WRITE_ENABLE */
/**< #define SPI_BURST_WRITE_ENABLE */

/******************************************************************************/
/*                     Main LCD 1 Panel Information                           */
/******************************************************************************/

struct lcd_panel_hw_info mlcd_1_panel_hw_info = {
 /**< width/ height/   LCD clock/       LCD ratio/  dot array type */
	   1024,	768,	54000000,	LPM_RATIO_16_9,	LPM_STRIPE,
 /**< dot even seq/ dot odd seq/ output format/ output bit-width */
		LPM_BGR,	LPM_BGR,	LPM_RGB_OUT,	LPM_BIT_WIDTH_24
};

struct lcd_panel_timing_info mlcd_1_panel_timing_info = {
 /**< total h/  total v/  h rising/ h falling/ v rising/ v falling */
		1139,		789,		0,		   32,			0,		4,

 /**< read h start/ enable h start/ enable h size/ enable v start/ enable v size */
		74,			82,				1024,				19,			768,

 /**< inv_dot clk/ inv_enable/ inv_hsync/ inv_vsync */
	  LPM_ON,		 LPM_OFF,    LPM_OFF, 	LPM_OFF
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
    " eMagin XGA EVF Panel "
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
	int i = 0;
	int set_count = 0;

#ifdef SPI_READ_ENABLE
	/**< Read SPI data */
	unsigned char read_value = 0;

	/**< table read */
	for (i = 0; i < 20; i++) {
		mlcd_1_read_single_table_value(i, &read_value);
	}
#endif

#ifdef SPI_TABLE_WRITE_ENABLE
	#ifdef SPI_BURST_WRITE_ENABLE
	/**< using SPI burst write */
	set_count = sizeof(init_burst_table_value_1);
	mlcd_1_set_burst_table_value(init_burst_table_value_1, set_count);

	set_count = sizeof(init_burst_table_value_2);
	mlcd_1_set_burst_table_value(init_burst_table_value_2, set_count);

	#else
	/**< using SPI single write */
	set_count = (sizeof(init_value) / 2);

	for (i = 0; i < set_count; i++) {
	mlcd_1_set_single_table_value(init_value[i][0], init_value[i][1]);
	}
	#endif

#else
	mlcd_1_set_single_table_value(0x27, 0x0F);
	mlcd_1_set_single_table_value(0x21, 0x03);
	mlcd_1_set_single_table_value(0x1A, 0x20);
	mlcd_1_set_single_table_value(0x1B, 0x64);
	mlcd_1_set_single_table_value(0x12, 0x22);
	mlcd_1_set_single_table_value(0x17, 0x0D);
	mlcd_1_set_single_table_value(0x14, 0x05);
	mlcd_1_set_single_table_value(0x01, 0x03);

	/**< VBP: 현재는 설정 하지 않아도 동작 */
	mlcd_1_set_single_table_value(0x07, 0x13);
	/**< HBP: 현재는 설정 하지 않아도 동작 */
	mlcd_1_set_single_table_value(0x08, 0x52);

#endif

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
	/**< delay time by data sheet*/
	mdelay(80);

	mlcd_1_set_single_table_value(0x02, 0x00);
	mdelay(10);
}

static void mlcd_1_ctrl_display_off(void)
{
	mdelay(10);
	mlcd_1_set_single_table_value(0x2, 0x80);
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

static int mlcd_1_set_single_table_value(const unsigned char addr, const unsigned char data)
{
	int ret = 0;
	unsigned char spi_buffer[2] = {0, };
	struct lcd_panel_spi_write_info spi_set_info;

	spi_buffer[0] = addr;
	spi_buffer[1] = data;

	LPM_PRINTK("SPI addr: %#04x, data: %#04x \n", addr, data);

	spi_set_info.data_cnt = 2;
	spi_set_info.data_buf = (void *)&spi_buffer;

	mlcd_1_panel_set_gpio(LPM_GPIO_POWER_3, LPM_ON);
	udelay(10);
	mlcd_1_panel_set_gpio(LPM_GPIO_POWER_3, LPM_OFF);

	ret = mlcd_1_panel_spi_write(&spi_set_info);
	mlcd_1_panel_set_gpio(LPM_GPIO_POWER_3, LPM_ON);
	udelay(10);

	return ret;
}

static int mlcd_1_set_burst_table_value(const unsigned char *wbuf, unsigned int param_cnt)
{
	int ret = 0;
	struct lcd_panel_spi_write_info spi_set_info;

	spi_set_info.data_cnt = param_cnt;
	spi_set_info.data_buf = (void *)wbuf;

	mlcd_1_panel_set_gpio(LPM_GPIO_POWER_3, LPM_ON);
	udelay(10);
	mlcd_1_panel_set_gpio(LPM_GPIO_POWER_3, LPM_OFF);

	ret = mlcd_1_panel_spi_write(&spi_set_info);
	mlcd_1_panel_set_gpio(LPM_GPIO_POWER_3, LPM_ON);

	udelay(10);

	return ret;
}

static int mlcd_1_read_single_table_value(const unsigned char addr, unsigned char *data)
{
	int ret = 0;
	unsigned char spi_w_buffer[2] = {0, };
	unsigned char spi_r_buffer[2] = {0, };
	struct lcd_panel_spi_rw_info spi_set_info;

	spi_w_buffer[0] = (addr | 0x80);
	spi_w_buffer[1] = 0x00;

	spi_set_info.data_cnt = 2;
	spi_set_info.w_data_buf = (void *)&spi_w_buffer;
	spi_set_info.r_data_buf = (void *)&spi_r_buffer;

	mlcd_1_panel_set_gpio(LPM_GPIO_POWER_3, LPM_ON);
	udelay(10);
	mlcd_1_panel_set_gpio(LPM_GPIO_POWER_3, LPM_OFF);

	ret = mlcd_1_panel_spi_rw(&spi_set_info);
	mlcd_1_panel_set_gpio(LPM_GPIO_POWER_3, LPM_ON);

	*data = spi_r_buffer[1];
	LPM_PRINTK("SPI read addr: %#04x, data: %#04x \n", addr, *data);
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
