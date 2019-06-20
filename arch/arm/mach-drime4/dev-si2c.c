/* linux/arch/arm/mach-drime4/dev-adc.c
 *
 * Copyright (c) 2011 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com/
 *
 * DRIME4 - ADC support
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/platform_device.h>

#include <mach/irqs.h>
#include <mach/map.h>
#include <mach/devs.h>
#include <mach/common.h>


#define D4_SI2C_RESOURCE_SIZE (2)

#define D4_SI2C_ADD_OFFSET(_si2c_no) \
	(0x1000 * _si2c_no)

#define D4_SI2C_NEXT_ADD_OFFSET(_si2c_no) \
		(0x1000 * (_si2c_no + 1))

#define D4_SI2C_RESOURCE(_si2c_no, _add)			\
	(struct resource [D4_SI2C_RESOURCE_SIZE]) {	\
		{					\
			.start	= _add + D4_SI2C_ADD_OFFSET(_si2c_no),			\
			.end	= _add + D4_SI2C_NEXT_ADD_OFFSET(_si2c_no) - 1,	\
			.flags	= IORESOURCE_MEM	\
		},					\
		{					\
			.start	= IRQ_I2C##_si2c_no,			\
			.end	= IRQ_I2C##_si2c_no,			\
			.flags	= IORESOURCE_IRQ	\
		},					\
	}


#if 1
#define DEFINE_D4_SI2C(_si2c_no, _add)			\
	.name		= "drime4-si2c",		\
	.id		= _si2c_no,			\
	.num_resources	= D4_SI2C_RESOURCE_SIZE,		\
	.resource	= D4_SI2C_RESOURCE(_si2c_no, _add),	\


struct platform_device drime4_device_si2c[] = {
	[0] = { DEFINE_D4_SI2C(0, DRIME4_PA_I2C) },
	[1] = { DEFINE_D4_SI2C(1, DRIME4_PA_I2C) },
	[2] = { DEFINE_D4_SI2C(2, DRIME4_PA_I2C) },
	[3] = { DEFINE_D4_SI2C(3, DRIME4_PA_I2C) },
	[4] = { DEFINE_D4_SI2C(4, DRIME4_PA_I2C) },
	[5] = { DEFINE_D4_SI2C(5, DRIME4_PA_I2C) },
	[6] = { DEFINE_D4_SI2C(6, DRIME4_PA_I2C) },
};
#else



static struct resource drime4_si2c_resource[] = {
	[0] = {
		.start = DRIME4_PA_I2C,
		.end   = DRIME4_PA_I2C + 0x1000 - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = IRQ_I2C0,
		.end   = IRQ_I2C0,
		.flags = IORESOURCE_IRQ,
	},
};


struct platform_device drime4_device_si2c = {
	.name       = "drime4-si2c",
	.id		    = 0,
	.num_resources	  = ARRAY_SIZE(drime4_si2c_resource),
	.resource	  = drime4_si2c_resource,
};
#endif
