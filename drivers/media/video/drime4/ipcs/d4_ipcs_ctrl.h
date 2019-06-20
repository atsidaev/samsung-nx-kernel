/**
 * @file	d4_ipcs_ctrl.h
 * @brief	IPCS Total Control header for Samsung DRIMe VI camera interface driver
 *
 * @author	Dongjin Jung <djin81.jung@samsung.com>
 *
 * Copyright (c) 2012 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __D4_K_IPCS_CTRL_H__
#define __D4_K_IPCS_CTRL_H__

#include <linux/init.h>
#include <linux/spinlock.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/clk.h>
#include <linux/io.h>
#include <linux/slab.h>

#include "d4_ipcs_if.h"
#include "d4_ipcs_regs.h"

extern unsigned int ipcs_k_reg_base;

/******************** Register Read/Write Macros *********************/
#define WRITE_IPCS_K_REG(num, val) \
		__raw_writel(val, (ipcs_k_reg_base + IPCS_K_TOP_BASE + 4 * (num)));

#define READ_IPCS_K_REG(num) \
		__raw_readl(ipcs_k_reg_base + IPCS_K_TOP_BASE + 4 * (num));
/*********************************************************************/

/********************* Structure *************************************/
/*********************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

void ipcs_k_set_dev_info(struct ipcs_k_reg_ctrl_base_info *info);

#ifdef __cplusplus
}
#endif

#endif /* __D4_K_IPCS_CTRL_H__ */

