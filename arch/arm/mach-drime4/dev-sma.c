/**
 * @file dev-sma.c
 * @brief DRIMe4 SMA Device File
 * @author Junkwon Choi <junkwon.choi@samsung.com>
 * 2012 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/
#include <linux/platform_device.h>
#include <mach/irqs.h>
#include <mach/map.h>
#include <mach/sma/d4_sma.h>
#include <linux/dma-mapping.h>

static u64 sma_dma_mask = DMA_BIT_MASK(32);

struct platform_device drime4_device_sma = {
   .name	    = SMA_MODULE_NAME,
   .id		    = -1,
   .num_resources   = 0,
	.dev	= {
		.dma_mask		= &sma_dma_mask,
		.coherent_dma_mask	= DMA_BIT_MASK(32),
	}
};

