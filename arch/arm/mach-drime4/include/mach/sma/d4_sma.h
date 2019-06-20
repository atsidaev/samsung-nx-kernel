/**
 * @file d4_sma.h
 * @brief DRIMe4 SMA
 * @author Junkwon Choi <junkwon.choin@samsung.com>
 * 2012 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifndef __SMA_H__
#define __SMA_H__

#define SMA_MODULE_NAME		"d4_sma"

/******************************************************************************/
/*                                Structure                                   */
/******************************************************************************/
struct drime4_sma {
   struct device *dev;
   const char *name;
   int id;
   struct clk *clock;
   int irq;
   volatile int ref_count;
   int irqlock;
};


#endif /* __SMA_H__ */
