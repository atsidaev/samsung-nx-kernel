/* linux/arch/arm/mach-drime4/dev-mfc.c
 *
 * Copyright (c) 2011 Samsung Electronics
 *
 * Base S5P MFC resource and device definitions
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/ioport.h>
#include <linux/dma-mapping.h>

#include <mach/map.h>
#include <mach/devs.h>
#include <mach/irqs.h>
#include <mach/regs-codec_fb.h>

#ifdef CONFIG_VIDEO_MFC5X

static struct drime4_codec_framebuffer_info codec_fb_pdata[] = {
	[0] = {
		.name = "CIF 352*288",
		.partition = (8 << 12) | (8 << 6) | 6,
		.mbx_tile_info = (2 << 4) | (1),
		.address_range_luma = 0x1f000,
		.address_range_chroma = 0xff00,
		.img_width = 352,
		.img_height = 288,
		.img_mask = 0xffffffff,
	},
	[1] = {
		.name = "VGA 640*480",
		.partition = (16 << 12) | (16 << 6) | 8,
		.mbx_tile_info = (3 << 4) | (2),
		.address_range_luma = 0x4f000,
		.address_range_chroma = 0x27fff,
		.img_width = 640,
		.img_height = 480,
		.img_mask = 0xffffffff,
	},
	[2] = {
		.name = "HD 1920*1080",
		.partition = (40 << 12) | (40 << 6) | 40,
		.mbx_tile_info = (8 << 4) | (7),
		.address_range_luma = 0x1fe000,
		.address_range_chroma = 0xffc00,
		.img_width = 1920,
		.img_height = 1088,
		.img_mask = 0xffffffff,
	},
	[3] = {
		.name = "720P 1280*720",
		.partition = (32 << 12) | (32 << 6) | 16,
		.mbx_tile_info = (5 << 4) | (5),
		.address_range_luma = 0xe6000,
		.address_range_chroma = 0x78000,
		.img_width = 1280,
		.img_height = 720,
		.img_mask = 0xffffffff,
	},
	[4] = {
		.name = "SD 720*480",
		.partition = (16 << 12) | (16 << 6) | 13,
		.mbx_tile_info = (3 << 4) | (3),
		.address_range_luma = 0x5a000,
		.address_range_chroma = 0x2ffff,
		.img_width = 1280,
		.img_height = 720,
		.img_mask = 0xffffffff,
	},
	[5] = {
		.name = "HD 1920*810",
		.partition = (40 << 12) | (40 << 6) | 40,
		.mbx_tile_info = (8 << 4) | (7),
		.address_range_luma = 0x186000,
		.address_range_chroma = 0xc3c00,
		.img_width = 1920,
		.img_height = 816,
		.img_mask = 0xffffffff,
	},
	/* TODO: add more format */
};

static struct resource s5p_mfc_resource[] = {
	[0] = {
		.start  = DRIME4_PA_CODEC,
		.end    = DRIME4_PA_CODEC + SZ_64K - 1,
		.flags  = IORESOURCE_MEM,
	},
	[1] = {
		.start  = IRQ_CODEC_MFC,
		.end    = IRQ_CODEC_MFC,
		.flags  = IORESOURCE_IRQ,
	}
};

static u64 s5p_mfc_dma_mask = DMA_BIT_MASK(32);

struct platform_device drime4_device_mfc = {
	.name		= "s5p-mfc",
	.id		= -1,
	.num_resources	= ARRAY_SIZE(s5p_mfc_resource),
	.resource	= s5p_mfc_resource,
	.dev		= {
		.dma_mask		= &s5p_mfc_dma_mask,
		.coherent_dma_mask	= DMA_BIT_MASK(32),
		.platform_data		= codec_fb_pdata,
	},
};
#endif
