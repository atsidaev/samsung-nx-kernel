/**
 * @file d4_be_top.h
 * @brief DRIMe4 BE TOP Interface / Control Header File
 * @author Niladri Mukherjee <n.mukherjee@samsung.com>
 * 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef D4_BE_TOP_H_
#define D4_BE_TOP_H_

#include <linux/device.h>
#include <linux/clk.h>
#include <linux/interrupt.h>
#include <linux/uaccess.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/io.h>
#include <mach/d4_cma.h>

#include <media/drime4/be/d4_be_type.h>
#include <mach/be/d4_be.h>
#include "d4_be_regs.h"
#include "d4_be_if.h"



/**
 * @struct	be_reg_ctrl_base_info
 * @brief	register resource member
 */
struct be_reg_ctrl_base_info {
	struct device *dev_info;
	unsigned int top_reg_base; /**< Top Register - Virtual Base Address */
	unsigned int ghost_reg_base; /**< GHOST Register - Virtual Base Address */
	unsigned int snr_reg_base; /**< SNR Register - Virtual Base Address */
	unsigned int sg_reg_base; /**< SG Register - Virtual Base Address */
	unsigned int blend_reg_base; /**< BLEND Register - Virtual Base Address */
	unsigned int fme_reg_base; /**< FME Register - Virtual Base Address */
	unsigned int cme_reg_base; /**< 3DME Register - Virtual Base Address */
	unsigned int dma_reg_base; /**< DMA Register - Virtual Base Address */
	int irq_num;

	struct be_phys_reg_info phys_top_reg_info; /**< Top register - Physical register information */
	struct be_phys_reg_info phys_ghost_reg_info; /**< GHOST register - Physical register information */
	struct be_phys_reg_info phys_snr_reg_info; /**< SNR register - Physical register information */
	struct be_phys_reg_info phys_sg_reg_info; /**< SG register - Physical register information */
	struct be_phys_reg_info phys_blend_reg_info; /**< BLEND register - Physical register information */
	struct be_phys_reg_info phys_fme_reg_info; /**< FME register - Physical register information */
	struct be_phys_reg_info phys_3dme_reg_info; /**< 3DME register - Physical register information */
	struct be_phys_reg_info phys_dma_reg_info; /**< DMA register - Physical register information */
};


#ifdef __cplusplus
extern "C" {
#endif


void be_set_reg_ctrl_base_info(struct be_reg_ctrl_base_info *info);
void be_wakeup_core_intr(enum be_interrupt_level intr);
void __be_dma_isr_clbk(void);


#ifdef __cplusplus
}
#endif

#endif /* D4_BE_TOP_H_ */

