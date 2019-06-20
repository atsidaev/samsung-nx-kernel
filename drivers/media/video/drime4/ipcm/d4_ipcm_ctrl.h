/**
 * @file	d4_ipcm_ctrl.h
 * @brief	IPCM Total Control header for Samsung DRIMe VI camera interface driver
 *
 * @author	TaeWook Nam <tw.nam@samsung.com>
 *
 * Copyright (c) 2012 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __D4_K_IPCM_CTRL_H__
#define __D4_K_IPCM_CTRL_H__

#include <linux/init.h>
#include <linux/spinlock.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/clk.h>
#include <linux/io.h>
#include <linux/slab.h>

#include "d4_ipcm_if.h"
#include "d4_ipcm_regs.h"

extern unsigned int ipcm_k_reg_base;
extern unsigned int ipcm_k_md_reg_base;
extern unsigned int ipcm_k_ldcm_top_reg_base;

/******************** Register Read/Write Macros *********************/
#define WRITE_IPCM_K_TOP_REG(num, val) \
		__raw_writel(val, (ipcm_k_reg_base + num));

#define READ_IPCM_K_TOP_REG(num) \
		__raw_readl(ipcm_k_reg_base + num);

#define WRITE_IPCM_K_MD_REG(num, val) \
		__raw_writel(val, (ipcm_k_md_reg_base + num));

#define READ_IPCM_K_MD_REG(num) \
		__raw_readl(ipcm_k_md_reg_base + num);

#define WRITE_IPCM_K_LDCM_TOP_REG(num, val) \
		__raw_writel(val, (ipcm_k_ldcm_top_reg_base + num));

#define READ_IPCM_K_LDCM_TOP_REG(num) \
		__raw_readl(ipcm_k_ldcm_top_reg_base + num);
/*********************************************************************/

/********************* Structure *************************************/
/*********************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

void ipcm_k_set_dev_info(struct ipcm_k_reg_ctrl_base_info *info);

#ifdef __cplusplus
}
#endif

#endif /* __D4_K_IPCM_CTRL_H__ */

