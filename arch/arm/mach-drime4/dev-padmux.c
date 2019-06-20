/* linux/arch/arm/mach-drime4/dev-padmux.c
 *
 * Copyright (c) 2011 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com/
 *
 * DRIME4 - PAD Sharing
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <asm-generic/sizes.h>
#include <mach/map.h>
#include <mach/devs.h>

static struct resource drime4_padmux_resource[] = {
	[0] = {
		.start	= DRIME4_PA_GLOBAL_CTRL,
		.end	= DRIME4_PA_GLOBAL_CTRL + SZ_4K + SZ_256 - 1,
		.flags	= IORESOURCE_MEM,
	},
};

struct platform_device drime4_device_padmux = {
	.name		= "drime4-pinmux",
	.id		= -1,
	.num_resources	= ARRAY_SIZE(drime4_padmux_resource),
	.resource	= drime4_padmux_resource,
};
