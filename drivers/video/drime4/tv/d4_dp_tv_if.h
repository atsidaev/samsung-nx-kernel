/**
 * @file d4_dp_tv_if.h
 * @brief DRIMe4 DP TV Interface header file
 * @author somabha bhattacharjya<b.somabha@samsung.com>
 * 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifndef __DP_TV_IF_H_
#define __DP_TV_IF_H_

#include <video/drime4/d4_dp_type.h>

/******************** TV Video set ******************************/
void d4_dp_tv_video_on(struct stvideo_on video);
void d4_dp_tv_vid_set_layer(struct stdp_video_layer_info *layer);
void d4_dp_tv_change_layer(enum edp_layer layer);
int d4_dp_tv_video_stride(unsigned int stride);
void d4_dp_tv_video_background(struct stdp_ycbcr *videobackground);
void d4_dp_tv_set_vid_display_area(enum edp_window win,
		enum edp_video_bit vid_bit, struct stdp_display_area *display);

void d4_dp_tv_video_window_onoff(enum edp_window window, enum edp_onoff on_off);

void d4_dp_tv_boundarybox_table_color(enum edp_bb_color_table table,
		struct stdp_rgb *rgb_info);
void d4_dp_tv_boundarybox_info_set(struct stbb_info *bb_info);
void d4_dp_tv_boundarybox_onoff(struct stbb_onoff *bb_onoff);

/******************* TV Graphics set ****************************/
void d4_dp_tv_graphics_set_scale(enum egrp_scale scale);
void d4_dp_tv_graphics_set_vertical_scale(enum egrp_scale scale);
int d4_dp_tv_graphics_set_stride(unsigned int stride);
void d4_dp_tv_graphics_background(struct stdp_argb *argb);
void d4_dp_tv_graphics_display_area(struct stgraphic_display_area *area);
void d4_dp_tv_graphics_window_onoff(enum edp_window window,
		enum edp_onoff on_off);
void d4_dp_tv_graphics_set(struct stgrpdisplay graphic);
void d4_dp_tv_graphics_window_priority(struct stdp_grp_prority *priority);

void d4_dp_tv_set_mode(enum edp_tv_mode mode);

void d4_dp_tv_graphic_window_alpha(struct sttv_graphic_alpha alpha);

void d4_dp_tv_graphic_order_change(enum edp_onoff on_off);



void d4_dp_tv_vid_init(void);
void d4_dp_tv_grp_init(void);
void d4_dp_tv_mode_set(void);
void d4_dp_tv_vid_set_layer(struct stdp_video_layer_info *vidinfo);
void d4_dp_tv_turn_video_on(void);

void d4_dp_tv_3d_video_on(struct stvideo_on_3d);

#endif
