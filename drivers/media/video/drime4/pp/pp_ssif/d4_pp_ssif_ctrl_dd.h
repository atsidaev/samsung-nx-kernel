/**
 * @file d4_pp_ssif_ctrl_dd.h
 * @brief DRIMe4 PP Sensor Interface Control Device Driver Internal Header
 * @author Main : DeokEun Cho <de.cho@samsung.com>
 *         MIPI : Gunwoo Nam <gunwoo.nam@samsung.com>
 * 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifndef _D4_PP_SSIF_CTRL_DD_H_
#define _D4_PP_SSIF_CTRL_DD_H_

#include "d4_pp_ssif_regs.h"
#include "d4_pp_ssif_if.h"

struct pp_ssif_reg_ctrl_base_info {
	struct device *dev_info;
	unsigned int ctrl_reg_base;     /**< SSIF Control Register - Virtual Base Address */
	unsigned int slvds_reg_base;    /**< SubLVDS Control Register - Virtual Base Address */
	int irq_num;
};

#ifdef __cplusplus
extern "C" {
#endif

void pp_ssif_set_reg_ctrl_base_info(struct pp_ssif_reg_ctrl_base_info *info);

#ifdef __cplusplus
}
#endif

#endif /*_D4_PP_SSIF_CTRL_DD_H_ */

