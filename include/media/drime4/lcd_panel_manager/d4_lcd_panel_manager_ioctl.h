/**
 * @file d4_lcd_panel_manager_ioctl.h
 * @brief DRIMe4 LCD Panel Manager IOCTL Interface Header File
 * @author Kim Sunghoon <bluesay.kim@samsung.com>
 * 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __D4_LCD_PANEL_MANAGER_IOCTL_H__
#define __D4_LCD_PANEL_MANAGER_IOCTL_H__

#include "d4_lcd_panel_manager_type.h"

#define LCD_PANEL_MANAGER_MAGIC 'l'

/******************************************************************************/
/** 				Main LCD Panel Control Function 						***/
/******************************************************************************/

/**< Basic Control Function */

#define LCD_PANEL_IOCTL_MLCD_SELECT 			_IOW(LCD_PANEL_MANAGER_MAGIC, 1, enum lpm_mlcd_select)

#define LCD_PANEL_IOCTL_MLCD_POWER_ON			_IO(LCD_PANEL_MANAGER_MAGIC, 2)

#define LCD_PANEL_IOCTL_MLCD_RESET 				_IO(LCD_PANEL_MANAGER_MAGIC, 3)

#define LCD_PANEL_IOCTL_MLCD_INIT 				_IO(LCD_PANEL_MANAGER_MAGIC, 4)

#define LCD_PANEL_IOCTL_MLCD_LIGHT_ON 			_IO(LCD_PANEL_MANAGER_MAGIC, 5)
#define LCD_PANEL_IOCTL_MLCD_LIGHT_OFF 			_IO(LCD_PANEL_MANAGER_MAGIC, 6)


#define LCD_PANEL_IOCTL_MLCD_SET_BRIGHTNESS 	_IOW(LCD_PANEL_MANAGER_MAGIC, 7, enum lpm_lcd_brightness_level)

#define LCD_PANEL_IOCTL_MLCD_POWER_OFF			_IO(LCD_PANEL_MANAGER_MAGIC, 8)

#define LCD_PANEL_IOCTL_MLCD_GET_H_SIZE			_IOR(LCD_PANEL_MANAGER_MAGIC, 9, unsigned short)
#define LCD_PANEL_IOCTL_MLCD_GET_V_SIZE			_IOR(LCD_PANEL_MANAGER_MAGIC, 10, unsigned short)

/**< Optional Control Function */

#define LCD_PANEL_IOCTL_MLCD_STANDBY_ON 		_IO(LCD_PANEL_MANAGER_MAGIC, 21)
#define LCD_PANEL_IOCTL_MLCD_STANDBY_OFF 		_IO(LCD_PANEL_MANAGER_MAGIC, 22)

#define LCD_PANEL_IOCTL_MLCD_H_FLIP_ON 			_IO(LCD_PANEL_MANAGER_MAGIC, 23)
#define LCD_PANEL_IOCTL_MLCD_H_FLIP_OFF			_IO(LCD_PANEL_MANAGER_MAGIC, 24)

#define LCD_PANEL_IOCTL_MLCD_V_FLIP_ON 			_IO(LCD_PANEL_MANAGER_MAGIC, 25)
#define LCD_PANEL_IOCTL_MLCD_V_FLIP_OFF			_IO(LCD_PANEL_MANAGER_MAGIC, 26)

#define LCD_PANEL_IOCTL_MLCD_CTRL_ROTATION 		_IOW(LCD_PANEL_MANAGER_MAGIC, 27, enum lpm_lcd_rotation)
#define LCD_PANEL_IOCTL_MLCD_CTRL_BRIGHTNESS_GAMMA	_IOW(LCD_PANEL_MANAGER_MAGIC, 28, enum lpm_lcd_brightness_level)

#define LCD_PANEL_IOCTL_MLCD_CTRL_EXTENSION		_IOW(LCD_PANEL_MANAGER_MAGIC, 29, void *)

#define LCD_PANEL_IOCTL_MLCD_GET_DISPLAY_RATIO	_IOR(LCD_PANEL_MANAGER_MAGIC, 30, enum lpm_lcd_ratio)

#define LCD_PANEL_IOCTL_MLCD_CTRL_CSC			_IOW(LCD_PANEL_MANAGER_MAGIC, 31, void *)
#define LCD_PANEL_IOCTL_MLCD_CTRL_TCC			_IOW(LCD_PANEL_MANAGER_MAGIC, 32, void *)

/******************************************************************************/
/** 				Sub LCD Panel Control Function 						***/
/******************************************************************************/

/**< Basic Control Function */

#define LCD_PANEL_IOCTL_SLCD_POWER_ON 			_IO(LCD_PANEL_MANAGER_MAGIC, 41)
#define LCD_PANEL_IOCTL_SLCD_RESET 				_IO(LCD_PANEL_MANAGER_MAGIC, 42)

#define LCD_PANEL_IOCTL_SLCD_INIT 				_IO(LCD_PANEL_MANAGER_MAGIC, 43)

#define LCD_PANEL_IOCTL_SLCD_LIGHT_ON 			_IO(LCD_PANEL_MANAGER_MAGIC, 44)
#define LCD_PANEL_IOCTL_SLCD_LIGHT_OFF 			_IO(LCD_PANEL_MANAGER_MAGIC, 45)

#define LCD_PANEL_IOCTL_SLCD_SET_BRIGHTNESS 	_IOW(LCD_PANEL_MANAGER_MAGIC, 46, enum lpm_lcd_brightness_level)

#define LCD_PANEL_IOCTL_SLCD_POWER_OFF 			_IO(LCD_PANEL_MANAGER_MAGIC, 47)

#define LCD_PANEL_IOCTL_SLCD_GET_H_SIZE			_IOR(LCD_PANEL_MANAGER_MAGIC, 48, unsigned short)
#define LCD_PANEL_IOCTL_SLCD_GET_V_SIZE			_IOR(LCD_PANEL_MANAGER_MAGIC, 49, unsigned short)

/**< Optional Control Function */

#define LCD_PANEL_IOCTL_SLCD_STANDBY_ON 		_IO(LCD_PANEL_MANAGER_MAGIC, 61)
#define LCD_PANEL_IOCTL_SLCD_STANDBY_OFF 		_IO(LCD_PANEL_MANAGER_MAGIC, 62)

#define LCD_PANEL_IOCTL_SLCD_H_FLIP_ON 			_IO(LCD_PANEL_MANAGER_MAGIC, 63)
#define LCD_PANEL_IOCTL_SLCD_H_FLIP_OFF			_IO(LCD_PANEL_MANAGER_MAGIC, 64)

#define LCD_PANEL_IOCTL_SLCD_V_FLIP_ON 			_IO(LCD_PANEL_MANAGER_MAGIC, 65)
#define LCD_PANEL_IOCTL_SLCD_V_FLIP_OFF			_IO(LCD_PANEL_MANAGER_MAGIC, 66)

#define LCD_PANEL_IOCTL_SLCD_CTRL_ROTATION 		_IOW(LCD_PANEL_MANAGER_MAGIC, 67, enum lpm_lcd_rotation)
#define LCD_PANEL_IOCTL_SLCD_CTRL_BRIGHTNESS_GAMMA	_IOW(LCD_PANEL_MANAGER_MAGIC, 68, enum lpm_lcd_brightness_level)

#define LCD_PANEL_IOCTL_SLCD_CTRL_EXTENSION		_IOW(LCD_PANEL_MANAGER_MAGIC, 69, void *)

#define LCD_PANEL_IOCTL_SLCD_GET_DISPLAY_RATIO	_IOR(LCD_PANEL_MANAGER_MAGIC, 70, enum lpm_lcd_ratio)

#define LCD_PANEL_IOCTL_SLCD_CTRL_CSC			_IOW(LCD_PANEL_MANAGER_MAGIC, 71, void *)
#define LCD_PANEL_IOCTL_SLCD_CTRL_TCC			_IOW(LCD_PANEL_MANAGER_MAGIC, 72, void *)

#endif /* __D4_LCD_PANEL_MANAGER_IOCTL_H__ */
