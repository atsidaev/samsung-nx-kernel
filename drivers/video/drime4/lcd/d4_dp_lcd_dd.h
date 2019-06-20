/**
 * @file d4_dp_lcd_ddl.h
 * @brief DRIMe4 DP LCD Device Driver change with a information LCD Sub Device Driver,
 * @author sejong oh<sejong55.oh@samsung.com>
 * 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifndef __D4_DP_LCD_DD_H__
#define __D4_DP_LCD_DD_H__

#include "d4_dp_lcd_if.h"

/**
 * @enum edp_lcd_datawidth
 * @brief dp lcd pannel data width ?�택
 */
enum edp_lcd_datawidth {
	DATA_8,/**< data size 8  */
	DATA_24,/**< data size 24  */
	DATA_FUJITU_8,
	DATA_FUJITU_24,
};

/**
 * @enum edp_lcd_type
 * @brief dp lcd pannel type
 */
enum edp_lcd_type {
	DELTA,/**< delta type */
	STRIPE
/**< stripe type  */
};

/**
 * @enum	elcd_dis_seq
 * @brief   dp lcd pannel display sequence
 */
enum elcd_dis_seq {
	RGB, GBR, BGR, RBG, GRB, BRG
};

/**
 * @enum elcd_path_onoff
 * @brief dp LCD Control Path ����
 */
enum elcd_path_onoff {
	LCD_CTRL_LCD_DMAC_ON,/**< LCD DMA On/Off */
	LCD_CTRL_VID_HRZFLT_ON,/**< LCD Video hrfilter On/Off */
	LCD_CTRL_GRP_MIX_ON,/**< LCD Graphic Mix On/Off */
	LCD_CTRL_RGB2YCBCR_ON,/**< LCD RGB2YCbCr CSC On/Off */
	LCD_CTRL_ZEBRA_ON,/**< LCD zebra On/Off */
	LCD_CTRL_DITHER_ON, /**< LCD dither On/Off */
	LCD_CTRL_GRP_DMAC_ON, /**< LCD Graphic DMA On/Off */
	LCD_CTRL_GRP_HRZFLT_ON,/**< LCD Graphic hrfilter On/Off */
	LCD_CTRL_OLED_GM_ON,/**< LCD OLED GM On/Off */
	LCD_CTRL_TCC_ON,/**< LCD TCC On/Off */
	LCD_CTRL_3D_VID,/**< LCD 3D On/Off */
	LCD_CTRL_SD_ON,/**< LCD SD On/Off */
	LCD_CTRL_YCBCR422,/**< LCD Ycbcr422 On/Off */
	LCD_CTRL_ROTATE_LR,/**< LCD Rotate LR On/Off */
	LCD_CTRL_VID_10BIT,/**< LCD Video 10bit On/Off */
	LCD_CTRL_TCC_VID,/**< LCD TCC Video Only On/Off */
	LCD_CTRL_VID_SCAN, LCD_CTRL_GRP_SCAN, LCD_CTRL_BBOX_2X,/**< LCD Boundary Box double scale On/Off */
	LCD_CTRL_LINE_INV, LCD_CTRL_VID_HSWAP,/**< LCD Video Horizontal flip On/Off */
	LCD_CTRL_VID_VSWAP,/**< LCD Video Vertical flip On/Off */
	LCD_CTRL_GRP_HSWAP,/**< LCD Graphc Horizontal flip On/Off */
	LCD_CTRL_GRP_VSWAP,
/**< LCD Graphic Vertical flip On/Off */
};

enum egrp_mix_onoff {
	GRP_MIX_ALP_BLD_ON,
	GRP_MIX_LIMIT_ON,
	GRP_MIX_VID_RANGE_DET,
	GRP_MIX_GRP_RANGE_DET,
	GRP_MIX_RANGE_TARGET,
	GRP_MIX_LAYER_SWAP,
	GRP_MIX_FLAG_R_ON,
	GRP_MIX_FLAG_G_ON,
	GRP_MIX_FLAG_B_ON,
};

enum elcd_csc_type {
	LCD_CSC_SD_STANDARD, LCD_CSC_SD_FULL, LCD_CSC_HD_STANDARD, LCD_CSC_HD_FULL,
};

enum emdma_select {
	MDMA_Y, MDMA_C, MDMA_G,
};

/**
 * @enum egrp_mix_channel_onoff
 * @brief dp graphic mix chanel control
 */
enum egrp_mix_channel_onoff {
	GRP_MIX_CH_R_ON,
	GRP_MIX_CH_G_ON,
	GRP_MIX_CH_B_ON,
	GRP_MIX_CH_R_INV_ON,
	GRP_MIX_CH_G_INV_ON,
	GRP_MIX_CH_B_INV_ON,
	GRP_MIX_CH_R_OFS_ON,
	GRP_MIX_CH_G_OFS_ON,
	GRP_MIX_CH_B_OFS_ON
};

enum egrp_alpha_ctrl {
	ALP_CTRL_ON, ALP_CTRL_INV, ALP_CTRL_OFS
};

enum egrp_ctrl_onoff {
	GRP_CTRL_ARGB_ORDER,
	GRP_CTRL_WIN0_FLAT_ALPHA,
	GRP_CTRL_WIN1_FLAT_ALPHA,
	GRP_CTRL_WIN2_FLAT_ALPHA,
	GRP_CTRL_WIN3_FLAT_ALPHA,
	GRP_CTRL_WIN0_ADDR_SWAP,
	GRP_CTRL_WIN1_ADDR_SWAP,
	GRP_CTRL_WIN2_ADDR_SWAP,
	GRP_CTRL_WIN3_ADDR_SWAP,
	GRP_CTRL_WIN0_ON,
	GRP_CTRL_WIN1_ON,
	GRP_CTRL_WIN2_ON,
	GRP_CTRL_WIN3_ON
};

enum elcd_out_type {
	LCD_OUT_DELTA_8BIT, LCD_OUT_STRIPE_24BIT,
	LCD_OUT_STRIPE_8BIT_LSB, /**< LSB 8 bit, 3Times Transmission */
	LCD_OUT_BT656_8BIT_CbYCrY,
	LCD_OUT_BT656_8BIT_CrYCbY,
	LCD_OUT_BT656_10BIT_CbYCrY,
	LCD_OUT_BT656_10BIT_CrYCbY,
	LCD_OUT_FUJITU_8BIT_CbYCrY,
	LCD_OUT_FUJITU_8BIT_CrYCbY,
	LCD_OUT_FUJITU_8BIT_YCbYCr,
	LCD_OUT_FUJITU_8BIT_YCrYCb,
	LCD_OUT_FUJITU_RGB_3TIMES,
	LCD_OUT_FUJITU_RGB_DUMMY,
	LCD_OUT_FUJITU_RGB565
};

enum elcd_gm_ctrl_mode {
	GM_FICT_SW, GM_BLOACK_C, GM_GAMMA_C, GM_CHROMA_C
};

enum elcd_gm_chroma_gain_mode {
	GM_CG_GB, GM_CG_RB, GM_CG_GR
};

enum elcd_gm_hue_threshold_mode {
	GM_HTH_GB, GM_HTH_RB, GM_HTH_GR
};

enum elcd_gm_fine {
	GM_BO_FINE, GM_CCG_FINE, GM_GAME_FINE
};

enum elcd_dither_type {
	TRUNCTION, ROUNDING
};

struct stdp_filter_set {
	int Tap0_Coef;
	int Tap1_Coef;
	int Tap2_Coef;
	int Tap3_Coef;
	int Tap4_Coef;
	int Post_Coef;
};

struct stdp_lcd_tg {
	unsigned short total_h_size;
	unsigned short total_v_size;
	unsigned short h_sync_rise;
	unsigned short h_sync_fall;
	unsigned short v_sync_rise;
	unsigned short v_sync_fall;
	unsigned short buf_read_h_start;
	unsigned short enable_h_start;
	unsigned short enable_h_end;
	unsigned short enable_v_start;
	unsigned short enable_v_end;
	unsigned short inv_dot_clk;
	unsigned short inv_enable_clk;
	unsigned short inv_h_sync;
	unsigned short inv_v_sync;
};

struct stfb_lcd_pannel {
	int h_size;
	int v_size;
	enum edp_lcd_datawidth lcd_data_width;
	enum edp_lcd_type type;
	enum elcd_dis_seq even_seq;
	enum elcd_dis_seq odd_seq;
	struct stdp_lcd_tg timing;
	unsigned int pixclock;
};

struct stfb_get_info {
	unsigned int dp_global;
	unsigned int dp_tv;
	unsigned int dp_lcd;
	struct stfb_video_info video;
	struct stfb_graphic_info graphic;
};

#ifdef CONFIG_PM
struct lcd_sleep_save {
	unsigned int reg;
	unsigned int val;
};
#endif

struct stfb_lcd_pannel_size {
	unsigned int h_size;
	unsigned int v_size;
};

void dp_lcd_pannel_set(void);
void dp_sublcd_pannel_set(void);
void drime4_lcd_display_init(void);
void drime4_sublcd_display_init(void);
void d4_dp_lcd_off(void);
void d4_dp_sublcd_off(void);
int d4_lcd_video_init_display(void);
int d4_sublcd_video_init_display(struct stfb_video_info *video);
void dp_lcd_set_info(struct stfb_get_info *getinfo);
void dp_lcd_get_info(struct stfb_get_info *getinfo);
void dp_lcd_set_vid_display_area(struct stfb_info *info, enum edp_window win,
		enum edp_video_bit vid_bit, struct stdp_display_area *display);
void dp_lcd_save_reg(void);
void dp_lcd_restore_reg(void);
void dp_lcd_check_test(struct stfb_info *info);
void d4_lcd_pannel_manager_set(struct stfb_lcd_pannel *pannel);
void d4_sublcd_pannel_manager_set(struct stfb_lcd_pannel *pannel);
void d4_dp_sublcd_interrupt_onoff(enum edp_onoff onoff);
int dp_mlcd_ISR_pending_clear(void);
int dp_slcd_ISR_pending_clear(void);

int d4_dp_lcd_video_stride(unsigned int stride);
void d4_dp_lcd_graphic_window_onoff(enum edp_window window, enum edp_onoff on_off);
void d4_dp_bt656_onoff(struct stlcd_bt656 val);
void dp_pmu_on_off(enum edp_onoff onoff);
void d4_dp_mlcd_clock_ctrl(unsigned int mlcdclock);
void d4_dp_slcd_clock_ctrl(unsigned int slcdclock);
void dp_bus_clock_change(unsigned long isHighRate);
void dp_lcd_mlcd_clock_onoff(int onoff);

void lcd_csc_manual_set(struct stlcd_csc_matrix *matrix);
#endif /* __D4_DP_LCD_DD_H__ */
