/**
 * @file dev-ep.c
 * @brief DRIMe4 EP Platform Device
 * @author Wooram Son <wooram.son@samsung.com>
 * 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/platform_device.h>
#include <linux/dma-mapping.h>
#include <linux/gfp.h>
#include <linux/kernel.h>
#include <mach/map.h>
#include <mach/irqs.h>

#include <mach/ep/d4_ep.h>

static struct resource drime4_ep_resource[] = {
/* EP TOP */
	[0] = {
		.start = DRIME4_PA_EP,
		.end = DRIME4_PA_EP + SZ_4K - 1,
		.flags = IORESOURCE_MEM,
	},
/* LDC */
	[1] = {
		.start = DRIME4_PA_EP + 0x1000,
		.end = DRIME4_PA_EP + 0x1000 + SZ_4K - 1,
		.flags = IORESOURCE_MEM,
	},
/* HDR */
	[2] = {
		.start = DRIME4_PA_EP + 0x2000,
		.end = DRIME4_PA_EP + 0x2000 + SZ_4K - 1,
		.flags = IORESOURCE_MEM,
	},
/* NRm */
	[3] = {
		.start = DRIME4_PA_EP + 0x3000,
		.end = DRIME4_PA_EP + 0x3000 + SZ_4K - 1,
		.flags = IORESOURCE_MEM,
	},
/* MnF */
	[4] = {
		.start = DRIME4_PA_EP + 0x4000,
		.end = DRIME4_PA_EP + 0x4000 + SZ_4K - 1,
		.flags = IORESOURCE_MEM,
	},
/* FD */
	[5] = {
		.start = DRIME4_PA_EP + 0x5000,
		.end = DRIME4_PA_EP + 0x5000 + SZ_4K - 1,
		.flags = IORESOURCE_MEM,
	},
/* BBLT */
	[6] = {
		.start = DRIME4_PA_EP + 0x6000,
		.end = DRIME4_PA_EP + 0x6000 + SZ_4K - 1,
		.flags = IORESOURCE_MEM,
	},
/* LVR */
	[7] = {
		.start = DRIME4_PA_EP + 0x7000,
		.end = DRIME4_PA_EP + 0x7000 + SZ_4K - 1,
		.flags = IORESOURCE_MEM,
	},
/* EP DMA */
	[8] = {
		.start = DRIME4_PA_EP + 0x8000,
		.end = DRIME4_PA_EP + 0x8000 + SZ_4K - 1,
		.flags = IORESOURCE_MEM,
	},
/* CORE IRQ */
	[9] = {
		.start = IRQ_EP_CORE,
		.end = IRQ_EP_CORE,
		.flags = IORESOURCE_IRQ,
	},
/* DMA IRQ */
	[10] = {
		.start = IRQ_EP_DMA,
		.end = IRQ_EP_DMA,
		.flags = IORESOURCE_IRQ,
	},
};

static u64 drime4_ep_dma_mask = DMA_BIT_MASK(32);

struct platform_device drime4_device_ep = {
		.name = EP_MODULE_NAME,
		.id = 0,
		.num_resources = ARRAY_SIZE(drime4_ep_resource),
		.resource = drime4_ep_resource,
		.dev = {
			.dma_mask = &drime4_ep_dma_mask,
			.coherent_dma_mask = DMA_BIT_MASK(32),
		},
};
