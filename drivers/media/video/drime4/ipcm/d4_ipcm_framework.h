/* arch/arm/march-drime4/include/mach/ipcm/d4_ipcm_framework.h
 * Copyright (c) 2011 Samsung Electronics
 *	Jangwon Lee <jang_won.lee@samsung.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#ifndef __DRIME4_IPCM_FRAMEWORK_H_
#define __DRIME4_IPCM_FRAMEWORK_H_

extern void drime4_ipcm_fw_init(struct drime4_ipcm *ipcm);

extern int drime4_ipcm_fw_sleep(struct drime4_ipcm *ipcm,
	struct ipcm_ctx *ctx);

extern int drime4_ipcm_fw_wakeup(struct drime4_ipcm *ipcm,
	struct ipcm_ctx *ctx);

extern void drime4_ipcm_fw_enable_top_clks(struct drime4_ipcm *ipcm,
	struct ipcm_ctx *ctx);

extern void drime4_ipcm_fw_enable_sys_clks(struct drime4_ipcm *ipcm,
	struct ipcm_ctx *ctx, unsigned int enable);

extern int drime4_ipcm_fw_set_src_param(struct ipcm_ctx *ctx,
	struct ipcm_frame *frame);

extern int drime4_ipcm_fw_set_dst_param(struct ipcm_ctx *ctx,
	struct ipcm_frame *frame);

extern struct ipcm_mem_info *drime4_ipcm_fw_get_buffer_info(
	struct drime4_ipcm *ipcm, struct ipcm_ctx *ctx);

extern int drime4_ipcm_fw_free_buffer_info(struct drime4_ipcm *ipcm,
	struct ipcm_ctx *ctx);

extern int drime4_ipcm_fw_set_bayer_inbuf(struct drime4_ipcm *ipcm,
	struct v4l2_requestbuffers *b);

extern int drime4_ipcm_fw_set_ycc_inbuf(struct drime4_ipcm *ipcm,
	struct v4l2_requestbuffers *b);

extern int drime4_ipcm_fw_set_main_outbuf(struct drime4_ipcm *ipcm,
	struct v4l2_requestbuffers *b);

extern int drime4_ipcm_fw_set_rsz_outbuf(struct drime4_ipcm *ipcm,
	struct v4l2_requestbuffers *b);

extern int drime4_ipcm_fw_set_subrsz_outbuf(struct drime4_ipcm *ipcm,
	struct v4l2_requestbuffers *b);

extern int drime4_ipcm_fw_change_src_addr(struct drime4_ipcm *ipcm,
	struct v4l2_outputparm *output);

extern int drime4_ipcm_fw_change_dst_addr(struct drime4_ipcm *ipcm,
	struct v4l2_captureparm *capture);

extern int drime4_ipcm_fw_src_enqueue_buffer(struct drime4_ipcm *ipcm,
	struct ipcm_ctx *ctx, struct v4l2_buffer *b);

extern int drime4_ipcm_fw_src_dequeue_buffer(struct drime4_ipcm *ipcm,
	struct ipcm_ctx *ctx, struct v4l2_buffer *b);

extern int drime4_ipcm_fw_dst_enqueue_buffer(struct drime4_ipcm *ipcm,
	struct ipcm_ctx *ctx, struct v4l2_buffer *b);

extern int drime4_ipcm_fw_dst_dequeue_buffer(struct drime4_ipcm *ipcm,
	struct ipcm_ctx *ctx, struct v4l2_buffer *b);

extern int drime4_ipcm_fw_set_buffer_to_ctx(struct drime4_ipcm *ipcm,
	struct ipcm_ctx *ctx, struct v4l2_buffer *b);

extern int drime4_ipcm_fw_free_buffer(struct drime4_ipcm *ipcm,
	struct v4l2_requestbuffers *b);

extern void drime4_ipcm_fw_start_liveview(struct drime4_ipcm *ipcm,
	struct ipcm_ctx *ctx);

extern void drime4_ipcm_fw_stop_liveview(struct drime4_ipcm *ipcm,
	struct ipcm_ctx *ctx);

extern void drime4_ipcm_fw_start_resize(struct drime4_ipcm *ipcm,
	struct ipcm_ctx *ctx);

extern void drime4_ipcm_fw_stop_resize(struct drime4_ipcm *ipcm,
	struct ipcm_ctx *ctx);

extern void drime4_ipcm_fw_start_md(struct drime4_ipcm *ipcm,
	struct ipcm_ctx *ctx);

extern long drime4_ipcm_fw_sub_module_ctrl(
	struct drime4_ipcm *ipcm, struct v4l2_d4_private_control *ctrl);
#endif

