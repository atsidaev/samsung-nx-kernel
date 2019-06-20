 /**
 * @file d4_lcd_panel_manager_if.h
 * @brief DRIMe4 LCD Panel Manager Device Driver Header
 * @author Kim Sunghoon <bluesay.kim@samsung.com>
 * 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifndef __LCD_PANEL_MANAGER_DD_IF_H__
#define __LCD_PANEL_MANAGER_DD_IF_H__

#include <media/drime4/lcd_panel_manager/d4_lcd_panel_manager_type.h>

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
/** 				Main LCD Panel Control Function 						***/
/******************************************************************************/

/**< Basic Control Function */
int mlcd_panel_select(enum lpm_mlcd_select selection);
void mlcd_panel_reset(void);

void mlcd_panel_power_on(void);
void mlcd_panel_power_off(void);
void mlcd_panel_init(void);

void mlcd_panel_light_on(void);
void mlcd_panel_light_off(void);

void mlcd_panel_set_brightness(enum lpm_lcd_brightness_level level);

unsigned short mlcd_panel_get_h_size(void);
unsigned short mlcd_panel_get_v_size(void);

/**< Optional Control Function */
void mlcd_panel_standby_on(void);
void mlcd_panel_standby_off(void);

void mlcd_panel_h_flip_on(void);
void mlcd_panel_h_flip_off(void);

void mlcd_panel_v_flip_on(void);
void mlcd_panel_v_flip_off(void);

void mlcd_panel_control_rotation(enum lpm_lcd_rotation rotation);
void mlcd_panel_control_brightness_gamma(enum lpm_lcd_brightness_level level);
void mlcd_panel_control_csc(void *control);
void mlcd_panel_control_tcc(void *control);
void mlcd_panel_control_extension(void *ext_control);

enum lpm_lcd_ratio mlcd_panel_get_display_ratio(void);

/******************************************************************************/
/** 				 Sub LCD Panel Control Function 						***/
/******************************************************************************/

/**< Basic Control Function */
void slcd_panel_reset(void);

void slcd_panel_power_on(void);
void slcd_panel_power_off(void);
void slcd_panel_init(void);

void slcd_panel_light_on(void);
void slcd_panel_light_off(void);

void slcd_panel_set_brightness(enum lpm_lcd_brightness_level level);

unsigned short slcd_panel_get_h_size(void);
unsigned short slcd_panel_get_v_size(void);

/**< Optional Control Function */
void slcd_panel_standby_on(void);
void slcd_panel_standby_off(void);

void slcd_panel_h_flip_on(void);
void slcd_panel_h_flip_off(void);

void slcd_panel_v_flip_on(void);
void slcd_panel_v_flip_off(void);

void slcd_panel_control_rotation(enum lpm_lcd_rotation rotation);
void slcd_panel_control_brightness_gamma(enum lpm_lcd_brightness_level level);
void slcd_panel_control_csc(void *control);
void slcd_panel_control_tcc(void *control);
void slcd_panel_control_extension(void *ext_control);

enum lpm_lcd_ratio slcd_panel_get_display_ratio(void);

#ifdef __cplusplus
}
#endif


#endif /* __LCD_PANEL_MANAGER_DD_IF_H__ */
