 /**
 * @file d4_sma_if.h
 * @brief DRIMe4 SMA Device Driver Header
 * @author Junkwon Choi <junkwon.choi@samsung.com>
 * 2012 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifndef __SMA_IF_DD_H__
#define __SMA_IF_DD_H__

#include  <media/drime4/sma/d4_sma_type.h>

void d4_sma_set_base_addr(unsigned int addr);
unsigned int d4_sma_get_base_addr(void);
void d4_sma_set_dev_info(struct device *info);
int d4_sma_alloc_buf(struct SMA_Buffer_Info *info);
int d4_sma_free_buf(unsigned int addr);
unsigned int d4_sma_get_allocated_memory_size(void);

#define SMA_TEMP_RESERVED_AREA

#if defined( SMA_TEMP_RESERVED_AREA )
//Temp I/F
void d4_sma_set_remain_buf(struct SMA_Buffer_Info *info);
int d4_sma_get_remain_buf(struct SMA_Buffer_Info *info);
#endif


#endif /* __SMA_IF_DD_H__ */
