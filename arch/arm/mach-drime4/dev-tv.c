/*
 * Copyright 2010 Samsung Electronics
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/io.h>
#include <linux/platform_device.h>

#include <mach/map.h>
#include <mach/irqs.h>
#include <mach/dp/d4_dp.h>

static struct resource drime4_fb_resource[] = { [0] = {
	.start = DRIME4_PA_DP,
	.end = DRIME4_PA_DP + SZ_64K - 1,
	.flags = IORESOURCE_MEM,
},
[1] = {
	.start = IRQ_DP_CORE,
	.end = IRQ_DP_CORE,
	.flags = IORESOURCE_IRQ,
},
[2] = {
	.start = IRQ_DP_DMA,
	.end = IRQ_DP_DMA,
	.flags = IORESOURCE_IRQ,
}, };

static u64 fb_dma_mask = 0xffffffffUL;

struct platform_device drime4_device_tv = { .name = TV_MODULE_NAME, .id = 0,
		.num_resources = ARRAY_SIZE(drime4_fb_resource),
		.resource = drime4_fb_resource, .dev = {
			.dma_mask = &fb_dma_mask,
			.coherent_dma_mask = 0xffffffffUL
		} };
