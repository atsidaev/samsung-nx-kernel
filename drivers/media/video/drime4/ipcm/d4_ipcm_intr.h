/**
 * @file d4_ipcm_intr.h
 * @brief DRIMe4 IPCM Driver Header
 * @author TaeWook Nam <tw.@samsung.com>
 * 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */


#ifndef __D4_IPCM_INTR_H__
#define __D4_IPCM_INTR_H__

#include "d4_ipcm_if.h"

#ifdef __cplusplus
extern "C" {
#endif

int ipcm_k_dma_int_init(void);
void ipcm_k_dma_int_off(void);
void ipcm_k_dma_int_clear(enum ipcm_k_int_sig int_sig);
int ipcm_k_dma_int_enable(enum ipcm_k_int_sig int_sig, void(*callback)(void));
void ipcm_k_dma_int_disable(enum ipcm_k_int_sig int_sig);

void ipcm_k_md_int_init(void);
void ipcm_k_md_int_off(void);
int ipcm_k_md_int_enable(enum md_k_int_sig int_sig, void(*callback)(void));
void ipcm_k_md_int_disable(enum md_k_int_sig int_sig);

void ipcm_k_ldcm_int_init(void);
void ipcm_k_ldcm_int_off(void);
int ipcm_k_ldcm_int_enable(enum ldcm_k_intr_flags int_sig, void(*callback)(enum ldcm_k_intr_flags));
void ipcm_k_ldcm_int_disable(enum md_k_int_sig int_sig);

#ifndef IPCM_TIMEOUT_CHECK
void ipcm_k_wdma0_wait_done(void);
void ipcm_k_wdma1_wait_done(void);
void ipcm_k_wdma2_wait_done(void);

void ipcm_k_md_gmv_wait_done(void);
void ipcm_k_md_rmv_wait_done(void);
#else
int ipcm_k_wdma0_wait_for_completion(unsigned long timeout);
int ipcm_k_wdma1_wait_for_completion(unsigned long timeout);
int ipcm_k_wdma2_wait_for_completion(unsigned long timeout);

int ipcm_k_md_gmv_wait_for_completion(unsigned long timeout);
int ipcm_k_md_rmv_wait_for_completion(unsigned long timeout);
int ipcm_k_ldcm_wait_for_completion(enum ldcm_k_intr_flags intr, unsigned long timeout);

void ipcm_k_quickview_done(void);

#endif

#ifdef __cplusplus
}
#endif

#endif /* __D4_IPCM_INTR_H__ */

