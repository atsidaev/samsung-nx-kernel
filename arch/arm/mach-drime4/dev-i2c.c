/* linux/arch/arm/mach-drime4/dev-i2c.c
 *
 * Copyright (c) 2011 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com/
 *
 * DRIME4 - I2C support
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/gfp.h>
#include <linux/kernel.h>
#include <mach/i2c.h>
#include <mach/map.h>
#include <mach/irqs.h>

static struct drime4_platform_i2c default_i2c_data __initdata = {
	.flags		= 0,
	.slave_addr	= 0x10,
	.frequency	= 100*1000,
	.sda_delay	= 100,
};

static struct drime4_platform_i2c melfas_i2c_data __initdata = {
	.flags		= 0,
	.slave_addr	= 0x10,
	.frequency	= 400*1000,
	.sda_delay	= 100,
};

struct platform_device * __init drime4_register_i2c(int instance,
		struct drime4_platform_i2c *pdata)
{
	struct resource res[] = {
		[0] = { .flags	= IORESOURCE_MEM, },
		[1] = {	.flags	= IORESOURCE_IRQ, },
	};

	if (!pdata) {
		if(instance == 4)
			pdata = &melfas_i2c_data;
		else
			pdata = &default_i2c_data;
		
		pdata->bus_num = instance;
	}
	res[0].start = DRIME4_PA_I2C_BASE(instance);
	res[0].end = res[0].start + SZ_4K - 1;
	res[1].start = res[1].end = IRQ_I2C0 + instance;

	return platform_device_register_resndata(NULL,
		"drime4-i2c", instance, &res[0], 2, pdata, sizeof(*pdata));
}
