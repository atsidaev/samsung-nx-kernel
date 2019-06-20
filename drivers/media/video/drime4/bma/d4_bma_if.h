 /**
 * @file d4_bma_if.h
 * @brief DRIMe4 BMA Device Driver Header
 * @author Junkwon Choi <junkwon.choi@samsung.com>
 * 2012 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifndef __BMA_IF_DD_H__
#define __BMA_IF_DD_H__

#include  <media/drime4/bma/d4_bma_type.h>

void d4_bma_set_dev_info(struct device *info);
int d4_bma_alloc_buf(struct BMA_Buffer_Info *info);
int d4_bma_free_buf(unsigned int addr);

#endif /* __BMA_IF_DD_H__ */
