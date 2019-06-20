/* linux/arch/arm/mach-drime4/include/mach/
 *
 * Copyright (c) 2010 Samsung Electronics
 *	sejong <sejong55@samsung.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifndef __D4_DP_H__
#define __D4_DP_H__

#define MAX_DP_WINDOW	8
#define DP_TV_BASE	0x100
#define DP_LCD_BASE	0x600

#define LCD_MODULE_NAME		"drime4_lcd"
#define SUBLCD_MODULE_NAME		"drime4_sublcd"
#define FB_MODULE_NAME		"d4_fb"
#define TV_MODULE_NAME		"drime4_tv"

/**
 * @enum	enum dppath
 * @brief  DP Display path로 Main LCD/ Sub LCD/ TV 로 구성되어진다.
 */
enum dppath {
	DP_SEL_MLCD, DP_SEL_SLCD, DP_SEL_TV
};

/**
 * @enum	DP Layer
 * @brief  DP Display Layer로 Video/Graphic로 구성되어진다.
 */
enum dp_layer {
	DP_VIDEO_LAYER, DP_GRP_LAYER
};

/**
 * @enum  dp_videoformat
 * @brief  Ycbcr420, Ycbcr422 로 구성되어진다.
 */
enum dp_videoformat {
	DP_YUV_420 = 0, DP_YUV_422,
};

/**
 * @enum enum dp_scantype
 * @brief DP TV Scan type
 */
enum dp_scantype {
	DP_SCAN_INTERLACE, DP_SCAN_PROGRESSIVE
};

/**
 * @enum enum lcd_datawidth
 * @brief  DP LCD Pannel Interface Bus Width <br>
 * LCD Pannel Bus width 8bit/24bit interface 지원
 */
enum lcd_datawidth {
	DATA_WIDTH_8, DATA_WIDTH_24
};

/**
 * @enum	lcd_type
 * @brief  DP LCD Pannel Interface type
 */
enum lcd_type {
	TYPE_DELTA, TYPE_STRIPE
};

/**
 * @enum  lcd_dis_seq
 * @brief  DP LCD Pannel Interface RGB Sequence
 */
enum lcd_dis_seq {
	SEQ_RGB, SEQ_GBR, SEQ_BGR, SEQ_RBG, SEQ_GRB, SEQ_BRG
};

/**
 * @enum	grp_scale
 * @brief  DP Graphic Display Scale,dp에서 간단한 그래픽 스케일기능 제공 오리지널 크기, double size 크기 제공
 */
enum grp_scale {
	GRP_SCL_X1, GRP_SCL_X2
};

/**
 * @enum	video_bit
 * @brief  DP Ycbcr video bit수
 */
enum video_bit {
	vid8bit, vid10bit
};

/**
 * @enum	dp_nlc
 * @brief  dp nlc decoder enable/disable
 */
enum dp_nlc {
	nlc_off, nlc_vid_on, nlc_grp_on
};

/**
 * @enum	dp_tv_mode
 * @brief	tv resolutions
 */
enum dp_tv_mode {
	ntsc,
	pal,
	res_480p,
	res_576p,
	res_720p_60,
	res_720p_50,
	res_1080i_60,
	res_1080i_50,
	res_1080p_60,
	res_1080p_50,
	res_1080p_30,
	res_1080p_24,
	res_720p_60_3d_fp,
	res_720p_60_3d_sbs,
	res_720p_60_3d_tb,
	res_720p_50_3d_fp,
	res_720p_50_3d_sbs,
	res_720p_50_3d_tb,
	res_1080i_60_3d_sbs,
	res_1080i_50_3d_sbs,
	res_1080p_60_3d_tb,
	res_1080p_50_3d_tb,
	res_1080p_30_3d_fp,
	res_1080p_30_3d_tb,
	res_1080p_24_3d_fp,
	res_1080p_24_3d_sbs,
	res_1080p_24_3d_tb,
	res_1080p_30_3d_sbs,
	res_max
};


/**
 * @enum	bb_color_table
 * @brief   Color pallette of boundary box.
 */
enum bb_color_table {
	table0, table1, table2, table3
};

/**
 * @enum	hdmi_pixel_mode
 * @brief   hdmi input data format
 */
enum hdmi_pixel_mode {
	pixel_rgb_444, pixel_ycbcr_422
};

/**
 * @enum	tv_10bit_mode
 * @brief   tv input/output modes
 */
enum tv_10bit_mode {
	input8_out8, input8_out10, input10_out10, input10_out8
};


/**
 * @enum	dp_window
 * @brief   display window number
 */
enum dp_window {
	win0, win1, win2, win3
};


/**
 * @enum	dp_tv_win
 * @brief   3D L/R  Window Selection
 */
enum dp_tv_win {
	l_window, r_window
};

/**
 * @enum	dp_onoff
 * @brief   on/off switch
 */
enum dp_onoff {
	dp_off, dp_on
};

/**
 * @enum	boundary_box_style
 * @brief   boundary box style selection
 */
enum boundary_box_style {
	style_0, style_1
};

/**
 * @enum	boundary_box
 * @brief   boundary box number
 */
enum boundary_box {
	bb_00,
	bb_01,
	bb_02,
	bb_03,
	bb_04,
	bb_05,
	bb_06,
	bb_07,
	bb_08,
	bb_09,
	bb_10,
	bb_11,
	bb_12,
	bb_13,
	bb_14,
	bb_15
};

/**
 * @struct	dp_rgb
 * @brief   RGB data format
 */
struct dp_rgb {
	unsigned short a;
	unsigned short r;
	unsigned short g;
	unsigned short b;
};

/**
 * @struct	graphics_layer_info
 * @brief   graphics layer settings
 */
struct graphics_layer_info {
	unsigned int stride;
	enum dp_scantype scan_type;
	unsigned short endian;
	struct dp_rgb background_color;
};

/**
 * @struct	ycbcr
 * @brief   YCbCr settings
 */
struct ycbcr {
	unsigned char y;
	unsigned char cb;
	unsigned char cr;
};


/**
 * @struct	image_address
 * @brief   input image address
 */
struct image_address {
	unsigned int y0;
	unsigned int c0;
	unsigned int y1;
	unsigned int c1;
};


/**
 * @struct	rgb_set
 * @brief   RGB setting
 */
struct rgb_set {
	unsigned short r_value;
	unsigned short g_value;
	unsigned short b_value;
};


/**
 * @struct	struct img_size
 * @brief  img_width: input image width size <br>
 * img_height: input image height size
 */
struct img_size {
	unsigned int img_width;
	unsigned int img_height;
};

/**
 * @struct	struct display_area
 * @brief  Display_H_Start: Display Pannel에서 image display horizontal start position <br>
 * Display_H_Size: Display Pannel에서 image display width <br>
 * Display_V_Start:Display Pannel에서 image display vertical start position <br>
 * Display_V_Size: Display Pannel에서 image display height<br>
 */
struct display_area {
	unsigned int Display_H_Start;
	unsigned int Display_H_Size;
	unsigned int Display_V_Start;
	unsigned int Display_V_Size;
};

struct video_layer_info_tv {
    unsigned int stride;
    enum dp_scantype scan_type;
    struct ycbcr background_color;
};

/**
 * @struct	struct lcd_tg
 * @brief  lcd pannel timimg information
 * total_h_size: horizontal total size <br>
 * total_v_size: vertical toatal size <br>
 * h_sync_rise: h sync rising position<br>
 * h_sync_fall: h sync falling position<br>
 * v_sync_rise: v sync rising position<br>
 * v_sync_fall: v sync falling position<br>
 * buf_read_h_start: buffer에서 Horizontal 신호를 읽는 시점 <br>
 * enable_h_start: horizontal enable start 신호, rising 되는 시점 <br>
 * enable_h_end: horizontal enable last 신호, falling 되는 시점 <br>
 * enable_v_start: vertical enable start 신호, rising 되는 시점 <br>
 * enable_v_end: vertical enable last 신호, falling 되는 시점 <br>
 * inv_dot_clk: dot clock의 inverse 설정 <br>
 * inv_enable_clk: enable clock inverse 설정 <br>
 * inv_h_sync: H sync inverse 설정<br>
 * inv_v_sync: V sync inverse 설정<br>
 */
struct lcd_tg {
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

/**
 * @struct	struct tv_tg
 * @brief  TV timimg information
 *
 */
struct tv_tg {
	unsigned short line_full_m1;
	unsigned short pixel_full_m1;
	unsigned short h_sync_act;
	unsigned short h_sync_blk;
	unsigned short v_sync_act0;
	unsigned short v_sync_blk0;
	unsigned short v_sync_act1;
	unsigned short v_sync_blk1;
	unsigned short f_sync_act0;
	unsigned short f_sync_blk0;
	unsigned short f_sync_act1;
	unsigned short pixel_start;
	unsigned short frame_finalize;
	unsigned short frame_init;
	unsigned short buf_read_start;
	unsigned short field_init;
	unsigned short pre_line_end;
	unsigned short pre_line_start;
	unsigned short de_pixel_end;
	unsigned short de_pixel_start;
	unsigned short active_line_end0;
	unsigned short active_line_start0;
	unsigned short active_line_end1;
	unsigned short active_line_start1;
	unsigned short field_act;
	unsigned short field_blk;
	unsigned short v_offset1;
	unsigned short v_offset0;
	unsigned short h_offset;
};

/**
 * @struct d4_lcd
 * @brief LCD Panel struct, lcd pannel관련 모든 정보 포함한다.<br>
 * lcd pannel 추가혹은 변경시 pannel관련 정보를 반드시 입력시켜야한다.<br>
 * h_size: LCD Pannel H size <br>
 * v_size: LCD Pannel V size <br>
 * freq: LCD Pannel frequency = 27M/(timming.total_h_size * timming.total_v_size)<br>
 * lcd_data_width: lcd panel bus width <br>
 * even_seq: lcd panel even line R/G/B sequence <br>
 * odd_seq: lcd panel odd line R/G/B sequence <br>
 * timming: lcd panel timming 정보<br>
 */
struct d4_lcd {
	int h_size;
	int v_size;
	unsigned int freq;
	enum lcd_datawidth lcd_data_width;
	enum lcd_type type;
	enum lcd_dis_seq even_seq;
	enum lcd_dis_seq odd_seq;
	struct lcd_tg timing;
};

/**
 * @struct drime4 tv struct
 * @brief TV structure
 *
 */
struct d4_tv {
	int h_size;
	int v_size;
	unsigned int freq;
	enum dp_tv_mode mode;
	enum hdmi_pixel_mode color_space;
	enum tv_10bit_mode inout;
	enum dp_videoformat format;
	struct video_layer_info_tv info;
	struct tv_tg timing;
};

/**
 * @struct drime4_fb_video_window
 * @brief drime4 framebuffer video 설정<br>
 */
struct drime4_fb_video_window {
	enum dp_videoformat format;
	struct img_size vid_image;
	struct display_area vid_display;
};

/**
 * @struct drime4_fb_graphic_window
 * @brief drime4 framebuffer graphic 설정<br>
 */
struct drime4_fb_graphic_window {
	struct img_size vid_image;
	struct display_area vid_display;
};

/**
 * @struct drime4_fb_layer
 * @brief drime4 framebuffer layer 정보설정<br>
 */
struct drime4_fb_layer {
	unsigned int video_stride;
	unsigned int graphic_stride;
	enum grp_scale graphic_scale;
	struct drime4_fb_video_window vid_win[MAX_DP_WINDOW];
	struct drime4_fb_graphic_window grp_win[MAX_DP_WINDOW];
};

/**
 * @struct drime4_tvfb_layer
 * @brief drime4 tv frame-buffer layer settings>
 */
struct drime4_tvfb_layer {
	unsigned int video_stride;
	unsigned int graphic_stride;
	enum dp_scantype scan_type;
	enum grp_scale graphic_scale;
	struct drime4_fb_video_window vid_win[MAX_DP_WINDOW];
	struct drime4_fb_graphic_window grp_win[MAX_DP_WINDOW];
};

/**
 * @struct drime4_platform_fb
 * @brief drime4 tv framebuffer platform resource
 */
struct drime4_platform_tvfb {
	unsigned char buffer_cnt;
	unsigned int vid_wins;
	unsigned int grp_wins;
	struct drime4_fb_layer layer;
};

/**
 * @struct drime4_platform_fb
 * @brief drime4 tv framebuffer platform resource
 */
struct drime4_platform_fb {
	unsigned char buffer_cnt;
	unsigned int vid_wins;
	unsigned int grp_wins;
	struct drime4_fb_layer layer;
};

/*
 * @struct drime4_tvfb_window
 * @brief  TV frame-buffer window information
 */
struct drime4_tvfb_window {
	int id;
	enum dp_videoformat yuv_format;
	enum dp_scantype scan_type;
	unsigned int video_stride;
	unsigned int graphic_stride;
	enum grp_scale graphic_scale;
	unsigned long vid_addr;
	unsigned long grp_addr;
	enum video_bit vid_bit;
	struct img_size vid_image;
	struct img_size grp_image;
	struct display_area vid_display;
	struct display_area grp_display;
	unsigned int pseudo_pal[16];
};

/*
 * @struct drime4_tvfb
 * @brief  TV frame-buffer information
 */
struct drime4_tvfb {
	void __iomem *regs_global;
	void __iomem *regs_tv;
	void __iomem *regs_lcd;

	struct mutex lock;
	struct device *dev;
	struct clk *dp_clk;
	struct clk *mlcd_clk;
	struct clk *mlcd_sel;
	struct clk *mlcd_sel1;
	struct clk *mlcd_sel2;
	struct clk *mlcd_out_clk;

	/* wait for vsync */
	wait_queue_head_t wq;

	unsigned int irq;
	struct fb_info **fb;
	unsigned int max_wins;
	struct d4_lcd *lcd;
	struct d4_lcd *sublcd;
	struct d4_tv *tv;
	void *platform_data;
};

/*
 *
 */
struct drime4_fb_window {
	int id;
	enum dp_videoformat yuv_format;
	unsigned int video_stride;
	unsigned int graphic_stride;
	enum grp_scale graphic_scale;
	unsigned long vid_addr;
	unsigned long grp_addr;
	enum video_bit vid_bit;
	struct img_size vid_image;
	struct img_size grp_image;
	struct display_area vid_display;
	struct display_area grp_display;
	unsigned int pseudo_pal[16];
};

/**
 * @struct drime4_fb
 * @brief drime4 fb 정보<br>
 */
struct drime4_fb {
	void __iomem *regs_global;
	void __iomem *regs_tv;
	void __iomem *regs_lcd;

	struct mutex lock;
	struct device *dev;
	struct clk *dp_clk;
	struct clk *mlcd_clk;/*< mlcd path :GCLKSEL5 bit 10*/
	struct clk *mlcd_sel;/*< mlcd path :GCLKSEL5 bit 11,12*/
	struct clk *mlcd_sel1;/*< mlcd clock :GCLKSEL5 bit 5~0*/
	struct clk *mlcd_sel2;
	struct clk *mlcd_out_clk;/*< slcd path :GCLKSEL5 bit 9*/
	struct clk *slcd_clk;/*< slcd path :GCLKSEL5 bit 23*/
	struct clk *slcd_sel;/*< slcd path :GCLKSEL5 bit 24,25*/
	struct clk *slcd_sel1;/*< slcd clock :GCLKSEL5 bit 18~13*/
	struct clk *slcd_out_clk;/*< slcd path :GCLKSEL5 bit 22*/

	/* wait for vsync */
	wait_queue_head_t wq;

	unsigned int irq;
	struct fb_info **fb;
	unsigned int max_wins;
	struct d4_lcd *lcd;
	struct d4_lcd *sublcd;
	void *platform_data;
};

#endif
