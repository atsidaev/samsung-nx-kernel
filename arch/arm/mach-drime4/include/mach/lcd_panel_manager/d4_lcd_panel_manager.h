/**
 * @file d4_lcd_panel_manager.h
 * @brief DRIMe4 LCD Panel Manager Platform Driver Header File
 * @author Kim Sunghoon <bluesay.kim@samsung.com>
 * 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifndef __DRIME_LCD_PANEL_MANAGER_H_
#define __DRIME_LCD_PANEL_MANAGER_H_

#include <media/drime4/lcd_panel_manager/d4_lcd_panel_manager_type.h>
#include <linux/hs_spi.h>

#define LCD_PANEL_MANAGER_MODULE_NAME		"d4_lcd_panel_manager"

/******************************************************************************/
/*                                Structure                                   */
/******************************************************************************/

struct drime4_lcd_panel_manager {
	struct device *dev;
	const char	  *name;
	int			  id;
	struct clk 	  *clock;
	atomic_t 	  ref_count;
};

#define LPM_HW_CONNECT_NO_USE	0xA5A5

struct d4_lcd_hw_connect_info {
	unsigned int gpio_reset;
	unsigned int gpio_power_1;
	unsigned int gpio_power_2;
	unsigned int gpio_power_3;

	int spi_ch;
	struct d4_hs_spi_config *spi_config;

	void *ext_data;
};

struct d4_lcd_panel_manager_data {
	struct d4_lcd_hw_connect_info *mlcd_1_hw_connect_info;
	struct d4_lcd_hw_connect_info *mlcd_2_hw_connect_info;
	struct d4_lcd_hw_connect_info *slcd_hw_connect_info;
};

#endif /* __DRIME_LCD_PANEL_MANAGER_H_ */

