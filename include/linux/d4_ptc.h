/**
 * @file d4_ht_ptc.h
 * @brief DRIMe4 Pules Trigger Counter Interface
 * @author kyuchun han <kyuchun.han@samsung.com>
 * 2010 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifndef _D4_HT_PTC_H
#define _D4_HT_PTC_H

#include <mach/ptc_type.h>


struct ptc_device;

struct ptc_device *ptc_request(int ptc_id);

int ptc_oneinput_ctrl(struct ptc_device *ptc, struct ptc_oneinput_info *ctl_info);
int ptc_twoinput_ctrl(struct ptc_device *ptc, struct ptc_twoinput_info *ctl_info);

void ptc_nr_set(struct ptc_device *ptc, unsigned short nr_clk);

void ptc_revers_ctrl(struct ptc_device *ptc, ptc_callback pCallback_rev);

void ptc_revers_clear(struct ptc_device *ptc);

void ptc_cnt_clear(struct ptc_device *ptc, enum ptc_input_mode val);
void ptc_df_clear(struct ptc_device *ptc);

void ptc_count_set(struct ptc_device *ptc, enum ptc_run_type cnt_type, unsigned short cnt_value);

int ptc_count_get(struct ptc_device *ptc, enum ptc_run_type cnt_type);

unsigned int ptc_start(struct ptc_device *ptc, enum ptc_input_mode mode);
void ptc_stop(struct ptc_device *ptc);

void ptc_int_set(struct ptc_device *ptc, enum ptc_int_type int_type, enum ptc_input_mode mode);

#endif /* _D4_HT_PTC_H */

