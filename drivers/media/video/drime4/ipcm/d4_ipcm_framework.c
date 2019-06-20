/* linux/drivers/media/video/drime4/ipcm/d4_ipcm_framework.c
 *
 * Copyright (c) 2011 Samsung Electronics
 *	Jangwon Lee <jang_won.lee@samsung.com>
 *
 * IPCM Framework driver.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

/*
#define DEBUG
*/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/mutex.h>
#include <linux/mm.h>
#include <linux/platform_device.h>
#include <linux/memory.h>
#include <linux/ctype.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/videodev2.h>
#include <linux/videodev2_drime4.h>

#include <asm/bug.h>

#include <mach/d4_cma.h>
#include <media/drime4/usih_type.h>
#include <media/drime4/ipcm/d4_ipcm_type.h>

#include "mach/ipcm/d4-ipcm.h"
#include "d4_ipcm_if.h"
#include "d4_ipcm_submodule_if.h"
#include "d4_ipcm_framework.h"

extern struct drime4_ipcm *g_ipcm;
extern struct ipcm_ctx *g_ipcm_ctx;
extern struct device *ipcm_dev;
extern void drime4_ipcm_send_interrupt(unsigned int intr_type);
extern unsigned int ipcm_liveview_operation;

/* for debugging */
extern void drime4_ipcm_fw_print_resize_parameter(
	struct ipcm_ycc_resize_info *resize_param);
extern void drime4_ipcm_fw_print_combi_op_parameter(
	struct ipcm_ycc_combi_op_info *combi_op_param);

void drime4_ipcm_fw_wdma0_intr_callback_func(void)
{
	drime4_ipcm_send_interrupt(INTR_IPCM_B2Y_DONE);
	complete(&g_ipcm->wdma0_completion);
	dev_dbg(ipcm_dev, "IPCM WDMA0 Interrupt call back.\n");
}

void drime4_ipcm_fw_wdma1_intr_callback_func(void)
{
	drime4_ipcm_send_interrupt(INTR_IPCM_RSZ_DONE);
	complete(&g_ipcm->wdma1_completion);
	dev_dbg(ipcm_dev, "IPCM WDMA1 Interrupt call back.\n");
}

void drime4_ipcm_fw_wdma2_intr_callback_func(void)
{
	drime4_ipcm_send_interrupt(INTR_IPCM_SRSZ_DONE);
	complete(&g_ipcm->wdma2_completion);
	dev_dbg(ipcm_dev, "IPCM WDMA2 Interrupt call back.\n");
}

void drime4_ipcm_fw_wdma3_intr_callback_func(void)
{
	drime4_ipcm_send_interrupt(INTR_IPCM_LDC_DONE);
	complete(&g_ipcm->wdma3_completion);
	dev_dbg(ipcm_dev, "IPCM WDMA3 Interrupt call back.\n");
}

void drime4_ipcm_fw_md_gmd_intr_callback_func(void)
{
	drime4_ipcm_send_interrupt(INTR_IPCM_MD_GMD_DONE);
	dev_dbg(ipcm_dev, "IPCM MD(GMD) Interrupt call back.\n");
}

void drime4_ipcm_fw_md_rmd_intr_callback_func(void)
{
	drime4_ipcm_send_interrupt(INTR_IPCM_MD_RMD_DONE);
	dev_dbg(ipcm_dev, "IPCM MD(RMD) Interrupt call back.\n");
}

void drime4_ipcm_fw_init(struct drime4_ipcm *ipcm)
{
	WARN_ON(ipcm == NULL);

	/* load ipcm core default register value */
	d4_ipcm_load_default_regset();

	/* IPCM Interrupt initialization */
	d4_ipcm_sys_int_init();
}

int drime4_ipcm_fw_sleep(struct drime4_ipcm *ipcm,
	struct ipcm_ctx *ctx)
{
	if (!g_ipcm_ctx)
		g_ipcm_ctx = kzalloc(sizeof(struct ipcm_ctx), GFP_KERNEL);

	/* sleep when liveview operation is working */
	if (ipcm_liveview_operation == IPCM_LIVEVIEW_ON)
		drime4_ipcm_fw_stop_liveview(ipcm, ctx);

	/* store ipcm current context info. */
	if (ctx)
		memcpy(g_ipcm_ctx, ctx, sizeof(struct ipcm_ctx));

	/* disable ipcs clock. */
	clk_disable(ipcm->clock);

	return 0;
}

int drime4_ipcm_fw_wakeup(struct drime4_ipcm *ipcm,
	struct ipcm_ctx *ctx)
{
	/* enalbe ipcm clock */
	clk_enable(g_ipcm->clock);
	/* IPCM Clock freq. */
	clk_set_rate(g_ipcm->clock, 200000000);

	/* load IPCM core default register value */
	d4_ipcm_load_default_regset();

	/* IPCM Interrupt initialization */
	d4_ipcm_sys_int_init();

	/* wake up when liveview operation is working */
	if (ipcm_liveview_operation == IPCM_LIVEVIEW_ON)
		drime4_ipcm_fw_start_liveview(g_ipcm, g_ipcm_ctx);

	return 0;
}

void drime4_ipcm_fw_enable_top_clks(struct drime4_ipcm *ipcm,
	struct ipcm_ctx *ctx)
{
	WARN_ON((ipcm == NULL) || (ctx == NULL));
}

void drime4_ipcm_fw_enable_sys_clks(struct drime4_ipcm *ipcm,
	struct ipcm_ctx *ctx, unsigned int enable)
{
	WARN_ON((ipcm == NULL) || (ctx == NULL));
}

int drime4_ipcm_fw_set_src_param(struct ipcm_ctx *ctx,
	struct ipcm_frame *frame)
{
	struct ipcm_bayer_info *bayer_input = NULL;
	struct ipcm_ycc_info *ycc_input = NULL;
	struct ipcm_md_info *md_input = NULL;

	switch (ctx->v4l2_op_mode) {
	case V4L2_CID_DRIME_IPC_LIVEVIEW:
		/* set liveview source information */
		bayer_input = &ctx->liveview_info.Src_Img_Bayer;

		/* set bayer input parameters */
		bayer_input->width = frame->bayer_info.width;
		bayer_input->height = frame->bayer_info.height;
		bayer_input->BitPerPix = frame->bayer_info.BitPerPix;

		/* temp: distinguish liveview or movie */
		/*
		if (bayer_input->width > 640)
			bayer_input->input_ch = IPCM_IN_4CH;
		else
			bayer_input->input_ch = IPCM_IN_2CH;
		*/
		bayer_input->input_ch = IPCM_IN_4CH;

		bayer_input->head_color = frame->bayer_info.head_color;
		/* flag: data through On The Flay(OTF) from pp*/
		bayer_input->bayer_packed_flag = 1;
		bayer_input->stride = frame->bayer_info.stride;

		/* temp */
		bayer_input->base_addr_ch0 = 0X0;
		bayer_input->base_addr_ch1 = 0X0;
		bayer_input->startx = 0;
		break;

	case V4L2_CID_DRIME_IPC_RESIZE:
		/* resize with liveview */
		if (ipcm_liveview_operation == IPCM_LIVEVIEW_ON)
			ycc_input = &ctx->combi_op_info.Src_Img_Ycc;
		/* resize only (without liveview) */
		else
			ycc_input = &ctx->resize_info.Src_Img_Ycc;

		/* set ycc input parameters */
		ycc_input->width = frame->ycc_info.width;
		ycc_input->height = frame->ycc_info.height;
		ycc_input->stride_y = frame->ycc_info.stride_y;
		ycc_input->stride_c = frame->ycc_info.stride_c;
		ycc_input->ycc_type = frame->ycc_info.ycc_type;
		ycc_input->ss_mode = IPCM_OUT_SS_CENTER;
		ycc_input->scan_type = frame->ycc_info.scan_type;
		ycc_input->pix_offset_x = 0;
		ycc_input->pix_offset_y = 0;
		ycc_input->reverse_CbCr = 0;
		ycc_input->NLC_sw = 0;
		ycc_input->NLC_rc = 0;
		break;

	case V4L2_CID_DRIME_IPC_MD:
		md_input = &ctx->md_info;
		md_input->in_width = frame->ycc_info.width;
		md_input->in_height = frame->ycc_info.height;
	default:
		break;
	}

	return 0;
}

int drime4_ipcm_fw_set_dst_param(struct ipcm_ctx *ctx,
	struct ipcm_frame *frame)
{
	struct drime4_ipcm *ipcm = NULL;
	struct ipcm_ycc_info *ycc_output = NULL;
	struct ipcm_md_info *md_output = NULL;

	ipcm = ctx->ipcm_dev;

	switch (ctx->v4l2_op_mode) {
	case V4L2_CID_DRIME_IPC_LIVEVIEW:
		/* main out: liveview with no resize */
		if (ctx->curr_buf_type == IPCM_DST_MAIN_BUFFER) {
			ctx->liveview_info.MainOut_SW = IPCM_SW_ON;
			ycc_output = &ctx->liveview_info.MainOut_Img;
		/* resize out */
		} else if (ctx->curr_buf_type == IPCM_DST_RSZ_BUFFER) {
			ctx->liveview_info.RSZ_SW = IPCM_SW_ON;
			ycc_output = &ctx->liveview_info.RSZOut_Img;
		/* sub-resize out */
		} else if (ctx->curr_buf_type == IPCM_DST_SRSZ_BUFFER) {
			ctx->liveview_info.SRSZ_SW = IPCM_SW_ON;
			ycc_output = &ctx->liveview_info.SRSZOut_Img;
		}

		if (ycc_output == NULL) {
			dev_err(ipcm->dev, "%s:wrong buffer type.\n", __func__);
			return -EINVAL;
		} else {
			/* set IPCM destination parameters */
			ycc_output->width =
				frame->ycc_info.width;
			ycc_output->height =
				frame->ycc_info.height;
			ycc_output->stride_y =
				frame->ycc_info.stride_y;
			ycc_output->stride_c =
				frame->ycc_info.stride_c;
			ycc_output->ycc_type =
				frame->ycc_info.ycc_type;
			ycc_output->ss_mode =
				IPCM_OUT_SS_CENTER;
			ycc_output->scan_type =
				frame->ycc_info.scan_type;
			ycc_output->pix_offset_x = 0;
			ycc_output->pix_offset_y = 0;
			ycc_output->reverse_CbCr = 0;
			ycc_output->NLC_sw = 0;
			ycc_output->NLC_rc = 0;
		}

		/* Even if user does not use main out image,
		 * main out image size should be set. */
		if (ctx->liveview_info.MainOut_SW == IPCM_SW_OFF) {
			ctx->liveview_info.MainOut_Img.width =
				ctx->liveview_info.Src_Img_Bayer.width;
			ctx->liveview_info.MainOut_Img.height =
				ctx->liveview_info.Src_Img_Bayer.height;
		}
		break;

	case V4L2_CID_DRIME_IPC_RESIZE:
		/* resize with liveview */
		if (ipcm_liveview_operation == IPCM_LIVEVIEW_ON) {
			/* Resize Out */
			if (ctx->curr_buf_type == IPCM_DST_RSZ_BUFFER)
				ycc_output = &ctx->combi_op_info.RSZOut_Img;
			/* Sub-Resize Out */
			else if (ctx->curr_buf_type == IPCM_DST_SRSZ_BUFFER)
				ycc_output = &ctx->combi_op_info.SRSZOut_Img;
		/* resize only (without liveview) */
		} else {
			/* Main Out */
			if (ctx->curr_buf_type == IPCM_DST_MAIN_BUFFER)
				ycc_output = &ctx->resize_info.MainOut_Img;
			/* Resize Out */
			else if (ctx->curr_buf_type == IPCM_DST_RSZ_BUFFER)
				ycc_output = &ctx->resize_info.RSZOut_Img;
			/* Sub-Resize Out */
			else if (ctx->curr_buf_type == IPCM_DST_SRSZ_BUFFER)
				ycc_output = &ctx->resize_info.SRSZOut_Img;
		}

		if (ycc_output == NULL) {
			dev_err(ipcm->dev, "%s:wrong buffer type.\n", __func__);
			return -EINVAL;
		} else {
			/* set IPCM destination parameters */
			ycc_output->width =
				frame->ycc_info.width;
			ycc_output->height =
				frame->ycc_info.height;
			ycc_output->stride_y =
				frame->ycc_info.stride_y;
			ycc_output->stride_c =
				frame->ycc_info.stride_c;
			ycc_output->ycc_type =
				frame->ycc_info.ycc_type;
			ycc_output->ss_mode =
				IPCM_OUT_SS_CENTER;
			ycc_output->scan_type =
				frame->ycc_info.scan_type;
			ycc_output->pix_offset_x = 0;
			ycc_output->pix_offset_y = 0;
			ycc_output->reverse_CbCr = 0;
			ycc_output->NLC_sw = 0;
			ycc_output->NLC_rc = 0;
		}
		break;

	case V4L2_CID_DRIME_IPC_MD:
		md_output = &ctx->md_info;
		md_output->mc_width = frame->ycc_info.width;
		md_output->mc_height = frame->ycc_info.height;
		break;
	default:
		break;
	}
	return 0;
}

struct ipcm_mem_info *drime4_ipcm_fw_get_buffer_info(
	struct drime4_ipcm *ipcm, struct ipcm_ctx *ctx)
{
	unsigned int index = 0;
	struct ipcm_mem_info *mem_info = NULL;

	if ((ipcm == NULL) || (ctx == NULL)) {
		dev_dbg(ipcm->dev,
			"%s: ipcm/ctx indicates NULL pointer.", __func__);
		return -EINVAL;
	}

	/* for source. */
	if (ctx->curr_buf_type == IPCM_SRC_BUFFER) {
		switch (ctx->v4l2_op_mode) {
		case V4L2_CID_DRIME_IPC_LIVEVIEW:
			break;
		case V4L2_CID_DRIME_IPC_RESIZE:
			if (ctx->ycc_in_buf.is_allocated) {
				index = ctx->ycc_in_buf.index;
				mem_info = ctx->ycc_in_buf.meminfo[index];
			} else {
				dev_err(ipcm->dev, "%s: there are no requested buffer.\n", __func__);
				return NULL;
			}
			break;
		default:
			break;
		}
	/* for destination. */
	} else if (ctx->curr_buf_type == IPCM_DST_MAIN_BUFFER) {
		if (ctx->main_out_buf.is_allocated) {
			index = ctx->main_out_buf.index;
			mem_info = ctx->main_out_buf.meminfo[index];
		} else {
			dev_err(ipcm->dev, "%s: there are no requested buffer.\n", __func__);
			return NULL;
		}
	} else if (ctx->curr_buf_type == IPCM_DST_RSZ_BUFFER) {
		if (ctx->rsz_out_buf.is_allocated) {
			index = ctx->rsz_out_buf.index;
			mem_info = ctx->rsz_out_buf.meminfo[index];
		} else {
			dev_err(ipcm->dev, "%s: there are no requested buffer.\n", __func__);
			return NULL;
		}
	} else if (ctx->curr_buf_type == IPCM_DST_SRSZ_BUFFER) {
		if (ctx->subrsz_out_buf.is_allocated) {
			index = ctx->subrsz_out_buf.index;
			mem_info = ctx->subrsz_out_buf.meminfo[index];
		} else {
			dev_err(ipcm->dev, "%s: there are no requested buffer.\n", __func__);
			return NULL;
		}
	} else {
		dev_err(ipcm->dev, "%s:wrong buffer type.\n", __func__);
		return NULL;
	}

	return mem_info;
}

int drime4_ipcm_fw_free_buffer_info(struct drime4_ipcm *ipcm,
	struct ipcm_ctx *ctx)
{
	int ret = 0;
	unsigned int i = 0;

	if ((ipcm == NULL) || (ctx == NULL)) {
		dev_dbg(ipcm->dev,
			"%s: ipcm/ctx indicates NULL pointer.", __func__);
		return -EINVAL;
	}

	/* if buffer was allocated then free it's info. */
	if (ctx->bayer_in_buf.is_allocated) {
		WARN_ON(ctx->bayer_in_buf.meminfo == NULL);

		for (i = 0; i < ctx->bayer_in_buf.buf_cnt; i++) {
			WARN_ON(ctx->bayer_in_buf.meminfo[i] == NULL);
			kfree(ctx->bayer_in_buf.meminfo[i]);
		}
		if (ctx->bayer_in_buf.meminfo) {
			kfree(ctx->bayer_in_buf.meminfo);
			ctx->bayer_in_buf.meminfo = NULL;
		}
	}

	/* if buffer was allocated then free it's info. */
	if (ctx->ycc_in_buf.is_allocated) {
		WARN_ON(ctx->ycc_in_buf.meminfo == NULL);

		for (i = 0; i < ctx->ycc_in_buf.buf_cnt; i++) {
			WARN_ON(ctx->ycc_in_buf.meminfo[i] == NULL);
			kfree(ctx->ycc_in_buf.meminfo[i]);
		}
		if (ctx->ycc_in_buf.meminfo) {
			kfree(ctx->ycc_in_buf.meminfo);
			ctx->ycc_in_buf.meminfo = NULL;
		}
	}

	/* if buffer was allocated then free it's info. */
	if (ctx->main_out_buf.is_allocated) {
		WARN_ON(ctx->main_out_buf.meminfo == NULL);

		for (i = 0; i < ctx->main_out_buf.buf_cnt; i++) {
			WARN_ON(ctx->main_out_buf.meminfo[i] == NULL);
			kfree(ctx->main_out_buf.meminfo[i]);
		}
		if (ctx->main_out_buf.meminfo) {
			kfree(ctx->main_out_buf.meminfo);
			ctx->main_out_buf.meminfo = NULL;
		}
	}

	/* if buffer was allocated then free it's info. */
	if (ctx->rsz_out_buf.is_allocated) {
		WARN_ON(ctx->rsz_out_buf.meminfo == NULL);

		for (i = 0; i < ctx->rsz_out_buf.buf_cnt; i++) {
			WARN_ON(ctx->rsz_out_buf.meminfo[i] == NULL);
			kfree(ctx->rsz_out_buf.meminfo[i]);
		}
		if (ctx->rsz_out_buf.meminfo) {
			kfree(ctx->rsz_out_buf.meminfo);
			ctx->rsz_out_buf.meminfo = NULL;
		}
	}

	/* if buffer was allocated then free it's info. */
	if (ctx->subrsz_out_buf.is_allocated) {
		WARN_ON(ctx->subrsz_out_buf.meminfo == NULL);

		for (i = 0; i < ctx->subrsz_out_buf.buf_cnt; i++) {
			WARN_ON(ctx->subrsz_out_buf.meminfo[i] == NULL);
			kfree(ctx->subrsz_out_buf.meminfo[i]);
		}
		if (ctx->subrsz_out_buf.meminfo) {
			kfree(ctx->subrsz_out_buf.meminfo);
			ctx->subrsz_out_buf.meminfo = NULL;
		}
	}

	return ret;
}

int drime4_ipcm_fw_set_bayer_inbuf(struct drime4_ipcm *ipcm,
	struct v4l2_requestbuffers *b)
{
	WARN_ON(ipcm == NULL);

	return 0;
}

int drime4_ipcm_fw_set_ycc_inbuf(struct drime4_ipcm *ipcm,
	struct v4l2_requestbuffers *b)
{
	unsigned int buf_size = 0, align = 4096, id = 0, i = 0;
	unsigned int plane_index = 0;
	struct ipcm_ctx *ctx = ipcm->ipcm_ctx;
	struct ipcm_buffer *ycc_in_buf = NULL;
	struct ipcm_mem_info mem_info;
	struct ipcm_ycc_info *ycc_in_img = NULL;

	ycc_in_buf = &ctx->ycc_in_buf;

	if (ipcm_liveview_operation == IPCM_LIVEVIEW_ON)
		ycc_in_img = &ctx->combi_op_info.Src_Img_Ycc;
	else
		ycc_in_img = &ctx->resize_info.Src_Img_Ycc;

	if (ycc_in_buf->is_allocated)
		dev_dbg(ipcm->dev, "%s:already allocated.\n", __func__);

	ycc_in_buf->meminfo =
		(struct ipcm_mem_info **)kzalloc(sizeof(struct ipcm_mem_info *)
					* b->count, GFP_KERNEL);

	WARN_ON(ycc_in_buf->meminfo == NULL);

	/* set buffer size */
	if (ycc_in_img->ycc_type == IPCM_YCC422)
		buf_size = ycc_in_img->stride_y *
				ycc_in_img->height * 2;
	/* in case of ycc 420. */
	else {
		buf_size = ycc_in_img->stride_y *
				ycc_in_img->height;
		buf_size += buf_size / 2;
	}

	/* allocate buffers as count requested by user. */
	for (i = 0; i < b->count; i++) {
		ycc_in_buf->meminfo[i] =
			kzalloc(sizeof(struct ipcm_mem_info), GFP_KERNEL);

		WARN_ON(ycc_in_buf->meminfo[i] == NULL);

		if (b->memory == V4L2_MEMORY_MMAP) {

			/* check multi plane use */
			if (b->type == V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE) {
				plane_index = (i % 2);
				buf_size =
					ctx->src_frame.plane_buf_size[plane_index];

				dev_dbg(ipcm->dev, "%s:plane_index:%d, buf_size = %d\n",
					__func__, plane_index, buf_size);
			}

			buf_size = ALIGN(buf_size, align);

			/* get physical address using cma */
			mem_info.phy_addr =
				cma_alloc(ipcm->dev, "c", buf_size, align);

			WARN_ON(mem_info.phy_addr < 0);

			/* convert address physical to virtual */
			mem_info.vir_addr =
				(unsigned int)phys_to_virt(mem_info.phy_addr);

			dev_dbg(ipcm->dev, "%s:vir_addr=%x, buf_size = %d\n",
					__func__, mem_info.vir_addr, buf_size);

			ycc_in_buf->meminfo[i]->id	= id++;
			ycc_in_buf->meminfo[i]->size = buf_size;
			ycc_in_buf->meminfo[i]->vir_addr = mem_info.vir_addr;
			ycc_in_buf->meminfo[i]->phy_addr = mem_info.phy_addr;
		/* in case of userptr */
		} else
			ycc_in_buf->meminfo[i]->id = -1;
	}

	/* set buffer count. */
	ycc_in_buf->buf_cnt = b->count;

	/* initialize index to 0. */
	ycc_in_buf->index = 0;

	if (b->memory == V4L2_MEMORY_MMAP) {
		/* first buffer index should be 0. */
		ycc_in_img->addr_y0 = ycc_in_buf->meminfo[0]->phy_addr;
		ycc_in_img->addr_y1 = ycc_in_img->addr_y0 +
				ycc_in_img->stride_y * (ycc_in_img->height / 2);
		/* check multi plane use */
		if (b->type == V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE) {
			ycc_in_img->addr_c0 =
				ycc_in_buf->meminfo[1]->phy_addr;
		} else {
			ycc_in_img->addr_c0 = ycc_in_img->addr_y0 +
					ycc_in_img->stride_y * ycc_in_img->height;
		}
		/* temp - progressive, ycc 422 */
		if (ycc_in_img->ycc_type == IPCM_YCC422) {
			ycc_in_img->addr_c1 =
				ycc_in_img->addr_c0 +
					 ycc_in_img->stride_y * (ycc_in_img->height / 2);
		/* temp - progressive, ycc 420 */
		} else {
			ycc_in_img->addr_c1 =
				ycc_in_img->addr_c0 +
					 ycc_in_img->stride_y * (ycc_in_img->height / 4);
		}
	}

	/* flag this memory is allocated. */
	ycc_in_buf->is_allocated = 1;

	return 0;
}

int drime4_ipcm_fw_set_main_outbuf(struct drime4_ipcm *ipcm,
	struct v4l2_requestbuffers *b)
{
	unsigned int buf_size = 0, align = 4096, id = 0, i = 0;
	unsigned int plane_index = 0;
	struct ipcm_ctx *ctx = ipcm->ipcm_ctx;
	struct ipcm_buffer *main_out_buf = NULL;
	struct ipcm_mem_info mem_info;
	struct ipcm_ycc_info *main_out_img = NULL;

	main_out_buf = &ctx->main_out_buf;

	/* set image info according to data path */
	switch (ctx->v4l2_op_mode) {
	case V4L2_CID_DRIME_IPC_LIVEVIEW:
		main_out_img = &ctx->liveview_info.MainOut_Img;
		break;

	case V4L2_CID_DRIME_IPC_RESIZE:
		/* resize with liveview */
		if (ipcm_liveview_operation == IPCM_LIVEVIEW_ON)
			dev_dbg(ipcm->dev, "%s:ipcm_liveview_on.\n", __func__);
		/* resize only (without liveview) */
		else
			main_out_img = &ctx->resize_info.MainOut_Img;

		break;

	default:
		break;
	}

	if (main_out_buf->is_allocated)
		dev_dbg(ipcm->dev, "%s:already allocated.\n", __func__);

	main_out_buf->meminfo =
		(struct ipcm_mem_info **)kzalloc(sizeof(struct ipcm_mem_info *)
					* b->count, GFP_KERNEL);

	WARN_ON(main_out_buf->meminfo == NULL);

	/* set buffer size */
	if (main_out_img->ycc_type == IPCM_YCC422)
		buf_size = main_out_img->stride_y *
				main_out_img->height * 2;
	/* in case of ycc 420. */
	else {
		buf_size = main_out_img->stride_y *
				main_out_img->height;
		buf_size += buf_size / 2;
	}

	/* allocate buffers as count requested by user. */
	for (i = 0; i < b->count; i++) {
		main_out_buf->meminfo[i] =
			kzalloc(sizeof(struct ipcm_mem_info), GFP_KERNEL);

		WARN_ON(main_out_buf->meminfo[i] == NULL);

		if (b->memory == V4L2_MEMORY_MMAP) {
			/* check multi plane use */
			if (b->type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) {
				plane_index = (i % 2);
				buf_size =
					ctx->dst_frame.plane_buf_size[plane_index];

				dev_dbg(ipcm->dev, "%s:plane_index:%d, buf_size = %d\n",
					__func__, plane_index, buf_size);
			}

			buf_size = ALIGN(buf_size, align);

			/* get physical address using cma */
			mem_info.phy_addr =
				cma_alloc(ipcm->dev, "c", buf_size, align);

			WARN_ON(mem_info.phy_addr < 0);

			/* convert address physical to virtual */
			mem_info.vir_addr =
				(unsigned int)phys_to_virt(mem_info.phy_addr);

			dev_dbg(ipcm->dev, "%s:vir_addr=%x, buf_size = %d\n",
					__func__, mem_info.vir_addr, buf_size);

			main_out_buf->meminfo[i]->id	= id++;
			main_out_buf->meminfo[i]->size = buf_size;
			main_out_buf->meminfo[i]->vir_addr = mem_info.vir_addr;
			main_out_buf->meminfo[i]->phy_addr = mem_info.phy_addr;
		/* in case of userptr */
		} else
			main_out_buf->meminfo[i]->id = -1;
	}

	/* set buffer count. */
	main_out_buf->buf_cnt = b->count;

	/* initialize index to 0. */
	main_out_buf->index = 0;

	if (b->memory == V4L2_MEMORY_MMAP) {
		/* first buffer index should be 0. */
		main_out_img->addr_y0 = main_out_buf->meminfo[0]->phy_addr;
		main_out_img->addr_y1 = main_out_img->addr_y0 +
				main_out_img->stride_y * (main_out_img->height / 2);
		/* check multi plane use */
		if (b->type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) {
			main_out_img->addr_c0 =
				main_out_buf->meminfo[1]->phy_addr;
		} else {
			main_out_img->addr_c0 = main_out_img->addr_y0 +
					main_out_img->stride_y * main_out_img->height;
		}
		/* temp - progressive, ycc 422 */
		if (main_out_img->ycc_type == IPCM_YCC422) {
			main_out_img->addr_c1 =
				main_out_img->addr_c0 +
					 main_out_img->stride_y * (main_out_img->height / 2);
		/* temp - progressive, ycc 420 */
		} else {
			main_out_img->addr_c1 =
				main_out_img->addr_c0 +
					 main_out_img->stride_y * (main_out_img->height / 4);
		}
	}

	/* flag this memory is allocated. */
	main_out_buf->is_allocated = 1;

	return 0;
}

int drime4_ipcm_fw_set_rsz_outbuf(struct drime4_ipcm *ipcm,
	struct v4l2_requestbuffers *b)
{
	unsigned int buf_size = 0, align = 4096, id = 0, i = 0;
	unsigned int plane_index = 0;
	struct ipcm_ctx *ctx = ipcm->ipcm_ctx;
	struct ipcm_buffer *rsz_out_buf = NULL;
	struct ipcm_mem_info mem_info;
	struct ipcm_ycc_info *rsz_out_img = NULL;

	rsz_out_buf = &ctx->rsz_out_buf;

	/* set image info according to data path */
	switch (ctx->v4l2_op_mode) {
	case V4L2_CID_DRIME_IPC_LIVEVIEW:
		rsz_out_img = &ctx->liveview_info.RSZOut_Img;
		break;

	case V4L2_CID_DRIME_IPC_RESIZE:
		/* resize with liveview */
		if (ipcm_liveview_operation == IPCM_LIVEVIEW_ON)
			rsz_out_img = &ctx->combi_op_info.RSZOut_Img;
		/* resize only (without liveview) */
		else
			rsz_out_img = &ctx->resize_info.RSZOut_Img;

		break;

	default:
		break;
	}

	if (rsz_out_buf->is_allocated)
		dev_dbg(ipcm->dev, "%s:already allocated.\n", __func__);

	rsz_out_buf->meminfo =
		(struct ipcm_mem_info **)kzalloc(sizeof(struct ipcm_mem_info *)
					* b->count, GFP_KERNEL);

	WARN_ON(rsz_out_buf->meminfo == NULL);

	/* set buffer size */
	if (rsz_out_img->ycc_type == IPCM_YCC422)
		buf_size = rsz_out_img->stride_y *
				rsz_out_img->height * 2;
	/* in case of ycc 420. */
	else {
		buf_size = rsz_out_img->stride_y *
				rsz_out_img->height;
		buf_size += buf_size / 2;
	}

	/* allocate buffers as count requested by user. */
	for (i = 0; i < b->count; i++) {
		rsz_out_buf->meminfo[i] =
			kzalloc(sizeof(struct ipcm_mem_info), GFP_KERNEL);

		WARN_ON(rsz_out_buf->meminfo[i] == NULL);

		if (b->memory == V4L2_MEMORY_MMAP) {
			/* check multi plane use */
			if (b->type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) {
				plane_index = (i % 2);
				buf_size =
					ctx->dst_frame.plane_buf_size[plane_index];

				dev_dbg(ipcm->dev, "%s:plane_index:%d,buf_size = %d\n",
					__func__, plane_index, buf_size);
			}

			buf_size = ALIGN(buf_size, align);

			/* get physical address using cma */
			mem_info.phy_addr =
				cma_alloc(ipcm->dev, "c", buf_size, align);

			WARN_ON(mem_info.phy_addr < 0);

			/* convert address physical to virtual */
			mem_info.vir_addr =
				(unsigned int)phys_to_virt(mem_info.phy_addr);

			dev_dbg(ipcm->dev, "%s:vir_addr=%x, buf_size = %d\n",
					__func__, mem_info.vir_addr, buf_size);

			rsz_out_buf->meminfo[i]->id	= id++;
			rsz_out_buf->meminfo[i]->size = buf_size;
			rsz_out_buf->meminfo[i]->vir_addr = mem_info.vir_addr;
			rsz_out_buf->meminfo[i]->phy_addr = mem_info.phy_addr;
		/* in case of userptr */
		} else
			rsz_out_buf->meminfo[i]->id = -1;
	}

	/* set buffer count. */
	rsz_out_buf->buf_cnt = b->count;

	/* initialize index to 0. */
	rsz_out_buf->index = 0;

	if (b->memory == V4L2_MEMORY_MMAP) {
		/* first buffer index should be 0. */
		rsz_out_img->addr_y0 = rsz_out_buf->meminfo[0]->phy_addr;
		rsz_out_img->addr_y1 = rsz_out_img->addr_y0 +
				rsz_out_img->stride_y * (rsz_out_img->height / 2);
		/* check multi plane use */
		if (b->type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) {
			rsz_out_img->addr_c0 =
				rsz_out_buf->meminfo[1]->phy_addr;
		} else {
			rsz_out_img->addr_c0 = rsz_out_img->addr_y0 +
					rsz_out_img->stride_y * rsz_out_img->height;
		}
		/* temp - progressive, ycc 422 */
		if (rsz_out_img->ycc_type == IPCM_YCC422) {
			rsz_out_img->addr_c1 =
				rsz_out_img->addr_c0 +
					 rsz_out_img->stride_y * (rsz_out_img->height / 2);
		/* temp - progressive, ycc 420 */
		} else {
			rsz_out_img->addr_c1 =
				rsz_out_img->addr_c0 +
					 rsz_out_img->stride_y * (rsz_out_img->height / 4);
		}
	}

	/* flag this memory is allocated. */
	rsz_out_buf->is_allocated = 1;

	return 0;
}

int drime4_ipcm_fw_set_subrsz_outbuf(struct drime4_ipcm *ipcm,
	struct v4l2_requestbuffers *b)
{
	unsigned int buf_size = 0, align = 4096, id = 0, i = 0;
	unsigned int plane_index = 0;
	struct ipcm_ctx *ctx = ipcm->ipcm_ctx;
	struct ipcm_buffer *subrsz_out_buf = NULL;
	struct ipcm_mem_info mem_info;
	struct ipcm_ycc_info *subrsz_out_img = NULL;

	subrsz_out_buf = &ctx->subrsz_out_buf;

	/* set image info according to data path */
	switch (ctx->v4l2_op_mode) {
	case V4L2_CID_DRIME_IPC_LIVEVIEW:
		subrsz_out_img = &ctx->liveview_info.SRSZOut_Img;
		break;

	case V4L2_CID_DRIME_IPC_RESIZE:
		/* resize with liveview */
		if (ipcm_liveview_operation == IPCM_LIVEVIEW_ON)
			subrsz_out_img = &ctx->combi_op_info.SRSZOut_Img;
		/* resize only (without liveview) */
		else
			subrsz_out_img = &ctx->resize_info.SRSZOut_Img;

		break;

	default:
		break;
	}

	if (subrsz_out_buf->is_allocated)
		dev_dbg(ipcm->dev, "%s:already allocated.\n", __func__);

	subrsz_out_buf->meminfo =
		(struct ipcm_mem_info **)kzalloc(sizeof(struct ipcm_mem_info *)
					* b->count, GFP_KERNEL);

	WARN_ON(subrsz_out_buf->meminfo == NULL);

	/* set buffer size */
	if (subrsz_out_img->ycc_type == IPCM_YCC422)
		buf_size = subrsz_out_img->stride_y *
				subrsz_out_img->height * 2;
	/* in case of ycc 420. */
	else {
		buf_size = subrsz_out_img->stride_y *
				subrsz_out_img->height;
		buf_size += buf_size / 2;
	}

	/* allocate buffers as count requested by user. */
	for (i = 0; i < b->count; i++) {
		subrsz_out_buf->meminfo[i] =
			kzalloc(sizeof(struct ipcm_mem_info), GFP_KERNEL);

		WARN_ON(subrsz_out_buf->meminfo[i] == NULL);

		if (b->memory == V4L2_MEMORY_MMAP) {
			/* check multi plane use */
			if (b->type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) {
				plane_index = (i % 2);
				buf_size =
					ctx->dst_frame.plane_buf_size[plane_index];

				dev_dbg(ipcm->dev, "%s:plane_index:%d,buf_size = %d\n",
					__func__, plane_index, buf_size);
			}

			buf_size = ALIGN(buf_size, align);

			/* get physical address using cma */
			mem_info.phy_addr =
				cma_alloc(ipcm->dev, "c", buf_size, align);

			WARN_ON(mem_info.phy_addr < 0);

			/* convert address physical to virtual */
			mem_info.vir_addr =
				(unsigned int)phys_to_virt(mem_info.phy_addr);

			dev_dbg(ipcm->dev, "%s:vir_addr=%x, buf_size = %d\n",
					__func__, mem_info.vir_addr, buf_size);

			subrsz_out_buf->meminfo[i]->id	= id++;
			subrsz_out_buf->meminfo[i]->size = buf_size;
			subrsz_out_buf->meminfo[i]->vir_addr = mem_info.vir_addr;
			subrsz_out_buf->meminfo[i]->phy_addr = mem_info.phy_addr;
		/* in case of userptr */
		} else
			subrsz_out_buf->meminfo[i]->id = -1;
	}

	/* set buffer count. */
	subrsz_out_buf->buf_cnt = b->count;

	/* initialize index to 0. */
	subrsz_out_buf->index = 0;

	if (b->memory == V4L2_MEMORY_MMAP) {
		/* first buffer index should be 0. */
		subrsz_out_img->addr_y0 = subrsz_out_buf->meminfo[0]->phy_addr;
		subrsz_out_img->addr_y1 = subrsz_out_img->addr_y0 +
				subrsz_out_img->stride_y * (subrsz_out_img->height / 2);
		/* check multi plane use */
		if (b->type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) {
			subrsz_out_img->addr_c0 =
				subrsz_out_buf->meminfo[1]->phy_addr;
		} else {
			subrsz_out_img->addr_c0 = subrsz_out_img->addr_y0 +
					subrsz_out_img->stride_y * subrsz_out_img->height;
		}
		/* temp - progressive, ycc 422 */
		if (subrsz_out_img->ycc_type == IPCM_YCC422) {
			subrsz_out_img->addr_c1 =
				subrsz_out_img->addr_c0 +
					 subrsz_out_img->stride_y * (subrsz_out_img->height / 2);
		/* temp - progressive, ycc 420 */
		} else {
			subrsz_out_img->addr_c1 =
				subrsz_out_img->addr_c0 +
					 subrsz_out_img->stride_y * (subrsz_out_img->height / 4);
		}
	}

	/* flag this memory is allocated. */
	subrsz_out_buf->is_allocated = 1;

	return 0;
}

int drime4_ipcm_fw_set_ycc_in_addr(struct drime4_ipcm *ipcm,
	struct ipcm_ctx *ctx, struct v4l2_buffer *b)
{
	unsigned int phy_addr;
	struct ipcm_buffer *ycc_in_buf = NULL;
	struct ipcm_ycc_info *ycc_in_img = NULL;
	unsigned int plane_index = 0;

	if ((ipcm == NULL) || (ctx == NULL)) {
		dev_dbg(ipcm->dev,
			"%s: ipcm/ctx indicates NULL pointer.", __func__);
		return -EINVAL;
	}

	if (ipcm_liveview_operation == IPCM_LIVEVIEW_ON)
		ycc_in_img = &ctx->combi_op_info.Src_Img_Ycc;
	else
		ycc_in_img = &ctx->resize_info.Src_Img_Ycc;

	ycc_in_buf = &ctx->ycc_in_buf;

	if (b->index > ycc_in_buf->buf_cnt) {
		dev_err(ipcm->dev,
			"%s : index exceeded maximum buffer count.(%d:%d)\n",
			__func__, b->index, ycc_in_buf->buf_cnt);
		return -EINVAL;
	}

	phy_addr = ycc_in_buf->meminfo[b->index]->phy_addr;

	dev_dbg(ipcm->dev, "index = %d, phy_addr = 0x%x\n",
		b->index, phy_addr);

	plane_index = (b->index % 2);

	/* check multi plane use */
	if (b->type == V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE) {
		/* buffer for Y data */
		if (plane_index == 0)
			ycc_in_img->addr_y0 = phy_addr;
		/* buffer for CbCr data */
		else
			ycc_in_img->addr_c0 = phy_addr;
	/* use single plane buffer */
	} else {
		ycc_in_img->addr_y0 = phy_addr;
		ycc_in_img->addr_c0 = phy_addr +
			ycc_in_img->stride_y * ycc_in_img->height;
	}

	ycc_in_img->addr_y1 = ycc_in_img->addr_y0 +
		ycc_in_img->stride_y * (ycc_in_img->height / 2);

	/* temp - progressive, ycc 422 */
	if (ycc_in_img->ycc_type == IPCM_YCC422) {
		ycc_in_img->addr_c1 =
			ycc_in_img->addr_c0 +
				 ycc_in_img->stride_y * (ycc_in_img->height / 2);
	/* temp - progressive, ycc 420 */
	} else {
		ycc_in_img->addr_c1 =
			ycc_in_img->addr_c0 +
				 ycc_in_img->stride_y * (ycc_in_img->height / 4);
	}

	return 0;
}

int drime4_ipcm_fw_set_main_out_addr(struct drime4_ipcm *ipcm,
	struct ipcm_ctx *ctx, struct v4l2_buffer *b)
{
	unsigned int phy_addr;
	struct ipcm_buffer *main_out_buf = NULL;
	struct ipcm_ycc_info *main_out_img = NULL;
	struct ipcm_ycc_addr_info *change_addr_info = NULL;
	unsigned int plane_index = 0;

	if ((ipcm == NULL) || (ctx == NULL)) {
		dev_dbg(ipcm->dev,
			"%s: ipcm/ctx indicates NULL pointer.", __func__);
		return -EINVAL;
	}

	/* set image info according to data path */
	switch (ctx->v4l2_op_mode) {
	case V4L2_CID_DRIME_IPC_LIVEVIEW:
		main_out_img = &ctx->liveview_info.MainOut_Img;
		break;

	case V4L2_CID_DRIME_IPC_RESIZE:
		/* resize with liveview */
		if (ipcm_liveview_operation == IPCM_LIVEVIEW_ON)
			dev_dbg(ipcm->dev, "%s:ERR:ipcm_liveview_on.\n", __func__);
		/* resize only (without liveview) */
		else
			main_out_img = &ctx->resize_info.MainOut_Img;

		break;

	default:
		break;
	}

	main_out_buf = &ctx->main_out_buf;

	if (b->index > main_out_buf->buf_cnt) {
		dev_err(ipcm->dev,
			"%s : index exceeded maximum buffer count.(%d:%d)\n",
			__func__, b->index, main_out_buf->buf_cnt);
		return -EINVAL;
	}

	phy_addr = main_out_buf->meminfo[b->index]->phy_addr;

	dev_dbg(ipcm->dev, "index = %d, phy_addr = 0x%x\n",
		b->index, phy_addr);

	main_out_img->addr_y0 = phy_addr;

	plane_index = (b->index % 2);

	/* check multi plane use */
	if (b->type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) {
		/* buffer for Y data */
		if (plane_index == 0)
			main_out_img->addr_y0 = phy_addr;
		/* buffer for CbCr data */
		else
			main_out_img->addr_c0 = phy_addr;
	/* use single plane buffer */
	} else {
		main_out_img->addr_y0 = phy_addr;
		main_out_img->addr_c0 = phy_addr +
			main_out_img->stride_y * main_out_img->height;
	}

	main_out_img->addr_y1 = main_out_img->addr_y0 +
		main_out_img->stride_y * (main_out_img->height / 2);

	/* temp - progressive, ycc 422 */
	if (main_out_img->ycc_type == IPCM_YCC422) {
		main_out_img->addr_c1 =
			main_out_img->addr_c0 +
				 main_out_img->stride_y * (main_out_img->height / 2);
	/* temp - progressive, ycc 420 */
	} else {
		main_out_img->addr_c1 =
			main_out_img->addr_c0 +
				 main_out_img->stride_y * (main_out_img->height / 4);
	}

	/* if user want to chage output address
		with remaining other parameters */
	if (ctx->v4l2_op_status == IPCM_V4L2_STREAM_ON) {
		change_addr_info = &ctx->change_out_addr_info;
		change_addr_info->out_sel = IPCM_OUT_MAIN;
		change_addr_info->scan_type =
			main_out_img->scan_type;
		change_addr_info->y0 =
			main_out_img->addr_y0;
		change_addr_info->y1 =
			main_out_img->addr_y1;
		change_addr_info->c0 =
			main_out_img->addr_c0;
		change_addr_info->c1 =
			main_out_img->addr_c1;
		change_addr_info->stride_y =
			main_out_img->stride_y;
		change_addr_info->stride_c =
			main_out_img->stride_c;
		/* update output addres */
		d4_ipcm_change_output_addr(change_addr_info);
	}

	return 0;
}

int drime4_ipcm_fw_set_rsz_out_addr(struct drime4_ipcm *ipcm,
	struct ipcm_ctx *ctx, struct v4l2_buffer *b)
{
	unsigned int phy_addr;
	struct ipcm_buffer *rsz_out_buf = NULL;
	struct ipcm_ycc_info *rsz_out_img = NULL;
	struct ipcm_ycc_addr_info *change_addr_info = NULL;
	unsigned int plane_index = 0;

	if ((ipcm == NULL) || (ctx == NULL)) {
		dev_dbg(ipcm->dev,
			"%s: ipcm/ctx indicates NULL pointer.", __func__);
		return -EINVAL;
	}

	/* set image info according to data path */
	switch (ctx->v4l2_op_mode) {
	case V4L2_CID_DRIME_IPC_LIVEVIEW:
		rsz_out_img = &ctx->liveview_info.RSZOut_Img;
		break;

	case V4L2_CID_DRIME_IPC_RESIZE:
		/* resize with liveview */
		if (ipcm_liveview_operation == IPCM_LIVEVIEW_ON)
			rsz_out_img = &ctx->combi_op_info.RSZOut_Img;
		/* resize only (without liveview) */
		else
			rsz_out_img = &ctx->resize_info.RSZOut_Img;

		break;

	default:
		break;
	}

	rsz_out_buf = &ctx->rsz_out_buf;

	if (b->index > rsz_out_buf->buf_cnt) {
		dev_err(ipcm->dev,
			"%s : index exceeded maximum buffer count.(%d:%d)\n",
			__func__, b->index, rsz_out_buf->buf_cnt);
		return -EINVAL;
	}

	phy_addr = rsz_out_buf->meminfo[b->index]->phy_addr;

	dev_dbg(ipcm->dev, "index = %d, phy_addr = 0x%x\n",
		b->index, phy_addr);

	rsz_out_img->addr_y0 = phy_addr;

	plane_index = (b->index % 2);

	/* check multi plane use */
	if (b->type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) {
		/* buffer for Y data */
		if (plane_index == 0)
			rsz_out_img->addr_y0 = phy_addr;
		/* buffer for CbCr data */
		else
			rsz_out_img->addr_c0 = phy_addr;
	/* use single plane buffer */
	} else {
		rsz_out_img->addr_y0 = phy_addr;
		rsz_out_img->addr_c0 = phy_addr +
			rsz_out_img->stride_y * rsz_out_img->height;
	}

	rsz_out_img->addr_y1 = rsz_out_img->addr_y0 +
		rsz_out_img->stride_y * (rsz_out_img->height / 2);

	/* temp - progressive, ycc 422 */
	if (rsz_out_img->ycc_type == IPCM_YCC422) {
		rsz_out_img->addr_c1 =
			rsz_out_img->addr_c0 +
				 rsz_out_img->stride_y * (rsz_out_img->height / 2);
	/* temp - progressive, ycc 420 */
	} else {
		rsz_out_img->addr_c1 =
			rsz_out_img->addr_c0 +
				 rsz_out_img->stride_y * (rsz_out_img->height / 4);
	}

	/* if user want to chage output address
		with remaining other parameters */
	if (ctx->v4l2_op_status == IPCM_V4L2_STREAM_ON) {
		change_addr_info = &ctx->change_out_addr_info;
		change_addr_info->out_sel = IPCM_OUT_RSZ;
		change_addr_info->scan_type =
			rsz_out_img->scan_type;
		change_addr_info->y0 =
			rsz_out_img->addr_y0;
		change_addr_info->y1 =
			rsz_out_img->addr_y1;
		change_addr_info->c0 =
			rsz_out_img->addr_c0;
		change_addr_info->c1 =
			rsz_out_img->addr_c1;
		change_addr_info->stride_y =
			rsz_out_img->stride_y;
		change_addr_info->stride_c =
			rsz_out_img->stride_c;
		/* update output addres */
		d4_ipcm_change_output_addr(change_addr_info);
	}

	return 0;
}

int drime4_ipcm_fw_set_subrsz_out_addr(struct drime4_ipcm *ipcm,
	struct ipcm_ctx *ctx, struct v4l2_buffer *b)
{
	unsigned int phy_addr;
	struct ipcm_buffer *subrsz_out_buf = NULL;
	struct ipcm_ycc_info *subrsz_out_img = NULL;
	struct ipcm_ycc_addr_info *change_addr_info = NULL;
	unsigned int plane_index = 0;

	if ((ipcm == NULL) || (ctx == NULL)) {
		dev_dbg(ipcm->dev,
			"%s: ipcm/ctx indicates NULL pointer.", __func__);
		return -EINVAL;
	}

	/* set image info according to data path */
	switch (ctx->v4l2_op_mode) {
	case V4L2_CID_DRIME_IPC_LIVEVIEW:
		subrsz_out_img = &ctx->liveview_info.SRSZOut_Img;
		break;

	case V4L2_CID_DRIME_IPC_RESIZE:
		/* resize with liveview */
		if (ipcm_liveview_operation == IPCM_LIVEVIEW_ON)
			subrsz_out_img = &ctx->combi_op_info.SRSZOut_Img;
		/* resize only (without liveview) */
		else
			subrsz_out_img = &ctx->resize_info.SRSZOut_Img;

		break;

	default:
		break;
	}

	subrsz_out_buf = &ctx->subrsz_out_buf;

	if (b->index > subrsz_out_buf->buf_cnt) {
		dev_err(ipcm->dev,
			"%s : index exceeded maximum buffer count.(%d:%d)\n",
			__func__, b->index, subrsz_out_buf->buf_cnt);
		return -EINVAL;
	}

	phy_addr = subrsz_out_buf->meminfo[b->index]->phy_addr;

	dev_dbg(ipcm->dev, "index = %d, phy_addr = 0x%x\n",
		b->index, phy_addr);

	subrsz_out_img->addr_y0 = phy_addr;

	plane_index = (b->index % 2);

	/* check multi plane use */
	if (b->type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) {
		/* buffer for Y data */
		if (plane_index == 0)
			subrsz_out_img->addr_y0 = phy_addr;
		/* buffer for CbCr data */
		else
			subrsz_out_img->addr_c0 = phy_addr;
	/* use single plane buffer */
	} else {
		subrsz_out_img->addr_y0 = phy_addr;
		subrsz_out_img->addr_c0 = phy_addr +
			subrsz_out_img->stride_y * subrsz_out_img->height;
	}

	subrsz_out_img->addr_y1 = subrsz_out_img->addr_y0 +
		subrsz_out_img->stride_y * (subrsz_out_img->height / 2);

	/* temp - progressive, ycc 422 */
	if (subrsz_out_img->ycc_type == IPCM_YCC422) {
		subrsz_out_img->addr_c1 =
			subrsz_out_img->addr_c0 +
				 subrsz_out_img->stride_y * (subrsz_out_img->height / 2);
	/* temp - progressive, ycc 420 */
	} else {
		subrsz_out_img->addr_c1 =
			subrsz_out_img->addr_c0 +
				 subrsz_out_img->stride_y * (subrsz_out_img->height / 4);
	}

	/* if user want to chage output address
		with remaining other parameters */
	if (ctx->v4l2_op_status == IPCM_V4L2_STREAM_ON) {
		change_addr_info = &ctx->change_out_addr_info;
		change_addr_info->out_sel = IPCM_OUT_SRSZ;
		change_addr_info->scan_type =
			subrsz_out_img->scan_type;
		change_addr_info->y0 =
			subrsz_out_img->addr_y0;
		change_addr_info->y1 =
			subrsz_out_img->addr_y1;
		change_addr_info->c0 =
			subrsz_out_img->addr_c0;
		change_addr_info->c1 =
			subrsz_out_img->addr_c1;
		change_addr_info->stride_y =
			subrsz_out_img->stride_y;
		change_addr_info->stride_c =
			subrsz_out_img->stride_c;
		/* update output addres */
		d4_ipcm_change_output_addr(change_addr_info);
	}

	return 0;
}

unsigned int drime4_ipcm_fw_uservirt_to_phys(struct drime4_ipcm *ipcm,
	unsigned int virt_addr)
{
	struct mm_struct *mm = current->mm;
	unsigned long offset, phy_addr = 0, curr_pfn;
	struct vm_area_struct *vma = NULL;
	unsigned int ret = 1;

	down_read(&mm->mmap_sem);

	vma = find_vma(mm, virt_addr);
	if (!vma) {
		dev_err(ipcm->dev, "invalid userspace address.\n");
		up_read(&mm->mmap_sem);
		return 0;
	}

	/* for kernel direct-mapped memory. */
	if (virt_addr >= PAGE_OFFSET)
		phy_addr = virt_to_phys((void *)virt_addr);
	/* for memory mapped to dma coherent region. */
	else {
		offset = virt_addr & ~PAGE_MASK;

		/* get current page frame number to virt_addr. */
		ret = follow_pfn(vma, virt_addr, &curr_pfn);
		if (ret) {
			dev_err(ipcm->dev, "invalid userspace address.\n");
			up_read(&mm->mmap_sem);
			return 0;
		}

		phy_addr = (curr_pfn << PAGE_SHIFT) + offset;
	}

	up_read(&mm->mmap_sem);

	return phy_addr;
}

int drime4_ipcm_fw_change_src_addr(struct drime4_ipcm *ipcm,
	struct v4l2_outputparm *output)
{
	struct ipcm_ctx *ctx = ipcm->ipcm_ctx;
	struct ipcm_ycc_info *ycc_input = NULL;

	dev_dbg(ipcm->dev, "%s: start\n", __func__);

	switch (ctx->v4l2_op_mode) {
	case V4L2_CID_DRIME_IPC_LIVEVIEW:
		/* in liveview case,
		 * there is no allocated memory */
		break;

	case V4L2_CID_DRIME_IPC_RESIZE:
		/* resize with liveview */
		if (ipcm_liveview_operation == IPCM_LIVEVIEW_ON)
			ycc_input = &ctx->combi_op_info.Src_Img_Ycc;
		/* resize only (without liveview) */
		else
			ycc_input = &ctx->resize_info.Src_Img_Ycc;

		/* set ycc input address */
		/* use virtual address */
		if (output->extendedmode != V4L2_MEMORY_OVERLAY) {
			if (output->writebuffers == 2) {
				ycc_input->addr_y0 =
					drime4_ipcm_fw_uservirt_to_phys(ipcm, output->reserved[0]);
				ycc_input->addr_c0 =
					drime4_ipcm_fw_uservirt_to_phys(ipcm, output->reserved[1]);
			} else if (output->writebuffers == 4) {
				ycc_input->addr_y0 =
					drime4_ipcm_fw_uservirt_to_phys(ipcm, output->reserved[0]);
				ycc_input->addr_y1 =
					drime4_ipcm_fw_uservirt_to_phys(ipcm, output->reserved[1]);
				ycc_input->addr_c0 =
					drime4_ipcm_fw_uservirt_to_phys(ipcm, output->reserved[2]);
				ycc_input->addr_c1 =
					drime4_ipcm_fw_uservirt_to_phys(ipcm, output->reserved[3]);
			} else
				ycc_input->addr_y0 =
					drime4_ipcm_fw_uservirt_to_phys(ipcm, output->reserved[0]);
		/* use physical address */
		} else {
			if (output->writebuffers == 2) {
				ycc_input->addr_y0 = output->reserved[0];
				ycc_input->addr_c0 = output->reserved[1];
			} else if (output->writebuffers == 4) {
				ycc_input->addr_y0 = output->reserved[0];
				ycc_input->addr_y1 = output->reserved[1];
				ycc_input->addr_c0 = output->reserved[2];
				ycc_input->addr_c1 = output->reserved[3];
			} else
				ycc_input->addr_y0 = output->reserved[0];
		}
		break;
	default:
		break;
	}

	return 0;
}

int drime4_ipcm_fw_change_dst_addr(struct drime4_ipcm *ipcm,
	struct v4l2_captureparm *capture)
{
	struct ipcm_ctx *ctx = ipcm->ipcm_ctx;
	struct ipcm_ycc_info *ycc_output = NULL;
	struct ipcm_ycc_addr_info *change_addr_info = NULL;

	dev_dbg(ipcm->dev, "%s: start\n", __func__);

	change_addr_info = &ctx->change_out_addr_info;

	switch (ctx->v4l2_op_mode) {
	case V4L2_CID_DRIME_IPC_LIVEVIEW:
		/* main out: liveview with no resize */
		if (ctx->curr_buf_type == IPCM_DST_MAIN_BUFFER) {
			ycc_output = &ctx->liveview_info.MainOut_Img;
			change_addr_info->out_sel = IPCM_OUT_MAIN;
		/* resize out */
		} else if (ctx->curr_buf_type == IPCM_DST_RSZ_BUFFER) {
			ycc_output = &ctx->liveview_info.RSZOut_Img;
			change_addr_info->out_sel = IPCM_OUT_RSZ;
		/* sub-resize out */
		} else if (ctx->curr_buf_type == IPCM_DST_SRSZ_BUFFER) {
			ycc_output = &ctx->liveview_info.SRSZOut_Img;
			change_addr_info->out_sel = IPCM_OUT_SRSZ;
		}
		break;

	case V4L2_CID_DRIME_IPC_RESIZE:
		/* resize with liveview */
		if (ipcm_liveview_operation == IPCM_LIVEVIEW_ON) {
			/* Resize Out */
			if (ctx->curr_buf_type == IPCM_DST_RSZ_BUFFER) {
				ycc_output = &ctx->combi_op_info.RSZOut_Img;
				change_addr_info->out_sel = IPCM_OUT_RSZ;
			/* Sub-Resize Out */
			} else if (ctx->curr_buf_type == IPCM_DST_SRSZ_BUFFER) {
				ycc_output = &ctx->combi_op_info.SRSZOut_Img;
				change_addr_info->out_sel = IPCM_OUT_SRSZ;
			}
		/* resize only (without liveview) */
		} else {
			/* Main Out */
			if (ctx->curr_buf_type == IPCM_DST_MAIN_BUFFER) {
				ycc_output = &ctx->resize_info.MainOut_Img;
				change_addr_info->out_sel = IPCM_OUT_MAIN;
			/* Resize Out */
			} else if (ctx->curr_buf_type == IPCM_DST_RSZ_BUFFER) {
				ycc_output = &ctx->resize_info.RSZOut_Img;
				change_addr_info->out_sel = IPCM_OUT_RSZ;
			/* Sub-Resize Out */
			} else if (ctx->curr_buf_type == IPCM_DST_SRSZ_BUFFER) {
				ycc_output = &ctx->resize_info.SRSZOut_Img;
				change_addr_info->out_sel = IPCM_OUT_SRSZ;
			}
		}
		break;

	default:
		break;
	}

	if (ycc_output == NULL) {
		dev_err(ipcm->dev, "%s:wrong buffer type.\n", __func__);
		return -EINVAL;
	}

	/* set ycc output address */
	/* use virtual address */
	if (capture->extendedmode != V4L2_MEMORY_OVERLAY) {
		if (capture->readbuffers == 2) {
			ycc_output->addr_y0 =
				drime4_ipcm_fw_uservirt_to_phys(ipcm, capture->reserved[0]);
			ycc_output->addr_c0 =
				drime4_ipcm_fw_uservirt_to_phys(ipcm, capture->reserved[1]);
		} else if (capture->readbuffers == 4) {
			ycc_output->addr_y0 =
				drime4_ipcm_fw_uservirt_to_phys(ipcm, capture->reserved[0]);
			ycc_output->addr_y1 =
				drime4_ipcm_fw_uservirt_to_phys(ipcm, capture->reserved[1]);
			ycc_output->addr_c0 =
				drime4_ipcm_fw_uservirt_to_phys(ipcm, capture->reserved[2]);
			ycc_output->addr_c1 =
				drime4_ipcm_fw_uservirt_to_phys(ipcm, capture->reserved[3]);
		} else
			ycc_output->addr_y0 =
				drime4_ipcm_fw_uservirt_to_phys(ipcm, capture->reserved[0]);
	/* use physical address */
	} else {
		if (capture->readbuffers == 2) {
			ycc_output->addr_y0 = capture->reserved[0];
			ycc_output->addr_c0 = capture->reserved[1];
		} else if (capture->readbuffers == 4) {
			ycc_output->addr_y0 = capture->reserved[0];
			ycc_output->addr_y1 = capture->reserved[1];
			ycc_output->addr_c0 = capture->reserved[2];
			ycc_output->addr_c1 = capture->reserved[3];
		} else
			ycc_output->addr_y0 = capture->reserved[0];
	}

	/* if user want to chage output address
		with remaining other parameters */
	if (ctx->v4l2_op_status == IPCM_V4L2_STREAM_ON) {
		change_addr_info->scan_type =
			ycc_output->scan_type;
		change_addr_info->y0 =
			ycc_output->addr_y0;
		change_addr_info->y1 =
			ycc_output->addr_y1;
		change_addr_info->c0 =
			ycc_output->addr_c0;
		change_addr_info->c1 =
			ycc_output->addr_c1;
		change_addr_info->stride_y =
			ycc_output->stride_y;
		change_addr_info->stride_c =
			ycc_output->stride_c;
		/* update output addres */
		d4_ipcm_change_output_addr(change_addr_info);
	}

	return 0;
}


int drime4_ipcm_fw_src_enqueue_buffer(struct drime4_ipcm *ipcm,
	struct ipcm_ctx *ctx, struct v4l2_buffer *b)
{
	struct ipcm_buffer *buffer = NULL;

	if ((ipcm == NULL) || (ctx == NULL) || (b == NULL)) {
		dev_dbg(ipcm->dev,
			"%s: ipcm/ctx/b indicates NULL pointer.", __func__);
		return -EINVAL;
	}

	switch (ctx->v4l2_op_mode) {

	case V4L2_CID_DRIME_IPC_LIVEVIEW:
		break;
	case V4L2_CID_DRIME_IPC_RESIZE:
		buffer = &ctx->ycc_in_buf;
	default:
		break;
	}

	if (buffer == NULL) {
		dev_dbg(ipcm->dev,
			"%s: buffer indicates NULL pointer.", __func__);
		return -EINVAL;
	}

	if (b->index >= buffer->buf_cnt) {
		dev_err(ipcm->dev,
			"index is bigger then buffer count.\n");

		dev_err(ipcm->dev, "%s:index=%d, buffer->buf_cnt=%d\n",
			__func__, b->index, buffer->buf_cnt);

		return -EINVAL;
	}

	if (b->memory == V4L2_MEMORY_USERPTR) {
		/* set memory allocated by user. */
		buffer->meminfo[b->index]->phy_addr =
				drime4_ipcm_fw_uservirt_to_phys(ipcm,
						(unsigned int)b->m.userptr);

		dev_dbg(ipcm->dev, "%s : index: %d, phy_addr = 0x%x, virt_addr = 0x%x\n",
			__func__, b->index, buffer->meminfo[b->index]->phy_addr,
			(unsigned int)b->m.userptr);
	}

	/* This is not a v4l2 memory overlay scheme. */
	/* Only using video playback scenario
		( for sharing mfc output buffer and ipcm source buffer */
	if (b->memory == V4L2_MEMORY_OVERLAY) {
		buffer->meminfo[b->index]->phy_addr =
						(unsigned int)b->m.userptr;

		dev_dbg(ipcm->dev, "%s : phy_addr = 0x%x", __func__,
			(unsigned int)b->m.userptr);
	}

	return 0;
}

int drime4_ipcm_fw_src_dequeue_buffer(struct drime4_ipcm *ipcm,
	struct ipcm_ctx *ctx, struct v4l2_buffer *b)
{
	struct ipcm_buffer *buffer = NULL;

	if ((ipcm == NULL) || (ctx == NULL) || (b == NULL)) {
		dev_dbg(ipcm->dev,
			"%s: ipcm/ctx/b indicates NULL pointer.", __func__);
		return -EINVAL;
	}

	switch (ctx->v4l2_op_mode) {

	case V4L2_CID_DRIME_IPC_LIVEVIEW:
		break;
	case V4L2_CID_DRIME_IPC_RESIZE:
		buffer = &ctx->ycc_in_buf;
	default:
		break;
	}

	/* temp - just check buffer index */
	dev_dbg(ipcm->dev, "%s: index = %d, buffer->buf_cnt=%d\n",
		__func__, b->index, buffer->buf_cnt);

	return 0;
}

int drime4_ipcm_fw_dst_enqueue_buffer(struct drime4_ipcm *ipcm,
	struct ipcm_ctx *ctx, struct v4l2_buffer *b)
{
	struct ipcm_buffer *buffer = NULL;

	if ((ipcm == NULL) || (ctx == NULL) || (b == NULL)) {
		dev_dbg(ipcm->dev,
			"%s: ipcm/ctx/b indicates NULL pointer.", __func__);
		return -EINVAL;
	}

	switch (ctx->curr_buf_type) {

	case IPCM_DST_MAIN_BUFFER:
		buffer = &ctx->main_out_buf;
		break;
	case IPCM_DST_RSZ_BUFFER:
		buffer = &ctx->rsz_out_buf;
		break;
	case IPCM_DST_SRSZ_BUFFER:
		buffer = &ctx->subrsz_out_buf;
		break;
	default:
		break;
	}

	if (buffer == NULL) {
		dev_dbg(ipcm->dev,
			"%s: buffer indicates NULL pointer.", __func__);
		return -EINVAL;
	}

	if (b->index >= buffer->buf_cnt) {
		dev_err(ipcm->dev,
			"index is bigger then buffer count.\n");

		dev_err(ipcm->dev, "%s:index=%d, buffer->buf_cnt=%d\n",
			__func__, b->index, buffer->buf_cnt);

		return -EINVAL;
	}

	if (b->memory == V4L2_MEMORY_USERPTR) {
		/* set memory allocated by user. */
		buffer->meminfo[b->index]->phy_addr =
				drime4_ipcm_fw_uservirt_to_phys(ipcm,
						(unsigned int)b->m.userptr);

		dev_dbg(ipcm->dev, "%s : index = %d, phy_addr = 0x%x, virt_addr = 0x%x\n",
			__func__, b->index, buffer->meminfo[b->index]->phy_addr,
			(unsigned int)b->m.userptr);
	}

	return 0;
}

int drime4_ipcm_fw_dst_dequeue_buffer(struct drime4_ipcm *ipcm,
	struct ipcm_ctx *ctx, struct v4l2_buffer *b)
{
	struct ipcm_buffer *buffer = NULL;

	if ((ipcm == NULL) || (ctx == NULL) || (b == NULL)) {
		dev_dbg(ipcm->dev,
			"%s: ipcm/ctx/b indicates NULL pointer.", __func__);
		return -EINVAL;
	}

	switch (ctx->curr_buf_type) {

	case IPCM_DST_MAIN_BUFFER:
		buffer = &ctx->main_out_buf;
		break;
	case IPCM_DST_RSZ_BUFFER:
		buffer = &ctx->rsz_out_buf;
		break;
	case IPCM_DST_SRSZ_BUFFER:
		buffer = &ctx->subrsz_out_buf;
		break;
	default:
		break;
	}

	/* temp - just check buffer index */
	dev_dbg(ipcm->dev, "%s:index=%d, buffer->buf_cnt=%d\n",
		__func__, b->index, buffer->buf_cnt);

	return 0;
}

int drime4_ipcm_fw_set_buffer_to_ctx(struct drime4_ipcm *ipcm,
	struct ipcm_ctx *ctx, struct v4l2_buffer *b)
{
	if ((ipcm == NULL) || (ctx == NULL)) {
		dev_dbg(ipcm->dev,
			"%s: ipcm/ctx indicates NULL pointer.", __func__);
		return -EINVAL;
	}

	if (ctx->curr_buf_type == IPCM_SRC_BUFFER) {
		switch (ctx->v4l2_op_mode) {
		case V4L2_CID_DRIME_IPC_LIVEVIEW:
			break;
		case V4L2_CID_DRIME_IPC_RESIZE:
			drime4_ipcm_fw_set_ycc_in_addr(ipcm, ctx, b);
			break;
		default:
			return -1;
		}

	} else {

		/* No Resize Image */
		if (ctx->curr_buf_type == IPCM_DST_MAIN_BUFFER)
			drime4_ipcm_fw_set_main_out_addr(ipcm, ctx, b);
		/* Main Resize Image */
		if (ctx->curr_buf_type == IPCM_DST_RSZ_BUFFER)
			drime4_ipcm_fw_set_rsz_out_addr(ipcm, ctx, b);
		/* Sub Resize Image */
		if (ctx->curr_buf_type == IPCM_DST_SRSZ_BUFFER)
			drime4_ipcm_fw_set_subrsz_out_addr(ipcm, ctx, b);
	}

	return 0;
}

int drime4_ipcm_fw_free_buffer(struct drime4_ipcm *ipcm,
	struct v4l2_requestbuffers *b)
{
	int ret = 0;
	unsigned int virt_addr = 0;
	unsigned int phy_addr = 0;

	if (b->memory != V4L2_MEMORY_OVERLAY) {
		/* get virtual address
			virtual address was stored in b->reserved[0] */
		virt_addr = b->reserved[0];
		/* get physical address from virtual address */
		phy_addr =
			drime4_ipcm_fw_uservirt_to_phys(ipcm, virt_addr);
	/* This is not a v4l2 memory overlay scheme.
		in case of using physical address directly in user space. */
	} else {
		/* get physical address directly from user */
		phy_addr = b->reserved[0];
	}

	/* free buffer */
	ret = cma_free(phy_addr);
	dev_dbg(ipcm->dev, "%s :phy_addr = 0x%x, virt_addr = 0x%x\n",
			__func__, phy_addr, virt_addr);

	return ret;
}

void drime4_ipcm_fw_start_liveview(struct drime4_ipcm *ipcm,
	struct ipcm_ctx *ctx)
{
	int ret = 0;

	struct ipcm_liveview_info *liveview_param = NULL;
	struct ipcm_wb wb_info;

	if ((ipcm == NULL) || (ctx == NULL)) {
		dev_dbg(ipcm->dev,
			"%s: ipcm/ctx indicates NULL pointer.", __func__);
		return;
	}

	liveview_param = &ctx->liveview_info;

	/* WDMA0(Main Out) interrupt enable */
	if (liveview_param->MainOut_SW) {
		d4_ipcm_sub_int_enable(IPCM_INTR_BUSEND_WDMA0,
				drime4_ipcm_fw_wdma0_intr_callback_func);
		d4_ipcm_sub_int_clear(IPCM_INTR_BUSEND_WDMA0);
		ctx->interrupt_wdma0 = IPCM_INTR_BUSEND_WDMA0;
		dev_dbg(ipcm->dev, "WDMA0(Main Out) interrupt enable\n");
	}
	/* WDMA1(MRSZ Out) interrupt enable */
	if (liveview_param->RSZ_SW) {
		d4_ipcm_sub_int_enable(IPCM_INTR_BUSEND_WDMA1,
				drime4_ipcm_fw_wdma1_intr_callback_func);
		d4_ipcm_sub_int_clear(IPCM_INTR_BUSEND_WDMA1);
		ctx->interrupt_wdma1 = IPCM_INTR_BUSEND_WDMA1;
		dev_dbg(ipcm->dev, "WDMA1(Main Resize Out) interrupt enable\n");
	}
	/* WDMA2(SRSZ Out) interrupt enable */
	if (liveview_param->SRSZ_SW) {
		d4_ipcm_sub_int_enable(IPCM_INTR_BUSEND_WDMA2,
				drime4_ipcm_fw_wdma2_intr_callback_func);
		d4_ipcm_sub_int_clear(IPCM_INTR_BUSEND_WDMA2);
		ctx->interrupt_wdma2 = IPCM_INTR_BUSEND_WDMA2;
		dev_dbg(ipcm->dev, "WDMA2(Sub Resize1 Out) interrupt enable\n");
	}

	/* apply wb gain - temp */
	wb_info.sel = IPCM_GAIN;
	wb_info.rr = 500;
	wb_info.gr = 170;
	wb_info.gb = 170;
	wb_info.bb = 400;
	d4_ipcm_wb_param(&wb_info);

	/* start liveview */
	ret = d4_ipcm_liveview_start(liveview_param);
	dev_dbg(ipcm->dev, "liveview start return val: %d\n", ret);

	if (IPCM_OP_ERROR_NONE != d4_ipcm_set_dma_thread_id(IPCM_DMA_ID_W0, 3))
		dev_err(ipcm->dev, "error ! d4_ipcm_set_dma_thread_id\n");

	if (IPCM_OP_ERROR_NONE != d4_ipcm_set_dma_thread_id(IPCM_DMA_ID_W1, 3))
		dev_err(ipcm->dev, "error ! d4_ipcm_set_dma_thread_id\n");

	if (IPCM_OP_ERROR_NONE != d4_ipcm_set_dma_thread_id(IPCM_DMA_ID_W2, 3))
		dev_err(ipcm->dev, "error ! d4_ipcm_set_dma_thread_id\n");

	if (IPCM_OP_ERROR_NONE != d4_ipcm_set_dma_waittime(IPCM_DMA_ID_R, 0x0030))
		dev_err(ipcm->dev, "error ! d4_ipcm_set_dma_waittime\n");

	if (IPCM_OP_ERROR_NONE != d4_ipcm_set_dma_waittime(IPCM_DMA_ID_W0, 0x0000))
		dev_err(ipcm->dev, "error ! d4_ipcm_set_dma_waittime\n");

	if (IPCM_OP_ERROR_NONE != d4_ipcm_set_dma_waittime(IPCM_DMA_ID_W1, 0x0040))
		dev_err(ipcm->dev, "error ! d4_ipcm_set_dma_waittime\n");

	if (IPCM_OP_ERROR_NONE != d4_ipcm_set_dma_waittime(IPCM_DMA_ID_W2, 0x0030))
		dev_err(ipcm->dev, "error ! d4_ipcm_set_dma_waittime\n");
}

void drime4_ipcm_fw_stop_liveview(struct drime4_ipcm *ipcm,
	struct ipcm_ctx *ctx)
{
	struct ipcm_liveview_info *liveview_param = NULL;

	if ((ipcm == NULL) || (ctx == NULL)) {
		dev_dbg(ipcm->dev,
			"%s: ipcm/ctx indicates NULL pointer.", __func__);
		return -EINVAL;
	}

	liveview_param = &ctx->liveview_info;

	/* WDMA0(Main Out) interrupt disable */
	if (liveview_param->MainOut_SW) {
		d4_ipcm_sub_int_disable(IPCM_INTR_BUSEND_WDMA0);
		dev_dbg(ipcm->dev, "WDMA0(Main Out) interrupt disable\n");
	}
	/* WDMA1(MRSZ Out) interrupt disable */
	if (liveview_param->RSZ_SW) {
		d4_ipcm_sub_int_disable(IPCM_INTR_BUSEND_WDMA1);
		dev_dbg(ipcm->dev, "WDMA1(Main Resize Out) interrupt disable\n");
	}
	/* WDMA2(SRSZ Out) interrupt disable */
	if (liveview_param->SRSZ_SW) {
		d4_ipcm_sub_int_disable(IPCM_INTR_BUSEND_WDMA2);
		dev_dbg(ipcm->dev, "WDMA2(Sub Resize1 Out) interrupt disable\n");
	}

	/* stop liveview  */
	d4_ipcm_liveview_stop();
}

void drime4_ipcm_fw_start_resize(struct drime4_ipcm *ipcm,
	struct ipcm_ctx *ctx)
{
	struct ipcm_ycc_combi_op_info *combi_op_param = NULL;
	struct ipcm_ycc_resize_info *resize_param = NULL;

	if ((ipcm == NULL) || (ctx == NULL)) {
		dev_dbg(ipcm->dev,
			"%s: ipcm/ctx indicates NULL pointer.", __func__);
		return -EINVAL;
	}

	/* resize with liveview */
	if (ipcm_liveview_operation == IPCM_LIVEVIEW_ON) {
		combi_op_param = &ctx->combi_op_info;

		/* print debugging msg */
		drime4_ipcm_fw_print_combi_op_parameter(combi_op_param);

		/* WDMA1(MRSZ Out) interrupt enable */
		if (combi_op_param->RSZ_SW == IPCM_SW_ON) {
			d4_ipcm_sub_int_enable(IPCM_INTR_BUSEND_WDMA1,
					drime4_ipcm_fw_wdma1_intr_callback_func);
			d4_ipcm_sub_int_clear(IPCM_INTR_BUSEND_WDMA1);
			ctx->interrupt_wdma1 = IPCM_INTR_BUSEND_WDMA1;
			dev_dbg(ipcm->dev, "WDMA1(Main Resize Out) interrupt enable\n");
		}
		/* WDMA2(SRSZ Out) interrupt enable */
		if (combi_op_param->SRSZ_SW == IPCM_SW_ON) {
			d4_ipcm_sub_int_enable(IPCM_INTR_BUSEND_WDMA2,
					drime4_ipcm_fw_wdma2_intr_callback_func);
			d4_ipcm_sub_int_clear(IPCM_INTR_BUSEND_WDMA2);
			ctx->interrupt_wdma2 = IPCM_INTR_BUSEND_WDMA2;
			dev_dbg(ipcm->dev, "WDMA2(Sub Resize1 Out) interrupt enable\n");
		}

		/* set combi resize operation parameters */
		d4_ipcm_ycc_combi_set(combi_op_param);

		/* start resize */
		d4_ipcm_ycc_combi_do_1frm();
	/* resize only (without liveview) */
	} else {
		resize_param = &ctx->resize_info;

		/* print debugging msg */
		drime4_ipcm_fw_print_resize_parameter(resize_param);

		/* WDMA0(Main Out) interrupt enable */
		if (resize_param->MainOut_SW == IPCM_SW_ON) {
			d4_ipcm_sub_int_enable(IPCM_INTR_BUSEND_WDMA0,
					drime4_ipcm_fw_wdma0_intr_callback_func);
			d4_ipcm_sub_int_clear(IPCM_INTR_BUSEND_WDMA0);
			ctx->interrupt_wdma0 = IPCM_INTR_BUSEND_WDMA0;
			dev_dbg(ipcm->dev, "WDMA0(Main Out) interrupt enable\n");
		}
		/* WDMA1(MRSZ Out) interrupt enable */
		if (resize_param->RSZ_SW == IPCM_SW_ON) {
			d4_ipcm_sub_int_enable(IPCM_INTR_BUSEND_WDMA1,
					drime4_ipcm_fw_wdma1_intr_callback_func);
			d4_ipcm_sub_int_clear(IPCM_INTR_BUSEND_WDMA1);
			ctx->interrupt_wdma1 = IPCM_INTR_BUSEND_WDMA1;
			dev_dbg(ipcm->dev, "WDMA1(Main Resize Out) interrupt enable\n");
		}
		/* WDMA2(SRSZ Out) interrupt enable */
		if (resize_param->SRSZ_SW == IPCM_SW_ON) {
			d4_ipcm_sub_int_enable(IPCM_INTR_BUSEND_WDMA2,
					drime4_ipcm_fw_wdma2_intr_callback_func);
			d4_ipcm_sub_int_clear(IPCM_INTR_BUSEND_WDMA2);
			ctx->interrupt_wdma2 = IPCM_INTR_BUSEND_WDMA2;
			dev_dbg(ipcm->dev, "WDMA2(Sub Resize1 Out) interrupt enable\n");
		}
		/* set & start resize operation */
		d4_ipcm_ycc_resize(resize_param);
	}

	if (IPCM_OP_ERROR_NONE != d4_ipcm_enable_dma_moa(IPCM_DMA_ID_R, 0x1))
		dev_err(ipcm->dev, "error ! d4_ipcm_enable_dma_moa\n");

	if (IPCM_OP_ERROR_NONE != d4_ipcm_enable_dma_moa(IPCM_DMA_ID_W1, 0x1))
		dev_err(ipcm->dev, "error ! d4_ipcm_enable_dma_moa\n");

	if (IPCM_OP_ERROR_NONE != d4_ipcm_enable_dma_moa(IPCM_DMA_ID_W2, 0x1))
		dev_err(ipcm->dev, "error ! d4_ipcm_enable_dma_moa\n");
}

void drime4_ipcm_fw_stop_resize(struct drime4_ipcm *ipcm,
	struct ipcm_ctx *ctx)
{
	struct ipcm_ycc_combi_op_info *combi_op_param = NULL;
	struct ipcm_ycc_resize_info *resize_param = NULL;

	if ((ipcm == NULL) || (ctx == NULL)) {
		dev_dbg(ipcm->dev,
			"%s: ipcm/ctx indicates NULL pointer.", __func__);
		return -EINVAL;
	}

	/* resize with liveview */
	if (ipcm_liveview_operation == IPCM_LIVEVIEW_ON) {
		combi_op_param = &ctx->combi_op_info;

		/* diable interrupt */
		/* WDMA1(MRSZ Out) interrupt disable */
		if (combi_op_param->RSZ_SW == IPCM_SW_ON) {
			d4_ipcm_sub_int_disable(IPCM_INTR_BUSEND_WDMA1);
			dev_dbg(ipcm->dev, "WDMA1(Main Resize Out) interrupt disable\n");
		}
		/* WDMA2(SRSZ Out) interrupt disable */
		if (combi_op_param->SRSZ_SW == IPCM_SW_ON) {
			d4_ipcm_sub_int_disable(IPCM_INTR_BUSEND_WDMA2);
			dev_dbg(ipcm->dev, "WDMA2(Sub Resize1 Out) interrupt disable\n");
		}
	/* resize only (without liveview) */
	} else {
		resize_param = &ctx->resize_info;

		/* disable interrupt */
		/* WDMA0(Main Out) interrupt disable */
		if (resize_param->MainOut_SW == IPCM_SW_ON) {
			d4_ipcm_sub_int_disable(IPCM_INTR_BUSEND_WDMA0);
			dev_dbg(ipcm->dev, "WDMA0(Main Out) interrupt disable\n");
		}
		/* WDMA1(MRSZ Out) interrupt disable */
		if (resize_param->RSZ_SW == IPCM_SW_ON) {
			d4_ipcm_sub_int_disable(IPCM_INTR_BUSEND_WDMA1);
			dev_dbg(ipcm->dev, "WDMA1(Main Resize Out) interrupt disable\n");
		}
		/* WDMA2(SRSZ Out) interrupt disable */
		if (resize_param->SRSZ_SW == IPCM_SW_ON) {
			d4_ipcm_sub_int_disable(IPCM_INTR_BUSEND_WDMA2);
			dev_dbg(ipcm->dev, "WDMA2(Sub Resize1 Out) interrupt disable\n");
		}
	}
}

void drime4_ipcm_fw_start_md(struct drime4_ipcm *ipcm,
	struct ipcm_ctx *ctx)
{
	struct ipcm_md_info *md_op_param = NULL;

	if ((ipcm == NULL) || (ctx == NULL)) {
		dev_dbg(ipcm->dev,
			"%s: ipcm/ctx indicates NULL pointer.", __func__);
		return -EINVAL;
	}

	md_op_param = &ctx->md_info;

	/* initialize MD interrupts */
	d4_ipcm_md_int_init();

	/* GMD(Golobal Motion Detector) interrupt enable */
	d4_ipcm_md_int_enable(IPCM_MD_INTR_GMV_END,
		drime4_ipcm_fw_md_gmd_intr_callback_func);
	/* RMD(Rotation Motion Detector) interrupt enable */
	d4_ipcm_md_int_enable(IPCM_MD_INTR_RMV_END,
		drime4_ipcm_fw_md_rmd_intr_callback_func);

	/* start Motion Detection */
	d4_ipcm_md_start(md_op_param);

	/* disable interrupts */
	d4_ipcm_md_int_disable(IPCM_MD_INTR_GMV_END);
	d4_ipcm_md_int_disable(IPCM_MD_INTR_RMV_END);
}

long drime4_ipcm_fw_sub_module_ctrl(
	struct drime4_ipcm *ipcm, struct v4l2_d4_private_control *ctrl)
{
	switch (ctrl->id) {

	case V4L2_CID_IPCM_WB:
	{
		struct ipcm_wb *wb_info =
			(struct ipcm_wb *)ctrl->params;
		ctrl->error_idx =
			d4_ipcm_wb_param(wb_info);
		break;
	}
	case V4L2_CID_IPCM_CFAI:
	{
		struct ipcm_cfa *cfa_info =
			(struct ipcm_cfa *)ctrl->params;
		ctrl->error_idx =
			d4_ipcm_cfai_param(cfa_info);
		break;
	}
	case V4L2_CID_IPCM_YCC2RGB:
	{
		struct ipcm_ycc2rgb *ycc2rgb_info =
			(struct ipcm_ycc2rgb *)ctrl->params;
		ctrl->error_idx =
			d4_ipcm_ycc2rgb_param(ycc2rgb_info);
		break;
	}
	case V4L2_CID_IPCM_COLOR_CORRECTION:
	{
		struct ipcm_color_correction *color_correction_info =
			(struct ipcm_color_correction *)ctrl->params;
		ctrl->error_idx =
			d4_ipcm_color_correction_param(color_correction_info);
		break;
	}
	case V4L2_CID_IPCM_GAMUT_MAPPING:
	{
		struct ipcm_gamut_mapping *gamut_mapping_info =
			(struct ipcm_gamut_mapping *)ctrl->params;
		ctrl->error_idx =
			d4_ipcm_gamut_mapping_param(gamut_mapping_info);
		break;
	}
	case V4L2_CID_IPCM_RGB2YCC:
	{
		struct ipcm_rgb2ycc *rgb2ycc_info =
			(struct ipcm_rgb2ycc *)ctrl->params;
		ctrl->error_idx =
			d4_ipcm_rgb2ycc_param(rgb2ycc_info);
		break;
	}
	case V4L2_CID_IPCM_YCNR_BOOSTER:
	{
		struct ipcm_ycnr_booster *ycnr_booster_info =
			(struct ipcm_ycnr_booster *)ctrl->params;
		ctrl->error_idx =
			d4_ipcm_ycnr_booster_param(ycnr_booster_info);
		break;
	}
	case V4L2_CID_IPCM_YCNR:
	{
		struct ipcm_ycnr *ycnr_info =
			(struct ipcm_ycnr *)ctrl->params;
		ctrl->error_idx =
			d4_ipcm_ycnr_param(ycnr_info);
		break;
	}
	case V4L2_CID_IPCM_EDGE_EXTRACTION:
	{
		struct ipcm_edge_extraction *edge_extraction_info =
			(struct ipcm_edge_extraction *)ctrl->params;
		ctrl->error_idx =
			d4_ipcm_edge_extraction_param(edge_extraction_info);
		break;
	}
	case V4L2_CID_IPCM_EDGE_ENHANCEMENT:
	{
		struct ipcm_edge_enhancement *edge_enhancement_info =
			(struct ipcm_edge_enhancement *)ctrl->params;
		ctrl->error_idx =
			d4_ipcm_edge_enhancement_param(edge_enhancement_info);
		break;
	}
	case V4L2_CID_IPCM_CHM_EDGE:
	{
		struct ipcm_chm_edge *chm_edge_info =
			(struct ipcm_chm_edge *)ctrl->params;
		ctrl->error_idx =
			d4_ipcm_chm_edge_param(chm_edge_info);
		break;
	}
	case V4L2_CID_IPCM_SATURATION_CORRECTION:
	{
		struct ipcm_saturation_correction *saturation_correction_info =
			(struct ipcm_saturation_correction *)ctrl->params;
		ctrl->error_idx =
			d4_ipcm_saturation_correction_param(saturation_correction_info);
		break;
	}
	case V4L2_CID_IPCM_PCSE_SP:
	{
		struct ipcm_pcse_sp_rage *pcse_sp_info =
			(struct ipcm_pcse_sp_rage *)ctrl->params;
		ctrl->error_idx =
			d4_ipcm_pcse_sp_param(pcse_sp_info);
		break;
	}
	case V4L2_CID_IPCM_PCSE_GENERAL_GAIN:
	{
		struct ipcm_pcse_general_gain *pcse_general_gain_info =
			(struct ipcm_pcse_general_gain *)ctrl->params;
		ctrl->error_idx =
			d4_ipcm_pcse_general_gain_param(pcse_general_gain_info);
		break;
	}
	case V4L2_CID_IPCM_PCSE_GRAY_TH:
	{
		struct ipcm_pcse_gray_th *pcse_gray_th_info =
			(struct ipcm_pcse_gray_th *)ctrl->params;
		ctrl->error_idx =
			d4_ipcm_pcse_gray_th_param(pcse_gray_th_info);
		break;
	}
	case V4L2_CID_IPCM_PCSE_PRIMARY_GAIN:
	{
		struct ipcm_pcse_primary_gain *pcse_primary_gain_info =
			(struct ipcm_pcse_primary_gain *)ctrl->params;
		ctrl->error_idx =
			d4_ipcm_pcse_primary_gain_param(pcse_primary_gain_info);
		break;
	}
	case V4L2_CID_IPCM_CHM_CONVERTER:
	{
		struct ipcm_chm_converter *chm_converter_info =
			(struct ipcm_chm_converter *)ctrl->params;
		ctrl->error_idx =
			d4_ipcm_chm_converter_param(chm_converter_info);
		break;
	}
	case V4L2_CID_IPCM_MONOCHROME:
	{
		struct ipcm_monochrome *monochrome_info =
			(struct ipcm_monochrome *)ctrl->params;
		ctrl->error_idx =
			d4_ipcm_monochrome_param(monochrome_info);
		break;
	}
	case V4L2_CID_IPCM_YCNR_LUT:
	{
		d4_ipcm_ycnr_lut((unsigned char *)ctrl->params);
		break;
	}
	case V4L2_CID_IPCM_GAMUT_MAPPING_SET:
	{
		d4_ipcm_gamut_mapping_set(ctrl->value[0]);
		break;
	}
	case V4L2_CID_IPCM_MONOCHROME_SET:
	{
		d4_ipcm_monochrome_set(ctrl->value[0]);
		break;
	}
	case V4L2_CID_IPCM_EDGE_LUT:
	{
		d4_ipcm_edge_lut((unsigned char *)ctrl->params);
		break;
	}
	case V4L2_CID_IPCM_YLUT:
	{
		d4_ipcm_ylut((unsigned char *)ctrl->params);
		break;
	}
	case V4L2_CID_IPCM_CHROMA_LUT:
	{
		d4_ipcm_chroma_lut(ctrl->value[0],
			(unsigned char *)ctrl->params);
		break;
	}
	case V4L2_CID_IPCM_TONE_LUT:
	{
		d4_ipcm_tone_lut((unsigned char *)ctrl->params);
		break;
	}
	default:
	{
		dev_dbg(ipcm_dev, "check sub-module control ID.\n");
		break;
	}
	} /* switch */

	/* check error */
	if (ctrl->error_idx != IPCM_OK)
		return -EINVAL;

	return 0;
}

void drime4_ipcm_fw_print_resize_parameter(
	struct ipcm_ycc_resize_info *resize_param)
{
	/* source */
	dev_dbg(ipcm_dev, "resize_param->Src_Img_Ycc.width = %d\n",
		resize_param->Src_Img_Ycc.width);
	dev_dbg(ipcm_dev, "resize_param->Src_Img_Ycc.height = %d\n",
		resize_param->Src_Img_Ycc.height);
	dev_dbg(ipcm_dev, "resize_param->Src_Img_Ycc.stride_y = %d\n",
		resize_param->Src_Img_Ycc.stride_y);
	dev_dbg(ipcm_dev, "resize_param->Src_Img_Ycc.stride_c = %d\n",
		resize_param->Src_Img_Ycc.stride_c);
	dev_dbg(ipcm_dev, "resize_param->Src_Img_Ycc.ycc_type = %d\n",
		resize_param->Src_Img_Ycc.ycc_type);
	dev_dbg(ipcm_dev, "resize_param->Src_Img_Ycc.ss_mode = 0x%x\n",
		resize_param->Src_Img_Ycc.ss_mode);
	dev_dbg(ipcm_dev, "resize_param->Src_Img_Ycc.scan_type = 0x%x\n",
		resize_param->Src_Img_Ycc.scan_type);
	dev_dbg(ipcm_dev, "resize_param->Src_Img_Ycc.addr_y0 = 0x%x\n",
		resize_param->Src_Img_Ycc.addr_y0);
	dev_dbg(ipcm_dev, "resize_param->Src_Img_Ycc.addr_y1 = 0x%x\n",
		resize_param->Src_Img_Ycc.addr_y1);
	dev_dbg(ipcm_dev, "resize_param->Src_Img_Ycc.addr_c0 = 0x%x\n",
		resize_param->Src_Img_Ycc.addr_c0);
	dev_dbg(ipcm_dev, "resize_param->Src_Img_Ycc.addr_c1 = 0x%x\n",
		resize_param->Src_Img_Ycc.addr_c1);
	dev_dbg(ipcm_dev, "resize_param->Src_Img_Ycc.pix_offset_x = %d\n",
		resize_param->Src_Img_Ycc.pix_offset_x);
	dev_dbg(ipcm_dev, "resize_param->Src_Img_Ycc.pix_offset_y = %d\n",
		resize_param->Src_Img_Ycc.pix_offset_y);
	dev_dbg(ipcm_dev, "resize_param->Src_Img_Ycc.reverse_CbCr = %d\n",
		resize_param->Src_Img_Ycc.reverse_CbCr);
	dev_dbg(ipcm_dev, "resize_param->Src_Img_Ycc.NLC_sw = %d\n",
		resize_param->Src_Img_Ycc.NLC_sw);
	dev_dbg(ipcm_dev, "resize_param->Src_Img_Ycc.NLC_rc = %d\n\n",
		resize_param->Src_Img_Ycc.NLC_rc);

	/* main out */
	dev_dbg(ipcm_dev, "resize_param->MainOut_Img.width = %d\n",
		resize_param->MainOut_Img.width);
	dev_dbg(ipcm_dev, "resize_param->MainOut_Img.height = %d\n",
		resize_param->MainOut_Img.height);
	dev_dbg(ipcm_dev, "resize_param->MainOut_Img.stride_y = %d\n",
		resize_param->MainOut_Img.stride_y);
	dev_dbg(ipcm_dev, "resize_param->MainOut_Img.stride_c = %d\n",
		resize_param->MainOut_Img.stride_c);
	dev_dbg(ipcm_dev, "resize_param->MainOut_Img.ycc_type = %d\n",
		resize_param->MainOut_Img.ycc_type);
	dev_dbg(ipcm_dev, "resize_param->MainOut_Img.ss_mode = 0x%x\n",
		resize_param->MainOut_Img.ss_mode);
	dev_dbg(ipcm_dev, "resize_param->MainOut_Img.scan_type = 0x%x\n",
		resize_param->MainOut_Img.scan_type);
	dev_dbg(ipcm_dev, "resize_param->MainOut_Img.addr_y0 = 0x%x\n",
		resize_param->MainOut_Img.addr_y0);
	dev_dbg(ipcm_dev, "resize_param->MainOut_Img.addr_y1 = 0x%x\n",
		resize_param->MainOut_Img.addr_y1);
	dev_dbg(ipcm_dev, "resize_param->MainOut_Img.addr_c0 = 0x%x\n",
		resize_param->MainOut_Img.addr_c0);
	dev_dbg(ipcm_dev, "resize_param->MainOut_Img.addr_c1 = 0x%x\n",
		resize_param->MainOut_Img.addr_c1);
	dev_dbg(ipcm_dev, "resize_param->MainOut_Img.pix_offset_x = %d\n",
		resize_param->MainOut_Img.pix_offset_x);
	dev_dbg(ipcm_dev, "resize_param->MainOut_Img.pix_offset_y = %d\n",
		resize_param->MainOut_Img.pix_offset_y);
	dev_dbg(ipcm_dev, "resize_param->MainOut_Img.reverse_CbCr = %d\n",
		resize_param->MainOut_Img.reverse_CbCr);
	dev_dbg(ipcm_dev, "resize_param->MainOut_Img.NLC_sw = %d\n",
		resize_param->MainOut_Img.NLC_sw);
	dev_dbg(ipcm_dev, "resize_param->MainOut_Img.NLC_rc = %d\n\n",
		resize_param->MainOut_Img.NLC_rc);

	/* resize out */
	dev_dbg(ipcm_dev, "resize_param->RSZOut_Img.width = %d\n",
		resize_param->RSZOut_Img.width);
	dev_dbg(ipcm_dev, "resize_param->RSZOut_Img.height = %d\n",
		resize_param->RSZOut_Img.height);
	dev_dbg(ipcm_dev, "resize_param->RSZOut_Img.stride_y = %d\n",
		resize_param->RSZOut_Img.stride_y);
	dev_dbg(ipcm_dev, "resize_param->RSZOut_Img.stride_c = %d\n",
		resize_param->RSZOut_Img.stride_c);
	dev_dbg(ipcm_dev, "resize_param->RSZOut_Img.ycc_type = %d\n",
		resize_param->RSZOut_Img.ycc_type);
	dev_dbg(ipcm_dev, "resize_param->RSZOut_Img.ss_mode = 0x%x\n",
		resize_param->RSZOut_Img.ss_mode);
	dev_dbg(ipcm_dev, "resize_param->RSZOut_Img.scan_type = 0x%x\n",
		resize_param->RSZOut_Img.scan_type);
	dev_dbg(ipcm_dev, "resize_param->RSZOut_Img.addr_y0 = 0x%x\n",
		resize_param->RSZOut_Img.addr_y0);
	dev_dbg(ipcm_dev, "resize_param->RSZOut_Img.addr_y1 = 0x%x\n",
		resize_param->RSZOut_Img.addr_y1);
	dev_dbg(ipcm_dev, "resize_param->RSZOut_Img.addr_c0 = 0x%x\n",
		resize_param->RSZOut_Img.addr_c0);
	dev_dbg(ipcm_dev, "resize_param->RSZOut_Img.addr_c1 = 0x%x\n",
		resize_param->RSZOut_Img.addr_c1);
	dev_dbg(ipcm_dev, "resize_param->RSZOut_Img.pix_offset_x = %d\n",
		resize_param->RSZOut_Img.pix_offset_x);
	dev_dbg(ipcm_dev, "resize_param->RSZOut_Img.pix_offset_y = %d\n",
		resize_param->RSZOut_Img.pix_offset_y);
	dev_dbg(ipcm_dev, "resize_param->RSZOut_Img.reverse_CbCr = %d\n",
		resize_param->RSZOut_Img.reverse_CbCr);
	dev_dbg(ipcm_dev, "resize_param->RSZOut_Img.NLC_sw = %d\n",
		resize_param->RSZOut_Img.NLC_sw);
	dev_dbg(ipcm_dev, "resize_param->RSZOut_Img.NLC_rc = %d\n\n",
		resize_param->RSZOut_Img.NLC_rc);

	/* sub resize out */
	dev_dbg(ipcm_dev, "resize_param->SRSZOut_Img.width = %d\n",
		resize_param->SRSZOut_Img.width);
	dev_dbg(ipcm_dev, "resize_param->SRSZOut_Img.height = %d\n",
		resize_param->SRSZOut_Img.height);
	dev_dbg(ipcm_dev, "resize_param->SRSZOut_Img.stride_y = %d\n",
		resize_param->SRSZOut_Img.stride_y);
	dev_dbg(ipcm_dev, "resize_param->SRSZOut_Img.stride_c = %d\n",
		resize_param->SRSZOut_Img.stride_c);
	dev_dbg(ipcm_dev, "resize_param->SRSZOut_Img.ycc_type = %d\n",
		resize_param->SRSZOut_Img.ycc_type);
	dev_dbg(ipcm_dev, "resize_param->SRSZOut_Img.ss_mode = 0x%x\n",
		resize_param->SRSZOut_Img.ss_mode);
	dev_dbg(ipcm_dev, "resize_param->SRSZOut_Img.scan_type = 0x%x\n",
		resize_param->SRSZOut_Img.scan_type);
	dev_dbg(ipcm_dev, "resize_param->SRSZOut_Img.addr_y0 = 0x%x\n",
		resize_param->SRSZOut_Img.addr_y0);
	dev_dbg(ipcm_dev, "resize_param->SRSZOut_Img.addr_y1 = 0x%x\n",
		resize_param->SRSZOut_Img.addr_y1);
	dev_dbg(ipcm_dev, "resize_param->SRSZOut_Img.addr_c0 = 0x%x\n",
		resize_param->SRSZOut_Img.addr_c0);
	dev_dbg(ipcm_dev, "resize_param->SRSZOut_Img.addr_c1 = 0x%x\n",
		resize_param->SRSZOut_Img.addr_c1);
	dev_dbg(ipcm_dev, "resize_param->SRSZOut_Img.pix_offset_x = %d\n",
		resize_param->SRSZOut_Img.pix_offset_x);
	dev_dbg(ipcm_dev, "resize_param->SRSZOut_Img.pix_offset_y = %d\n",
		resize_param->SRSZOut_Img.pix_offset_y);
	dev_dbg(ipcm_dev, "resize_param->SRSZOut_Img.reverse_CbCr = %d\n",
		resize_param->SRSZOut_Img.reverse_CbCr);
	dev_dbg(ipcm_dev, "resize_param->SRSZOut_Img.NLC_sw = %d\n",
		resize_param->SRSZOut_Img.NLC_sw);
	dev_dbg(ipcm_dev, "resize_param->SRSZOut_Img.NLC_rc = %d\n\n",
		resize_param->SRSZOut_Img.NLC_rc);

	dev_dbg(ipcm_dev, "resize_param->MainOut_SW = %d\n",
		resize_param->MainOut_SW);
	dev_dbg(ipcm_dev, "resize_param->RSZ_SW = %d\n",
		resize_param->RSZ_SW);
	dev_dbg(ipcm_dev, "resize_param->SRSZ_SW = %d\n",
		resize_param->SRSZ_SW);
}

void drime4_ipcm_fw_print_combi_op_parameter(
	struct ipcm_ycc_combi_op_info *combi_op_param)
{
	/* source */
	dev_dbg(ipcm_dev, "combi_op_param->Src_Img_Ycc.width = %d\n",
		combi_op_param->Src_Img_Ycc.width);
	dev_dbg(ipcm_dev, "combi_op_param->Src_Img_Ycc.height = %d\n",
		combi_op_param->Src_Img_Ycc.height);
	dev_dbg(ipcm_dev, "combi_op_param->Src_Img_Ycc.stride_y = %d\n",
		combi_op_param->Src_Img_Ycc.stride_y);
	dev_dbg(ipcm_dev, "combi_op_param->Src_Img_Ycc.stride_c = %d\n",
		combi_op_param->Src_Img_Ycc.stride_c);
	dev_dbg(ipcm_dev, "combi_op_param->Src_Img_Ycc.ycc_type = %d\n",
		combi_op_param->Src_Img_Ycc.ycc_type);
	dev_dbg(ipcm_dev, "combi_op_param->Src_Img_Ycc.ss_mode = 0x%x\n",
		combi_op_param->Src_Img_Ycc.ss_mode);
	dev_dbg(ipcm_dev, "combi_op_param->Src_Img_Ycc.scan_type = 0x%x\n",
		combi_op_param->Src_Img_Ycc.scan_type);
	dev_dbg(ipcm_dev, "combi_op_param->Src_Img_Ycc.addr_y0 = 0x%x\n",
		combi_op_param->Src_Img_Ycc.addr_y0);
	dev_dbg(ipcm_dev, "combi_op_param->Src_Img_Ycc.addr_y1 = 0x%x\n",
		combi_op_param->Src_Img_Ycc.addr_y1);
	dev_dbg(ipcm_dev, "combi_op_param->Src_Img_Ycc.addr_c0 = 0x%x\n",
		combi_op_param->Src_Img_Ycc.addr_c0);
	dev_dbg(ipcm_dev, "combi_op_param->Src_Img_Ycc.addr_c1 = 0x%x\n",
		combi_op_param->Src_Img_Ycc.addr_c1);
	dev_dbg(ipcm_dev, "combi_op_param->Src_Img_Ycc.pix_offset_x = %d\n",
		combi_op_param->Src_Img_Ycc.pix_offset_x);
	dev_dbg(ipcm_dev, "combi_op_param->Src_Img_Ycc.pix_offset_y = %d\n",
		combi_op_param->Src_Img_Ycc.pix_offset_y);
	dev_dbg(ipcm_dev, "combi_op_param->Src_Img_Ycc.reverse_CbCr = %d\n",
		combi_op_param->Src_Img_Ycc.reverse_CbCr);
	dev_dbg(ipcm_dev, "combi_op_param->Src_Img_Ycc.NLC_sw = %d\n",
		combi_op_param->Src_Img_Ycc.NLC_sw);
	dev_dbg(ipcm_dev, "combi_op_param->Src_Img_Ycc.NLC_rc = %d\n\n",
		combi_op_param->Src_Img_Ycc.NLC_rc);

	/* resize out */
	dev_dbg(ipcm_dev, "combi_op_param->RSZOut_Img.width = %d\n",
		combi_op_param->RSZOut_Img.width);
	dev_dbg(ipcm_dev, "combi_op_param->RSZOut_Img.height = %d\n",
		combi_op_param->RSZOut_Img.height);
	dev_dbg(ipcm_dev, "combi_op_param->RSZOut_Img.stride_y = %d\n",
		combi_op_param->RSZOut_Img.stride_y);
	dev_dbg(ipcm_dev, "combi_op_param->RSZOut_Img.stride_c = %d\n",
		combi_op_param->RSZOut_Img.stride_c);
	dev_dbg(ipcm_dev, "combi_op_param->RSZOut_Img.ycc_type = %d\n",
		combi_op_param->RSZOut_Img.ycc_type);
	dev_dbg(ipcm_dev, "combi_op_param->RSZOut_Img.ss_mode = 0x%x\n",
		combi_op_param->RSZOut_Img.ss_mode);
	dev_dbg(ipcm_dev, "combi_op_param->RSZOut_Img.scan_type = 0x%x\n",
		combi_op_param->RSZOut_Img.scan_type);
	dev_dbg(ipcm_dev, "combi_op_param->RSZOut_Img.addr_y0 = 0x%x\n",
		combi_op_param->RSZOut_Img.addr_y0);
	dev_dbg(ipcm_dev, "combi_op_param->RSZOut_Img.addr_y1 = 0x%x\n",
		combi_op_param->RSZOut_Img.addr_y1);
	dev_dbg(ipcm_dev, "combi_op_param->RSZOut_Img.addr_c0 = 0x%x\n",
		combi_op_param->RSZOut_Img.addr_c0);
	dev_dbg(ipcm_dev, "combi_op_param->RSZOut_Img.addr_c1 = 0x%x\n",
		combi_op_param->RSZOut_Img.addr_c1);
	dev_dbg(ipcm_dev, "combi_op_param->RSZOut_Img.pix_offset_x = %d\n",
		combi_op_param->RSZOut_Img.pix_offset_x);
	dev_dbg(ipcm_dev, "combi_op_param->RSZOut_Img.pix_offset_y = %d\n",
		combi_op_param->RSZOut_Img.pix_offset_y);
	dev_dbg(ipcm_dev, "combi_op_param->RSZOut_Img.reverse_CbCr = %d\n",
		combi_op_param->RSZOut_Img.reverse_CbCr);
	dev_dbg(ipcm_dev, "combi_op_param->RSZOut_Img.NLC_sw = %d\n",
		combi_op_param->RSZOut_Img.NLC_sw);
	dev_dbg(ipcm_dev, "combi_op_param->RSZOut_Img.NLC_rc = %d\n\n",
		combi_op_param->RSZOut_Img.NLC_rc);

	/* sub resize out */
	dev_dbg(ipcm_dev, "combi_op_param->SRSZOut_Img.width = %d\n",
		combi_op_param->SRSZOut_Img.width);
	dev_dbg(ipcm_dev, "combi_op_param->SRSZOut_Img.height = %d\n",
		combi_op_param->SRSZOut_Img.height);
	dev_dbg(ipcm_dev, "combi_op_param->SRSZOut_Img.stride_y = %d\n",
		combi_op_param->SRSZOut_Img.stride_y);
	dev_dbg(ipcm_dev, "combi_op_param->SRSZOut_Img.stride_c = %d\n",
		combi_op_param->SRSZOut_Img.stride_c);
	dev_dbg(ipcm_dev, "combi_op_param->SRSZOut_Img.ycc_type = %d\n",
		combi_op_param->SRSZOut_Img.ycc_type);
	dev_dbg(ipcm_dev, "combi_op_param->SRSZOut_Img.ss_mode = 0x%x\n",
		combi_op_param->SRSZOut_Img.ss_mode);
	dev_dbg(ipcm_dev, "combi_op_param->SRSZOut_Img.scan_type = 0x%x\n",
		combi_op_param->SRSZOut_Img.scan_type);
	dev_dbg(ipcm_dev, "combi_op_param->SRSZOut_Img.addr_y0 = 0x%x\n",
		combi_op_param->SRSZOut_Img.addr_y0);
	dev_dbg(ipcm_dev, "combi_op_param->SRSZOut_Img.addr_y1 = 0x%x\n",
		combi_op_param->SRSZOut_Img.addr_y1);
	dev_dbg(ipcm_dev, "combi_op_param->SRSZOut_Img.addr_c0 = 0x%x\n",
		combi_op_param->SRSZOut_Img.addr_c0);
	dev_dbg(ipcm_dev, "combi_op_param->SRSZOut_Img.addr_c1 = 0x%x\n",
		combi_op_param->SRSZOut_Img.addr_c1);
	dev_dbg(ipcm_dev, "combi_op_param->SRSZOut_Img.pix_offset_x = %d\n",
		combi_op_param->SRSZOut_Img.pix_offset_x);
	dev_dbg(ipcm_dev, "combi_op_param->SRSZOut_Img.pix_offset_y = %d\n",
		combi_op_param->SRSZOut_Img.pix_offset_y);
	dev_dbg(ipcm_dev, "combi_op_param->SRSZOut_Img.reverse_CbCr = %d\n",
		combi_op_param->SRSZOut_Img.reverse_CbCr);
	dev_dbg(ipcm_dev, "combi_op_param->SRSZOut_Img.NLC_sw = %d\n",
		combi_op_param->SRSZOut_Img.NLC_sw);
	dev_dbg(ipcm_dev, "combi_op_param->SRSZOut_Img.NLC_rc = %d\n\n",
		combi_op_param->SRSZOut_Img.NLC_rc);

	dev_dbg(ipcm_dev, "combi_op_param->RSZ_SW = %d\n",
		combi_op_param->RSZ_SW);
	dev_dbg(ipcm_dev, "combi_op_param->SRSZ_SW = %d\n",
		combi_op_param->SRSZ_SW);
}

MODULE_AUTHOR("Jangwon Lee <jang_won.lee@samsung.com>");
MODULE_LICENSE("GPL");

