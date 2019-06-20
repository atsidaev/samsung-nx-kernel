/**
 * @file dev-srp.c
 * @brief DRIMe4 SRP platform device definitions
 * @author Geunjae Yu <geunjae.yu@samsung.com>
 * 2011 Samsung Electronics
 *linux/arch/arm/mach-drime4/dev-srp.c
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/platform_device.h>
#include <mach/irqs.h>
#include <mach/map.h>
#include <mach/srp/d4_srp.h>
#include <asm/sizes.h>

static struct resource drime4_srp_resource[] = {
/* SPM + CMEM + REG */
	[0] = {
		.start = DRIME4_PA_SONICS_SRP,
		.end   = DRIME4_PA_SONICS_SRP + SZ_1M - 1,
		.flags = IORESOURCE_MEM,
	},
/* DAP */
	[1] = {
		.start = DRIME4_PA_SRP_DAP,
		.end   = DRIME4_PA_SRP_DAP + SZ_32K - 1,
		.flags = IORESOURCE_MEM,
	},
/* COMMBOX */
	[2] = {
		.start = DRIME4_PA_SRP_COMMBOX,
		.end   = DRIME4_PA_SRP_COMMBOX + SZ_32K - 1,
		.flags = IORESOURCE_MEM,
		},
/* Shared SRAM */
	[3] = {
		.start = DRIME4_PA_SONICS_MSRAM,
		.end   = DRIME4_PA_SONICS_MSRAM + SZ_128K - 1,
		.flags = IORESOURCE_MEM,
	},
/* IRQ */
	[4] = {
		.start = IRQ_CODEC_SRP,
		.end   = IRQ_CODEC_SRP,
		.flags = IORESOURCE_IRQ,
	},

};

static struct drime4_srp_dev_data srp_device_data = {
	.param0 = 0x12345678,
	.param1 = 0x44444444,
	.param2 = 0x88888888,
	.param3 = 0xcccccccc
};

struct platform_device drime4_device_srp = {
	.name		= SRP_MODULE_NAME,
	.id		= -1,
	.num_resources	= ARRAY_SIZE(drime4_srp_resource),
	.resource	= drime4_srp_resource,
	.dev		= {
				.platform_data = &srp_device_data,
	},
};

