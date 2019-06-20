/**
 * @file d4_ipcm.h
 * @brief DRIMe4 IPCM Driver Header
 * @author TaeWook Nam <tw.@samsung.com>
 * 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __D4_IPCM_H__
#define __D4_IPCM_H__

#define IPCM_MODULE_NAME	"d4_ipcm"

/*
 * @brief Main context
 */
struct drime4_ipcm {
	struct device *dev;
	const char *name;
	int id;
	struct clk *clock;

	atomic_t ref_count;
	spinlock_t irqlock;
};

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#endif /* __D4_IPCM_H__ */
