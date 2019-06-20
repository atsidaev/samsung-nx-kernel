/**
 * @file d4_pp_core_regs.h
 * @brief  DRIMe4 PP Core Register Define for Device Driver
 * @author Main : Sunghoon Kim <bluesay.kim@samsung.com>
 *         MIPI : Gunwoo Nam <gunwoo.nam@samsung.com>
 * 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifndef _DRIME4_REGS_PP_CORE_H_
#define _DRIME4_REGS_PP_CORE_H_

#include <mach/d4_reg_macro.h>

/******************************************************************************/
/*                         PP Core Register Define                            */
/******************************************************************************/

/******************************************************************************/
/*                          Register Offset Define                            */
/******************************************************************************/

/**< PP Core Common Register */
#define PP_CORE_COMMON_CTRL_1               0x0000
#define PP_CORE_COMMON_CTRL_2               0x0008
#define PP_CORE_COMMON_RST                  0x000C
#define PP_CORE_COMMON_LUT_INIT             0x0010
#define PP_CORE_COMMON_LUT_CLK              0x0014
#define PP_CORE_COMMON_INT_STATUS           0x0020
#define PP_CORE_COMMON_INT_ENABLE           0x0024
#define PP_CORE_COMMON_INT_CLEAR            0x0028

/**< DPC( Deffect Pixel Correction ) Control Register */
#define PP_CORE_DPC_COMMON_CTRL				0x0000
#define PP_CORE_DPC_LUT_INIT_CTRL			0x0004
#define PP_CORE_DPC_LUT_INIT_DEFECT_NUM		0x0008
#define PP_CORE_DPC_LUT_INIT_ADDR			0x000C
#define PP_CORE_DPC_IMAGE_SIZE              0x0010
#define PP_CORE_DPC_OB_SIZE					0x0014
#define PP_CORE_DPC_OB_TYPE					0x0018
#define PP_CORE_DPC_CORRECTION				0x001C
#define PP_CORE_DPC_LUT_GEN_CTRL			0x0030
#define PP_CORE_DPC_LUT_GEN_THRESHOLD		0x0034
#define PP_CORE_DPC_LUT_GEN_DEFECT_NUM		0x0038
#define PP_CORE_DPC_HP						0x0040
#define PP_CORE_DPC_HP_RANGE_0				0x0044
#define PP_CORE_DPC_HP_RANGE_1				0x0048
#define PP_CORE_DPC_HP_RANGE_2				0x004C
#define PP_CORE_DPC_HP_RANGE_3				0x0050
#define PP_CORE_DPC_HP_RANGE_4				0x0054
#define PP_CORE_DPC_HP_RANGE_5				0x0058
#define PP_CORE_DPC_HP_RANGE_6				0x005C
#define PP_CORE_DPC_HP_RANGE_7				0x0060
#define PP_CORE_DPC_HP_GAIN_H_0				0x0064
#define PP_CORE_DPC_HP_GAIN_H_1				0x0068
#define PP_CORE_DPC_HP_GAIN_H_2				0x006C
#define PP_CORE_DPC_HP_GAIN_H_3				0x0070
#define PP_CORE_DPC_HP_GAIN_H_4				0x0074
#define PP_CORE_DPC_HP_GAIN_H_5				0x0078
#define PP_CORE_DPC_HP_GAIN_H_6				0x007C
#define PP_CORE_DPC_HP_GAIN_H_7				0x0080
#define PP_CORE_DPC_HP_GAIN_L_0				0x0084
#define PP_CORE_DPC_HP_GAIN_L_1				0x0088
#define PP_CORE_DPC_HP_GAIN_L_2				0x008C
#define PP_CORE_DPC_HP_GAIN_L_3				0x0090
#define PP_CORE_DPC_HP_GAIN_L_4				0x0094
#define PP_CORE_DPC_HP_GAIN_L_5				0x0098
#define PP_CORE_DPC_HP_GAIN_L_6				0x009C
#define PP_CORE_DPC_HP_GAIN_L_7				0x00A0

/**< Smear Control Register */
#define PP_CORE_SMEAR_CTRL                  0x00C0
#define PP_CORE_SMEAR_TOP_OB                0x00C4
#define PP_CORE_SMEAR_OB_SIZE				0x00C8
#define PP_CORE_SMEAR_INPUT_SIZE			0x00CC
#define PP_CORE_SMEAR_SATURATION_THRESHOLD	0x00D0
#define PP_CORE_SMEAR_MODIFY_THRESHOLD		0x00D4
#define PP_CORE_SMEAR_V_START_LINE			0x00D8
#define PP_CORE_SMEAR_MANUAL_SET_1			0x00DC
#define PP_CORE_SMEAR_MANUAL_SET_2			0x00E0

/**< XYS( XY Shading ) Control Register */
#define PP_CORE_XYS_CTRL_1                  0x0140
#define PP_CORE_XYS_CTRL_2                  0x0144
#define PP_CORE_XYS_INPUT_SIZE              0x0148
#define PP_CORE_XYS_H_OB					0x0154
#define PP_CORE_XYS_V_OB					0x0158
#define PP_CORE_XYS_REG_OFFSET_1			0x0160
#define PP_CORE_XYS_REG_OFFSET_2			0x0164
#define PP_CORE_XYS_REG_OB_OFFSET_1			0x0168
#define PP_CORE_XYS_REG_OB_OFFSET_2			0x016C
#define PP_CORE_XYS_SATURATION_THRESHOLD	0x0170

/**< HBR( Horizontal Bayer Resize ) Control Register */
#define PP_CORE_HBR_CTRL                    0x0180
#define PP_CORE_HBR_IN_SIZE                 0x0184
#define PP_CORE_HBR_SKIP_SIZE               0x0188
#define PP_CORE_HBR_OUT_SIZE                0x018C
#define PP_CORE_HBR_SHOOT_CTRL_1            0x0190
#define PP_CORE_HBR_SHOOT_CTRL_2            0x0194

/**< VBR( Vertical Bayer Resize ) Control Register */
#define PP_CORE_VBR_CTRL                    0x01A0
#define PP_CORE_VBR_IN_SIZE                 0x01A4
#define PP_CORE_VBR_SKIP_SIZE               0x01A8
#define PP_CORE_VBR_OUT_SIZE                0x01AC
#define PP_CORE_VBR_SHOOT_CTRL_1            0x01B0
#define PP_CORE_VBR_SHOOT_CTRL_2            0x01B4

/**< BG( Bayer Gamma ) Control Register */
#define PP_CORE_BG_CTRL                     0x01C0
#define PP_CORE_BG_TOP_OB                   0x01C4
#define PP_CORE_BG_SIDE_OB                  0x01C8

/**< HRN( Horizontal Random Noise Reduction ) Control Register */
#define PP_CORE_HRN_CTRL                    0x0240
#define PP_CORE_HRN_USE_SIDE_OB				0x0244
#define PP_CORE_HRN_MANUAL_CTRL_1			0x0248
#define PP_CORE_HRN_MANUAL_CTRL_2			0x024C
#define PP_CORE_HRN_TOP_OB_SIZE				0x0250
#define PP_CORE_HRN_SIDE_OB					0x0254
#define PP_CORE_HRN_TOP_OB_POSITION			0x0258

/**< VFPN( Vertical Fixed Pattern Noise Correction ) Control Register */
#define PP_CORE_VFPN_CTRL                   0x0280
#define PP_CORE_VFPN_OB_INFO				0x0284
#define PP_CORE_VFPN_RESULT_SAVE_START_ADDR 0x0288
#define PP_CORE_VFPN_RESULT_SAVE_END_ADDR	0x028C

/**< CS( Color Shading ) Control Register */
#define PP_CORE_CS_CTRL                     0x02D0
#define PP_CORE_CS_SAMPLE_DIVIDER			0x02D4
#define PP_CORE_CS_INPUT_SIZE				0x02D8
#define PP_CORE_CS_START_POSITION			0x02DC
#define PP_CORE_CS_INTERPOLATION_RATIO		0x02E0
#define PP_CORE_CS_VS_RATIO					0x02E4
#define PP_CORE_CS_GAIN_RATIO_1				0x02E8
#define PP_CORE_CS_GAIN_RATIO_2				0x02EC
#define PP_CORE_CS_LUT_INFO					0x02F0

/**< DAMP( Digital AMP ) Control Register */
#define PP_CORE_DAMP_CTRL                   0x0310
#define PP_CORE_DAMP_OFFSET_1               0x0314
#define PP_CORE_DAMP_OFFSET_2               0x0318
#define PP_CORE_DAMP_GAIN_1                 0x031C
#define PP_CORE_DAMP_GAIN_2                 0x0320

/**< OBD1( Optical Black Detection 1 ) Control Register */
#define PP_CORE_OBD1_CTRL                   0x0340
#define PP_CORE_OBD1_H_POS                  0x0344
#define PP_CORE_OBD1_V_POS                  0x0348
#define PP_CORE_OBD1_RESULT_1               0x034C
#define PP_CORE_OBD1_RESULT_2               0x0350

/**< OBD2( Optical Black Detection 2 ) Control Register */
#define PP_CORE_OBD2_CTRL                   0x0380
#define PP_CORE_OBD2_H_POS                  0x0384
#define PP_CORE_OBD2_V_POS                  0x0388
#define PP_CORE_OBD2_RESULT_1               0x038C
#define PP_CORE_OBD2_RESULT_2               0x0390

/**< OBD3( Optical Black Detection 3 ) Control Register */
#define PP_CORE_OBD3_CTRL                   0x03C0
#define PP_CORE_OBD3_H_POS                  0x03C4
#define PP_CORE_OBD3_V_POS                  0x03C8
#define PP_CORE_OBD3_RESULT_1               0x03CC
#define PP_CORE_OBD3_RESULT_2               0x03D0

/**< OOC1( Optical Black Correction 1 ) Control Register */
#define PP_CORE_OOC1_CTRL                   0x0400
#define PP_CORE_OOC1_H_POS                  0x0404
#define PP_CORE_OOC1_V_POS                  0x0408
#define PP_CORE_OOC1_MANUAL_SET_1           0x040C
#define PP_CORE_OOC1_MANUAL_SET_2           0x0410

/**< OOC2( Optical Black Correction 2 ) Control Register */
#define PP_CORE_OOC2_CTRL                   0x0440
#define PP_CORE_OOC2_H_POS                  0x0444
#define PP_CORE_OOC2_V_POS                  0x0448
#define PP_CORE_OOC2_MANUAL_SET_1           0x044C
#define PP_CORE_OOC2_MANUAL_SET_2           0x0450

/**< OBR1( Optical Black Remove 1 ) Control Register */
#define PP_CORE_OBR1_CTRL                   0x0480
#define PP_CORE_OBR1_H_POS                  0x0484
#define PP_CORE_OBR1_V_POS                  0x0488

/**< OBR2( Optical Black Remove 2 ) Control Register */
#define PP_CORE_OBR2_CTRL                   0x04C0
#define PP_CORE_OBR2_H_POS                  0x04C4
#define PP_CORE_OBR2_V_POS                  0x04C8

/**< KNEE Control Register */
#define PP_CORE_KNEE_TH_STEP_1              0x0504
#define PP_CORE_KNEE_TH_STEP_2              0x0508
#define PP_CORE_KNEE_TH_STEP_3              0x050C
#define PP_CORE_KNEE_TH_STEP_4              0x0510
#define PP_CORE_KNEE_TH_STEP_5              0x0514
#define PP_CORE_KNEE_GAIN_1                 0x0518
#define PP_CORE_KNEE_GAIN_2                 0x051C
#define PP_CORE_KNEE_GAIN_3                 0x0520
#define PP_CORE_KNEE_GAIN_4                 0x0524
#define PP_CORE_KNEE_GAIN_5                 0x0528
#define PP_CORE_KNEE_OFFSET_1               0x052C
#define PP_CORE_KNEE_OFFSET_2               0x0530
#define PP_CORE_KNEE_OFFSET_3               0x0534
#define PP_CORE_KNEE_OFFSET_4               0x0538
#define PP_CORE_KNEE_OFFSET_5               0x053C
#define PP_CORE_KNEE_CTRL                   0x0540
#define PP_CORE_KNEE_OB_START               0x0544
#define PP_CORE_KNEE_OB_END                 0x0548

/**< DEKNEE Control Register */
#define PP_CORE_DEKNEE_CTRL                 0x0550

/**< PP - MDMA Control Register */
#define PP_CORE_MDMA_ID                     0x0600
#define PP_CORE_MDMA_DPC_ADDR               0x0604
#define PP_CORE_MDMA_CS_ADDR                0x0608
#define PP_CORE_MDMA_XYS_START_ADDR         0x060C
#define PP_CORE_MDMA_XYS_END_ADDR           0x0610
#define PP_CORE_MDMA_BG_START_ADDR          0x0614
#define PP_CORE_MDMA_BG_END_ADDR            0x0618
#define PP_CORE_MDMA_CS_2_START_ADDR        0x061C
#define PP_CORE_MDMA_CS_2_END_ADDR          0x0620
#define PP_CORE_MDMA_HBR_START_ADDR         0x0624
#define PP_CORE_MDMA_HBR_END_ADDR           0x0628
#define PP_CORE_MDMA_VBR_START_ADDR         0x062C
#define PP_CORE_MDMA_VBR_END_ADDR           0x0630

/**< PP - RDMA Control Register */
#define PP_RDMA_CTRL			            0x0000
#define PP_RDMA_WIDTH						0x0004
#define PP_RDMA_HEIGHT						0x0008
#define PP_RDMA_EVEN_ADDR_0                 0x000C
#define PP_RDMA_ODD_ADDR_0                  0x0010
#define PP_RDMA_DELTA_ADDR_0                0x0014
#define PP_RDMA_EVEN_ADDR_1                 0x0018
#define PP_RDMA_ODD_ADDR_1                  0x001C
#define PP_RDMA_DELTA_ADDR_1                0x0020
#define PP_RDMA_CH_MODE						0x0038

/**< PP - WDMA Control Register */
#define PP_WDMA_1_CLK_DATA_BIT_0            0x0300
#define PP_WDMA_EVEN_ADDR_0                 0x0304
#define PP_WDMA_ODD_ADDR_0                  0x0308
#define PP_WDMA_DELTA_ADDR_0                0x030C
#define PP_WDMA_BURST_LEN_0                 0x0310
#define PP_WDMA_CTRL                        0x0314
#define PP_WDMA_RESET_0                     0x0318
#define PP_WDMA_1_CLK_DATA_BIT_1            0x0320
#define PP_WDMA_EVEN_ADDR_1                 0x0324
#define PP_WDMA_ODD_ADDR_1                  0x0328
#define PP_WDMA_DELTA_ADDR_1                0x032C
#define PP_WDMA_BURST_LEN_1                 0x0330
#define PP_WDMA_RESET_1                     0x0338

/**< PP - DMA Control Register */
#define PP_DMA_START_CTRL      				0x0A00
#define PP_RW_DMA_ENABLE_CTRL  				0x0A04
#define PP_DMA_ENABLE_CTRL      			0x0A10

/******************************************************************************/
/*                        Register Structure Define                           */
/******************************************************************************/

/**< PP Core Common Register */

#define D4_PP_CORE_COMMON_CTRL_1_LUT_GEN_MODE_ON(val, x) \
    SET_REGISTER_VALUE(val, x, 0, 1)
#define D4_PP_CORE_COMMON_CTRL_1_BAYER_FORMAT(val, x) \
    SET_REGISTER_VALUE(val, x, 8, 2)
#define D4_PP_CORE_COMMON_CTRL_1_DATA_BIT_WIDTH(val, x) \
    SET_REGISTER_VALUE(val, x, 16, 3)
#define D4_PP_CORE_COMMON_CTRL_1_GET_DATA_BIT_WIDTH(val) \
    GET_REGISTER_VALUE(val, 16, 3)
#define D4_PP_CORE_COMMON_CTRL_1_IPC_OUTPUT_ENABLE(val, x) \
    SET_REGISTER_VALUE(val, x, 24, 1)
#define D4_PP_CORE_COMMON_CTRL_1_VS_ON(val, x) \
    SET_REGISTER_VALUE(val, x, 28, 1)


#define D4_PP_CORE_COMMON_CTRL_2_CTRL_MODE(val, x) \
    SET_REGISTER_VALUE(val, x, 0, 4)
#define D4_PP_CORE_COMMON_CTRL_2_LUT_MIRROR_INIT_ID(val, x) \
    SET_REGISTER_VALUE(val, x, 4, 3)
#define D4_PP_CORE_COMMON_CTRL_2_LUT_MIRROR_SIZE(val, x) \
    SET_REGISTER_VALUE(val, x, 8, 13)
#define D4_PP_CORE_COMMON_CTRL_2_IN_CH_NUM(val, x) \
    SET_REGISTER_VALUE(val, x, 24, 2)
#define D4_PP_CORE_COMMON_CTRL_2_GET_IN_CH_NUM(val) \
    GET_REGISTER_VALUE(val, 24, 2)
#define D4_PP_CORE_COMMON_CTRL_2_OUT_CH_NUM(val, x) \
    SET_REGISTER_VALUE(val, x, 28, 2)


#define D4_PP_CORE_COMMON_RST_RESET(val, x) \
    SET_REGISTER_VALUE(val, x, 0, 1)
#define D4_PP_CORE_COMMON_RST_MDMA_RESET(val, x) \
    SET_REGISTER_VALUE(val, x, 4, 1)
#define D4_PP_CORE_COMMON_RST_3A_RESET(val, x) \
    SET_REGISTER_VALUE(val, x, 8, 1)
#define D4_PP_CORE_COMMON_RST_DMA_SM_RESET(val, x) \
    SET_REGISTER_VALUE(val, x, 12, 1)
#define D4_PP_CORE_COMMON_RST_WDMA_RESET(val, x) \
    SET_REGISTER_VALUE(val, x, 16, 1)
#define D4_PP_CORE_COMMON_RST_RDMA_RESET(val, x) \
    SET_REGISTER_VALUE(val, x, 20, 1)
#define D4_PP_CORE_COMMON_RST_WDMA_MIPI_RESET(val, x) \
    SET_REGISTER_VALUE(val, x, 24, 1)
#define D4_PP_CORE_COMMON_RST_RDMA_MIPI_RESET(val, x) \
    SET_REGISTER_VALUE(val, x, 28, 1)
#define D4_PP_CORE_COMMON_RST_WDMA_SSIF_RESET(val, x) \
    SET_REGISTER_VALUE(val, x, 30, 1)
#define D4_PP_CORE_COMMON_RST_DMA_CTRL_RESET(val, x) \
    SET_REGISTER_VALUE(val, x, 31, 1)


#define D4_PP_CORE_COMMON_LUT_INIT_DPC_LUT_INIT(val, x) \
    SET_REGISTER_VALUE(val, x, 0, 1)
#define D4_PP_CORE_COMMON_LUT_INIT_XYS_LUT_INIT(val, x) \
    SET_REGISTER_VALUE(val, x, 2, 1)
#define D4_PP_CORE_COMMON_LUT_INIT_XYS_LUT_INIT_1ST(val, x) \
    SET_REGISTER_VALUE(val, x, 3, 1)
#define D4_PP_CORE_COMMON_LUT_INIT_XYS_LUT_INIT_H_OFFSET(val, x) \
    SET_REGISTER_VALUE(val, x, 4, 1)
#define D4_PP_CORE_COMMON_LUT_INIT_XYS_LUT_INIT_H_GAIN(val, x) \
    SET_REGISTER_VALUE(val, x, 5, 1)
#define D4_PP_CORE_COMMON_LUT_INIT_XYS_LUT_INIT_V_OFFSET(val, x) \
    SET_REGISTER_VALUE(val, x, 6, 1)
#define D4_PP_CORE_COMMON_LUT_INIT_XYS_LUT_INIT_V_GAIN(val, x) \
    SET_REGISTER_VALUE(val, x, 7, 1)
#define D4_PP_CORE_COMMON_LUT_INIT_BG_LUT_INIT(val, x) \
    SET_REGISTER_VALUE(val, x, 8, 1)
#define D4_PP_CORE_COMMON_LUT_INIT_CS_LUT_INIT(val, x) \
    SET_REGISTER_VALUE(val, x, 12, 1)
#define D4_PP_CORE_COMMON_LUT_INIT_HBR_LUT_INIT(val, x) \
    SET_REGISTER_VALUE(val, x, 16, 1)
#define D4_PP_CORE_COMMON_LUT_INIT_VBR_LUT_INIT(val, x) \
    SET_REGISTER_VALUE(val, x, 20, 1)

#define D4_PP_CORE_COMMON_LUT_CLK_DPC_CLK_ON(val, x) \
    SET_REGISTER_VALUE(val, x, 0, 1)
#define D4_PP_CORE_COMMON_LUT_CLK_CS_CLK_ON(val, x) \
    SET_REGISTER_VALUE(val, x, 4, 1)
#define D4_PP_CORE_COMMON_LUT_CLK_LUT_MIRROR_CLK_ON(val, x) \
    SET_REGISTER_VALUE(val, x, 8, 1)

#define D4_PP_CORE_COMMON_INT_DPC_LUT_INIT_DONE(val, x) \
    SET_REGISTER_VALUE(val, x, 0, 1)
#define D4_PP_CORE_COMMON_INT_DPC_LUT_GEN_DONE(val, x) \
    SET_REGISTER_VALUE(val, x, 2, 1)
#define D4_PP_CORE_COMMON_INT_CS_LUT_GEN_DONE(val, x) \
    SET_REGISTER_VALUE(val, x, 3, 1)
#define D4_PP_CORE_COMMON_INT_BG_LUT_INIT_DONE(val, x) \
    SET_REGISTER_VALUE(val, x, 4, 1)
#define D4_PP_CORE_COMMON_INT_XYS_LUT_INIT_DONE(val, x) \
    SET_REGISTER_VALUE(val, x, 5, 1)
#define D4_PP_CORE_COMMON_INT_CS_LUT_INIT_DONE(val, x) \
    SET_REGISTER_VALUE(val, x, 6, 1)
#define D4_PP_CORE_COMMON_INT_HBR_LUT_INIT_DONE(val, x) \
    SET_REGISTER_VALUE(val, x, 7, 1)
#define D4_PP_CORE_COMMON_INT_VBR_LUT_INIT_DONE(val, x) \
    SET_REGISTER_VALUE(val, x, 8, 1)
#define D4_PP_CORE_COMMON_INT_VFPN_WRITE_DONE(val, x) \
    SET_REGISTER_VALUE(val, x, 9, 1)
#define D4_PP_CORE_COMMON_INT_OBD1_DONE(val, x) \
    SET_REGISTER_VALUE(val, x, 12, 1)
#define D4_PP_CORE_COMMON_INT_OBD2_DONE(val, x) \
    SET_REGISTER_VALUE(val, x, 13, 1)
#define D4_PP_CORE_COMMON_INT_OBD3_DONE(val, x) \
    SET_REGISTER_VALUE(val, x, 14, 1)
#define D4_PP_CORE_COMMON_INT_PP_WDMA_FRAME_DONE(val, x) \
    SET_REGISTER_VALUE(val, x, 19, 1)
#define D4_PP_CORE_COMMON_INT_PP_WDMA_INPUT_DONE(val, x) \
    SET_REGISTER_VALUE(val, x, 20, 1)
#define D4_PP_CORE_COMMON_INT_PP_WDMA_ERROR(val, x) \
    SET_REGISTER_VALUE(val, x, 21, 1)
#define D4_PP_CORE_COMMON_INT_PP_RDMA_DONE(val, x) \
    SET_REGISTER_VALUE(val, x, 22, 1)
#define D4_PP_CORE_COMMON_INT_MIPI_WDMA_FRAME_DONE(val, x) \
    SET_REGISTER_VALUE(val, x, 23, 1)
#define D4_PP_CORE_COMMON_INT_MIPI_WDMA_INPUT_DONE(val, x) \
    SET_REGISTER_VALUE(val, x, 24, 1)
#define D4_PP_CORE_COMMON_INT_MIPI_WDMA_ERROR(val, x) \
    SET_REGISTER_VALUE(val, x, 25, 1)
#define D4_PP_CORE_COMMON_INT_MIPI_RDMA_DONE(val, x) \
    SET_REGISTER_VALUE(val, x, 26, 1)
#define D4_PP_CORE_COMMON_INT_SELF_DONE(val, x) \
    SET_REGISTER_VALUE(val, x, 28, 1)
#define D4_PP_CORE_COMMON_INT_WAIT_ERROR(val, x) \
    SET_REGISTER_VALUE(val, x, 30, 1)
#define D4_PP_CORE_COMMON_INT_MIPI_WAIT_ERROR(val, x) \
    SET_REGISTER_VALUE(val, x, 31, 1)

/**< DPC( Deffect Pixel Correction ) Control Register */
#define D4_PP_CORE_DPC_COMMON_CTRL_ON(val, x) \
	    SET_REGISTER_VALUE(val, x, 0, 1)
#define D4_PP_CORE_DPC_COMMON_CTRL_GET_ONOFF(val) \
	    GET_REGISTER_VALUE(val, 0, 1)
#define D4_PP_CORE_DPC_COMMON_CTRL_STATIC_ON(val, x) \
	    SET_REGISTER_VALUE(val, x, 1, 1)
#define D4_PP_CORE_DPC_COMMON_CTRL_DYNAMIC_ON(val, x) \
	    SET_REGISTER_VALUE(val, x, 2, 1)
#define D4_PP_CORE_DPC_COMMON_CTRL_GET_STATIC_ON(val) \
	    GET_REGISTER_VALUE(val, 1, 1)
#define D4_PP_CORE_DPC_COMMON_CTRL_GET_DYNAMIC_ON(val) \
	    GET_REGISTER_VALUE(val, 2, 1)

#define D4_PP_CORE_DPC_LUT_INIT_CTRL_LUT_CHANGE_MODE(val, x) \
	    SET_REGISTER_VALUE(val, x, 0, 1)
#define D4_PP_CORE_DPC_LUT_INIT_CTRL_LUT_UNLIMITED_MODE(val, x) \
	    SET_REGISTER_VALUE(val, x, 8, 1)
#define D4_PP_CORE_DPC_LUT_INIT_CTRL_LUT_REQ_BURST_SIZE(val, x) \
	    SET_REGISTER_VALUE(val, x, 16, 6)

#define D4_PP_CORE_DPC_LUT_INIT_DEFECT_NUM(val, x) \
	    SET_REGISTER_VALUE(val, x, 0, 24)

#define D4_PP_CORE_DPC_LUT_INIT_ADDR(val, x) \
	    SET_REGISTER_VALUE(val, x, 0, 32)

#define D4_PP_CORE_DPC_IMAGE_SIZE_H_SIZE(val, x) \
	    SET_REGISTER_VALUE(val, x, 16, 14)
#define D4_PP_CORE_DPC_IMAGE_SIZE_V_SIZE(val, x) \
	    SET_REGISTER_VALUE(val, x, 0, 13)

#define D4_PP_CORE_DPC_OB_SIZE_H_SIZE(val, x) \
	    SET_REGISTER_VALUE(val, x, 16, 14)
#define D4_PP_CORE_DPC_OB_SIZE_V_SIZE(val, x) \
	    SET_REGISTER_VALUE(val, x, 0, 13)

#define D4_PP_CORE_DPC_OB_TYPE(val, x) \
	    SET_REGISTER_VALUE(val, x, 16, 2)

#define D4_PP_CORE_DPC_CORRECTION_TH(val, x) \
	    SET_REGISTER_VALUE(val, x, 0, 14)
#define D4_PP_CORE_DPC_CORRECTION_ZCORRECT_RATIO(val, x) \
	    SET_REGISTER_VALUE(val, x, 16, 4)
#define D4_PP_CORE_DPC_CORRECTION_ZCORRECT_RSW(val, x) \
	    SET_REGISTER_VALUE(val, x, 24, 1)

#define D4_PP_CORE_DPC_LUT_GEN_CTRL_ENABLE(val, x) \
	    SET_REGISTER_VALUE(val, x, 0, 1)
#define D4_PP_CORE_DPC_LUT_GEN_CTRL_DEFECT_MODE(val, x) \
	    SET_REGISTER_VALUE(val, x, 8, 1)
#define D4_PP_CORE_DPC_LUT_GEN_CTRL_UPDATE_MODE(val, x) \
	    SET_REGISTER_VALUE(val, x, 16, 1)

#define D4_PP_CORE_DPC_LUT_GEN_THRESHOLD_WHITE_TH(val, x) \
	    SET_REGISTER_VALUE(val, x, 16, 14)
#define D4_PP_CORE_DPC_LUT_GEN_THRESHOLD_BLACK_TH(val, x) \
	    SET_REGISTER_VALUE(val, x, 0, 14)

#define D4_PP_CORE_DPC_LUT_GEN_GET_DEFECT_NUM(val) \
	    GET_REGISTER_VALUE(val, 0, 24)

#define D4_PP_CORE_DPC_HP_THRESHOLD(val, x) \
	    SET_REGISTER_VALUE(val, x, 0, 14)
#define D4_PP_CORE_DPC_HP_CORRECTION_MODE(val, x) \
	    SET_REGISTER_VALUE(val, x, 16, 1)
#define D4_PP_CORE_DPC_HP_EXCEPT_STATIC_DEFECT(val, x) \
	    SET_REGISTER_VALUE(val, x, 24, 1)

#define D4_PP_CORE_DPC_HP_RANGE_UPPER(val, x) \
	    SET_REGISTER_VALUE(val, x, 16, 13)
#define D4_PP_CORE_DPC_HP_RANGE_LOWER(val, x) \
	    SET_REGISTER_VALUE(val, x, 0, 13)

#define D4_PP_CORE_DPC_HP_GAIN_UPPER(val, x) \
	    SET_REGISTER_VALUE(val, x, 16, 14)
#define D4_PP_CORE_DPC_HP_GAIN_LOWER(val, x) \
	    SET_REGISTER_VALUE(val, x, 0, 14)

/**< Smear Control Register */
#define D4_PP_CORE_SMEAR_CTRL_SMEAR_ON(val, x) \
	    SET_REGISTER_VALUE(val, x, 0, 1)
#define D4_PP_CORE_SMEAR_CTRL_GET_SMEAR_ONOFF(val) \
	    GET_REGISTER_VALUE(val, 0, 1)
#define D4_PP_CORE_SMEAR_CTRL_TOP_OB_TYPE(val, x) \
	    SET_REGISTER_VALUE(val, x, 4, 1)
#define D4_PP_CORE_SMEAR_CTRL_OBD_SELECTION(val, x) \
	    SET_REGISTER_VALUE(val, x, 8, 2)

#define D4_PP_CORE_SMEAR_TOP_OB_START(val, x) \
	    SET_REGISTER_VALUE(val, x, 0, 13)
#define D4_PP_CORE_SMEAR_TOP_OB_END(val, x) \
	    SET_REGISTER_VALUE(val, x, 16, 13)

#define D4_PP_CORE_SMEAR_OB_SIZE_TOP_OB_SIZE(val, x) \
	    SET_REGISTER_VALUE(val, x, 0, 10)
#define D4_PP_CORE_SMEAR_OB_SIZE_H_END(val, x) \
	    SET_REGISTER_VALUE(val, x, 16, 13)

#define D4_PP_CORE_SMEAR_INPUT_SIZE_H_SIZE(val, x) \
	    SET_REGISTER_VALUE(val, x, 0, 13)
#define D4_PP_CORE_SMEAR_INPUT_SIZE_V_SIZE(val, x) \
	    SET_REGISTER_VALUE(val, x, 16, 13)

#define D4_PP_CORE_SMEAR_SATURATION_THRESHOLD_SAT_TH(val, x) \
	    SET_REGISTER_VALUE(val, x, 0, 14)
#define D4_PP_CORE_SMEAR_SATURATION_THRESHOLD_LOW_SAT_TH(val, x) \
	    SET_REGISTER_VALUE(val, x, 16, 14)

#define D4_PP_CORE_SMEAR_MODIFY_THRESHOLD_LPF_THRESHOLD(val, x) \
	    SET_REGISTER_VALUE(val, x, 0, 14)
#define D4_PP_CORE_SMEAR_MODIFY_THRESHOLD_SAT_THRESHOLD(val, x) \
	    SET_REGISTER_VALUE(val, x, 16, 14)

#define D4_PP_CORE_SMEAR_V_DATA_START(val, x) \
	    SET_REGISTER_VALUE(val, x, 0, 13)

#define D4_PP_CORE_SMEAR_MANUAL_SET_1_OB_0_0(val, x) \
	    SET_REGISTER_VALUE(val, x, 0, 14)
#define D4_PP_CORE_SMEAR_MANUAL_SET_1_OB_0_1(val, x) \
	    SET_REGISTER_VALUE(val, x, 16, 14)

#define D4_PP_CORE_SMEAR_MANUAL_SET_2_OB_1_0(val, x) \
	    SET_REGISTER_VALUE(val, x, 0, 14)
#define D4_PP_CORE_SMEAR_MANUAL_SET_2_OB_1_1(val, x) \
	    SET_REGISTER_VALUE(val, x, 16, 14)

/**< XYS( XY Shading ) Control Register */
#define D4_PP_CORE_XYS_CTRL_1_XYS_ON(val, x) \
    SET_REGISTER_VALUE(val, x, 0, 1)
#define D4_PP_CORE_XYS_CTRL_1_GET_XYS_ONOFF(val) \
    GET_REGISTER_VALUE(val, 0, 1)
#define D4_PP_CORE_XYS_CTRL_1_H_OFFSET_EN(val, x) \
    SET_REGISTER_VALUE(val, x, 4, 1)
#define D4_PP_CORE_XYS_CTRL_1_H_GAIN_EN(val, x) \
    SET_REGISTER_VALUE(val, x, 8, 1)
#define D4_PP_CORE_XYS_CTRL_1_V_OFFSET_EN(val, x) \
    SET_REGISTER_VALUE(val, x, 12, 1)
#define D4_PP_CORE_XYS_CTRL_1_V_GAIN_EN(val, x) \
    SET_REGISTER_VALUE(val, x, 16, 1)
#define D4_PP_CORE_XYS_CTRL_1_H_OB_TYPE_SELECTION(val, x) \
    SET_REGISTER_VALUE(val, x, 20, 1)
#define D4_PP_CORE_XYS_CTRL_1_V_OB_TYPE_SELECTION(val, x) \
    SET_REGISTER_VALUE(val, x, 24, 1)
#define D4_PP_CORE_XYS_CTRL_1_REG_OFFSET_OFF(val, x) \
    SET_REGISTER_VALUE(val, x, 28, 1)

#define D4_PP_CORE_XYS_CTRL_2_REG_OB_OFFSET_EN(val, x) \
    SET_REGISTER_VALUE(val, x, 0, 1)
#define D4_PP_CORE_XYS_CTRL_2_SATURATION_THRESHOLD_ON(val, x) \
    SET_REGISTER_VALUE(val, x, 4, 1)
#define D4_PP_CORE_XYS_CTRL_2_NO_OB_ON(val, x) \
    SET_REGISTER_VALUE(val, x, 12, 1)

#define D4_PP_CORE_XYS_INPUT_SIZE_H_SIZE(val, x) \
    SET_REGISTER_VALUE(val, x, 0, 13)
#define D4_PP_CORE_XYS_INPUT_SIZE_V_SIZE(val, x) \
    SET_REGISTER_VALUE(val, x, 16, 13)

#define D4_PP_CORE_XYS_H_OB_START_POSITION(val, x) \
    SET_REGISTER_VALUE(val, x, 0, 13)
#define D4_PP_CORE_XYS_H_OB_END_POSITION(val, x) \
    SET_REGISTER_VALUE(val, x, 16, 13)

#define D4_PP_CORE_XYS_V_OB_START_POSITION(val, x) \
    SET_REGISTER_VALUE(val, x, 0, 13)
#define D4_PP_CORE_XYS_V_OB_END_POSITION(val, x) \
    SET_REGISTER_VALUE(val, x, 16, 13)

#define D4_PP_CORE_XYS_REG_OFFSET_1_OFFSET_00(val, x) \
    SET_REGISTER_VALUE(val, x, 0, 14)
#define D4_PP_CORE_XYS_REG_OFFSET_1_OFFSET_01(val, x) \
    SET_REGISTER_VALUE(val, x, 16, 14)

#define D4_PP_CORE_XYS_REG_OFFSET_2_OFFSET_10(val, x) \
    SET_REGISTER_VALUE(val, x, 0, 14)
#define D4_PP_CORE_XYS_REG_OFFSET_2_OFFSET_11(val, x) \
    SET_REGISTER_VALUE(val, x, 16, 14)

#define D4_PP_CORE_XYS_REG_OB_OFFSET_1_OFFSET_00(val, x) \
    SET_REGISTER_VALUE(val, x, 0, 14)
#define D4_PP_CORE_XYS_REG_OB_OFFSET_1_OFFSET_01(val, x) \
    SET_REGISTER_VALUE(val, x, 16, 14)

#define D4_PP_CORE_XYS_REG_OB_OFFSET_2_OFFSET_10(val, x) \
    SET_REGISTER_VALUE(val, x, 0, 14)
#define D4_PP_CORE_XYS_REG_OB_OFFSET_2_OFFSET_11(val, x) \
    SET_REGISTER_VALUE(val, x, 16, 14)

#define D4_PP_CORE_XYS_SATURATION_THRESHOLD_SET(val, x) \
    SET_REGISTER_VALUE(val, x, 0, 14)


/**< HBR( Horizontal Bayer Resize ) Control Register */
#define D4_PP_CORE_HBR_CTRL_ON(val, x) \
    SET_REGISTER_VALUE(val, x, 0, 1)
#define D4_PP_CORE_HBR_CTRL_GET_ONOFF(val) \
    GET_REGISTER_VALUE(val, 0, 1)
#define D4_PP_CORE_HBR_CTRL_IN_CH_MODE(val, x) \
    SET_REGISTER_VALUE(val, x, 4, 2)
#define D4_PP_CORE_HBR_CTRL_GET_IN_CH_MODE(val) \
    GET_REGISTER_VALUE(val, 4, 2)
#define D4_PP_CORE_HBR_CTRL_OUT_CH_MODE(val, x) \
    SET_REGISTER_VALUE(val, x, 8, 2)
#define D4_PP_CORE_HBR_CTRL_LUT_UPDATE_MODE(val, x) \
    SET_REGISTER_VALUE(val, x, 12, 1)


#define D4_PP_CORE_HBR_IN_SIZE_H_SIZE(val, x) \
    SET_REGISTER_VALUE(val, x, 0, 14)
#define D4_PP_CORE_HBR_IN_SIZE_V_SIZE(val, x) \
    SET_REGISTER_VALUE(val, x, 16, 14)


#define D4_PP_CORE_HBR_SKIP_SIZE_LEFT_SKIP_SIZE(val, x) \
    SET_REGISTER_VALUE(val, x, 0, 14)
#define D4_PP_CORE_HBR_SKIP_SIZE_RIGHT_SKIP_SIZE(val, x) \
    SET_REGISTER_VALUE(val, x, 16, 14)


#define D4_PP_CORE_HBR_OUT_SIZE_H_SIZE(val, x) \
    SET_REGISTER_VALUE(val, x, 0, 14)
#define D4_PP_CORE_HBR_OUT_SIZE_PHASE_NUM(val, x) \
    SET_REGISTER_VALUE(val, x, 16, 8)


#define D4_PP_CORE_HBR_SHOOT_CTRL_1_CTRL_1(val, x) \
    SET_REGISTER_VALUE(val, x, 0, 10)
#define D4_PP_CORE_HBR_SHOOT_CTRL_1_CTRL_2(val, x) \
    SET_REGISTER_VALUE(val, x, 16, 10)


#define D4_PP_CORE_HBR_SHOOT_CTRL_2_ALPHA(val, x) \
    SET_REGISTER_VALUE(val, x, 0, 10)
#define D4_PP_CORE_HBR_SHOOT_CTRL_2_BETA(val, x) \
    SET_REGISTER_VALUE(val, x, 16, 11)

/**< VBR( Vertical Bayer Resize ) Control Register */
#define D4_PP_CORE_VBR_CTRL_ON(val, x) \
    SET_REGISTER_VALUE(val, x, 0, 1)
#define D4_PP_CORE_VBR_CTRL_GET_ONOFF(val) \
    GET_REGISTER_VALUE(val, 0, 1)
#define D4_PP_CORE_VBR_CTRL_IN_CH_MODE(val, x) \
    SET_REGISTER_VALUE(val, x, 4, 2)
#define D4_PP_CORE_VBR_CTRL_LINE_SKIP_ENABLE(val, x) \
    SET_REGISTER_VALUE(val, x, 8, 1)
#define D4_PP_CORE_VBR_CTRL_LUT_UPDATE_MODE(val, x) \
    SET_REGISTER_VALUE(val, x, 12, 1)


#define D4_PP_CORE_VBR_IN_SIZE_H_SIZE(val, x) \
    SET_REGISTER_VALUE(val, x, 0, 13)
#define D4_PP_CORE_VBR_IN_SIZE_V_SIZE(val, x) \
    SET_REGISTER_VALUE(val, x, 16, 13)


#define D4_PP_CORE_VBR_SKIP_SIZE_TOP_SKIP_POSITION(val, x) \
    SET_REGISTER_VALUE(val, x, 0, 13)
#define D4_PP_CORE_VBR_SKIP_SIZE_BOTTOM_SKIP_POSITION(val, x) \
    SET_REGISTER_VALUE(val, x, 16, 13)


#define D4_PP_CORE_VBR_OUT_SIZE_V_SIZE(val, x) \
    SET_REGISTER_VALUE(val, x, 0, 13)
#define D4_PP_CORE_VBR_OUT_SIZE_PHASE_NUM(val, x) \
    SET_REGISTER_VALUE(val, x, 16, 8)


#define D4_PP_CORE_VBR_SHOOT_CTRL_1_CTRL_1(val, x) \
    SET_REGISTER_VALUE(val, x, 0, 10)
#define D4_PP_CORE_VBR_SHOOT_CTRL_1_CTRL_2(val, x) \
    SET_REGISTER_VALUE(val, x, 16, 10)


#define D4_PP_CORE_VBR_SHOOT_CTRL_2_ALPHA(val, x) \
    SET_REGISTER_VALUE(val, x, 0, 10)
#define D4_PP_CORE_VBR_SHOOT_CTRL_2_BETA(val, x) \
    SET_REGISTER_VALUE(val, x, 16, 11)

/**< BG( Bayer Gamma ) Control Register */
#define D4_PP_CORE_BG_CTRL_BG_ON(val, x) \
	    SET_REGISTER_VALUE(val, x, 0, 1)
#define D4_PP_CORE_BG_CTRL_GET_BG_ONOFF(val) \
	    GET_REGISTER_VALUE(val, 0, 1)
#define D4_PP_CORE_BG_CTRL_OB_REGION_CORRECTION(val, x) \
	    SET_REGISTER_VALUE(val, x, 4, 1)
#define D4_PP_CORE_BG_CTRL_14_BIT_MSB_ALIGN_ON(val, x) \
	    SET_REGISTER_VALUE(val, x, 8, 1)
#define D4_PP_CORE_BG_CTRL_UPDATE_ON(val, x) \
	    SET_REGISTER_VALUE(val, x, 12, 1)

#define D4_PP_CORE_BG_TOP_OB_TOP_OB_START(val, x) \
	    SET_REGISTER_VALUE(val, x, 0, 13)
#define D4_PP_CORE_BG_TOP_OB_TOP_OB_END(val, x) \
	    SET_REGISTER_VALUE(val, x, 16, 13)

#define D4_PP_CORE_BG_TOP_OB_SIDE_OB_START(val, x) \
	    SET_REGISTER_VALUE(val, x, 0, 13)
#define D4_PP_CORE_BG_TOP_OB_SIDE_OB_END(val, x) \
	    SET_REGISTER_VALUE(val, x, 16, 13)

/**< HRN( Horizontal Random Noise Reduction ) Control Register */
#define D4_PP_CORE_HRN_CTRL_ON(val, x) \
	    SET_REGISTER_VALUE(val, x, 0, 1)
#define D4_PP_CORE_HRN_CTRL_GET_ONOFF(val) \
	    GET_REGISTER_VALUE(val, 0, 1)
#define D4_PP_CORE_HRN_CTRL_USE_OBD_DATA(val, x) \
	    SET_REGISTER_VALUE(val, x, 4, 2)
#define D4_PP_CORE_HRN_CTRL_MEDIAN_FILTER_ON(val, x) \
	    SET_REGISTER_VALUE(val, x, 8, 1)
#define D4_PP_CORE_HRN_CTRL_SIDE_OB_TYPE(val, x) \
	    SET_REGISTER_VALUE(val, x, 12, 1)
#define D4_PP_CORE_HRN_CTRL_USE_DIFF_UP_THERESHOLD(val, x) \
	    SET_REGISTER_VALUE(val, x, 16, 1)
#define D4_PP_CORE_HRN_CTRL_TOP_OB_BYPASS(val, x) \
	    SET_REGISTER_VALUE(val, x, 20, 1)
#define D4_PP_CORE_HRN_CTRL_TOGGLE_SIGN(val, x) \
	    SET_REGISTER_VALUE(val, x, 24, 1)
#define D4_PP_CORE_HRN_CTRL_CHANGE_COMP(val, x) \
	    SET_REGISTER_VALUE(val, x, 28, 1)

#define D4_PP_CORE_HRN_USE_SIDE_OB_START_POSITION(val, x) \
	    SET_REGISTER_VALUE(val, x, 0, 14)
#define D4_PP_CORE_HRN_USE_SIDE_OB_END_POSITION(val, x) \
	    SET_REGISTER_VALUE(val, x, 16, 14)

#define D4_PP_CORE_HRN_MANUAL_CTRL_1_OB_0_0(val, x) \
	    SET_REGISTER_VALUE(val, x, 0, 14)
#define D4_PP_CORE_HRN_MANUAL_CTRL_1_OB_0_1(val, x) \
	    SET_REGISTER_VALUE(val, x, 16, 14)

#define D4_PP_CORE_HRN_MANUAL_CTRL_2_OB_1_0(val, x) \
	    SET_REGISTER_VALUE(val, x, 0, 14)
#define D4_PP_CORE_HRN_MANUAL_CTRL_2_OB_1_1(val, x) \
	    SET_REGISTER_VALUE(val, x, 16, 14)

#define D4_PP_CORE_HRN_TOP_OB_SIZE_DIFF_UP_THERESHOLD(val, x) \
	    SET_REGISTER_VALUE(val, x, 0, 14)
#define D4_PP_CORE_HRN_TOP_OB_SIZE_SET_SIZE(val, x) \
	    SET_REGISTER_VALUE(val, x, 16, 10)

#define D4_PP_CORE_HRN_SIDE_OB_START_POSITION(val, x) \
	    SET_REGISTER_VALUE(val, x, 0, 14)
#define D4_PP_CORE_HRN_SIDE_OB_END_POSITION(val, x) \
	    SET_REGISTER_VALUE(val, x, 16, 14)

#define D4_PP_CORE_HRN_TOP_OB_POSITION_SET_POSITION(val, x) \
	    SET_REGISTER_VALUE(val, x, 0, 5)
#define D4_PP_CORE_HRN_TOP_OB_POSITION_BLOCK_NUM(val, x) \
	    SET_REGISTER_VALUE(val, x, 16, 14)

/**< VFPN( Vertical Fixed Pattern Noise Correction ) Control Register */
#define PP_CORE_VFPN_CTRL_VFPN_ON(val, x) \
	    SET_REGISTER_VALUE(val, x, 0, 1)
#define PP_CORE_VFPN_CTRL_GET_VFPN_ONOFF(val) \
	    GET_REGISTER_VALUE(val, 0, 1)
#define PP_CORE_VFPN_CTRL_MEDIA_ON(val, x) \
	    SET_REGISTER_VALUE(val, x, 4, 1)
#define PP_CORE_VFPN_CTRL_H_SIZE(val, x) \
	    SET_REGISTER_VALUE(val, x, 16, 14)

#define PP_CORE_VFPN_OB_INFO_V_OB_START(val, x) \
	    SET_REGISTER_VALUE(val, x, 0, 13)
#define PP_CORE_VFPN_OB_INFO_V_OB_END(val, x) \
	    SET_REGISTER_VALUE(val, x, 16, 13)

#define PP_CORE_VFPN_RESULT_SAVE_START_ADDR_SET(val, x) \
	    SET_REGISTER_VALUE(val, x, 0, 32)

#define PP_CORE_VFPN_RESULT_SAVE_END_ADDR_SET(val, x) \
	    SET_REGISTER_VALUE(val, x, 0, 32)

/**< CS( Color Shading ) Control Register */
#define D4_PP_CORE_CS_CTRL_CS_ON(val, x) \
	    SET_REGISTER_VALUE(val, x, 0, 1)
#define D4_PP_CORE_CS_CTRL_GET_CS_ONOFF(val) \
	    GET_REGISTER_VALUE(val, 0, 1)
#define D4_PP_CORE_CS_CTRL_LUT_INIT_EN(val, x) \
	    SET_REGISTER_VALUE(val, x, 4, 1)
#define D4_PP_CORE_CS_CTRL_LUT_UPDATE_MODE(val, x) \
	    SET_REGISTER_VALUE(val, x, 8, 1)
#define D4_PP_CORE_CS_CTRL_MIRROR_INTERPOLATION_ON(val, x) \
	    SET_REGISTER_VALUE(val, x, 12, 1)
#define D4_PP_CORE_CS_CTRL_PP_MODE(val, x) \
	    SET_REGISTER_VALUE(val, x, 16, 1)


#define D4_PP_CORE_CS_SAMPLE_DIVIDER_SAMPLE_POINT(val, x) \
	    SET_REGISTER_VALUE(val, x, 0, 8)
#define D4_PP_CORE_CS_SAMPLE_DIVIDER_INPUT_CHANNEL(val, x) \
	    SET_REGISTER_VALUE(val, x, 8, 2)

#define D4_PP_CORE_CS_INPUT_SIZE_H(val, x) \
	    SET_REGISTER_VALUE(val, x, 0, 13)
#define D4_PP_CORE_CS_INPUT_SIZE_V(val, x) \
	    SET_REGISTER_VALUE(val, x, 16, 13)

#define D4_PP_CORE_CS_START_POSITION_START_H(val, x) \
	    SET_REGISTER_VALUE(val, x, 0, 13)
#define D4_PP_CORE_CS_START_POSITION_START_V(val, x) \
	    SET_REGISTER_VALUE(val, x, 16, 13)

#define D4_PP_CORE_CS_INTERPOLATION_RATIO_GAIN(val, x) \
	    SET_REGISTER_VALUE(val, x, 0, 8)
#define D4_PP_CORE_CS_INTERPOLATION_RATIO_1_LUT_SIZE(val, x) \
	    SET_REGISTER_VALUE(val, x, 8, 13)

#define PP_CORE_CS_VS_RATIO_H_SET(val, x) \
	    SET_REGISTER_VALUE(val, x, 0, 13)
#define PP_CORE_CS_VS_RATIO_V_SET(val, x) \
	    SET_REGISTER_VALUE(val, x, 16, 13)

#define D4_PP_CORE_CS_GAIN_RATIO_1_R_GAIN(val, x) \
	    SET_REGISTER_VALUE(val, x, 0, 14)
#define D4_PP_CORE_CS_GAIN_RATIO_1_GR_GAIN(val, x) \
	    SET_REGISTER_VALUE(val, x, 16, 14)
#define D4_PP_CORE_CS_GAIN_RATIO_2_GB_GAIN(val, x) \
	    SET_REGISTER_VALUE(val, x, 0, 14)
#define D4_PP_CORE_CS_GAIN_RATIO_2_B_GAIN(val, x) \
	    SET_REGISTER_VALUE(val, x, 16, 14)

#define D4_PP_CORE_CS_LUT_INFO_LUT_X_SIZE(val, x) \
	    SET_REGISTER_VALUE(val, x, 0, 9)
#define D4_PP_CORE_CS_LUT_INFO_LUT_BAYER_FORMAT(val, x) \
	    SET_REGISTER_VALUE(val, x, 12, 2)
#define D4_PP_CORE_CS_LUT_INFO_LUT_SAMPLE_POINT(val, x) \
	    SET_REGISTER_VALUE(val, x, 16, 8)

/**< DAMP( Digital AMP ) Control Register */

#define D4_PP_CORE_DAMP_CTRL_DAMP_ON(val, x) \
    SET_REGISTER_VALUE(val, x, 0, 1)
#define D4_PP_CORE_DAMP_CTRL_GET_DAMP_ONOFF(val) \
    GET_REGISTER_VALUE(val, 0, 1)
#define D4_PP_CORE_DAMP_OFFSET_1_P01(val, x) \
    SET_REGISTER_VALUE(val, x, 0, 15)
#define D4_PP_CORE_DAMP_OFFSET_1_P00(val, x) \
    SET_REGISTER_VALUE(val, x, 16, 15)
#define D4_PP_CORE_DAMP_OFFSET_2_P11(val, x) \
    SET_REGISTER_VALUE(val, x, 0, 15)
#define D4_PP_CORE_DAMP_OFFSET_2_P10(val, x) \
    SET_REGISTER_VALUE(val, x, 16, 15)
#define D4_PP_CORE_DAMP_GAIN_1_P01(val, x) \
    SET_REGISTER_VALUE(val, x, 0, 11)
#define D4_PP_CORE_DAMP_GAIN_1_P00(val, x) \
    SET_REGISTER_VALUE(val, x, 16, 11)
#define D4_PP_CORE_DAMP_GAIN_2_P11(val, x) \
    SET_REGISTER_VALUE(val, x, 0, 11)
#define D4_PP_CORE_DAMP_GAIN_2_P10(val, x) \
    SET_REGISTER_VALUE(val, x, 16, 11)

/**< OBD( Optical Black Detection ) Control Register */

#define D4_PP_CORE_OBD_CTRL_OBD_ON(val, x) \
    SET_REGISTER_VALUE(val, x, 0, 1)
#define D4_PP_CORE_OBD_CTRL_GET_OBD_ONOFF(val) \
    GET_REGISTER_VALUE(val, 0, 1)
#define D4_PP_CORE_OBD_CTRL_V_MEDIAAN_ON(val, x) \
    SET_REGISTER_VALUE(val, x, 1, 1)
#define D4_PP_CORE_OBD_CTRL_H_MEDIAAN_ON(val, x) \
    SET_REGISTER_VALUE(val, x, 2, 1)
#define D4_PP_CORE_OBD_CTRL_CLIP_MODE_ON(val, x) \
    SET_REGISTER_VALUE(val, x, 4, 1)
#define D4_PP_CORE_OBD_CTRL_CLIP_THRESHOLD(val, x) \
    SET_REGISTER_VALUE(val, x, 8, 14)


#define D4_PP_CORE_OBD_H_POS_END(val, x) \
    SET_REGISTER_VALUE(val, x, 0, 13)
#define D4_PP_CORE_OBD_H_POS_START(val, x) \
    SET_REGISTER_VALUE(val, x, 16, 13)


#define D4_PP_CORE_OBD_V_POS_END(val, x) \
    SET_REGISTER_VALUE(val, x, 0, 13)
#define D4_PP_CORE_OBD_V_POS_START(val, x) \
    SET_REGISTER_VALUE(val, x, 16, 13)


#define D4_PP_CORE_OBD_RESULT_1_P01(val, x) \
    SET_REGISTER_VALUE(val, x, 0, 14)
#define D4_PP_CORE_OBD_RESULT_1_P00(val, x) \
    SET_REGISTER_VALUE(val, x, 16, 14)

#define D4_PP_CORE_OBD_RESULT_2_p11(val, x) \
    SET_REGISTER_VALUE(val, x, 0, 14)
#define D4_PP_CORE_OBD_RESULT_2_p10(val, x) \
    SET_REGISTER_VALUE(val, x, 16, 14)

#define D4_PP_CORE_OBD_GET_RESULT_1_P01(val) \
    GET_REGISTER_VALUE(val, 0, 14)
#define D4_PP_CORE_OBD_GET_RESULT_1_P00(val) \
    GET_REGISTER_VALUE(val, 16, 14)

#define D4_PP_CORE_OBD_GET_RESULT_2_p11(val) \
    GET_REGISTER_VALUE(val, 0, 14)
#define D4_PP_CORE_OBD_GET_RESULT_2_p10(val) \
    GET_REGISTER_VALUE(val, 16, 14)

/**< OOC( Optical Black Correction ) Control Register */
#define D4_PP_CORE_OOC_CTRL_OOC_ON(val, x) \
    SET_REGISTER_VALUE(val, x, 0, 1)
#define D4_PP_CORE_OOC_CTRL_GET_OOC_ONOFF(val) \
    GET_REGISTER_VALUE(val, 0, 1)
#define D4_PP_CORE_OOC_CTRL_OB_SEL(val, x) \
    SET_REGISTER_VALUE(val, x, 8, 2)
#define D4_PP_CORE_OOC_CTRL_EXCEPT_OB_MODE_ON(val, x) \
    SET_REGISTER_VALUE(val, x, 16, 1)


#define D4_PP_CORE_OOC_H_POS_END(val, x) \
    SET_REGISTER_VALUE(val, x, 0, 13)
#define D4_PP_CORE_OOC_H_POS_START(val, x) \
    SET_REGISTER_VALUE(val, x, 16, 13)


#define D4_PP_CORE_OOC_V_POS_END(val, x) \
    SET_REGISTER_VALUE(val, x, 0, 13)
#define D4_PP_CORE_OOC_V_POS_START(val, x) \
    SET_REGISTER_VALUE(val, x, 16, 13)


#define D4_PP_CORE_OOC_MANUAL_SET_1_P01(val, x) \
    SET_REGISTER_VALUE(val, x, 0, 14)
#define D4_PP_CORE_OOC_MANUAL_SET_1_P00(val, x) \
    SET_REGISTER_VALUE(val, x, 16, 14)


#define D4_PP_CORE_OOC_MANUAL_SET_2_P11(val, x) \
    SET_REGISTER_VALUE(val, x, 0, 14)
#define D4_PP_CORE_OOC_MANUAL_SET_2_P10(val, x) \
    SET_REGISTER_VALUE(val, x, 16, 14)

/**< OBR( Optical Black Remove ) Control Register */
#define D4_PP_CORE_OBR_CTRL_OBR_ON(val, x) \
    SET_REGISTER_VALUE(val, x, 0, 1)
#define D4_PP_CORE_OBR_CTRL_GET_OBR_ONOFF(val) \
    GET_REGISTER_VALUE(val, 0, 1)

#define D4_PP_CORE_OBR_H_POS_END(val, x) \
    SET_REGISTER_VALUE(val, x, 0, 14)
#define D4_PP_CORE_OBR_H_POS_START(val, x) \
    SET_REGISTER_VALUE(val, x, 16, 14)


#define D4_PP_CORE_OBR_V_POS_END(val, x) \
    SET_REGISTER_VALUE(val, x, 0, 14)
#define D4_PP_CORE_OBR_V_POS_START(val, x) \
    SET_REGISTER_VALUE(val, x, 16, 14)

/**< KNEE Control Register */

#define D4_PP_CORE_KNEE_TH_STEP_1_STEP1(val, x) \
    SET_REGISTER_VALUE(val, x, 0, 14)
#define D4_PP_CORE_KNEE_TH_STEP_1_STEP0(val, x) \
    SET_REGISTER_VALUE(val, x, 16, 14)


#define D4_PP_CORE_KNEE_TH_STEP_2_STEP3(val, x) \
    SET_REGISTER_VALUE(val, x, 0, 14)
#define D4_PP_CORE_KNEE_TH_STEP_2_STEP2(val, x) \
    SET_REGISTER_VALUE(val, x, 16, 14)


#define D4_PP_CORE_KNEE_TH_STEP_3_STEP5(val, x) \
    SET_REGISTER_VALUE(val, x, 0, 14)
#define D4_PP_CORE_KNEE_TH_STEP_3_STEP4(val, x) \
    SET_REGISTER_VALUE(val, x, 16, 14)


#define D4_PP_CORE_KNEE_TH_STEP_4_STEP7(val, x) \
    SET_REGISTER_VALUE(val, x, 0, 14)
#define D4_PP_CORE_KNEE_TH_STEP_4_STEP6(val, x) \
    SET_REGISTER_VALUE(val, x, 16, 14)


#define D4_PP_CORE_KNEE_TH_STEP_5_STEP8(val, x) \
    SET_REGISTER_VALUE(val, x, 16, 14)


#define D4_PP_CORE_KNEE_GAIN_1_GAIN1(val, x) \
    SET_REGISTER_VALUE(val, x, 0, 10)
#define D4_PP_CORE_KNEE_GAIN_1_GAIN0(val, x) \
    SET_REGISTER_VALUE(val, x, 16, 10)


#define D4_PP_CORE_KNEE_GAIN_2_GAIN3(val, x) \
    SET_REGISTER_VALUE(val, x, 0, 10)
#define D4_PP_CORE_KNEE_GAIN_2_GAIN2(val, x) \
    SET_REGISTER_VALUE(val, x, 16, 10)


#define D4_PP_CORE_KNEE_GAIN_3_GAIN5(val, x) \
    SET_REGISTER_VALUE(val, x, 0, 10)
#define D4_PP_CORE_KNEE_GAIN_3_GAIN4(val, x) \
    SET_REGISTER_VALUE(val, x, 16, 10)


#define D4_PP_CORE_KNEE_GAIN_4_GAIN7(val, x) \
    SET_REGISTER_VALUE(val, x, 0, 10)
#define D4_PP_CORE_KNEE_GAIN_4_GAIN6(val, x) \
    SET_REGISTER_VALUE(val, x, 16, 10)


#define D4_PP_CORE_KNEE_GAIN_5_GAIN9(val, x) \
    SET_REGISTER_VALUE(val, x, 0, 10)
#define D4_PP_CORE_KNEE_GAIN_5_GAIN8(val, x) \
    SET_REGISTER_VALUE(val, x, 16, 10)


#define D4_PP_CORE_KNEE_OFFSET_1_OFFSET1(val, x) \
    SET_REGISTER_VALUE(val, x, 0, 12)
#define D4_PP_CORE_KNEE_OFFSET_1_OFFSET0(val, x) \
    SET_REGISTER_VALUE(val, x, 16, 12)


#define D4_PP_CORE_KNEE_OFFSET_2_OFFSET3(val, x) \
    SET_REGISTER_VALUE(val, x, 0, 12)
#define D4_PP_CORE_KNEE_OFFSET_2_OFFSET2(val, x) \
    SET_REGISTER_VALUE(val, x, 16, 12)


#define D4_PP_CORE_KNEE_OFFSET_3_OFFSET5(val, x) \
    SET_REGISTER_VALUE(val, x, 0, 12)
#define D4_PP_CORE_KNEE_OFFSET_3_OFFSET4(val, x) \
    SET_REGISTER_VALUE(val, x, 16, 12)


#define D4_PP_CORE_KNEE_OFFSET_4_OFFSET7(val, x) \
    SET_REGISTER_VALUE(val, x, 0, 12)
#define D4_PP_CORE_KNEE_OFFSET_4_OFFSET6(val, x) \
    SET_REGISTER_VALUE(val, x, 16, 12)


#define D4_PP_CORE_KNEE_OFFSET_5_OFFSET9(val, x) \
    SET_REGISTER_VALUE(val, x, 0, 12)
#define D4_PP_CORE_KNEE_OFFSET_5_OFFSET8(val, x) \
    SET_REGISTER_VALUE(val, x, 16, 12)


#define D4_PP_CORE_KNEE_CTRL_KNEE_ON(val, x) \
    SET_REGISTER_VALUE(val, x, 0, 1)
#define D4_PP_CORE_KNEE_CTRL_GET_KNEE_ONOFF(val) \
    GET_REGISTER_VALUE(val, 0, 1)

#define D4_PP_CORE_KNEE_CTRL_OB_ON(val, x) \
    SET_REGISTER_VALUE(val, x, 4, 1)
#define D4_PP_CORE_KNEE_CTRL_OUT_BIT_WIDTH(val, x) \
    SET_REGISTER_VALUE(val, x, 8, 2)


#define D4_PP_CORE_KNEE_OB_START_OB_H(val, x) \
    SET_REGISTER_VALUE(val, x, 0, 13)
#define D4_PP_CORE_KNEE_OB_START_OB_V(val, x) \
    SET_REGISTER_VALUE(val, x, 16, 13)


#define D4_PP_CORE_KNEE_OB_END_OB_H(val, x) \
    SET_REGISTER_VALUE(val, x, 0, 13)
#define D4_PP_CORE_KNEE_OB_END_OB_V(val, x) \
    SET_REGISTER_VALUE(val, x, 16, 13)

/**< PP - MDMA Control Register */
#define D4_PP_CORE_MDMA_ID_CTRL(val, x) \
    SET_REGISTER_VALUE(val, x, 0, 3)

/**< PP - RDMA Control Register */
#define D4_PP_RDMA_CTRL_ENABLE_0(val, x) \
    SET_REGISTER_VALUE(val, x, 0, 1)
#define D4_PP_RDMA_CTRL_ENABLE_1(val, x) \
    SET_REGISTER_VALUE(val, x, 1, 1)
#define D4_PP_RDMA_CTRL_BIT_WIDTH_0(val, x) \
    SET_REGISTER_VALUE(val, x, 4, 4)
#define D4_PP_RDMA_CTRL_BIT_WIDTH_1(val, x) \
    SET_REGISTER_VALUE(val, x, 8, 4)
#define D4_PP_RDMA_CTRL_V_REPEAT_0(val, x) \
    SET_REGISTER_VALUE(val, x, 16, 1)
#define D4_PP_RDMA_CTRL_V_REPEAT_1(val, x) \
    SET_REGISTER_VALUE(val, x, 17, 1)
#define D4_PP_RDMA_CTRL_H_REPEAT_0(val, x) \
    SET_REGISTER_VALUE(val, x, 20, 1)
#define D4_PP_RDMA_CTRL_H_REPEAT_1(val, x) \
    SET_REGISTER_VALUE(val, x, 24, 1)

#define D4_PP_RDMA_WIDTH_0_SET(val, x) \
    SET_REGISTER_VALUE(val, x, 0, 13)
#define D4_PP_RDMA_WIDTH_1_SET(val, x) \
    SET_REGISTER_VALUE(val, x, 16, 13)

#define D4_PP_RDMA_HEIGHT_SET(val, x) \
    SET_REGISTER_VALUE(val, x, 16, 16)

#define D4_PP_RDMA_DELTA_ADDR_SET(val, x) \
    SET_REGISTER_VALUE(val, x, 0, 17)
#define D4_PP_RDMA_DELTA_ADDR_INV_MODE(val, x) \
    SET_REGISTER_VALUE(val, x, 28, 1)

#define PP_RDMA_CH_MODE_BURST_LEN(val, x) \
    SET_REGISTER_VALUE(val, x, 0, 4)
#define PP_RDMA_CH_MODE_SET(val, x) \
    SET_REGISTER_VALUE(val, x, 4, 3)

/**< PP - WDMA Control Register */
#define D4_PP_WDMA_BURST_LEN_BURST_LEN(val, x) \
    SET_REGISTER_VALUE(val, x, 0, 4)
#define D4_PP_WDMA_BURST_LEN_SYNC_DE_DEBUG(val, x) \
    SET_REGISTER_VALUE(val, x, 4, 4)
#define D4_PP_WDMA_BURST_LEN_STATE_DEBUG(val, x) \
    SET_REGISTER_VALUE(val, x, 8, 14)

#define D4_PP_WDMA_RESET_ON(val, x) \
    SET_REGISTER_VALUE(val, x, 0, 1)

#define D4_PP_WDMA_CTRL_SECC(val, x) \
    SET_REGISTER_VALUE(val, x, 0, 1)
#define D4_PP_WDMA_CTRL_INT_END_TYPE(val, x) \
    SET_REGISTER_VALUE(val, x, 1, 1)
#define D4_PP_WDMA_CTRL_BIT_FORMAT(val, x) \
    SET_REGISTER_VALUE(val, x, 4, 3)
#define D4_PP_WDMA_CTRL_GET_BIT_FORMAT(val) \
    GET_REGISTER_VALUE(val, 4, 3)
#define D4_PP_WDMA_CTRL_IN_CHANNEL(val, x) \
    SET_REGISTER_VALUE(val, x, 8, 3)
#define D4_PP_WDMA_CTRL_GET_IN_CHANNEL(val) \
    GET_REGISTER_VALUE(val, 8, 3)
#define D4_PP_WDMA_CTRL_VSYNC_WRITE_ENABLE(val, x) \
    SET_REGISTER_VALUE(val, x, 28, 3)

/**< PP - DMA Control Register */
#define D4_PP_DMA_START_CTRL_SET(val, x) \
    SET_REGISTER_VALUE(val, x, 0, 1)

#define D4_PP_DMA_ENABLE_CTRL_SM_ENABLE(val, x) \
    SET_REGISTER_VALUE(val, x, 0, 1)
#define D4_PP_DMA_ENABLE_CTRL_WDMA_ENABLE(val, x) \
    SET_REGISTER_VALUE(val, x, 1, 1)
#define D4_PP_DMA_ENABLE_CTRL_GET_WDMA_ENABLE(val) \
    GET_REGISTER_VALUE(val, 1, 1)
#define D4_PP_DMA_ENABLE_CTRL_RDMA_ENABLE(val, x) \
    SET_REGISTER_VALUE(val, x, 2, 1)
#define D4_PP_DMA_ENABLE_CTRL_GET_RDMA_ENABLE(val) \
    GET_REGISTER_VALUE(val, 2, 1)
#define D4_PP_DMA_ENABLE_CTRL_WSDMA_ENABLE(val, x) \
    SET_REGISTER_VALUE(val, x, 3, 1)
#define D4_PP_DMA_ENABLE_CTRL_MIPI_WDMA_ENABLE(val, x) \
    SET_REGISTER_VALUE(val, x, 3, 1)
#define D4_PP_DMA_ENABLE_CTRL_MIPI_RDMA_ENABLE(val, x) \
    SET_REGISTER_VALUE(val, x, 4, 1)


#endif /* _DRIME4_REGS_PP_CORE_H_ */

