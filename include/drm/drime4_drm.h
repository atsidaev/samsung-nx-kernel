/* drime4_drm.h
 *
 * Copyright (c) 2011 Samsung Electronics Co., Ltd.
 * Authors:
 *	Inki Dae <inki.dae@samsung.com>
 *	Joonyoung Shim <jy0922.shim@samsung.com>
 *	Seung-Woo Kim <sw0312.kim@samsung.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * VA LINUX SYSTEMS AND/OR ITS SUPPLIERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef _DRIME4_DRM_H_
#define _DRIME4_DRM_H_

#include "drm.h"
#include <video/drime4/d4_dp_type.h>
#include <linux/fb.h>
/**
 * User-desired buffer creation information structure.
 *
 * @size: user-desired memory allocation size.
 *	- this size value would be page-aligned internally.
 * @flags: user request for setting memory type or cache attributes.
 * @handle: returned a handle to created gem object.
 *	- this handle will be set by gem module of kernel side.
 */
struct drm_drime4_gem_create {
	uint64_t size;
	unsigned int flags;
	unsigned int handle;
};

/**
 * A structure for getting buffer offset.
 *
 * @handle: a pointer to gem object created.
 * @pad: just padding to be 64-bit aligned.
 * @offset: relatived offset value of the memory region allocated.
 *	- this value should be set by user.
 */
struct drm_drime4_gem_map_off {
	unsigned int handle;
	unsigned int pad;
	uint64_t offset;
};

/**
 * A structure for mapping buffer.
 *
 * @handle: a handle to gem object created.
 * @pad: just padding to be 64-bit aligned.
 * @size: memory size to be mapped.
 * @mapped: having user virtual address mmaped.
 *	- this variable would be filled by drime4 gem module
 *	of kernel side with user virtual address which is allocated
 *	by do_mmap().
 */
struct drm_drime4_gem_mmap {
	unsigned int handle;
	unsigned int pad;
	uint64_t size;
	uint64_t mapped;
};

/**
 * A structure to gem information.
 *
 * @handle: a handle to gem object created.
 * @flags: flag value including memory type and cache attribute and
 *	this value would be set by driver.
 * @size: size to memory region allocated by gem and this size would
 *	be set by driver.
 */
struct drm_drime4_gem_info {
	unsigned int handle;
	unsigned int flags;
	uint64_t size;
};

/**
 * A structure for user connection request of virtual display.
 *
 * @connection: indicate whether doing connetion or not by user.
 * @extensions: if this value is 1 then the vidi driver would need additional
 *	128bytes edid data.
 * @edid: the edid data pointer from user side.
 */
struct drm_drime4_vidi_connection {
	unsigned int connection;
	unsigned int extensions;
	uint64_t edid;
};

struct drm_drime4_gem_get_phy {
	unsigned int gem_handle;
	unsigned int pad;
	uint64_t size;
	uint64_t phy_addr;
};

struct drm_drime4_plane_set_zpos {
	__u32 plane_id;
	__s32 zpos;
};

/* memory type definitions. */
enum e_drm_drime4_gem_mem_type {
	/* Physically Continuous memory and used as default. */
	DRIME4_BO_CONTIG	= 0 << 0,
	/* Physically Non-Continuous memory. */
	DRIME4_BO_NONCONTIG	= 1 << 0,
	/* non-cachable mapping and used as default. */
	DRIME4_BO_NONCACHABLE	= 0 << 1,
	/* cachable mapping. */
	DRIME4_BO_CACHABLE	= 1 << 1,
	/* write-combine mapping. */
	DRIME4_BO_WC		= 1 << 2,
	DRIME4_BO_MASK		= DRIME4_BO_NONCONTIG | DRIME4_BO_CACHABLE |
					DRIME4_BO_WC
};

struct drm_drime4_g2d_get_ver {
	__u32	major;
	__u32	minor;
};

struct drm_drime4_g2d_cmd {
	__u32	offset;
	__u32	data;
};

enum drm_drime4_g2d_event_type {
	G2D_EVENT_NOT,
	G2D_EVENT_NONSTOP,
	G2D_EVENT_STOP,		/* not yet */
};

struct drm_drime4_g2d_set_cmdlist {
	__u64					cmd;
	__u64					cmd_gem;
	__u32					cmd_nr;
	__u32					cmd_gem_nr;

	/* for g2d event */
	__u64					event_type;
	__u64					user_data;
};

struct drm_drime4_g2d_exec {
	__u64					async;
};

#define DRM_DRIME4_GEM_CREATE		0x00
#define DRM_DRIME4_GEM_MAP_OFFSET	0x01
#define DRM_DRIME4_GEM_MMAP		0x02
/* Reserved 0x03 ~ 0x05 for drime4 specific gem ioctl */
#define DRM_DRIME4_GEM_GET		0x04
#define DRM_DRIME4_PLANE_SET_ZPOS	0x06
#define DRM_DRIME4_VIDI_CONNECTION	0x07
#define DRM_DRIME4_GEM_GET_PHY		0x13

/* G2D */
#define DRM_DRIME4_G2D_GET_VER		0x20
#define DRM_DRIME4_G2D_SET_CMDLIST	0x21
#define DRM_DRIME4_G2D_EXEC		0x22

#define DRM_IOCTL_DRIME4_GEM_CREATE		DRM_IOWR(DRM_COMMAND_BASE + \
		DRM_DRIME4_GEM_CREATE, struct drm_drime4_gem_create)

#define DRM_IOCTL_DRIME4_GEM_MAP_OFFSET	DRM_IOWR(DRM_COMMAND_BASE + \
		DRM_DRIME4_GEM_MAP_OFFSET, struct drm_drime4_gem_map_off)

#define DRM_IOCTL_DRIME4_GEM_MMAP	DRM_IOWR(DRM_COMMAND_BASE + \
		DRM_DRIME4_GEM_MMAP, struct drm_drime4_gem_mmap)

#define DRM_IOCTL_DRIME4_GEM_GET_PHY	DRM_IOWR(DRM_COMMAND_BASE + \
		DRM_DRIME4_GEM_GET_PHY, struct drm_drime4_gem_get_phy)

#define DRM_IOCTL_DRIME4_GEM_GET	DRM_IOWR(DRM_COMMAND_BASE + \
		DRM_DRIME4_GEM_GET,	struct drm_drime4_gem_info)

#define DRM_IOCTL_DRIME4_PLANE_SET_ZPOS	DRM_IOWR(DRM_COMMAND_BASE + \
		DRM_DRIME4_PLANE_SET_ZPOS, struct drm_drime4_plane_set_zpos)

#define DRM_IOCTL_DRIME4_VIDI_CONNECTION	DRM_IOWR(DRM_COMMAND_BASE + \
		DRM_DRIME4_VIDI_CONNECTION, struct drm_drime4_vidi_connection)

#define DRM_IOCTL_DRIME4_G2D_GET_VER		DRM_IOWR(DRM_COMMAND_BASE + \
		DRM_DRIME4_G2D_GET_VER, struct drm_drime4_g2d_get_ver)
#define DRM_IOCTL_DRIME4_G2D_SET_CMDLIST	DRM_IOWR(DRM_COMMAND_BASE + \
		DRM_DRIME4_G2D_SET_CMDLIST, struct drm_drime4_g2d_set_cmdlist)
#define DRM_IOCTL_DRIME4_G2D_EXEC		DRM_IOWR(DRM_COMMAND_BASE + \
		DRM_DRIME4_G2D_EXEC, struct drm_drime4_g2d_exec)

/* DRIME4 specific events */
#define DRM_DRIME4_G2D_EVENT		0x80000000

struct drm_drime4_g2d_event {
	struct drm_event	base;
	__u64			user_data;
	__u32			tv_sec;
	__u32			tv_usec;
	__u32			cmdlist_no;
	__u32			reserved;
};

#ifdef __KERNEL__

/**
 * A structure for lcd panel information.
 *
 * @timing: default video mode for initializing
 * @width_mm: physical size of lcd width.
 * @height_mm: physical size of lcd height.
 */
struct drime4_drm_panel_info {
	struct fb_videomode timing;
	u32 width_mm;
	u32 height_mm;
};

enum drime4_lcd_datawidth {
	D4_DATA_8,/**< data size 8  */
	D4_DATA_24,/**< data size 24  */
};

enum drime4_lcd_type {
	D4_DELTA,
	D4_STRIPE
};

enum drime4_lcd_dis_seq {
	D4_RGB, D4_GBR, D4_BGR, D4_RBG, D4_GRB, D4_BRG
};

/**
 * Platform Specific Structure for DRM based DRIME4 DP LCD.
 *
 * @panel: default panel info for initializing

 * @default_win: default window layer number to be used for UI.
 */
struct drime4_drm_dp_lcd_pdata {
	struct drime4_drm_panel_info panel;
	unsigned int pannel_buf_h_start;
	unsigned int pannel_inv_dot_clk;
	unsigned int pannel_inv_clk;
	unsigned int pannel_inv_h_sync;
	unsigned int pannel_inv_v_sync;
	enum drime4_lcd_datawidth data_width;
	enum drime4_lcd_type panel_type;
	enum drime4_lcd_dis_seq even_seq;
	enum drime4_lcd_dis_seq odd_seq;
	unsigned int default_vid_win;
	unsigned int default_grp_win;
};

/**
 * Platform Specific Structure for DRM based HDMI.
 *
 * @hdmi_dev: device point to specific hdmi driver.
 * @tv_dev: device point to specific tv driver.
 *
 * this structure is used for common hdmi driver and each device object
 * would be used to access specific device driver(hdmi or tv driver)
 */
struct drime4_drm_common_hdmi_pd {
	struct device *hdmi_dev;
	struct device *tv_dev;
	struct device *cec_dev;
};

/**
 * Platform Specific Structure for DRM based HDMI core.
 *
 * @is_v13: set if hdmi version 13 is.
 * @cfg_hpd: function pointer to configure hdmi hotplug detection pin
 * @get_hpd: function pointer to get value of hdmi hotplug detection pin
 */
struct drime4_drm_hdmi_pdata {
	int hpd_gpio;
	void (*cfg_hpd)(bool external);
	int (*get_hpd)(void);
};

#endif	/* __KERNEL__ */
#endif	/* _DRIME4_DRM_H_ */
