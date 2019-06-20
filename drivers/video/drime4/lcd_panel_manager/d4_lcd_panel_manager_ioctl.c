/**
 * @file d4_lcd_panel_manager_ioctl.c
 * @brief DRIMe4 LCD Panel Manager Ioctl Control Function File
 * @author Kim Sunghoon <bluesay.kim@samsung.com>
 * 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/delay.h>

#include <linux/uaccess.h>
#include <asm/cacheflush.h>

#include "d4_lcd_panel_manager_if.h"
#include "media/drime4/lcd_panel_manager/d4_lcd_panel_manager_ioctl.h"

/**
 * @brief LCD Panel Manager device open
 * @fn int lcd_panel_manager_open(struct inode *inode, struct file *filp)
 * @param struct inode *inode, struct file *filp
 * @return int
 * @author Kim Sunghoon <bluesay.kim@samsung.com>
 * @note  NONE
 */
int lcd_panel_manager_open(struct inode *inode, struct file *filp)
{
	return 0;
}

/**
 * @brief LCD Panel Manager device release
 * @fn int lcd_panel_manager_release(struct inode *inode, struct file *filp)
 * @param struct inode *inode, struct file *filp
 * @return int
 * @author Kim Sunghoon <bluesay.kim@samsung.com>
 * @note  NONE
 */
static int lcd_panel_manager_release(struct inode *inode, struct file *filp)
{
	return 0;
}

/**
 * @brief LCD Panel Manager device read
 * @fn ssize_t lcd_panel_manager_read(struct file *filp, char *buf, size_t count, loff_t *f_pos)
 * @param struct file *filp, char *buf, size_t count, loff_t *f_pos
 * @return ssize_t
 * @author Kim Sunghoon <bluesay.kim@samsung.com>
 * @note  NONE
 */
ssize_t lcd_panel_manager_read(struct file *filp, char *buf, size_t count, loff_t *f_pos)
{
	return 0;
}

/**
 * @brief LCD Panel Manager device write
 * @fn ssize_t lcd_panel_manager_write(struct file *filp, char *buf, size_t count, loff_t *f_pos)
 * @param struct file *filp, char *buf, size_t count, loff_t *f_pos
 * @return ssize_t
 * @author Kim Sunghoon <bluesay.kim@samsung.com>
 * @note  NONE
 */
ssize_t lcd_panel_manager_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos)
{
	return 0;
}

/**
 * @brief LCD Panel Manager DD interface function
 * @fn long lcd_panel_manager_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
 * @param struct file *filp, unsigned int cmd, unsigned long arg
 * @return long
 * @author Kim Sunghoon <bluesay.kim@samsung.com>
 * @note  NONE
 */
long lcd_panel_manager_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	long err = -1;
	int size = 0, ret = -1;
	unsigned short read_value = 0;
	void *ext_control = NULL;
	void *control = NULL;

	enum lpm_mlcd_select mlcd_selection 		= LPM_MLCD_1_SELECT;
	enum lpm_lcd_brightness_level bright_level  = LPM_NORMAL_LEVEL;
	enum lpm_lcd_ratio lcd_display_ratio		= LPM_RATIO_4_3;
	enum lpm_lcd_rotation lcd_rotation			= LPM_ROTATION_OFF;

	if (_IOC_TYPE(cmd) != LCD_PANEL_MANAGER_MAGIC)
		return err;

	size = _IOC_SIZE(cmd);

	if (_IOC_DIR(cmd) & _IOC_READ)
		ret = access_ok(VERIFY_WRITE, (void *)arg, size);
	else if (_IOC_DIR(cmd) & _IOC_WRITE)
		ret = access_ok(VERIFY_READ, (void *)arg, size);

	if (!ret)
		return err;

	switch (cmd) {
	case LCD_PANEL_IOCTL_MLCD_SELECT:
		ret = copy_from_user((void *)&mlcd_selection, (const void *)arg, size);
		if (ret < 0) {
			printk("ioctl fail[command: %d], %s", cmd, __FUNCTION__);
			return ret;
		}
		mlcd_panel_select(mlcd_selection);
		break;

	case LCD_PANEL_IOCTL_MLCD_RESET:
		mlcd_panel_reset();
		break;

	case LCD_PANEL_IOCTL_MLCD_POWER_ON:
		mlcd_panel_power_on();
		break;

	case LCD_PANEL_IOCTL_MLCD_POWER_OFF:
		mlcd_panel_power_off();
		break;

	case LCD_PANEL_IOCTL_MLCD_INIT:
		mlcd_panel_init();
		break;

	case LCD_PANEL_IOCTL_MLCD_LIGHT_ON:
		mlcd_panel_light_on();
		break;

	case LCD_PANEL_IOCTL_MLCD_LIGHT_OFF:
		mlcd_panel_light_off();
		break;

	case LCD_PANEL_IOCTL_MLCD_SET_BRIGHTNESS:
		ret = copy_from_user((void *)&bright_level, (const void *)arg, size);
		if (ret < 0) {
			printk("ioctl fail[command: %d], %s", cmd, __FUNCTION__);
			return ret;
		}
		mlcd_panel_set_brightness(bright_level);
		break;

	case LCD_PANEL_IOCTL_MLCD_GET_H_SIZE:
		read_value = mlcd_panel_get_h_size();

		ret = copy_to_user((void *)arg, (const void *)&read_value, size);
		if (ret < 0) {
			printk("ioctl fail: [%d]", cmd);
			return ret;
		}
		break;

	case LCD_PANEL_IOCTL_MLCD_GET_V_SIZE:
		read_value = mlcd_panel_get_v_size();

		ret = copy_to_user((void *)arg, (const void *)&read_value, size);
		if (ret < 0) {
			printk("ioctl fail: [%d]", cmd);
			return ret;
		}
		break;

	case LCD_PANEL_IOCTL_MLCD_STANDBY_ON:
		mlcd_panel_standby_on();
		break;

	case LCD_PANEL_IOCTL_MLCD_STANDBY_OFF:
		mlcd_panel_standby_off();
		break;

	case LCD_PANEL_IOCTL_MLCD_H_FLIP_ON:
		mlcd_panel_h_flip_on();
		break;

	case LCD_PANEL_IOCTL_MLCD_H_FLIP_OFF:
		mlcd_panel_h_flip_off();
		break;

	case LCD_PANEL_IOCTL_MLCD_V_FLIP_ON:
		mlcd_panel_v_flip_on();
		break;

	case LCD_PANEL_IOCTL_MLCD_V_FLIP_OFF:
		mlcd_panel_v_flip_off();
		break;

	case LCD_PANEL_IOCTL_MLCD_CTRL_ROTATION:
		ret = copy_from_user((void *)&lcd_rotation, (const void *)arg, size);
		if (ret < 0) {
			printk("ioctl fail[command: %d], %s", cmd, __FUNCTION__);
			return ret;
		}
		mlcd_panel_control_rotation(lcd_rotation);
		break;

	case LCD_PANEL_IOCTL_MLCD_CTRL_BRIGHTNESS_GAMMA:
		ret = copy_from_user((void *)&bright_level, (const void *)arg, size);
		if (ret < 0) {
			printk("ioctl fail[command: %d], %s", cmd, __FUNCTION__);
			return ret;
		}
		mlcd_panel_control_brightness_gamma(bright_level);
		break;

	case LCD_PANEL_IOCTL_MLCD_CTRL_CSC:
		ret = copy_from_user(&control, (const void *)arg, size);
		if (ret < 0) {
			printk("ioctl fail[command: %d], %s", cmd, __FUNCTION__);
			return ret;
		}
		mlcd_panel_control_csc(control);
		break;

	case LCD_PANEL_IOCTL_MLCD_CTRL_TCC:
		ret = copy_from_user(&control, (const void *)arg, size);
		if (ret < 0) {
			printk("ioctl fail[command: %d], %s", cmd, __FUNCTION__);
			return ret;
		}
		mlcd_panel_control_tcc(control);
		break;

	case LCD_PANEL_IOCTL_MLCD_GET_DISPLAY_RATIO:
		lcd_display_ratio = mlcd_panel_get_display_ratio();

		ret = copy_to_user((void *)arg, (const void *)&lcd_display_ratio, size);
		if (ret < 0) {
			printk("ioctl fail: [%d]", cmd);
			return ret;
		}
		break;

	case LCD_PANEL_IOCTL_SLCD_RESET:
		slcd_panel_reset();
		break;

	case LCD_PANEL_IOCTL_SLCD_POWER_ON:
		slcd_panel_power_on();
		break;

	case LCD_PANEL_IOCTL_SLCD_POWER_OFF:
		slcd_panel_power_off();
		break;

	case LCD_PANEL_IOCTL_SLCD_INIT:
		slcd_panel_init();
		break;

	case LCD_PANEL_IOCTL_SLCD_LIGHT_ON:
		slcd_panel_light_on();
		break;

	case LCD_PANEL_IOCTL_SLCD_LIGHT_OFF:
		slcd_panel_light_off();
		break;

	case LCD_PANEL_IOCTL_SLCD_SET_BRIGHTNESS:
		ret = copy_from_user((void *)&bright_level, (const void *)arg, size);
		if (ret < 0) {
			printk("ioctl fail[command: %d], %s", cmd, __FUNCTION__);
			return ret;
		}
		slcd_panel_set_brightness(bright_level);
		break;

	case LCD_PANEL_IOCTL_SLCD_GET_H_SIZE:
		read_value = slcd_panel_get_h_size();

		ret = copy_to_user((void *)arg, (const void *)&read_value, size);
		if (ret < 0) {
			printk("ioctl fail: [%d]", cmd);
			return ret;
		}
		break;

	case LCD_PANEL_IOCTL_SLCD_GET_V_SIZE:
		read_value = slcd_panel_get_v_size();

		ret = copy_to_user((void *)arg, (const void *)&read_value, size);
		if (ret < 0) {
			printk("ioctl fail: [%d]", cmd);
			return ret;
		}
		break;

	case LCD_PANEL_IOCTL_SLCD_STANDBY_ON:
		slcd_panel_standby_on();
		break;

	case LCD_PANEL_IOCTL_SLCD_STANDBY_OFF:
		slcd_panel_standby_off();
		break;

	case LCD_PANEL_IOCTL_SLCD_H_FLIP_ON:
		slcd_panel_h_flip_on();
		break;

	case LCD_PANEL_IOCTL_SLCD_H_FLIP_OFF:
		slcd_panel_h_flip_off();
		break;

	case LCD_PANEL_IOCTL_SLCD_V_FLIP_ON:
		slcd_panel_v_flip_on();
		break;

	case LCD_PANEL_IOCTL_SLCD_V_FLIP_OFF:
		slcd_panel_v_flip_off();
		break;

	case LCD_PANEL_IOCTL_SLCD_CTRL_ROTATION:
		ret = copy_from_user((void *)&lcd_rotation, (const void *)arg, size);
		if (ret < 0) {
			printk("ioctl fail[command: %d], %s", cmd, __FUNCTION__);
			return ret;
		}
		slcd_panel_control_rotation(lcd_rotation);
		break;

	case LCD_PANEL_IOCTL_SLCD_CTRL_BRIGHTNESS_GAMMA:
		ret = copy_from_user((void *)&bright_level, (const void *)arg, size);
		if (ret < 0) {
			printk("ioctl fail[command: %d], %s", cmd, __FUNCTION__);
			return ret;
		}
		slcd_panel_control_brightness_gamma(bright_level);
		break;

	case LCD_PANEL_IOCTL_SLCD_CTRL_CSC:
		ret = copy_from_user(&control, (const void *)arg, size);
		if (ret < 0) {
			printk("ioctl fail[command: %d], %s", cmd, __FUNCTION__);
			return ret;
		}
		slcd_panel_control_csc(control);
		break;

	case LCD_PANEL_IOCTL_SLCD_CTRL_TCC:
		ret = copy_from_user(&control, (const void *)arg, size);
		if (ret < 0) {
			printk("ioctl fail[command: %d], %s", cmd, __FUNCTION__);
			return ret;
		}
		slcd_panel_control_tcc(control);
		break;

	case LCD_PANEL_IOCTL_SLCD_CTRL_EXTENSION:
		ret = copy_from_user(&ext_control, (const void *)arg, size);
		if (ret < 0) {
			printk("ioctl fail[command: %d], %s", cmd, __FUNCTION__);
			return ret;
		}
		slcd_panel_control_extension(ext_control);
		break;

	case LCD_PANEL_IOCTL_SLCD_GET_DISPLAY_RATIO:
		lcd_display_ratio = slcd_panel_get_display_ratio();

		ret = copy_to_user((void *)arg, (const void *)&lcd_display_ratio, size);
		if (ret < 0) {
			printk("ioctl fail: [%d]", cmd);
			return ret;
		}
		break;

	default:
		break;
	}
	return 0;
}

const struct file_operations lcd_panel_manager_ops = {
		.open 			= lcd_panel_manager_open,
		.release 		= lcd_panel_manager_release,
		.read 			= lcd_panel_manager_read,
		.write 			= lcd_panel_manager_write,
		.unlocked_ioctl = lcd_panel_manager_ioctl,
};

MODULE_AUTHOR("Kim Sunghoon <bluesay.kim@samsung.com>");
MODULE_DESCRIPTION("Samsung DRIMe4 LCD Panel Manager driver using ioctl");
MODULE_LICENSE("GPL");

