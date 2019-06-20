/**
 * @file	d4_ipcs.h
 * @brief	IPCS device header for Samsung DRIMe4 Camera Interface driver
 *
 * @author	Donjin Jung <djin81.jung@samsung.com>,
 * Copyright (c) 2012 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __D4_IPCS_H__
#define __D4_IPCS_H__

#define IPCS_MODULE_NAME	"d4_ipcs"

/*
 * @struct	drime4_ipcs
 * @brief	Main context
 */
struct drime4_ipcs {
	struct device *dev;
	const char *name;
	int id;
	struct clk *clock;
	/*void __iomem *reg_base;*/
	/*int irq;*/
	atomic_t ref_count;
	spinlock_t irqlock;
	struct drime4_ipcs_dev_data *pd;
};

struct drime4_ipcs_dev_data {
	unsigned int param0;
	unsigned int param1;
	unsigned int param2;
	unsigned int param3;
};

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#endif /* __D4_IPCS_H__ */
