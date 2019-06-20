/**
 * @file dev-pp-ssif.c
 * @brief DRIMe4 PP Sensor Interface Platform Device File
 * @author DeokEun Cho <de.cho@samsung.com>
 * 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/platform_device.h>
#include <mach/irqs.h>
#include <mach/map.h>
#include <mach/pp/pp_ssif/d4_pp_ssif.h>

static struct resource drime4_pp_ssif_resource[] = {
	[0] = {
		.start = DRIME4_PA_PP_SSIF,
		.end   = DRIME4_PA_PP_SSIF + DRIME4_PP_SSIF_SIZE - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = DRIME4_PA_SLVDS,
		.end   = DRIME4_PA_SLVDS + DRIME4_SLVDS_SIZE - 1,
		.flags = IORESOURCE_MEM,
	},
	[2] = {
		.start = IRQ_PP_SSIF,
		.end   = IRQ_PP_SSIF,
		.flags = IORESOURCE_IRQ,
	},

};

static struct drime4_pp_ssif_dev_data pp_ssif_device_data = {
	.param0 = 0x12345678,
	.param1 = 0x44444444,
	.param2 = 0x88888888,
	.param3 = 0xcccccccc
};

struct platform_device drime4_device_pp_ssif = {
	.name           = PP_SSIF_MODULE_NAME,
	.id             = 0,
	.num_resources  = ARRAY_SIZE(drime4_pp_ssif_resource),
	.resource       = drime4_pp_ssif_resource,
	.dev    = {
	    .platform_data = &pp_ssif_device_data,
	},
};
