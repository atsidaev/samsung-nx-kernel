/**
 * @file dev-ipcm.c
 * @brief DRIMe4 IPCM platform device definitions
 * @author TaeWook Nam <tw.@samsung.com>
 * 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/platform_device.h>
#include <mach/irqs.h>
#include <mach/map.h>
#include <mach/ipcm/d4_ipcm.h>

static struct resource drime4_ipcm_resource[] = {
	[0] = {
		.start = DRIME4_PA_IPCM,
		.end   = DRIME4_PA_IPCM + DRIME4_IPCM_OFF,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = IRQ_IPC_IPCM,
		.end   = IRQ_IPC_IPCM,
		.flags = IORESOURCE_IRQ,
	},
	[2] = {
		.start = IRQ_IPC_MD,
		.end   = IRQ_IPC_MD,
		.flags = IORESOURCE_IRQ,
	},
	[3] = {
		.start = IRQ_IPC_LDC,
		.end   = IRQ_IPC_LDC,
		.flags = IORESOURCE_IRQ,
	},

};

struct platform_device drime4_device_ipcm = {
	.name		  = IPCM_MODULE_NAME,
	.id		  = -1,
	.num_resources	  = ARRAY_SIZE(drime4_ipcm_resource),
	.resource	  = drime4_ipcm_resource,
};
