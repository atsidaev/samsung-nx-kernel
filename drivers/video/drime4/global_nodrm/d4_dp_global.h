/**
 * @file d4_dp_global.h
 * @brief DRIMe4 DP global (top) clock, interrupt, nlc control define head file
 * @author sejong oh<sejong55.oh@samsung.com>
 * 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifndef __D4_DP_GLOBAL_H__
#define __D4_DP_GLOBAL_H__

#include <video/drime4/d4_dp_type.h>

enum edp_lcd_clock {
	MLCD_SYS_CLK_EN,
	MLCD_CLK_EN,
	MLCD_CLK_INV,
	SLCD_SYS_CLK_EN,
	SLCD_CLK_EN,
	SLCD_CLK_INV,
	NLC_VID_CLK_EN,
	NLC_GRP_CLK_EN
};

enum edp_reset_clock {
	MLCD_Reset,
	SLCD_Reset,
	TV_Reset,
	NLC_Reset,
	HDMI_SYS_Reset,
	HDMI_Video_Reset,
	HDMI_TMDS_Reset,
	HDMI_Audio_Reset
};

void dp_interrupt_on_off(struct stfb_info *info, enum edp_interrupt type,
		enum edp_onoff onoff);
void dp_set_interrupt_interval(struct stfb_info *info, enum edp_path path,
		enum edp_isr_interval interval);
void dp_reg_update(struct stfb_info *info, enum edp_path path,
		enum edp_onoff automatic, enum edp_onoff manual);
void dp_operation_start(struct stfb_info *info, enum edp_path path);
void dp_operation_lcd_stop(struct stfb_info *info, enum edp_path path);
void dp_reg_reset(struct stfb_info *info, enum edp_reset_clock selection);
void dp_lcd_clock_on_off(struct stfb_info *info, enum edp_lcd_clock clock,
		enum edp_onoff onoff);
enum edp_onoff dp_get_interrupt_status(struct stfb_info *info,
		enum edp_interrupt type);
void clear_dp_interrupt(struct stfb_info *info, enum edp_interrupt type);
void dp_nlc_vid_init(struct stfb_info *info);
int dp_nlc_vid_set(struct stfb_info *info, struct sttnlc_video nlcSet);
void dp_nlc_control_set(struct stfb_info *info, enum edp_path path,
		enum edpnlc onoff);
void dp_nlc_grp_init(struct stfb_info *info);
void dp_nlc_grp_set(struct stfb_info *info, struct stnlc_graphic nlcset);

#endif  /*__D4_DP_GLOBAL_H__*/
