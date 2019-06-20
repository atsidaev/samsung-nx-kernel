/**
 * @file	d4_ipcs_ioctl.h
 * @brief	IPCS IOCTL file for Samsung DRIMe VI camera interface driver
 *
 * @author	Dongjin Jung <djin81.jung@samsung.com>
 * Copyright (c) 2012 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __D4_IPCS_IOCTL_CMD_H__
#define __D4_IPCS_IOCTL_CMD_H__

#include "d4_ipcs_type.h"

#define IPCS_MAGIC	 's'

struct ipcs_ioctl_intr {
	enum ipcs_k_interrupt wdma0;
	enum ipcs_k_interrupt wdma1;
	enum ipcs_k_interrupt wdma2;
	enum ipcs_k_interrupt wdma3;
};

struct ipcs_ioctl_wdma {
	enum ipcs_k_on_off main_out;
	enum ipcs_k_on_off main_rsz;
	enum ipcs_k_on_off sub_rsz1;
	enum ipcs_k_on_off sub_rsz2;
};

struct ipcs_ioctl_wdma_done {
	enum ipcs_k_on_off main_out;
	enum ipcs_k_on_off main_rsz;
	enum ipcs_k_on_off sub_rsz1;
	enum ipcs_k_on_off sub_rsz2;

	unsigned int timeout;
	int err;
};

#define IPCS_IOCTL_OPEN_IRQ 			_IOR(IPCS_MAGIC, 1, int)
#define IPCS_IOCTL_CLOSE_IRQ 			_IO(IPCS_MAGIC, 2)
#define IPCS_IOCTL_PHYSICAL_REG_INFO 	_IOWR(IPCS_MAGIC, 3, struct ipcs_k_physical_reg_info)
#define IPCS_IOCTL_PMU_ON_OFF			_IOW(IPCS_MAGIC, 4, enum ipcs_k_on_off)
#define IPCS_IOCTL_CLOCK_FREQUENCY		_IOW(IPCS_MAGIC, 5, unsigned int)

#define IPCS_IOCTL_INTERRUPT_ENABLE		_IOW(IPCS_MAGIC, 10, struct ipcs_ioctl_wdma)
#define IPCS_IOCTL_INTERRUPT_DISABLE	_IOW(IPCS_MAGIC, 11, struct ipcs_ioctl_wdma)
#define IPCS_IOCTL_WAIT_MAINOUT_DONE 	_IOWR(IPCS_MAGIC, 12, int)
#define IPCS_IOCTL_WAIT_MRSZ_DONE 		_IOWR(IPCS_MAGIC, 13, int)
#define IPCS_IOCTL_WAIT_SRSZ1_DONE 		_IOWR(IPCS_MAGIC, 14, int)
#define IPCS_IOCTL_WAIT_SRSZ2_DONE 		_IOWR(IPCS_MAGIC, 15, int)
#define IPCS_IOCTL_WAIT_1TILE_DONE		_IOWR(IPCS_MAGIC, 16, struct ipcs_ioctl_wdma_done)

#ifdef __cplusplus
extern "C" {
#endif

void _ipcs_wdma0_interrupt_set_callback_function(void);
void _ipcs_wdma1_interrupt_set_callback_function(void);
void _ipcs_wdma2_interrupt_set_callback_function(void);
void _ipcs_wdma3_interrupt_set_callback_function(void);

#ifdef __cplusplus
}
#endif

#endif   /* __D4_IPCS_IOCTL_CMD_H__ */


