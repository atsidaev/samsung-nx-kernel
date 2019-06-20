 /**
 * @file d4_pp_core_dd.h
 * @brief DRIMe4 PP Core Common Device Driver Internal Header
 * 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifndef _PP_CORE_COMMON_DD_H_
#define _PP_CORE_COMMON_DD_H_

#include "d4_pp_core_if.h"

/******************************************************************************/
/*                                Enumeration                                 */
/******************************************************************************/

enum pp_core_int_select {
	PP_DPC_LUT_LOAD_DONE_INT       = (0x1 << 0),
	PP_DPC_LUT_GENERATION_DONE_INT = (0x1 << 2),
	PP_CS_LUT_GENERATION_DONE_INT  = (0x1 << 3),
	PP_BG_LUT_LOAD_DONE_INT        = (0x1 << 4),
	PP_XYS_LUT_LOAD_DONE_INT       = (0x1 << 5),
	PP_CS_LUT_LOAD_DONE_INT        = (0x1 << 6),
	PP_HBR_LUT_LOAD_DONE_INT       = (0x1 << 7),
	PP_VBR_LUT_LOAD_DONE_INT       = (0x1 << 8),
	PP_VFPN_DATA_WRITE_DONE_INT    = (0x1 << 9),
	PP_WDMA_FRAME_WRITE_DONE_INT   = (0x1 << 19),
	PP_RDMA_FRAME_READ_DONE_INT    = (0x1 << 22),
	PP_MIPI_WDMA_FRAME_WRITE_DONE_INT   = (0x1 << 23),
	PP_MIPI_RDMA_FRAME_READ_DONE_INT    = (0x1 << 26)
};

enum pp_core_lut_load_select {
	PP_DPC_LUT_LOAD            = (0x1 << 0),
	PP_XYS_LUT_1ST_LOAD        = ((0x1 << 2) | (0x1 << 3)),
	PP_XYS_LUT_H_OFFSET_LOAD   = ((0x1 << 2) | (0x1 << 4)),
	PP_XYS_LUT_H_GAIN_LOAD     = ((0x1 << 2) | (0x1 << 5)),
	PP_XYS_LUT_V_OFFSET_LOAD   = ((0x1 << 2) | (0x1 << 6)),
	PP_XYS_LUT_V_GAIN_LOAD     = ((0x1 << 2) | (0x1 << 7)),
	PP_BG_LUT_LOAD             = (0x1 << 8),
	PP_CS_LUT_LOAD             = (0x1 << 12),
	PP_HBR_LUT_LOAD            = (0x1 << 16),
	PP_VBR_LUT_LOAD            = (0x1 << 20)
};

/******************************************************************************/
/*                                Structure                                   */
/******************************************************************************/

struct pp_core_reg_ctrl_base_info {
	struct device *dev_info;
	unsigned int common_reg_base;   /**< Common Register - Virtual Base Address */
	unsigned int ctrl_reg_base;     /**< Control Register - Virtual Base Address */
	unsigned int dma_reg_base;      /**< DMA Register - Virtual Base Address */
	int irq_num;

	unsigned int pp_phys_base_addr; /**< PP - Physical register information */
};

#ifdef __cplusplus
extern "C" {
#endif

/**< 내부 함수 */
void pp_core_set_reg_ctrl_base_info(struct pp_core_reg_ctrl_base_info *info);

#ifdef __cplusplus
}
#endif

#endif /* _PP_CORE_COMMON_DD_H_ */

