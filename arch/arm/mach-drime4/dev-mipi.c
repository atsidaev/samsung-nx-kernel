/**
 * @file dev-mipi.c
 * @brief DRIMe4 MIPI Platform Device File
 * @author Gunwoo Nam <gunwoo.nam@samsung.com>
 * 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/platform_device.h>
#include <mach/irqs.h>
#include <mach/dma.h>
#include <mach/map.h>
#include <mach/mipi/d4_mipi.h>

static struct resource drime4_mipi_resource[] = {
	[0] = {
		.start = DRIME4_PA_MIPI_CON,
		.end   = DRIME4_PA_MIPI_CON + DRIME4_MIPI_CON_SIZE - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = DRIME4_PA_MIPI_CSIM,
		.end   = DRIME4_PA_MIPI_CSIM + DRIME4_MIPI_CSIM_SIZE - 1,
		.flags = IORESOURCE_MEM,
	},
	[2] = {
		.start = DRIME4_PA_MIPI_CSIS,
		.end   = DRIME4_PA_MIPI_CSIS + DRIME4_MIPI_CSIS_SIZE - 1,
		.flags = IORESOURCE_MEM,
	},
	[3] = {
		.start = IRQ_MIPI_RX,
		.end   = IRQ_MIPI_RX,
		.flags = IORESOURCE_IRQ,
	},
	[4] = {
		.start = IRQ_MIPI_TX,
		.end   = IRQ_MIPI_TX,
		.flags = IORESOURCE_IRQ,
	},
	[5] = {
		.start = DMACH_MIPI_TX,
		.end   = DMACH_MIPI_TX,
		.flags = IORESOURCE_DMA,
	},
};

static struct drime4_mipi_dev_data mipi_device_data = {
	.param0 = 0x12345678,
	.param1 = 0x44444444,
	.param2 = 0x88888888,
	.param3 = 0xcccccccc
};

struct platform_device drime4_device_mipi = {
	.name		    = MIPI_MODULE_NAME,
	.id		        = -1,
	.num_resources	= ARRAY_SIZE(drime4_mipi_resource),
	.resource	    = drime4_mipi_resource,
	.dev		    = {
	.platform_data = &mipi_device_data,
	},
};


