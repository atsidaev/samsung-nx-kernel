/**
 * @file d4_dp_global_reg.h
 * @brief DRIMe4 DP Register Define for Device Driver
 * @author sejong oh<sejong55.oh@samsung.com>
 * 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef _DP_COMMON_REG_
#define _DP_COMMON_REG_

#include <mach/d4_reg_macro.h>

/* global sub register */
#define dp_start_update                  0x00
#define dp_clk_en                        0x04
#define dp_rst_ctrl                      0x08
#define dp_pwr_ctrl                      0x0c
#define dp_int_on                        0x10
#define dp_int_set_clr                   0x14
#define dp_periodic_int_cnt              0x18
#define dp_apb_rdy_cnt                   0x1c
#define dp_nlc_ctrl                      0x20
#define dp_evf_on_path                   0x24
#define dp_bist_ctrl                     0x28
#define dp_test_ctrl                     0x2c

/* NLC register */
#define dp_nlc_y_tile_heigh              0x1300
#define dp_nlc_y_start_addr              0x1304
#define dp_nlc_y_addr_offset             0x1308
#define dp_nlc_buffer_check              0x130C
#define dp_nlc_c_tile_heigh              0x1320
#define dp_nlc_c_start_addr              0x1324
#define dp_nlc_c_addr_offset             0x1328
#define dp_nlc_rate_ctrl                 0x132C
#define dp_nlc_fmt                       0x1318
#define dp_nlc_burst_len                 0x131C
#define dp_nlc_fmt_width                 0x1310

#define dp_nlc_y_tile_heigh_grp0         0x1000
#define dp_nlc_y_start_addr_grp0         0x1004
#define dp_nlc_y_addr_offset_grp0        0x1008
#define dp_nlc_buffer_check_grp0         0x100C
#define dp_nlc_c_tile_heigh_grp0         0x1020

#define DP_NLC_C_START_ADDR_GRP0         0x1024
#define DP_NLC_C_ADDR_OFFSET_GRP0        0x1028
#define DP_NLC_RATE_CTRL_GRP0            0x102C
#define DP_NLC_FMT_GRP0                  0x1018
#define DP_NLC_BURST_LEN_GRP0            0x101C
#define DP_NLC_FMT_WIDTH_GRP0            0x1010

#define DP_NLC_Y_TILE_HEIGH_GRP1         0x1100
#define DP_NLC_Y_START_ADDR_GRP1         0x1104
#define DP_NLC_Y_ADDR_OFFSET_GRP1        0x1108
#define DP_NLC_BUFFER_CHECK_GRP1         0x110C
#define DP_NLC_C_TILE_HEIGH_GRP1         0x1120
#define DP_NLC_C_START_ADDR_GRP1         0x1124
#define DP_NLC_C_ADDR_OFFSET_GRP1        0x1128
#define DP_NLC_RATE_CTRL_GRP1            0x112C
#define DP_NLC_FMT_GRP1                  0x1118
#define DP_NLC_BURST_LEN_GRP1            0x111C
#define DP_NLC_FMT_WIDTH_GRP1            0x1110

#define DP_NLC_TILE_WIDTH_GROUP0         0x1200
#define DP_NLC_TILE_WIDTH_GROUP1         0x1204
#define DP_NLC_TILE_WIDTH_GROUP2         0x1208
#define DP_NLC_TILE_WIDTH_GROUP3         0x120c

/*          DP GLOBAL START           */
#define D4_DP_COMMON_TV_ON(val, x) \
    SET_REGISTER_VALUE(val, x, 0, 1)
#define D4_DP_COMMON_TV_AUTO(val, x) \
    SET_REGISTER_VALUE(val, x, 1, 1)
#define D4_DP_COMMON_TV_MANUAL(val, x) \
    SET_REGISTER_VALUE(val, x, 2, 1)
#define D4_DP_COMMON_MLCD_ON(val, x) \
    SET_REGISTER_VALUE(val, x, 4, 1)
#define D4_DP_COMMON_MLCD_AUTO(val, x) \
    SET_REGISTER_VALUE(val, x, 5, 1)
#define D4_DP_COMMON_MLCD_MANUAL(val, x) \
    SET_REGISTER_VALUE(val, x, 6, 1)
#define D4_DP_COMMON_SLCD_ON(val, x) \
    SET_REGISTER_VALUE(val, x, 8, 1)
#define D4_DP_COMMON_SLCD_AUTO(val, x) \
    SET_REGISTER_VALUE(val, x, 9, 1)
#define D4_DP_COMMON_SLCD_MANUAL(val, x) \
    SET_REGISTER_VALUE(val, x, 10, 1)

/*          DP GLOBAL CLOCK ENABLE           */
#define D4_DP_COMMON_SYS_CLK_EN(val, x) \
    SET_REGISTER_VALUE(val, x, 0, 1)
#define D4_DP_COMMON_TV_CLK_EN(val, x) \
    SET_REGISTER_VALUE(val, x, 1, 1)
#define D4_DP_COMMON_MLCD_SYS_CLK_EN(val, x) \
	SET_REGISTER_VALUE(val, x, 4, 1)
#define D4_DP_COMMON_MLCD_CLK_EN(val, x) \
	SET_REGISTER_VALUE(val, x, 5, 1)
#define D4_DP_COMMON_MLCD_CLK_INV(val, x) \
	SET_REGISTER_VALUE(val, x, 6, 1)
#define D4_DP_COMMON_SLCD_SYS_CLK_EN(val, x) \
	SET_REGISTER_VALUE(val, x, 8, 1)
#define D4_DP_COMMON_SLCD_CLK_EN(val, x) \
	SET_REGISTER_VALUE(val, x, 9, 1)
#define D4_DP_COMMON_SLCD_CLK_INV(val, x) \
	SET_REGISTER_VALUE(val, x, 10, 1)
#define D4_DP_COMMON_NLC_VID_CLK_EN(val, x) \
	SET_REGISTER_VALUE(val, x, 13, 1)
#define D4_DP_COMMON_NLC_GRP_CLK_EN(val, x) \
	SET_REGISTER_VALUE(val, x, 14, 1)
#define D4_DP_COMMON_HDMI_SYS_CLK_EN(val, x) \
	SET_REGISTER_VALUE(val, x, 16, 1)
#define D4_DP_COMMON_HDMI_VID_CLK_EN(val, x) \
	SET_REGISTER_VALUE(val, x, 17, 1)
#define D4_DP_COMMON_HDMI_TMDS_CLK_EN(val, x) \
	SET_REGISTER_VALUE(val, x, 18, 1)
#define D4_DP_COMMON_HDMI_REF_CLK_EN(val, x) \
	SET_REGISTER_VALUE(val, x, 19, 1)
#define D4_DP_COMMON_HDMI_PIXEL_CLK_EN(val, x) \
	SET_REGISTER_VALUE(val, x, 20, 1)

/*          DP GLOBAL RESET CONTROL           */
#define D4_DP_COMMON_RST_TV_SYS(val, x) \
	SET_REGISTER_VALUE(val, x, 0, 1)
#define D4_DP_COMMON_RST_TV(val, x) \
	SET_REGISTER_VALUE(val, x, 1, 1)
#define D4_DP_COMMON_RST_MLCD_SYS(val, x) \
	SET_REGISTER_VALUE(val, x, 4, 1)
#define D4_DP_COMMON_RST_MLCD(val, x) \
	SET_REGISTER_VALUE(val, x, 5, 1)
#define D4_DP_COMMON_RST_SLCD_SYS(val, x) \
	SET_REGISTER_VALUE(val, x, 8, 1)
#define D4_DP_COMMON_RST_SLCD(val, x) \
	SET_REGISTER_VALUE(val, x, 9, 1)
#define D4_DP_COMMON_RST_NLC_VID(val, x) \
	SET_REGISTER_VALUE(val, x, 13, 1)
#define D4_DP_COMMON_RST_NLC_GRP(val, x) \
	SET_REGISTER_VALUE(val, x, 14, 1)
#define D4_DP_COMMON_RST_HDMI_SYS(val, x) \
	SET_REGISTER_VALUE(val, x, 16, 1)
#define D4_DP_COMMON_RST_HDMI_VIDEO(val, x) \
	SET_REGISTER_VALUE(val, x, 17, 1)
#define D4_DP_COMMON_RST_HDMI_TMDS(val, x) \
	SET_REGISTER_VALUE(val, x, 18, 1)
#define D4_DP_COMMON_RST_HDMI_AUDIO(val, x) \
	SET_REGISTER_VALUE(val, x, 19, 1)

/*          DP GLOBAL PW CTRL           */
#define D4_DP_COMMON_HDMI_PHY_PD(val, x) \
    SET_REGISTER_VALUE(val, x, 0, 1)

/*          DP GLOBAL INTERRUPT ON           */
#define D4_DP_COMMON_TV_INT_ON(val, x) \
	SET_REGISTER_VALUE(val, x, 0, 1)
#define D4_DP_COMMON_TV_INT_FIELD(val, x) \
	SET_REGISTER_VALUE(val, x, 1, 1)
#define D4_DP_COMMON_MLCD_INT_ON(val, x) \
	SET_REGISTER_VALUE(val, x, 4, 1)
#define D4_DP_COMMON_MLCD_INT_FIELD(val, x) \
	SET_REGISTER_VALUE(val, x, 5, 1)
#define D4_DP_COMMON_SLCD_INT_ON(val, x) \
	SET_REGISTER_VALUE(val, x, 8, 1)
#define D4_DP_COMMON_SLCD_INT_FIELD(val, x) \
	SET_REGISTER_VALUE(val, x, 9, 1)
#define D4_DP_COMMON_PERIODIC_INT_ON(val, x) \
	SET_REGISTER_VALUE(val, x, 12, 1)
#define D4_DP_COMMON_PERIODIC_INT_RST(val, x) \
	SET_REGISTER_VALUE(val, x, 13, 1)

/*          DP GLOBAL INTERRUPT CLEAR           */
#define D4_DP_COMMON_TV_INT_SET(val, x) \
	SET_REGISTER_VALUE(val, x, 0, 1)
#define D4_DP_COMMON_TV_INT_CLR(val, x) \
	SET_REGISTER_VALUE(val, x, 1, 1)
#define D4_DP_COMMON_MLCD_INT_SET(val, x) \
	SET_REGISTER_VALUE(val, x, 4, 1)
#define D4_DP_COMMON_MLCD_INT_CLR(val, x) \
	SET_REGISTER_VALUE(val, x, 5, 1)
#define D4_DP_COMMON_SLCD_INT_CLR(val, x) \
	SET_REGISTER_VALUE(val, x, 9, 1)
#define D4_DP_COMMON_PERIODIC_INT_CLR(val, x) \
	SET_REGISTER_VALUE(val, x, 13, 1)

/*          DP GLOBAL PERIOIDC COUNT          */
#define D4_DP_COMMON_PERIODIC_INT_CNT(val, x) \
	SET_REGISTER_VALUE(val, x, 0, 32)

/*          DP GLOBAL NLC CONTROL          */
#define D4_DP_COMMON_NLC_VID_ON(val, x) \
	SET_REGISTER_VALUE(val, x, 0, 1)
#define D4_DP_COMMON_NLC_GRP_ON(val, x) \
	SET_REGISTER_VALUE(val, x, 1, 1)
#define D4_DP_COMMON_NLC_MLCD_SEL(val, x) \
	SET_REGISTER_VALUE(val, x, 4, 1)
#define D4_DP_COMMON_NLC_SLCD_SEL(val, x) \
	SET_REGISTER_VALUE(val, x, 5, 1)

/*          DP GLOBAL NLC RDMA          */
#define D4_DP_COMMON_NLC_TILE_HEIGHT(val, x) \
	SET_REGISTER_VALUE(val, x, 0, 10)
#define D4_DP_COMMON_NLC_YUV_420(val, x) \
	SET_REGISTER_VALUE(val, x, 12, 1)
#define D4_DP_COMMON_NLC_TILE_NUM(val, x) \
	SET_REGISTER_VALUE(val, x, 16, 5)
#define D4_DP_COMMON_NLC_MAX_BUFFER(val, x) \
	SET_REGISTER_VALUE(val, x, 24, 8)

/*          DP GLOBAL NLC START ADDRESS          */
#define D4_DP_COMMON_NLC_ADDRESS(val, x) \
	SET_REGISTER_VALUE(val, x, 0, 32)

/*          DP GLOBAL NLC ADDRESS OFFSET           */
#define D4_DP_COMMON_NLC_ADDR_OFFSET(val, x) \
	SET_REGISTER_VALUE(val, x, 0, 24)

/*          DP GLOBAL NLC BUFFER CHECK           */
#define D4_DP_COMMON_NLC_SBLEV_TH(val, x) \
	SET_REGISTER_VALUE(val, x, 0, 8)
#define D4_DP_COMMON_NLC_BUFCHK_TH(val, x) \
	SET_REGISTER_VALUE(val, x, 8, 8)

/*          DP GLOBAL NLC FMT           */
#define D4_DP_COMMON_NLC_FMT(val, x) \
	SET_REGISTER_VALUE(val, x, 0, 4)
#define D4_DP_COMMON_NLC_DISPLAY_H_SIZE(val, x) \
	SET_REGISTER_VALUE(val, x, 16, 11)
#define D4_DP_COMMON_NLC_FMT_INTERLACE(val, x) \
	SET_REGISTER_VALUE(val, x, 26, 1)

/*          DP GLOBAL BURSTLENTH           */
#define D4_DP_COMMON_NLC_SW_RESET(val, x) \
	SET_REGISTER_VALUE(val, x, 0, 1)
#define D4_DP_COMMON_NLC_BURSTLEN(val, x) \
	SET_REGISTER_VALUE(val, x, 4, 4)
#define D4_DP_COMMON_NLC_ENDIAN(val, x) \
	SET_REGISTER_VALUE(val, x, 28, 1)

#endif

