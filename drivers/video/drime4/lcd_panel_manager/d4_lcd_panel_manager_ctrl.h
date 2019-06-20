 /**
 * @file d4_lcd_panel_manager_ctrl.h
 * @brief DRIMe4 LCD Panel Manager Internal Header
 * 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifndef _LCD_PANEL_MANAGER_CONTROL_H_
#define _LCD_PANEL_MANAGER_CONTROL_H_

#include "d4_lcd_panel_manager_if.h"
#include <mach/lcd_panel_manager/d4_lcd_panel_manager.h>

/**< Debug LCD Panel Manager Print */
#ifdef CONFIG_LPM_DEBUG_MODE
	#define LPM_PRINTK(fmt, args...)								\
		{															\
			printk("\r[%s() %d]  ", __FUNCTION__, __LINE__);		\
			printk(fmt, ##args);									\
			printk("\n");											\
		}
#else
    #define LPM_PRINTK(fmt, args...)
#endif

/******************************************************************************/
/*                                Enumeration                                 */
/******************************************************************************/

enum lpm_gpio_select {
	LPM_GPIO_RESET,
	LPM_GPIO_POWER_1,
	LPM_GPIO_POWER_2,
	LPM_GPIO_POWER_3
};

enum lpm_lcd_select {
	LPM_MLCD_1,
	LPM_MLCD_2,
	LPM_SLCD
};

enum lpm_cotrol_lcd_select {
	LPM_MLCD_CTRL,
	LPM_SLCD_CTRL
};

/******************************************************************************/
/*                                Structure                                   */
/******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

int mlcd_1_panel_set_info(struct lcd_panel_manager_info *panel_info);
int mlcd_2_panel_set_info(struct lcd_panel_manager_info *panel_info);
int slcd_panel_set_info(struct lcd_panel_manager_info *panel_info);

void lcd_panel_set_init_data(struct d4_lcd_panel_manager_data *init_data);

int mlcd_1_panel_set_spi_init_config(struct d4_hs_spi_config *config);
int mlcd_2_panel_set_spi_init_config(struct d4_hs_spi_config *config);
int slcd_panel_set_spi_init_config(struct d4_hs_spi_config *config);

void mlcd_1_panel_spi_init(void);
void mlcd_2_panel_spi_init(void);
void slcd_panel_spi_init(void);

int mlcd_1_panel_spi_write(struct lcd_panel_spi_write_info *set_info);
int mlcd_2_panel_spi_write(struct lcd_panel_spi_write_info *set_info);
int slcd_panel_spi_write(struct lcd_panel_spi_write_info *set_info);

int mlcd_1_panel_spi_read(struct lcd_panel_spi_rw_info *set_info);
int mlcd_2_panel_spi_read(struct lcd_panel_spi_rw_info *set_info);
int slcd_panel_spi_read(struct lcd_panel_spi_rw_info *set_info);

int mlcd_1_panel_spi_rw(struct lcd_panel_spi_rw_info *set_info);
int mlcd_2_panel_spi_rw(struct lcd_panel_spi_rw_info *set_info);
int slcd_panel_spi_rw(struct lcd_panel_spi_rw_info *set_info);

void mlcd_1_panel_set_gpio(enum lpm_gpio_select select, enum lpm_onoff set_value);
void mlcd_2_panel_set_gpio(enum lpm_gpio_select select, enum lpm_onoff  set_value);
void slcd_panel_set_gpio(enum lpm_gpio_select select, enum lpm_onoff  set_value);

void lcd_panel_suspend(void);
void lcd_panel_resume(void);

/**< 내부 함수 */

#ifdef __cplusplus
}
#endif

#endif /* _LCD_PANEL_MANAGER_CONTROL_H_ */

