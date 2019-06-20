/**
 * @file d4_ep_dd.h
 * @brief DRIMe4 EP Driver Interface Header File
 * @author Wooram Son <wooram.son@samsung.com>
 * 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef D4_EP_DD_H_
#define D4_EP_DD_H_

#include "d4_ep_if.h"

/**
 * @brief EP Path를 관리하기 위한 Preprocessor
 * 		  ID_OFFSET은 Path의 ID를 생성하기 위해 사용하는 Offset
 * 		  LIST_CNT는 최대 생성가능한 Path의 갯수를 나타냄.
 */
#define EP_PATH_ID_MIN		10
#define EP_PATH_ID_MAX		1073741823
#define EP_PATH_LIST_CNT 	EP_K_STATE_MAX

enum EP_PMU_STATE {
	EP_PMU_INVALID = 0x0,
	EP_PMU_ON = 0x1,
	EP_PMU_OFF = 0x2,
};

/*
 * EP IOCTL Debug Message Macro
 */
/**< EP DD 코드에서 디버그 메세지 출력 (TRUE: 출력, FALSE: 출력안함) */
/* #define EP_DD_DEBUG_MSG_ON TRUE */
/* #define EP_DD_ERROR_MSG_ON TRUE */

#ifdef EP_DD_DEBUG_MSG_ON
	#define EP_DD_DEBUG_MSG(format, args...) pr_debug(format, ##args)
#else
	#define EP_DD_DEBUG_MSG(format, args...)
#endif

#ifdef EP_DD_ERROR_MSG_ON
	#define EP_DD_ERROR_MSG(format, args...) pr_err(format, ##args)
#else
	#define EP_DD_ERROR_MSG(format, args...)
#endif

void ep_set_top_reg_info(unsigned int addr, unsigned int size);
void ep_set_ldc_reg_info(unsigned int addr, unsigned int size);
void ep_set_hdr_reg_info(unsigned int addr, unsigned int size);
void ep_set_nrm_reg_info(unsigned int addr, unsigned int size);
void ep_set_mnf_reg_info(unsigned int addr, unsigned int size);
void ep_set_fd_reg_info(unsigned int addr, unsigned int size);
void ep_set_bblt_reg_info(unsigned int addr, unsigned int size);
void ep_set_lvr_reg_info(unsigned int addr, unsigned int size);
void ep_set_dma_reg_info(unsigned int addr, unsigned int size);
int ep_set_device_info(struct device *dev);

/*
 * EP Interrupt Wait-queue Interface
 */
void ep_top_init_wait_queue(void);
void ep_top_wakeup_core_intr(enum ep_k_intr_flags intr);
void ep_top_wakeup_dma_intr(enum ep_k_dma_intr_flags intr);
void ep_dd_set_top_reg(void __iomem *addr);
void ep_dd_set_dma_reg(void __iomem *addr);

#endif
