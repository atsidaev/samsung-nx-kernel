/* arch/arm/mach-drime4/include/mach/ipcm/d4-ipcm.h
 *
 * V4L2 based Samsung Drime III IPC driver.
 *
 * Jangwon Lee <jang_won.lee@samsung.com>
 *
 * Note: This driver supports common i2c client driver style
 * which uses i2c_board_info for backward compatibility and
 * new v4l2_subdev as well.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#ifndef __D4_IPCM_H__
#define __D4_IPCM_H__

#include <linux/init.h>
#include <linux/spinlock.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/clk.h>
#include <linux/io.h>
#include <linux/slab.h>

#include <media/v4l2-device.h>

#include "media/drime4/ipcm/d4_ipcm_type.h"
#include "media/drime4/ipcm/d4_ipcm_v4l2.h"

#define IPCM_MODULE_NAME	"d4_ipcm"
#define IPCM_MAX_VIDEO_DEVICE	1

struct ipcm_ctx {
	struct mutex					lock;
	struct mutex					s_lock;
	struct mutex					d_lock;

	unsigned int					v4l2_op_mode;
	unsigned int					v4l2_ctrl_value;
	enum ipcm_v4l2_op_status		v4l2_op_status;

	unsigned int					interrupt_wdma0;
	unsigned int					interrupt_wdma1;
	unsigned int					interrupt_wdma2;
	enum ipcm_curr_buf_type			curr_buf_type;

	struct ipcm_liveview_info		liveview_info;
	struct ipcm_ycc_combi_op_info	combi_op_info;
	struct ipcm_ycc_resize_info		resize_info;
	struct ipcm_ycc_addr_info		change_out_addr_info;
	struct ipcm_md_info				md_info;

	struct ipcm_frame				src_frame;
	struct ipcm_frame				dst_frame;

	struct ipcm_buffer				bayer_in_buf;
	struct ipcm_buffer				ycc_in_buf;
	struct ipcm_buffer				main_out_buf;
	struct ipcm_buffer				rsz_out_buf;
	struct ipcm_buffer				subrsz_out_buf;

	struct drime4_ipcm				*ipcm_dev;
};

/*
 * IPCM Main context
 */
struct drime4_ipcm {
	struct device *dev;
	const char *name;
	int id;
	struct clk *clock;
	void __iomem *reg_base;
	int irq;
	atomic_t ref_count;
	spinlock_t irqlock;
	struct ipcm_ctx	*ipcm_ctx;
	struct video_device *vfd[IPCM_MAX_VIDEO_DEVICE];
	struct v4l2_device v4l2_dev;
	struct completion rdma0_completion;
	struct completion rdma1_completion;
	struct completion wdma0_completion;
	struct completion wdma1_completion;
	struct completion wdma2_completion;
	struct completion wdma3_completion;
	struct drime4_ipcm_dev_data *pd;
};

struct drime4_ipcm_dev_data {
	unsigned int param0;
	unsigned int param1;
	unsigned int param2;
	unsigned int param3;
};

#endif /* __D4_IPCM_H__ */

