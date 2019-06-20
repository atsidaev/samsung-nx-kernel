/*
 * linux/drivers/media/video/samsung/mfc5x/mfc_inst.h
 *
 * Copyright (c) 2010 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com/
 *
 * Instance manager file for Samsung MFC (Multi Function Codec - FIMV) driver
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __MFC_INST_H
#define __MFC_INST_H __FILE__

#include <linux/list.h>

#include "mfc.h"
//#include "mfc_enc.h"
#include "mfc_interface.h"


/* FIXME: instance state should be more specific */
enum instance_state {
	INST_STATE_NULL		= 0,

	/* open */
	INST_STATE_CREATE	= 0x0001,

	/* ioctl - *_INIT */
	INST_STATE_OPEN		= 0x0010,
	INST_STATE_INIT,

	/* ioctl - *_EXE */
	INST_STATE_EXE		= 0x0020,
	INST_STATE_EXE_DONE,
};

//enum frm_type {
//	ENC_FRM_I	= 0,
//	ENC_FRM_P	= 1,
//	ENC_FRM_B	= 2,
//	DEC_FRM_I	= 3,
//	DEC_FRM_P	= 4,
//	DEC_FRM_B	= 5,
//};

struct mfc_inst_ctx;

struct codec_operations {
	/* initialization routines */
	int (*alloc_ctx_buf) (struct mfc_inst_ctx *ctx);
	int (*alloc_desc_buf) (struct mfc_inst_ctx *ctx);
	int (*get_init_arg) (struct mfc_inst_ctx *ctx, void *arg);
	int (*pre_seq_start) (struct mfc_inst_ctx *ctx);
	int (*post_seq_start) (struct mfc_inst_ctx *ctx);
	int (*set_init_arg) (struct mfc_inst_ctx *ctx, void *arg);
	int (*set_codec_bufs) (struct mfc_inst_ctx *ctx);
	int (*set_dpbs) (struct mfc_inst_ctx *ctx);		/* decoder */
	/* execution routines */
	int (*get_exe_arg) (struct mfc_inst_ctx *ctx, void *arg);
	int (*pre_frame_start) (struct mfc_inst_ctx *ctx);
	int (*post_frame_start) (struct mfc_inst_ctx *ctx);
	int (*multi_data_frame) (struct mfc_inst_ctx *ctx);
	int (*set_exe_arg) (struct mfc_inst_ctx *ctx, void *arg);
	/* configuration routines */
	int (*get_codec_cfg) (struct mfc_inst_ctx *ctx, unsigned int type, int *value);
	int (*set_codec_cfg) (struct mfc_inst_ctx *ctx, unsigned int type, int *value);
};

struct mfc_pre_cfg {
	struct list_head list;
	unsigned int type;
	unsigned int value[4];
};

struct mfc_dec_cfg {
	unsigned int crc;
	unsigned int pixelcache;
	unsigned int slice;
	unsigned int numextradpb;

	unsigned int postfilter;	/* MPEG4 */
	unsigned int dispdelay_en;	/* H.264 */
	unsigned int dispdelay_val;	/* H.264 */
	unsigned int width;		/* FIMV1 */
	unsigned int height;		/* FIMV1 */
};

struct mfc_enc_cfg {
	/*
	type:
	  init
	  runtime
	  init + runtime
	*/

	/* init */
	unsigned int pixelcache;

	unsigned int frameskip;
	unsigned int frammode;
	unsigned int hier_p;

	/* runtime ? */
	#if 0
	unsigned int frametype;
	unsigned int fraemrate;
	unsigned int bitrate;
	unsigned int vui;		/* H.264 */
	unsigned int hec;		/* MPEG4 */
	unsigned int seqhdrctrl;

	unsigned int i_period;
	#endif
};

enum mfc_job_status {
	MFC_ENCODING = 1,
	MFC_DECODING = 2,
};

enum mfc_resolution_status {
	RES_INCREASED = 1,
	RES_DECERASED = 2,
};

enum mfc_resolution_change_status {
	RES_NO_CHANGE = 0,
	RES_SET_CHANGE = 1,
	RES_SET_REALLOC = 2,
	RES_WAIT_FRAME_DONE = 3,
};

struct mfc_inst_ctx {
	int id;				/* assigned by driver */
	int cmd_id;			/* assigned by F/W */
	int codecid;
	unsigned int type;
	unsigned int codec_type;
	/* for encode */
	enum instance_state state;
	unsigned int width;
	unsigned int height;
	volatile unsigned char *shm;
	unsigned int shmofs;
	unsigned int ctxbufofs;
	unsigned int ctxbufsize;
	unsigned int descbufofs;	/* FIXME: move to decoder context */
	unsigned int descbufsize;	/* FIXME: move to decoder context */
	unsigned long userbase;
	SSBIP_MFC_BUFFER_TYPE buf_cache_type;
	enum mfc_job_status job_status;
	unsigned int numofbframe;	/* FIXME: move to other context? */
	unsigned int period_of_iframe;

	int resolution_status;
	/*
	struct mfc_dec_cfg deccfg;
	struct mfc_enc_cfg enccfg;
	*/
	struct list_head presetcfgs;

	void *c_priv;
	struct codec_operations *c_ops;
	struct mfc_dev *dev;
#ifdef SYSMMU_MFC_ON
	unsigned long pgd;
#endif
#ifdef CONFIG_CPU_FREQ
	int busfreq_flag; /* context bus frequency flag*/
#endif
	enum fb_state fb_state; /* turn on/off framebuffer h/w */
	unsigned int fb_info;
	enum nlc_state nlc_state; /* turn on/off NLC h/w */
	u32  convert_buf_y_addr; /* extra y buffer for tile2linear converting */
	u32  convert_buf_c_addr; /* extra c buffer for tile2linear converting */
	/* the addresses should be physical/dma-able */
	u32  dec_out_buf_y_addr[6]; /* user managed buffer address for dec output y*/
	u32  dec_out_buf_c_addr[6]; /* user managed buffer address for dec output c*/

	/* To keep the current output buffer address return by MFC H/W */
	/* these buffers are shifted by 11 bit to the right */

	u32  current_dec_out_buf_y;
	u32  current_dec_out_buf_c;

	/* for multi instance encoding with FB on */
	u32  enc_ref_buf_y0_addr; /* each instance has addresses of reconstruction buf */
	u32  enc_ref_buf_y1_addr;
	u32  enc_ref_buf_y2_addr;
	u32  enc_ref_buf_y3_addr;
	u32  enc_ref_buf_c0_addr;
	u32  enc_ref_buf_c1_addr;
	u32  enc_ref_buf_c2_addr;
	u32  enc_ref_buf_c3_addr;

	/* for multi instance decoding with FB on */
	u32  *dpb_y;
	u32  *dpb_c;

	/* common variable for multi instance coding with FB on */
	u32  ref_for_next_frame_y;
	u32  ref_for_next_frame_c;

	u32  rec_for_cur_frame_y;
	u32  rec_for_cur_frame_c;

	u32  first_frame;
	u32  current_recon_index;
};

struct mfc_inst_ctx *mfc_create_inst(void);
void mfc_destroy_inst(struct mfc_inst_ctx* ctx);
int mfc_set_inst_state(struct mfc_inst_ctx *ctx, enum instance_state state);
int mfc_chk_inst_state(struct mfc_inst_ctx *ctx, enum instance_state state);
int mfc_set_inst_cfg(struct mfc_inst_ctx *ctx, unsigned int type, int *value);

#endif /* __MFC_INST_H */
