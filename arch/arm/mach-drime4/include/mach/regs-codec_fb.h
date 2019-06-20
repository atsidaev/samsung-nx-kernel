/* linux/arch/arm/mach-drime4/include/mach/regs-codec_fb.h
 *
 * Copyright (c) 2011 Samsung Electronics
 *
 * Base S5P MFC resource and device definitions
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __REGS_CODEC_FB_H
#define __REGS_CODEC_FB_H

struct NlcEnc {
	unsigned int COMP0_HEIGHT;
	unsigned int COMP0_NUM_TILE;
	unsigned int COMP0_OFFSET;

	unsigned int COMP0_MAX_BUFSIZE;
	unsigned int COMP0_SBLEV_TH;
	unsigned int COMP0_BUFCHK_TH;
	unsigned int COMP0_RC_OFF;
	unsigned int COMP0_RC_OFF_EB;
	unsigned int COMP0_BYTE_PER_LINE;
	unsigned int COMP0_ERR_TERM1;
	unsigned int COMP0_ERR_TERM2;
	unsigned int COMP0_RC_STRT_LINE;
	unsigned int COMP0_RC_FREQ_LINE;

	unsigned int COMP1_HEIGHT;
	unsigned int COMP1_NUM_TILE;
	unsigned int COMP1_OFFSET;

	unsigned int COMP1_MAX_BUFSIZE;
	unsigned int COMP1_SBLEV_TH;
	unsigned int COMP1_BUFCHK_TH;
	unsigned int COMP1_RC_OFF;
	unsigned int COMP1_RC_OFF_EB;
	unsigned int COMP1_BYTE_PER_LINE;
	unsigned int COMP1_ERR_TERM1;
	unsigned int COMP1_ERR_TERM2;
	unsigned int COMP1_RC_STRT_LINE;
	unsigned int COMP1_RC_FREQ_LINE;
};


struct NlcDec {
	unsigned int COMP0_HEIGHT; //Image height
	unsigned int COMP0_NUM_TILE; //# of Tile - 1
	unsigned int COMP0_OFFSET; //Tile offset address

	unsigned int COMP0_MAX_BUFSIZE;
	unsigned int COMP0_SBLEV_TH;
	unsigned int COMP0_BUFCHK_TH;

	unsigned int COMP1_HEIGHT;
	unsigned int COMP1_NUM_TILE;
	unsigned int COMP1_OFFSET;

	unsigned int COMP1_MAX_BUFSIZE;
	unsigned int COMP1_SBLEV_TH;
	unsigned int COMP1_BUFCHK_TH;
};

enum codec_fb_support_type {
	CIF = 352,
	VGA = 640,
	SD = 720,
	HD = 1280,
	FULLHD = 1920,
};

struct drime4_codec_framebuffer_info {
	char *name;
	int partition;
	int mbx_tile_info;
	int address_range_luma;
	int address_range_chroma;
	int img_width;
	int img_height;
	int img_mask;
};

#define NLC_ENC_COMP0_TILE_HEIGHT	0xF100
#define NLC_ENC_COMP0_ADDROFFSET	0xF108
#define NLC_ENC_COMP0_RATE_CTRL_0	0xF10C
#define NLC_ENC_COMP0_RATE_CTRL_1	0xF110
#define NLC_ENC_COMP0_RATE_CTRL_2	0xF114

#define NLC_ENC_COMP1_TILE_HEIGHT	0xF120
#define NLC_ENC_COMP1_ADDROFFSET	0xF128
#define NLC_ENC_COMP1_RATE_CTRL_0	0xF12C
#define NLC_ENC_COMP1_RATE_CTRL_1	0xF130
#define NLC_ENC_COMP1_RATE_CTRL_2	0xF134

#define NLC_DEC_COMP0_TILE_HEIGHT	0x00
#define NLC_DEC_COMP0_ADDROFFSET	0x08
#define NLC_DEC_COMP0_BUFFERCHECK	0x0C

#define NLC_DEC_COMP1_TILE_HEIGHT	0x20
#define NLC_DEC_COMP1_ADDROFFSET	0x28
#define NLC_DEC_COMP1_BUFFERCHECK	0x2C

/* framebuffer reg definition */
#define DRIME4_FB_DUAL_BANK		0xb14
#define DRIME4_FB_CODEC_TOP_CTRL	0xf000
#define DRIME4_FB_INTR_MASK		0xf004
#define DRIME4_FB_INTR_STAT		0xf008
#define DRIME4_FB_IMG_WIDTH		0xf00c
#define DRIME4_FB_IMG_HEIGHT		0xf010
#define DRIME4_FB_CUR_LUMA_BASE	0xf014
#define DRIME4_FB_CUR_CHROMA_BASE	0xf018
#define DRIME4_FB_PARTITION		0xf01c
#define DRIME4_FB_MBX_TILE_INFO	0xf020
#define DRIME4_FB_PORTB_BASE		0xf024
#define DRIME4_FB_ADDR_RANGE_LUMA	0xf028
#define DRIME4_FB_ADDR_RANGE_CHROMA	0xf02c
#define DRIME4_FB_INTR_CLR		0xf0f0

#define DRIME4_FB_TILE_SRC_BASE	0xf030
#define DRIME4_FB_TILE_DST_BASE	0xf034
#define DRIME4_FB_SW_RECON_Y		0xf038
#define DRIME4_FB_SW_RECON_C		0xf03c
#define DRIME4_FB_SW_REF_Y		0xf040
#define DRIME4_FB_SW_REF_C		0xf044
#define DRIME4_FB_SW_REF2_Y		0xf048
#define DRIME4_FB_SW_REF2_C		0xf04c
#define DRIME4_FB_TILE_WIDTH_HEIGHT	0xf050
#define DRIME4_FB_REF_REC_TEMP_LUMA	0xf054
#define DRIME4_FB_REF_REC_TEMP_CHROMA	0xf058
#define DRIME4_FB_REF1_LUMA_STATE	0xf07c
#define DRIME4_FB_REF1_CHROMA_STATE	0xf080
#define DRIME4_FB_REF2_LUMA_STATE	0xf084
#define DRIME4_FB_REF2_CHROMA_STATE	0xf088
#define DRIME4_FB_RECON_LUMA_STATE	0xf08c
#define DRIME4_FB_RECON_CHROMA_STATE	0xf090
#define DRIME4_FB_FSM_MASK		0x7
#define DRIME4_FB_STATUS0		0xf0ac
#define DRIME4_FB_STATUS1		0xf0b0

/* bit field definition for each register */
/* codec top control register 0xf000 */
#define DRIME4_FB_CTRL_BYPASS		(1<<0)
#define DRIME4_FB_CTRL_FB_ON		(0<<1)
#define DRIME4_FB_CTRL_FB_OFF		(1<<1)
#define DRIME4_FB_CTRL_TILE_START	(1<<4)
#define DRIME4_FB_CTRL_STATE(x)		((x)<<1)
#define DRIME4_FB_CTRL_NR_OF_REF(x)	(((x) & 0x3) << 2)
#define DRIME4_FB_CTRL_SW_MODE_START	(1 << 9)
#define DRIME4_FB_CTRL_BASE_ADDR_MASK	(0xF)
#define DRIME4_FB_CTRL_BASE_ADDR_SHIFT	(25)
#define DRIME4_FB_CTRL_BASE_ADDR_OPT1	(1 << 25)
#define DRIME4_FB_CTRL_RESET		(1 << 30)
#define DRIME4_FB_CTRL_SOFT_RESET_STEP1	(1 << 31)
#define DRIME4_FB_CTRL_SOFT_RESET_STEP2	(0 << 31)

/* added 2012-04-25 for codec nlc block */
#define DRIME4_FB_CTRL_NLC_CUR_ON	(1<<7)
#define DRIME4_FB_CTRL_NLC_REF_ON	(1<<8)
#define DRIME4_FB_CTRL_NLC_CUR_OFF	(0<<7)
#define DRIME4_FB_CTRL_NLC_REF_OFF	(0<<8)

#define DRIME4_FB_INTR_MASK_FB_ACTIVE	(1<<0)
#define DRIME4_FB_INTR_MASK_T2R_ACTIVE	(1<<1)


#endif /* __REGS_CODEC_FB */
