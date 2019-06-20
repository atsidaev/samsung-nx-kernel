/**
 * @file d4_jxr.h
 * @brief DRIMe4 JPEG_XR Device Driver Header
 * @author JinHyoung An <jh0913.an@samsung.com>
 * 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifndef __JXR_H__
#define __JXR_H__

#define JXR_MODULE_NAME		"d4_jpeg_xr"

/******************************************************************************/
/*                                Structure                                   */
/******************************************************************************/

struct drime4_jxr {
   struct device *dev;
   const char *name;
   int id;
   struct clk *clock;
   int irq;
   volatile int ref_count;
   int irqlock;
   struct drime4_jxr_dev_data *pd;
};

struct drime4_jxr_dev_data{
   unsigned int param0;
   unsigned int param1;
   unsigned int param2;
   unsigned int param3;
};
#ifdef __cplusplus
}
#endif

#endif /* __JXR_H__ */

