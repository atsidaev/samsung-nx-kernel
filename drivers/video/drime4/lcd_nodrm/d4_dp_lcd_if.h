/**
 * @file d4_dp_lcd_if.h
 * @brief DRIMe4 DP lcd Interface header file
 * @author sejong oh<sejong55.oh@samsung.com>
 * 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifndef __DP_LCD_IF_H_
#define __DP_LCD_IF_H_

#include <video/drime4/d4_dp_type.h>
#include <linux/platform_device.h>

int d4_dp_lcd_video_stride(unsigned int stride);
void d4_dp_lcd_video_background(struct stdp_ycbcr *videobackground);
void d4_dp_lcd_video_display_area(enum edp_window win,
		enum edp_video_bit vid_bit, struct stdp_display_area display);
void
d4_dp_lcd_video_window_onoff(enum edp_window window, enum edp_onoff on_off);
void d4_dp_lcd_video_set(struct stvideodisplay video);
void d4_dp_lcd_video_address_set(struct stvideo_address video);

int d4_dp_lcd_graphic_stride(unsigned int stride);
void d4_dp_lcd_graphic_display_area(struct stgraphic_display_area *area);
void d4_dp_lcd_graphic_background(struct stdp_argb *argb);
void d4_dp_lcd_graphic_set(struct stgrpdisplay graphic);
void d4_dp_lcd_graphic_address_set(struct stgrp_address graphic);
void d4_dp_lcd_graphic_window_onoff(enum edp_window window,
		enum edp_onoff on_off);
void d4_dp_lcd_graphic_scale(enum egrp_scale scale);
void d4_dp_lcd_graphic_window_priority(struct stdp_grp_prority *priority);
void d4_dp_lcd_interrupt_onoff(enum edp_onoff onoff);
void d4_dp_lcd_video_nlc(struct sttnlc_video nlcSet, enum edpnlc onoff);
void d4_dp_lcd_boundarybox_table_color(enum edp_bb_color_table table,
		struct stdp_rgb *rgb_info);
void d4_dp_lcd_bouddarybox_info_set(struct stbb_info *bb_info);
void d4_dp_lcd_boudarybox_onoff(struct stbb_onoff *bb_onoff);

void d4_dp_lcd_filter_onoff(struct stlcdfilter *filter_ctrl);
void d4_dp_lcd_zebra_control(struct stzebra_set *zebra_set);
void d4_dp_lcd_gm_onoff(struct stlcd_gm_lcd val);
void d4_dp_lcd_flip(enum edp_layer layer, enum eswap direct);
void d4_dp_lcd_tcc_set(struct stlcd_tcc tcc);
void d4_dp_lcd_limit_set(struct stdp_rgb_range range);
void d4_dp_lcd_overlayer(enum edp_layer over_layer);
void d4_dp_lcd_graphic_window_alpha(struct stlcd_graphic_alpha alpha);
void d4_dp_lcd_graphic_nlc(struct stnlc_graphic nlcset, enum edpnlc onoff);

void d4_dp_sublcd_video_set(struct stvideodisplay video);
void d4_dp_sublcd_video_window_onoff(enum edp_window window, enum edp_onoff on_off);
int d4_dp_sublcd_video_stride(unsigned int stride);
void d4_dp_sublcd_video_background(struct stdp_ycbcr *videobackground);
void d4_dp_sublcd_video_display_area(enum edp_window win, enum edp_video_bit vid_bit, struct stdp_display_area display);
void d4_dp_sublcd_video_address_set(struct stvideo_address video);
int d4_dp_sublcd_graphic_stride(unsigned int stride);
void d4_dp_sublcd_graphic_background(struct stdp_argb *argb);
void d4_dp_sublcd_graphic_display_area(struct stgraphic_display_area *area);
void d4_dp_sublcd_graphic_window_onoff(enum edp_window window, enum edp_onoff on_off);
void d4_dp_sublcd_graphic_set(struct stgrpdisplay graphic);
void d4_dp_sublcd_graphic_address_set(struct stgrp_address graphic);
void d4_dp_sublcd_graphic_scale(enum egrp_scale scale);
void d4_dp_sublcd_graphic_window_priority(struct stdp_grp_prority *priority);
void d4_dp_sublcd_boundarybox_table_color(enum edp_bb_color_table table,
		struct stdp_rgb *rgb_info);
void d4_dp_sublcd_bouddarybox_info_set(struct stbb_info *bb_info);
void d4_dp_sublcd_boudarybox_onoff(struct stbb_onoff *bb_onoff);
void d4_dp_sublcd_filter_onoff(struct stlcdfilter *filter_ctrl);
void d4_dp_sublcd_zebra_control(struct stzebra_set *zebra_set);
void d4_dp_sublcd_graphic_window_alpha(struct stlcd_graphic_alpha alpha);
void d4_dp_sublcd_flip(enum edp_layer layer, enum eswap direct);
void d4_dp_sublcd_tcc_set(struct stlcd_tcc tcc);
void d4_dp_sublcd_limit_set(struct stdp_rgb_range range);
void d4_dp_sublcd_overlayer(enum edp_layer over_layer);

void d4_dp_bt656_onoff(enum edp_onoff onoff);

void d4_dp_lcd_graphic_order_change(enum edp_onoff onoff);
void d4_dp_lcd_pannel_ctrl(enum edp_onoff onoff);
#endif
