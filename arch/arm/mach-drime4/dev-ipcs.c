/**
 * @file	dev-ipcs.c
 * @brief	IPCS Base DRIME4 platform device definitions
 *
 * @author	Dongjin Jung <djin81.jung@samsung.com>
 *
 * Copyright (c) 2012 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
*/

#include <linux/platform_device.h>
#include <mach/irqs.h>
#include <mach/map.h>
#include <mach/ipcs/d4_ipcs.h>

static struct resource drime4_ipcs_resource[] = {
	[0] = {
		.start = DRIME4_PA_IPCS,
		.end   = DRIME4_PA_IPCS + DRIME4_IPCS_OFF,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = IRQ_IPC_IPCS,
		.end   = IRQ_IPC_IPCS,
		.flags = IORESOURCE_IRQ,
	},

};

struct platform_device drime4_device_ipcs = {
	.name			= IPCS_MODULE_NAME,
	.id				= -1,
	.num_resources	= ARRAY_SIZE(drime4_ipcs_resource),
	.resource		= drime4_ipcs_resource,
	.dev			= {
	}
};
