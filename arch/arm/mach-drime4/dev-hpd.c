/**
 * @file dev-hpd.c
 * @brief DRIMe4 HDMI HPD Platform Device
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

#include <mach/hdmi/video/d4_hdmi_video.h>

static struct resource drime4_hdmi_hpd_resource[] = {
};

static u64 drime4_hdmi_dma_mask = DMA_BIT_MASK(32);

struct platform_device drime4_device_hdmi_hpd = {
		.name = HDMI_HPD_MODULE_NAME,
		.id = 0,
		.num_resources = ARRAY_SIZE(drime4_hdmi_hpd_resource),
		.resource = drime4_hdmi_hpd_resource,
		.dev = {
			.dma_mask = &drime4_hdmi_dma_mask,
			.coherent_dma_mask = DMA_BIT_MASK(32),
		},
};

