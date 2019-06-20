/**
 * @file dev-alsa_hdmi.c
 * @brief DRIMe4 ALSA HDMI Platform Device
 * @author Byungho Ahn <bh1212.ahn@samsung.com>
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

#include <mach/alsa_hdmi/d4_alsa_hdmi.h>


static struct resource drime4_alsa_hdmi_resource[] = {
};


struct platform_device drime4_device_alsa_hdmi = {
	.name		= ALSA_HDMI_MODULE_NAME,
	.id		= -1,
	.num_resources	= ARRAY_SIZE(drime4_alsa_hdmi_resource),
	.resource	= drime4_alsa_hdmi_resource,
	.dev		= {
/*		.platform_data = &alsa_hdmi_device_data,*/
	},
};


