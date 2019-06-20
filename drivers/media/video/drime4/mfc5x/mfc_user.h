/*
 * linux/drivers/media/video/samsung/mfc5x/mfc_user.h
 *
 * Copyright (c) 2010 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com/
 *
 * Definition used commonly between application and driver
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __MFC_USER_H
#define __MFC_USER_H __FILE__

#include "mfc_errno.h"

#define IOCTL_MFC_DEC_INIT		(0x00800001)
#define IOCTL_MFC_ENC_INIT		(0x00800002)
#define IOCTL_MFC_DEC_EXE		(0x00800003)
#define IOCTL_MFC_ENC_EXE		(0x00800004)

#define IOCTL_MFC_GET_IN_BUF		(0x00800010)
#define IOCTL_MFC_FREE_BUF		(0x00800011)
#define IOCTL_MFC_GET_REAL_ADDR		(0x00800012)
#define IOCTL_MFC_GET_MMAP_SIZE		(0x00800014)
#define IOCTL_MFC_SET_IN_BUF		(0x00800018)
#define IOCTL_MFC_WORK_TILE2LINEAR	(0x0080001c)

#define IOCTL_MFC_SET_CONFIG		(0x00800101)
#define IOCTL_MFC_GET_CONFIG		(0x00800102)

/* MFC H/W support maximum 32 extra DPB. */
#define MFC_MAX_EXTRA_DPB	5
#define MFC_MAX_DISP_DELAY	0xF

enum codec_type {
	UNKNOWN,

	H264_DEC = 0x10,
	MPEG4_DEC,
	XVID_DEC,
	H263_DEC,

	FIMV1_DEC,
	FIMV2_DEC,
	FIMV3_DEC,
	FIMV4_DEC,

	VC1_DEC,		/* VC1 advaced Profile decoding  */
	VC1RCV_DEC,		/* VC1 simple/main profile decoding  */

	MPEG1_DEC,
	MPEG2_DEC,

	MPEG4_ENC = 0x40,
	H263_ENC,
	H264_ENC,
};

enum inst_type {
	DECODER = 0x1,
	ENCODER = 0x2,
};

enum force_frame_type {
	DONT_CARE = 0,
	I_FRAME,
	NOT_CODED,
};

#define SET_CFG		(0x1 << 16)
#define GET_CFG		(0x2 << 16)

#define DEC_SET		(DECODER << 24 | SET_CFG)
#define DEC_GET		(DECODER << 24 | GET_CFG)
#define ENC_SET		(ENCODER << 24 | SET_CFG)
#define ENC_GET		(ENCODER << 24 | GET_CFG)

enum dec_cfg_type {
	MFC_DEC_SETCONF_EXTRA_BUFFER_NUM = DEC_SET,
	MFC_DEC_SETCONF_SLICE_ENABLE,
	MFC_DEC_SETCONF_CRC_ENABLE,
	MFC_DEC_SETCONF_PIXEL_CACHE,

	MFC_DEC_SETCONF_DISPLAY_DELAY,
	MFC_DEC_SETCONF_POST_ENABLE,
	MFC_DEC_SETCONF_PACKEDPB,
	MFC_DEC_SETCONF_WIDTH_HEIGHT,

	MFC_DEC_SETCONF_IS_LAST_FRAME,
	MFC_DEC_SETCONF_FRAME_TAG,

	MFC_DEC_GETCONF_CRC_DATA	= DEC_GET,
	MFC_DEC_GETCONF_BUF_WIDTH_HEIGHT,
	/* MFC_DEC_GETCONF_IMG_RESOLUTION, */
	MFC_DEC_GETCONF_FRAME_TAG,
};

enum enc_cfg_type {
	MFC_ENC_SETCONF_FRAME_TYPE	= ENC_SET,
	MFC_ENC_SETCONF_CHANGE_FRAME_RATE,
	MFC_ENC_SETCONF_CHANGE_BIT_RATE,
	MFC_ENC_SETCONF_FRAME_TAG,
	MFC_ENC_SETCONF_ALLOW_FRAME_SKIP,
	MFC_ENC_SETCONF_VUI_INFO,
	MFC_ENC_SETCONF_I_PERIOD,
	MFC_ENC_SETCONF_HIER_P,

	MFC_ENC_GETCONF_FRAME_TAG	= ENC_GET,
};

struct mfc_strm_ref_buf_arg {
	unsigned int strm_ref_y;
	unsigned int mv_ref_yc;
};

struct mfc_frame_buf_arg {
	unsigned int luma;
	unsigned int chroma;
};

struct mfc_enc_init_common_arg {
	enum codec_type in_codec_type;	/* [IN] codec type */

	int in_width;		/* [IN] width of YUV420 frame to be encoded */
	int in_height;		/* [IN] height of YUV420 frame to be encoded */

	int in_gop_num;		/* [IN] GOP Number (interval of I-frame) */
	int in_vop_quant;	/* [IN] VOP quant */
	int in_vop_quant_p;	/* [IN] VOP quant for P frame */

	/* [IN] RC enable */
	/* [IN] RC enable (0:disable, 1:frame level RC) */
	int in_rc_fr_en;
	int in_rc_bitrate;	/* [IN]  RC parameter (bitrate in kbps) */

	int in_rc_qbound_min;	/* [IN]  RC parameter (Q bound Min) */
	int in_rc_qbound_max;	/* [IN]  RC parameter (Q bound Max) */
	int in_rc_rpara;	/* [IN]  RC parameter (Reaction Coefficient) */

	/* [IN] Multi-slice mode (0:single, 1:multiple) */
	int in_ms_mode;
	/* [IN] Multi-slice size (in num. of mb or byte) */
	int in_ms_arg;

	int in_mb_refresh;	/* [IN]  Macroblock refresh */

	/* [IN] Enable (1) / Disable (0) padding with the specified values */
	int in_pad_ctrl_on;

	/* [IN] pad value if pad_ctrl_on is Enable */
	int in_y_pad_val;
	int in_cb_pad_val;
	int in_cr_pad_val;

	/* linear or tiled */
	int in_frame_map;

	unsigned int in_pixelcache;

	unsigned int in_mapped_addr;
	struct mfc_strm_ref_buf_arg out_u_addr;
	struct mfc_strm_ref_buf_arg out_p_addr;
	struct mfc_strm_ref_buf_arg out_buf_size;
	unsigned int out_header_size;
};

struct mfc_enc_init_h263_arg {
	int in_rc_framerate;	/* [IN]  RC parameter (framerate) */
};

struct mfc_enc_init_mpeg4_arg {
	int in_profile;		/* [IN] profile */
	int in_level;		/* [IN] level */

	int in_vop_quant_b;	/* [IN] VOP quant for B frame */

	/* [IN] B frame number */
	int in_bframenum;

	/* [IN] Quarter-pel MC enable (1:enabled, 0:disabled) */
	int in_quart_pixel;

	int in_TimeIncreamentRes;	/* [IN] VOP time resolution */
	int in_VopTimeIncreament;	/* [IN] Frame delta */
};

struct mfc_enc_init_h264_arg {
	int in_profile;		/* [IN] profile */
	int in_level;		/* [IN] level */

	int in_vop_quant_b;	/* [IN] VOP quant for B frame */

	/* [IN] B frame number */
	int in_bframenum;

	/* [IN] interlace mode(0:progressive, 1:interlace) */
	int in_interlace_mode;

	/* [IN]  reference number */
	int in_reference_num;
	/* [IN]  reference number of P frame */
	int in_ref_num_p;

	int in_rc_framerate;	/* [IN]  RC parameter (framerate) */
	int in_rc_mb_en;	/* [IN] RC enable (0:disable, 1:MB level RC) */
	/* [IN] MB level rate control dark region adaptive feature */
	int in_rc_mb_dark_dis;	/* (0:enable, 1:disable) */
	/* [IN] MB level rate control smooth region adaptive feature */
	int in_rc_mb_smooth_dis;	/* (0:enable, 1:disable) */
	/* [IN] MB level rate control static region adaptive feature */
	int in_rc_mb_static_dis;	/* (0:enable, 1:disable) */
	/* [IN] MB level rate control activity region adaptive feature */
	int in_rc_mb_activity_dis;	/* (0:enable, 1:disable) */

	/* [IN]  disable deblocking filter idc */
	int in_deblock_dis;	/* (0: enable,1: disable, 2:Disable at slice boundary) */
	/* [IN]  slice alpha c0 offset of deblocking filter */
	int in_deblock_alpha_c0;
	/* [IN]  slice beta offset of deblocking filter */
	int in_deblock_beta;

	/* [IN]  ( 0 : CAVLC, 1 : CABAC ) */
	int in_symbolmode;
	/* [IN] (0: only 4x4 transform, 1: allow using 8x8 transform) */
	int in_transform8x8_mode;

	/* [IN] Inter weighted parameter for mode decision */
	int in_md_interweight_pps;
	/* [IN] Intra weighted parameter for mode decision */
	int in_md_intraweight_pps;
};

struct mfc_enc_init_arg {
	struct mfc_enc_init_common_arg cmn;
	union {
		struct mfc_enc_init_h264_arg h264;
		struct mfc_enc_init_mpeg4_arg mpeg4;
		struct mfc_enc_init_h263_arg h263;
	} codec;
};

struct mfc_enc_exe_arg {
	enum codec_type in_codec_type;	/* [IN] codec type */
	unsigned int in_Y_addr;		/* [IN]In-buffer addr of Y component */
	unsigned int in_CbCr_addr;	/* [IN]In-buffer addr of CbCr component */
	unsigned int in_Y_addr_vir;	/* [IN]In-buffer addr of Y component */
	unsigned int in_CbCr_addr_vir;	/* [IN]In-buffer addr of CbCr component */
	unsigned int in_strm_st;	/* [IN]Out-buffer start addr of encoded strm */
	unsigned int in_strm_end;	/* [IN]Out-buffer end addr of encoded strm */
	unsigned int out_frame_type;	/* [OUT] frame type  */
	int out_encoded_size;		/* [OUT] Length of Encoded video stream */
	unsigned int out_Y_addr;	/* [OUT]Out-buffer addr of encoded Y component */
	unsigned int out_CbCr_addr;	/* [OUT]Out-buffer addr of encoded CbCr component */

#if defined(CONFIG_VIDEO_MFC_VCM_UMP)
	unsigned int out_y_secure_id;
	unsigned int out_c_secure_id;
#elif defined(CONFIG_S5P_VMEM)
	unsigned int out_y_cookie;
	unsigned int out_c_cookie;
#endif
};

struct mfc_dec_init_arg {
	enum codec_type in_codec_type;	/* [IN] codec type */
	int in_strm_buf;		/* [IN] address of stream buffer */
	int in_strm_size;		/* [IN] filled size in stream buffer */

	unsigned int in_crc;		/* [IN] */
	unsigned int in_pixelcache;	/* [IN] */
	unsigned int in_slice;		/* [IN] */
	unsigned int in_numextradpb;	/* [IN] */

	unsigned int in_mapped_addr;

	int out_frm_width;		/* [OUT] width  of YUV420 frame */
	int out_frm_height;		/* [OUT] height of YUV420 frame */
	int out_buf_width;		/* [OUT] width  of YUV420 frame */
	int out_buf_height;		/* [OUT] height of YUV420 frame */

	int out_dpb_cnt;		/* [OUT] the number of buffers which is nessary during decoding. */
};

struct mfc_dec_exe_arg {
	enum codec_type in_codec_type;	/* [IN]  codec type */
	int in_strm_buf;	/* [IN]  the physical address of STRM_BUF */
	/* [IN]  Size of video stream filled in STRM_BUF */
	int in_strm_size;
	/* [IN] the address of dpb FRAME_BUF */
	struct mfc_frame_buf_arg in_frm_buf;
	/* [IN] size of dpb FRAME_BUF */
	struct mfc_frame_buf_arg in_frm_size;
	/* [OUT]  the physical address of display buf */
	int out_display_Y_addr;
	/* [OUT]  the physical address of display buf */
	int out_display_C_addr;
	int out_display_status;
	int out_pic_time_top;
	int out_pic_time_bottom;
	int out_consumed_byte;

	int out_crop_right_offset;
	int out_crop_left_offset;
	int out_crop_bottom_offset;
	int out_crop_top_offset;

	/* in new driver, each buffer offset must be return to the user */
	int out_y_offset;
	int out_c_offset;

#if defined(CONFIG_VIDEO_MFC_VCM_UMP)
	unsigned int out_y_secure_id;
	unsigned int out_c_secure_id;
#elif defined(CONFIG_S5P_VMEM)
	unsigned int out_y_cookie;
	unsigned int out_c_cookie;
#endif
};

struct mfc_get_config_arg {
	/* [IN] Configurable parameter type */
	int in_config_param;

	/* [IN] Values to get for the configurable parameter. */
	/* Maximum four integer values can be obtained; */
	int out_config_value[4];
};

struct mfc_set_config_arg {
	/* [IN] Configurable parameter type */
	int in_config_param;

	/* [IN]  Values to be set for the configurable parameter. */
	/* Maximum four integer values can be set. */
	int in_config_value[4];
};

struct mfc_get_real_addr_arg {
	unsigned int key;
	unsigned int addr;
};

struct mfc_buf_alloc_arg {
	enum inst_type type;
	int size;
	/*
	unsigned int mapped;
	*/
	unsigned int align;

	unsigned int addr;
	/*
	unsigned int phys;
	*/
#if defined(CONFIG_VIDEO_MFC_VCM_UMP)
	/* FIMXE: invalid secure id == -1 */
	unsigned int secure_id;
#elif defined(CONFIG_S5P_VMEM)
	unsigned int cookie;
#else
	unsigned int offset;
#endif
};

struct mfc_buf_free_arg {
	unsigned int addr;
};


/* RMVME */
struct mfc_mem_alloc_arg {
	enum inst_type type;
	int buff_size;
	unsigned int mapped_addr;
#if defined(CONFIG_VIDEO_MFC_VCM_UMP)
	unsigned int secure_id;
#elif defined(CONFIG_S5P_VMEM)
	unsigned int cookie;
#else
	unsigned int offset;
#endif
};

struct mfc_mem_free_arg {
	unsigned int key;
};
/* RMVME */

union mfc_args {
	/*
	struct mfc_enc_init_arg enc_init;

	struct mfc_enc_init_mpeg4_arg enc_init_mpeg4;
	struct mfc_enc_init_mpeg4_arg enc_init_h263;
	struct mfc_enc_init_h264_arg enc_init_h264;
	*/
	struct mfc_enc_init_arg enc_init;
	struct mfc_enc_exe_arg enc_exe;

	struct mfc_dec_init_arg dec_init;
	struct mfc_dec_exe_arg dec_exe;

	struct mfc_get_config_arg get_config;
	struct mfc_set_config_arg set_config;

	struct mfc_buf_alloc_arg buf_alloc;
	struct mfc_buf_free_arg buf_free;
	struct mfc_get_real_addr_arg real_addr;

	/* RMVME */
	struct mfc_mem_alloc_arg mem_alloc;
	struct mfc_mem_free_arg mem_free;
	/* RMVME */
};

struct mfc_common_args {
	enum mfc_ret_code ret_code;	/* [OUT] error code */
	union mfc_args args;
};

struct mfc_enc_vui_info {
	int aspect_ratio_idc;
};

struct mfc_dec_fimv1_info {
	int width;
	int height;
};

struct mfc_enc_hier_p_qp {
	int t0_frame_qp;
	int t2_frame_qp;
	int t3_frame_qp;
};

#define ENC_PROFILE_LEVEL(profile, level)      ((profile) | ((level) << 8))
#define ENC_RC_QBOUND(min_qp, max_qp)          ((min_qp) | ((max_qp) << 8))

#endif /* __MFC_USER_H */
