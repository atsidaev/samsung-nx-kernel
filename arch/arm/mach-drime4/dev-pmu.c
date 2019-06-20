/* linux/arch/arm/mach-drime4/dev-spi.c
 *
 * Copyright (c) 2011 Samsung Electronics
 *	Hwan Soon Sung <hs2704.sung@samsung.com>
 *
 * Base DRIME4 platform device definitions
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
*/

#include <linux/platform_device.h>
#include <linux/dma-mapping.h>
#include <mach/irqs.h>
#include <mach/map.h>
#include <mach/dma.h>

#define DRIME4_PMU_OFFSET	0xC0

static struct resource d4_pmu_resource[] = {
	[0] = {
		.start = DRIME4_PA_PMU,
		.end   = DRIME4_PA_PMU + DRIME4_PMU_OFFSET - 1,
		.flags = IORESOURCE_MEM,
	},

	[1] = {
		.start = DRIME4_PA_SONICS_CONF,
		.end   = DRIME4_PA_SONICS_CONF + 0x5000 - 1,
		.flags = IORESOURCE_MEM,
	},
/*
	[2] = {
		.start = DRIME4_PA_UART + SZ_4K,
		.end   = DRIME4_PA_UART + SZ_8K - 1,
		.flags = IORESOURCE_MEM,
	},

	[3] = {
		.start = DMACH_UART1_TX,
		.end   = DMACH_UART1_TX,
		.flags = IORESOURCE_DMA,
	},
*/
};

struct platform_device d4_device_pmu = {
	.name		= "drime4_pmu",
	.id		= 1,
	.num_resources	= ARRAY_SIZE(d4_pmu_resource),
	.resource	= d4_pmu_resource,
};
