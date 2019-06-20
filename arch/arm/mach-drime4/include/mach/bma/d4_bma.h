/**
 * @file d4_bma.h
 * @brief DRIMe4 BMA
 * @author Junkwon Choi <junkwon.choin@samsung.com>
 * 2012 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifndef __BMA_H__
#define __BMA_H__

#define BMA_MODULE_NAME	"d4_bma"

/******************************************************************************/
/*                                Structure                                   */
/******************************************************************************/
struct drime4_bma {
	struct device *dev;
	const char *name;
	int id;
	struct clk *clock;
	int irq;
	volatile int ref_count;
	int irqlock;
};

#endif /* __BMA_H__ */
