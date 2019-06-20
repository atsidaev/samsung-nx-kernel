/**
 * @file dev-gpu.c
 * @brief DRIMe4 GPU Platform Device
 * @author Byungho Ahn <bh1212.ahn@samsung.com>
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

#include <mach/gpu/d4_gpu.h>


static struct resource drime4_gpu_resource[] = {
	[0] = {
		.start = DRIME4_PA_SONICS_GPU,
		.end   = DRIME4_PA_SONICS_GPU + 0x1000 - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = IRQ_GPU_C,
		.end   = IRQ_GPU_C,
		.flags = IORESOURCE_IRQ,
	},
};


struct platform_device powervr_device = {
	.name		= GPU_MODULE_NAME,
	.id		= 0,
	.num_resources	= ARRAY_SIZE(drime4_gpu_resource),
	.resource	= drime4_gpu_resource,
	.dev		= {
	},
};

