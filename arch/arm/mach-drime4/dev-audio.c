/* linux/arch/arm/mach-drime4/dev-audio.c
 *
 * Copyright (c) 2011 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com/
 *
 * DRIME4 - AUDIO support
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/platform_device.h>
#include <linux/dma-mapping.h>
#include <linux/gpio.h>

#include <mach/map.h>
#include <mach/dma.h>
#include <mach/irqs.h>
#include <mach/audio.h>

static char *rclksrc[] = {
	[0] = "i2s",
	[1] = "audio-bus",
};

static int drime4_cfg_i2s(struct platform_device *pdev)
{
	/* TODO: */
	return 0;
}

static struct drime4_audio_pdata i2s_pdata = {
	.cfg_gpio = drime4_cfg_i2s,
	.type = {
		.i2s = {
			.quirks = QUIRK_PRI_6CHAN,
			.src_clk = rclksrc,
		},
	},
};

static struct resource drime4_i2s0_resource[] = {
	[0] = {
		.start = DRIME4_PA_I2S0,
		.end   = DRIME4_PA_I2S0 + SZ_4K - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = DMACH_I2S0_TX,
		.end   = DMACH_I2S0_TX,
		.flags = IORESOURCE_DMA,
	},
	[2] = {
		.start = DMACH_I2S0_RX,
		.end   = DMACH_I2S0_RX,
		.flags = IORESOURCE_DMA,
	},
};

struct platform_device drime4_device_i2s0 = {
	.name = "drime4-i2s",
	.id = 0,
	.num_resources	  = ARRAY_SIZE(drime4_i2s0_resource),
	.resource	  = drime4_i2s0_resource,
	.dev = {
		.platform_data = &i2s_pdata,
	},
};

static struct resource drime4_i2s1_resource[] = {
	[0] = {
		.start = DRIME4_PA_I2S1,
		.end   = DRIME4_PA_I2S1 + SZ_4K - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = DMACH_I2S1_TX,
		.end   = DMACH_I2S1_TX,
		.flags = IORESOURCE_DMA,
	},
	[2] = {
		.start = DMACH_I2S1_RX,
		.end   = DMACH_I2S1_RX,
		.flags = IORESOURCE_DMA,
	},
};

struct platform_device drime4_device_i2s1 = {
	.name = "drime4-i2s",
	.id = 1,
	.num_resources	  = ARRAY_SIZE(drime4_i2s1_resource),
	.resource	  = drime4_i2s1_resource,
	.dev = {
		.platform_data = &i2s_pdata,
	},
};

static struct resource drime4_i2s2_resource[] = {
	[0] = {
		.start = DRIME4_PA_I2S2,
		.end   = DRIME4_PA_I2S2 + SZ_4K - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = DMACH_I2S2_TX,
		.end   = DMACH_I2S2_TX,
		.flags = IORESOURCE_DMA,
	},
	[2] = {
		.start = DMACH_I2S2_RX,
		.end   = DMACH_I2S2_RX,
		.flags = IORESOURCE_DMA,
	},
};

struct platform_device drime4_device_i2s2 = {
	.name = "drime4-i2s",
	.id = 2,
	.num_resources	  = ARRAY_SIZE(drime4_i2s2_resource),
	.resource	  = drime4_i2s2_resource,
	.dev = {
		.platform_data = &i2s_pdata,
	},
};

static u64 drime4_asoc_dmamask = DMA_BIT_MASK(32);

struct platform_device drime4_asoc_dma = {
	.name		  = "drime4-audio",
	.id		  = -1,
	.dev              = {
		.dma_mask = &drime4_asoc_dmamask,
		.coherent_dma_mask = DMA_BIT_MASK(32),
	}
};
