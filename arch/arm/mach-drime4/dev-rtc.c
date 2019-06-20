/* linux/arch/arm/mach-drime4/dev-rtc.c
 *
 * Copyright (c) 2011 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com/
 *
 * DRIME4 - RTC support
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

static struct resource drime4_rtc_resource[] = {
	[0] = {
		.start	= DRIME4_PA_RTC,
		.end	= DRIME4_PA_RTC + 0xff,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= IRQ_RTC_ALM,
		.end	= IRQ_RTC_ALM,
		.flags	= IORESOURCE_IRQ,
	},
	[2] = {
		.start	= IRQ_RTC_PRI,
		.end	= IRQ_RTC_PRI,
		.flags	= IORESOURCE_IRQ
	}
};

struct platform_device drime4_device_rtc = {
	.name		= "drime4-rtc",
	.id		= -1,
	.num_resources	= ARRAY_SIZE(drime4_rtc_resource),
	.resource	= drime4_rtc_resource,
};
EXPORT_SYMBOL(drime4_device_rtc);
