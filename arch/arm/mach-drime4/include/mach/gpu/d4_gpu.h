/**
 * @file d4_gpu.h
 * @brief DRIMe4 GPU Platform Driver Header File
 * @author Byungho Ahn <bh1212.ahn@samsung.com>
 * 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef _D4_GPU_H_
#define _D4_GPU_H_

#define GPU_MODULE_NAME		"pvrsrvkm"

struct drime4_gpu {
	struct device			*dev;
	const char			*name;
	int				id;
	struct clk			*clock;
	int				irq;
	atomic_t			ref_count;
	spinlock_t			irqlock;
	struct drime4_gpu_dev_data	*pd;
};

#endif
