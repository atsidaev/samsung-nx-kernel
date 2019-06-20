/**
 * @file d4_opener.h
 * @brief DRIMe4 Driver Opener
 * @author Wooram Son <wooram.son@samsung.com>
 * 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifndef __OPENER_H__
#define __OPENER_H__

#define OPENER_MODULE_NAME		"d4_opener"

/******************************************************************************/
/*                                Structure                                   */
/******************************************************************************/
struct drime4_opener {
   struct device *dev;
   const char *name;
   int id;
   struct clk *clock;
   int irq;
   volatile int ref_count;
   int irqlock;
};

#endif /* __OPENER_H__ */
