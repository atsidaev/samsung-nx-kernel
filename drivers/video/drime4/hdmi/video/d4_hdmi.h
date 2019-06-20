/*
 * @file d4_hdmi.h
 * @brief DRIMe4 HDMI header file,
 * @author somabha bhattacharjya<b.somabha@samsung.com>
 * 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef _D4_HDMI_H_
#define _D4_HDMI_H_

#include "../../../../../include/video/hdmi/d4_hdmi_video_type.h"
#include <mach/hdmi/video/d4_hdmi_video.h>

/**
 * @enum hdcp_event
 * @brief HDCP event to be used for HDCP processing
 */
enum hdcp_event {
	HDCP_EVENT_STOP = 0,/**< Stop HDCP  */
	HDCP_EVENT_READ_BKSV_START,/**< Start to read Bksv,Bcaps */
	HDCP_EVENT_WRITE_AKSV_START,/**< Start to write Aksv,An */
	HDCP_EVENT_CHECK_RI_START,/**< Start to check if Ri is equal to Rj */
	HDCP_EVENT_SECOND_AUTH_START,/**< Start 2nd authentication process */
	HDCP_EVENT_MAXS,/**< Number of HDCP Event */
};

struct hdcp_structa {
	spinlock_t lock;/**< Spinlock for synchronizing event */
	wait_queue_head_t waitq;/**< Wait queue */
	enum hdcp_event event;/**< Contains event that occurs */
};

struct d4_hdmi_registers {
	void __iomem *register_sys;
	void __iomem *register_core;
	void __iomem *register_aes;
};

void d4_hdmi_set_info(struct d4_hdmi_registers *hdmi_regs);

int d4_hdmi_set_pixel_aspect_ratio(enum PixelAspectRatio);
int d4_hdmi_set_colorimetry(enum HDMIColorimetry);
int d4_hdmi_set_color_space(enum ColorSpace);
int d4_hdmi_set_pixel_limit(enum PixelLimit);
int d4_hdmi_set_color_depth(enum ColorDepth);
int d4_hdmi_set_video_mode(struct HDMIVideoParameter *pVideo);
void d4_hdmi_mode_select(enum HDMIMode mode);

void d4_hdmi_start(void);
void d4_hdmi_stop(void);
void d4_hdmi_global_interrupt_enable(int is_enable);
void d4_hdmi_hdcp_interrupt_enable(int is_enable);
void d4_hdmi_cec_interrupt_enable(int is_enable);

extern struct hdcp_structa hdcp_struct;
#endif /* _D4_HDMI_H_ */
