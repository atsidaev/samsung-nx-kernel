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
#include <linux/input.h>
#include <mach/hdmi/video/d4_hdmi_video.h>

struct d4_cec_remote_button {
	int code;
	int type;
};

struct d4_cec_but_platform_data {
	struct d4_cec_remote_button *buttons;
	int butsize;
};

static struct d4_cec_remote_button d4_cec_buttons[] = {
		{ .code = 0x84, .type =	EV_KEY, },
		{ .code = 0x8c, .type = EV_KEY, },
		{ .code = 0x9f, .type = EV_KEY, },
		{ .code = 0x8f, .type = EV_KEY, },
		{ .code = 0x80, .type = EV_KEY, },
		{ .code = 0xb5,  .type = EV_KEY, }, /* KEY_SELECT */
		{ .code = 0xb6,	.type = EV_KEY, },	/* KEY_UP */
		{ .code = 0xb7,	.type = EV_KEY, },	/* KEY_DOWN */
		{ .code = 0xb8,	.type = EV_KEY, },	/* KEY_LEFT */
		{ .code = 0xb9,	.type = EV_KEY, },	/* KEY_RIGHT */
		{ .code = 0xba, .type = EV_KEY, },	/* KEY_EXIT */
		{ .code = 0xbc, .type = EV_KEY, },	/* KEY_RED */
		{ .code = 0xbd, .type = EV_KEY, },	/* KEY_GREEN */
		{ .code = 0xbe, .type = EV_KEY, },	/* KEY_YELLOW */
		{ .code = 0xbb, .type = EV_KEY, },	/* KEY_BLUE */
};


static struct d4_cec_but_platform_data d4_cec_keys_data = { .buttons =
		d4_cec_buttons, .butsize = ARRAY_SIZE(d4_cec_buttons), };

static struct resource drime4_hdmi_cec_resource[] = { [0] = { .start =
		DRIME4_PA_HDMI + 0x30000, .end = DRIME4_PA_HDMI + 0x30000 + SZ_4K - 1,
		.flags = IORESOURCE_MEM, },
		[1] = { .start = IRQ_HDMI, .end = IRQ_HDMI,
		.flags = IORESOURCE_IRQ, }, };

static u64 drime4_hdmi_dma_mask = DMA_BIT_MASK(32);

struct platform_device drime4_device_cec = { .name = HDMI_CEC_MODULE_NAME,
		.id = 0, .num_resources = ARRAY_SIZE(drime4_hdmi_cec_resource),
		.resource = drime4_hdmi_cec_resource, .dev = { .dma_mask =
				&drime4_hdmi_dma_mask, .coherent_dma_mask = DMA_BIT_MASK(32),
				.platform_data = &d4_cec_keys_data, }, };

