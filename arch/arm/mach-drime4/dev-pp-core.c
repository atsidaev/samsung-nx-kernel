/**
 * @file dev-pp-core.c
 * @brief DRIMe4 PP Core Platform Device File
 * @author Sunghoon Kim <bluesay.kim@samsung.com>
 * 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/platform_device.h>
#include <mach/irqs.h>
#include <mach/map.h>
#include <mach/pp/pp_core/d4_pp_core.h>

static struct resource drime4_pp_core_resource[] = {
	[0] = {
		.start = DRIME4_PA_PP,
		.end   = DRIME4_PA_PP + DRIME4_PP_CORE_COMMON_SIZE - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = DRIME4_PA_PP + DRIME4_PP_DMA_OFFSET,
		.end   = DRIME4_PA_PP + DRIME4_PP_DMA_OFFSET + DRIME4_PP_DMA_SIZE - 1,
		.flags = IORESOURCE_MEM,
	},
	[2] = {
		.start = DRIME4_PA_PP + DRIME4_PP_CORE_CTRL_OFFSET,
		.end   = DRIME4_PA_PP + DRIME4_PP_CORE_CTRL_OFFSET + DRIME4_PP_CORE_CTRL_SIZE - 1,
		.flags = IORESOURCE_MEM,
	},
	[3] = {
		.start = IRQ_PP_CORE,
		.end   = IRQ_PP_CORE,
		.flags = IORESOURCE_IRQ,
	},

};

static struct drime4_pp_core_dev_data pp_core_device_data = {
	.param0 = 0x12345678,
	.param1 = 0x44444444,
	.param2 = 0x88888888,
	.param3 = 0xcccccccc
};

struct platform_device drime4_device_pp_core = {
	.name		    = PP_CORE_MODULE_NAME,
	.id		        = -1,
	.num_resources	= ARRAY_SIZE(drime4_pp_core_resource),
	.resource	    = drime4_pp_core_resource,
	.dev		    = {
				.platform_data = &pp_core_device_data,
	},
};

