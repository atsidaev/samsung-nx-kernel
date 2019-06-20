/* linux/arch/arm/mach-drime4/dev-hsic.c
 *
 * Copyright (c) 2012 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com/
 *
 * DRIME4 - HSIC support
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/platform_device.h>
#include <linux/dma-mapping.h>
#include <linux/gpio.h>

#include <mach/map.h>
#include <mach/dma.h>
#include <mach/irqs.h>
#include <mach/audio.h>

/* HSIC_SISO */
#define __HSIC_BASE			(0x20100000)
#define	__HSIC_LINK_BASE		(__HSIC_BASE)
#define	__HSIC_PHY_BASE			(__HSIC_BASE + 0xF0000)

static u64 hsic_dmamask = DMA_BIT_MASK(32);

static struct resource drime4_hsic_resource[] = {
	[0] = {
		.start = __HSIC_LINK_BASE,
		.end   = __HSIC_LINK_BASE + SZ_1M - 1,
		.flags = IORESOURCE_MEM,
	},

	[1] = {
		.start	= IRQ_HSIC,
		.end	= IRQ_HSIC,
		.flags	= IORESOURCE_IRQ,
	},
};

struct platform_device drime4_device_hsic = {
	.name = "drime4_hsic",
	.id = 0,
	.num_resources	  = ARRAY_SIZE(drime4_hsic_resource),
	.resource	  = drime4_hsic_resource,
	.dev		= {
		.dma_mask = &hsic_dmamask,
		.coherent_dma_mask = DMA_BIT_MASK(32),
	}
};

EXPORT_SYMBOL(drime4_device_hsic);
