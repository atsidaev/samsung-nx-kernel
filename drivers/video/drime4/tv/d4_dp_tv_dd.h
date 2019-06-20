/*
 * @file d4_dp_tv_dd.h
 * @brief DRIMe4 DP TV Interface/control header file,
 * @author somabha bhattacharjya<b.somabha@samsung.com>
 * 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifndef __D4_DP_TV_DD_H__
#define __D4_DP_TV_DD_H__

#include <linux/i2c.h>

#include <mach/dp/d4_dp.h>
#include "d4_dp_tv_if.h"

enum tv_csc_type {
	csc_hd_standard, csc_hd_full, csc_sd_standard, csc_sd_full
};

enum tv_cvbs_format {
	PAL_BDGH, NTSC_M, PAL_N, PAL_60
};

enum tv_frame_rate {
	R_60, R_50, R_30, R_24
};

enum egrp_alpha_control {
	ALPHA_CTRL_ON, ALPHA_CTRL_INV, ALPHA_CTRL_OFS
};

struct edp_tv_displaysize {
	unsigned int h_str;
	unsigned int h_end;
	unsigned int v_str;
	unsigned int v_en;
};

struct stdp_tv_filter_set {
	unsigned int Tap0_Coef;
	unsigned int Tap1_Coef;
	unsigned int Tap2_Coef;
	unsigned int Tap3_Coef;
	unsigned int Tap4_Coef;
	unsigned int Post_Coef;
};

struct grp_layer_info {
	unsigned int stride;
	enum scan_type scan_type;
	unsigned short endian;
	struct stdp_argb background_color;
};

struct tvfb_get_info {
	unsigned int dp_global;
	unsigned int dp_tv;
	unsigned int dp_lcd;
	struct stfb_tv_video_info video;
	struct stfb_graphic_info graphic;
	struct d4_tv tv;
	struct i2c_client *tv_i2c_client;
};

void d4_dp_tv_grp1_set_layer_info(struct grp_layer_info *grpinfo);
enum scan_type d4_dp_tv_grp_scan_get(void);
void d4_dp_tv_video_window_onoff(enum edp_window window, enum edp_onoff on_off);
enum scan_type d4_dp_tv_vid_scan_get(void);
enum scan_type dp_tv_vid_scan_get(void);

void d4_dp_tv_mode_set(void);
void dp_tv_mode_global_variable_set(enum edp_tv_mode mode);
void dp_tv_mode_set(enum edp_tv_mode mode, enum edp_onoff onoff, enum tv_10bit_mode inout);

void dp_tv_grp1_set_scaler(struct stfb_info *info, enum egrp_scale scale);
void dp_tv_grp_vertical_zoom_set(struct stfb_info *info, enum egrp_scale scale);
void dp_tv_grp_zoom_set(struct stfb_info *info, enum egrp_scale scale);
void dp_tv_3d_onoff(struct stfb_info *info, enum scan_type type);
void dp_tv_3d_set(struct stfb_info *info, enum edp_onoff onoff);
void dp_tv_vid_set_format(struct stfb_info *info, enum edp_videoformat format);
void dp_tv_vid_window_onoff(struct stfb_info *info, enum edp_window window,
		enum edp_onoff on_off);
void dp_tv_3d_mode_set(struct stfb_info *info, enum scan_type type);
void dp_tv_set_vid_display_area(struct stfb_info *info, enum edp_window win,
		enum edp_video_bit vid_bit, struct stdp_display_area *display);
void dp_tv_set_vid_image_address(struct stfb_info *info, enum edp_window window,
		struct stdp_image_address *addr);
void dp_tv_mode_get(enum edp_tv_mode *mode);
void dp_tv_bbox_2x(struct stfb_info *info, enum edp_onoff onoff);
void dp_tv_sync_inv_onoff(struct stfb_info *info, unsigned int sync,
		enum edp_onoff onoff);
void dp_tv_cvbs_format(struct stfb_info *info, enum tv_cvbs_format mode);
void dp_tv_sd_pattern_onoff(struct stfb_info *info, enum edp_onoff onoff);
void dp_tv_sd_pattern_sync_match(struct stfb_info *info, enum edp_onoff onoff);
void dp_tv_lock_mode_onoff(struct stfb_info *info, enum edp_onoff onoff);
void dp_tv_y_filter_onoff(struct stfb_info *info, enum edp_onoff onoff);
void dp_tv_c_filter_onoff(struct stfb_info *info, enum edp_onoff onoff);
void dp_tv_mono_onoff(struct stfb_info *info, enum edp_onoff onoff);
void dp_tv_pedestal_onoff(struct stfb_info *info, enum edp_onoff onoff);
void dp_tv_cy_delay_adj(struct stfb_info *info, unsigned int cdelay,
		unsigned int ydelay);
void dp_tv_out_default(struct stfb_info *info);
void d4_tv_hrzflt_onoff(enum edp_layer layer, enum edp_filter filter);
void dp_tv_grpmix_onoff(struct stfb_info *info, enum edp_onoff onoff);
void dp_tv_out_hrz_filter_off(struct stfb_info *info, enum edp_layer layer);
void dp_tv_grp1hrzflt_onoff(struct stfb_info *info, enum edp_onoff onoff);
void dp_tv_hrzflt_onoff(struct stfb_info *info, enum edp_onoff onoff);
void dp_tv_out_hrz_filter_on(struct stfb_info *info, enum edp_layer layer,
		enum edp_filter filter_value);
void dp_tv_g1_moalen_set(struct stfb_info *info, unsigned int value);
void dp_tv_g1_moalen_set(struct stfb_info *info, unsigned int value);
void dp_tv_c_moalen_set(struct stfb_info *info, unsigned int value);
void dp_tv_c_8burst_onoff(struct stfb_info *info, enum edp_onoff onoff);
void dp_tv_y_moalen_set(struct stfb_info *info, unsigned int value);
void dp_tv_y_8burst_onoff(struct stfb_info *info, enum edp_onoff onoff);
void dp_tv_bb_init(struct stfb_info *info);
void dp_tv_bkg_color_set(struct stfb_info *info, struct stdp_ycbcr *color);
void dp_tv_vidmac_onoff(struct stfb_info *info, enum edp_onoff onoff);
void dp_tv_register_update(struct stfb_info *info);
void dp_tv_manual_update_onoff(struct stfb_info *info, enum edp_onoff onoff);
void dp_tv_auto_update_onoff(struct stfb_info *info, enum edp_onoff onoff);
void dp_tv_start_onoff(struct stfb_info *info, enum edp_onoff onoff);
void dp_tv_g1_8burst_onoff(struct stfb_info *info, enum edp_onoff onoff);

void dp_global_nlc_clock_onoff(struct stfb_info *info, enum edp_onoff onoff);
void dp_tv_clock_onoff(struct stfb_info *info, enum edp_tv_clock clock,
		enum edp_onoff onoff);
void dp_tv_hdmi_phy_onoff(struct stfb_info *info, enum edp_onoff onoff);
void dp_tv_global_clock_onoff(struct stfb_info *info, enum edp_onoff onoff);
enum scan_type dp_tv_grp_scan_get(void);
void dp_sd_ycbcr_range_check(struct ycbcr input, struct stdp_ycbcr *output);
int dp_tv_set_stride(struct stfb_info *info, enum edp_layer layer,
		unsigned int stride_value);
void dp_tv_grp1_set_layer_info(struct stfb_info *info,
		struct grp_layer_info *grpinfo);
void dp_tv_rgb_ycbcr(struct stfb_info *info, enum edp_onoff onoff);
void dp_10bit_select(struct stfb_info *info, enum tv_10bit_mode mode,
		unsigned short range);
void dp_tv_dither_on(struct stfb_info *info, enum edp_onoff onoff);
void dp_tv_dither_sel(struct stfb_info *info, enum edp_onoff onoff);
void dp_tv_vid_10bit_dma_set(struct stfb_info *info, enum edp_onoff onoff);
void dp_tv_expan_on(struct stfb_info *info, enum edp_onoff onoff);
void dp_tv_expan_bitrange(struct stfb_info *info, unsigned short range);
void dp_tv_hdmi_10bit_sel(struct stfb_info *info, enum edp_onoff onoff);
void dp_tv_bkg_chroma_order(struct stfb_info *info, enum edp_onoff onoff);
void dp_tv_hdmi_enc_onoff(struct stfb_info *info, enum edp_onoff onoff);
void dp_tv_cvbs_enc_onoff(struct stfb_info *info, enum edp_onoff onoff);
void dp_tv_bt656_enc_onoff(struct stfb_info *info, enum edp_onoff onoff);
void dp_tv_cvbs_lsi_select(struct stfb_info *info, enum edp_onoff onoff);
void dp_tv_hv_resolution(struct stfb_info *info, unsigned short xsize,
		unsigned short ysize);
void dp_tv_vid_scan_set(struct stfb_info *info,
		enum edp_input_img_type scan_type);
void dp_tv_grp1_scan_set(struct stfb_info *info,
		enum edp_input_img_type scan_type);
void dp_tv_hd_pattern_interace_progressive(struct stfb_info *info,
		unsigned short mode);
void dp_tv_hd_pattern_framerate(struct stfb_info *info, unsigned short data);
void dp_tv_grp1dma_onoff(struct stfb_info *info, enum edp_onoff onoff);
void dp_tv_alphablend1_onoff(struct stfb_info *info, enum edp_onoff onoff);
void dp_tv_rangedetect_onoff(struct stfb_info *info, enum edp_layer layer,
		enum edp_onoff onoff);
void dp_tv_rangetarget_onoff(struct stfb_info *info, enum edp_layer layer,
		enum edp_onoff onoff);
void dp_tv_vid_rangedetect_flag(struct stfb_info *info, enum edp_onoff rflag,
		enum edp_onoff gflag, enum edp_onoff bflag, enum edp_onoff ranginv);
void dp_tv_grp1_rangedetect_flag(struct stfb_info *info, enum edp_onoff rflag,
		enum edp_onoff gflag, enum edp_onoff bflag, enum edp_onoff ranginv);

enum scan_type dp_tv_grp_scan_get(void);
void dp_tv_set_info(struct tvfb_get_info *tvgetinfo);
void dp_tv_alphablend1_value_set(struct stfb_info *info, unsigned short value);
void dp_tv_vid_colorspace_conversion(struct stfb_info *info,
		enum tv_cscmode mode);
void hdmi_out_select(struct stfb_info *info, unsigned short phy);
void hdmi_phy_clock_set(enum edp_tv_mode mode, enum tv_10bit_mode inout);
void dp_hdmi_phy_clock_set_32n_8bit(enum edp_hdmi_clock mode);
void dp_hdmi_phy_clock_set_32n_10bit(enum edp_hdmi_clock mode);
void dp_tv_vid_set_bb_color_table(struct stfb_info *info,
		enum edp_bb_color_table table, struct stdp_rgb *rgb_info);
void dp_tv_boundarybox_onoff(struct stfb_info *info, enum edp_bb_box boxwin,
		enum edp_onoff onoff);
void dp_tv_set_bb_display_area(struct stfb_info *info,
		enum edp_bb_box selection, struct stdp_display_area area);

void dp_tv_boundarybox_info_set(struct stfb_info *info,
		struct stbb_info *bb_info);
void dp_tv_set_bb_mode(struct stfb_info *info, enum edp_bb_box selection,
		enum edp_bb_style_set style);
void dp_tv_set_bb_width(struct stfb_info *info, unsigned char h_width,
		unsigned char v_width);

void dp_tv_set_bb_alpha(struct stfb_info *info, unsigned char alpha);

void dp_tv_set_bb_outline_color(struct stfb_info *info,
		struct stdp_rgb *bb_out_color);

int dptv_interrupt_check(void);
int d4_tv_video_init_display(void);
void d4_dp_tv_set_info(struct tvfb_get_info *tvgetinfo);

void d4_dp_tv_set(void);
void dp_tv_vid_set_layer(struct stdp_video_layer_info *vidinfo);

void d4_dp_tv_vid_init(void);
void d4_dp_tv_clock_onoff(enum edp_onoff on_off);
void dp_tv_vid_init(void);

void d4_dp_tv_grp_init(void);
void dp_tv_grp_init(void);
int dp_tv_graphic_display_area(struct stfb_info *info,
		struct stgraphic_display_area *area);
void dp_tv_graphics_window_onoff(struct stfb_info *info, enum edp_window window,
		enum edp_onoff on_off);
void dp_tv_graphics_window_priority(struct stfb_info *info, struct stdp_grp_prority *priority);
void dp_tv_set_graphics_image_address(struct stfb_info *info, enum edp_window window, unsigned int address);

void dp_tv_channelswap(struct stfb_info *info, unsigned int type);
void dp_tv_timegenerate_set(struct stfb_info *info, enum edp_tv_mode mode);
void dp_tv_path_out_mode(struct stfb_info *info, enum edp_tv_mode mode);
void hdmi_link(struct stfb_info *info, enum edp_tv_mode mode,
		enum tv_10bit_mode inout);
void dp_tv_set_grp_alpha_ctrl_onoff(struct stfb_info *info,
		enum egrp_alpha_control selection, enum edp_onoff on_off);


void d4_dp_tv_interrupt_onoff(enum edp_onoff onoff, enum einterrupt_type sel, int arg);
int d4_dp_tv_interrupt_state(void);
int d4_dp_tv_interrupt_arg(enum einterrupt_type sel);

void d4_dp_link_set(enum edp_tv_mode mode, enum edp_onoff onoff);
void d4_dp_hdmi_clock_onoff(enum edp_tv_clock clock, enum edp_onoff on_off);
void d4_dp_tv_set_vid_image_address(enum edp_window window,
		struct stdp_image_address *addr);
void d4_dp_tv_set_3d_vid_image_address(struct stdp_image_address *laddr, struct stdp_image_address *raddr);
void dp_tv_off(void);

#endif
