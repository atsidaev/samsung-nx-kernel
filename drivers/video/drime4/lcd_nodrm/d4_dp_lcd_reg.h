/**
 * @file d4_dp_lcd_reg.h
 * @brief DRIMe4 DP LCD Register Define for Device Driver
 * @author sejong oh<sejong55.oh@samsung.com>
 * 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef _DP_LCD_REG_
#define _DP_LCD_REG_

#include <mach/d4_reg_macro.h>

/* LCD TG Control Register */
#define DP_LCD_TG_SIZE                  0x000
#define DP_LCD_TG_HSYNC                 0x004
#define DP_LCD_TG_F0_VSYNC              0x008
#define DP_LCD_TG_F1_VSYNC              0x00C
#define DP_LCD_TG_FSYNC0                0x010
#define DP_LCD_TG_FVSYNC                0x014
#define DP_LCD_TG_INFO0                 0x018
#define DP_LCD_TG_INFO1                 0x01C
#define DP_LCD_TG_INFO2                 0x020
#define DP_LCD_TG_ACTIVE_PIXEL          0x024
#define DP_LCD_TG_ACTIVE_LINE0          0x028
#define DP_LCD_TG_ACTIVE_LINE1          0x02C

/* LCD Path Control Register */

#define DP_LCD_PATH_CTRL                0x100
#define DP_LCD_SIZE                     0x104
#define DP_LCD_STRIDE					0x108
#define DP_LCD_DITHER					0x10c
#define DP_LCD_HRZ_FLT                  0x110
#define DP_LCD_DMA_MODE0                0x114
#define DP_LCD_DMA_MODE1                0x118
#define DP_LCD_DMA_MODE2                0x11C
#define DP_LCD_GRP_MIX_ON               0x120
#define DP_LCD_MIX_ALPHA_CTRL           0x124
#define DP_LCD_GRP_MIX_CH0              0x128
#define DP_LCD_GRP_MIX_CH1              0x12C
#define DP_LCD_R_RANGE_CH0              0x130
#define DP_LCD_G_RANGE_CH0              0x134
#define DP_LCD_B_RANGE_CH0              0x138
#define DP_LCD_GRP_MIX_LIMIT_R          0x13C
#define DP_LCD_GRP_MIX_LIMIT_G          0x140
#define DP_LCD_GRP_MIX_LIMIT_B          0x144
#define DP_LCD_ZEBRA_CTRL0              0x148
#define DP_LCD_ZEBRA_CTRL1              0x14C

#define DP_LCD_CSC0                     0x150
#define DP_LCD_CSC1                     0x154
#define DP_LCD_CSC2                     0x158
#define DP_LCD_CSC3                     0x15C
#define DP_LCD_CSC4                     0x160
#define DP_LCD_CSC5                     0x164
#define DP_LCD_CSC6                     0x168
#define DP_LCD_CSC7                     0x16C

#define DP_GRP_CSC0                     0x170
#define DP_GRP_CSC1                     0x174
#define DP_GRP_CSC2                     0x178
#define DP_GRP_CSC3                     0x17C
#define DP_GRP_CSC4                     0x180
#define DP_GRP_CSC5                     0x184
#define DP_GRP_CSC6                     0x188
#define DP_GRP_CSC7                     0x18C
#define DP_GRP_CSC8                     0x190
#define DP_TCC_LUT                      0x194
#define DP_TCC_RD_ADDR                  0x198
#define DP_TCC_RETURN_CH                0x19C

/* LCD Control Register */

#define DP_LCD_OUT_CTRL                 0x1A0
#define DP_LCD_OUT_DISP_SEQ             0x1A4
#define DP_LCD_OUT_INV                  0x1A8
#define DP_LCD_OUT_SAMPLING             0x1AC
#define DP_LCD_OUT_SAMP_SEQ             0x1B0
#define DP_LCD_OUT_BKG_COLOR            0x1B4
#define DP_LCD_OUT_HRZ_FLT_ON           0x1C0
#define DP_LCD_OUT_HRZ_FLT_R            0x1C4
#define DP_LCD_OUT_HRZ_FLT_G            0x1C8
#define DP_LCD_OUT_HRZ_FLT_B            0x1CC

/* LCD Video Window Control Register */

#define DP_LCD_WIN0_F0_Y_ADDR           0x200
#define DP_LCD_WIN0_F0_C_ADDR           0x204
#define DP_LCD_WIN0_F1_Y_ADDR           0x208
#define DP_LCD_WIN0_F1_C_ADDR           0x20C
#define DP_LCD_WIN0_H_POS               0x210
#define DP_LCD_WIN0_V_POS               0x214
#define DP_LCD_BB00_H_POS               0x218
#define DP_LCD_BB00_V_POS               0x21C

#define DP_LCD_WIN1_F0_Y_ADDR           0x220
#define DP_LCD_WIN1_F0_C_ADDR           0x224
#define DP_LCD_WIN1_F1_Y_ADDR           0x228
#define DP_LCD_WIN1_F1_C_ADDR           0x22C
#define DP_LCD_WIN1_H_POS               0x230
#define DP_LCD_WIN1_V_POS               0x234
#define DP_LCD_BB01_H_POS               0x238
#define DP_LCD_BB01_V_POS               0x23C

#define DP_LCD_WIN2_F0_Y_ADDR           0x240
#define DP_LCD_WIN2_F0_C_ADDR           0x244
#define DP_LCD_WIN2_F1_Y_ADDR           0x248
#define DP_LCD_WIN2_F1_C_ADDR           0x24C
#define DP_LCD_WIN2_H_POS               0x250
#define DP_LCD_WIN2_V_POS               0x254
#define DP_LCD_BB02_H_POS               0x258
#define DP_LCD_BB02_V_POS               0x25C

#define DP_LCD_WIN3_F0_Y_ADDR           0x260
#define DP_LCD_WIN3_F0_C_ADDR           0x264
#define DP_LCD_WIN3_F1_Y_ADDR           0x268
#define DP_LCD_WIN3_F1_C_ADDR           0x26C
#define DP_LCD_WIN3_H_POS               0x270
#define DP_LCD_WIN3_V_POS               0x274
#define DP_LCD_BB03_H_POS               0x278
#define DP_LCD_BB03_V_POS               0x27C

#define DP_LCD_BB04_H_POS               0x280
#define DP_LCD_BB04_V_POS               0x284
#define DP_LCD_BB05_H_POS               0x288
#define DP_LCD_BB05_V_POS               0x28C
#define DP_LCD_BB06_H_POS               0x290
#define DP_LCD_BB06_V_POS               0x294
#define DP_LCD_BB07_H_POS               0x298
#define DP_LCD_BB07_V_POS               0x29C
#define DP_LCD_BB08_H_POS               0x2A0
#define DP_LCD_BB08_V_POS               0x2A4
#define DP_LCD_BB09_H_POS               0x2A8
#define DP_LCD_BB09_V_POS               0x2AC
#define DP_LCD_BB10_H_POS               0x2B0
#define DP_LCD_BB10_V_POS               0x2B4
#define DP_LCD_BB11_H_POS               0x2B8
#define DP_LCD_BB11_V_POS               0x2BC
#define DP_LCD_BB12_H_POS               0x2C0
#define DP_LCD_BB12_V_POS               0x2C4
#define DP_LCD_BB13_H_POS               0x2C8
#define DP_LCD_BB13_V_POS               0x2CC
#define DP_LCD_BB14_H_POS               0x2D0
#define DP_LCD_BB14_V_POS               0x2D4
#define DP_LCD_BB15_H_POS               0x2D8
#define DP_LCD_BB15_V_POS               0x2DC

#define DP_LCD_BB_MODE_ON               0x2E0
#define DP_LCD_BB_COLOR_0               0x2E4
#define DP_LCD_BB_COLOR_1               0x2E8
#define DP_LCD_BB_COLOR_2               0x2EC
#define DP_LCD_BB_COLOR_3               0x2F0
#define DP_LCD_BB_COLOR_SELECT          0x2F4
#define DP_LCD_BB_WIDTH_ALPHA           0x2F8
#define DP_LCD_BB_OUTLINE_COLOR         0x2FC

/* LCD Graphic1 Path Control Register */

#define DP_LCD_GRP1_DMA_CTRL            0x300
#define DP_LCD_GRP1_DMA_SCALER          0x304
#define DP_LCD_GRP1_BKG                 0x30C
#define DP_LCD_GRP1_WIN_FLAT_ALPHA      0x310
#define DP_LCD_GRP1_WIN_PRIORITY        0x318

#define DP_LCD_GRP1_WIN0_F0_ADDR        0x320
#define DP_LCD_GRP1_WIN0_F1_ADDR        0x324
#define DP_LCD_GRP1_WIN0_H_POS          0x328
#define DP_LCD_GRP1_WIN0_V_POS          0x32C

#define DP_LCD_GRP1_WIN1_F0_ADDR        0x330
#define DP_LCD_GRP1_WIN1_F1_ADDR        0x334
#define DP_LCD_GRP1_WIN1_H_POS          0x338
#define DP_LCD_GRP1_WIN1_V_POS          0x33C

#define DP_LCD_GRP1_WIN2_F0_ADDR        0x340
#define DP_LCD_GRP1_WIN2_F1_ADDR        0x344
#define DP_LCD_GRP1_WIN2_H_POS          0x348
#define DP_LCD_GRP1_WIN2_V_POS          0x34C

#define DP_LCD_GRP1_WIN3_F0_ADDR        0x350
#define DP_LCD_GRP1_WIN3_F1_ADDR        0x354
#define DP_LCD_GRP1_WIN3_H_POS          0x358
#define DP_LCD_GRP1_WIN3_V_POS          0x35C

#define DP_LCD_GRP1_HRZ_FLT             0x360

/*GM CTRL*/
#define DP_LCD_GM_CTRL					0x400

#define DP_LCD_GM_LUX				    0x404
#define DP_LCD_GM_FINE				    0x408
#define DP_LCD_GM_GAIN				    0x40C
#define DP_LCD_GM_HTH				    0x410

#define DP_LCD_GM_DELGB				    0x414
#define DP_LCD_GM_DELRB				    0x418
#define DP_LCD_GM_DELGR				    0x41C

#define DP_LCD_GM_FIC0				    0x420
#define DP_LCD_GM_FIC1			    	0x424
#define DP_LCD_GM_FIC2			    	0x428
#define DP_LCD_GM_FIC3			    	0x42c
#define DP_LCD_GM_FIC4			    	0x430

#define DP_LCD_GM_OLED0				    0x434
#define DP_LCD_GM_OLED1			    	0x438
#define DP_LCD_GM_OLED2			    	0x43C
#define DP_LCD_GM_OLED3			    	0x440
#define DP_LCD_GM_OLED4			    	0x444

/************************************************/
/*          Register Structure Define           */
/************************************************/

/*          LCD TG SIZE           */
#define D4_DP_LCD_TOTAL_PIXEL_NUM(val, x) \
	SET_REGISTER_VALUE(val, x, 0, 12)
#define D4_DP_LCD_TOTAL_LINE_NUM(val, x) \
	SET_REGISTER_VALUE(val, x, 16, 12)

/*          LCD TG HSIZE           */
#define D4_DP_LCD_H_SYNC_BLK(val, x) \
	SET_REGISTER_VALUE(val, x, 0, 12)
#define D4_DP_LCD_H_SYNC_ACT(val, x) \
	SET_REGISTER_VALUE(val, x, 16, 12)

/*          LCD TG F  VSYNC           */
#define D4_DP_LCD_V_SYNC_BLK0(val, x) \
	SET_REGISTER_VALUE(val, x, 0, 12)
#define D4_DP_LCD_V_SYNC_ACT0(val, x) \
	SET_REGISTER_VALUE(val, x, 16, 12)

/*          LCD TG PRELINE 0           */
#define D4_DP_LCD_FRAME_INIT_LINE(val, x) \
	SET_REGISTER_VALUE(val, x, 0, 12)
#define D4_DP_LCD_FRAME_FINAL_LINE(val, x) \
	SET_REGISTER_VALUE(val, x, 16, 12)

/*          LCD TG PRELINE 1           */
#define D4_DP_LCD_FIELD_INIT_LINE(val, x) \
	SET_REGISTER_VALUE(val, x, 0, 12)
#define D4_DP_LCD_BUF_READ_START_PIXEL(val, x) \
	SET_REGISTER_VALUE(val, x, 16, 12)

/*          LCD TG PREPIXEL           */
#define D4_DP_LCD_PRE_PIXEL_START(val, x) \
	SET_REGISTER_VALUE(val, x, 0, 12)
#define D4_DP_LCD_PRE_PIXEL_END(val, x) \
	SET_REGISTER_VALUE(val, x, 16, 12)

/*          LCD TG PIXEL ACTIVE           */
#define D4_DP_LCD_ACTIVE_PIXEL_START(val, x) \
	SET_REGISTER_VALUE(val, x, 0, 12)
#define D4_DP_LCD_ACTIVE_PIXEL_END(val, x) \
		SET_REGISTER_VALUE(val, x, 16, 12)

/*          LCD TG LINE ACTIVE F          */
#define D4_DP_LCD_ACTIVE_LINE_START(val, x) \
	SET_REGISTER_VALUE(val, x, 0, 12)
#define D4_DP_LCD_ACTIVE_LINE_END(val, x) \
	SET_REGISTER_VALUE(val, x, 16, 12)

/*          LCD PATH CTRL          */
#define D4_DP_LCD_PATH_DMA_ON(val, x) \
    SET_REGISTER_VALUE(val, x, 0, 1)

#define D4_DP_LCD_PATH_HRZ_FILTER_ON(val, x) \
    SET_REGISTER_VALUE(val, x, 1, 1)

#define D4_DP_LCD_PATH_GRP_MIX_ON(val, x) \
    SET_REGISTER_VALUE(val, x, 2, 1)

#define D4_DP_LCD_PATH_GRP_RGB2YCBCR_ON(val, x) \
    SET_REGISTER_VALUE(val, x, 3, 1)

#define D4_DP_LCD_PATH_GRP_ZEBRA_ON(val, x) \
    SET_REGISTER_VALUE(val, x, 4, 1)

#define D4_DP_LCD_PATH_GRP_DITHER_ON(val, x) \
    SET_REGISTER_VALUE(val, x, 6, 1)

#define D4_DP_LCD_PATH_GRP_DMA_CTRL_ON(val, x) \
    SET_REGISTER_VALUE(val, x, 8, 1)

#define D4_DP_LCD_PATH_GRP_HRZ_FILTER_ON(val, x) \
    SET_REGISTER_VALUE(val, x, 9, 1)

#define D4_DP_LCD_PATH_OLED_GM_ON(val, x) \
    SET_REGISTER_VALUE(val, x, 12, 1)

#define D4_DP_LCD_PATH_TCC_ON(val, x) \
    SET_REGISTER_VALUE(val, x, 13, 1)

#define D4_DP_LCD_PATH_3D_ON(val, x) \
    SET_REGISTER_VALUE(val, x, 16, 1)

#define D4_DP_LCD_PATH_FORMAT_3D(val, x) \
    SET_REGISTER_VALUE(val, x, 17, 2)

#define D4_DP_LCD_PATH_SD_ON(val, x) \
    SET_REGISTER_VALUE(val, x, 19, 1)

#define D4_DP_LCD_PATH_YCBCR422_ON(val, x) \
    SET_REGISTER_VALUE(val, x, 20, 1)

#define D4_DP_LCD_PATH_ROTATE_ON(val, x) \
    SET_REGISTER_VALUE(val, x, 21, 1)

#define D4_DP_LCD_PATH_VID_10BIT_ON(val, x) \
    SET_REGISTER_VALUE(val, x, 22, 1)

#define D4_DP_LCD_PATH_TCC_VID_ON(val, x) \
    SET_REGISTER_VALUE(val, x, 23, 1)

#define D4_DP_LCD_PATH_VID_SCAN_ON(val, x) \
    SET_REGISTER_VALUE(val, x, 24, 1)

#define D4_DP_LCD_PATH_GRP_SCAN_ON(val, x) \
    SET_REGISTER_VALUE(val, x, 25, 1)

#define D4_DP_LCD_PATH_BBOX_2X_ON(val, x) \
    SET_REGISTER_VALUE(val, x, 26, 1)

#define D4_DP_LCD_PATH_LINE_INV_ON(val, x) \
    SET_REGISTER_VALUE(val, x, 27, 1)

#define D4_DP_LCD_PATH_LINE_VID_H_SWAP_ON(val, x) \
    SET_REGISTER_VALUE(val, x, 28, 1)

#define D4_DP_LCD_PATH_LINE_VID_V_SWAP_ON(val, x) \
    SET_REGISTER_VALUE(val, x, 29, 1)

#define D4_DP_LCD_PATH_LINE_GRP_H_SWAP_ON(val, x) \
    SET_REGISTER_VALUE(val, x, 30, 1)

#define D4_DP_LCD_PATH_LINE_GRP_V_SWAP_ON(val, x) \
    SET_REGISTER_VALUE(val, x, 31, 1)

/*          LCD PATH RESOLUTION          */
#define D4_DP_LCD_PATH_SIZE_H(val, x) \
    SET_REGISTER_VALUE(val, x, 0, 11)
#define D4_DP_LCD_PATH_SIZE_V(val, x) \
	SET_REGISTER_VALUE(val, x, 16, 11)

/*          LCD PATH STRIDE         */
#define D4_DP_LCD_PATH_VID_STRIDE(val, x) \
	SET_REGISTER_VALUE(val, x, 0, 11)
#define D4_DP_LCD_PATH_GRP_STRIDE(val, x) \
	SET_REGISTER_VALUE(val, x, 16, 11)

/*          LCD PATH VID HRZFLT         */
#define D4_DP_LCD_PATH_T0_COEF(val, x) \
	SET_REGISTER_VALUE(val, x, 0, 3)
#define D4_DP_LCD_PATH_T1_COEF(val, x) \
	SET_REGISTER_VALUE(val, x, 4, 3)
#define D4_DP_LCD_PATH_T2_COEF(val, x) \
	SET_REGISTER_VALUE(val, x, 8, 3)
#define D4_DP_LCD_PATH_T3_COEF(val, x) \
	SET_REGISTER_VALUE(val, x, 12, 3)
#define D4_DP_LCD_PATH_T4_COEF(val, x) \
	SET_REGISTER_VALUE(val, x, 16, 3)
#define D4_DP_LCD_PATH_T0_SIGN(val, x) \
	SET_REGISTER_VALUE(val, x, 20, 1)
#define D4_DP_LCD_PATH_T1_SIGN(val, x) \
	SET_REGISTER_VALUE(val, x, 21, 1)
#define D4_DP_LCD_PATH_T3_SIGN(val, x) \
	SET_REGISTER_VALUE(val, x, 22, 1)
#define D4_DP_LCD_PATH_T4_SIGN(val, x) \
	SET_REGISTER_VALUE(val, x, 23, 1)
#define D4_DP_LCD_PATH_POST_COEF(val, x) \
	SET_REGISTER_VALUE(val, x, 24, 2)

/*          LCD PATH DMA M1         */
#define D4_DP_LCD_PATH_Y_MOALEN(val, x) \
	SET_REGISTER_VALUE(val, x, 0, 3)
#define D4_DP_LCD_Y_8BURST(val, x) \
    SET_REGISTER_VALUE(val, x, 3, 1)
#define D4_DP_LCD_PATH_C_MOALEN(val, x) \
	SET_REGISTER_VALUE(val, x, 4, 3)
#define D4_DP_LCD_C_8BURST(val, x) \
    SET_REGISTER_VALUE(val, x, 7, 1)
#define D4_DP_LCD_PATH_G_MOALEN(val, x) \
	SET_REGISTER_VALUE(val, x, 8, 3)
#define D4_DP_LCD_G_8BURST(val, x) \
    SET_REGISTER_VALUE(val, x, 11, 1)

/*          LCD PATH DMA M1         */
#define D4_DP_LCD_PATH_BKG_Y(val, x) \
    SET_REGISTER_VALUE(val, x, 0, 8)
#define D4_DP_LCD_PATH_BKG_Cb(val, x) \
	SET_REGISTER_VALUE(val, x, 8, 8)
#define D4_DP_LCD_PATH_BKG_Cr(val, x) \
	SET_REGISTER_VALUE(val, x, 16, 8)

/*          LCD PATH GRP MIX ALP FUNC         */
#define D4_DP_LCD_ALPHA_ON(val, x) \
    SET_REGISTER_VALUE(val, x, 0, 1)
#define D4_DP_LCD_LIMIT_ON(val, x) \
    SET_REGISTER_VALUE(val, x, 1, 1)
#define D4_DP_LCD_RANGE_DET_VID(val, x) \
	SET_REGISTER_VALUE(val, x, 4, 1)
#define D4_DP_LCD_RANGE_DET_GRP(val, x) \
	SET_REGISTER_VALUE(val, x, 5, 1)
#define D4_DP_LCD_RANGE_TARGET(val, x) \
	SET_REGISTER_VALUE(val, x, 8, 1)
#define D4_DP_LCD_CH_SWAP(val, x) \
	SET_REGISTER_VALUE(val, x, 12, 1)
#define D4_DP_LCD_FLAG_R_ON(val, x) \
	SET_REGISTER_VALUE(val, x, 16, 1)
#define D4_DP_LCD_FLAG_G_ON(val, x) \
	SET_REGISTER_VALUE(val, x, 17, 1)
#define D4_DP_LCD_FLAG_B_ON(val, x) \
	SET_REGISTER_VALUE(val, x, 18, 1)

/*          LCD PATH GRP MIX ALP CTRL         */
#define D4_DP_LCD_GRP_ALP_ON(val, x) \
    SET_REGISTER_VALUE(val, x, 0, 1)
#define D4_DP_LCD_GRP_ALP_INV_ON(val, x) \
    SET_REGISTER_VALUE(val, x, 1, 1)
#define D4_DP_LCD_GRP_ALP_OFS(val, x) \
	SET_REGISTER_VALUE(val, x, 2, 1)
#define D4_DP_LCD_PATH_ALP_OFS_4BIT(val, x) \
    SET_REGISTER_VALUE(val, x, 8, 4)
#define D4_DP_LCD_PATH_CKEY_ALP_8BIT(val, x) \
    SET_REGISTER_VALUE(val, x, 16, 8)

/*          LCD PATH GRP MIX CH0         */
#define D4_DP_LCD_GRP_MIX_R_ON(val, x) \
    SET_REGISTER_VALUE(val, x, 0, 1)
#define D4_DP_LCD_GRP_MIX_G_ON(val, x) \
    SET_REGISTER_VALUE(val, x, 1, 1)
#define D4_DP_LCD_GRP_MIX_B_ON(val, x) \
    SET_REGISTER_VALUE(val, x, 2, 1)
#define D4_DP_LCD_GRP_MIX_R_INV(val, x) \
    SET_REGISTER_VALUE(val, x, 4, 1)
#define D4_DP_LCD_GRP_MIX_G_INV(val, x) \
    SET_REGISTER_VALUE(val, x, 5, 1)
#define D4_DP_LCD_GRP_MIX_B_INV(val, x) \
    SET_REGISTER_VALUE(val, x, 6, 1)
#define D4_DP_LCD_GRP_MIX_R_OFFSET(val, x) \
    SET_REGISTER_VALUE(val, x, 8, 1)
#define D4_DP_LCD_GRP_MIX_G_OFFSET(val, x) \
    SET_REGISTER_VALUE(val, x, 9, 1)
#define D4_DP_LCD_GRP_MIX_B_OFFSET(val, x) \
    SET_REGISTER_VALUE(val, x, 10, 1)
#define D4_DP_LCD_GRP_MIX_R_OFFSET_VALUE(val, x) \
    SET_REGISTER_VALUE(val, x, 12, 4)
#define D4_DP_LCD_GRP_MIX_G_OFFSET_VALUE(val, x) \
    SET_REGISTER_VALUE(val, x, 16, 4)
#define D4_DP_LCD_GRP_MIX_B_OFFSET_VALUE(val, x) \
    SET_REGISTER_VALUE(val, x, 20, 4)
#define D4_DP_LCD_GRP_MIX_R_SIGN(val, x) \
    SET_REGISTER_VALUE(val, x, 24, 1)
#define D4_DP_LCD_GRP_MIX_G_SIGN(val, x) \
    SET_REGISTER_VALUE(val, x, 25, 1)
#define D4_DP_LCD_GRP_MIX_B_SIGN(val, x) \
    SET_REGISTER_VALUE(val, x, 26, 1)

/*          LCD PATH GRP MIX RANGE   & LIMIT      */
#define D4_DP_LCD_GRP_MIX_RANGE_LOW(val, x) \
    SET_REGISTER_VALUE(val, x, 0, 8)
#define D4_DP_LCD_GRP_MIX_RANGE_UP(val, x) \
    SET_REGISTER_VALUE(val, x, 16, 8)

/*          LCD PATH ZEBRA CTRL0         */
#define D4_DP_LCD_ZEBRA_THSLD_LOW(val, x) \
    SET_REGISTER_VALUE(val, x, 0, 8)
#define D4_DP_LCD_ZEBRA_THSLD_UP(val, x) \
	SET_REGISTER_VALUE(val, x, 8, 8)
#define D4_DP_LCD_ZEBRA_ANGLE(val, x) \
	SET_REGISTER_VALUE(val, x, 16, 4)
#define D4_DP_LCD_ZEBRA_SPEED(val, x) \
	SET_REGISTER_VALUE(val, x, 20, 4)
#define D4_DP_LCD_ZEBRA_WIDTH(val, x) \
	SET_REGISTER_VALUE(val, x, 24, 2)
#define D4_DP_LCD_ZEBRA_ALPHA(val, x) \
	SET_REGISTER_VALUE(val, x, 26, 2)

/*          LCD PATH ZEBRA CTRL1         */
#define D4_DP_LCD_ZEBRA_R_PATTERN(val, x) \
	SET_REGISTER_VALUE(val, x, 0, 4)
#define D4_DP_LCD_ZEBRA_G_PATTERN(val, x) \
	SET_REGISTER_VALUE(val, x, 4, 4)
#define D4_DP_LCD_ZEBRA_B_PATTERN(val, x) \
	SET_REGISTER_VALUE(val, x, 8, 4)

/*          LCD PATH YCBCR2RGB0         */
#define D4_DP_LCD_YCC2RGB_MTRX0(val, x) \
	SET_REGISTER_VALUE(val, x, 0, 11)
#define D4_DP_LCD_YCC2RGB_MTRX1(val, x) \
	SET_REGISTER_VALUE(val, x, 16, 11)

/*          LCD PATH YCBCR2RGB7         */
#define D4_DP_LCD_YCC2RGB_UNDER_RGB(val, x) \
	SET_REGISTER_VALUE(val, x, 0, 12)
#define D4_DP_LCD_YCC2RGB_OVER_RGB(val, x) \
	SET_REGISTER_VALUE(val, x, 16, 12)

/*          LCD PATH RGB2YCBCR7       */
#define D4_DP_LCD_YCC2RGB_UNDER_Y(val, x) \
	SET_REGISTER_VALUE(val, x, 0, 8)
#define D4_DP_LCD_YCC2RGB_OVER_Y(val, x) \
	SET_REGISTER_VALUE(val, x, 16, 8)

/*          LCD PATH OUT CTRL     */
#define D4_DP_LCD_OUT_CTRL_ON(val, x) \
	SET_REGISTER_VALUE(val, x, 0, 1)
#define D4_DP_LCD_OUT_CTRL_YCBCR(val, x) \
	SET_REGISTER_VALUE(val, x, 2, 1)
#define D4_DP_LCD_OUT_BUS_WIDTH(val, x) \
	SET_REGISTER_VALUE(val, x, 4, 2)
#define D4_DP_LCD_OUT_PATTERN_GEN(val, x) \
	SET_REGISTER_VALUE(val, x, 8, 1)
#define D4_DP_LCD_OUT_8BIT_BYPASS(val, x) \
	SET_REGISTER_VALUE(val, x, 12, 1)
#define D4_DP_LCD_OUT_8BIT_DATA(val, x) \
	SET_REGISTER_VALUE(val, x, 13, 1)
#define D4_DP_LCD_OUT_BT656_ON(val, x) \
	SET_REGISTER_VALUE(val, x, 16, 1)
#define D4_DP_LCD_OUT_BT656_BITSEL(val, x) \
	SET_REGISTER_VALUE(val, x, 17, 1)
#define D4_DP_LCD_OUT_BT656_CBCR_SEL(val, x) \
	SET_REGISTER_VALUE(val, x, 18, 1)
#define D4_DP_LCD_OUT_YCC_SEQ(val, x) \
	SET_REGISTER_VALUE(val, x, 20, 2)

/*          LCD PATH OUT DISP SEQ     */
#define D4_DP_LCD_OUT_DISPLAY_SEQ_ODD(val, x) \
	SET_REGISTER_VALUE(val, x, 0, 3)
#define D4_DP_LCD_OUT_DISPLAY_SEQ_EVEN(val, x) \
	SET_REGISTER_VALUE(val, x, 4, 3)
#define D4_DP_LCD_OUT_MULTI_CLK_TRANS(val, x) \
	SET_REGISTER_VALUE(val, x, 8, 1)

/*          LCD PATH OUT INVERSE     */
#define D4_DP_LCD_INV_DOT_CLK(val, x) \
    SET_REGISTER_VALUE(val, x, 0, 1)
#define D4_DP_LCD_INV_ENABLE(val, x) \
	SET_REGISTER_VALUE(val, x, 1, 1)
#define D4_DP_LCD_INV_H_SYNC(val, x) \
	SET_REGISTER_VALUE(val, x, 2, 1)
#define D4_DP_LCD_INV_V_SYNC(val, x) \
	SET_REGISTER_VALUE(val, x, 3, 1)

/*          LCD VIDEO POS H    */
#define D4_DP_LCD_POS_H_START(val, x) \
    SET_REGISTER_VALUE(val, x, 0, 11)
#define D4_DP_LCD_POS_H_END(val, x) \
    SET_REGISTER_VALUE(val, x, 16, 11)

/*          LCD VIDEO POS V    */
#define D4_DP_LCD_POS_V_START(val, x) \
    SET_REGISTER_VALUE(val, x, 0, 11)
#define D4_DP_LCD_POS_V_END(val, x) \
    SET_REGISTER_VALUE(val, x, 16, 11)

/*          LCD VIDEO BB POS H    */
#define D4_DP_LCD_BB_H_START(val, x) \
    SET_REGISTER_VALUE(val, x, 0, 11)
#define D4_DP_LCD_BB_H_END(val, x) \
	SET_REGISTER_VALUE(val, x, 16, 11)
#define D4_DP_LCD_BB_V_START(val, x) \
    SET_REGISTER_VALUE(val, x, 0, 11)
#define D4_DP_LCD_BB_V_END(val, x) \
	SET_REGISTER_VALUE(val, x, 16, 11)

/*          LCD VIDEO BB ON   */
#define D4_DP_LCD_BB00_MODE_ON(val, x) \
    SET_REGISTER_VALUE(val, x, 0, 1)

#define D4_DP_LCD_BB01_MODE_ON(val, x) \
    SET_REGISTER_VALUE(val, x, 1, 1)

#define D4_DP_LCD_BB02_MODE_ON(val, x) \
    SET_REGISTER_VALUE(val, x, 2, 1)

#define D4_DP_LCD_BB03_MODE_ON(val, x) \
    SET_REGISTER_VALUE(val, x, 3, 1)

#define D4_DP_LCD_BB04_MODE_ON(val, x) \
    SET_REGISTER_VALUE(val, x, 4, 1)

#define D4_DP_LCD_BB05_MODE_ON(val, x) \
    SET_REGISTER_VALUE(val, x, 5, 1)

#define D4_DP_LCD_BB06_MODE_ON(val, x) \
    SET_REGISTER_VALUE(val, x, 6, 1)

#define D4_DP_LCD_BB07_MODE_ON(val, x) \
    SET_REGISTER_VALUE(val, x, 7, 1)

#define D4_DP_LCD_BB08_MODE_ON(val, x) \
    SET_REGISTER_VALUE(val, x, 8, 1)

#define D4_DP_LCD_BB09_MODE_ON(val, x) \
    SET_REGISTER_VALUE(val, x, 9, 1)

#define D4_DP_LCD_BB10_MODE_ON(val, x) \
    SET_REGISTER_VALUE(val, x, 10, 1)

#define D4_DP_LCD_BB11_MODE_ON(val, x) \
    SET_REGISTER_VALUE(val, x, 11, 1)

#define D4_DP_LCD_BB12_MODE_ON(val, x) \
    SET_REGISTER_VALUE(val, x, 12, 1)

#define D4_DP_LCD_BB13_MODE_ON(val, x) \
    SET_REGISTER_VALUE(val, x, 13, 1)

#define D4_DP_LCD_BB14_MODE_ON(val, x) \
    SET_REGISTER_VALUE(val, x, 14, 1)

#define D4_DP_LCD_BB15_MODE_ON(val, x) \
    SET_REGISTER_VALUE(val, x, 15, 1)

#define D4_DP_LCD_BB00_ON(val, x) \
    SET_REGISTER_VALUE(val, x, 16, 1)

#define D4_DP_LCD_BB01_ON(val, x) \
    SET_REGISTER_VALUE(val, x, 17, 1)

#define D4_DP_LCD_BB02_ON(val, x) \
    SET_REGISTER_VALUE(val, x, 18, 1)

#define D4_DP_LCD_BB03_ON(val, x) \
    SET_REGISTER_VALUE(val, x, 19, 1)

#define D4_DP_LCD_BB04_ON(val, x) \
    SET_REGISTER_VALUE(val, x, 20, 1)

#define D4_DP_LCD_BB05_ON(val, x) \
    SET_REGISTER_VALUE(val, x, 21, 1)

#define D4_DP_LCD_BB06_ON(val, x) \
    SET_REGISTER_VALUE(val, x, 22, 1)

#define D4_DP_LCD_BB07_ON(val, x) \
    SET_REGISTER_VALUE(val, x, 23, 1)

#define D4_DP_LCD_BB08_ON(val, x) \
    SET_REGISTER_VALUE(val, x, 24, 1)

#define D4_DP_LCD_BB09_ON(val, x) \
    SET_REGISTER_VALUE(val, x, 25, 1)

#define D4_DP_LCD_BB10_ON(val, x) \
    SET_REGISTER_VALUE(val, x, 26, 1)

#define D4_DP_LCD_BB11_ON(val, x) \
    SET_REGISTER_VALUE(val, x, 27, 1)

#define D4_DP_LCD_BB12_ON(val, x) \
    SET_REGISTER_VALUE(val, x, 28, 1)

#define D4_DP_LCD_BB13_ON(val, x) \
    SET_REGISTER_VALUE(val, x, 29, 1)

#define D4_DP_LCD_BB14_ON(val, x) \
    SET_REGISTER_VALUE(val, x, 30, 1)

#define D4_DP_LCD_BB15_ON(val, x) \
    SET_REGISTER_VALUE(val, x, 31, 1)

/*          LCD VIDEO BB COLOR   */
#define D4_DP_LCD_BB_B_COLOR(val, x) \
    SET_REGISTER_VALUE(val, x, 0, 8)
#define D4_DP_LCD_BB_G_COLOR(val, x) \
	SET_REGISTER_VALUE(val, x, 8, 8)
#define D4_DP_LCD_BB_R_COLOR(val, x) \
	SET_REGISTER_VALUE(val, x, 16, 8)

/*          LCD VIDEO BB COLOR BOX   */
#define D4_DP_LCD_BB_COLOR00(val, x) \
    SET_REGISTER_VALUE(val, x, 0, 2)
#define D4_DP_LCD_BB_COLOR01(val, x) \
    SET_REGISTER_VALUE(val, x, 2, 2)
#define D4_DP_LCD_BB_COLOR02(val, x) \
	SET_REGISTER_VALUE(val, x, 4, 2)
#define D4_DP_LCD_BB_COLOR03(val, x) \
	SET_REGISTER_VALUE(val, x, 6, 2)
#define D4_DP_LCD_BB_COLOR04(val, x) \
	SET_REGISTER_VALUE(val, x, 8, 2)
#define D4_DP_LCD_BB_COLOR05(val, x) \
	SET_REGISTER_VALUE(val, x, 10, 2)
#define D4_DP_LCD_BB_COLOR06(val, x) \
	SET_REGISTER_VALUE(val, x, 12, 2)
#define D4_DP_LCD_BB_COLOR07(val, x) \
	SET_REGISTER_VALUE(val, x, 14, 2)
#define D4_DP_LCD_BB_COLOR08(val, x) \
	SET_REGISTER_VALUE(val, x, 16, 2)
#define D4_DP_LCD_BB_COLOR09(val, x) \
	SET_REGISTER_VALUE(val, x, 18, 2)
#define D4_DP_LCD_BB_COLOR10(val, x) \
	SET_REGISTER_VALUE(val, x, 20, 2)
#define D4_DP_LCD_BB_COLOR11(val, x) \
	SET_REGISTER_VALUE(val, x, 22, 2)
#define D4_DP_LCD_BB_COLOR12(val, x) \
	SET_REGISTER_VALUE(val, x, 24, 2)
#define D4_DP_LCD_BB_COLOR13(val, x) \
	SET_REGISTER_VALUE(val, x, 26, 2)
#define D4_DP_LCD_BB_COLOR14(val, x) \
	SET_REGISTER_VALUE(val, x, 28, 2)
#define D4_DP_LCD_BB_COLOR15(val, x) \
	SET_REGISTER_VALUE(val, x, 30, 2)

#define D4_DP_LCD_BB_COLOR_SELECT_00(val, x) \
    SET_REGISTER_VALUE(val, x, 0, 2)
#define D4_DP_LCD_BB_COLOR_SELECT_01(val, x) \
	SET_REGISTER_VALUE(val, x, 2, 2)
#define D4_DP_LCD_BB_COLOR_SELECT_02(val, x) \
    SET_REGISTER_VALUE(val, x, 4, 2)
#define D4_DP_LCD_BB_COLOR_SELECT_03(val, x) \
	SET_REGISTER_VALUE(val, x, 6, 2)
#define D4_DP_LCD_BB_COLOR_SELECT_04(val, x) \
    SET_REGISTER_VALUE(val, x, 8, 2)
#define D4_DP_LCD_BB_COLOR_SELECT_05(val, x) \
	SET_REGISTER_VALUE(val, x, 10, 2)
#define D4_DP_LCD_BB_COLOR_SELECT_06(val, x) \
    SET_REGISTER_VALUE(val, x, 12, 2)
#define D4_DP_LCD_BB_COLOR_SELECT_07(val, x) \
	SET_REGISTER_VALUE(val, x, 14, 2)
#define D4_DP_LCD_BB_COLOR_SELECT_08(val, x) \
    SET_REGISTER_VALUE(val, x, 16, 2)
#define D4_DP_LCD_BB_COLOR_SELECT_09(val, x) \
	SET_REGISTER_VALUE(val, x, 18, 2)
#define D4_DP_LCD_BB_COLOR_SELECT_10(val, x) \
    SET_REGISTER_VALUE(val, x, 20, 2)
#define D4_DP_LCD_BB_COLOR_SELECT_11(val, x) \
	SET_REGISTER_VALUE(val, x, 22, 2)
#define D4_DP_LCD_BB_COLOR_SELECT_12(val, x) \
    SET_REGISTER_VALUE(val, x, 24, 2)
#define D4_DP_LCD_BB_COLOR_SELECT_13(val, x) \
	SET_REGISTER_VALUE(val, x, 26, 2)
#define D4_DP_LCD_BB_COLOR_SELECT_14(val, x) \
    SET_REGISTER_VALUE(val, x, 28, 2)
#define D4_DP_LCD_BB_COLOR_SELECT_15(val, x) \
	SET_REGISTER_VALUE(val, x, 30, 2)

/*          LCD VIDEO BB COLOR BOX   */
#define D4_DP_LCD_BB_ALPHA(val, x) \
	SET_REGISTER_VALUE(val, x, 0, 8)
#define D4_DP_LCD_BB_H_WIDTH(val, x) \
	SET_REGISTER_VALUE(val, x, 8, 4)
#define D4_DP_LCD_BB_V_WIDTH(val, x) \
	SET_REGISTER_VALUE(val, x, 12, 4)

#define D4_DP_LCD_BB_OUTLINE_00(val, x) \
	SET_REGISTER_VALUE(val, x, 16, 1)
#define D4_DP_LCD_BB_OUTLINE_01(val, x) \
	SET_REGISTER_VALUE(val, x, 17, 1)
#define D4_DP_LCD_BB_OUTLINE_02(val, x) \
	SET_REGISTER_VALUE(val, x, 18, 1)
#define D4_DP_LCD_BB_OUTLINE_03(val, x) \
	SET_REGISTER_VALUE(val, x, 19, 1)
#define D4_DP_LCD_BB_OUTLINE_04(val, x) \
	SET_REGISTER_VALUE(val, x, 20, 1)
#define D4_DP_LCD_BB_OUTLINE_05(val, x) \
	SET_REGISTER_VALUE(val, x, 21, 1)
#define D4_DP_LCD_BB_OUTLINE_06(val, x) \
	SET_REGISTER_VALUE(val, x, 22, 1)
#define D4_DP_LCD_BB_OUTLINE_07(val, x) \
	SET_REGISTER_VALUE(val, x, 23, 1)
#define D4_DP_LCD_BB_OUTLINE_08(val, x) \
	SET_REGISTER_VALUE(val, x, 24, 1)
#define D4_DP_LCD_BB_OUTLINE_09(val, x) \
	SET_REGISTER_VALUE(val, x, 25, 1)
#define D4_DP_LCD_BB_OUTLINE_10(val, x) \
	SET_REGISTER_VALUE(val, x, 26, 1)
#define D4_DP_LCD_BB_OUTLINE_11(val, x) \
	SET_REGISTER_VALUE(val, x, 27, 1)
#define D4_DP_LCD_BB_OUTLINE_12(val, x) \
	SET_REGISTER_VALUE(val, x, 28, 1)
#define D4_DP_LCD_BB_OUTLINE_13(val, x) \
	SET_REGISTER_VALUE(val, x, 29, 1)
#define D4_DP_LCD_BB_OUTLINE_14(val, x) \
	SET_REGISTER_VALUE(val, x, 30, 1)
#define D4_DP_LCD_BB_OUTLINE_15(val, x) \
	SET_REGISTER_VALUE(val, x, 31, 1)

/*          LCD VIDEO BB BOUDARY COLOR   */
#define D4_DP_LCD_BB_OUTLINE_B(val, x) \
	SET_REGISTER_VALUE(val, x, 0, 8)
#define D4_DP_LCD_BB_OUTLINE_G(val, x) \
	SET_REGISTER_VALUE(val, x, 8, 8)
#define D4_DP_LCD_BB_OUTLINE_R(val, x) \
	SET_REGISTER_VALUE(val, x, 16, 8)

/*          LCD GRAPHIC DMA CTRL   */
#define D4_DP_LCD_DMA_WIN0_ON(val, x) \
	SET_REGISTER_VALUE(val, x, 0, 1)
#define D4_DP_LCD_DMA_WIN1_ON(val, x) \
	SET_REGISTER_VALUE(val, x, 1, 1)
#define D4_DP_LCD_DMA_WIN2_ON(val, x) \
	SET_REGISTER_VALUE(val, x, 2, 1)
#define D4_DP_LCD_DMA_WIN3_ON(val, x) \
	SET_REGISTER_VALUE(val, x, 3, 1)
#define D4_DP_LCD_DMA_WIN0_SWAP(val, x) \
	SET_REGISTER_VALUE(val, x, 4, 1)
#define D4_DP_LCD_DMA_WIN1_SWAP(val, x) \
	SET_REGISTER_VALUE(val, x, 5, 1)
#define D4_DP_LCD_DMA_WIN2_SWAP(val, x) \
	SET_REGISTER_VALUE(val, x, 6, 1)
#define D4_DP_LCD_DMA_WIN3_SWAP(val, x) \
	SET_REGISTER_VALUE(val, x, 7, 1)
#define D4_DP_LCD_DMA_WIN0_FLAT_ALPHA(val, x) \
	SET_REGISTER_VALUE(val, x, 8, 1)
#define D4_DP_LCD_DMA_WIN1_FLAT_ALPHA(val, x) \
	SET_REGISTER_VALUE(val, x, 9, 1)
#define D4_DP_LCD_DMA_WIN2_FLAT_ALPHA(val, x) \
	SET_REGISTER_VALUE(val, x, 10, 1)
#define D4_DP_LCD_DMA_WIN3_FLAT_ALPHA(val, x) \
	SET_REGISTER_VALUE(val, x, 11, 1)
#define D4_DP_LCD_DMA_ARGB_ORDER(val, x) \
	SET_REGISTER_VALUE(val, x, 12, 1)

/*          LCD GRAPHIC DMA SCL   */
#define D4_DP_LCD_DMA_ZOOM_V(val, x) \
	SET_REGISTER_VALUE(val, x, 0, 1)
#define D4_DP_LCD_DMA_ZOOM_H(val, x) \
	SET_REGISTER_VALUE(val, x, 1, 1)

/*          LCD GRAPHIC DMA BACK GROUND   */
#define D4_DP_LCD_DMA_BKG_R(val, x) \
	SET_REGISTER_VALUE(val, x, 0, 8)
#define D4_DP_LCD_DMA_BKG_G(val, x) \
	SET_REGISTER_VALUE(val, x, 8, 8)
#define D4_DP_LCD_DMA_BKG_B(val, x) \
	SET_REGISTER_VALUE(val, x, 16, 8)
#define D4_DP_LCD_DMA_BKG_A(val, x) \
	SET_REGISTER_VALUE(val, x, 24, 8)

#define D4_DP_LCD_GRP_WIN0_ALPHA(val, x) \
	SET_REGISTER_VALUE(val, x, 0, 8)
#define D4_DP_LCD_GRP_WIN1_ALPHA(val, x) \
	SET_REGISTER_VALUE(val, x, 8, 8)
#define D4_DP_LCD_GRP_WIN2_ALPHA(val, x) \
	SET_REGISTER_VALUE(val, x, 16, 8)
#define D4_DP_LCD_GRP_WIN3_ALPHA(val, x) \
	SET_REGISTER_VALUE(val, x, 24, 8)

/*          LCD GRAPHIC DMA PRORITY   */
#define D4_DP_LCD_WIN0_PRORITY(val, x) \
	SET_REGISTER_VALUE(val, x, 0, 2)
#define D4_DP_LCD_WIN1_PRORITY(val, x) \
	SET_REGISTER_VALUE(val, x, 2, 2)
#define D4_DP_LCD_WIN2_PRORITY(val, x) \
	SET_REGISTER_VALUE(val, x, 4, 2)
#define D4_DP_LCD_WIN3_PRORITY(val, x) \
	SET_REGISTER_VALUE(val, x, 6, 2)

/*          LCD GM CTRL         */
#define D4_DP_LCD_GM_FICT_SW(val, x) \
	SET_REGISTER_VALUE(val, x, 0, 1)
#define D4_DP_LCD_GM_BLACK(val, x) \
	SET_REGISTER_VALUE(val, x, 1, 1)
#define D4_DP_LCD_GM_GAMMA(val, x) \
	SET_REGISTER_VALUE(val, x, 2, 1)
#define D4_DP_LCD_GM_CHROMA(val, x) \
	SET_REGISTER_VALUE(val, x, 3, 1)

/*          LCD GM LUX         */
#define D4_DP_LCD_GM_LUX(val, x) \
	SET_REGISTER_VALUE(val, x, 0, 8)

/*          LCD GM FINE       */
#define D4_DP_LCD_GM_BO_FINE(val, x) \
	SET_REGISTER_VALUE(val, x, 0, 8)
#define D4_DP_LCD_GM_CCG_FINE(val, x) \
	SET_REGISTER_VALUE(val, x, 8, 8)
#define D4_DP_LCD_GM_GAM_FINE(val, x) \
	SET_REGISTER_VALUE(val, x, 16, 8)

/*          LCD GM GAIN       */
#define D4_DP_LCD_GM_CG_GB(val, x) \
	SET_REGISTER_VALUE(val, x, 0, 8)
#define D4_DP_LCD_GM_CG_RB(val, x) \
	SET_REGISTER_VALUE(val, x, 8, 8)
#define D4_DP_LCD_GM_CG_GR(val, x) \
	SET_REGISTER_VALUE(val, x, 16, 8)

/*          LCD GM HTH       */
#define D4_DP_LCD_GM_HTH_GB(val, x) \
	SET_REGISTER_VALUE(val, x, 0, 8)
#define D4_DP_LCD_GM_HTH_RB(val, x) \
	SET_REGISTER_VALUE(val, x, 8, 8)
#define D4_DP_LCD_GM_HTH_GR(val, x) \
	SET_REGISTER_VALUE(val, x, 16, 8)

/*          LCD GM DELGB       */
#define D4_DP_LCD_GM_DEL_LOW(val, x) \
	SET_REGISTER_VALUE(val, x, 0, 10)
#define D4_DP_LCD_GM_DEL_UP(val, x) \
	SET_REGISTER_VALUE(val, x, 16, 10)

/*          LCD GM FIC0       */
#define gm_fic_rgb00(x)			(((x) & 0x1fff) << 0)
#define gm_fic_rgb01(x)			(((x) & 0x1fff) << 16)

#define D4_DP_LCD_GM_FIC_LOW(val, x) \
	SET_REGISTER_VALUE(val, x, 0, 13)
#define D4_DP_LCD_GM_FIC_UP(val, x) \
	SET_REGISTER_VALUE(val, x, 16, 13)

/* LCD CMS( Color Management System ) Register */
#define D4_DP_LCD_TCC_R_LUT(val, x) \
	SET_REGISTER_VALUE(val, x, 0, 8)
#define D4_DP_LCD_TCC_G_LUT(val, x) \
	SET_REGISTER_VALUE(val, x, 8, 8)
#define D4_DP_LCD_TCC_B_LUT(val, x) \
	SET_REGISTER_VALUE(val, x, 16, 8)
#define D4_DP_LCD_TCC_LUT_INDEX(val, x) \
	SET_REGISTER_VALUE(val, x, 24, 8)

#endif /* _DP_LCD_REG_ */
