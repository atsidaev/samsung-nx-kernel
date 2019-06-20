/**
 * @file	d4_ipcs_if.h
 * @brief	IPCS interface driver header for Samsung DRIMe4 Camera driver
 *
 * @author	Donjin Jung <djin81.jung@samsung.com>,
 * Copyright (c) 2012 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __D4_IPCS_INTERFACE_H__
#define __D4_IPCS_INTERFACE_H__

#include <media/drime4/ipcs/d4_ipcs_type.h>

#define IPCS_TIMEOUT_CHECK
#define IPCS_INTERRUPT_ERROR_CHECK

#ifdef __cplusplus
extern "C" {
#endif


void ipcs_k_get_physical_reg_info(struct ipcs_k_physical_reg_info *reg_info);
void ipcs_k_clk_set_rate(unsigned int clk_rate);
int ipcs_k_interrupt_init(void);
void ipcs_k_interrupt_free(void);
void ipcs_k_interrupt_enable(enum ipcs_k_interrupt intr, void(*callback_func)(
		void));
void ipcs_k_interrupt_disable(enum ipcs_k_interrupt intr);
void ipcs_k_pmu_on_off(enum ipcs_k_on_off pmu_type);


#ifndef IPCS_TIMEOUT_CHECK

void ipcs_k_wdma0_wait_done(void);
void ipcs_k_wdma1_wait_done(void);
void ipcs_k_wdma2_wait_done(void);
void ipcs_k_wdma3_wait_done(void);

#else

int ipcs_k_wdma0_wait_for_completion(unsigned long timeout);
int ipcs_k_wdma1_wait_for_completion(unsigned long timeout);
int ipcs_k_wdma2_wait_for_completion(unsigned long timeout);
int ipcs_k_wdma3_wait_for_completion(unsigned long timeout);

#endif

#ifdef __cplusplus
}
#endif

#endif /* __D4_IPCS_INTERFACE_H__ */
