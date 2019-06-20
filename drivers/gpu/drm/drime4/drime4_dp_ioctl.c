/* drime4_drm_dp_lcd.c
 *
 * Copyright (C) 2012 Samsung Electronics Co.Ltd
 * Authors:
 *	sejong <sejong55.oh@samsung.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 *
 */
#include "drmP.h"

#include <linux/kernel.h>
#include <linux/uaccess.h>
#include <linux/delay.h>

#include <drm/drime4_drm.h>
#include <mach/d4_mem.h>

#include "drime4_drm_drv.h"
#include "drime4_drm_fbdev.h"
#include "drime4_drm_crtc.h"
#include "../../../video/drime4/lcd/d4_dp_lcd_dd.h"
#include "../../../video/drime4/tv/d4_dp_tv_dd.h"

#include <mach/dp/d4_dp.h>
#include <video/drime4/d4_dp_type.h>
#include <video/drime4/d4_dp_ioctl.h>
#include "fault_display.h"

/* Return count in buffer.  */
#define RING_BUF_CNT(head,tail,size) ((head+size-tail) % (size))

/* Return space available, 0..size-1.  We always leave one free char
   as a completely full buffer has head == tail, which is the same as
   empty.  */
#define RING_BUF_SPACE(head,tail,size) RING_BUF_CNT((tail),((head)+1),(size))

#define ring_buffer_free(ring_buf)	\
	(RING_BUF_SPACE((ring_buf)->head, (ring_buf)->tail, (ring_buf)->size))

#define ring_buffer_cnt(ring_buf)	\
	(RING_BUF_CNT((ring_buf)->head, (ring_buf)->tail, (ring_buf)->size))

static unsigned int stride;
static struct stdp_ycbcr videobackground;
static enum edp_layer over_layer;
static struct stdp_ycbcr background;
static struct stvideo_display_area vid_display;
static struct stvideo_on video;
static struct stvideo_address video_address;

static struct stdp_argb grp_background;
static struct stgraphic_display_area grp_display;
static struct stgrpdisplay graphic;
static struct stdp_grp_prority priority;
static enum edp_tv_mode tvmode;

static enum egrp_scale scale;
static struct stdp_grp_prority prority;
static struct stbb_table bb_table;
static struct stdp_rgb rgb_info;
static struct stbb_info bb_info;
static struct stbb_onoff bb_onoff;
static struct stlcdfilter filter_ctrl;
static struct stzebra_set zebra_set;
static struct stdp_rgb_range range;
static struct stfbnlcvideo nlc_video;
static struct sttnlc_video nlc_vid_Set;
static struct stfbnlcgraphic nlc_grp;
static struct stnlc_graphic nlc_grp_set;
static struct stlcd_graphic_alpha alpha;
static struct sttv_graphic_alpha tv_alpha;
static struct stdp_window_onoff win_onoff;

static unsigned int firmware_upgrade_gui_addr;
wait_queue_head_t g_lcd_interrupt_wait;
int g_is_lcd_interrupt_occur;

wait_queue_head_t g_tv_interrupt_wait;
int g_is_tv_interrupt_occur;

struct ring_buf {
	spinlock_t		lock;
	struct stvideo_address *buf;
	unsigned long head;
	unsigned long tail;
	unsigned long size;
	unsigned long is_remain_presetted;
};

static struct ring_buf	g_ring_buf_q[MAX_DP_WIN];
static struct ring_buf	g_tv_ring_buf_q[MAX_DP_WIN];

void init_ring_buf(struct ring_buf * ring_buf, unsigned long size)
{
	if (size == 0) {
		if (ring_buf->buf != NULL)
			kfree(ring_buf->buf);
		ring_buf->buf = NULL;
		ring_buf->size = ring_buf->head = ring_buf->tail = 0;
	} else {
		if (ring_buf->buf != NULL)
			kfree(ring_buf->buf);
	
		ring_buf->buf = kmalloc(sizeof(struct stvideo_address)*size, GFP_KERNEL);
		ring_buf->head = ring_buf->tail = 0;
		ring_buf->size = max((unsigned long)2 , size);
		spin_lock_init(&(ring_buf->lock));
	}
}

int push_frame_q(struct ring_buf * ring_buf , struct stvideo_address frame_addr)
{
	unsigned long flags;
	int ret = 0;
	
	if(!ring_buf->buf)
		return -1;

	spin_lock_irqsave(&ring_buf->lock, flags);

	if ((ring_buf->is_remain_presetted == 0) && (!ring_buffer_cnt(ring_buf))) {
		ring_buf->is_remain_presetted = 1;
	} else {
		if (ring_buffer_free(ring_buf) != 0) {
			ring_buf->buf[ring_buf->head] = frame_addr;
			ring_buf->head = (ring_buf->head + 1) % (ring_buf->size);
			ret = ring_buffer_cnt(ring_buf);
		}
		else
		{
			ret = -1;
		}
	}
	spin_unlock_irqrestore(&ring_buf->lock, flags);

	return ret;
}

int pop_frame_q(struct ring_buf * ring_buf , struct stvideo_address * frame_addr)
{
	unsigned long flags;
	int ret = 0;

	if(!ring_buf->buf)
		return -1;

	spin_lock_irqsave(&ring_buf->lock, flags);

	if(!ring_buffer_cnt(ring_buf))
	{
		ring_buf->is_remain_presetted = 0;
		ret = -1;
	}
	else
	{
		ring_buf->is_remain_presetted = 1;
		*frame_addr = ring_buf->buf[ring_buf->tail];		
		ring_buf->tail = (ring_buf->tail + 1) % (ring_buf->size);			
	}
	
	spin_unlock_irqrestore(&ring_buf->lock, flags);	
	return ret;
}

void dp_lcd_video_isr(void)
{
	struct stvideo_address frame_addr;
	int win = 0;
	int dp_lcd_interrupt_state = d4_dp_lcd_interrupt_state();
	
	for (win = 0; win < MAX_DP_WIN; win++) {
		if (g_ring_buf_q[win].size && ((dp_lcd_interrupt_state & (1 << win)) != 0)) {
			if (0 == pop_frame_q(&g_ring_buf_q[win], &frame_addr)) {
				d4_dp_lcd_video_address_set(frame_addr);
			}
		}
	}
}

#if defined(CONFIG_DRM_DRIME4_DP_TV)
void dp_tv_video_isr(void)
{
	struct stvideo_address frame_addr;
	int win = 0;
	int dp_tv_interrupt_state = d4_dp_tv_interrupt_state();
	
	for (win = 0; win < MAX_DP_WIN; win++) {
		if (g_tv_ring_buf_q[win].size && ((dp_tv_interrupt_state & (1 << win)) != 0)) {
			if (0 == pop_frame_q(&g_tv_ring_buf_q[win], &frame_addr)) {
				d4_dp_tv_set_vid_image_address(frame_addr.win, &frame_addr.address);
			}
		}
	}
}
#endif

extern void set_ts_h_flip_info(int h);
extern void set_ts_v_flip_info(int v);

int drime4_fb_dp_ioctl(struct fb_info *info, unsigned int cmd,
		unsigned long arg)
{
	unsigned int retval = 0;
	int size;
	int ret = 0;
	int err = -1;

	if (_IOC_TYPE(cmd) != DP_MAGIC)
		return -1;

	size = _IOC_SIZE(cmd);

    if (_IOC_DIR(cmd) & _IOC_READ)
    	err = access_ok(VERIFY_WRITE, (void *) arg, size);
    else if (_IOC_DIR(cmd) & _IOC_WRITE)
    	err = access_ok(VERIFY_READ, (void *) arg, size);

    if (!err) {
    	return -1;
    }


	switch (cmd) {

	case DP_IOCTL_LCD_VID_STRIDE: {

		DpPRINTK(" fb ioctl video stride arg = %d\n", arg);
		ret = copy_from_user((void *) &stride, (const void *) arg,
				sizeof(stride));
		if (ret < 0)
			return -EFAULT;

		if (d4_dp_lcd_video_stride(stride) < 0)
			printk(KERN_WARNING "ioctl fail: [%d]", cmd);
		break;
	}
	case DP_IOCTL_SLCD_VID_STRIDE: {
		DpPRINTK(" fb ioctl video stride arg = %d\n", arg);
		ret = copy_from_user((void *) &stride, (const void *) arg,
				sizeof(stride));
		if (ret < 0)
			return -EFAULT;
		if (d4_dp_sublcd_video_stride(stride) < 0)
			printk("stride ioctl set fail\n");
		break;
	}

	case DP_IOCTL_LCD_VID_BACKGROUND: {

		ret = copy_from_user((void *) &background, (const void *) arg,
				sizeof(background));
		if (ret < 0)
			return -EFAULT;

		DpPRINTK(" back ground data.y =0x%x\n", background.DP_Y);
		DpPRINTK(" back ground data.cb =0x%x\n", background.DP_Cb);
		DpPRINTK(" back ground data.cr =0x%x\n", background.DP_Cr);

		d4_dp_lcd_video_background(&background);

		break;
	}

	case DP_IOCTL_SLCD_VID_BACKGROUND: {

		ret = copy_from_user((void *) &background, (const void *) arg,
				sizeof(background));
		if (ret < 0)
			return -EFAULT;

		DpPRINTK(" back ground data.y =0x%x\n", background.DP_Y);
		DpPRINTK(" back ground data.cb =0x%x\n", background.DP_Cb);
		DpPRINTK(" back ground data.cr =0x%x\n", background.DP_Cr);

		d4_dp_sublcd_video_background(&background);

		break;
	}

	case DP_IOCTL_LCD_VID_DISPLAY: {

		ret = copy_from_user((void *) &vid_display, (const void *) arg,
				sizeof(vid_display));

		if (ret < 0)
			return -EFAULT;

		DpPRINTK(" fb ioctl video display h star = %d\n", vid_display.display.H_Start);
		DpPRINTK(" fb ioctl video display h size = %d\n", vid_display.display.H_Size);
		DpPRINTK(" fb ioctl video display v star = %d\n", vid_display.display.V_Start);
		DpPRINTK(" fb ioctl video display v size = %d\n", vid_display.display.V_Size);

		d4_dp_lcd_video_display_area(vid_display.win, vid_display.bit,
				vid_display.display);

		break;
	}

	case DP_IOCTL_SLCD_VID_DISPLAY: {

		ret = copy_from_user((void *) &vid_display, (const void *) arg,
				sizeof(vid_display));

		if (ret < 0)
			return -EFAULT;

		DpPRINTK(" fb ioctl video display h star = %d\n", vid_display.display.H_Start);
		DpPRINTK(" fb ioctl video display h size = %d\n", vid_display.display.H_Size);
		DpPRINTK(" fb ioctl video display v star = %d\n", vid_display.display.V_Start);
		DpPRINTK(" fb ioctl video display v size = %d\n", vid_display.display.V_Size);

		d4_dp_sublcd_video_display_area(vid_display.win, vid_display.bit,
				vid_display.display);

		break;
	}

	case DP_IOCTL_LCD_VID_WINDOW_ONOFF: {
		ret = copy_from_user((void *) &win_onoff, (const void *) arg,
				sizeof(win_onoff));
		if (ret < 0)
			return -EFAULT;

		DpPRINTK("video window%d  on/off =%d\n", win_onoff.win, win_onoff.onoff);

		d4_dp_lcd_video_window_onoff(win_onoff.win, win_onoff.onoff);
		break;
	}

	case DP_IOCTL_SLCD_VID_WINDOW_ONOFF: {
		ret = copy_from_user((void *) &win_onoff, (const void *) arg,
				sizeof(win_onoff));
		if (ret < 0)
			return -EFAULT;

		DpPRINTK("video window%d  on/off =%d\n", win_onoff.win, win_onoff.onoff);

		d4_dp_sublcd_video_window_onoff(win_onoff.win, win_onoff.onoff);
		break;
	}

	case DP_IOCTL_LCD_VID_SET: {

		struct stvideodisplay video;

		DpPRINTK(" Video Set I/O cntrl \n");

		ret = copy_from_user((void *) &video, (const void *) arg,
				sizeof(video));
		if (ret < 0)
			return -EFAULT;

		if (video.address_mode == DP_VIRTUAL_SET) {
			video.address.y0_address = d4_uservirt_to_phys(
					video.address.y0_address);
			video.address.c0_address = d4_uservirt_to_phys(
					video.address.c0_address);
			video.address.y1_address = d4_uservirt_to_phys(
					video.address.y1_address);
			video.address.c1_address = d4_uservirt_to_phys(
					video.address.c1_address);
		}
		
		DpPRINTK("video.address.y0_address =0x%x\n" , video.address.y0_address);
		DpPRINTK("video.address.y1_address =0x%x\n" , video.address.y1_address);
		DpPRINTK("video.address.c0_address =0x%x\n" , video.address.c0_address);
		DpPRINTK("video.address.c1_address =0x%x\n" , video.address.c1_address);

		d4_dp_lcd_video_set(video);
		break;
	}

	case DP_IOCTL_LCD_VID_ADDRESS: {

		struct stvideo_address vid;
#if defined(CONFIG_DRM_DRIME4_DP_TV)
		int dp_tv_interrupt_state = d4_dp_tv_interrupt_state();
#else
		int dp_tv_interrupt_state = 0;
#endif

		DpPRINTK(" Video address set I/O Ctrl \n");

		ret = copy_from_user((void *) &vid, (const void *) arg, sizeof(vid));
		if (ret < 0)
			return -EFAULT;

		if (vid.address_mode == DP_VIRTUAL_SET) {
			vid.address.y0_address = d4_uservirt_to_phys(
					vid.address.y0_address);
			vid.address.c0_address = d4_uservirt_to_phys(
					vid.address.c0_address);
			vid.address.y1_address = d4_uservirt_to_phys(
					vid.address.y1_address);
			vid.address.c1_address = d4_uservirt_to_phys(
					vid.address.c1_address);
		}

		if (g_ring_buf_q[vid.win].buf == NULL) {
			d4_dp_lcd_video_address_set(vid);
		} else {
			ret = push_frame_q(&g_ring_buf_q[vid.win], vid);
			if (ret < 0) {
				if (!(dp_tv_interrupt_state & 0x0f)) {
					wait_event_interruptible_timeout(g_lcd_interrupt_wait, (0 != ring_buffer_free(&g_ring_buf_q[vid.win])),HZ/10);
					push_frame_q(&g_ring_buf_q[vid.win], vid);
				} else {
					return -EFAULT;
				}
			} else if (ret == 0) {
				d4_dp_lcd_video_address_set(vid);
			} else if (ring_buffer_free(&g_ring_buf_q[vid.win]) == 0) {
				if (!(dp_tv_interrupt_state & 0x0f)) {
					wait_event_interruptible_timeout(g_lcd_interrupt_wait, (0 != ring_buffer_free(&g_ring_buf_q[vid.win])),HZ/10);
				}
			}
		}
		break;
	}

	case DP_IOCTL_LCD_CURRENT_BUF_CNT:
	{
		struct stdp_display_buf display_buf;
		ret = copy_from_user((void *) &display_buf, (const void *) arg, sizeof(display_buf));
		if (ret < 0)
			return -EFAULT;
		display_buf.buf_size = ring_buffer_cnt(&g_ring_buf_q[display_buf.win]) + g_ring_buf_q[display_buf.win].is_remain_presetted;
		ret = copy_to_user((void *) arg, (const void *)&display_buf, sizeof(display_buf));
		if (ret < 0)
			return -EFAULT;
		break;
	}

	case DP_IOCTL_LCD_FW_MEM_SET:
	{
		ret = copy_from_user((void *) &firmware_upgrade_gui_addr, (const void *) arg, sizeof(firmware_upgrade_gui_addr));
		if (ret < 0)
			return -EFAULT;

		break;
	}

	case DP_IOCTL_LCD_FW_MEM_GET:
	{
		ret = copy_to_user((void *) arg, (const void *)&firmware_upgrade_gui_addr, sizeof(firmware_upgrade_gui_addr));
		if (ret < 0)
			return -EFAULT;
		firmware_upgrade_gui_addr = 0;
		break;
	}
	
	case DP_IOCTL_SLCD_VID_ADDRESS: {

		struct stvideo_address vid;

		DpPRINTK(" Video address set I/O Ctrl \n");

		ret = copy_from_user((void *) &vid, (const void *) arg, sizeof(vid));
		if (ret < 0)
			return -EFAULT;

		if (vid.address_mode == DP_VIRTUAL_SET) {
			vid.address.y0_address = d4_uservirt_to_phys(
					vid.address.y0_address);
			vid.address.c0_address = d4_uservirt_to_phys(
					vid.address.c0_address);
			vid.address.y1_address = d4_uservirt_to_phys(
					vid.address.y1_address);
			vid.address.c1_address = d4_uservirt_to_phys(
					vid.address.c1_address);
		}
		d4_dp_sublcd_video_address_set(vid);
		break;
	}

	case DP_IOCTL_SLCD_VID_SET: {
		struct stvideodisplay video;

		DpPRINTK(" Video Set I/O cntrl \n");

		ret = copy_from_user((void *) &video, (const void *) arg,
				sizeof(video));
		if (ret < 0)
			return -EFAULT;

		if (video.address_mode == DP_VIRTUAL_SET) {
			video.address.y0_address = d4_uservirt_to_phys(
					video.address.y0_address);
			video.address.c0_address = d4_uservirt_to_phys(
					video.address.c0_address);
			video.address.y1_address = d4_uservirt_to_phys(
					video.address.y1_address);
			video.address.c1_address = d4_uservirt_to_phys(
					video.address.c1_address);
		}

		DpPRINTK("video.address.y0_address =0x%x\n" , video.address.y0_address);
		DpPRINTK("video.address.y1_address =0x%x\n" , video.address.y1_address);
		DpPRINTK("video.address.c0_address =0x%x\n" , video.address.c0_address);
		DpPRINTK("video.address.c1_address =0x%x\n" , video.address.c1_address);

		d4_dp_sublcd_video_set(video);
		d4_dp_sublcd_video_window_onoff(video.win, video.win_onoff);
		break;
	}

	case DP_IOCTL_LCD_GRP_STRIDE: {
		ret = copy_from_user((void *) &stride, (const void *) arg,
				sizeof(stride));
		if (ret < 0)
			return -EFAULT;

		d4_dp_lcd_graphic_stride(stride);
		break;
	}

	case DP_IOCTL_SLCD_GRP_STRIDE: {
		ret = copy_from_user((void *) &stride, (const void *) arg,
				sizeof(stride));
		if (ret < 0)
			return -EFAULT;

		d4_dp_sublcd_graphic_stride(stride);
		break;
	}

	case DP_IOCTL_LCD_GRP_BACKGROUND: {
		DpPRINTK(" fb ioctl graphic background first arg = %d\n", arg);
		ret = copy_from_user((void *) &grp_background, (const void *) arg,
				sizeof(grp_background));
		if (ret < 0)
			return -EFAULT;

		DpPRINTK(" back ground data.alpha =0x%x\n" , grp_background.DP_A);
		DpPRINTK(" back ground data.r =0x%x\n" , grp_background.DP_R);
		DpPRINTK(" back ground data.g =0x%x\n" , grp_background.DP_G);
		DpPRINTK(" back ground data.g =0x%x\n" , grp_background.DP_B);

		d4_dp_lcd_graphic_background(&grp_background);
		break;
	}

	case DP_IOCTL_SLCD_GRP_BACKGROUND: {
		DpPRINTK(" fb ioctl graphic background first arg = %d\n", arg);
		ret = copy_from_user((void *) &grp_background, (const void *) arg,
				sizeof(grp_background));
		if (ret < 0)
			return -EFAULT;

		DpPRINTK(" back ground data.alpha =0x%x\n" , grp_background.DP_A);
		DpPRINTK(" back ground data.r =0x%x\n" , grp_background.DP_R);
		DpPRINTK(" back ground data.g =0x%x\n" , grp_background.DP_G);
		DpPRINTK(" back ground data.g =0x%x\n" , grp_background.DP_B);

		d4_dp_sublcd_graphic_background(&grp_background);
		break;
	}

	case DP_IOCTL_LCD_GRP_DISPLAY: {
		ret = copy_from_user((void *) &grp_display, (const void *) arg,
				sizeof(grp_display));
		if (ret < 0)
			return -EFAULT;

		d4_dp_lcd_graphic_display_area(&grp_display);
		break;
	}

	case DP_IOCTL_SLCD_GRP_DISPLAY: {
		ret = copy_from_user((void *) &grp_display, (const void *) arg,
				sizeof(grp_display));
		if (ret < 0)
			return -EFAULT;

		d4_dp_sublcd_graphic_display_area(&grp_display);
		break;
	}

	case DP_IOCTL_LCD_GRP_SET: {
		unsigned int address;
		ret = copy_from_user((void *) &graphic, (const void *) arg,
				sizeof(graphic));
		if (ret < 0)
			return -EFAULT;

		if (graphic.address_mode == DP_VIRTUAL_SET) {
			address = d4_uservirt_to_phys(graphic.address);
			graphic.address = address;
		}

		d4_dp_lcd_graphic_set(graphic);
		break;
	}

	case DP_IOCTL_SLCD_GRP_SET: {
		unsigned int address;
		ret = copy_from_user((void *) &graphic, (const void *) arg,
				sizeof(graphic));
		if (ret < 0)
			return -EFAULT;

		if (graphic.address_mode == DP_VIRTUAL_SET) {
			address = d4_uservirt_to_phys(graphic.address);
			graphic.address = address;
		}

		d4_dp_sublcd_graphic_set(graphic);
		break;
	}

	case DP_IOCTL_LCD_GRP_ADDRESS: {
		struct stgrp_address grp;
		ret = copy_from_user((void *) &grp, (const void *) arg, sizeof(grp));
		if (ret < 0)
			return -EFAULT;

		if (grp.address_mode == DP_VIRTUAL_SET) {
			grp.address = d4_uservirt_to_phys(grp.address);
		}

		d4_dp_lcd_graphic_address_set(grp);
		break;
	}

	case DP_IOCTL_SLCD_GRP_ADDRESS: {
		struct stgrp_address grp;
		ret = copy_from_user((void *) &grp, (const void *) arg, sizeof(grp));
		if (ret < 0)
			return -EFAULT;

		if (grp.address_mode == DP_VIRTUAL_SET) {
			grp.address = d4_uservirt_to_phys(grp.address);
		}

		d4_dp_sublcd_graphic_address_set(grp);
		break;
	}

	case DP_IOCTL_LCD_GRP_WINDOW_ONOFF: {
		ret = copy_from_user((void *) &win_onoff, (const void *) arg,
				sizeof(win_onoff));
		if (ret < 0)
			return -EFAULT;

		DpPRINTK("graphic window%d  on/off =%d\n", win_onoff.win, win_onoff.onoff);
		d4_dp_lcd_graphic_window_onoff(win_onoff.win, win_onoff.onoff);
		break;
	}

	case DP_IOCTL_SLCD_GRP_WINDOW_ONOFF: {
		ret = copy_from_user((void *) &win_onoff, (const void *) arg,
				sizeof(win_onoff));
		if (ret < 0)
			return -EFAULT;

		DpPRINTK("graphic window%d  on/off =%d\n", win_onoff.win, win_onoff.onoff);
		d4_dp_sublcd_graphic_window_onoff(win_onoff.win, win_onoff.onoff);
		break;
	}

	case DP_IOCTL_LCD_GRP_SCALE: {
		ret = copy_from_user((void *) &scale, (const void *) arg,
				sizeof(scale));
		if (ret < 0)
			return -EFAULT;

		DpPRINTK("graphic scale =%d\n", scale);
		d4_dp_lcd_graphic_scale(scale);
		break;
	}

	case DP_IOCTL_SLCD_GRP_SCALE: {
		ret = copy_from_user((void *) &scale, (const void *) arg,
				sizeof(scale));
		if (ret < 0)
			return -EFAULT;

		DpPRINTK("graphic scale =%d\n", scale);
		d4_dp_sublcd_graphic_scale(scale);
		break;
	}

	case DP_IOCTL_LCD_GRP_PRORITY: {
		ret = copy_from_user((void *) &prority, (const void *) arg,
				sizeof(prority));
		if (ret < 0)
			return -EFAULT;

		d4_dp_lcd_graphic_window_priority(&prority);

		break;
	}

	case DP_IOCTL_SLCD_GRP_PRORITY: {
		ret = copy_from_user((void *) &prority, (const void *) arg,
				sizeof(prority));
		if (ret < 0)
			return -EFAULT;

		d4_dp_sublcd_graphic_window_priority(&prority);

		break;
	}

	case DP_IOCTL_LCD_BB_TABLE: {
		ret = copy_from_user((void *) &bb_table, (const void *) arg,
				sizeof(bb_table));
		if (ret < 0)
			return -EFAULT;

		rgb_info = bb_table.rgb_info;
		d4_dp_lcd_boundarybox_table_color(bb_table.table, &rgb_info);
		break;
	}

	case DP_IOCTL_SLCD_BB_TABLE: {
		ret = copy_from_user((void *) &bb_table, (const void *) arg,
				sizeof(bb_table));
		if (ret < 0)
			return -EFAULT;

		rgb_info = bb_table.rgb_info;
		d4_dp_sublcd_boundarybox_table_color(bb_table.table, &rgb_info);
		break;
	}

	case DP_IOCTL_LCD_BB_INFO: {
		ret = copy_from_user((void *) &bb_info, (const void *) arg,
				sizeof(bb_info));
		if (ret < 0)
			return -EFAULT;

		d4_dp_lcd_bouddarybox_info_set(&bb_info);
		break;
	}

	case DP_IOCTL_SLCD_BB_INFO: {
		ret = copy_from_user((void *) &bb_info, (const void *) arg,
				sizeof(bb_info));
		if (ret < 0)
			return -EFAULT;

		d4_dp_sublcd_bouddarybox_info_set(&bb_info);
		break;
	}

	case DP_IOCTL_LCD_BB_ONOFF: {
		ret = copy_from_user((void *) &bb_onoff, (const void *) arg,
				sizeof(bb_onoff));
		if (ret < 0)
			return -EFAULT;

		d4_dp_lcd_boudarybox_onoff(&bb_onoff);

		break;
	}

	case DP_IOCTL_SLCD_BB_ONOFF: {
		ret = copy_from_user((void *) &bb_onoff, (const void *) arg,
				sizeof(bb_onoff));
		if (ret < 0)
			return -EFAULT;

		d4_dp_sublcd_boudarybox_onoff(&bb_onoff);

		break;
	}

	case DP_IOCTL_LCD_FILTER: {
		ret = copy_from_user((void *) &filter_ctrl, (const void *) arg,
				sizeof(filter_ctrl));
		if (ret < 0)
			return -EFAULT;

		if (filter_ctrl.filer_value > DP_BYPASS) {
			printk("ioctl fail: [%d], lcd filter value out of range\n", cmd);
			return -1;
		}
		d4_dp_lcd_filter_onoff(&filter_ctrl);
		break;
	}

	case DP_IOCTL_SLCD_FILTER: {
		ret = copy_from_user((void *) &filter_ctrl, (const void *) arg,
				sizeof(filter_ctrl));
		if (ret < 0)
			return -EFAULT;

		if (filter_ctrl.filer_value > DP_BYPASS) {
			printk("ioctl fail: [%d], lcd filter value out of range\n", cmd);
			return -1;
		}
		d4_dp_sublcd_filter_onoff(&filter_ctrl);
		break;
	}

	case DP_IOCTL_LCD_ZEBRA: {
		ret = copy_from_user((void *) &zebra_set, (const void *) arg,
				sizeof(zebra_set));
		if (ret < 0)
			return -EFAULT;

		d4_dp_lcd_zebra_control(&zebra_set);
		break;
	}

	case DP_IOCTL_SLCD_ZEBRA: {
		ret = copy_from_user((void *) &zebra_set, (const void *) arg,
				sizeof(zebra_set));
		if (ret < 0)
			return -EFAULT;

		d4_dp_sublcd_zebra_control(&zebra_set);
		break;
	}

	case DP_IOCTL_LCD_GM: {
		struct stlcd_gm_lcd val;

		ret = copy_from_user((void *) &val, (const void *) arg, sizeof(val));
		if (ret < 0)
			return -EFAULT;

		d4_dp_lcd_gm_onoff(val);
		break;
	}

	case DP_IOCTL_LCD_DISPLAY_SWAP: {
		struct stdp_display_swap swap;
		int h, v;
		ret = copy_from_user((void *) &swap, (const void *) arg, sizeof(swap));
		if (ret < 0)
			return -EFAULT;

		d4_dp_lcd_flip(swap.layer, swap.direct);

		if(swap.layer == DP_GRP)
		{
			switch(swap.direct)
			{
			case DP_H_SWAP:
				h=1, v=0;
				break;
			case DP_V_SWAP:
				h=0, v=1;
				break;
			case DP_H_V_SWAP:
				h=1, v=1;
				break;
			case DP_None_Change:
			default:
				h=0, v=0;
				break;
			}
			set_ts_h_flip_info(h);
			set_ts_v_flip_info(v);
		}
		break;
	}

	case DP_IOCTL_SLCD_DISPLAY_SWAP: {
		struct stdp_display_swap swap;

		ret = copy_from_user((void *) &swap, (const void *) arg, sizeof(swap));
		if (ret < 0)
			return -EFAULT;

		d4_dp_sublcd_flip(swap.layer, swap.direct);
		break;
	}

	case DP_IOCTL_LCD_TCC: {
		struct stlcd_tcc *tcc;
		tcc = kmalloc(sizeof(struct stlcd_tcc), GFP_KERNEL);
		if (!tcc)
			return -ENOMEM;
		
		ret = copy_from_user((void *) tcc, (const void *) arg, sizeof(tcc));
		if (ret < 0) {
			kfree(tcc);
			return -EFAULT;
		}
		d4_dp_lcd_tcc_set(*tcc);
		kfree(tcc);
		break;
	}

	case DP_IOCTL_SLCD_TCC: {
		struct stlcd_tcc *tcc;
		tcc = kmalloc(sizeof(struct stlcd_tcc), GFP_KERNEL);
		if (!tcc)
			return -ENOMEM;

		ret = copy_from_user((void *) tcc, (const void *) arg, sizeof(tcc));
		if (ret < 0) {
			kfree(tcc);
			return -EFAULT;
		}

		d4_dp_sublcd_tcc_set(*tcc);
		kfree(tcc);
		break;
	}

	case DP_IOCTL_LCD_LIMIT: {
		ret = copy_from_user((void *) &range, (const void *) arg,
				sizeof(range));
		if (ret < 0)
			return -EFAULT;

		d4_dp_lcd_limit_set(range);
		break;
	}

	case DP_IOCTL_SLCD_LIMIT: {
		ret = copy_from_user((void *) &range, (const void *) arg,
				sizeof(range));
		if (ret < 0)
			return -EFAULT;

		d4_dp_sublcd_limit_set(range);
		break;
	}

	case DP_IOCTL_LCD_LAYER_CHANGE: {
		ret = copy_from_user((void *) &over_layer, (const void *) arg,
				sizeof(over_layer));
		if (ret < 0)
			return -EFAULT;

		d4_dp_lcd_overlayer(over_layer);
		break;
	}

	case DP_IOCTL_SLCD_LAYER_CHANGE: {
		ret = copy_from_user((void *) &over_layer, (const void *) arg,
				sizeof(over_layer));
		if (ret < 0)
			return -EFAULT;

		d4_dp_sublcd_overlayer(over_layer);
		break;
	}

	case DP_IOCTL_NLC_VIDEO: {
		DpPRINTK("NLC Video Set I/O cntrl \n");
		ret = copy_from_user((void *) &nlc_video, (const void *) arg,
				sizeof(nlc_video));
		if (ret < 0)
			return -EFAULT;

		if (nlc_video.dp_path == DP_MLCD) {
			if (nlc_video.address_mode == DP_VIRTUAL_SET) {
				nlc_video.y_address = d4_uservirt_to_phys(nlc_video.y_address);
				nlc_video.c_address = d4_uservirt_to_phys(nlc_video.c_address);
			} else {
				nlc_vid_Set.address.Addr_Y0 = nlc_video.y_address;
				nlc_vid_Set.address.Addr_C0 = nlc_video.c_address;
			}

			nlc_vid_Set.path = DP_MLCD;
			nlc_vid_Set.format = nlc_video.format;
			nlc_vid_Set.inputImage_height = nlc_video.img_height;
			nlc_vid_Set.inputImage_width = nlc_video.img_width;

			nlc_vid_Set.display.H_Start = nlc_video.H_Start;
			nlc_vid_Set.display.H_Size = nlc_video.H_Size;
			nlc_vid_Set.display.V_Start = nlc_video.V_Start;
			nlc_vid_Set.display.V_Size = nlc_video.V_Size;

			if (nlc_video.nlc == NLC_VID_ON) {
				d4_dp_lcd_video_nlc(nlc_vid_Set, NLC_VID_ON);
			} else if (nlc_video.nlc == NLC_OFF)
				d4_dp_lcd_video_nlc(nlc_vid_Set, NLC_OFF);
		}

		break;
	}
	case DP_IOCTL_NLC_GRAPHIC: {
		struct stgraphic_display_area display;

		DpPRINTK("NLC Video Set I/O cntrl \n");

		ret = copy_from_user((void *) &nlc_grp, (const void *) arg,
				sizeof(nlc_grp));
		if (ret < 0)
			return -EFAULT;

		if (nlc_grp.dp_path == DP_MLCD) {
			nlc_grp_set = nlc_grp.nlcset;
			display.win = nlc_grp_set.display_win;
			display.display = nlc_grp_set.display;

			if (nlc_grp_set.image_width < nlc_grp_set.display.H_Size)
				nlc_grp_set.display.H_Size = nlc_grp_set.image_width;
			if (nlc_grp_set.image_height < nlc_grp_set.image_height)
				nlc_grp_set.display.V_Size = nlc_grp_set.image_height;

			if (nlc_grp.onoff == NLC_GRP_ON)
				d4_dp_lcd_graphic_nlc(nlc_grp_set, NLC_GRP_ON);
			else if (nlc_grp.onoff == NLC_OFF)
				d4_dp_lcd_graphic_nlc(nlc_grp_set, NLC_OFF);
		}

		break;
	}

	case DP_IOCTL_LCD_GRP_ALPHA: {
		ret = copy_from_user((void *) &alpha, (const void *) arg,
				sizeof(alpha));
		if (ret < 0)
			return -EFAULT;

		d4_dp_lcd_graphic_window_alpha(alpha);
		break;
	}

	case DP_IOCTL_SLCD_GRP_ALPHA: {
		ret = copy_from_user((void *) &alpha, (const void *) arg,
				sizeof(alpha));
		if (ret < 0)
			return -EFAULT;

		d4_dp_sublcd_graphic_window_alpha(alpha);
		break;
	}

	case DP_IOCTL_BT656_SET: {
		struct stlcd_bt656 val;

		ret = copy_from_user((void *) &val, (const void *) arg, sizeof(val));
		if (ret < 0)
			return -EFAULT;
		d4_dp_bt656_onoff(val);
		break;
	}

	case DP_IOCTL_LCD_GRP_ORDER_SET: {
		enum edp_onoff val;

		ret = copy_from_user((void *) &val, (const void *) arg, sizeof(val));
		if (ret < 0)
			return -EFAULT;

		d4_dp_lcd_graphic_order_change(val);
		break;
	}

	case DP_IOCTL_LCD_TURN_ON: {
		drime4_lcd_display_init();
		dp_lcd_pannel_set();
		d4_lcd_video_init_display();	
		break;
	}
	case DP_IOCTL_LCD_COLOR_ADJUST: {
		struct stlcd_csc_matrix val;
		ret = copy_from_user((void *) &val, (const void *) arg, sizeof(val));
		if (ret < 0)
			return -EFAULT;

		lcd_csc_manual_set(&val);
		break;
	}

	case DP_IOCTL_LCD_FRAME_INTERRUPT_ON: {
		enum edp_window val;
		ret = copy_from_user((void *) &val, (const void *) arg, sizeof(val));
		if (ret < 0)
			return -EFAULT;

		d4_dp_lcd_interrupt_onoff(DP_ON, (enum einterrupt_type)val, 0);
		break;
	}

	case DP_IOCTL_LCD_FRAME_INTERRUPT_OFF: {
		enum edp_window val;
		ret = copy_from_user((void *) &val, (const void *) arg, sizeof(val));
		if (ret < 0)
			return -EFAULT;

		d4_dp_lcd_interrupt_onoff(DP_OFF, (enum einterrupt_type)val, 0);
		break;		
	}
	
	case DP_IOCTL_LCD_WAIT_FRAME_INTERRUPT:
		g_is_lcd_interrupt_occur = 0;
		wait_event_interruptible_timeout(g_lcd_interrupt_wait, (g_is_lcd_interrupt_occur == 1), HZ/10);
		break;

	case DP_IOCTL_LCD_SET_RING_BUF_CNT:
	{
		struct stdp_display_buf val;
		ret = copy_from_user((void *) &val, (const void *) arg, sizeof(val));
		init_ring_buf(&g_ring_buf_q[val.win], val.buf_size);
		break;
	}

	case DP_IOCTL_SET_LCD_ON_OFF:
	{
		int val;
		ret = copy_from_user((void *) &val, (const void *) arg, sizeof(val));
		dp_lcd_mlcd_clock_onoff(val);
		break;
	}
	
	case DP_IOCTL_SET_DP_CLOCK_HIGH:
	{
		unsigned long val;
		ret = copy_from_user((void *) &val, (const void *) arg, sizeof(val));
		dp_bus_clock_change(val);
		break;
	}

#if defined(CONFIG_DRM_DRIME4_DP_TV)
	case DP_IOCTL_TV_VID_STRIDE:
		ret = copy_from_user((void *) &stride, (const void *) arg,
				sizeof(stride));
		if (ret < 0)
			return -EFAULT;

		if (d4_dp_tv_video_stride(stride) < 0)
			printk("stride ioctl set fail\n");
		break;
	case DP_IOCTL_TV_VID_BACKGROUND:

		ret = copy_from_user((void *) &videobackground, (const void *) arg,
				sizeof(videobackground));
		if (ret < 0)
			return -EFAULT;

		d4_dp_tv_video_background(&videobackground);
		break;
	case DP_IOCTL_TV_LAYER_CHANGE:

		ret = copy_from_user((void *) &over_layer, (const void *) arg,
				sizeof(over_layer));
		if (ret < 0)
			return -EFAULT;

		d4_dp_tv_change_layer(over_layer);
		break;

	case DP_IOCTL_TV_VID_DISPLAY:

		ret = copy_from_user((void *) &vid_display, (const void *) arg,
				sizeof(vid_display));

		if (ret < 0)
			return -EFAULT;

		d4_dp_tv_set_vid_display_area(vid_display.win, vid_display.bit,
				&(vid_display.display));

		break;

	case DP_IOCTL_TV_VID_SET:
		ret = copy_from_user((void *) &video, (const void *) arg,
				sizeof(video));
		if (ret < 0)
			return -EFAULT;


		if (video.address_mode == DP_VIRTUAL_SET) {
			video.image_addr.y0_address = d4_uservirt_to_phys(
					video.image_addr.y0_address);
			video.image_addr.c0_address = d4_uservirt_to_phys(
					video.image_addr.c0_address);
			video.image_addr.y1_address = d4_uservirt_to_phys(
					video.image_addr.y1_address);
			video.image_addr.c1_address = d4_uservirt_to_phys(
					video.image_addr.c1_address);
		}
		d4_dp_tv_video_on(video);
		break;

	case DP_IOCTL_TV_SET_3DVIDEO:
	{
		struct stvideo_on_3d video_3d;
		struct stvideo_address left_frame_addr;
		struct stvideo_address right_frame_addr;
		ret = copy_from_user((void *) &video_3d, (const void *) arg,
				sizeof(video_3d));
		if (ret < 0)
			return -EFAULT;

		if (video_3d.address_mode == DP_VIRTUAL_SET) {
			video_3d.left_window_image_addr.y0_address = d4_uservirt_to_phys(
					video_3d.left_window_image_addr.y0_address);
			video_3d.left_window_image_addr.c0_address = d4_uservirt_to_phys(
					video_3d.left_window_image_addr.c0_address);
			video_3d.left_window_image_addr.y1_address = d4_uservirt_to_phys(
					video_3d.left_window_image_addr.y1_address);
			video_3d.left_window_image_addr.c1_address = d4_uservirt_to_phys(
					video_3d.left_window_image_addr.c1_address);

			video_3d.right_window_image_addr.y0_address = d4_uservirt_to_phys(
					video_3d.right_window_image_addr.y0_address);
			video_3d.right_window_image_addr.c0_address = d4_uservirt_to_phys(
					video_3d.right_window_image_addr.c0_address);
			video_3d.right_window_image_addr.y1_address = d4_uservirt_to_phys(
					video_3d.right_window_image_addr.y1_address);
			video_3d.right_window_image_addr.c1_address = d4_uservirt_to_phys(
					video_3d.right_window_image_addr.c1_address);
		}

		d4_dp_tv_3d_video_on(video_3d);
		left_frame_addr.address_mode = right_frame_addr.address_mode = video_3d.address_mode;
		
		left_frame_addr.win = DP_WIN0;
		left_frame_addr.address = video_3d.left_window_image_addr;

		right_frame_addr.win = DP_WIN1;
		right_frame_addr.address = video_3d.right_window_image_addr;
		

		if (g_tv_ring_buf_q[DP_WIN1].buf == NULL) {
			d4_dp_tv_set_vid_image_address(DP_WIN0, &video_3d.left_window_image_addr);
			d4_dp_tv_set_vid_image_address(DP_WIN1, &video_3d.right_window_image_addr);
		} else {
			ret = push_frame_q(&g_tv_ring_buf_q[DP_WIN0], left_frame_addr);
			ret = push_frame_q(&g_tv_ring_buf_q[DP_WIN1], right_frame_addr);
			if (ret < 0) {
				wait_event_interruptible_timeout(g_tv_interrupt_wait, (0 != ring_buffer_free(&g_tv_ring_buf_q[DP_WIN1])),HZ/10);
				push_frame_q(&g_tv_ring_buf_q[DP_WIN0], left_frame_addr);
				push_frame_q(&g_tv_ring_buf_q[DP_WIN1], right_frame_addr);
			} else if (ret == 0) {
				d4_dp_tv_set_vid_image_address(DP_WIN0, &video_3d.left_window_image_addr);
				d4_dp_tv_set_vid_image_address(DP_WIN1, &video_3d.right_window_image_addr);
			} else if (ring_buffer_free(&g_tv_ring_buf_q[1]) == 0) {
				wait_event_interruptible_timeout(g_tv_interrupt_wait, (0 != ring_buffer_free(&g_tv_ring_buf_q[DP_WIN1])),HZ/10);
			}
		}
		break;
	}
	case DP_IOCTL_TV_VID_ADDRESS:
		ret = copy_from_user((void *) &video_address, (const void *) arg,
						sizeof(video_address));
		if (ret < 0)
					return -EFAULT;
		if (video.address_mode == DP_VIRTUAL_SET) {
			video_address.address.y0_address = d4_uservirt_to_phys(
					video_address.address.y0_address);
			video_address.address.c0_address = d4_uservirt_to_phys(
					video_address.address.c0_address);
			video_address.address.y1_address = d4_uservirt_to_phys(
					video_address.address.y1_address);
			video_address.address.c1_address = d4_uservirt_to_phys(
					video_address.address.c1_address);
		}

		if (g_tv_ring_buf_q[video_address.win].buf == NULL) {
			d4_dp_tv_set_vid_image_address(video_address.win, &video_address.address);
		} else {
			ret = push_frame_q(&g_tv_ring_buf_q[video_address.win], video_address);
			if (ret < 0) {
				wait_event_interruptible_timeout(g_tv_interrupt_wait, (0 != ring_buffer_free(&g_tv_ring_buf_q[video_address.win])),HZ/10);
				push_frame_q(&g_tv_ring_buf_q[video_address.win], video_address);
			} else if (ret == 0) {
				d4_dp_tv_set_vid_image_address(video_address.win, &video_address.address);			
			} else if (ring_buffer_free(&g_tv_ring_buf_q[video_address.win]) == 0) {
				wait_event_interruptible_timeout(g_tv_interrupt_wait, (0 != ring_buffer_free(&g_tv_ring_buf_q[video_address.win])),HZ/10);
			}
		}
		break;

	case DP_IOCTL_TV_3D_VID_ADDRESS:
	{
		struct stvideo_address tv_video_address[2];
		ret = copy_from_user((void *) tv_video_address, (const void *) arg,
						sizeof(tv_video_address));
		if (ret < 0)
					return -EFAULT;
		if (tv_video_address[0].address_mode == DP_VIRTUAL_SET) {
			/* L image */
			tv_video_address[0].address.y0_address = d4_uservirt_to_phys(
					tv_video_address[0].address.y0_address);
			tv_video_address[0].address.c0_address = d4_uservirt_to_phys(
					tv_video_address[0].address.c0_address);
			tv_video_address[0].address.y1_address = d4_uservirt_to_phys(
					tv_video_address[0].address.y1_address);
			tv_video_address[0].address.c1_address = d4_uservirt_to_phys(
					tv_video_address[0].address.c1_address);

			/* R image */
			tv_video_address[1].address.y0_address = d4_uservirt_to_phys(
					tv_video_address[1].address.y0_address);
			tv_video_address[1].address.c0_address = d4_uservirt_to_phys(
					tv_video_address[1].address.c0_address);
			tv_video_address[1].address.y1_address = d4_uservirt_to_phys(
					tv_video_address[1].address.y1_address);
			tv_video_address[1].address.c1_address = d4_uservirt_to_phys(
					tv_video_address[1].address.c1_address);
		}

		d4_dp_tv_set_3d_vid_image_address(&tv_video_address[0].address, &tv_video_address[1].address);
		break;
	}

	case DP_IOCTL_TV_FRAME_INTERRUPT_ON: {
		enum edp_window val;
		ret = copy_from_user((void *) &val, (const void *) arg, sizeof(val));
		if (ret < 0)
			return -EFAULT;

		d4_dp_tv_interrupt_onoff(DP_ON, (enum einterrupt_type)val, 0);
		break;
	}

	case DP_IOCTL_TV_FRAME_INTERRUPT_OFF: {
		enum edp_window val;
		ret = copy_from_user((void *) &val, (const void *) arg, sizeof(val));
		if (ret < 0)
			return -EFAULT;

		d4_dp_tv_interrupt_onoff(DP_OFF, (enum einterrupt_type)val, 0);
		break;		
	}
	
	case DP_IOCTL_TV_WAIT_FRAME_INTERRUPT:
		g_is_tv_interrupt_occur = 0;
		wait_event_interruptible_timeout(g_tv_interrupt_wait, (g_is_tv_interrupt_occur == 1), HZ/10);
		break;

	case DP_IOCTL_TV_SET_RING_BUF_CNT:
	{
		struct stdp_display_buf val;
		ret = copy_from_user((void *) &val, (const void *) arg, sizeof(val));
		init_ring_buf(&g_tv_ring_buf_q[val.win], val.buf_size);
		break;
	}

	case DP_IOCTL_TV_CURRENT_BUF_CNT:
	{
		struct stdp_display_buf display_buf;
		ret = copy_from_user((void *) &display_buf, (const void *) arg, sizeof(display_buf));
		if (ret < 0)
			return -EFAULT;
		display_buf.buf_size = ring_buffer_cnt(&g_tv_ring_buf_q[display_buf.win]) + g_tv_ring_buf_q[display_buf.win].is_remain_presetted;
		ret = copy_to_user((void *) arg, (const void *)&display_buf, sizeof(display_buf));
		if (ret < 0)
			return -EFAULT;
		break;
	}

	case DP_IOCTL_TV_FRAME_RATE:
	{
		unsigned long frame_rate = 60;
		ret = copy_to_user((void *) arg, (const void *) frame_rate, sizeof(frame_rate));
		if (ret < 0)
			return -EFAULT;
		break;
	}
	case DP_IOCTL_TV_VID_WINDOW_ONOFF:
		ret = copy_from_user((void *) &win_onoff, (const void *) arg,
				sizeof(win_onoff));
		if (ret < 0)
			return -EFAULT;

		d4_dp_tv_video_window_onoff(win_onoff.win, win_onoff.onoff);
		break;

	case DP_IOCTL_TV_BB_TABLE:
		ret = copy_from_user((void *) &bb_table, (const void *) arg,
				sizeof(bb_table));
		if (ret < 0)
			return -EFAULT;

		rgb_info = bb_table.rgb_info;
		d4_dp_tv_boundarybox_table_color(bb_table.table, &rgb_info);
		break;

	case DP_IOCTL_TV_BB_INFO:
		ret = copy_from_user((void *) &bb_info, (const void *) arg,
				sizeof(bb_info));
		if (ret < 0)
			return -EFAULT;

		d4_dp_tv_boundarybox_info_set(&bb_info);
		break;

	case DP_IOCTL_TV_BB_ONOFF:
		ret = copy_from_user((void *) &bb_onoff, (const void *) arg,
				sizeof(bb_onoff));
		if (ret < 0)
			return -EFAULT;

		d4_dp_tv_boundarybox_onoff(&bb_onoff);

		break;

	case DP_IOCTL_TV_GRP_SCALE:
		ret = copy_from_user((void *) &scale, (const void *) arg,
				sizeof(scale));
		if (ret < 0)
			return -EFAULT;

		d4_dp_tv_graphics_set_scale(scale);
		break;

	case DP_IOCTL_TV_GRP_STRIDE:
		ret = copy_from_user((void *) &stride, (const void *) arg,
				sizeof(stride));
		if (ret < 0)
			return -EFAULT;

		if (d4_dp_tv_graphics_set_stride(stride) < 0)
			printk("stride ioctl set fail\n");
		break;

	case DP_IOCTL_TV_GRP_BACKGROUND:
		ret = copy_from_user((void *) &grp_background, (const void *) arg,
				sizeof(grp_background));
		if (ret < 0)
			return -EFAULT;

		d4_dp_tv_graphics_background(&grp_background);
		break;

	case DP_IOCTL_TV_GRP_DISPLAY:
		ret = copy_from_user((void *) &grp_display, (const void *) arg,
				sizeof(grp_display));
		if (ret < 0)
			return -EFAULT;

		d4_dp_tv_graphics_display_area(&grp_display);
		break;

	case DP_IOCTL_TV_GRP_WINDOW_ONOFF:
		ret = copy_from_user((void *) &win_onoff, (const void *) arg,
				sizeof(win_onoff));
		if (ret < 0)
			return -EFAULT;

		d4_dp_tv_graphics_window_onoff(win_onoff.win, win_onoff.onoff);
		break;

	case DP_IOCTL_TV_GRP_SET: {
		ret = copy_from_user((void *) &graphic, (const void *) arg,
				sizeof(graphic));
		if (ret < 0)
			return -EFAULT;

		if (graphic.address_mode == DP_VIRTUAL_SET) {
			graphic.address = d4_uservirt_to_phys(graphic.address);
		}

		d4_dp_tv_graphics_set(graphic);
		break;
	}

	case DP_IOCTL_TV_GRP_PRIORITY:
		ret = copy_from_user((void *) &priority, (const void *) arg,
				sizeof(priority));
		if (ret < 0)
			return -EFAULT;

		d4_dp_tv_graphics_window_priority(&priority);
		break;

	case DP_IOCTL_TV_MODE:

		ret = copy_from_user((void *) &tvmode, (const void *) arg,
				sizeof(tvmode));
		if (ret < 0)
			return -EFAULT;

		d4_dp_tv_set_mode(tvmode);
		break;

	case DP_IOCTL_TV_GRP_ALPHA:
		ret = copy_from_user((void *) &tv_alpha, (const void *) arg,
				sizeof(tv_alpha));
		if (ret < 0)
			return -EFAULT;
		d4_dp_tv_graphic_window_alpha(tv_alpha);
		break;

	case DP_IOCTL_TV_GRP_ORDER_SET: {
		enum edp_onoff val;
		ret = copy_from_user((void *) &val, (const void *) arg, sizeof(val));
		if (ret < 0)
			return -EFAULT;

		d4_dp_tv_graphic_order_change(val);
		break;
	}

	case DP_IOCTL_TV_TURN_ON:
		d4_dp_tv_clock_onoff(DP_ON);		
		d4_dp_tv_turn_video_on(); /* To test DRM bypas by MMF.*/
		break;

	case DP_IOCTL_TV_VID_HRZ_FILTER_SET:
	{
		enum edp_filter val;
		ret = copy_from_user((void *) &val, (const void *) arg, sizeof(val));
		if (ret < 0)
			return -EFAULT;
		
		d4_tv_hrzflt_onoff(DP_VIDEO, val);
		break;
	}
	
	case DP_IOCTL_TV_GRP_HRZ_FILTER_SET:
	{
		enum edp_filter val;
		ret = copy_from_user((void *) &val, (const void *) arg, sizeof(val));
		if (ret < 0)
			return -EFAULT;
		
		d4_tv_hrzflt_onoff(DP_GRP, val);
		break;
	}

#endif

	default:
		retval = -ENOIOCTLCMD;
		break;
	}

	return retval;
}

void display_fault(void)
{
	unsigned int phys_display_addr;
	void *virt_display_addr;
	phys_display_addr = d4_dp_lcd_graphic_address_get(DP_WIN0);
	virt_display_addr = phys_to_virt(phys_display_addr);
	memcpy(virt_display_addr, fault_gui, sizeof(fault_gui));
	d4_dp_lcd_graphic_window_onoff(DP_WIN0, DP_ON);
}

