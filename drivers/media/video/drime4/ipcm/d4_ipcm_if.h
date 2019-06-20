/**
 * @file	d4_ipcm_if.h
 * @brief	IPCM interface driver header for Samsung DRIMe4 Camera driver
 *
 * @author	TaeWook Nam <tw.nam@samsung.com>,
 * Copyright (c) 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __D4_IPCM_INTERFACE_H__
#define __D4_IPCM_INTERFACE_H__

#include <media/drime4/ipcm/d4_ipcm_type.h>

#define IPCM_TIMEOUT_CHECK
#define IPCM_MD_TIMEOUT_CHECK

#ifdef __cplusplus
extern "C" {
#endif


void ipcm_k_get_physical_reg_info(struct ipcm_k_physical_reg_info *reg_info);
void ipcm_k_clk_set_rate(unsigned int clk_rate);
void ipcm_k_pmu_on_off(enum ipcm_k_on_off pmu_type);

#if 0
/* ipcm interrupt controller */
void ipcm_k_dma_int_init(void);
int ipcm_k_dma_int_enable(enum ipcm_k_int_sig int_sig, void(*callback)(void));
void ipcm_k_dma_int_disable(enum ipcm_k_int_sig int_sig);
void ipcm_k_dma_int_clear(enum ipcm_k_int_sig int_sig);
void ipcm_k_dma_int_off(void);
void ipcm_k_dma_int_handler(void);

void ipcm_k_md_int_init(void);
int ipcm_k_md_int_enable(enum md_k_int_sig int_sig, void(*callback)(void));
void ipcm_k_md_int_disable(enum md_k_int_sig int_sig);
void ipcm_k_md_int_handler(void);

void ipcm_k_ldcm_int_handler(void);


#ifndef IPCM_TIMEOUT_CHECK

void ipcm_k_wdma0_wait_done(void);
void ipcm_k_wdma1_wait_done(void);
void ipcm_k_wdma2_wait_done(void);

#else

int ipcm_k_wdma0_wait_for_completion(unsigned long timeout);
int ipcm_k_wdma1_wait_for_completion(unsigned long timeout);
int ipcm_k_wdma2_wait_for_completion(unsigned long timeout);

#endif
#endif

#ifdef __cplusplus
}
#endif

#endif /* __D4_IPCM_INTERFACE_H__ */
