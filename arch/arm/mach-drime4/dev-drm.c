/*
 * linux/arch/arm/mach-exynos/dev-drm.c
 *
 * Copyright (c) 2012 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com
 *
 * EXYNOS - core DRM device
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <linux/kernel.h>
#include <linux/dma-mapping.h>
#include <linux/platform_device.h>

static u64 drime4_drm_dma_mask = DMA_BIT_MASK(32);

struct platform_device drime4_device_drm = {
	.name	= "drime4-drm",
	.dev	= {
		.dma_mask		= &drime4_drm_dma_mask,
		.coherent_dma_mask	= DMA_BIT_MASK(32),
	}
};
