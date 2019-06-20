/**
 * @file pwmdev_conf.h
 * @brief PWM user dev data config set
 * @author kyuchun han <kyuchun.han@samsung.com>
 * 2010 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifndef __LINUX_PWMDEV_CONF_H
#define __LINUX_PWMDEV_CONF_H

enum pwmdev_ch {
	PWMDEV_CH0,
	PWMDEV_CH1,
	PWMDEV_CH2,
	PWMDEV_CH3,
	PWMDEV_CH4,
	PWMDEV_CH5,
	PWMDEV_CH6,
	PWMDEV_CH7,
	PWMDEV_CH8,
	PWMDEV_CH9,
	PWMDEV_CH10,
	PWMDEV_CH11,
	PWMDEV_CH12,
	PWMDEV_CH13,
	PWMDEV_CH14,
	PWMDEV_CH15,
	PWMDEV_CH16,
	PWMDEV_CH17,
	PWMDEV_MAX,
	PWMDEV_AUTO,
	PWMDEV_NULL
};

struct platform_pwm_dev_data {
	enum pwmdev_ch pwm_sch;
	unsigned int input_trg_port;
	unsigned int pin_num;
};

#endif
