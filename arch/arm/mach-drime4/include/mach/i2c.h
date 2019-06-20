/* linux/arch/arm/mach-drime4/include/mach/i2c.h
 *
 * Copyright (c) 2011 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com/
 *
 * DRIME4 - I2C definition
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#ifndef __MACH_DRIME4_I2C_H
#define __MACH_DRIME4_I2C_H

#include <linux/platform_device.h>

struct drime4_platform_i2c {
	int             bus_num;
	unsigned int    flags;
	unsigned int    slave_addr;
	unsigned long   frequency;
	unsigned int    sda_delay;

	void	(*cfg_gpio)(struct platform_device *dev);
};

extern struct platform_device * __init drime4_register_i2c(
	int instance, struct drime4_platform_i2c *pdata);

#endif
