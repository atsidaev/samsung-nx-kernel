/**
 * @file dev-ep.c
 * @brief DRIMe4 HDMI Platform Device
 * @author Somabha Bhattacharjya <b.somabha@samsung.com>
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
#include <mach/gpio.h>

#include <mach/hdmi/video/d4_hdmi_video.h>
#include <drm/drime4_drm.h>

static struct resource drime4_hdmi_resource[] = {
	[0] = {
		.start = DRIME4_PA_HDMI,
		.end = DRIME4_PA_HDMI + SZ_4K - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = IRQ_HDMI,
		.end = IRQ_HDMI,
		.flags = IORESOURCE_IRQ,
	},
};

static u64 drime4_hdmi_dma_mask = DMA_BIT_MASK(32);

static struct drime4_drm_hdmi_pdata drm_hdmi_pdata = {
		.hpd_gpio		= DRIME4_GPIO0(1),		/* hpd gpio */
        .cfg_hpd        = NULL,
        .get_hpd        = NULL,
};

struct platform_device drime4_device_hdmi = {
		.name = HDMI_MODULE_NAME,
		.id = 0,
		.num_resources = ARRAY_SIZE(drime4_hdmi_resource),
		.resource = drime4_hdmi_resource,
		.dev = {
			.dma_mask = &drime4_hdmi_dma_mask,
			.coherent_dma_mask = DMA_BIT_MASK(32),
			.platform_data = &drm_hdmi_pdata,
		},
};

