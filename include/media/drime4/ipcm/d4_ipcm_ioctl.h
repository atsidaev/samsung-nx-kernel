/**
 * @file	d4_ipcm_ioctl.h
 * @brief	IPCM IOCTL file for Samsung DRIMe VI camera interface driver
 *
 * @author	TaeWook Nam <tw.nam@samsung.com>
 * Copyright (c) 2012 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __D4_IPCM_IOCTL_H__
#define __D4_IPCM_IOCTL_H__

#include "d4_ipcm_type.h"

#define IPCM_MAGIC	 'm'

struct ipcm_ioctl_intr {
	enum ipcm_k_int_sig wdma0;
	enum ipcm_k_int_sig wdma1;
	enum ipcm_k_int_sig wdma2;
};

struct ipcm_ioctl_wdma {
	enum ipcm_k_on_off main_out;
	enum ipcm_k_on_off main_rsz;
	enum ipcm_k_on_off main_rsz_nlc_flag;
	enum ipcm_k_on_off sub_rsz;
};

struct ipcm_ioctl_wdma_done {
	enum ipcm_k_on_off main_out;
	enum ipcm_k_on_off main_rsz;
	enum ipcm_k_on_off sub_rsz;

	unsigned int timeout;
	unsigned int err;
};

/*struct for md*/
struct ipcm_md_ioctl_intr {
	enum ipcm_k_on_off gmv_end;
	enum ipcm_k_on_off rmv_end;
};

struct ipcm_ldcm_ioctl_intr {
	enum ldcm_k_intr_flags intr;
};

struct ipcm_ldcm_ioctl_intr_wait {
	unsigned long timeout;
	enum ldcm_k_intr_flags intr;
};

#define IPCM_IOCTL_PHYSICAL_REG_INFO	_IOWR(IPCM_MAGIC, 1, struct ipcm_k_physical_reg_info)
#define IPCM_IOCTL_CLOCK_FREQUENCY	_IOW(IPCM_MAGIC, 2, unsigned int)

#define IPCM_IOCTL_WAIT_MAINOUT_DONE	_IOWR(IPCM_MAGIC, 3, int)
#define IPCM_IOCTL_WAIT_MRSZ_DONE	_IOWR(IPCM_MAGIC, 4, int)
#define IPCM_IOCTL_WAIT_SRSZ_DONE	_IOWR(IPCM_MAGIC, 5, int)

#define IPCM_IOCTL_OPEN_IRQ		_IOR(IPCM_MAGIC, 6, int)
#define IPCM_IOCTL_CLOSE_IRQ		_IO(IPCM_MAGIC, 7)

#define IPCM_IOCTL_INTERRUPT_ENABLE	_IOW(IPCM_MAGIC, 8, struct ipcm_ioctl_wdma)
#define IPCM_IOCTL_INTERRUPT_DISABLE	_IOW(IPCM_MAGIC, 9, struct ipcm_ioctl_wdma)

#define IPCM_IOCTL_ERROR_INTERRUPT_ENABLE	_IOW(IPCM_MAGIC, 10, struct ipcm_ioctl_wdma)
#define IPCM_IOCTL_ERROR_INTERRUPT_DISABLE	_IOW(IPCM_MAGIC, 11, struct ipcm_ioctl_wdma)

#define IPCM_IOCTL_MD_WAIT_GMV_DONE	_IOWR(IPCM_MAGIC, 12, int)
#define IPCM_IOCTL_MD_WAIT_RMV_DONE	_IOWR(IPCM_MAGIC, 13, int)

#define IPCM_IOCTL_MD_OPEN_IRQ		_IO(IPCM_MAGIC, 14)
#define IPCM_IOCTL_MD_CLOSE_IRQ		_IO(IPCM_MAGIC, 15)

#define IPCM_IOCTL_MD_INTERRUPT_ENABLE	_IOW(IPCM_MAGIC, 16, struct ipcm_md_ioctl_intr)
#define IPCM_IOCTL_MD_INTERRUPT_DISABLE	_IOW(IPCM_MAGIC, 17, struct ipcm_md_ioctl_intr)

#define IPCM_IOCTL_LDCM_LOCK		_IO(IPCM_MAGIC, 18)
#define IPCM_IOCTL_LDCM_UNLOCK		_IO(IPCM_MAGIC, 19)

#define IPCM_IOCTL_LDCM_OPEN_IRQ	_IO(IPCM_MAGIC, 20)
#define IPCM_IOCTL_LDCM_CLOSE_IRQ	_IO(IPCM_MAGIC, 21)

#define IPCM_IOCTL_LDCM_INTERRUPT_ENABLE	_IOW(IPCM_MAGIC, 22, struct ipcm_ldcm_ioctl_intr)
#define IPCM_IOCTL_LDCM_INTERRUPT_DISABLE	_IOW(IPCM_MAGIC, 23, struct ipcm_ldcm_ioctl_intr)

#define IPCM_IOCTL_WAIT_LDCM_DONE		_IOW(IPCM_MAGIC, 24, struct ipcm_ldcm_ioctl_intr_wait)

#define IPCM_IOCTL_INTERRUPT_ENABLE_FOR_QVIEW	_IOW(IPCM_MAGIC, 25, struct ipcm_ioctl_wdma)
#define IPCM_IOCTL_INTERRUPT_DISABLE_FOR_QVIEW	_IOW(IPCM_MAGIC, 26, struct ipcm_ioctl_wdma)

#ifdef __cplusplus
extern "C" {
#endif

void _ipcm_wdma0_interrupt_set_callback_function(void);
void _ipcm_wdma1_interrupt_set_callback_function(void);
void _ipcm_wdma2_interrupt_set_callback_function(void);

#ifdef __cplusplus
}
#endif

#endif   /* __D4_IPCM_IOCTL_H__ */


