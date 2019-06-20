/**
 * @file dev-pp-3a.c
 * @brief DRIMe4 PP 3A Platform Device File
 * @author Kyoung Hwan Moon <kh.moon@samsung.com>
 * 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/platform_device.h>
#include <mach/irqs.h>
#include <mach/map.h>
#include <mach/pp/pp_3a/d4_pp_3a.h>
#include <linux/dma-mapping.h>

static u64 pp_3a_dma_mask = DMA_BIT_MASK(32);

static struct resource drime4_pp_3a_resource[] = {
	[0] = {
		.start = DRIME4_PA_PP_3A,
		.end   = DRIME4_PA_PP_3A + DRIME4_PP_3A_OFF - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = IRQ_PP_3A,
		.end   = IRQ_PP_3A,
		.flags = IORESOURCE_IRQ,
	},

};

static struct drime4_pp_3a_dev_data pp_3a_device_data = {
	.param0 = 0x12345678,
	.param1 = 0x44444444,
	.param2 = 0x88888888,
	.param3 = 0xcccccccc
};

struct platform_device drime4_device_pp_3a = {
	.name		  = PP_3A_MODULE_NAME,
	.id		  = 0,
	.num_resources	  = ARRAY_SIZE(drime4_pp_3a_resource),
	.resource	  = drime4_pp_3a_resource,
	.dev		= {
				.platform_data = &pp_3a_device_data,
				.dma_mask	=	&pp_3a_dma_mask,
				.coherent_dma_mask	=	DMA_BIT_MASK(32),
	},
};
