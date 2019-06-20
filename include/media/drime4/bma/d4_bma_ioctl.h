/**
 * @file d4_bma_ioctl.h
 * @brief DRIMe4 BMA IOCTL Interface Header File
 * @author Junkwon Choi <junkwon.choi@samsung.com>
 * 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include "d4_bma_type.h"

#ifndef __D4_BMA_IOCTL_H__
#define __D4_BMA_IOCTL_H__

#define BMA_MAGIC 'b'

#define BMA_ALLOC _IOWR(BMA_MAGIC, 1, struct BMA_Buffer_Info) /**< Allocation Buffer */
#define BMA_FREE _IOWR(BMA_MAGIC, 2, unsigned int) /**< Free Buffer - user virtual memory address*/
#define BMA_VIRT_TO_PHYS _IOWR(BMA_MAGIC, 3, unsigned int) /**< get physical memory address */
#define BMA_FREE_PHYS _IOWR(BMA_MAGIC, 4, unsigned int) /**< Free Buffer - physical memory address */

#endif /* __D4_BMA_IOCTL_H__ */
