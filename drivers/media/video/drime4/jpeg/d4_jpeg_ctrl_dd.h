/**
 * @file d4_jpeg_ctrl_dd.h
 * @brief DRIMe4 jpeg register Control Device Driver Header
 * @author JinHyoung An <jh0913.an@samsung.com>
 * 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifndef __JPEG_CTRL_DD_H__
#define __JPEG_CTRL_DD_H__

#include "d4_jpeg_if.h"

/**
 * @struct	jpeg_reg_ctrl_base_info
 * @brief	register resource member
 */
struct JPEG_REG_CTRL_BASE_INFO {
	struct device *dev_info;
	unsigned int topRegBase; /**< Top Register - Virtual Base Address */
	unsigned int ctrlRegBase; /**< Ctrl Register - Virtual Base Address */
	int irq_num;

	struct JPEG_PHY_REG_INFO physTopReg; /**< Top register - Physical register information */
	struct JPEG_PHY_REG_INFO physCtrlReg; /**< Ctrl register - Physical register information */
};

#ifdef __cplusplus
extern "C" {
#endif

void jpeg_set_reg_ctrl_base_info(struct JPEG_REG_CTRL_BASE_INFO *info);

#ifdef __cplusplus
}
#endif

#endif /* __JPEG_CTRL_DD_H__ */
