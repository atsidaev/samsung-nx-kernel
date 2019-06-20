/**
 * @file d4_ep_regs.h
 * @brief DRIMe4 EP Register Define Header File
 * @author Wooram Son <wooram.son@samsung.com>
 * 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef D4_EP_REGS_H_
#define D4_EP_REGS_H_

/* Register Read/Write Macros */
#define WRITE_EPTOP_REG(reg_base, offset, val) __raw_writel(val, reg_base + offset)
#define READ_EPTOP_REG(reg_base, offset) __raw_readl(reg_base + offset)

/* Register Read/Write Macros */
#define D4_EP_TOP_INT_STATUS_IP		0x20 /* Read-Only */
#define D4_EP_TOP_INT_ENABLE_IP		0x24
#define D4_EP_TOP_INT_CLEAR_IP		0x28
#define D4_EP_TOP_INT_STATUS_DMA	0x2c /* Read-Only */
#define D4_EP_TOP_INT_ENABLE_DMA	0x30
#define D4_EP_TOP_INT_CLEAR_DMA 	0x34

#endif
