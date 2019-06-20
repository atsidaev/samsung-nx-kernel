/**
 * @file d4_sma_ioctl.h
 * @brief DRIMe4 SMA IOCTL Interface Header File
 * @author Junkwon Choi <junkwon.choi@samsung.com>
 * 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __D4_SMA_IOCTL_H__
#define __D4_SMA_IOCTL_H__

#include "d4_sma_type.h"

#define SMA_MAGIC 's'

#define SMA_GET_REGION_SIZE _IOR(SMA_MAGIC, 1, unsigned int) /**< Get SMA region size info */
#define SMA_VIRT_TO_PHYS _IOWR(SMA_MAGIC, 2, unsigned int) /**< get physical memory address */
#define SMA_SET_CACHE  _IOWR(SMA_MAGIC, 3, int)
#define SMA_GET_REGION_START_ADDR _IOR(SMA_MAGIC, 4, unsigned int) /**< Get SMA region Start Addr */
#define SMA_ALLOC _IOWR(SMA_MAGIC, 5, struct SMA_Buffer_Info) /**< Allocation Buffer */
#define SMA_FREE _IOWR(SMA_MAGIC, 6, unsigned int) /**< Free Buffer - user virtual memory address*/
#define SMA_FREE_PHYS _IOWR(SMA_MAGIC, 7, unsigned int) /**< Free Buffer - physical memory address */
#define SMA_GET_ALLOCATED_SIZE _IOR(SMA_MAGIC, 8, unsigned int) /**< Get SMA allocated size info */
#define SMA_CACHE_FLUSH _IOWR(SMA_MAGIC, 9, struct SMA_Buffer_Info) 	/**< Cache Flush> */
#define SMA_SET_REMAIN_BUF _IOWR(SMA_MAGIC, 10, struct SMA_Buffer_Info) 	/**< Set SMA Remain size info(Temp) */

#endif /* __D4_SMA_IOCTL_H__ */
