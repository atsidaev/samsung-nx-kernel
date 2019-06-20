/*
 * linux/drivers/media/video/samsung/mfc5x/mfc_cmd.c
 *
 * Copyright (c) 2010 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com/
 *
 * Command interface for Samsung MFC (Multi Function Codec - FIMV) driver
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/jiffies.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <asm/io.h>

#if defined(CONFIG_CPU_DRIME4)
#include <mach/regs-mfc.h>
#include <mach/regs-codec_fb.h>
#elif defined(CONFIG_ARCH_S5PV210)
#include <plat/regs-mfc.h>
#endif

#include "mfc_cmd.h"
#include "mfc_reg.h"
#include "mfc_log.h"
#include "mfc_dec.h"
#include "mfc_enc.h"
#include "mfc_mem.h"
#include "mfc_buf.h"
#include "mfc_dev.h"

static unsigned int r2h_cmd;
static unsigned int fb_intr_stat;
static struct mfc_cmd_args r2h_args;

#undef MFC_PERF

#ifdef MFC_PERF
static int framecnt = 0;
#endif

#ifdef CONFIG_VIDEO_MFC5X_MEASURE_CODING_TIME
static ktime_t tv5, tv6;
static u32 time_start, time_end;
#endif

#ifdef _MFC_DEBUG_

#define	MAX_MFC_DBG_SIZE	10000
static unsigned int	usec1, usec2, usec3;
static char	g_dbg_mfc_buf[MAX_MFC_DBG_SIZE][32];
static int		g_dbg_mfc_count =0;
static int		g_dbg_total_mfc_count = 0;
static unsigned int sum_t2_t1, sum_t3_t1, total_t2_t1, total_t3_t1;


unsigned int get_utime(void)
{
	unsigned int	usec;
	struct timeval tv;
	do_gettimeofday(&tv);
	
	usec = tv.tv_sec*1000000+tv.tv_usec;
	return usec;
}

void mfc_init_debug(void)
{
	g_dbg_mfc_count =0;
	g_dbg_total_mfc_count = 0;
	sum_t2_t1 = 0;
	sum_t3_t1 = 0;
	total_t2_t1 = 0;
	total_t3_t1 = 0;
}

void mfc_record_duration(unsigned int t1, unsigned int t2, unsigned int t3)
{
#ifdef _MFC_DEBUG_SYNC_PRINT_
	sum_t2_t1 += (t2-t1);
	total_t2_t1 += (t2-t1);
	sum_t3_t1 += (t3-t1);
	total_t3_t1 += (t3-t1);
	g_dbg_mfc_count++;
	g_dbg_total_mfc_count++;
	if(g_dbg_mfc_count%60  == 0 ){
		printk(/*KERN_ERR, */"ISR aver : %d usec,  DD Aver : %d usec, frame count : %d\n", (sum_t2_t1)/g_dbg_mfc_count,  (sum_t3_t1)/g_dbg_mfc_count, g_dbg_mfc_count);
		printk(/*KERN_ERR, */"[Total]ISR aver : %d usec,  DD Aver : %d usec, frame count : %d\n", (total_t2_t1)/g_dbg_total_mfc_count,  (total_t3_t1)/g_dbg_total_mfc_count, g_dbg_total_mfc_count);
		sum_t2_t1 = 0;
		sum_t3_t1 = 0;
		g_dbg_mfc_count = 0;
//mfc_init_debug();
	}
#else

  #ifdef _MFC_DEBUG_DIRECT_PRINT_
		printk("|%d|%d|\n", (t2-t1), (t3-t1));
  #else
	if(g_dbg_mfc_count < MAX_MFC_DBG_SIZE){
		sprintf(g_dbg_mfc_buf[g_dbg_mfc_count], "|%d|%d|", (t2-t1), (t3-t1));
		g_dbg_mfc_count++;
	}
  #endif
#endif
}

void mfc_Print_Debug_Info(void)
{
	int i;
#ifdef _MFC_DEBUG_SYNC_PRINT_

#else
	for(i=0;i<g_dbg_mfc_count;i++){
		printk(/*KERN_ERR,*/ "%s\n",g_dbg_mfc_buf[i]);
	}
#endif
	
}

#endif


irqreturn_t mfc_irq(int irq, void *dev_id)
{
	struct mfc_dev *dev = (struct mfc_dev *)dev_id;
	/* FIXME: there are multiple instances */
	struct mfc_inst_ctx *ctx = dev->inst_ctx[0];

	r2h_cmd = read_reg(MFC_RISC2HOST_CMD);
	fb_intr_stat = read_reg(DRIME4_FB_INTR_STAT);
	mfc_dbg("MFC IRQ: 0x%02x\n", r2h_cmd);

	/* T2L_DONE_RET is not defined in MFC h/w */

	if (((r2h_cmd >= OPEN_CH_RET) && (r2h_cmd <= CLOSE_CH_RET)) ||
	    ((r2h_cmd >= SEQ_DONE_RET) && (r2h_cmd <= EDFU_INIT_RET)) ||
	    ( r2h_cmd == ERR_RET)) {
		memset(&r2h_args, 0, sizeof(struct mfc_cmd_args));

		r2h_args.arg[0] = read_reg(MFC_RISC2HOST_ARG1);
		r2h_args.arg[1] = read_reg(MFC_RISC2HOST_ARG2);
		r2h_args.arg[2] = read_reg(MFC_RISC2HOST_ARG3);
		r2h_args.arg[3] = read_reg(MFC_RISC2HOST_ARG4);

		if (r2h_cmd == ERR_RET)
			//mfc_dbg("F/W error code: %d", r2h_args.arg[1] & 0xFFFF);
			mfc_dbg("F/W error code: 0x%08x", r2h_args.arg[1]);
		if (r2h_cmd == FRAME_DONE_RET) {
#ifdef CONFIG_VIDEO_MFC5X_MEASURE_CODING_TIME
			tv6 = ktime_get();
			time_start = (u32)ktime_to_us(tv5);
			time_end = (u32)ktime_to_us(tv6);
			printk("it takes %uus to encode one frame\n",
				(time_end - time_start) );
#if 0
			if (ctx->job_status == MFC_ENCODING) {
				printk("it takes %uus to encode one frame\n",
					(time_end - time_start) );
			} else {
				printk("it takes %uus to decode one frame\n",
					(time_end - time_start) );
			}
#endif
#endif /* CONFIG_VIDEO_MFC5X_MEASURE_CODING_TIME */
		}
	/* FB interrupt or T2L interrupt occurred,
	 * no driver are waiting for the FB interrupt
	 */
	} else if ((fb_intr_stat & 0x1) || (fb_intr_stat & 0x2)) {
		mfc_dbg("FB intr occured: 0x%02x\n", fb_intr_stat);
		/* clear fb intr */
		WRITEL(0, DRIME4_FB_INTR_CLR);
		if (ctx && ctx->fb_state == FB_OFF) {
			dev->irq_sys = 1;
			wake_up(&dev->wait_sys);
		}
		return IRQ_HANDLED;
	} else {
		printk("[MFC_ERR]Invalid interrupt!\n");
		write_reg(0, MFC_RISC_HOST_INT);
		return IRQ_HANDLED;
	}


#ifdef MFC_PERF
	if (framecnt > 0) {
		do_gettimeofday(&tv2);

		mfc_info("%d, %ld", framecnt,
			(long)(((tv2.tv_sec * 1000000) + tv2.tv_usec) \
				- ((tv1.tv_sec * 1000000) + tv1.tv_usec)));

		framecnt++;
	}
#endif
#ifdef _MFC_DEBUG_
	usec2 = get_utime();
#endif

	/* FIXME: codec wait_queue processing */
	dev->irq_sys = 1;
	wake_up(&dev->wait_sys);

	/*
	 * FIXME: check is codec command return or error
	 * move to mfc_wait_codec() ?
	 */
	write_reg(0xFFFF, MFC_SI_RTN_CHID);

	write_reg(0, MFC_RISC2HOST_CMD);
	write_reg(0, MFC_RISC_HOST_INT);

	return IRQ_HANDLED;
}

#if 0
static bool mfc_wait_codec(struct mfc_inst_ctx *ctx, enum mfc_r2h_ret ret)
{
	/*
	if (wait_event_timeout(dev->wait_codec[0], 0, timeout) == 0) {
		mfc_err("F/W timeout: 0x%02x\n", ret);

		return false;
	}

	if (r2h_cmd == ERR_RET)
		mfc_err("F/W error code: 0x%02x", r2h_args.arg[1] & 0xFFFF);

		return false;
	}

	if (r2h_cmd != ret) {
		mfc_err("F/W return (0x%02x) waiting for (0x%02x)\n",
			r2h_cmd, ret);

		return false;
	}
	*/
	return true;
}
#endif

static bool
mfc_wait_sys(struct mfc_dev *dev, enum mfc_r2h_ret ret, long timeout)
{

	if (wait_event_timeout(dev->wait_sys, dev->irq_sys, timeout) == 0) {
		mfc_err("F/W timeout: 0x%02x\n", ret);
		dev->irq_sys = 0;
		return false;
	}

	dev->irq_sys = 0;
	if (r2h_cmd == ERR_RET) {
		mfc_err("F/W error code: 0x%02x", r2h_args.arg[1] & 0xFFFF);

		return false;
	}

	/* CAUTION: r2h_cmd is global variable
	 * FB intr can often occur in this region
	 * The order of occurrence for two intr is random
	 * It is possible for IRQ handler to change the r2h_cmd value
	 */

	/*
	if (r2h_cmd != ret) {
		mfc_err("F/W return (0x%02x) waiting for (0x%02x)\n",
			r2h_cmd, ret);

		return false;
	}
	*/

	return true;
}

int mfc_fb_convert_4x2tile2linear(struct mfc_inst_ctx *ctx, u32 src_y, \
	u32 src_c, u32 dst_y, u32 dst_c)
{
	u32 height;
	u32 width;
	u32 reg;
	height = (((ctx->height + 15) >> 4) << 4);
	width = (((ctx->width + 15) >> 4) << 4);

	/* luma processing */
	write_reg(src_y, DRIME4_FB_TILE_SRC_BASE);
	write_reg(dst_y, DRIME4_FB_TILE_DST_BASE);
	write_reg((width << 11) | height, DRIME4_FB_TILE_WIDTH_HEIGHT);

	/* start command */
	reg = read_reg(DRIME4_FB_CODEC_TOP_CTRL);
	reg |= DRIME4_FB_CTRL_TILE_START;
	write_reg(reg, DRIME4_FB_CODEC_TOP_CTRL);

	/* check for start */
	reg = read_reg(DRIME4_FB_CODEC_TOP_CTRL);
	if (reg &= DRIME4_FB_CTRL_TILE_START)
		mfc_dbg("tile2linear converter started\n");
	else
		mfc_dbg("tile2linear converter fail to start\n");

	/* clear start command */
	reg = read_reg(DRIME4_FB_CODEC_TOP_CTRL);
	reg &= ~DRIME4_FB_CTRL_TILE_START;
	write_reg(reg, DRIME4_FB_CODEC_TOP_CTRL);

	/* wait for interrupt */

	if (mfc_wait_sys(ctx->dev, T2L_DONE_RET, \
		msecs_to_jiffies(H2R_INT_TIMEOUT)) == false) {
		mfc_err("failed to convert t2r\n");
		return -1;
	}

	/* chroma processing */
	write_reg(src_c, DRIME4_FB_TILE_SRC_BASE);
	write_reg(dst_c, DRIME4_FB_TILE_DST_BASE);
	write_reg((width << 11) | (height/2), DRIME4_FB_TILE_WIDTH_HEIGHT);

	/* start command */
	reg = read_reg(DRIME4_FB_CODEC_TOP_CTRL);
	reg |= DRIME4_FB_CTRL_TILE_START;
	write_reg(reg, DRIME4_FB_CODEC_TOP_CTRL);

	/* check for start */
	reg = read_reg(DRIME4_FB_CODEC_TOP_CTRL);
	if (reg &= DRIME4_FB_CTRL_TILE_START)
		mfc_dbg("tile2linear converter started\n");
	else
		mfc_dbg("tile2linear converter fail to start\n");

	/* clear start command */
	reg = read_reg(DRIME4_FB_CODEC_TOP_CTRL);
	reg &= ~DRIME4_FB_CTRL_TILE_START;
	write_reg(reg, DRIME4_FB_CODEC_TOP_CTRL);

	/* wait for interrupt */

	if (mfc_wait_sys(ctx->dev, T2L_DONE_RET, \
		msecs_to_jiffies(H2R_INT_TIMEOUT)) == false) {
		mfc_err("failed to convert t2r\n");
		return -1;
	}

	return 0;
}

static bool write_h2r_cmd(enum mfc_h2r_cmd cmd, struct mfc_cmd_args *args)
{
	enum mfc_h2r_cmd pending_cmd;
	unsigned long timeo = jiffies;

	timeo += msecs_to_jiffies(H2R_CMD_TIMEOUT);

	/* wait until host to risc command register becomes 'NOP' */
	do {
		pending_cmd = read_reg(MFC_HOST2RISC_CMD);

		if (pending_cmd == H2R_NOP)
			break;

		schedule_timeout_uninterruptible(1);
		/* FiXME: cpu_relax() */
	} while (time_before(jiffies, timeo));

	if (pending_cmd != H2R_NOP)
		return false;

	write_reg(args->arg[0], MFC_HOST2RISC_ARG1);
	write_reg(args->arg[1], MFC_HOST2RISC_ARG2);
	write_reg(args->arg[2], MFC_HOST2RISC_ARG3);
	write_reg(args->arg[3], MFC_HOST2RISC_ARG4);

	write_reg(cmd, MFC_HOST2RISC_CMD);

	return true;
}

int mfc_cmd_fw_start(struct mfc_dev *dev)
{
	/* release RISC reset */
	write_reg(0x3FF, MFC_SW_RESET);

	if (mfc_wait_sys(dev, FW_STATUS_RET,
		msecs_to_jiffies(H2R_INT_TIMEOUT)) == false) {
		mfc_err("failed to check F/W\n");
		return MFC_FW_LOAD_FAIL;
	}

	return MFC_OK;
}

int mfc_cmd_sys_init(struct mfc_dev *dev)
{
	struct mfc_cmd_args h2r_args;
	unsigned int fw_version, fw_memsize;

	memset(&h2r_args, 0, sizeof(struct mfc_cmd_args));
	h2r_args.arg[0] = MFC_FW_SYSTEM_SIZE;

	if (write_h2r_cmd(SYS_INIT, &h2r_args) == false)
		return MFC_CMD_FAIL;

	if (mfc_wait_sys(dev, SYS_INIT_RET,
		msecs_to_jiffies(H2R_INT_TIMEOUT)) == false) {
		mfc_err("failed to init system\n");
		return MFC_FW_INIT_FAIL;
	}

	fw_version = read_reg(MFC_FW_VERSION);
	fw_memsize = r2h_args.arg[0];

	mfc_info("MFC F/W version: %02x-%02x-%02x, %dkB\n",
		 (fw_version >> 16) & 0xff,
		 (fw_version >> 8) & 0xff,
		 (fw_version) & 0xff,
		 (fw_memsize) >> 10);

	return MFC_OK;
}

int mfc_cmd_sys_sleep(struct mfc_dev *dev)
{
	struct mfc_cmd_args h2r_args;

	memset(&h2r_args, 0, sizeof(struct mfc_cmd_args));

	if (write_h2r_cmd(SLEEP, &h2r_args) == false)
		return MFC_CMD_FAIL;

	if (mfc_wait_sys(dev, SLEEP_RET,
		msecs_to_jiffies(H2R_INT_TIMEOUT)) == false) {
		mfc_err("failed to sleep\n");
		return MFC_SLEEP_FAIL;
	}

	return MFC_OK;
}

int mfc_cmd_sys_wakeup(struct mfc_dev *dev)
{
	struct mfc_cmd_args h2r_args;

	memset(&h2r_args, 0, sizeof(struct mfc_cmd_args));

	if (write_h2r_cmd(WAKEUP, &h2r_args) == false)
		return MFC_CMD_FAIL;

	/* release RISC reset */
	write_reg(0x3FF, MFC_SW_RESET);

	if (mfc_wait_sys(dev, WAKEUP_RET,
		//msecs_to_jiffies(H2R_INT_TIMEOUT)) == false) {
		msecs_to_jiffies(20000)) == false) {
		mfc_err("failed to wakeup\n");
		return MFC_WAKEUP_FAIL;
	}

	return MFC_OK;
}

int mfc_cmd_inst_open(struct mfc_inst_ctx *ctx)
{
	struct mfc_cmd_args h2r_args;
	unsigned int crc = 0;
	unsigned int pixelcache = 0;
	struct mfc_dec_ctx *dec_ctx;
	struct mfc_enc_ctx *enc_ctx;

	if (ctx->type == DECODER) {
		dec_ctx = (struct mfc_dec_ctx *)ctx->c_priv;

		crc = dec_ctx->crc & 0x1;
		pixelcache = dec_ctx->pixelcache & 0x3;
	} else {
		enc_ctx = (struct mfc_enc_ctx *)ctx->c_priv;

		pixelcache = enc_ctx->pixelcache & 0x3;
	}

	memset(&h2r_args, 0, sizeof(struct mfc_cmd_args));
	h2r_args.arg[0] = ctx->codecid;
	h2r_args.arg[1] = crc << 31 | pixelcache;
	h2r_args.arg[2] = ctx->ctxbufofs;
	h2r_args.arg[3] = ctx->ctxbufsize;

	if (write_h2r_cmd(OPEN_CH, &h2r_args) == false)
		return MFC_CMD_FAIL;

	if (mfc_wait_sys(ctx->dev, OPEN_CH_RET,
		msecs_to_jiffies(H2R_INT_TIMEOUT)) == false) {
		mfc_err("failed to open instance\n");
		return MFC_OPEN_FAIL;
	}

	ctx->cmd_id = r2h_args.arg[0];

	mfc_dbg("inst id: %d, cmd id: %d, codec id: %d",
		ctx->id, ctx->cmd_id, ctx->codecid);

#ifdef MFC_PERF
	framecnt = 0;
#endif

	return ctx->cmd_id;
}

int mfc_cmd_inst_close(struct mfc_inst_ctx *ctx)
{
	struct mfc_cmd_args h2r_args;

	memset(&h2r_args, 0, sizeof(struct mfc_cmd_args));
	h2r_args.arg[0] = ctx->cmd_id;

	if (write_h2r_cmd(CLOSE_CH, &h2r_args) == false)
		return MFC_CMD_FAIL;

	if (mfc_wait_sys(ctx->dev, CLOSE_CH_RET,
		msecs_to_jiffies(H2R_INT_TIMEOUT)) == false) {
		mfc_err("failed to close instance\n");
		return MFC_CLOSE_FAIL;
	}

	return MFC_OK;
}

int mfc_cmd_seq_start(struct mfc_inst_ctx *ctx)
{
	/* all codec command pass the shared mem addrees */
	write_reg(ctx->shmofs, MFC_SI_CH1_HOST_WR_ADR);

	write_reg((SEQ_HEADER << 16 & 0x70000) | ctx->cmd_id,
		  MFC_SI_CH1_INST_ID);

	/* FIXME: close_instance ? */
	/* FIXME: mfc_wait_codec */
	if (mfc_wait_sys(ctx->dev, SEQ_DONE_RET,
		msecs_to_jiffies(CODEC_INT_TIMEOUT)) == false) {
		mfc_err("failed to init seq start\n");
		return MFC_DEC_INIT_FAIL;
	}

	return MFC_OK;
}

int mfc_cmd_init_buffers(struct mfc_inst_ctx *ctx)
{
	/* all codec command pass the shared mem addrees */
	write_reg(ctx->shmofs, MFC_SI_CH1_HOST_WR_ADR);

	write_reg((INIT_BUFFERS << 16 & 0x70000) | ctx->cmd_id,
		  MFC_SI_CH1_INST_ID);

	/* FIXME: close_instance ? */
	/* FIXME: mfc_wait_codec */
	if (mfc_wait_sys(ctx->dev, INIT_BUFFERS_RET,
		msecs_to_jiffies(CODEC_INT_TIMEOUT)) == false) {
		mfc_err("failed to init buffers\n");
		return MFC_DEC_INIT_FAIL;
	}

#ifdef MFC_PERF
	framecnt = 1;
#endif

	return MFC_OK;
}

int mfc_cmd_frame_start(struct mfc_inst_ctx *ctx)
{
	struct mfc_dec_ctx *dec_ctx;

	/* all codec command pass the shared mem addrees */
	write_reg(ctx->shmofs, MFC_SI_CH1_HOST_WR_ADR);

	if (ctx->type == DECODER) {
#ifdef CONFIG_VIDEO_MFC5X_MEASURE_CODING_TIME
		tv5 = ktime_get();
#endif
		dec_ctx = (struct mfc_dec_ctx *)ctx->c_priv;

		mfc_dbg("dec_ctx->lastframe: %d", dec_ctx->lastframe);

		if (dec_ctx->lastframe) {
			write_reg((LAST_SEQ << 16 & 0x70000) | ctx->cmd_id,
				  MFC_SI_CH1_INST_ID);
			dec_ctx->lastframe = 0;
		} else if (ctx->resolution_status == RES_SET_CHANGE) {
			mfc_dbg("FRAME_START_REALLOC\n");
			write_reg((FRAME_START_REALLOC << 16 & 0x70000) \
				| ctx->cmd_id,
				  MFC_SI_CH1_INST_ID);
			ctx->resolution_status = RES_WAIT_FRAME_DONE;
		} else {
			write_reg((FRAME_START << 16 & 0x70000) | ctx->cmd_id,
				  MFC_SI_CH1_INST_ID);
		}
	} else { /* == ENCODER */
#ifdef CONFIG_VIDEO_MFC5X_MEASURE_CODING_TIME
		tv5 = ktime_get();
#endif
#ifdef _MFC_DEBUG_
		usec1 = get_utime();
#endif
		write_reg((FRAME_START << 16 & 0x70000) | ctx->cmd_id,
			  MFC_SI_CH1_INST_ID);
	}

#ifdef MFC_PERF
	do_gettimeofday(&tv1);
#endif

	/* FIXME: close_instance ? */
	/* FIXME: mfc_wait_codec */
	if (mfc_wait_sys(ctx->dev, FRAME_DONE_RET,
		msecs_to_jiffies(CODEC_INT_TIMEOUT)) == false) {
		mfc_err("failed to frame start\n");
		return MFC_DEC_EXE_TIME_OUT;
	}

#ifdef _MFC_DEBUG_
	usec3 = get_utime();
	mfc_record_duration(usec1, usec2, usec3);
#endif

	return MFC_OK;
}

