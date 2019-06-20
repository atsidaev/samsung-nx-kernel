/* d4_cma.h
 *
 * Copyright (c) 2012 Samsung Electronics
 *	Sujin Ryu <sujin81.ryu@samsung.com>
 *
 *   DRIMe4 CMA Header.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __D4_CMA_H__
#define __D4_CMA_H__

#include <linux/mm.h>
#include <linux/dma-mapping.h>
#include <linux/list.h>

#define DEBUG_MEM_ALLOC

#ifdef DEBUG_MEM_ALLOC
	#define DMPRINTF(format, args...) printk(format, ##args)
#else
	#define DMPRINTF(format, args...)
#endif


#define D4_CMA_REG_ALIGN	0x1000

#define CMA_DDR_END			0xE0000000	// temp

/* Region 1은 SMA 전용 Buffer 임.*/
#define CMA_REGION1_NAME	"sma"
#define CMA_REGION1_START	0xC6800000	/* 104MB */
#define CMA_REGION1_SIZE		0x19000000	/* 400MB */

#define CMA_REGION1_SMA_SIZE	0x18B00000	/* 395MB */
#define CMA_REGION1_DISP_SIZE	0x200000		/* 2MB */

dma_addr_t d4_cma_alloc(struct device *dev, size_t size);
int d4_cma_free(struct device *dev, dma_addr_t addr);
unsigned int d4_cma_get_allocated_memory_size(struct device *dev);

#endif /* __D4_CMA_H__ */
