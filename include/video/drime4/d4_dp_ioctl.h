/**
 * @file	d4_dp_ioctl.h
 * @brief	dp driver header for Samsung DRIMe4 Camera Interface driver
 *
 * @author	sejong<sejong55.oh@samsung.com>,
 * Copyright (c) 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifndef __DRIME4_DP_IOCTL_CMD_H__
#define __DRIME4_DP_IOCTL_CMD_H__

#include "d4_dp_type.h"

#define DP_MAGIC	 't'

/**
 * @struct 	stfbnlcvideo
 * @brief   video NLC Control 구조체
 */
struct stfbnlcvideo {
	enum edp_path dp_path;/**< dp path select : tv/lcd*/
	enum edp_window win;/**< dp video window 0~3 선택 */
	enum edpnlc nlc;/**< dp video nlc on/off 선택 */
	enum edp_videoformat format;/**< dp input video image format(Ycbcr4:2:0, Ycbcr4:2:2)*/
	enum edp_address_set address_mode;/**< select address mode, Pyhsical or virtual*/
	unsigned int H_Start;/**< display horizontal start positon */
	unsigned int H_Size;/**< display horizontal width */
	unsigned int V_Start;/**< display vertical start positon */
	unsigned int V_Size;/**< display vertical width */
	unsigned int img_width;/**< input image pixel size */
	unsigned int img_height;/**< input image height */
	unsigned int y_address;/**< nlc input image y0 address */
	unsigned int c_address;/**< nlc input image c0 address */
};

/**
 * @struct 	stfbnlcgraphic
 * @brief   video NLC Control
 */
struct stfbnlcgraphic {
	enum edp_path dp_path;/**< dp path select : tv/lcd*/
	struct stnlc_graphic nlcset;/**< dp graphic nlc on/off */
	enum edpnlc onoff;/**<dp graphic nlc on/off */
};

/**
 * @struct 	stbb_table
 * @brief   select dp boundary box table and boudary box  color (RGB) set
 */
struct stbb_table {
	enum edp_bb_color_table table;/**< boundary box color table */
	struct stdp_rgb rgb_info; /**< boundary box color */
};

/**
 * @struct 	stdp_display_swap
 * @brief  dp display  flip (horizontal ,vertical  swap)
 */

struct stdp_display_swap {
	enum edp_layer layer; /**< dp layer select video/graphic*/
	enum eswap direct; /**< display mirroring direction*/
};

/**
 * @struct 	stdp_window_onoff
 * @brief  display window select, window on/off set
 */
struct stdp_window_onoff {
	enum edp_window win; /**< display window 0~3 */
	enum edp_onoff onoff; /**< display window on/off */
};

struct stdp_display_buf {
	enum edp_window win; /**< display window 0~3 */
	int buf_size; /**< buf_size */
};


/*DP NLC IOCTL */
#define DP_IOCTL_NLC_VIDEO	_IOW(DP_MAGIC, 0, struct stfbnlcvideo)/**< lcd Video NLC \image html dp_vid_nlc.JPG*/
#define DP_IOCTL_NLC_GRAPHIC	_IOW(DP_MAGIC, 1, struct stfbnlcgraphic)/**< lcd Graphic NLC \image html dp_grp_nlc.JPG */

/* DP LCD VIDEO IOCTL */
#define DP_IOCTL_LCD_VID_STRIDE	_IOW(DP_MAGIC, 2, unsigned char)/**< video stride 설정,  8의 배수로 입력가능하다. 범위는 0~16384 \image html dp_vid_stride.JPG */
#define DP_IOCTL_LCD_VID_BACKGROUND	_IOW(DP_MAGIC, 3, struct stdp_ycbcr)/**< video background 설정, YCbCr로 입력받는다.\image html dp_vid_background.JPG*/
#define DP_IOCTL_LCD_VID_DISPLAY	_IOW(DP_MAGIC, 4, struct stvideo_display_area)/**< video display 영역 설정<br> video 8bit경우 horizontal start/end는 2의 배수이어야 한다. <br> video 10bit경우 horizontal start/end는 16의 배수이어야 한다. <br>\image html dp_vid_display.JPG */
#define DP_IOCTL_LCD_VID_WINDOW_ONOFF	_IOW(DP_MAGIC, 5, struct stdp_window_onoff)/**< video window0 on/off설정 <br> \image html dp_window_onoff.JPG */
#define DP_IOCTL_LCD_VID_SET	_IOW(DP_MAGIC, 6, struct stvideodisplay)/**< video framebuffer control, 위 컨트롤로 dp display area, address, format, stride 등 모두 컨틀롤 할 수 있다.<br> video control은 위의 io control 추천한다.\image html dp_video_total1.JPG \image html dp_video_total2.JPG */
#define DP_IOCTL_LCD_VID_ADDRESS _IOW(DP_MAGIC, 7, struct stvideo_address)/**< lcd Video address 설정 */

/* DP LCD GRAPHIC IOCTL */
#define DP_IOCTL_LCD_GRP_STRIDE	_IOW(DP_MAGIC, 8, unsigned char)/**< graphic stride 설정 \image html dp_grp_stride.JPG */
#define DP_IOCTL_LCD_GRP_BACKGROUND	_IOW(DP_MAGIC, 9, struct stdp_argb)/**< graphic background 설정, alpha/R/G/B입력받는다. */
#define DP_IOCTL_LCD_GRP_DISPLAY	_IOW(DP_MAGIC, 10, struct stgraphic_display_area)/**< graphic display 영역 설정 <br> horizontal start/end는 2의 배수이어야 한다. <br> \image html dp_grp_display.JPG */
#define DP_IOCTL_LCD_GRP_WINDOW_ONOFF	_IOW(DP_MAGIC, 11, struct stdp_window_onoff)/**< graphic window0 on/off설정 <br> \image html dp_grp_window_onoff.JPG */
#define DP_IOCTL_LCD_GRP_SET	_IOW(DP_MAGIC, 12, struct stgrpdisplay)/**< graphic framebuffer control, 위 컨트롤로 dp display area, address,stride등 모두 컨틀롤할 수 있다.<br> graphic control은 위의 io control 추천한다.\image html dp_graphic_total.JPG */
#define DP_IOCTL_LCD_GRP_ADDRESS _IOW(DP_MAGIC, 13, struct stgrp_address)/**< lcd graphic address 설정 */
#define DP_IOCTL_LCD_GRP_SCALE	_IOW(DP_MAGIC, 14, enum egrp_scale)/**< graphic Scale 컨트롤 ioctl \image html dp_grp_scale.JPG */
#define DP_IOCTL_LCD_GRP_PRORITY	_IOW(DP_MAGIC, 15, struct stdp_grp_prority)/**< graphic window display 우선순위 설정<br> \image html dp_grp_prority.JPG*/

/*DP LCD Boundary Box  IOCTL */
#define DP_IOCTL_LCD_BB_TABLE	_IOW(DP_MAGIC, 17, struct stbb_table)/**< Boundary Box Table Set \image html dp_bb_table.JPG*/
#define DP_IOCTL_LCD_BB_INFO	_IOW(DP_MAGIC, 18, struct stbb_info)/**< Boundary Box Information \image html dp_bb_table_info.JPG */
#define DP_IOCTL_LCD_BB_ONOFF	_IOW(DP_MAGIC, 19, struct stbb_onoff)/**< Boundary Box Display area set, On/Off  \image html dp_bb_table_display.JPG*/

/*DP LCD Filter  IOCTL */
#define DP_IOCTL_LCD_FILTER	_IOW(DP_MAGIC, 20, struct stlcdfilter)/**< Horizontal Filter,6가지의 filter로 각 filter 따른 sharpness 달라짐 */

/*DP LCD Zebra  IOCTL */
#define DP_IOCTL_LCD_ZEBRA	_IOW(DP_MAGIC, 21, struct stzebra_set)/**< 영상 과다노출 정도를 lcd에서 표시  \image html dp_zebra.JPG */

/*DP LCD GM  IOCTL */
#define DP_IOCTL_LCD_GM	_IOW(DP_MAGIC, 22, struct stlcd_gm_lcd)/**< 실제영상과 OLED LCD출력영상의 색차이를 제거  */

/*DP LCD Graphic Window Alpha  IOCTL */
#define DP_IOCTL_LCD_GRP_ALPHA	_IOW(DP_MAGIC, 23, struct stlcd_graphic_alpha)/**< LCD Graphic  Window Alpha \image html dp_alpha.JPG*/

/*DP LCD Display Swap  IOCTL */
#define DP_IOCTL_LCD_DISPLAY_SWAP	_IOW(DP_MAGIC, 24, struct stdp_display_swap)/**< LCD Display Swap \image html dp_display_swap.JPG*/

/*DP LCD TCC  IOCTL */
#define DP_IOCTL_LCD_TCC	_IOW(DP_MAGIC, 25, struct stlcd_tcc)/**< LCD TCC */

/*DP LCD Graphic Limit  IOCTL */
#define DP_IOCTL_LCD_LIMIT	_IOW(DP_MAGIC, 26, struct stdp_rgb_range)/**< LCD Graphic Limit */

/*DP LCD Layer Change  IOCTL */
#define DP_IOCTL_LCD_LAYER_CHANGE	_IOW(DP_MAGIC, 27, enum edp_layer)/**< LCD Video/Graphic Layer chagne */

/* DP SUB LCD VIDEO IOCTL */
#define DP_IOCTL_SLCD_VID_STRIDE	_IOW(DP_MAGIC, 28, unsigned char)/**< video stride 설정,  8의 배수로 입력가능하다. 범위는 0~16384 \image html dp_vid_stride.JPG */
#define DP_IOCTL_SLCD_VID_BACKGROUND	_IOW(DP_MAGIC, 29, struct stdp_ycbcr)/**< video background 설정, YCbCr로 입력받는다.\image html dp_vid_background.JPG*/
#define DP_IOCTL_SLCD_VID_DISPLAY	_IOW(DP_MAGIC, 30, struct stvideo_display_area)/**< video display 영역 설정<br> video 8bit경우 horizontal start/end는 2의 배수이어야 한다. <br> video 10bit경우 horizontal start/end는 16의 배수이어야 한다. <br>\image html dp_vid_display.JPG */
#define DP_IOCTL_SLCD_VID_WINDOW_ONOFF	_IOW(DP_MAGIC, 31, struct stdp_window_onoff)/**< video window0 on/off설정 <br> \image html dp_window_onoff.JPG */
#define DP_IOCTL_SLCD_VID_SET	_IOW(DP_MAGIC, 32, struct stvideodisplay) /**< SLCD Video 설정  */
#define DP_IOCTL_SLCD_VID_ADDRESS _IOW(DP_MAGIC, 33, struct stvideo_address)/**< lcd Video address 설정 */

/* DP SUB LCD GRAPHIC IOCTL */
#define DP_IOCTL_SLCD_GRP_STRIDE	_IOW(DP_MAGIC, 34, unsigned char)/**< graphic stride 설정 \image html dp_grp_stride.JPG */
#define DP_IOCTL_SLCD_GRP_BACKGROUND	_IOW(DP_MAGIC, 35, struct stdp_argb)/**< graphic background 설정, alpha/R/G/B입력받는다. */
#define DP_IOCTL_SLCD_GRP_DISPLAY	_IOW(DP_MAGIC, 36, struct stgraphic_display_area)/**< graphic display 영역 설정 <br> horizontal start/end는 2의 배수이어야 한다. <br> \image html dp_grp_display.JPG */
#define DP_IOCTL_SLCD_GRP_WINDOW_ONOFF	_IOW(DP_MAGIC, 37, struct stdp_window_onoff)/**< graphic window0 on/off설정 <br> \image html dp_grp_window_onoff.JPG */
#define DP_IOCTL_SLCD_GRP_SET	_IOW(DP_MAGIC, 38, struct stgrpdisplay)/**< graphic framebuffer control, 위 컨트롤로 dp display area, address,stride등 모두 컨틀롤할 수 있다.<br> graphic control은 위의 io control 추천한다.\image html dp_graphic_total.JPG */
#define DP_IOCTL_SLCD_GRP_ADDRESS _IOW(DP_MAGIC, 39, struct stgrp_address)/**< lcd graphic address 설정 */
#define DP_IOCTL_SLCD_GRP_SCALE	_IOW(DP_MAGIC, 40, enum egrp_scale)/**< graphic Scale 컨트롤 ioctl \image html dp_grp_scale.JPG */
#define DP_IOCTL_SLCD_GRP_PRORITY	_IOW(DP_MAGIC, 41, struct stdp_grp_prority)/**< graphic window display 우선순위 설정<br> \image html dp_grp_prority.JPG*/

/*DP SUB LCD Boundary Box  IOCTL */
#define DP_IOCTL_SLCD_BB_TABLE	_IOW(DP_MAGIC, 42, struct stbb_table)/**< Boundary Box Table Set \image html dp_bb_table.JPG*/
#define DP_IOCTL_SLCD_BB_INFO	_IOW(DP_MAGIC, 43, struct stbb_info)/**< Boundary Box Information \image html dp_bb_table_info.JPG */
#define DP_IOCTL_SLCD_BB_ONOFF	_IOW(DP_MAGIC, 44, struct stbb_onoff)/**< Boundary Box Display area set, On/Off  \image html dp_bb_table_display.JPG*/

/*DP SUB LCD Filter  IOCTL */
#define DP_IOCTL_SLCD_FILTER	_IOW(DP_MAGIC, 45, struct stlcdfilter)/**< Horizontal Filter,6가지의 filter로 각 filter 따른 sharpness 달라짐 */

/*DP SUB LCD Zebra  IOCTL */
#define DP_IOCTL_SLCD_ZEBRA	_IOW(DP_MAGIC, 46, struct stzebra_set)/**< 영상 과다노출 정도를 lcd에서 표시  \image html dp_zebra.JPG */

/*DP SUB LCD Graphic Window Alpha  IOCTL */
#define DP_IOCTL_SLCD_GRP_ALPHA	_IOW(DP_MAGIC, 47, struct stlcd_graphic_alpha)/**< LCD Graphic  Window Alpha \image html dp_alpha.JPG*/

/*DP SUB LCD Display Swap  IOCTL */
#define DP_IOCTL_SLCD_DISPLAY_SWAP	_IOW(DP_MAGIC, 48, struct stdp_display_swap)/**< LCD Display Swap \image html dp_display_swap.JPG*/

/*DP SUB LCD TCC  IOCTL */
#define DP_IOCTL_SLCD_TCC	_IOW(DP_MAGIC, 49, struct stlcd_tcc)/**< LCD TCC */

/*DP SUB LCD Graphic Limit  IOCTL */
#define DP_IOCTL_SLCD_LIMIT	_IOW(DP_MAGIC, 50, struct stdp_rgb_range)/**< LCD Graphic Limit */

/*DP SUB LCD Layer Change  IOCTL */
#define DP_IOCTL_SLCD_LAYER_CHANGE	_IOW(DP_MAGIC, 51, enum edp_layer)/**< LCD Video/Graphic Layer chagne */
#define DP_IOCTL_BT656_SET	_IOW(DP_MAGIC, 52, struct stlcd_bt656) /**< BT656 */
#define DP_IOCTL_LCD_GRP_ORDER_SET  _IOW(DP_MAGIC, 80, enum edp_onoff)/**< LCD Graphic Order change*/
#define DP_IOCTL_LCD_COLOR_ADJUST  _IOW(DP_MAGIC, 99, struct stlcd_csc_matrix)
/****************************** TV IOCTL Set ****************************************/
/* DP TV VIDEO IOCTL */
#define DP_IOCTL_TV_VID_STRIDE	_IOW(DP_MAGIC, 53, unsigned char)/**< video stride 설정,  8의 배수로 입력가능하다. 범위는 0~16384  */
#define DP_IOCTL_TV_VID_BACKGROUND	_IOW(DP_MAGIC, 54, struct stdp_ycbcr)/**< video background 설정, YCbCr로 입력받는다.*/
#define DP_IOCTL_TV_VID_DISPLAY	_IOW(DP_MAGIC, 55, struct stvideo_display_area)/**< video display 영역 설정<br> \image html display_area.JPG  */
#define DP_IOCTL_TV_VID_WINDOW_ONOFF	_IOW(DP_MAGIC, 56, struct stdp_window_onoff)/**< video window0 on/off설정 <br> [참고] frame buffer 구조에서 Video window별 framebuffer이다.    */
#define DP_IOCTL_TV_VID_SET	_IOW(DP_MAGIC, 57, struct stvideo_on)/**< set the video settings, it turns on tv out*/
#define DP_IOCTL_TV_VID_ADDRESS _IOW(DP_MAGIC, 58, struct stvideo_address)/**< TV video address */

/*DP TV Boundary Box  IOCTL */
#define DP_IOCTL_TV_BB_TABLE   _IOW(DP_MAGIC, 59, struct stbb_table)/**< Boundary Box Table Set \image html dp_bb_table.JPG*/
#define DP_IOCTL_TV_BB_INFO    _IOW(DP_MAGIC, 60, struct stbb_info)/**< Boundary Box Information \image html dp_bb_table_info.JPG */
#define DP_IOCTL_TV_BB_ONOFF   _IOW(DP_MAGIC, 61, struct stbb_onoff)/**< Boundary Box Display area set, On/Off  \image html dp_bb_table_display.JPG*/

/*DP TV Layer Change  IOCTL */
#define DP_IOCTL_TV_LAYER_CHANGE	_IOW(DP_MAGIC, 62, enum edp_layer)/**< TV Video/Graphic Layer change */

/* DP TV GRAPHIC IOCTL */
#define DP_IOCTL_TV_GRP_STRIDE  _IOW(DP_MAGIC, 63, unsigned char)	/**< graphic stride setting */
#define DP_IOCTL_TV_GRP_BACKGROUND      _IOW(DP_MAGIC, 64, struct stdp_argb)	/**< graphic background 설정, RGB입력받는다. */
#define DP_IOCTL_TV_GRP_DISPLAY _IOW(DP_MAGIC, 65, struct stgraphic_display_area)	/**< graphic display 영역 설정 <br> \image html display_area.JPG  */
#define DP_IOCTL_TV_GRP_WINDOW_ONOFF    _IOW(DP_MAGIC, 66, enum edp_onoff)	/**< graphic window0 on/off 설정 <br> [참고] frame buffer 구조에서 graphic window별 frame buffer이다.    */
#define DP_IOCTL_TV_GRP_SET     _IOW(DP_MAGIC, 67, struct stgrpdisplay)	/**< graphic frame buffer control, 위 컨트롤로 dp display area, address, stride등 모두 컨틀롤할
//				수 있다.<br> graphic control은 위의 io control 추천한다.  */
#define DP_IOCTL_TV_GRP_SCALE   _IOW(DP_MAGIC, 68, enum egrp_scale)/**< graphic Scale 컨트롤 ioctl*/
#define DP_IOCTL_TV_GRP_PRIORITY _IOW(DP_MAGIC, 69, struct stdp_grp_prority)/**< graphic window display 우선순위 설정<br> \image html grp_prority.JPG*/

/*DP TV Mode Change  IOCTL */
#define DP_IOCTL_TV_MODE                _IOW(DP_MAGIC, 70, enum edp_tv_mode) /**set TV modes  */

/*DP TV Graphic Window Alpha value setting IOCTL */
#define DP_IOCTL_TV_GRP_ALPHA	_IOW(DP_MAGIC, 71, struct sttv_graphic_alpha)/**< TV Graphic  Window Alpha */

#define DP_IOCTL_TV_GRP_ORDER_SET  _IOW(DP_MAGIC, 72, enum edp_onoff)/**< TV Graphic Order change*/

#define DP_IOCTL_LCD_TURN_ON  _IOW(DP_MAGIC, 73, enum edp_onoff)/**< for lcd turn on stand alone */
#define DP_IOCTL_TV_TURN_ON  _IOW(DP_MAGIC, 74, enum edp_onoff)/**< for lcd turn on stand alone */

#define DP_IOCTL_LCD_FRAME_INTERRUPT_ON  _IOW(DP_MAGIC, 75, enum edp_window) /**< for lcd frame interrupt onoff */
#define DP_IOCTL_LCD_WAIT_FRAME_INTERRUPT  _IO(DP_MAGIC, 76) /**< for wait lcd frame interrupt onoff */
#define DP_IOCTL_LCD_SET_RING_BUF_CNT  _IOW(DP_MAGIC, 77, struct stdp_display_buf) /**< for set lcd ring buf cnt set */
#define DP_IOCTL_TV_SET_3DVIDEO	_IOW(DP_MAGIC, 78, struct stvideo_on_3d)/**< set the 3D video settings */
#define DP_IOCTL_SET_DP_CLOCK_HIGH  _IOW(DP_MAGIC, 79, unsigned long) /**< for set dp bus clock rate */
#define DP_IOCTL_LCD_FRAME_INTERRUPT_OFF  _IOW(DP_MAGIC, 81, enum edp_window) /**< for lcd frame interrupt onoff */
#define DP_IOCTL_LCD_CURRENT_BUF_CNT  _IOWR(DP_MAGIC, 82, struct stdp_display_buf) /**< return lcd ring buffer cnt */

#define DP_IOCTL_TV_FRAME_INTERRUPT_ON  _IOW(DP_MAGIC, 83, enum edp_window) /**< for tv frame interrupt on */
#define DP_IOCTL_TV_WAIT_FRAME_INTERRUPT  _IO(DP_MAGIC, 84) /**< for wait tv frame interrupt onoff */
#define DP_IOCTL_TV_SET_RING_BUF_CNT  _IOW(DP_MAGIC, 85, struct stdp_display_buf) /**< for set tv ring buf cnt set */
#define DP_IOCTL_TV_FRAME_INTERRUPT_OFF  _IOW(DP_MAGIC, 86, enum edp_window) /**< for tv frame interrupt off */
#define DP_IOCTL_TV_CURRENT_BUF_CNT  _IOWR(DP_MAGIC, 87, struct stdp_display_buf) /**< return tv ring buffer cnt */
#define DP_IOCTL_TV_FRAME_RATE  _IOR(DP_MAGIC, 88, unsigned long) /**< return tv ring buffer cnt */
#define DP_IOCTL_TV_3D_VID_ADDRESS _IOW(DP_MAGIC, 89, struct stvideo_address)/**< TV 3d video address */

#define DP_IOCTL_LCD_FW_MEM_SET	_IOW(DP_MAGIC, 90, int)
#define DP_IOCTL_LCD_FW_MEM_GET	_IOR(DP_MAGIC, 91, int)

#define DP_IOCTL_TV_VID_HRZ_FILTER_SET  _IOW(DP_MAGIC, 92, enum edp_filter) /**< tv video layer horizontal filter set */
#define DP_IOCTL_TV_GRP_HRZ_FILTER_SET 	_IOW(DP_MAGIC, 93, enum edp_filter)/**< tv graphic layer horizontal filter set */
#define DP_IOCTL_SET_LCD_ON_OFF 	_IOW(DP_MAGIC, 94, int)/**< lcd on off */
#endif   /* __DRIME4_DP_IOCTL_CMD_H__ */
