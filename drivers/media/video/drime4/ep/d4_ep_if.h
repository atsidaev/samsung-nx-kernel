/**
 * @file d4_ep_if.h
 * @brief DRIMe4 EP Driver Interface Header File
 * @author Wooram Son <wooram.son@samsung.com>
 * 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef D4_EP_IF_H_
#define D4_EP_IF_H_

#include <media/drime4/ep/d4_ep_type.h>

int ep_get_reg_info(struct ep_reg_info *ep_reg_info);

int ep_top_wait_core_intr(enum ep_k_intr_flags intr, int timeout_ms);
int ep_top_wait_dma_intr(enum ep_k_dma_intr_flags intr, int timeout_ms);
void ep_top_init_core_intr_wait_queue(enum ep_k_intr_flags intr);
void ep_top_init_dma_intr_wait_queue(enum ep_k_dma_intr_flags intr);

void ep_path_blocker_init(void);
int ep_path_init(void);
int ep_path_try_init(void);
struct ep_path_desc *ep_path_get_descriptor(int path_id);
int ep_path_open(struct ep_k_path_info *path_info);
int ep_path_close(int path_id);
int ep_k_path_block_start(struct ep_k_path_block_start_info *block_start_info,
		enum ep_k_path_start_type start_type);

void ep_pmu_on_off(enum ep_dd_onoff onoff);
int ep_set_clk_rate(int rate);

int ep_pmu_requeset(void);
void ep_pmu_clear(void);
#endif
