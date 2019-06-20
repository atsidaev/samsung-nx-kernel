/**
 * @file d4_jpeg.h
 * @brief jpeg Device Driver Header
 * @author JinHyoung An <jh0913.an@samsung.com>
 * 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifndef __JPEG_H__
#define __JPEG_H__

#define JPEG_MODULE_NAME		"d4_jpeg"

/******************************************************************************/
/*                                Structure                                   */
/******************************************************************************/
struct drime4_jpeg_dev_data {
   unsigned int param0;
   unsigned int param1;
   unsigned int param2;
   unsigned int param3;
};

struct drime4_jpeg {
   struct device *dev;
   const char *name;
   int id;
   struct clk *clock;
   int irq;
   volatile int ref_count;
   int irqlock;
   struct drime4_jpeg_dev_data *pd;
};


#endif /* __JPEG_H__ */

