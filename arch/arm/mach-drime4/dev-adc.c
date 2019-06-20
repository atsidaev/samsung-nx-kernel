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

static struct resource drime4_adc_resource[] = {
	[0] = {
		.start	= DRIME4_PA_ADC,
		.end	= DRIME4_PA_ADC + SZ_256 - 1,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= IRQ_ADC,
		.end	= IRQ_ADC,
		.flags	= IORESOURCE_IRQ,
	},
};

struct platform_device drime4_device_adc = {
	.name		= "drime4-adc",
	.id		= -1,
	.num_resources	= ARRAY_SIZE(drime4_adc_resource),
	.resource	= drime4_adc_resource,
};

