/**
 * @file dev-be.c
 * @brief DRIMe4 Bayer Platform Device
 * @author Niladri Mukherjee <n.mukherjee@samsung.com>
 * 2012 Samsung Electronics
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

#include <mach/be/d4_be.h>
/*#include <mach/be/d4_be_regs.h>*/

static struct resource drime4_be_resource[] = {
/* BE TOP */
	[0] = {
		.start = DRIME4_PA_BAYER,
		.end = DRIME4_PA_BAYER + SZ_4K - 1,
		.flags = IORESOURCE_MEM,
	},
/* BE GHOST */
	[1] = {
		.start = DRIME4_PA_BAYER + 0x1000,
		.end = DRIME4_PA_BAYER + 0x1000 + SZ_4K - 1,
		.flags = IORESOURCE_MEM,
	},
/* BE SNR */
	[2] = {
		.start = DRIME4_PA_BAYER + 0x2000,
		.end = DRIME4_PA_BAYER + 0x2000 + SZ_4K - 1,
		.flags = IORESOURCE_MEM,
	},
/* BE SG */
	[3] = {
		.start = DRIME4_PA_BAYER + 0x3000,
		.end = DRIME4_PA_BAYER + 0x3000 + SZ_4K - 1,
		.flags = IORESOURCE_MEM,
	},
/* BE BLEND */
	[4] = {
		.start = DRIME4_PA_BAYER + 0x4000,
		.end = DRIME4_PA_BAYER + 0x4000 + SZ_4K - 1,
		.flags = IORESOURCE_MEM,
	},
/* BE FME */
	[5] = {
		.start = DRIME4_PA_BAYER + 0x5000,
		.end = DRIME4_PA_BAYER + 0x5000 + SZ_4K - 1,
		.flags = IORESOURCE_MEM,
	},
/* BE 3DME */
	[6] = {
		.start = DRIME4_PA_BAYER + 0x6000,
		.end = DRIME4_PA_BAYER + 0x6000 + SZ_4K - 1,
		.flags = IORESOURCE_MEM,
	},
/* BE DMA */
	[7] = {
		.start = DRIME4_PA_BAYER + 0x7000,
		.end = DRIME4_PA_BAYER + 0x7000 + SZ_4K - 1,
		.flags = IORESOURCE_MEM,
	},

/* CORE IRQ */
	[9] = {
		.start = IRQ_BAYERE,
		.end = IRQ_BAYERE,
		.flags = IORESOURCE_IRQ,
	},

};

static u64 drime4_be_dma_mask = DMA_BIT_MASK(32);

struct platform_device drime4_device_be = {
		.name = BE_MODULE_NAME,
		.id = 0,
		.num_resources = ARRAY_SIZE(drime4_be_resource),
		.resource = drime4_be_resource,
		.dev = {
			.dma_mask = &drime4_be_dma_mask,
			.coherent_dma_mask = DMA_BIT_MASK(32),
		},
};

/*static struct be_platform_data drime4_be_pd __initdata = {
	.clk_name = "be",
};*/

/*void __init drime4_be_init()
{
	struct be_platform_data *pd = NULL;

	pd = kmemdup(&drime4_be_pd, sizeof(struct be_platform_data), GFP_KERNEL);

	if (!pd)
		printk(KERN_ERR "%s:failed to allocate Bayer_Engine_platform_data.\n", __func__);

	drime4_device_be.dev.platform_data = (void *)pd;
}*/

