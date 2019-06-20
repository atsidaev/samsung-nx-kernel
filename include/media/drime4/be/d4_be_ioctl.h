/**
 * @file d4_be_ioctl.h
 * @brief DRIMe4 Bayer Engine IOCTL Interface Header File
 * @author Niladri Mukherjee <n.mukherjee@samsung.com>
 * 2012 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include "d4_be_type.h"

#ifndef D4_BE_IOCTL_H_
#define D4_BE_IOCTL_H_

#define BE_IOCTL_DEBUG_MSG BE_KERN_FALSE
#if BE_IOCTL_DEBUG_MSG
#define BE_IOCTL_DEBUG(format, args...) printk(KERN_DEBUG format, ##args)
#else
#define BE_IOCTL_DEBUG(format, args...)
#endif


#define BE_IOCTL_CMD_MAGIC      'b'


struct be_ioctl_get_reg_info {
	enum be_reg_selection phys_reg_selection;
	struct be_phys_reg_info phys_reg_info;
};

struct be_ioctl_ip_wait {
	enum be_interrupt_level be_core_intr;
	int timeout;
	int result;
};

struct be_ioctl_dma_wait {
	enum be_dma_interrupt_level be_dma_intr;
	int timeout;
	int result;
};


#define BE_IOCTL_GET_PHYS_REG_INFO	_IOWR(BE_IOCTL_CMD_MAGIC, 0, struct be_ioctl_get_reg_info)
#define BE_IOCTL_OPEN_IRQ					_IOR(BE_IOCTL_CMD_MAGIC, 1, int)
#define BE_IOCTL_CLOSE_IRQ					_IO(BE_IOCTL_CMD_MAGIC, 2)
#define BE_IOCTL_SET_CLOCK	_IOW(BE_IOCTL_CMD_MAGIC, 3, unsigned int)
#define BE_IOCTL_INIT_COMPLELETION 			_IO(BE_IOCTL_CMD_MAGIC, 4)
#define BE_IOCTL_INIT_ALL_IP_COMPLETETION 	_IO(BE_IOCTL_CMD_MAGIC, 5)
#define BE_IOCTL_INIT_ALL_DMA_COMPLETETION 	_IO(BE_IOCTL_CMD_MAGIC, 6)
#define BE_IOCTL_INIT_AN_IP_COMPLETETION 	_IOW(BE_IOCTL_CMD_MAGIC, 7, unsigned int /*enum be_core_intr_level_intr*/)
#define BE_IOCTL_INIT_A_DMA_COMPLETETION 	_IOW(BE_IOCTL_CMD_MAGIC, 8, unsigned int /*enum be_dma_intr_level*/)
#define BE_IOCTL_WAIT_IP_INTERRUPT 			_IOWR(BE_IOCTL_CMD_MAGIC, 9, struct be_ioctl_ip_wait)
#define BE_IOCTL_WAIT_DMA_INTERRUPT 		_IOWR(BE_IOCTL_CMD_MAGIC, 10, struct be_ioctl_dma_wait)
#define BE_IOCTL_LOCK_MUTEX 				_IO(BE_IOCTL_CMD_MAGIC, 11)
#define BE_IOCTL_UNLOCK_MUTEX 				_IO(BE_IOCTL_CMD_MAGIC, 12)

#define BE_IOCTL_3DME_KERNEL_DO 			_IO(BE_IOCTL_CMD_MAGIC, 13)

#define BE_IOCTL_HDRLLS_REG_INTR			_IOWR(BE_IOCTL_CMD_MAGIC, 14, unsigned int)
#define BE_IOCTL_HDRLLS_DEREG_INTR			_IOWR(BE_IOCTL_CMD_MAGIC, 15, unsigned int)
#define BE_IOCTL_HDRLLS_DEREG_DMAINTR		_IOWR(BE_IOCTL_CMD_MAGIC, 16, unsigned int)
#define BE_IOCTL_HDRLLS_NR_IWT				_IOWR(BE_IOCTL_CMD_MAGIC, 17, struct be_nr_iwt_info)
#define BE_IOCTL_HDRLLS_EVENT_WAIT 			_IOWR(BE_IOCTL_CMD_MAGIC, 18, struct be_hdrlls_completion_event)


#endif /*D4_BE_IOCTL_H_*/

