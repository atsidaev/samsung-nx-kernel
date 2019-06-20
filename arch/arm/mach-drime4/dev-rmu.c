/* linux/arch/arm/mach-drime4/dev-ptc.c
 *
 * Copyright (c) 2011 Samsung Electronics
 *	kyuchun han <kyuchun.han@samsung.com>
 *
 * Base DRIME4 platform device definitions
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
*/

#include <linux/kernel.h>
#include <linux/platform_device.h>

#include <mach/irqs.h>
#include <mach/map.h>


static struct resource drime4_rmu[] = {
	[0] = {
		.start = DRIME4_PA_RESET_CTRL,
		.end   = DRIME4_PA_RESET_CTRL + 0x114 - 1,
		.flags = IORESOURCE_MEM,
	},
};


struct platform_device drime4_device_rmu = {
	.name		  = "drime4-rmu",
	.id		  = 0,
	.num_resources	  = ARRAY_SIZE(drime4_rmu),
	.resource	  = drime4_rmu,
};

