/**
 * @file d4_pp_core_if.h
 * @brief DRIMe4 PP Core Device Driver Interface Header
 * 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifndef _PP_CORE_INTERFACE_H_
#define _PP_CORE_INTERFACE_H_

#include <media/drime4/pp/pp_core/d4_pp_core_type.h>

/**<  For POLL using */
#define PP_CORE_POLL_ENABLE
#define PP_MIPI_POLL_ENABLE

#ifdef __cplusplus
extern "C" {
#endif

extern struct completion pp_core_wdma_completion;
extern struct completion pp_core_rdma_completion;
extern struct completion mipi_rx_wdma_completion;
extern struct completion mipi_tx_rdma_completion;

/* PP Core Common Control Function */

void pp_core_get_phys_reg_info(unsigned int *reg_info);

void pp_core_com_request_irq(void);
void pp_core_com_free_irq(void);

void pp_wdma_set_callback_func(void (*callback_func)(void));
void pp_rdma_set_callback_func(void (*callback_func)(void));
void mipi_wdma_set_callback_func(void (*callback_func)(void));
void mipi_rdma_set_callback_func(void (*callback_func)(void));

void pp_core_com_wait_lut_load_done(void);
int pp_core_com_wait_lut_load_done_with_timeout(unsigned int timemout);
void pp_core_com_wait_lut_generation_done(void);
void pp_core_com_wait_vfpn_done(void);
void pp_core_com_wait_init_wdma(void);
void pp_core_com_wait_init_rdma(void);
void pp_core_com_wait_init_lut_load(void);
void pp_core_com_wait_wdma_done(void);
void pp_core_com_wait_rdma_done(void);
void pp_core_com_core_reset(void);
void mipi_com_wait_wdma_done(void);
void mipi_com_wait_rdma_done(void);
void pp_core_com_set_pp_clock(unsigned int clock_set);

void pp_com_pmu_on_off(enum pp_dd_onoff onoff);

#ifdef __cplusplus
}
#endif

#endif /* _PP_CORE_INTERFACE_H_ */

