/*
 * linux/drivers/media/video/samsung/mfc5x/mfc_dev.c
 *
 * Copyright (c) 2010 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com/
 *
 * Driver interface for Samsung MFC (Multi Function Codec - FIMV) driver
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/interrupt.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <linux/wait.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/dma-mapping.h>
#include <linux/delay.h>
#include <linux/jiffies.h>
#include <linux/types.h>

#include <linux/sched.h>
#include <linux/firmware.h>
#ifdef CONFIG_CPU_FREQ
#include <mach/cpufreq.h>
#endif
#include <mach/regs-codec_fb.h>
#include <mach/codec_nlc_regs.h>

#include <asm/io.h>
#include <asm/uaccess.h>

#include "mfc_dev.h"
#include "mfc_interface.h"
#include "mfc_reg.h"
#include "mfc_log.h"
#include "mfc_ctrl.h"
#include "mfc_buf.h"
#include "mfc_inst.h"
#include "mfc_pm.h"
#include "mfc_dec.h"
#include "mfc_enc.h"
#include "mfc_mem.h"
#include "mfc_cmd.h"
#include "mfc_errno.h"

#ifdef SYSMMU_MFC_ON
#include <plat/sysmmu.h>
#endif

#ifdef CONFIG_PM_RUNTIME
#include <linux/clk.h>
#endif

#define MFC_MINOR	252
//#define MFC_FW_NAME	"mfd_vo_120406.bin"
#define MFC_FW_NAME	"mfd_vo_130329.bin"

//#define _BUS_TUNE_

static struct mfc_dev *mfcdev;

static const struct NlcEnc const enc = {
	.COMP0_HEIGHT = 1088,
	.COMP0_NUM_TILE = 14,
	.COMP0_OFFSET = 0x22000,
	.COMP0_MAX_BUFSIZE = 28,
	.COMP0_SBLEV_TH = 20,
	.COMP0_BUFCHK_TH = 8,
	.COMP0_RC_OFF = 1,
	.COMP0_RC_OFF_EB = 0,
	.COMP0_BYTE_PER_LINE = 64,
	.COMP0_ERR_TERM1 = 320,
	.COMP0_ERR_TERM2 = 3,
	.COMP0_RC_STRT_LINE = 5,
	.COMP0_RC_FREQ_LINE = 0,
	.COMP1_HEIGHT = 544,
	.COMP1_NUM_TILE = 14,
	.COMP1_OFFSET = 0x11000,
	.COMP1_MAX_BUFSIZE = 28,
	.COMP1_SBLEV_TH = 20,
	.COMP1_BUFCHK_TH = 8,
	.COMP1_RC_OFF = 1,
	.COMP1_RC_OFF_EB = 0,
	.COMP1_BYTE_PER_LINE = 64,
	.COMP1_ERR_TERM1 = 320,
	.COMP1_ERR_TERM2 = 3,
	.COMP1_RC_STRT_LINE = 5,
	.COMP1_RC_FREQ_LINE = 0,
};

struct NlcDec const decCur = {
	.COMP0_HEIGHT = 1088,//Image height
	.COMP0_NUM_TILE = 14, //# of Tile - 1
//	.COMP0_OFFSET = 0x21c00, //Tile offset address
	.COMP0_OFFSET = 0x22000, //Tile offset address

	.COMP0_MAX_BUFSIZE = 28,
	.COMP0_SBLEV_TH = 20,
	.COMP0_BUFCHK_TH = 8,

	.COMP1_HEIGHT = 544,
	.COMP1_NUM_TILE = 14,
//	.COMP1_OFFSET = 0x10e00,
	.COMP1_OFFSET = 0x11000,

	.COMP1_MAX_BUFSIZE = 28,
	.COMP1_SBLEV_TH = 20,
	.COMP1_BUFCHK_TH = 8,
};

struct NlcDec const decRef = {
	.COMP0_HEIGHT = 1088,//Image height
	.COMP0_NUM_TILE = 14, //# of Tile - 1
	.COMP0_OFFSET = 0x22000, //Tile offset address

	.COMP0_MAX_BUFSIZE = 28,
	.COMP0_SBLEV_TH = 20,
	.COMP0_BUFCHK_TH = 8,

	.COMP1_HEIGHT = 544,
	.COMP1_NUM_TILE = 14,
	.COMP1_OFFSET = 0x11000,

	.COMP1_MAX_BUFSIZE = 28,
	.COMP1_SBLEV_TH = 20,
	.COMP1_BUFCHK_TH = 8,
};



#ifdef _BUS_TUNE_
	void __iomem	*base50060;
	void __iomem	*base50078;
	void __iomem	*base50000;
	void __iomem	*base5e001;
	void __iomem	*base5e002;
	void __iomem	*base5e003;
	void __iomem	*base5e004;
#endif


void codec_nlc_enc(struct mfc_inst_ctx *ctx)
{
	unsigned int value = 0;
	struct mfc_dev *dev = ctx->dev;

	D4_CODEC_NLC_ENC_COMP0_HEIGHT(value, enc.COMP0_HEIGHT);
	D4_CODEC_NLC_ENC_COMP0_NUMTILEm1(value, enc.COMP0_NUM_TILE);
	D4_CODEC_NLC_ENC_COMP0_MAX_BUFSIZE(value, enc.COMP0_MAX_BUFSIZE);
	WRITEL(value, NLC_ENC_COMP0_TILE_HEIGHT);

	WRITEL(enc.COMP0_OFFSET, NLC_ENC_COMP0_ADDROFFSET);

	value = 0;
	D4_CODEC_NLC_ENC_COMP0_RATE_CTRL_0_SBLEV_TH(value, enc.COMP0_SBLEV_TH);
	D4_CODEC_NLC_ENC_COMP0_RATE_CTRL_0_BUFCHK_TH(value, enc.COMP0_BUFCHK_TH);
	D4_CODEC_NLC_ENC_COMP0_RATE_CTRL_0_RC_OFF(value, enc.COMP0_RC_OFF);
	D4_CODEC_NLC_ENC_COMP0_RATE_CTRL_0_RC_OFF_EB(value, enc.COMP0_RC_OFF_EB);
	WRITEL(value, NLC_ENC_COMP0_RATE_CTRL_0);


	value = 0;
	D4_CODEC_NLC_ENC_COMP0_RATE_CTRL_1_BYTE_PER_LINE(value, enc.COMP0_BYTE_PER_LINE);
	D4_CODEC_NLC_ENC_COMP0_RATE_CTRL_1_RC_STRT_LINE(value, enc.COMP0_RC_STRT_LINE);
	D4_CODEC_NLC_ENC_COMP0_RATE_CTRL_1_RC_FREQ_LINE(value, enc.COMP0_RC_FREQ_LINE);
	WRITEL(value, NLC_ENC_COMP0_RATE_CTRL_1);

	value = 0;
	D4_CODEC_NLC_ENC_COMP0_RATE_CTRL_2_ERR_TERM1(value, enc.COMP0_ERR_TERM1);
	D4_CODEC_NLC_ENC_COMP0_RATE_CTRL_2_ERR_TERM2(value, enc.COMP0_ERR_TERM2);
	WRITEL(value, NLC_ENC_COMP0_RATE_CTRL_2);

	WRITEL(0xf << 4, 0xF11C);

	value = 0;
	D4_CODEC_NLC_ENC_COMP1_HEIGHT(value, enc.COMP1_HEIGHT);
	D4_CODEC_NLC_ENC_COMP1_NUMTILEm1(value, enc.COMP1_NUM_TILE);
	D4_CODEC_NLC_ENC_COMP1_MAX_BUFSIZE(value, enc.COMP1_MAX_BUFSIZE);
	WRITEL(value, NLC_ENC_COMP1_TILE_HEIGHT);

	WRITEL(enc.COMP1_OFFSET, NLC_ENC_COMP1_ADDROFFSET);

	value = 0;
	D4_CODEC_NLC_ENC_COMP1_RATE_CTRL_0_SBLEV_TH(value, enc.COMP1_SBLEV_TH);
	D4_CODEC_NLC_ENC_COMP1_RATE_CTRL_0_BUFCHK_TH(value, enc.COMP1_BUFCHK_TH);
	D4_CODEC_NLC_ENC_COMP1_RATE_CTRL_0_RC_OFF(value, enc.COMP1_RC_OFF);
	D4_CODEC_NLC_ENC_COMP1_RATE_CTRL_0_RC_OFF_EB(value, enc.COMP1_RC_OFF_EB);
	WRITEL(value, NLC_ENC_COMP1_RATE_CTRL_0);

	value = 0;
	D4_CODEC_NLC_ENC_COMP1_RATE_CTRL_1_BYTE_PER_LINE(value, enc.COMP1_BYTE_PER_LINE);
	D4_CODEC_NLC_ENC_COMP1_RATE_CTRL_1_RC_STRT_LINE(value, enc.COMP1_RC_STRT_LINE);
	D4_CODEC_NLC_ENC_COMP1_RATE_CTRL_1_RC_FREQ_LINE(value, enc.COMP1_RC_FREQ_LINE);
	WRITEL(value, NLC_ENC_COMP1_RATE_CTRL_1);

	value = 0;
	D4_CODEC_NLC_ENC_COMP1_RATE_CTRL_2_ERR_TERM1(value, enc.COMP1_ERR_TERM1);
	D4_CODEC_NLC_ENC_COMP1_RATE_CTRL_2_ERR_TERM2(value, enc.COMP1_ERR_TERM2);
	WRITEL(value, NLC_ENC_COMP1_RATE_CTRL_2);

}

void codec_nlc_dec_current(struct mfc_inst_ctx *ctx)
{
	unsigned int value = 0;
	struct mfc_dev *dev = ctx->dev;

	D4_CODEC_NLC_DEC_COMP0_HEIGHT(value, decCur.COMP0_HEIGHT);
	D4_CODEC_NLC_DEC_COMP0_NUMTILEm1(value, decCur.COMP0_NUM_TILE);
	D4_CODEC_NLC_DEC_COMP0_MAX_BUFSIZE(value, decCur.COMP0_MAX_BUFSIZE);
	WRITEL(value, 0xF200 + NLC_DEC_COMP0_TILE_HEIGHT);

	WRITEL(decCur.COMP0_OFFSET, 0xF200 + NLC_DEC_COMP0_ADDROFFSET);

	value = 0;
	D4_CODEC_NLC_DEC_COMP0_SBLEV_TH(value, decCur.COMP0_SBLEV_TH);
	D4_CODEC_NLC_DEC_COMP0_BUFCHK_TH(value, decCur.COMP0_BUFCHK_TH);
	WRITEL(value, 0xF200 + NLC_DEC_COMP0_BUFFERCHECK);

	WRITEL(0xf << 4, 0xF21C);

	value = 0;
	D4_CODEC_NLC_DEC_COMP1_HEIGHT(value, decCur.COMP1_HEIGHT);
	D4_CODEC_NLC_DEC_COMP1_NUMTILEm1(value, decCur.COMP1_NUM_TILE);
	D4_CODEC_NLC_DEC_COMP1_MAX_BUFSIZE(value, decCur.COMP1_MAX_BUFSIZE);
	WRITEL(value, 0xF200 + NLC_DEC_COMP1_TILE_HEIGHT);

	WRITEL(decCur.COMP1_OFFSET, 0xF200 + NLC_DEC_COMP1_ADDROFFSET);

	value = 0;
	D4_CODEC_NLC_DEC_COMP1_SBLEV_TH(value, decCur.COMP1_SBLEV_TH);
	D4_CODEC_NLC_DEC_COMP1_BUFCHK_TH(value, decCur.COMP1_BUFCHK_TH);
	WRITEL(value, 0xF200 + NLC_DEC_COMP1_BUFFERCHECK);
}


void codec_nlc_dec_ref(struct mfc_inst_ctx *ctx)
{
	unsigned int DATA00 = 0;
	unsigned int DATA0C = 0;
	unsigned int DATA20 = 0;
	unsigned int DATA2C = 0;
	struct mfc_dev *dev = ctx->dev;

	D4_CODEC_NLC_DEC_COMP0_HEIGHT(DATA00, decRef.COMP0_HEIGHT);
	D4_CODEC_NLC_DEC_COMP0_NUMTILEm1(DATA00, decRef.COMP0_NUM_TILE);
	D4_CODEC_NLC_DEC_COMP0_MAX_BUFSIZE(DATA00, decRef.COMP0_MAX_BUFSIZE);
	WRITEL(DATA00, 0xF300 + NLC_DEC_COMP0_TILE_HEIGHT);

	WRITEL(decRef.COMP0_OFFSET, 0xF300 + NLC_DEC_COMP0_ADDROFFSET);

	D4_CODEC_NLC_DEC_COMP0_SBLEV_TH(DATA0C, decRef.COMP0_SBLEV_TH);
	D4_CODEC_NLC_DEC_COMP0_BUFCHK_TH(DATA0C, decRef.COMP0_BUFCHK_TH);
	WRITEL(DATA0C, 0xF300 + NLC_DEC_COMP0_BUFFERCHECK);

	WRITEL(0xf << 4, 0xF31C);

	D4_CODEC_NLC_DEC_COMP1_HEIGHT(DATA20, decRef.COMP1_HEIGHT);
	D4_CODEC_NLC_DEC_COMP1_NUMTILEm1(DATA20, decRef.COMP1_NUM_TILE);
	D4_CODEC_NLC_DEC_COMP1_MAX_BUFSIZE(DATA20, decRef.COMP1_MAX_BUFSIZE);
	WRITEL(DATA20, 0xF300 + NLC_DEC_COMP1_TILE_HEIGHT);

	WRITEL(decRef.COMP1_OFFSET, 0xF300 + NLC_DEC_COMP1_ADDROFFSET);

	D4_CODEC_NLC_DEC_COMP1_SBLEV_TH(DATA2C, decRef.COMP1_SBLEV_TH);
	D4_CODEC_NLC_DEC_COMP1_BUFCHK_TH(DATA2C, decRef.COMP1_BUFCHK_TH);
	WRITEL(DATA2C, 0xF300 + NLC_DEC_COMP1_BUFFERCHECK);

	WRITEL(DATA00, 0xF400 + NLC_DEC_COMP0_TILE_HEIGHT);
	WRITEL(decRef.COMP0_OFFSET, 0xF400 + NLC_DEC_COMP0_ADDROFFSET);
	WRITEL(DATA0C, 0xF400 + NLC_DEC_COMP0_BUFFERCHECK);

	WRITEL(0xf << 4, 0xF41C);

	WRITEL(DATA20, 0xF400 + NLC_DEC_COMP1_TILE_HEIGHT);
	WRITEL(decRef.COMP1_OFFSET, 0xF400 + NLC_DEC_COMP1_ADDROFFSET);
	WRITEL(DATA2C, 0xF400 + NLC_DEC_COMP1_BUFFERCHECK);
}
/* define drime4 framebuffer init func */
static int mfc_fb_find_idx(struct mfc_common_args *in_param, int target_frame)
{
	unsigned int i = 0, idx = 0;

	while (i < ENC_INPUT_BUF_NO) {
		if (in_param->encoder_input_info[i].frame_num  == \
			target_frame) {
			idx = i;
			mfc_dbg("input buffer[%d] has %dth target frame\n", \
				idx, target_frame);
			break;
	}
	i++;
	}

	if (i == ENC_INPUT_BUF_NO)
		return -1;

	return idx;
}

static int mfc_fb_calc_frame_num(struct mfc_inst_ctx *ctx, int num_curframe)
{
	unsigned int gopframes;
	unsigned int num_bframe, period_iframe;
	unsigned int totalframes, order_in_gop, num_gop;

	/* FIXME: hardcoded */
	totalframes = 90;

	num_bframe = ctx->numofbframe;
	period_iframe = ctx->period_of_iframe;

	if (period_iframe == 0)
		gopframes = totalframes;
	else
		gopframes = period_iframe + (period_iframe - 1)*num_bframe;

	num_gop = num_curframe / gopframes;
	order_in_gop = num_curframe % gopframes;

	mfc_dbg("%s: num_gop[%d], order_in_gop[%d], num_curframe[%d], \
		gopframes[%d]\n", __func__,  num_gop, order_in_gop, \
		num_curframe, gopframes);

	if (order_in_gop == 0)
		/* for I frame */
		return num_curframe;
	else if (order_in_gop % (num_bframe + 1) == 1)
		/* for P frame */
		return num_gop * gopframes + order_in_gop + num_bframe;
	else
		/* for B frame */
		return num_gop * gopframes + order_in_gop - 1;

	return 0;
}

static int mfc_fb_cal_enc_frame_type(struct mfc_inst_ctx *ctx, \
		struct mfc_common_args *in_param)
{
	unsigned int num_curframe, period_of_iframe;
	struct mfc_enc_ctx *enc_ctx = (struct mfc_enc_ctx *)ctx->c_priv;

	num_curframe = in_param->args.enc_exe.current_frame_num;
	period_of_iframe = ctx->period_of_iframe;
	mfc_dbg("current encoding frame num=%d\n", num_curframe);

	if ((num_curframe % (period_of_iframe)) == 0)
		enc_ctx->frametype = ENC_FRM_I;
	else
		enc_ctx->frametype = ENC_FRM_P;

	mfc_dbg("current frame type=%d\n", enc_ctx->frametype);

	return 0;
}

static int mfc_fb_enc_switch_buf_y(struct mfc_inst_ctx *ctx)
{
	if (ctx->rec_for_cur_frame_y == ctx->enc_ref_buf_y0_addr)
		return ctx->enc_ref_buf_y2_addr;
	else
		return ctx->enc_ref_buf_y0_addr;
}

static int mfc_fb_enc_switch_buf_c(struct mfc_inst_ctx *ctx)
{
	if (ctx->rec_for_cur_frame_c == ctx->enc_ref_buf_c0_addr)
		return ctx->enc_ref_buf_c2_addr;
	else
		return ctx->enc_ref_buf_c0_addr;
}

static int mfc_fb_set_enc_buf_addr(struct mfc_inst_ctx *ctx, \
		struct mfc_common_args *in_param)
{
	unsigned int a;
	struct mfc_enc_ctx *enc_ctx = (struct mfc_enc_ctx *)ctx->c_priv;
	if (enc_ctx->frametype == ENC_FRM_I) {
		/* set recon buffer only
		 * for I frame, only y0 buf is used for recon buf
		 * I dont know buf MFC decide to do so
		 * we can see that by checking f054, f058 regs
		 */
		ctx->rec_for_cur_frame_y = ctx->enc_ref_buf_y0_addr;
		ctx->rec_for_cur_frame_c = ctx->enc_ref_buf_c0_addr;

		write_reg(ctx->rec_for_cur_frame_y, DRIME4_FB_SW_RECON_Y);
		write_reg(ctx->rec_for_cur_frame_c, DRIME4_FB_SW_RECON_C);

		mfc_dbg("recon buf address for FB is 0x%x\n",\
				ctx->rec_for_cur_frame_y);

		ctx->ref_for_next_frame_y = ctx->rec_for_cur_frame_y;
		ctx->ref_for_next_frame_c = ctx->rec_for_cur_frame_c;

		/* I frame do not need any reference frame */

	} else { /* ENC_FRM_P */
		/* set recon and ref buffer both */
		ctx->rec_for_cur_frame_y = mfc_fb_enc_switch_buf_y(ctx);
		ctx->rec_for_cur_frame_c = mfc_fb_enc_switch_buf_c(ctx);
		mfc_dbg("recon buf address for FB is 0x%x\n",\
				ctx->rec_for_cur_frame_y);

		write_reg(ctx->rec_for_cur_frame_y, DRIME4_FB_SW_RECON_Y);
		write_reg(ctx->rec_for_cur_frame_c, DRIME4_FB_SW_RECON_C);
		/* write reference frame addresses */
		write_reg(ctx->ref_for_next_frame_y, DRIME4_FB_SW_REF_Y);
		write_reg(ctx->ref_for_next_frame_c, DRIME4_FB_SW_REF_C);
		mfc_dbg("ref buf address for FB is 0x%x\n",\
				ctx->ref_for_next_frame_y);

		ctx->ref_for_next_frame_y = ctx->rec_for_cur_frame_y;
		ctx->ref_for_next_frame_c = ctx->rec_for_cur_frame_c;
	}

	return 0;
}
/* return current recon buffer's index */
static int find_index(struct mfc_inst_ctx *ctx, unsigned int flag)
{
	unsigned int idx, i = 0;
	struct mfc_dec_ctx *dec_ctx = (struct mfc_dec_ctx *)ctx->c_priv;

	while(i < dec_ctx->numtotaldpb) {
		if (flag == 0) {
			if (ctx->rec_for_cur_frame_y == *(ctx->dpb_y + i)) {
				idx = i;
				break;
			}
			i++;
		} else if (flag == 1) {
			if (ctx->rec_for_cur_frame_c == *(ctx->dpb_c + i)) {
				idx = i;
				break;
			}
			i++;
		}
	}
	if (idx == dec_ctx->numtotaldpb - 1) {
		mfc_dbg("next idx of buffer is 0\n");
		ctx->current_recon_index = 0;
		return 0;
	} else {
		mfc_dbg("next idx of buffer is %d\n", idx + 1);
		ctx->current_recon_index = idx + 1;
		return idx + 1;
	}
}

static int mfc_fb_dec_switch_buf_y(struct mfc_inst_ctx *ctx)
{
	unsigned int idx;
	u32 base_addr = 0;
	u32 ret = 0;

	if (ctx->dpb_y == NULL) {
		mfc_err("ctx->dpb_y has NULL pointer\n");
		return -1;
	}
	base_addr = *(ctx->dpb_y);

	if (ctx->first_frame == 0) {
		ret = *(ctx->dpb_y);
		mfc_dbg("first frame\n");
	} else if (ctx->first_frame == 1) {
		idx = find_index(ctx, 0);
		ret = *(ctx->dpb_y + idx);
	}

	return ret;
}

static int mfc_fb_dec_switch_buf_c(struct mfc_inst_ctx *ctx)
{
	unsigned int idx;
	u32 base_addr = 0;
	u32 ret = 0;

	if (ctx->dpb_c == NULL) {
		mfc_err("ctx->dpb_c has NULL pointer\n");
		return -1;
	}
	base_addr = *(ctx->dpb_c);

	if (ctx->first_frame == 0) {
		ret = *(ctx->dpb_c);
		mfc_dbg("first frame\n");
	} else if (ctx->first_frame == 1) {
		idx = find_index(ctx, 1);
		ret = *(ctx->dpb_c + idx);
	}

	return ret;
}

static int mfc_fb_set_dec_buf_addr(struct mfc_inst_ctx *ctx, \
		struct mfc_common_args *in_param)
{
	unsigned int a;
	struct mfc_enc_ctx *enc_ctx = (struct mfc_enc_ctx *)ctx->c_priv;

	if (ctx->first_frame == 0) {
	/* No need to tell frame type */
	/* set recon and ref buffer both */
		ctx->rec_for_cur_frame_y = mfc_fb_dec_switch_buf_y(ctx);
		ctx->rec_for_cur_frame_c = mfc_fb_dec_switch_buf_c(ctx);
		mfc_dbg("first frame\n");
		mfc_dbg("recon y buf address for FB is 0x%x\n",\
				ctx->rec_for_cur_frame_y);
		mfc_dbg("recon c buf address for FB is 0x%x\n",\
				ctx->rec_for_cur_frame_c);
		ctx->first_frame = 1;
	} else if (ctx->first_frame == 1) {
		ctx->rec_for_cur_frame_y = mfc_fb_dec_switch_buf_y(ctx);
		ctx->rec_for_cur_frame_c = mfc_fb_dec_switch_buf_c(ctx);
		mfc_dbg("recon y buf address for FB is 0x%x\n",\
				ctx->rec_for_cur_frame_y);
		mfc_dbg("recon c buf address for FB is 0x%x\n",\
				ctx->rec_for_cur_frame_c);
	}

	write_reg(ctx->rec_for_cur_frame_y, DRIME4_FB_SW_RECON_Y);
	write_reg(ctx->rec_for_cur_frame_c, DRIME4_FB_SW_RECON_C);

	write_reg(0, DRIME4_FB_SW_REF_Y);
	write_reg(0, DRIME4_FB_SW_REF_C);

	/* write reference frame addresses */
	write_reg(ctx->ref_for_next_frame_y, DRIME4_FB_SW_REF2_Y);
	write_reg(ctx->ref_for_next_frame_c, DRIME4_FB_SW_REF2_C);
	mfc_dbg("ref buf address for FB is 0x%x\n",\
			ctx->ref_for_next_frame_y);

	ctx->ref_for_next_frame_y = ctx->rec_for_cur_frame_y;
	ctx->ref_for_next_frame_c = ctx->rec_for_cur_frame_c;

	return 0;
}
static int mfc_fb_sw_mode_start(struct mfc_inst_ctx *ctx)
{
	/* in s/w base FB, only 1ref works 
	 * h/w team decided not to use 2ref in revision chip
	 * due to timing problem
	 */
	unsigned int reg = 0;
	if (ctx->job_status == MFC_ENCODING) {

		struct mfc_enc_ctx *enc_ctx = (struct mfc_enc_ctx *)ctx->c_priv;

		if (enc_ctx->frametype == ENC_FRM_I) {
			/* set recon buffer only */
			mfc_dbg("frametype is ENC_FRM_I\n");
			reg = read_reg(DRIME4_FB_CODEC_TOP_CTRL);
			reg = reg | (0x9 << 10);
		} else { /* ENC_FRM_P */
			/* set recon and ref buffer both */
			mfc_dbg("frametype is ENC_FRM_P\n");
			reg = read_reg(DRIME4_FB_CODEC_TOP_CTRL);
			reg = reg | (0xB << 10);
		}
	} else if (ctx->job_status == MFC_DECODING) {

		struct mfc_dec_ctx *dec_ctx = (struct mfc_dec_ctx *)ctx->c_priv;

//		if (dec_ctx->frametype == DEC_FRM_I) {
//			/* set recon buffer only */
//			mfc_dbg("frametype is DEC_FRM_I\n");
//			reg = read_reg(DRIME4_FB_CODEC_TOP_CTRL);
//			reg = reg | (0x8 << 10);
//		} else if (dec_ctx->frametype == DEC_FRM_P) { /* ENC_FRM_P */
//			/* set recon and ref buffer both */
//			mfc_dbg("frametype is DEC_FRM_P\n");
			mfc_dbg("sw mode start\n");
			reg = read_reg(DRIME4_FB_CODEC_TOP_CTRL);
			reg = reg | (0xC << 10);
//		}
	}

	/* common part */
	/* reset [13:10] bit to zero */
	write_reg(reg, DRIME4_FB_CODEC_TOP_CTRL);
	reg &= 0xFFFFC3FF;
	write_reg(reg, DRIME4_FB_CODEC_TOP_CTRL);

	return 0;
}

static int mfc_fb_set_curr_img(struct mfc_inst_ctx *ctx, \
		struct mfc_common_args *in_param)
{
	unsigned int num_curframe, target_frame;
	int idx;
	struct mfc_dev *dev = ctx->dev;

	if (ctx->numofbframe == 0) {
		/* [In case of IPPP sequence]
		 * For 60 frame/s encoding, the address of the input buffer
		 * containg YUV data which are ready to be encoded
		 * should be set to framebuffer register before mfc start encoding
		 */
		WRITEL(in_param->args.enc_exe.in_Y_addr, \
			DRIME4_FB_CUR_LUMA_BASE);
		WRITEL(in_param->args.enc_exe.in_CbCr_addr, \
			DRIME4_FB_CUR_CHROMA_BASE);
		mfc_dbg("in param->args.enc_exe.in_Y_addr =0x%x\n", in_param->args.enc_exe.in_Y_addr);
	} else if (ctx->numofbframe != 0){
		num_curframe = in_param->args.enc_exe.current_frame_num;
		mfc_dbg("current encoding frame num=%d\n", num_curframe);

		/* [In case of IBBP sequence]
		 * For 60 frame/s encoding, the address of the input buffer
		 * containg YUV data which are ready to be encoded
		 * should be set to framebuffer register before mfc start encoding
		 * so that framebuffer will be able to prepatch the input buffer.
		 * application just send the information of the frame number
		 * each buffer hold currently. calculating target frame number will
		 * be done in driver.
		 *
		 * [Driver status]
		 * At this moment, this driver only support IPPP and IBBP (B=2)sequences.
		 * The generic way of calculating target frame has not been
		 * implemented yet. Because the maximum number of b frame that MFC
		 * support is just 2 and IBPBP (B=1) sequence is not practical,
		 * I think there wouldn't be a problem.
		 */
		if (num_curframe < ctx->numofbframe)
			return 0;

		target_frame = mfc_fb_calc_frame_num(ctx, num_curframe \
			- ctx->numofbframe);

		/* find index of input buffer which has target frame*/
		idx = mfc_fb_find_idx(in_param, target_frame);

		/* use should hand over number of input buffer */
		if (idx < 0) {
			mfc_err("can not find input buffer containing %dth \
				target frame\n", target_frame);
			goto err;
		}

		WRITEL((unsigned int)in_param->encoder_input_info[idx].YPhyAddr, \
			DRIME4_FB_CUR_LUMA_BASE);
		WRITEL((unsigned int)in_param->encoder_input_info[idx].CPhyAddr, \
			DRIME4_FB_CUR_CHROMA_BASE);
		mfc_dbg("%s: addr f014(0x%08x) / f018(0x%08x)\n", __func__, \
			(unsigned int)in_param->encoder_input_info[idx].YPhyAddr,
			(unsigned int)in_param->encoder_input_info[idx].CPhyAddr);
	}

	return 0;

err:
	return MFC_FB_SET_FAIL;
};
EXPORT_SYMBOL(mfc_fb_set_curr_img);

int mfc_fb_init(struct mfc_inst_ctx *ctx)
{
	struct mfc_dev *dev = ctx->dev;
	struct drime4_codec_framebuffer_info *fb_info;
	struct drime4_codec_framebuffer_info *pd_array;
	enum fb_state fb_state = ctx->fb_state;
	pd_array = (struct drime4_codec_framebuffer_info *)dev->device->platform_data;
	if (pd_array == NULL) {
		dev_err(dev->device, "platform_data missing!\n");
		return -ENODEV;
	}

	/* [FB restriction]
	 * FB spec says that it should be turn off so that MFC
	 * can access data bus directly because FB only supports
	 * H264 format for decoding and encoding. If you try to
	 * run MFC for other formats except H264 with FB on,
	 * it won't work.
	 */
	if (ctx->codec_type != H264_DEC && ctx->codec_type != H264_ENC) {
		mfc_dbg("FB is going to be off\n");
		WRITEL((DRIME4_FB_CTRL_NR_OF_REF(1) | \
			DRIME4_FB_CTRL_BASE_ADDR_OPT1 | \
			DRIME4_FB_CTRL_BYPASS | DRIME4_FB_CTRL_FB_OFF), \
			DRIME4_FB_CODEC_TOP_CTRL);
		return 0;
	}

	/* TODO: add other formats 
	 * we set fb_info as NULL for the formats that are not supported by FB
	 */
	switch (ctx->width) {
	case 352:
		fb_info = pd_array;
		break;
	case 320:
		fb_info = NULL;
		break;
	case 640:
		fb_info = pd_array + 1;
		break;
	case 1920:
		if(ctx->height == 816)
			fb_info = pd_array + 5;
		else
			fb_info = pd_array + 2;
		break;
	case 1280:
		fb_info = pd_array + 3;
		break;
	case 720:
		fb_info = pd_array + 4;
		break;
	default:
		printk(KERN_ERR "[%s] fb do not support %d \
		image\n", __func__, ctx->width);
		return -EINVAL;
	}

	/* used in IOCTL_MFC_DEC_EXE */
	ctx->fb_info = fb_info;

	/* This is where driver actually turn off FB
	 * therefore mfc_fb_init() must be called once during init
	 */
	if (ctx->job_status == MFC_DECODING) {
		/* framebuffer top control register */
		if (ctx->nlc_state == NLC_OFF) {
			/* only 1 reference is available in DRIME4 */
			WRITEL((DRIME4_FB_CTRL_NR_OF_REF(1) | \
				DRIME4_FB_CTRL_BASE_ADDR_OPT1 | \
				DRIME4_FB_CTRL_BYPASS | \
			/* s/w FB is set */
				DRIME4_FB_CTRL_SW_MODE_START | \
				DRIME4_FB_CTRL_STATE(fb_state)), \
				DRIME4_FB_CODEC_TOP_CTRL);
		} else {
			WRITEL((DRIME4_FB_CTRL_NR_OF_REF(1) | \
				DRIME4_FB_CTRL_BASE_ADDR_OPT1 | \
				DRIME4_FB_CTRL_BYPASS | \
			/* s/w FB is set */
				DRIME4_FB_CTRL_SW_MODE_START | \
				DRIME4_FB_CTRL_STATE(fb_state) | \
				DRIME4_FB_CTRL_NLC_REF_ON | \
				DRIME4_FB_CTRL_NLC_CUR_OFF), \
				DRIME4_FB_CODEC_TOP_CTRL);

			codec_nlc_enc(ctx);
			codec_nlc_dec_ref(ctx);
		}
		/* enable interrupt */
		WRITEL(DRIME4_FB_INTR_MASK_FB_ACTIVE, DRIME4_FB_INTR_MASK);
		/* enable interrupt for tile2linear module*/
		if (fb_state == FB_OFF)
			WRITEL(DRIME4_FB_INTR_MASK_T2R_ACTIVE, DRIME4_FB_INTR_MASK);
		WRITEL(0x0, DRIME4_FB_CUR_LUMA_BASE); /* current_luma_base */
		WRITEL(0x0, DRIME4_FB_CUR_CHROMA_BASE);
	} else if (ctx->job_status == MFC_ENCODING) {
		/* framebuffer top control register */
		if (ctx->nlc_state == NLC_OFF) {
			WRITEL((DRIME4_FB_CTRL_NR_OF_REF(1) | \
				DRIME4_FB_CTRL_BASE_ADDR_OPT1 | \
				DRIME4_FB_CTRL_BYPASS | \
			/* s/w FB is set */
				DRIME4_FB_CTRL_SW_MODE_START | \
				DRIME4_FB_CTRL_STATE(fb_state)), \
				DRIME4_FB_CODEC_TOP_CTRL);
		} else {
			WRITEL((DRIME4_FB_CTRL_NR_OF_REF(1) | \
				DRIME4_FB_CTRL_BASE_ADDR_OPT1 | \
				DRIME4_FB_CTRL_BYPASS | \
			/* s/w FB is set */
				DRIME4_FB_CTRL_SW_MODE_START | \
				DRIME4_FB_CTRL_STATE(fb_state) | \
				DRIME4_FB_CTRL_NLC_REF_ON | \
				DRIME4_FB_CTRL_NLC_CUR_ON), \
				DRIME4_FB_CODEC_TOP_CTRL);

			codec_nlc_enc(ctx);
			codec_nlc_dec_current(ctx);
			codec_nlc_dec_ref(ctx);
		}
		/* enable interrupt */
		WRITEL(DRIME4_FB_INTR_MASK_FB_ACTIVE, DRIME4_FB_INTR_MASK);
		WRITEL(0, DRIME4_FB_CUR_LUMA_BASE);
		WRITEL(0, DRIME4_FB_CUR_CHROMA_BASE);
	}

	/*FIXME*/
	/* Althought fb_info is NULL, upper routine will be executed */
	if (fb_info == NULL) {
		mfc_dbg("mfc_fb_init(): FB is going to be off \
			cause resolution that are not supported by FB is used\n");
		WRITEL((DRIME4_FB_CTRL_NR_OF_REF(1) | \
			DRIME4_FB_CTRL_BASE_ADDR_OPT1 | \
			DRIME4_FB_CTRL_BYPASS | DRIME4_FB_CTRL_FB_OFF), \
			DRIME4_FB_CODEC_TOP_CTRL);
		return 0;
	} else {
	/* image width  */
	WRITEL(fb_info->img_width, DRIME4_FB_IMG_WIDTH);
	/* image height */
	WRITEL(fb_info->img_height, DRIME4_FB_IMG_HEIGHT);
	/* partition */
	WRITEL(fb_info->partition, DRIME4_FB_PARTITION);
	/* mbx_tile_info */
	WRITEL(fb_info->mbx_tile_info, DRIME4_FB_MBX_TILE_INFO);
	/* port B base */
	WRITEL(dev->mem_infos[1].base, DRIME4_FB_PORTB_BASE);
	/* address range luma */
	WRITEL(fb_info->address_range_luma, \
	DRIME4_FB_ADDR_RANGE_LUMA);
	/* address range chroma */
	WRITEL(fb_info->address_range_chroma, \
	DRIME4_FB_ADDR_RANGE_CHROMA);
	}

	mfc_dbg("drime4 codec framebuffer are setted  %d \
	image\n", ctx->width);
	return 0;
}
EXPORT_SYMBOL(mfc_fb_init);

static int mfc_fb_reset(void)
{
	unsigned int reg;
	unsigned int ref1_luma_state;
	unsigned int ref1_chroma_state;
	unsigned int ref2_luma_state;
	unsigned int ref2_chroma_state;
	unsigned int recon_luma_state;
	unsigned int recon_chroma_state;
	unsigned long timeout = jiffies;

	timeout += msecs_to_jiffies(FB_RESET_TIMEOUT);

	reg = read_reg(DRIME4_FB_CODEC_TOP_CTRL);
	reg |= DRIME4_FB_CTRL_SOFT_RESET_STEP1;
	write_reg(reg, DRIME4_FB_CODEC_TOP_CTRL);

	reg = read_reg(DRIME4_FB_CODEC_TOP_CTRL);
	reg &= DRIME4_FB_CTRL_SOFT_RESET_STEP2;
	write_reg(reg, DRIME4_FB_CODEC_TOP_CTRL);

	while (1) {
		ref1_luma_state = (read_reg(DRIME4_FB_REF1_LUMA_STATE) \
			& DRIME4_FB_FSM_MASK);
		ref1_chroma_state = (read_reg(DRIME4_FB_REF1_CHROMA_STATE) \
			& DRIME4_FB_FSM_MASK);
		ref2_luma_state = (read_reg(DRIME4_FB_REF2_LUMA_STATE) \
			& DRIME4_FB_FSM_MASK);
		ref2_chroma_state = (read_reg(DRIME4_FB_REF2_CHROMA_STATE) \
			& DRIME4_FB_FSM_MASK);
		recon_luma_state = (read_reg(DRIME4_FB_RECON_LUMA_STATE) \
			& DRIME4_FB_FSM_MASK);
		recon_chroma_state = (read_reg(DRIME4_FB_RECON_CHROMA_STATE) \
			& DRIME4_FB_FSM_MASK);

		if ((ref1_luma_state == 0) & (ref1_chroma_state == 0) \
			& (ref2_luma_state == 0) & (ref2_chroma_state == 0) \
			& (recon_luma_state == 0) & (recon_chroma_state == 0))
			break;

		if (time_after(jiffies, timeout)) {
			mfc_err("FB reset timeout!!\n");
			printk("FB reset timeout!!\n");
			return MFC_FB_RESET_FAIL;
		}
	}

	reg = read_reg(DRIME4_FB_CODEC_TOP_CTRL);
	reg = reg & 0xBFFFFFFF;
	write_reg(reg, DRIME4_FB_CODEC_TOP_CTRL);
	return 0;
}

static int mfc_fb_debug(void)
{
	unsigned int reg;
	unsigned int ref1_luma_state;
	unsigned int ref1_chroma_state;
	unsigned int ref2_luma_state;
	unsigned int ref2_chroma_state;
	unsigned int recon_luma_state;
	unsigned int recon_luma_buf;
	unsigned int recon_chroma_state;
	unsigned int recon_chroma_buf;

	ref1_luma_state = read_reg(DRIME4_FB_REF1_LUMA_STATE);
	ref1_chroma_state = read_reg(DRIME4_FB_REF1_CHROMA_STATE);
//	ref2_luma_state = read_reg(DRIME4_FB_REF2_LUMA_STATE);
//	ref2_chroma_state = read_reg(DRIME4_FB_REF2_CHROMA_STATE);
//	recon_luma_state = read_reg(DRIME4_FB_RECON_LUMA_STATE);
//	recon_chroma_state = read_reg(DRIME4_FB_RECON_CHROMA_STATE);

	recon_luma_buf = read_reg(DRIME4_FB_REF_REC_TEMP_LUMA);
	recon_chroma_buf = read_reg(DRIME4_FB_REF_REC_TEMP_CHROMA);

//	mfc_dbg("recon y addr = 0x%x\n", recon_luma_buf);
//	mfc_dbg("recon c addr = 0x%x\n", recon_chroma_buf);
	mfc_dbg("ref1 y addr = 0x%x\n", ref1_luma_state);
	mfc_dbg("ref1 c addr = 0x%x\n", ref1_chroma_state);

	return 0;
}
static int get_free_inst_id(struct mfc_dev *dev)
{
	int slot = 0;

	while (dev->inst_ctx[slot]) {
		slot++;
		if (slot >= MFC_MAX_INSTANCE_NUM)
			return -1;
	}

	return slot;
}

#ifdef _MFC_DEBUG_
extern void	mfc_init_debug();
extern void mfc_Print_Debug_Info(void);
#endif

static int mfc_open(struct inode *inode, struct file *file)
{
	struct mfc_inst_ctx *mfc_ctx = NULL;
	int ret;
	enum mfc_ret_code retcode;

	/* prevent invalid reference */
	file->private_data = NULL;

	mutex_lock(&mfcdev->lock);

	ret = mfc_init_pm(mfcdev);
	if (ret < 0) {
		printk(KERN_ERR "failed to init. MFC PM interface\n");
	}

	mfc_ctx = mfc_create_inst();
	if (!mfc_ctx) {
		mfc_err("failed to create instance context\n");
		ret = -ENOMEM;
	}

	/* there was hardware init code 2012-07-24 */

	mfc_ctx->id = get_free_inst_id(mfcdev);
	if (mfc_ctx->id < 0) {
		mfc_err("There are no openings for the instance\n");
		ret = -EINVAL;
	}

	mfc_info("[%s]Opened Instance id %d\n", __func__, mfc_ctx->id);
	mfc_ctx->dev = mfcdev;

	atomic_inc(&mfcdev->inst_cnt);
	mfcdev->inst_ctx[mfc_ctx->id] = mfc_ctx;

	file->private_data = (struct mfc_inst_ctx *)mfc_ctx;
#ifdef _MFC_DEBUG_
	mfc_init_debug();
#endif

	//#define	_CACHE_OFF
#ifdef _CACHE_OFF
    // Clean & invalidate L2 cache
    // Clean-up L2 cache for whole address space
    outer_clean_range(0x0, 0xffffffff);
    outer_inv_all();

    // Disable L1 cache & invalidate TLB
    __asm__ __volatile__ (
        "mrc p15, 0, %0, c1, c0, 0\n\t"
        "bic %0, %0, #(1 << 2)          @ Disable L1 cache\n\t"
        "mcr p15, 0, %0, c1, c0, 0\n\t"
        :"+r"(reg) : : "memory");
    __asm__ __volatile__ (
        "mcr p15, 0, r0, c8, c3, 0      @ Invalidate TLB\n\t"
        "mcr p15, 0, r0, c8, c5, 0\n\t"
        "mcr p15, 0, r0, c8, c6, 0\n\t"
        "mcr p15, 0, r0, c8, c7, 0\n\t"
        : : : "memory");

    // L2 cache off
    outer_disable();

    // Enable L1 cache
    __asm__ __volatile__ (
        "mrc p15, 0, %0, c1, c0, 0\n\t"
        "orr %0, %0, #(1 << 2)          @ Enable L1 cache\n\t"
        "mcr p15, 0, %0, c1, c0, 0\n\t"
        :"+r"(reg) : : "memory");
#endif

#define TUNE_2ND
#ifdef	 _BUS_TUNE_
 // set IPCM wait time
	writel(0x00400000, base50060 + 0xf04);
	writel(0x00300030, base50060 + 0xf08);
	writel(0x00300030, base50060 + 0xf0c);
	writel(0x00300030, base50060 + 0xf10);
	writel(0x00300030, base50060 + 0xf14);
	writel(0x00300030, base50060 + 0xf18);
	writel(0x00300030, base50060 + 0xf1c);
	writel(0x00300030, base50060 + 0xf20);
 // IPCM Thread Mapping Read-0, Write-3
	writel(0x0000ffff, base50060 + 0xf54);

 //EP thread mapping read-0, write-3
	writel(0x0000ffff, base50078 + 0x254);

 // IPCM Resize's DMA tune
	writel(0x11, base50060 + 0xb24);
	writel(0x11, base50060 + 0xd64);
	writel(0x11, base50060 + 0xda4);

  // DREX tune
	writel((readl(base50000 )&0x0000ffff), base50000 );
	writel((readl(base50000 )|0x00ff0000), base50000 );
	writel(0x00000000, base50000 + 0x60);
	writel(0x00030000, base50000 + 0x64);

	// DREX Codec Tune
	writel(0x00000003, base50000 + 0x68);
	writel(0x00030002, base50000 + 0x6c);

	// DREX Peri Tune
	//writel(0x00000003, base50000 + 0x70);
	//writel(0x00030003, base50000 + 0x74);

#ifdef TUNE_2ND
	// DREX ARM Tune
	writel(0x00000003, base50000 + 0x78);
	writel(0x00030001, base50000 + 0x7c);
#endif

  // Sonics bus tune
	writel(0xff, base5e002+ 0x900);
	writel(0x20, base5e003+ 0x100);
	writel(0xff, base5e003+ 0x900);
	writel(0xff, base5e003+ 0xd00);
	writel(0x80, base5e004+ 0x500);

#ifdef TUNE_2ND
	// Sonics Peri + Arm QoS
	writel(0xff, base5e001+ 0x100); // Arm0
	writel(0xff, base5e001+ 0x500); // Arm1
	//writel(0xff, base5e002+ 0x100); // Peri
#endif
	
#endif


	mutex_unlock(&mfcdev->lock);

	

	return 0;

err_start_hw:
	if (atomic_read(&mfcdev->inst_cnt) == 0) {
		if (mfc_power_off() < 0)
			mfc_err("power disable failed\n");
	}
/* err_fw_state: */
	kfree(mfc_ctx);
	mutex_unlock(&mfcdev->lock);

	return ret;
}


static int mfc_release(struct inode *inode, struct file *file)
{
	struct mfc_inst_ctx *mfc_ctx;
	struct mfc_dev *dev;
	int ret;

	mfc_ctx = (struct mfc_inst_ctx *)file->private_data;
	if (!mfc_ctx)
		return -EINVAL;

	dev = mfc_ctx->dev;

	mutex_lock(&dev->lock);

#ifdef CONFIG_CPU_FREQ
	/* Release MFC & Bus Frequency lock for High resolution */
	if (mfc_ctx->busfreq_flag == true){
		atomic_dec(&dev->busfreq_lock_cnt);
		mfc_ctx->busfreq_flag = false;
		if (atomic_read(&dev->busfreq_lock_cnt) == 0) {
			/* release Freq lock back to normal */
			s5pv310_busfreq_lock_free(DVFS_LOCK_ID_MFC);
			mfc_dbg("[%s] Bus Freq lock Released Normal!\n", __func__);
		}
	}
#endif

	file->private_data = NULL;

	mfc_info("[%s]Released Instance id %d\n", __func__, mfc_ctx->id);
	dev->inst_ctx[mfc_ctx->id] = NULL;
	atomic_dec(&dev->inst_cnt);

	mfc_destroy_inst(mfc_ctx);

	/* FIXME: why this code need? in multi instance is this ok? */
	/* destroy instance -> fb reset is right order */

	if (atomic_read(&dev->inst_cnt) == 0) {
		ret = mfc_fb_reset();
		if (ret < 0)
			mfc_err("fb reset failed\n");
		ret = mfc_power_off();
		if (ret < 0) {
			mfc_err("power disable failed.\n");
			goto err_pwr_disable;
		}
	}


	//mfc_clock_off();
#ifdef _MFC_DEBUG_
	mfc_Print_Debug_Info();
#endif

	ret = 0;

err_pwr_disable:
	mutex_unlock(&dev->lock);

	return ret;
}

/* FIXME: add request firmware ioctl */
static long mfc_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{

	struct mfc_inst_ctx *mfc_ctx;
	int ret, ex_ret;
	struct mfc_common_args in_param;
	struct mfc_buf_alloc_arg buf_arg;
	int port;

	struct mfc_dev *dev;
	struct mfc_dev *mfcdev;
	int i;
	unsigned int reg;

	struct mfc_set_config_arg *set_cnf_arg;
	enum mfc_ret_code retcode;

	mfc_ctx = (struct mfc_inst_ctx *)file->private_data;
	if (!mfc_ctx)
		return -EINVAL;

	dev = mfc_ctx->dev;
	mfcdev = mfc_ctx->dev;

	mutex_lock(&dev->lock);

	memset(&in_param, 0, sizeof(struct mfc_common_args));

	ret = copy_from_user(&in_param, (struct mfc_common_args *)arg,
			sizeof(struct mfc_common_args));
	if (ret < 0) {
		mfc_err("failed to copy parameters\n");
		ret = -EIO;
		in_param.ret_code = MFC_INVALID_PARAM_FAIL;
		goto out_ioctl;
	}

	mutex_unlock(&dev->lock);

	/* FIXME: add locking */

	mfc_dbg("cmd: 0x%08x\n", cmd);

	switch (cmd) {

	case IOCTL_MFC_FB_STATE:
		mutex_lock(&dev->lock);

		mfc_ctx->fb_state = in_param.fb_state;
		mfc_dbg("fb_state = %d\n", mfc_ctx->fb_state);
		if (in_param.fb_state != FB_ON && \
			in_param.fb_state != FB_OFF) {
			mfc_err("invalid fb state\n");
			ret = -EINVAL;
		}

		mfc_ctx->nlc_state = in_param.nlc_state;
		if (in_param.nlc_state != NLC_ON && \
			in_param.nlc_state != NLC_OFF) {
			mfc_err("invalid nlc state\n");
			ret = -EINVAL;
		}

		printk("MFC NLC : %d\n ",mfc_ctx->nlc_state);
		
		mutex_unlock(&dev->lock);
		break;

	case IOCTL_MFC_HW_INIT:
		mutex_lock(&dev->lock);

		printk("id = %d\n", mfc_ctx->id);

		if (mfc_ctx->id == 0) {
		printk("IOCTL_MFC_HW_INIT\n");
		mfc_power_on();
		ret = mfc_init_mem_mgr(mfcdev, &(in_param.args));
		if (ret < 0) {
			mfc_err("failed to init. MFC memory manager\n");
			ret = -EIO;
			//goto err_mem_mgr;
		}
		/* let memory manager know that where it can use */
		mfc_init_buf(&(in_param.args));

		/* reload F/W for first instance again */
		/* for multi instance coding, if there is another instance
		 * already working, we dont need to init MFC h/w again
		 */
		mfcdev->fw.state = mfc_load_firmware(mfcdev->fw.info->data, \
			mfcdev->fw.info->size);
		/* mfc_load_firmware function won't fail */
		/*
		if (!mfcdev->fw.state) {
			mfc_err("MFC F/W not load yet\n");
			ret = -ENODEV;
			goto err_fw_state;
		}
		*/
		mfc_info("MFC F/W reloaded for first Instance \
			successfully (size: %d)\n", mfcdev->fw.info->size);

		//mfc_clock_on();

		/* MFC hardware initialization */
		retcode = mfc_start(mfcdev);
		if (retcode != MFC_OK) {
			mfc_err("MFC H/W init failed: %d\n", retcode);
			ret = -ENODEV;
			//goto err_start_hw;
		}

		}
		mutex_unlock(&dev->lock);
		break;



	case IOCTL_MFC_DEC_INIT:
		mutex_lock(&dev->lock);

		if (mfc_chk_inst_state(mfc_ctx, INST_STATE_CREATE) < 0) {
			mfc_err("IOCTL_MFC_DEC_INIT invalid state: 0x%08x\n",
				 mfc_ctx->state);
			in_param.ret_code = MFC_STATE_INVALID;
			ret = -EINVAL;

			break;
		}

		//mfc_clock_on();
		mfc_ctx->job_status = MFC_DECODING;
		in_param.ret_code = mfc_init_decoding(mfc_ctx, &(in_param.args));
		ret = in_param.ret_code;
		//mfc_clock_off();

		mutex_unlock(&dev->lock);
		break;

	case IOCTL_MFC_ENC_INIT:
		mutex_lock(&dev->lock);

		if (mfc_chk_inst_state(mfc_ctx, INST_STATE_CREATE) < 0) {
			mfc_err("IOCTL_MFC_ENC_INIT invalid state: 0x%08x\n",
				 mfc_ctx->state);
			in_param.ret_code = MFC_STATE_INVALID;
			ret = -EINVAL;

			break;
		}

		//mfc_clock_on();
		mfc_ctx->job_status = MFC_ENCODING;
		in_param.ret_code = mfc_init_encoding(mfc_ctx, &(in_param.args));
		ret = in_param.ret_code;
		//mfc_clock_off();

		mutex_unlock(&dev->lock);
		break;

	case IOCTL_MFC_DEC_EXE:
		mutex_lock(&dev->lock);
//		printk("=====================================================\n");
		mfc_dbg("instance id = %d\n", mfc_ctx->id);

		/* This means that the resolution of the input is supported
		 * by FB and user want to use FB h/w
		 *
		 * condition A: driver check if FB h/w is going to work
		 * condition B: user check if FB h/w is going to work
		 * A has a higher priority that B about FB h/w's status
		 */

		if (mfc_ctx->fb_info != NULL && mfc_ctx->fb_state == FB_ON) {
			mfc_fb_set_dec_buf_addr(mfc_ctx, &in_param);
			mfc_fb_sw_mode_start(mfc_ctx);
		} else if (mfc_ctx->fb_info == NULL || mfc_ctx->fb_state == FB_OFF) {
		/* This means that the resolution of the input is not supported
		 * by FB and user do not want to use FB h/w
		 */
			/* do nothing */
		}
		//mfc_clock_on();
		in_param.ret_code = mfc_exec_decoding(mfc_ctx, &(in_param.args));
		ret = in_param.ret_code;
		//mfc_clock_off();
		mfc_fb_debug();

		mutex_unlock(&dev->lock);
		break;

	case IOCTL_MFC_ENC_EXE:
		mutex_lock(&dev->lock);
//		printk("=====================================================\n");
		mfc_dbg("instance id = %d\n", mfc_ctx->id);

		//mfc_clock_on();
		/* framebuffer setting for encoding
		 * reorder input buffer setting time????
		 */
		mfc_fb_set_curr_img(mfc_ctx, &in_param);

		/* code for FB setting in case multi instance coding with FB on
		 * 1. calculate current frm type
		 * 2. set FB register for recon and ref buffer
		 */
		if (mfc_ctx->fb_state == FB_ON ) {
			mfc_fb_cal_enc_frame_type(mfc_ctx, &in_param);
			mfc_fb_set_enc_buf_addr(mfc_ctx, &in_param);
			mfc_fb_sw_mode_start(mfc_ctx);
		} else if (mfc_ctx->fb_state == FB_OFF) {
			/* do nothing */
		}

//		reg = read_reg(DRIME4_FB_CODEC_TOP_CTRL);
//		printk("dump 0x5009f000 = 0x%x\n", reg);

		in_param.ret_code = mfc_exec_encoding(mfc_ctx, &(in_param.args));
		ret = in_param.ret_code;

		mfc_fb_debug();
		//mfc_clock_off();

		mutex_unlock(&dev->lock);
		break;

	case IOCTL_MFC_GET_IN_BUF:
		mutex_lock(&dev->lock);
		if (in_param.args.mem_alloc.type == ENCODER) {
		/* [CAUTION]
		 * Other drivers (ex, IPCM driver) must allocate 
		 * their output buffer at mfc region ranged from
		 * port1 addr to port1 addr + 256MB
		 * when they want to use their out as input of MFC
		 * MFC's access range through port B named "mfc1"
		   is from DRAMBASE_ADDR_B to DRAMBASE_ADDR_B + 256MB
		 */
			buf_arg.type = ENCODER;
			port = 1;
		} else {
			buf_arg.type = DECODER;
			port = 0;
		}

		/* FIXME: consider the size */
		buf_arg.size = in_param.args.mem_alloc.buff_size;
		/*
		 *buf_arg.mapped = in_param.args.mem_alloc.mapped_addr;
		 *FIXME: encodeing linear: 2KB, tile: 8KB
		 */
		buf_arg.align = ALIGN_2KB;

		if (buf_arg.type == ENCODER) {
			in_param.ret_code = mfc_alloc_buf(mfc_ctx, \
				&buf_arg, MBT_DPB | port);
		} else {
			in_param.ret_code = mfc_alloc_buf(mfc_ctx, \
				&buf_arg, MBT_CPB | port);
		}
#if defined(CONFIG_VIDEO_MFC_VCM_UMP)
		in_param.args.mem_alloc.secure_id = buf_arg.secure_id;
#elif defined(CONFIG_S5P_VMEM)
		in_param.args.mem_alloc.cookie = buf_arg.cookie;
#else
		in_param.args.mem_alloc.offset = buf_arg.offset;
#endif
		ret = in_param.ret_code;

		mutex_unlock(&dev->lock);

		break;

	case IOCTL_MFC_FREE_BUF:
		mutex_lock(&dev->lock);

		in_param.ret_code =
			mfc_free_buf(mfc_ctx, in_param.args.mem_free.key);
		ret = in_param.ret_code;

		mutex_unlock(&dev->lock);

		break;

	case IOCTL_MFC_GET_REAL_ADDR:
		mutex_lock(&dev->lock);

		in_param.args.real_addr.addr =
			mfc_get_buf_real(mfc_ctx->id, in_param.args.real_addr.key);

		mfc_dbg("real addr: 0x%08x", in_param.args.real_addr.addr);

		if (in_param.args.real_addr.addr)
			in_param.ret_code = MFC_OK;
		else
			in_param.ret_code = MFC_MEM_INVALID_ADDR_FAIL;

		ret = in_param.ret_code;

		mutex_unlock(&dev->lock);

		break;

	/* user let driver knows the buffer addr for converting operation */
	case IOCTL_MFC_WORK_TILE2LINEAR:
		mutex_lock(&dev->lock);
		struct mfc_buf_t2l *t2l_buf = (struct mfc_buf_t2l *)arg;

		mfc_err(">>> tiled 2 linear >>>> : 0x%x 0x%x 0x%x 0x%x\n",
				t2l_buf->src_y,
				t2l_buf->src_c,
				t2l_buf->dst_y,
				t2l_buf->dst_c);

		if(mfc_ctx->fb_state == FB_OFF) {
			mfc_fb_convert_4x2tile2linear(mfc_ctx, \
				(t2l_buf->src_y), \
				(t2l_buf->src_c), \
				t2l_buf->dst_y, \
				t2l_buf->dst_c);
		}

		mutex_unlock(&dev->lock);
		break;


	case IOCTL_MFC_GET_MMAP_SIZE:
		if (mfc_chk_inst_state(mfc_ctx, INST_STATE_CREATE) < 0) {
			mfc_err("IOCTL_MFC_GET_MMAP_SIZE invalid state: \
				0x%08x\n", mfc_ctx->state);
			in_param.ret_code = MFC_STATE_INVALID;
			ret = -EINVAL;

			break;
		}

		in_param.ret_code = MFC_OK;
		ret = 0;
		for (i = 0; i < dev->mem_ports; i++)
			ret += mfc_mem_data_size(i);

		break;

#if defined(CONFIG_VIDEO_MFC_VCM_UMP)
	case IOCTL_MFC_SET_IN_BUF:
		if (in_param.args.mem_alloc.type == ENCODER) {
			buf_arg.secure_id = in_param.args.mem_alloc.secure_id;
			buf_arg.align = ALIGN_2KB;
			port = 1;
			ret = mfc_vcm_bind_from_others(mfc_ctx, &buf_arg, MBT_OTHER | port);
		}
		else {
		in_param.args.real_addr.addr =
			mfc_ump_get_virt(in_param.args.real_addr.key);

		mfc_dbg("real addr: 0x%08x", in_param.args.real_addr.addr);

		if (in_param.args.real_addr.addr)
			in_param.ret_code = MFC_OK;
		else
			in_param.ret_code = MFC_MEM_INVALID_ADDR_FAIL;

		ret = in_param.ret_code;
		}

		break;
#endif

	case IOCTL_MFC_SET_CONFIG:
		/* FIXME: mfc_chk_inst_state*/
		/* RMVME: need locking ? */
		mutex_lock(&dev->lock);

		/* in_param.ret_code = mfc_set_config(mfc_ctx, &(in_param.args)); */
		set_cnf_arg = (struct mfc_set_config_arg *)&in_param.args;

		in_param.ret_code = mfc_set_inst_cfg(mfc_ctx, set_cnf_arg->in_config_param, \
			set_cnf_arg->in_config_value);
		ret = in_param.ret_code;

		mutex_unlock(&dev->lock);
		break;

	case IOCTL_MFC_GET_CONFIG:
		/* FIXME: */
		/* FIXME: mfc_chk_inst_state */
		/* RMVME: need locking ? */

		in_param.ret_code = MFC_OK;
		ret = MFC_OK;
		break;

	case IOCTL_MFC_SET_BUF_CACHE:
		mfc_ctx->buf_cache_type = in_param.args.mem_alloc.buf_cache_type;
		in_param.ret_code = MFC_OK;
		break;

	default:
		mfc_err("failed to execute ioctl cmd: 0x%08x\n", cmd);

		in_param.ret_code = MFC_INVALID_PARAM_FAIL;
		ret = -EINVAL;
	}

out_ioctl:
	ex_ret = copy_to_user((struct mfc_common_args *)arg,
			&in_param,
			sizeof(struct mfc_common_args));
	if (ex_ret < 0) {
		mfc_err("Outparm copy to user error\n");
		ret = -EIO;
	}

	mfc_dbg("return = %d\n", ret);

	return ret;
}

static void mfc_vm_open(struct vm_area_struct *vma)
{
	/* FIXME:
	struct mfc_inst_ctx *mfc_ctx = (struct mfc_inst_ctx *)vma->vm_private_data;

	mfc_dbg("id: %d\n", mfc_ctx->id);
	*/

	/* FIXME: atomic_inc(mapped count) */
}

static void mfc_vm_close(struct vm_area_struct *vma)
{
	/* FIXME:
	struct mfc_inst_ctx *mfc_ctx = (struct mfc_inst_ctx *)vma->vm_private_data;

	mfc_dbg("id: %d\n", mfc_ctx->id);
	*/

	/* FIXME: atomic_dec(mapped count) */
}

static int mfc_vm_fault(struct vm_area_struct *vma, struct vm_fault *vmf)
{

	/* FIXME:
	struct mfc_inst_ctx *mfc_ctx = (struct mfc_inst_ctx *)vma->vm_private_data;
	struct page *pg = NULL;

	mfc_dbg("id: %d, pgoff: 0x%08lx, user: 0x%08lx\n",
		mfc_ctx->id, vmf->pgoff, (unsigned long)(vmf->virtual_address));

	if (mfc_ctx == NULL)
		return VM_FAULT_SIGBUS;

	mfc_dbg("addr: 0x%08lx\n",
		(unsigned long)(_mfc_get_buf_addr(mfc_ctx->id, vmf->virtual_address)));

	pg = vmalloc_to_page(_mfc_get_buf_addr(mfc_ctx->id, vmf->virtual_address));

	if (!pg)
		return VM_FAULT_SIGBUS;

	vmf->page = pg;
	*/

	return 0;
}

static const struct vm_operations_struct mfc_vm_ops = {
	.open	= mfc_vm_open,
	.close	= mfc_vm_close,
	.fault	= mfc_vm_fault,
};

static int mfc_mmap(struct file *file, struct vm_area_struct *vma)
{
	unsigned long user_size = vma->vm_end - vma->vm_start;
	unsigned long real_size;
	struct mfc_inst_ctx *mfc_ctx;
#if !(defined(CONFIG_VIDEO_MFC_VCM_UMP) || defined(CONFIG_S5P_VMEM))
	/* mmap support */
	unsigned long pfn;
	unsigned long remap_offset, remap_size;
	struct mfc_dev *dev;
#ifdef SYSMMU_MFC_ON
	/* kernel virtual memory allocator */
	char *ptr;
	unsigned long start, size;
#endif
#endif
	mfc_ctx = (struct mfc_inst_ctx *)file->private_data;
	if (!mfc_ctx)
		return -EINVAL;

#if !(defined(CONFIG_VIDEO_MFC_VCM_UMP) || defined(CONFIG_S5P_VMEM))
	dev = mfc_ctx->dev;
#endif

	mfc_dbg("vm_start: 0x%08lx, vm_end: 0x%08lx, size: %ld(%ldMB)\n",
		vma->vm_start, vma->vm_end, user_size, (user_size >> 20));

	real_size = (unsigned long)(mfc_mem_data_size(0) + mfc_mem_data_size(1));

	mfc_dbg("port 0 size: %d, port 1 size: %d, total: %ld\n",
		mfc_mem_data_size(0),
		mfc_mem_data_size(1),
		real_size);

	/*
	 * if memory size required from appl. mmap() is bigger than max data memory
	 * size allocated in the driver.
	 */
	if (user_size > real_size) {
		mfc_err("user requeste mem(%ld) is bigger than available mem(%ld)\n",
			user_size, real_size);
		return -EINVAL;
	}
#ifdef SYSMMU_MFC_ON
#if (defined(CONFIG_VIDEO_MFC_VCM_UMP) || defined(CONFIG_S5P_VMEM))
	vma->vm_flags |= VM_RESERVED | VM_IO;
	vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
	vma->vm_ops = &mfc_vm_ops;
	vma->vm_private_data = mfc_ctx;

	mfc_ctx->userbase = vma->vm_start;
#else	/* not CONFIG_VIDEO_MFC_VCM_UMP && not CONFIG_S5P_VMEM */
	/* kernel virtual memory allocator */
	if (dev->mem_ports == 1) {
		remap_offset = 0;
		remap_size = user_size;

		vma->vm_flags |= VM_RESERVED | VM_IO;
		vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);

		/*
		 * Port 0 mapping for stream buf & frame buf (chroma + MV + luma)
		 */
		ptr = (char *)mfc_mem_data_base(0);
		start = remap_offset;
		size = remap_size;
		while (size > 0) {
			pfn = vmalloc_to_pfn(ptr);
			if (remap_pfn_range(vma, vma->vm_start + start, pfn,
				PAGE_SIZE, vma->vm_page_prot)) {

				mfc_err("failed to remap port 0\n");
				return -EAGAIN;
			}

			start += PAGE_SIZE;
			ptr += PAGE_SIZE;
			size -= PAGE_SIZE;
		}
	} else {
		remap_offset = 0;
		remap_size = min((unsigned long)mfc_mem_data_size(0), user_size);

		vma->vm_flags |= VM_RESERVED | VM_IO;
		vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);

		/*
		 * Port 0 mapping for stream buf & frame buf (chroma + MV)
		 */
		ptr = (char *)mfc_mem_data_base(0);
		start = remap_offset;
		size = remap_size;
		while (size > 0) {
			pfn = vmalloc_to_pfn(ptr);
			if (remap_pfn_range(vma, vma->vm_start + start, pfn,
				PAGE_SIZE, vma->vm_page_prot)) {

				mfc_err("failed to remap port 0\n");
				return -EAGAIN;
			}

			start += PAGE_SIZE;
			ptr += PAGE_SIZE;
			size -= PAGE_SIZE;
		}

		remap_offset = remap_size;
		remap_size = min((unsigned long)mfc_mem_data_size(1),
			user_size - remap_offset);

		vma->vm_flags |= VM_RESERVED | VM_IO;
		vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);

		/*
		 * Port 1 mapping for frame buf (luma)
		 */
		ptr = (void *)mfc_mem_data_base(1);
		start = remap_offset;
		size = remap_size;
		while (size > 0) {
			pfn = vmalloc_to_pfn(ptr);
			if (remap_pfn_range(vma, vma->vm_start + start, pfn,
				PAGE_SIZE, vma->vm_page_prot)) {

				mfc_err("failed to remap port 1\n");
				return -EAGAIN;
			}

			start += PAGE_SIZE;
			ptr += PAGE_SIZE;
			size -= PAGE_SIZE;
		}
	}

	mfc_ctx->userbase = vma->vm_start;

	mfc_dbg("user request mem = %ld, available data mem = %ld\n",
		  user_size, real_size);

	if ((remap_offset + remap_size) < real_size)
		mfc_warn("The MFC reserved memory dose not mmap fully [%ld: %ld]\n",
		  real_size, (remap_offset + remap_size));
#endif	/* end of CONFIG_VIDEO_MFC_VCM_UMP */
#else	/* not SYSMMU_MFC_ON */
	/* early allocator */
	/* CMA or bootmem(memblock) */
	if (dev->mem_ports == 1) {
		remap_offset = 0;
		remap_size = user_size;

		vma->vm_flags |= VM_RESERVED | VM_IO;

		if(mfc_ctx->buf_cache_type == NO_CACHE){
			vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
			mfc_info("CONFIG_VIDEO_MFC_CACHE is not enabled\n");
		}else
			mfc_info("CONFIG_VIDEO_MFC_CACHE is enabled\n");


		/*
		 * Port 0 mapping for stream buf & frame buf (chroma + MV + luma)
		 */
		pfn = __phys_to_pfn(mfc_mem_data_base(0));
		if (remap_pfn_range(vma, vma->vm_start + remap_offset, pfn,
			remap_size, vma->vm_page_prot)) {

			mfc_err("failed to remap port 0\n");
			return -EINVAL;
		}
	} else {
		remap_offset = 0;
		remap_size = min((unsigned long)mfc_mem_data_size(0), user_size);

		vma->vm_flags |= VM_RESERVED | VM_IO;

		if(mfc_ctx->buf_cache_type == NO_CACHE){
			vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
			mfc_info("CONFIG_VIDEO_MFC_CACHE is not enabled\n");
		}else
			mfc_info("CONFIG_VIDEO_MFC_CACHE is enabled\n");



		/*
		 * Port 0 mapping for stream buf & frame buf (chroma + MV)
		 */
		pfn = __phys_to_pfn(mfc_mem_data_base(0));
		if (remap_pfn_range(vma, vma->vm_start + remap_offset, pfn,
			remap_size, vma->vm_page_prot)) {

			mfc_err("failed to remap port 0\n");
			return -EINVAL;
		}

		remap_offset = remap_size;
		remap_size = min((unsigned long)mfc_mem_data_size(1),
			user_size - remap_offset);

		vma->vm_flags |= VM_RESERVED | VM_IO;

		if(mfc_ctx->buf_cache_type == NO_CACHE)
			vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);


		/*
		 * Port 1 mapping for frame buf (luma)
		 */
		pfn = __phys_to_pfn(mfc_mem_data_base(1));
		if (remap_pfn_range(vma, vma->vm_start + remap_offset, pfn,
			remap_size, vma->vm_page_prot)) {

			mfc_err("failed to remap port 1\n");
			return -EINVAL;
		}
	}

	mfc_ctx->userbase = vma->vm_start;

	mfc_dbg("user request mem = %ld, available data mem = %ld\n",
		  user_size, real_size);

	if ((remap_offset + remap_size) < real_size)
		mfc_warn("The MFC reserved memory dose not mmap fully [%ld: %ld]\n",
		  real_size, (remap_offset + remap_size));
#endif	/* end of SYSMMU_MFC_ON */
	return 0;
}

static const struct file_operations mfc_fops = {
	.owner		= THIS_MODULE,
	.open		= mfc_open,
	.release	= mfc_release,
	.unlocked_ioctl	= mfc_ioctl,
	.mmap		= mfc_mmap,
};

static struct miscdevice mfc_miscdev = {
	.minor	= MFC_MINOR,
	.name	= MFC_DEV_NAME,
	.fops	= &mfc_fops,
};

static void mfc_firmware_request_complete_handler(const struct firmware *fw,
						  void *context)
{
	if (fw != NULL) {
//		mfcdev->fw.state = mfc_load_firmware(fw->data, fw->size);
//		mfc_info("MFC F/W loaded successfully (size: %d)\n", fw->size);

		mfcdev->fw.info = fw;
	} else {
		mfc_info("failed to load MFC F/W, MFC will not working\n");
	}
}

/* FIXME: check every exception case (goto) */
static int __devinit mfc_probe(struct platform_device *pdev)
{
	struct resource *res;
	int ret;

	mfcdev = kzalloc(sizeof(struct mfc_dev), GFP_KERNEL);
	if (unlikely(mfcdev == NULL)) {
		dev_err(&pdev->dev, "failed to allocate control memory\n");
		return -ENOMEM;
	}

	/* init. control structure */
	sprintf(mfcdev->name, "%s", MFC_DEV_NAME);

	mutex_init(&mfcdev->lock);
	init_waitqueue_head(&mfcdev->wait_sys);
	init_waitqueue_head(&mfcdev->wait_codec[0]);
	init_waitqueue_head(&mfcdev->wait_codec[1]);
	atomic_set(&mfcdev->inst_cnt, 0);
#ifdef CONFIG_CPU_FREQ
	atomic_set(&mfcdev->busfreq_lock_cnt, 0);
#endif
	mfcdev->device = &pdev->dev;

	platform_set_drvdata(pdev, mfcdev);

	/* get the memory region */
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (unlikely(res == NULL)) {
		dev_err(&pdev->dev, "no memory resource specified\n");
		ret = -ENOENT;
		goto err_mem_res;
	}

	mfcdev->reg.rsrc_start = res->start;
	mfcdev->reg.rsrc_len = resource_size(res);

	/* request mem region for MFC register (0x0000 ~ 0xE008) */
	/*res = request_mem_region(mfcdev->reg.rsrc_start,
			mfcdev->reg.rsrc_len, pdev->name);
	if (unlikely(res == NULL)) {
		dev_err(&pdev->dev, "failed to get memory region\n");
		ret = -ENOENT;
		goto err_mem_req;
	}*/

	/* ioremap for MFC register */
	mfcdev->reg.base = ioremap(mfcdev->reg.rsrc_start, mfcdev->reg.rsrc_len);

	if (unlikely(!mfcdev->reg.base)) {
		dev_err(&pdev->dev, "failed to ioremap memory region\n");
		ret = -EINVAL;
		goto err_mem_map;
	}

	init_reg(mfcdev->reg.base);

	mfcdev->irq = platform_get_irq(pdev, 0);
	if (unlikely(mfcdev->irq < 0)) {
		dev_err(&pdev->dev, "no irq resource specified\n");
		ret = -ENOENT;
		goto err_irq_res;
	}

	ret = request_irq(mfcdev->irq, mfc_irq, IRQF_DISABLED, mfcdev->name, mfcdev);
	if (ret) {
		dev_err(&pdev->dev, "failed to allocate irq (%d)\n", ret);
		goto err_irq_req;
	}


#ifdef	 _BUS_TUNE_
	/*MFC Bus Tunning*/
	base50060= ioremap(0x50060000, 0x1000);
	base50078= ioremap(0x50078000, 0x1000);
	base50000= ioremap(0x50000000, 0x1000);
	base5e001= ioremap(0x5e001000, 0x1000);
	base5e002= ioremap(0x5e002000, 0x1000);
	base5e003= ioremap(0x5e003000, 0x1000);
	base5e004= ioremap(0x5e004000, 0x1000);
#endif

	/*
	 * initialize PM(power, clock) interface
	 */
	ret = mfc_init_pm(mfcdev);
	if (ret < 0) {
		printk(KERN_ERR "failed to init. MFC PM interface\n");
		goto err_pm_if;
	}

	/*
	 * initialize memory manager
	 */

//	ret = mfc_init_mem_mgr(mfcdev);
//	if (ret < 0) {
//		mfc_err("failed to init. MFC memory manager\n");
//		goto err_mem_mgr;
//	}
//
//	mfc_init_buf();

	/*
	 * loading firmware
	 */
	ret = request_firmware_nowait(THIS_MODULE,
				      FW_ACTION_HOTPLUG,
				      MFC_FW_NAME,
				      &pdev->dev,
				      GFP_KERNEL,
				      pdev,
				      mfc_firmware_request_complete_handler);
	if (ret) {
		dev_err(&pdev->dev, "could not load firmware (err=%d)\n", ret);
		goto err_fw_req;
	}

#if defined(SYSMMU_MFC_ON) && defined(CONFIG_VIDEO_MFC_VCM_UMP)
	ret = vcm_activate(mfcdev->vcm_info.sysmmu_vcm);
	if (ret < 0) {
		mfc_err("failed to activate VCM: %d", ret);

		goto err_act_vcm;
	}
#endif

	/*
	 * initialize buffer manager
	 */
	/* mfc_init_buf(); */

	/* FIXME: final dec & enc */
	mfc_init_decoders();
	mfc_init_encoders();

	ret = misc_register(&mfc_miscdev);

	if (ret) {
		mfc_err("MFC can't misc register on minor=%d\n", MFC_MINOR);
		goto err_misc_reg;
	}

	mfc_info("MFC(Multi Function Codec - FIMV v5.x) registered successfully\n");

	return 0;

err_misc_reg:
	mfc_final_buf();

#ifdef SYSMMU_MFC_ON
#ifdef CONFIG_VIDEO_MFC_VCM_UMP
	mfc_clock_on();

	vcm_deactivate(mfcdev->vcm_info.sysmmu_vcm);

	mfc_clock_off();

err_act_vcm:
#endif
	mfc_clock_on();

	sysmmu_off(SYSMMU_MFC_L);
	sysmmu_off(SYSMMU_MFC_R);

	mfc_clock_off();
#endif
	if (mfcdev->fw.info)
		release_firmware(mfcdev->fw.info);

err_fw_req:
	/* FIXME: make kenel dump when probe fail */
	mfc_clock_on();

	mfc_final_mem_mgr(mfcdev);

	mfc_clock_off();

err_mem_mgr:
	mfc_final_pm(mfcdev);

err_pm_if:
	free_irq(mfcdev->irq, mfcdev);

err_irq_req:
err_irq_res:
	iounmap(mfcdev->reg.base);

err_mem_map:
	release_mem_region(mfcdev->reg.rsrc_start, mfcdev->reg.rsrc_len);

err_mem_req:
err_mem_res:
	platform_set_drvdata(pdev, NULL);
	mutex_destroy(&mfcdev->lock);
	kfree(mfcdev);

	return ret;
}

/* FIXME: check mfc_remove funtionalilty */
static int __devexit mfc_remove(struct platform_device *pdev)
{
	struct mfc_dev *dev = platform_get_drvdata(pdev);

	/* FIXME: close all instance? or check active instance? */

	misc_deregister(&mfc_miscdev);

	mfc_final_buf();
#ifdef SYSMMU_MFC_ON
	mfc_clock_on();

#ifdef CONFIG_VIDEO_MFC_VCM_UMP
	vcm_deactivate(mfcdev->vcm_info.sysmmu_vcm);
#endif

	sysmmu_off(SYSMMU_MFC_L);
	sysmmu_off(SYSMMU_MFC_R);

	mfc_clock_off();
#endif
	if (dev->fw.info)
		release_firmware(dev->fw.info);
	mfc_final_mem_mgr(dev);
	mfc_final_pm(dev);
	free_irq(dev->irq, dev);
	iounmap(dev->reg.base);
	release_mem_region(dev->reg.rsrc_start, dev->reg.rsrc_len);
	platform_set_drvdata(pdev, NULL);
	mutex_destroy(&dev->lock);
	kfree(dev);

	return 0;
}



#ifdef CONFIG_PM
static int mfc_suspend(struct device *dev)
{
	struct mfc_dev *m_dev = platform_get_drvdata(to_platform_device(dev));
	int ret;

	if (m_dev == NULL) {
		mfc_err("m_dev is NULL pointer\n");
		return -EINVAL;
	}

	if (atomic_read(&m_dev->inst_cnt) == 0) {
		mfc_bus_disable();
		return 0;
	}

	mutex_lock(&m_dev->lock);

	ret = mfc_sleep(m_dev);

	mfc_bus_disable();

	mutex_unlock(&m_dev->lock);

	mfc_clock_off();

	if (ret != MFC_OK)
		return ret;

	return 0;
}

static int mfc_resume(struct device *dev)
{
	struct mfc_dev *m_dev = platform_get_drvdata(to_platform_device(dev));
	int ret;
	u32 timeout;

	if (m_dev == NULL) {
		mfc_err("m_dev is NULL pointer\n");
		return -EINVAL;
	}
	if (atomic_read(&m_dev->inst_cnt) == 0)
		return 0;

	mutex_lock(&m_dev->lock);

	ret = mfc_wakeup(m_dev);

	mutex_unlock(&m_dev->lock);

	mfc_clock_on();

	if (ret != MFC_OK)
		return ret;
	return 0;
}

#ifdef CONFIG_PM_RUNTIME
static int mfc_runtime_suspend(struct device *dev)
{
	struct mfc_dev *m_dev = platform_get_drvdata(to_platform_device(dev));

	atomic_set(&m_dev->pm.power, 0);

	return 0;
}

static int mfc_runtime_idle(struct device *dev)
{
	return 0;
}

static int mfc_runtime_resume(struct device *dev)
{
	struct mfc_dev *m_dev = platform_get_drvdata(to_platform_device(dev));
	int pre_power;

	pre_power = atomic_read(&m_dev->pm.power);
	atomic_set(&m_dev->pm.power, 1);

#ifdef SYSMMU_MFC_ON
	if (pre_power == 0) {
		mfc_clock_on();

		sysmmu_on(SYSMMU_MFC_L);
		sysmmu_on(SYSMMU_MFC_R);

#ifdef CONFIG_VIDEO_MFC_VCM_UMP
		vcm_set_pgtable_base(VCM_DEV_MFC);
#else /* CONFIG_S5P_VMEM or kernel virtual memory allocator */
		sysmmu_set_tablebase_pgd(SYSMMU_MFC_L, __pa(swapper_pg_dir));
		sysmmu_set_tablebase_pgd(SYSMMU_MFC_R, __pa(swapper_pg_dir));
#endif

		mfc_clock_off();
	}
#endif

	return 0;
}
#endif

#else
#define mfc_suspend NULL
#define mfc_resume NULL
#ifdef CONFIG_PM_RUNTIME
#define mfc_runtime_idle	NULL
#define mfc_runtime_suspend	NULL
#define mfc_runtime_resume	NULL
#endif
#endif

static const struct dev_pm_ops mfc_pm_ops = {
	.suspend		= mfc_suspend,
	.resume			= mfc_resume,
#ifdef CONFIG_PM_RUNTIME
	.runtime_idle		= mfc_runtime_idle,
	.runtime_suspend	= mfc_runtime_suspend,
	.runtime_resume		= mfc_runtime_resume,
#endif
};

static struct platform_driver mfc_driver = {
	.probe		= mfc_probe,
	.remove		= __devexit_p(mfc_remove),
	.driver		= {
		.owner	= THIS_MODULE,
		.name	= MFC_DEV_NAME,
		.pm	= &mfc_pm_ops,
	},
};

static int __init mfc_init(void)
{
	if (platform_driver_register(&mfc_driver) != 0) {
		printk(KERN_ERR "FIMV MFC platform device registration failed..\n");
		return -1;
	}

	return 0;
}

static void __exit mfc_exit(void)
{
	platform_driver_unregister(&mfc_driver);
	mfc_info("FIMV MFC(Multi Function Codec) V5.x exit.\n");
}

#ifndef CONFIG_SCORE_FAST_RESUME
module_init(mfc_init);
#else
fast_dev_initcall(mfc_init);
#endif
module_exit(mfc_exit);

MODULE_AUTHOR("Jeongtae, Park");
MODULE_AUTHOR("Jaeryul, Oh");
MODULE_DESCRIPTION("FIMV MFC(Multi Function Codec) V5.x Device Driver");
MODULE_LICENSE("GPL");
