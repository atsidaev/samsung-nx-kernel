/**
 * @file	regs-tv.h
 * @brief
 * @author somabha (b.somabha@samsung.com)
 * 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef _DP_TV_REG_
#define _DP_TV_REG_

#include <mach/d4_reg_macro.h>

/*  TV  Address Offset*/
#define	DP_TV_TG			(0)
#define	DP_TV_PATH			(0x100)
#define	DP_TV_MCHDMA			(0x200)
#define	DP_TV_GRP_PATH1			(0x300)

/*  TG  Address Offset*/
#define DP_TV_TG_SIZE			(DP_TV_TG+0x00)
#define DP_TV_TG_HSYNC			(DP_TV_TG+0x04)
#define DP_TV_TG_F0VSYNC		(DP_TV_TG+0x08)
#define DP_TV_TG_F1VSYNC		(DP_TV_TG+0x0c)
#define DP_TV_TG_FSYNC			(DP_TV_TG+0x10)
#define DP_TV_TG_FVSNC			(DP_TV_TG+0x14)
#define DP_TV_TG_INFO_0		        (DP_TV_TG+0x18)
#define DP_TV_TG_INFO_1		        (DP_TV_TG+0x1c)
#define DP_TV_TG_INFO_2		        (DP_TV_TG+0x20)
#define DP_TV_TG_ACTIVE_PIXEL	        (DP_TV_TG+0x24)
#define DP_TV_TG_ACTIVE_0_LINE	        (DP_TV_TG+0x28)
#define DP_TV_TG_ACTIVE_1_LINE	        (DP_TV_TG+0x2c)
#define DP_TV_TG_FIELD			(DP_TV_TG+0x30)
#define DP_TV_TG_V_OFFSET		(DP_TV_TG+0x34)
#define DP_TV_TG_H_OFFSET		(DP_TV_TG+0x38)

/*  PATH  Address Offset*/
#define DP_TV_PATH_CONTROL		(DP_TV_PATH + 0x00)
#define DP_TV_PATH_VIDIN		(DP_TV_PATH + 0x04)
#define DP_TV_PATH_STRIDE		(DP_TV_PATH + 0x08)
#define DP_TV_PATH_VIDHRZFLT		(DP_TV_PATH + 0x10)
#define DP_TV_PATH_VIDDMA_M0		(DP_TV_PATH + 0x14)
#define DP_TV_PATH_VIDDMA_M1		(DP_TV_PATH + 0x18)
#define DP_TV_PATH_VIDDMA_M2		(DP_TV_PATH + 0x1c)
#define DP_TV_PATH_VIDGRPMIX_ON		(DP_TV_PATH + 0x20)
#define DP_TV_PATH_VIDGRPMIX0_ALPCTRL	(DP_TV_PATH + 0x24)
#define DP_TV_PATH_VIDGRPMIX0_CH0	(DP_TV_PATH + 0x28)
#define DP_TV_PATH_VIDGRPMIX0_CH1	(DP_TV_PATH + 0x2c)
#define DP_TV_PATH_GRPMIX_RANGE_R       (DP_TV_PATH + 0x30)
#define DP_TV_PATH_GRPMIX_RANGE_G       (DP_TV_PATH + 0x34)
#define DP_TV_PATH_GRPMIX_RANGE_B       (DP_TV_PATH + 0x38)
#define DP_TV_PATH_LIMIT_R		(DP_TV_PATH + 0x3c)
#define DP_TV_PATH_LIMIT_G		(DP_TV_PATH + 0x40)
#define DP_TV_PATH_LIMIT_B		(DP_TV_PATH + 0x44)
#define DP_TV_PATH_EXPAN		(DP_TV_PATH + 0x4c)

#define DP_TV_PATH_VID_CSC0		(DP_TV_PATH + 0x50)
#define DP_TV_PATH_VID_CSC1		(DP_TV_PATH + 0x54)
#define DP_TV_PATH_VID_CSC2		(DP_TV_PATH + 0x58)
#define DP_TV_PATH_VID_CSC3		(DP_TV_PATH + 0x5C)
#define DP_TV_PATH_VID_CSC4		(DP_TV_PATH + 0x60)
#define DP_TV_PATH_VID_CSC5		(DP_TV_PATH + 0x64)
#define DP_TV_PATH_VID_CSC6		(DP_TV_PATH + 0x68)
#define DP_TV_PATH_VID_CSC7		(DP_TV_PATH + 0x6C)

#define DP_TV_PATH_GRP_CSC0		(DP_TV_PATH + 0x70)
#define DP_TV_PATH_GRP_CSC1		(DP_TV_PATH + 0x74)
#define DP_TV_PATH_GRP_CSC2		(DP_TV_PATH + 0x78)
#define DP_TV_PATH_GRP_CSC3		(DP_TV_PATH + 0x7c)
#define DP_TV_PATH_GRP_CSC4		(DP_TV_PATH + 0x80)
#define DP_TV_PATH_GRP_CSC5		(DP_TV_PATH + 0x84)
#define DP_TV_PATH_GRP_CSC6		(DP_TV_PATH + 0x88)
#define DP_TV_PATH_GRP_CSC7		(DP_TV_PATH + 0x8C)
#define DP_TV_PATH_GRP_CSC8		(DP_TV_PATH + 0x90)
#define DP_TV_PATH_TVOUT		(DP_TV_PATH + 0x94)
#define DP_TV_PATH_TVOUT_PAT_GEN	(DP_TV_PATH + 0x98)

/*  Video  TV_VID_PATH Address Offset*/
#define DP_TV_VID_PATH_VIDCH0_ADDR_F0Y		(DP_TV_MCHDMA + 0x00)
#define DP_TV_VID_PATH_VIDCH0_ADDR_F0C		(DP_TV_MCHDMA + 0x04)
#define DP_TV_VID_PATH_VIDCH0_ADDR_F1Y		(DP_TV_MCHDMA + 0x08)
#define DP_TV_VID_PATH_VIDCH0_ADDR_F1C		(DP_TV_MCHDMA + 0x0c)
#define DP_TV_VID_PATH_VIDCH0_HPOS		(DP_TV_MCHDMA + 0x10)
#define DP_TV_VID_PATH_VIDCH0_VPOS		(DP_TV_MCHDMA + 0x14)
#define DP_TV_VID_PATH_BB_H_STARTEND0           (DP_TV_MCHDMA + 0x18)
#define DP_TV_VID_PATH_BB_V_STARTEND0           (DP_TV_MCHDMA + 0x1c)
#define DP_TV_VID_PATH_BB_H_STARTEND4           (DP_TV_MCHDMA + 0x80)
#define DP_TV_VID_PATH_BB_V_STARTEND4           (DP_TV_MCHDMA + 0x84)

#define DP_TV_VID_PATH_BB_ON                    (DP_TV_MCHDMA + 0xe0)
#define DP_TV_VID_PATH_BB_COLOR00               (DP_TV_MCHDMA + 0xe4)
#define DP_TV_VID_PATH_BB_COLOR_BOX             (DP_TV_MCHDMA + 0xf4)
#define DP_TV_VID_PATH_BB_ALPHA                 (DP_TV_MCHDMA + 0xf8)
#define DP_TV_VID_PATH_BB_BOUNDARY_COLOR        (DP_TV_MCHDMA + 0xfc)

/*  Graphic1 Address Offset*/
#define DP_TV_GRP_PATH1_DMA_CTRL		(DP_TV_GRP_PATH1+0x00)
#define DP_TV_GRP_PATH1_DMA_SCL			(DP_TV_GRP_PATH1+0x04)
#define DP_TV_GRP_PATH1_BKG			(DP_TV_GRP_PATH1+0x0c)
#define DP_TV_GRP_PATH1_FLAT_ALPHA		(DP_TV_GRP_PATH1+0x10)
#define DP_TV_GRP_PATH1_DMA_PRIORITY		(DP_TV_GRP_PATH1+0x18)

#define DP_TV_GRP_PATH1_CH0_ADDR_F0		(DP_TV_GRP_PATH1+0x20)
#define DP_TV_GRP_PATH1_CH0_ADDR_F1		(DP_TV_GRP_PATH1+0x24)
#define DP_TV_GRP_PATH1_CH0_HPOS		(DP_TV_GRP_PATH1+0x28)
#define DP_TV_GRP_PATH1_CH0_VPOS		(DP_TV_GRP_PATH1+0x2c)
#define DP_TV_GRP_PATH1_CH1_ADDR_F0		(DP_TV_GRP_PATH1+0x30)
#define DP_TV_GRP_PATH1_CH1_ADDR_F1		(DP_TV_GRP_PATH1+0x34)
#define DP_TV_GRP_PATH1_CH1_HPOS		(DP_TV_GRP_PATH1+0x38)
#define DP_TV_GRP_PATH1_CH1_VPOS		(DP_TV_GRP_PATH1+0x3c)
#define DP_TV_GRP_PATH1_CH2_ADDR_F0		(DP_TV_GRP_PATH1+0x40)
#define DP_TV_GRP_PATH1_CH2_ADDR_F1		(DP_TV_GRP_PATH1+0x44)
#define DP_TV_GRP_PATH1_CH2_HPOS		(DP_TV_GRP_PATH1+0x48)
#define DP_TV_GRP_PATH1_CH2_VPOS		(DP_TV_GRP_PATH1+0x4c)
#define DP_TV_GRP_PATH1_CH3_ADDR_F0		(DP_TV_GRP_PATH1+0x50)
#define DP_TV_GRP_PATH1_CH3_ADDR_F1		(DP_TV_GRP_PATH1+0x54)
#define DP_TV_GRP_PATH1_CH3_HPOS		(DP_TV_GRP_PATH1+0x58)
#define DP_TV_GRP_PATH1_CH3_VPOS		(DP_TV_GRP_PATH1+0x5c)
#define DP_TV_GRP_PATH1_HRZFILT			(DP_TV_GRP_PATH1+0x60)

/************************************************/
/*          Register Structure Define           */
/************************************************/

/*          TV TG SIZE           */
#define Pixel_Full_M1(x)	(((x) & 0xffff) << 0)
#define Line_Full_M1(x)		(((x) & 0xffff) << 16)

/*	    TV TG HSYNC         */
#define Hsync_BLK(x)		(((x) & 0xffff) << 0)
#define Hsync_ACT(x)		(((x) & 0xffff) << 16)

/*          TV TG F0 VSYNC      */
#define Vsync_BLK0(x)		(((x) & 0xffff) << 0)
#define Vsync_ACT0(x)		(((x) & 0xffff) << 16)

/*          TV TG F1 VSYNC      */
#define Vsync_BLK1(x)		(((x) & 0xffff) << 0)
#define Vsync_ACT1(x)		(((x) & 0xffff) << 16)

/*          TV TG FSYNC         */
#define Fsync_BLK0(x)		(((x) & 0xffff) << 0)
#define Fsync_ACT0(x)		(((x) & 0xffff) << 16)

/*          TV TG FVsnc         */
#define Pixel_Start(x)		(((x) & 0xffff) << 0)
#define Fsync_ACT1(x)		(((x) & 0xffff) << 16)

/*          TV TG Info0         */
#define Frame_Init(x)		(((x) & 0xffff) << 0)
#define Frame_Finalize(x)	(((x) & 0xffff) << 16)

/*          TV TG Info1         */
#define Field_Init(x)		(((x) & 0xffff) << 0)
#define BUF_Read_Start(x)	(((x) & 0xffff) << 16)

/*	TV TG Info2		*/
#define Pre_Line_Start(x)	(((x) & 0xffff) << 0)
#define Pre_Line_End(x)		(((x) & 0xffff) << 16)

/*	TV TG Active Pixel	*/
#define De_Pixel_Start(x)	(((x) & 0xffff) << 0)
#define De_Pixel_End(x)		(((x) & 0xffff) << 16)

/*	TV TG Active0 Line	*/
#define Active_Line_Start0(x)	(((x) & 0xffff) << 0)
#define Active_Line_End0(x)	(((x) & 0xffff) << 16)

/*	TV TG Active1 Line	*/
#define Active_Line_Start1(x)	(((x) & 0xffff) << 0)
#define Active_Line_End1(x)	(((x) & 0xffff) << 16)

/*	TV TG Field		*/
#define Field_BLK(x)		(((x) & 0xffff) << 0)
#define Field_ACT(x)		(((x) & 0xffff) << 16)

/*	TV TG VOffset		*/
#define v_offset0(x)		(((x) & 0xfff) << 0)
#define v_offset1(x)		(((x) & 0xfff) << 16)

/*	TV TG HOffset		*/
#define h_offset0(x)		(((x) & 0xfff) << 0)

/*	TV TG Path Control Register	*/
#define DMAC			(1 << 0)
#define HRZFLT			(1 << 1)
#define GRPMIX0			(1 << 2)
#define RGB2YCBCR		(1 << 3)
#define Diter_ON		(1 << 6)
#define EXPAN_ON		(1 << 7)
#define GRP1DMAC		(1 << 8)
#define GRP1HRZFLT		(1 << 9)
#define CSC_10BIT		(1 << 10)
#define HDMI_ENC		(1 << 12)
#define CVBS_ENC		(1 << 13)
#define BT656_ENC		(1 << 14)
#define VID_3D			(1 << 16)
#define FORMAT_3D(x)		(((x) & 0x3) << 17)
#define SD_ON			(1 << 19)
#define YCBCR422		(1 << 20)
#define QFHD_VID_ONLY		(1 << 21)
#define VID_10BIT		(1 << 22)
#define XV_YCC			(1 << 23)
#define VideoSCAN		(1 << 24)
#define GRP1SCAN		(1 << 25)
#define BBOX_2X			(1 << 26)
#define LINE_INV		(1 << 27)
#define VID_HSWAP		(1 << 28)
#define VID_VSWAP		(1 << 29)
#define GRP_HSWAP		(1 << 30)
#define GRP_VSWAP		(1 << 31)

/*	TV Display Size			*/
#define SizeX(x)		(((x) & 0xfff) << 0)
#define SizeY(x)		(((x) & 0xfff) << 16)

/*	TV Path Stride			*/
#define D4_DP_TV_PATH_VID_STRIDE(val, x) \
	SET_REGISTER_VALUE(val, x, 0, 12)
#define D4_DP_TV_PATH_GRP_STRIDE(val, x) \
	SET_REGISTER_VALUE(val, x, 16, 12)

#define VID_STRIDE(x)		(((x) & 0xfff) << 0)
#define GRP_STRIDE(x)		(((x) & 0xfff) << 16)

/*	TV Horizontal Filtering		*/
#define PRECOEF0(x)		(((x) & 0x7) << 0)
#define PRECOEF1(x)		(((x) & 0x7) << 4)
#define PRECOEF2(x)		(((x) & 0x7) << 8)
#define PRECOEF3(x)		(((x) & 0x7) << 12)
#define PRECOEF4(x)		(((x) & 0x7) << 16)
#define	Sign0			(1 << 20)
#define Sign1			(1 << 21)
#define Sign3			(1 << 22)
#define Sign4			(1 << 23)
#define POSTCOEF(x)		(((x) & 0x7) << 24)

/*	TV Path Video DMA Mode0		*/
#define Ch0Enable		(1 << 0)
#define Ch1Enable		(1 << 1)
#define Ch2Enable		(1 << 2)
#define Ch3Enable		(1 << 3)
#define WD0_ADDR_SWAP		(1 << 4)
#define WD1_ADDR_SWAP		(1 << 5)
#define WD2_ADDR_SWAP		(1 << 6)
#define WD3_ADDR_SWAP		(1 << 7)
#define CHROMA_ORDER		(1 << 8)

/*	TV Path Video DMA Mode1		*/
#define Y_MOALEN(x)		(((x) & 0x7) << 0)
#define Y_8BURST		(1 << 3)
#define C_MOALEN(x)		(((x) & 0x7) << 4)
#define C_8BURST		(1 << 7)
#define G1_MOALEN(x)		(((x) & 0x7) << 8)
#define G1_8BURST		(1 << 11)
#define G2_MOALEN(x)		(((x) & 0x7) << 12)
#define G2_8BURST		(1 << 15)

/*	TV Path Video DMA Mode2		*/
#define BKG_Y(x)		(((x) & 0xff) << 0)
#define BKG_CB(x)		(((x) & 0xff) << 8)
#define BKG_CR(x)		(((x) & 0xff) << 16)

/*          TV PATH DMA M1         */
#define D4_DP_TV_PATH_BKG_Y(val, x) \
    SET_REGISTER_VALUE(val, x, 0, 8)
#define D4_DP_TV_PATH_BKG_Cb(val, x) \
	SET_REGISTER_VALUE(val, x, 8, 8)
#define D4_DP_TV_PATH_BKG_Cr(val, x) \
	SET_REGISTER_VALUE(val, x, 16, 8)

/*	TV Path Video Graphic Mixer Opearation Set	*/
#define ALPBLD_ON		(1 << 0)
#define LIMIT_ON		(1 << 1)
#define RANGE_DET_ON(x)		(((x) & 0x3) << 4)
#define RANGEDET_TARGET		(1 << 6)
#define CH_SW			(1 << 10)
#define FLAG0_R_ON		(1 << 14)
#define FLAG0_G_ON		(1 << 15)
#define FLAG0_B_ON		(1 << 16)
#define RANGE0_INV		(1 << 17)

#define D4_DP_TV_CH_SWAP(val, x) \
	SET_REGISTER_VALUE(val, x, 12, 1)

/*	TV Path Video Graphic Mixer Alpha Control	*/
#define D4_DP_TV_GRP_ALP_ON(val, x) \
	SET_REGISTER_VALUE(val, x, 0, 1)
#define D4_DP_TV_GRP_ALP_INV_ON(val, x) \
	SET_REGISTER_VALUE(val, x, 1, 1)
#define D4_DP_TV_GRP_ALP_OFS(val, x) \
	SET_REGISTER_VALUE(val, x, 2, 1)
#define D4_DP_TV_PATH_ALP_OFS_4BIT(val, x) \
	SET_REGISTER_VALUE(val, x, 8, 4)
#define D4_DP_TV_PATH_CKEY_ALP_8BIT(val, x) \
	SET_REGISTER_VALUE(val, x, 16, 8)

#define CHx_ALP_ON		(1 << 0)
#define CHx_ALP_INV		(1 << 1)
#define CHx_ALP_OFS		(1 << 2)
#define CHx_ALP_OFS_4BIT(x)	(((x) & 0xf) << 8)
#define C_KEY_ALP_8BIT(x)	(((x) & 0xff) << 16)

/*	TV Graphic Mixer Ch0,Ch1,Ch2 setting		*/
#define D4_DP_TV_GRP_MIX_R_ON(val, x) \
    SET_REGISTER_VALUE(val, x, 0, 1)
#define D4_DP_TV_GRP_MIX_G_ON(val, x) \
    SET_REGISTER_VALUE(val, x, 1, 1)
#define D4_DP_TV_GRP_MIX_B_ON(val, x) \
    SET_REGISTER_VALUE(val, x, 2, 1)

#define CH_R_ON			(1 << 0)
#define CH_G_ON			(1 << 1)
#define CH_B_ON			(1 << 2)
#define CH_R_INV_ON		(1 << 4)
#define	CH_G_INV_ON		(1 << 5)
#define CH_B_INV_ON		(1 << 6)
#define CH_R_OFS_ON		(1 << 8)
#define CH_G_OFS_ON		(1 << 9)
#define CH_B_OFS_ON		(1 << 10)
#define R_OFS_5BIT(x)		(((x) & 0xf) << 12)
#define G_OFS_5BIT(x)		(((x) & 0xf) << 16)
#define B_OFS_5BIT(x)		(((x) & 0xf) << 20)
#define OFS_SIGN(x)		(((x) & 0x7) << 24)

/*	TV Range R					*/
#define RANGEDET_R_LOW(x)	(((x) & 0xfff) << 0)
#define RANGEDET_R_UPP(x)	(((x) & 0xfff) << 16)

/*	TV Range G					*/
#define RANGEDET_G_LOW(x)	(((x) & 0xfff) << 0)
#define RANGEDET_G_UPP(x)	(((x) & 0xfff) << 16)

/*	TV Range B					*/
#define RANGEDET_B_LOW(x)	(((x) & 0xfff) << 0)
#define RANGEDET_B_UPP(x)	(((x) & 0xfff) << 16)

/*	TV Graphic Mixer Limit on R			*/
#define LIMIT_R_LOW(x)		(((x) & 0xfff) << 0)
#define LIMIT_R_UPP(x)		(((x) & 0xfff) << 16)

/*	TV Graphic Mixer Limit on G			*/
#define LIMIT_G_LOW(x)		(((x) & 0xfff) << 0)
#define LIMIT_G_UPP(x)		(((x) & 0xfff) << 16)

/*	TV Graphic Mixer Limit on B			*/
#define	LIMIT_B_LOW(x)		(((x) & 0xfff) << 0)
#define LIMIT_B_UPP(x)		(((x) & 0xfff) << 16)

/*	TV Path Expan					*/
#define EXPAN_BIT_RANGE(x)	(((x) & 0x3) << 0)
#define Dither_sel		(1 << 4)

/*	TV YCbCr to RGB 0				*/
#define MTX33_00(x)		(((x) & 0x7ff) << 0)
#define MTX33_01(x)		(((x) & 0x7ff) << 16)

/*	TV YCbCr to RGB 1				*/
#define MTX33_02(x)		(((x) & 0x7ff) << 0)
#define MTX33_10(x)		(((x) & 0x7ff) << 16)

/*      TV YCbCr to RGB 2                               */
#define MTX33_11(x)             (((x) & 0x7ff) << 0)
#define MTX33_12(x)             (((x) & 0x7ff) << 16)

/*      TV YCbCr to RGB 3                               */
#define MTX33_20(x)             (((x) & 0x7ff) << 0)
#define MTX33_21(x)             (((x) & 0x7ff) << 16)

/*      TV YCbCr to RGB 4                               */
#define MTX33_22(x)             (((x) & 0x7ff) << 0)

/*      TV YCbCr to RGB 5                               */
#define MTX33_Y_OFFSET(x)	(((x) & 0x3ff) << 0)
#define MTX33_CB_OFFSET(x)      (((x) & 0x3ff) << 16)

/*      TV YCbCr to RGB 6                               */
#define MTX33_CR_OFFSET(x)       (((x) & 0x3ff) << 0)

/*      TV YCbCr to RGB 7                               */
#define underflow_rgb(x)	(((x) & 0xfff) << 0)
#define overflow_rgb(x)		(((x) & 0xfff) << 16)

/*      TV RGB to YCbCr 7       */
#define UNDERFLOW_Y(x)		(((x) & 0x3ff) << 0)
#define OVERFLOW_Y(x)		(((x) & 0x3ff) << 16)

/*      TV RGB to YCbCr 8       */
#define UNDERFLOW_C(x)          (((x) & 0x3ff) << 0)
#define OVERFLOW_C(x)           (((x) & 0x3ff) << 16)

/*	TV Path TV Out		*/
#define CdelayADJ(x)		(((x) & 0x7) << 0)
#define YdelayADJ(x)		(((x) & 0x7) << 3)
#define PEDESTAL		(1 << 6)
#define OnlyY			(1 << 7)
#define SelCFilter		(1 << 8)
#define SelYFilter		(1 << 9)
#define LockMode		(1 << 10)
#define SDPaternGENSyncMatch	(1 << 11)
#define SDPaternGENON		(1 << 12)
#define SelNTPAL(x)		(((x) & 0x3) << 13)
#define CVBS_ENC_SEL		(1 << 15)
#define RGB_SEQ(x)		(((x) & 0x7) << 16)
#define HDMIChipSelect		(1 << 19)
#define xvYCC_ON(x)		(((x) & 0x3) << 20)
#define BIT10_SEL		(1 << 22)
#define HDMI_Ycbcr		(1 << 23)
#define	BT656_BIT_SEL		(1 << 24)
#define BT656_CBCR_SEL		(1 << 25)
#define VSYNC_INV		(1 << 26)
#define HSYNC_INV		(1 << 27)
#define FSYNC_INV		(1 << 28)

/*	TV Out Pat Gen		*/
#define HD_PAT_GEN_ON		(1 << 0)
#define HD_PAT_3D_SEL(x)		(((x) & 0x3) << 4)
#define HD_PAT_2D_3D		(1 << 6)
#define HD_PAT_TEST_BAR_SEL	(1 << 7)
#define HD_PAT_FRAME_RATE(x)		(((x) & 0x3) << 8)
#define HD_PAT_INT_PROG		(1 << 10)
#define HD_PAT_BIT_SEL		(1 << 11)
#define HD_PAT_BOX_DIST(x)		(((x) & 0xff) << 12)
#define LSI_DAC_APB_TIMING(x)		(((x) & 0x3) << 20)

/*			MCHDMA				*/

/*	TV VIDEO Ch0 ADDR F0Y	*/
#define F0_Y_ADDR(x)			(((x) & 0xffffffff) << 0)

/*	TV VIDEO Ch0 ADDR F0C	*/
#define F0_C_ADDR(x)			(((x) & 0xffffffff) << 0)

/*	TV Video Ch0 ADDR F1Y	*/
#define F1_Y_ADDR(x)			(((x) & 0xffffffff) << 0)

/*	TV Video Ch0 ADDR F1C	*/
#define F1_C_ADDR(x)			(((x) & 0xffffffff) << 0)

/*	TV Video Ch0 HPos	*/
#define	H_START(x)			(((x) & 0xfff) << 0)
#define H_END(x)			(((x) & 0xfff) << 16)

/*	TV Video Ch0 VPos	*/
#define V_START(x)			(((x) & 0xfff) << 0)
#define V_END(x)			(((x) & 0xfff) << 16)

/*          TV VIDEO POS H    */
#define D4_DP_TV_POS_H_START(val, x) \
		SET_REGISTER_VALUE(val, x, 0, 12)
#define D4_DP_TV_POS_H_END(val, x) \
		SET_REGISTER_VALUE(val, x, 16, 12)

/*          TV VIDEO POS V    */
#define D4_DP_TV_POS_V_START(val, x) \
		SET_REGISTER_VALUE(val, x, 0, 12)
#define D4_DP_TV_POS_V_END(val, x) \
		SET_REGISTER_VALUE(val, x, 16, 12)

/*	TV BB HPos		*/
#define START0(x)			(((x) & 0xfff) << 0)
#define END0(x)				(((x) & 0xfff) << 16)

/*	TV BB VPos		*/
#define START0(x)			(((x) & 0xfff) << 0)
#define END0(x)                         (((x) & 0xfff) << 16)

/*	TV BB On		*/
#define BB_MODE_ON0	(1 << 0)
#define BB_MODE_ON1     (1 << 1)
#define BB_MODE_ON2     (1 << 2)
#define BB_MODE_ON3     (1 << 3)
#define BB_MODE_ON4     (1 << 4)
#define BB_MODE_ON5     (1 << 5)
#define BB_MODE_ON6     (1 << 6)
#define BB_MODE_ON7     (1 << 7)
#define BB_MODE_ON8     (1 << 8)
#define BB_MODE_ON9     (1 << 9)
#define BB_MODE_ON10    (1 << 10)
#define BB_MODE_ON11    (1 << 11)
#define BB_MODE_ON12    (1 << 12)
#define BB_MODE_ON13    (1 << 13)
#define BB_MODE_ON14    (1 << 14)
#define BB_MODE_ON15	(1 << 15)
#define BB_DISPLAY_ON0	(1 << 16)
#define BB_DISPLAY_ON1  (1 << 17)
#define BB_DISPLAY_ON2  (1 << 18)
#define BB_DISPLAY_ON3  (1 << 19)
#define BB_DISPLAY_ON4  (1 << 20)
#define BB_DISPLAY_ON5  (1 << 21)
#define BB_DISPLAY_ON6  (1 << 22)
#define BB_DISPLAY_ON7  (1 << 23)
#define BB_DISPLAY_ON8  (1 << 24)
#define BB_DISPLAY_ON9  (1 << 25)
#define BB_DISPLAY_ON10  (1 << 26)
#define BB_DISPLAY_ON11  (1 << 27)
#define BB_DISPLAY_ON12  (1 << 28)
#define BB_DISPLAY_ON13  (1 << 29)
#define BB_DISPLAY_ON14  (1 << 30)
#define BB_DISPLAY_ON15  (1 << 31)

/*	TV Bb Color	*/
#define BB_COLOR0_B(x)		(((x) & 0xff) << 0)
#define BB_COLOR0_G(x)		(((x) & 0xff) << 8)
#define BB_COLOR0_R(x)		(((x) & 0xff) << 16)

/*	TV BB Color Box	*/
#define COLOR_BOX00(x)		(((x) & 0x3) << 0)
#define COLOR_BOX01(x)          (((x) & 0x3) << 2)
#define COLOR_BOX02(x)          (((x) & 0x3) << 4)
#define COLOR_BOX03(x)          (((x) & 0x3) << 6)
#define COLOR_BOX04(x)          (((x) & 0x3) << 8)
#define COLOR_BOX05(x)          (((x) & 0x3) << 10)
#define COLOR_BOX06(x)          (((x) & 0x3) << 12)
#define COLOR_BOX07(x)          (((x) & 0x3) << 14)
#define COLOR_BOX08(x)          (((x) & 0x3) << 16)
#define COLOR_BOX09(x)          (((x) & 0x3) << 18)
#define COLOR_BOX10(x)          (((x) & 0x3) << 20)
#define COLOR_BOX11(x)          (((x) & 0x3) << 22)
#define COLOR_BOX12(x)          (((x) & 0x3) << 24)
#define COLOR_BOX13(x)          (((x) & 0x3) << 26)
#define COLOR_BOX14(x)          (((x) & 0x3) << 28)
#define COLOR_BOX15(x)          (((x) & 0x3) << 30)

/*	TV Bb Alpha	*/
#define BB_ALPHA(x)		(((x) & 0xff) << 0)
#define BB_H_WIDTH(x)		(((x) & 0xf) << 8)
#define BB_V_WIDTH(x)		(((x) & 0xf) << 12)
#define BOUNDARY_ON0	(1 << 16)
#define BOUNDARY_ON1    (1 << 17)
#define BOUNDARY_ON2    (1 << 18)
#define BOUNDARY_ON3    (1 << 19)
#define BOUNDARY_ON4    (1 << 20)
#define BOUNDARY_ON5    (1 << 21)
#define BOUNDARY_ON6    (1 << 22)
#define BOUNDARY_ON7    (1 << 23)
#define BOUNDARY_ON8    (1 << 24)
#define BOUNDARY_ON9    (1 << 25)
#define BOUNDARY_ON10    (1 << 26)
#define BOUNDARY_ON11    (1 << 27)
#define BOUNDARY_ON12    (1 << 28)
#define BOUNDARY_ON13    (1 << 29)
#define BOUNDARY_ON14    (1 << 30)
#define BOUNDARY_ON15    (1 << 31)
#define BOUNDARY_ON16    (1 << 32)

/*	TV BB Boundary Color	*/
#define BB_BOUNDAR_COLOR_B(x)		(((x) & 0xff) << 0)
#define BB_BOUNDAR_COLOR_G(x)		(((x) & 0xff) << 8)
#define BB_BOUNDAR_COLOR_R(x)           (((x) & 0xff) << 16)

/************************** GRAPHIC PATH *****************************/

/*	TV Graphic DMA Ctrl	*/
#define Win0_On	(1 << 0)
#define Win1_On	(1 << 1)
#define Win2_On	(1 << 2)
#define Win3_On	(1 << 3)
#define WD0_ADDR_SWAP	(1 << 4)
#define WD1_ADDR_SWAP	(1 << 5)
#define WD2_ADDR_SWAP	(1 << 6)
#define WD3_ADDR_SWAP	(1 << 7)
#define WD0_ALPHA_ON	(1 << 8)
#define WD1_ALPHA_ON	(1 << 9)
#define WD2_ALPHA_ON	(1 << 10)
#define WD3_ALPHA_ON	(1 << 11)
#define GRP_ARGB_ORDER	(1 << 12)

/*	TV Graphic DMA SCL	*/
#define V_ZOOM	(1 << 0)
#define H_ZOOM	(1 << 1)

/*          TV GRAPHIC DMA SCL   */
#define D4_DP_TV_DMA_ZOOM_V(val, x) \
	SET_REGISTER_VALUE(val, x, 0, 1)
#define D4_DP_TV_DMA_ZOOM_H(val, x) \
	SET_REGISTER_VALUE(val, x, 1, 1)

/*	TV Graphic DMA BCK Color	*/
#define D4_DP_TV_DMA_BKG_R(val, x) \
	SET_REGISTER_VALUE(val, x, 0, 8)
#define D4_DP_TV_DMA_BKG_G(val, x) \
	SET_REGISTER_VALUE(val, x, 8, 8)
#define D4_DP_TV_DMA_BKG_B(val, x) \
	SET_REGISTER_VALUE(val, x, 16, 8)
#define D4_DP_TV_DMA_BKG_A(val, x) \
	SET_REGISTER_VALUE(val, x, 24, 8)

/*	TV Graphic Flat Alpha	*/
#define D4_DP_TV_GRP_WIN0_ALPHA(val, x) \
		SET_REGISTER_VALUE(val, x, 0, 8)
#define D4_DP_TV_GRP_WIN1_ALPHA(val, x) \
		SET_REGISTER_VALUE(val, x, 8, 8)
#define D4_DP_TV_GRP_WIN2_ALPHA(val, x) \
		SET_REGISTER_VALUE(val, x, 16, 8)
#define D4_DP_TV_GRP_WIN3_ALPHA(val, x) \
		SET_REGISTER_VALUE(val, x, 24, 8)

#define WD0_A(x)                (((x) & 0xff) << 0)
#define WD1_A(x)                (((x) & 0xff) << 8)
#define WD2_A(x)                (((x) & 0xff) << 16)
#define WD3_A(x)                (((x) & 0xff) << 24)

/*	TV Graphic Window Priority	*/
#define WD0_PRIORITY(x)		(((x) & 0x3) << 0)
#define WD1_PRIORITY(x)         (((x) & 0x3) << 2)
#define WD2_PRIORITY(x)         (((x) & 0x3) << 4)
#define WD3_PRIORITY(x)         (((x) & 0x3) << 6)

/*	TV Graphic Ch0 ADDR F0	*/
#define F0_ARGB_ADDR(x)		(((x) & 0xffffffff) << 0)

/*	TV Graphic Ch0 ADDR F1	*/
#define F1_ARGB_ADDR(x)		(((x) & 0xffffffff) << 0)

/*	TV Graphic Ch0 HPos	*/
#define H_START(x)		(((x) & 0xfff) << 0)
#define H_END(x)		(((x) & 0xfff) << 16)

/*	TV Graphic Ch0 VPos	*/
#define V_START(x)		(((x) & 0xfff) << 0)
#define V_END(x)		(((x) & 0xfff) << 16)

#endif /* _DP_TV_REG_ */
