/* linux/arch/arm/mach-drime4/dev-wdt.c
 *
 * Copyright (c) 2011 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com/
 *
 * DRIME4 - Watchdog Timer support
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/kernel.h>
#include <linux/platform_device.h>

#include <mach/irqs.h>
#include <mach/map.h>

static struct resource drime4_wdt_resource[] = {
	[0] = {
		.start	= DRIME4_PA_WDT,
		.end	= DRIME4_PA_WDT + 0x100 - 1,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= IRQ_WDT,
		.end	= IRQ_WDT,
		.flags	= IORESOURCE_IRQ,
	}
};

struct platform_device drime4_device_wdt = {
	.name		= "drime4-wdt",
	.id		= -1,
	.num_resources	= ARRAY_SIZE(drime4_wdt_resource),
	.resource	= drime4_wdt_resource,
};
EXPORT_SYMBOL(drime4_device_wdt);
