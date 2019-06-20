/**
 * @file d4_dp_type.h
 * @brief DRIMe4 DP Type define file
 * @author sejong oh<sejong55.oh@samsung.com>
 * 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifndef __DP_TYPE_H_
#define __DP_TYPE_H_

/*
 #define DP_DEBUG
 */
#ifdef DP_DEBUG
#define DpPRINTK(args...)    printk(args)
#else
#define DpPRINTK(args...)
#endif

/**
 * @enum edp_path
 * @brief dp path select:
 */
enum edp_path {
	DP_MLCD,/**< DP Path  Main LCD */
	DP_SLCD,/**< DP Path  Sub LCD */
	DP_TV
/**< DP Path  TV */
};

/**
 * @enum edp_videoformat
 * @brief Ycbcr format : 420, 422
 */
enum edp_videoformat {
	Ycbcr_420,/**< Ycbcr420 */
	Ycbcr_422,
/**< Ycbcr422 */
};

/**
 * @enum	edp_layer
 * @brief   DP에는 2개의 layer가 존재한다. video/graphic layer
 */
enum edp_layer {
	DP_VIDEO,/**< Video layer */
	DP_GRP
/**< Graphic layer*/
};

/**
 * @enum	edp_onoff
 * @brief   on/off
 */
enum edp_onoff {
	DP_OFF,/**< On */
	DP_ON
/**< Off */
};

enum einterrupt_type {
	VID_WIN_0_INTERRUPT,
	VID_WIN_1_INTERRUPT,
	VID_WIN_2_INTERRUPT,
	VID_WIN_3_INTERRUPT,
	DRM_INTERRUPT,
	BUSCLOCK_CHANGE_INTERRUPT,
	MAX_INTERRUPT
};

/**
 * @enum	edp_window
 * @brief   DP layer에는 4개의 window가 사용가능.
 */
enum edp_window {
	DP_WIN0,/**< window0 */
	DP_WIN1,/**< window1 */
	DP_WIN2,/**< window2 */
	DP_WIN3,/**< window3 */
	MAX_DP_WIN
};

/**
 * @enum	tv_cscmode
 * @brief   tv color space conversion modes
 */
enum tv_cscmode {
	HD_Std_Range,/**< Standard range */
	xvYcc_Range/**< xvYcc range */
};

/**
 * @enum	edp_address
 * @brief   dp address type
 */
enum edp_address_set {
	DP_VIRTUAL_SET,/**< Virtual address set */
	DP_PHYSICAL_SET,
/**< Physical address set */
};

/**
 * @enum	edp_input_img_type
 * @brief   DP에 입력되는 이미지타입
 */
enum edp_input_img_type {
	DP_INTERLACE,/**< interlace : y0,y1,c0,c1*/
	DP_PROGRESSIVE
/**< progressive:y,c */
};

/**
 * @enum	edp_isr_interval
 * @brief   DP 인터럽터 발생주기
 */
enum edp_isr_interval {
	DP_FRAME_INTERVAL,/**< frame 단위 인터럽터 */
	DP_FIELD_INTREVAL
/**< field 단위 인터럽터 */
};

/**
 * @enum	edp_video_bit
 * @brief   video bit 수
 */
enum edp_video_bit {
	_8bit,/**< 8bit  */
	_16bit
/**< 10bit  */
};

enum edp_ycbcr_order {
	CB_Y_CR_Y,
	CR_Y_CB_Y,
};

/**
 * @enum	egrp_scale
 * @brief   dp graphic scale 기능으로 original size, double size 조정가능<br>
 *          dp video scale 기능은 없음
 */
enum egrp_scale {
	SCL_X1,/**< original size  */
	SCL_X2
/**< double size  */
};

/**
 * @enum	edp_bb_box
 * @brief   dp boundary box: face detect용 사각형 이미지를 그릴 수 있다.<br>
 *          boundary box는 16개를 그릴 수 있다.
 */
enum edp_bb_box {
	DP_BB_00,/**< boundary box window0  */
	DP_BB_01,/**< boundary box window1  */
	DP_BB_02,/**< boundary box window2  */
	DP_BB_03,/**< boundary box window3  */
	DP_BB_04,/**< boundary box window4  */
	DP_BB_05,/**< boundary box window5  */
	DP_BB_06,/**< boundary box window6  */
	DP_BB_07,/**< boundary box window7  */
	DP_BB_08,/**< boundary box window8  */
	DP_BB_09,/**< boundary box window9  */
	DP_BB_10,/**< boundary box window10  */
	DP_BB_11,/**< boundary box window11  */
	DP_BB_12,/**< boundary box window12  */
	DP_BB_13,/**< boundary box window13  */
	DP_BB_14,/**< boundary box window14  */
	DP_BB_15
/**< boundary box window15  */
};

/**
 * @enum	edp_bb_style_set
 * @brief  boundary box style
 *            0= ,  1=
 */
enum edp_bb_style_set {
	DP_BB_STYLE0,/**< Boundary Box Style0 */
	DP_BB_STYLE1,
/**< Boundary Box Style1 */
};

/**
 * @enum	edp_filter
 * @brief   dp filter, 이미지 경계 line 뚜렷하게만드는 정도를 변화시킨다.<br>
 *          LPF(lowpass filter)-> HPF(Highpass filter) HPF값이 높을 수록 경계값이 뚜렷해진다.
 */
enum edp_filter {
	DP_LPF1,/**< lowpass filter 1 */
	DP_LPF2,/**< lowpass filter 2 */
	DP_LPF3, /**< lowpass filter 3 */
	DP_HPF1, /**< heighpass filter 1 */
	DP_HPF2, /**< heighpass filter 2 */
	DP_HPF3, /**< heighpass filter 3 */
	DP_BYPASS
/**< bypass filter */
};

/**
 * @enum	edp_bb_color_table
 * @brief   boundary box의 Color pallete로서 4개의 pallete에 각각의 color값을 입력할 수 있다.
 */
enum edp_bb_color_table {
	DP_BB_COLOR_TABLE_0,/**< boundary box color table 0  */
	DP_BB_COLOR_TABLE_1,/**< boundary box color table 1  */
	DP_BB_COLOR_TABLE_2,/**< boundary box color table 2  */
	DP_BB_COLOR_TABLE_3
/**< boundary box color table 3  */
};

/**
 * @enum edp_interrupt
 * @brief dp interrupt
 */
enum edp_interrupt {
	INT_TV,/**< INT_TV: TV Interrupt  */
	INT_MLCD,/**<INT_MLCD: Main LCD Interrupt   */
	INT_SLCD,/**< INT_SLCD: Sub LCD Interrupt  */
	INT_PERIODIC_ON,/**< INT_PERIODIC_ON: Periodic Interrupt  */
	INT_PERIODIC_RST,
/**< INT_PERIODIC_RST: Periodic Interrupt Reset   */
};

/**
 * @enum ezebra_width
 * @brief zebra pattern의 폭
 */
enum ezebra_width {
	DP_ZEBRA_WIDTH1 = 1,/**< zebra width 1 */
	DP_ZEBRA_WIDTH2 = 2,/**< zebra width 2 */
	DP_ZEBRA_WIDTH3 = 3
/**< zebra width 3 */
};

/**
 * @enum ezebra_speed
 * @brief zebra pattern 변화 속도
 */
enum ezebra_speed {
	DP_ZEBRA_SPEED_STOP = 0,/**< zebra speed stop */
	DP_ZEBRA_L_SPEED1 = 1,/**< zebra Left speed 1 */
	DP_ZEBRA_L_SPEED2 = 2,/**< zebra Left speed 2 */
	DP_ZEBRA_L_SPEED3 = 3,/**< zebra Left speed 3 */
	DP_ZEBRA_R_SPEED1 = 9,/**< zebra Right speed 1 */
	DP_ZEBRA_R_SPEED2 = 10,/**< zebra Right speed 2 */
	DP_ZEBRA_R_SPEED3 = 11
/**< zebra Right speed 3 */
};

/**
 * @enum ezebra_angle
 * @brief zebra pattern 변화 각도
 */
enum ezebra_angle {
	DP_ZEBRA_ANGLE0 = 0,/**< zebra angle 0 */
	DP_ZEBRA_ANGLE_CCW1 = 1,/**< zebra angle ccw1 */
	DP_ZEBRA_ANGLE_CCW2 = 2,/**< zebra angle ccw2 */
	DP_ZEBRA_ANGLE_CCW3 = 3,/**< zebra angle ccw3 */
	DP_ZEBRA_ANGLE_CCW4 = 4,/**< zebra angle ccw4 */
	DP_ZEBRA_ANGLE_CCW5 = 5,/**< zebra angle ccw5 */
	DP_ZEBRA_ANGLE_CCW6 = 6,/**< zebra angle ccw6 */
	DP_ZEBRA_ANGLE_CCW7 = 7,/**< zebra angle ccw7 */
	DP_ZEBRA_ANGLE_CW1 = 9,/**< zebra angle cw1 */
	DP_ZEBRA_ANGLE_CW2 = 10,/**< zebra angle cw2 */
	DP_ZEBRA_ANGLE_CW3 = 11,/**< zebra angle cw3 */
	DP_ZEBRA_ANGLE_CW4 = 12,/**< zebra angle cw4 */
	DP_ZEBRA_ANGLE_CW5 = 13,/**< zebra angle cw5 */
	DP_ZEBRA_ANGLE_CW6 = 14,/**< zebra angle cw6 */
	DP_ZEBRA_ANGLE_CW7 = 15
/**< zebra angle cw17*/
};

/**
 * @enum eswap
 * @brief dp display flip
 */
enum eswap {
	DP_None_Change,/**< dp diplay 방향 변경 없음  */
	DP_H_SWAP, /**< horizontal 방향 변경   */
	DP_V_SWAP,
	DP_H_V_SWAP
/**< vertical 방향 변경 */
};

/**
 * @enum ehrz_filter_type
 * @brief filter type변경
 */
enum ehrz_filter_type {
	HRZ_FLT_TYPE_VIDEO,/**< filter video 선택 */
	HRZ_FLT_TYPE_GRP,
/**< filter graphic 선택 */
};

/**
 * @enum edpnlc
 * @brief dp NLC Control
 */
enum edpnlc {
	NLC_OFF,/**< DP NLC Off */
	NLC_VID_ON,/**< DP Video NLC On */
	NLC_GRP_ON,
/**< DP Graphic NLC On */
};

/**
 * @enum edp_tv_mode
 * @brief tv output modes
 *
 * @note:
 *	 NTSC        :SD Analog NTSC
 *	 PAL         :SD Analog PAL
 *   RES_480P    :SD HDMI 480P  60hz
 *   RES_576P    :SD HDMI 576P  50hz
 *   RES_720P_60 :HD HDMI 720P  60hz
 *   RES_720P_50 :HD HDMI 720P  50hz
 *   RES_1080I_60:HD HDMI 1080I  60hz
 *   RES_1080I_50:HD HDMI 1080I  50hz
 */
enum edp_tv_mode {
	NTSC,/**<SD Analog NTSC*/
	PAL,/**<SD Analog PAL */
	RES_480P,/**< SD HDMI 480P  60hz */
	RES_576P,/**< SD HDMI 576P  50hz */
	RES_720P_60,/**< HD HDMI 720P  60hz */
	RES_720P_50,/**< HD HDMI 720P  50hz */
	RES_1080I_60,/**< HD HDMI 1080I  60hz */
	RES_1080I_50,/**< HD HDMI 1080I  50hz */
	RES_1080P_60,/**< HD HDMI 1080P  60hz */
	RES_1080P_50,/**< HD HDMI 1080P  50hz*/
	RES_1080P_30,/**< HD HDMI 1080P  30hz*/
	RES_1080P_24,/**< HD HDMI 1080P  24hz*/
	RES_720P_60_3D_FP,/**< 3D HDMI 720P  FP 60hz */
	RES_720P_60_3D_SBS,/**< 3D HDMI 720P  side by side 60hz */
	RES_720P_60_3D_TB,/**< 3D HDMI 720P  top-bottom 60hz */
	RES_720P_50_3D_FP,/**< 3D HDMI 720P  FP 50hz */
	RES_720P_50_3D_SBS,/**< 3D HDMI 720P  side-by-side 50hz */
	RES_720P_50_3D_TB,/**< 3D HDMI 720P  top-bottom 50hz */
	RES_1080I_60_3D_SBS,/**< 3D side-by-side HDMI 1080I  60hz */
	RES_1080I_50_3D_SBS,/**< 3D side-by-side HDMI 1080I  50hz */
	RES_1080P_60_3D_TB,/**< 3D top-bottom HDMI 1080P  60hz */
	RES_1080P_50_3D_TB,/**< 3D top-bottom HDMI 1080P  50hz */
	RES_1080P_30_3D_FP,/**< 3D FP HDMI 1080P  30hz */
	RES_1080P_30_3D_TB,/**< 3D top-bottom HDMI 1080P  30hz */
	RES_1080P_24_3D_FP,/**< 3D FP HDMI 1080P  24hz */
	RES_1080P_24_3D_SBS,/**< 3D side-by-side HDMI 1080P 24hz */
	RES_1080P_24_3D_TB,/**< 3D top-bottom HDMI 1080P 24hz */
	RES_1080P_30_3D_SBS,/**< 3D side-by-side HDMI 1080P 30hz */	
	RES_MAX
/**< END marker */
};

/**
 * @enum	edp_hdmi_pixel_mode
 * @brief   hdmi input video data format
 */
enum edp_hdmi_pixel_mode {
	rgb_444, ycbcr_422
};

/**
 * @enum	edp_tv_10bit_mode
 * @brief   tv input/output modes
 */
enum edp_tv_10bit_mode {
	in8_out8, in8_out10, in10_out10, in10_out8
};

/**
 * @enum scan_type
 * @brief 3D format
 *
 * @note:
 *		Frame Packing Progressive
 *		Frame Packing Interlace
 *		Side by Side
 *		Top and Bottom
 *		none
 */
enum scan_type {
	tb,	/**< Top and Bottom */
	fp_progressive,	/**< Frame Packing Progressive */
	ssh,	/**< Side by Side */
	fp_interlace,	/**< Frame Packing Interlace */
	none	/**< none */
};

/**
 * @enum tv_win
 * @brief 3D L/R  Window Selection
 *
 */
enum tv_win {
	left_window,/**< left window */
	right_window
/**< right window */
};

/**
 * @enum edp_tv_clock
 * @brief tv clocks
 * @note: the clocks are
 *			-TV Path core clock
 *			-TV Path system clock
 *			-HDMI core clock
 *			-HDMI system clock
 *			-HDMI Pixel clock
 *			-TMDS clock for HDMI Link
 *			-HDMI reference clock
 */
enum edp_tv_clock {
	tv_clock,/**< TV Path core clock */
	tv_sys_clock,/**< TV Path system clock */
	hdmi_clock,/**< HDMI core clock */
	hdmi_sys_clock,/**< HDMI system clock */
	hdmi_pixel_clock,/**< HDMI Pixel clock */
	hdmi_tmds_clock,/**< TMDS clock for HDMI Link */
	hdmi_reference_clock/**< HDMI reference clock */
};

/**
 * @enum	edp_hdmi_clock
 * @brief   hdmi phy clock frequencies
 */
enum edp_hdmi_clock {
	clock27,/**< 27MHz */
	clock74_25,/**< 74.25MHz */
	clock74_176,/**< 74.176MHz */
	clock148_5,/**< 148.5MHz */
	clock148_352
/**< 148.352MHz */
};

/**
 * @struct	stdp_tv_modes
 * @brief   To set various video related formats
 */
struct stdp_tv_modes {
	enum edp_tv_mode tvmode;/**< TV Resolutions */
	enum edp_hdmi_pixel_mode hdmimode;/**< HDMI Input video format*/
	enum edp_tv_10bit_mode inoutmode;/**< TV input/output bit format*/
};

/**
 * @struct	edp_display_size
 * @brief   display size settings
 */
struct edp_display_size {
	unsigned int h_str;
	unsigned int h_end;
	unsigned int v_str;
	unsigned int v_en;
};

/**
 * @enum	edp_lcd_gm
 * @brief   lcd gm mode select
 */
enum edp_lcd_gm {
	GM_STANDARD,/**< GM Standard */
	GM_FICTIVE
/**< GM Fictive */
};

/**
 * @struct stdp_rgb
 * @brief graphic R/G/B 설정 구조체, boudndary box color table 색 설정시 사용
 *
 */
struct stdp_rgb {
	unsigned short DP_R;/**< Red color */
	unsigned short DP_G;/**< Green color */
	unsigned short DP_B;/**< Blue color */
};

/**
 * @struct stdp_argb
 * @brief graphic A/R/G/B 설정 구조체
 *
 */
struct stdp_argb {
	unsigned char DP_A;/**< alpha, 투명도*/
	unsigned char DP_R;/**< Red color*/
	unsigned char DP_G;/**< Green color*/
	unsigned char DP_B;/**< Blue color*/
};

/**
 * @struct stdp_ycbcr
 * @brief Video Y/Cb/Cr 설정 구조체
 *
 */
struct stdp_ycbcr {
	unsigned char DP_Y;/**< Y */
	unsigned char DP_Cb;/**< Cb */
	unsigned char DP_Cr;/**< Cr */
};

/**
 * @struct stdp_range
 * @brief struct stdp_range에서 사용 범위설정 구조체
 *
 */
struct stdp_range {
	unsigned short Lower_Range;/**< start range */
	unsigned short Upper_Range;/**< end range */
};

/**
 * @struct stdp_rgb_range
 * @brief dp graphic mix 영역설정 구조체
 *
 */
struct stdp_rgb_range {
	struct stdp_range R_Range;/**< Red Range low_ragne에서 upper_range까지 범위 설정 */
	struct stdp_range G_Range;/**< Green Range low_ragne에서 upper_range까지 범위 설정 */
	struct stdp_range B_Range;/**< Blue Range low_ragne에서 upper_range까지 범위 설정 */
};

/**
 * @struct stdp_grp_prority
 * @brief graphic window 우순선위 설정 구조체
 *        First_Priority 설정한 window는 4개의 window중 가장 상위로 보인다.
 *        4개의 window가 overlap되었을 경우 가장 위에 나타난다.
 *        다음으로 second/thrid/fourth_prority로 windowr로 보인다.
 */
struct stdp_grp_prority {
	enum edp_window First_Priority;/**< 4개의window중 가장 상위*/
	enum edp_window Second_Priority;/**< 4개의window중 두번째 상위 */
	enum edp_window Third_Priotrity;/**< 4개의window중 세번째 상위*/
	enum edp_window Fourth_Priority;/**< 4개의window중 가장 하위*/
};

/**
 * @struct stdp_image_size
 * @brief dp로 입력되는 이미지의 사이즈
 */
struct stdp_image_size {
	unsigned int image_width;/**< image width size */
	unsigned int image_height;/**< image height size */
};

/**
 * @struct stdp_display_area
 * @brief dp에서 display 영역설정 구조체
 */
struct stdp_display_area {
	unsigned int H_Start;/**< pannel display 영역에서 image display horizontal 시작지점과의 offset */
	unsigned int H_Size;/**< display image width 크기 */
	unsigned int V_Start;/**< pannel display 영역에서 image display vertical 시작지점과의 offset */
	unsigned int V_Size;/**< display image height 크기 */
};

/**
 * @struct stdp_image_address
 * @brief image address 설정 구조체
 */
struct stdp_image_address {
	unsigned int y0_address;/**< y0 address */
	unsigned int y1_address;/**< y1_address */
	unsigned int c0_address;/**< c0_address */
	unsigned int c1_address;/**< c1_address */
};

struct edp_grp_on {
	enum edp_window Window;
	unsigned int Image_Addr;
	struct stdp_display_area Display_Area;
};

/**
 * @struct stvideo_display_area
 * @brief video display 구조체로서 video display window 및 video bit수, display 영역설정 구조체로 구성되어있다.<br>
 *        비트수에 따라 display position에 제약 조건이 있다.
 *        video bit수에 따라 8bit경우 horizontal start/horizontal size는 2의 배수이어야 한다.<br>
 *        video bit수가 10bit경우 horizontal start/horizontal size는 16의 배수이어야 한다.<br>
 */
struct stvideo_display_area {
	enum edp_window win;/**< dp video window */
	enum edp_video_bit bit;/**< dp input video image bit	*/
	struct stdp_display_area display;/**< dp video display area*/
};

/**
 * @struct stgraphic_display_area
 * @brief graphic display 설정 구조체
 */
struct stgraphic_display_area {
	enum edp_window win;/**< dp graphic window */
	struct stdp_display_area display;/**< dp graphic display area*/
};


/**
 * @struct stdp_tv_tg
 * @brief TV time generator values
 */
struct stdp_tv_tg {
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
 * @struct stvideodisplay
 * @brief dp video설정 관련 모든 정보를 갖고 있는 구조체<br>
 *        video window선택 window 0~4개까지 선택 가능하다.<br>
 *        video window on/off 화면 출력 결정할 수 있다.<br>
 *        video format(4:2:2,4:2:0) 선택 할 수 있다.<br>
 *        video bit(8bit/10bit)정보를 셋팅 할 수 있다.<br><br>
 *		  video address 모드로 user영역에서 physical,virtual address입력 가능하다.<br>
 *		  video stride 값을 입력가능하며, display 영역을 설정 할수 있고<br>
 *		  출력이미지 주소를 입력할 수 있다.<br>
 */
struct stvideodisplay {
	enum edp_window win;/**< window0~4개 선택할 수 있다. window별 각각 다른 메모리 영역이미지를 출력 할 수 있다. */
	enum edp_onoff win_onoff;/**< 현재 출력하는 window 켜거나 끌수가 있다. */
	enum edp_videoformat format;/**< dp input video format ycbcr420, ycbcr422선택 가능*/
	enum edp_video_bit bit;/**< dp input video image bit수로 8bit, 10bit 선택가능*/
	enum edp_address_set address_mode;/**< 이미지가 있는 영역의 주소를 가상주소 혹은 직접주소(physical)모두 입력가능*/
	unsigned short stride;/**< video stride: 8의 배수만 입력 가능 */
	unsigned int img_width;/**< dp input image width 크기 */
	unsigned int img_height;/**< dp input image height 크기 */
	struct stdp_display_area display;/**< dp display area 설정, Horizontal start/end는 video 8bit 일 경우 2의 배수,10bit 16의 배수 */
	struct stdp_image_address address;/**< dp image address 설정*/
};

/**
 * @struct stdp_video_layer_info
 * @brief tv video layer settings info
 *
 *		stride	video stride value
 *		type	3d video format type
 *		background_color	Video Y/Cb/Cr structure
 */
struct stdp_video_layer_info {
	unsigned int stride;/**< video stride value */
	enum scan_type type;/**< 3d video format type */
	struct stdp_ycbcr background_color;/**< background_color, Video YCbCr structure */
};

/**
 * @struct dp_3d
 * @brief 3D video information
 *
 */
struct dp_3d {
	enum scan_type type;/**< 3D format */
	enum tv_win window; /**< 3D L/R  Window Selection */
};

/*
 * @struct video_on
 * @brief dp tv video on settings
 *		video window. Select the video window, window 0 ~ 4 are available for selection
 *		video window on / off, to turn on/off the screen output. <br>
 *		video format (4:2:2,4:2:0) can be selected
 *		video bit selection, video bit 8bit, or 10bit
 *		video address mode, either physical or virtual address
 *		video stride value, the stride value is between 8-16376 & must be a multiple of 8
 *		input image width size
 *		input image height size
 *		3D format,
 *		image address set
 *		display area position setting
 *
 * */
struct stvideo_on {
	enum edp_window window;/**< video window */
	enum edp_onoff win_onoff;/**< video window on / off */
	enum edp_videoformat format;/**< video format (4:2:2,4:2:0) */
	enum edp_video_bit bit;/**< video bit selection */
	enum edp_address_set address_mode;/**< video address mode */
	unsigned short stride;/**< video stride value */
	unsigned int img_width;/**< input image width size */
	unsigned int img_height;/**< input image height size */
	struct dp_3d mode;/**< 3D format */
	struct stdp_image_address image_addr;/**< image address set */
	struct stdp_display_area display_area;/**< display area position setting */
};

/*
 * @struct stvideo_on_3d
 * @brief dp tv 3D video on settings
 *		video window. Select the video window, window 0 ~ 4 are available for selection
 *		video window on / off, to turn on/off the screen output. <br>
 *		video format (4:2:2,4:2:0) can be selected
 *		video bit selection, video bit 8bit, or 10bit
 *		video address mode, either physical or virtual address
 *		video stride value, the stride value is between 8-16376 & must be a multiple of 8
 *		input image width size
 *		input image height size
 *		3D format,
 *		left image address set
 *		right image address set
 *		left window display area position
 *		right windiw display area position
 *
 * */
struct stvideo_on_3d {
	enum edp_onoff win_onoff;	/**< video window on / off */
	enum edp_videoformat format;	/**< video format (4:2:2,4:2:0) */
	enum edp_video_bit bit;	/**< video bit selection */
	enum edp_address_set address_mode;	/**< video address mode */
	unsigned short stride;	/**< video stride value */
	unsigned int img_width;	/**< input image width size */
	unsigned int img_height;	/**< input image height size */
	struct dp_3d mode;/**< 3D format */
	struct stdp_image_address left_window_image_addr;	/**< image address set */
	struct stdp_image_address right_window_image_addr;	/**< image address set */
	struct stdp_display_area left_window_display_area;	/**< display area position setting */
	struct stdp_display_area right_window_display_area;	/**< display area position setting */
};

/**
 * @struct stvideo_address
 * @brief dp video address 변경관련 정보를 갖고 있는 구조체<br>
 *        video window선택 window 0~4개까지 선택 가능하다.<br>
 *		  video address 모드로 user영역에서 physical,virtual address입력 가능하다.<br>
 *		  video stride 값을 입력가능하며, display 영역을 설정 할수 있고<br>
 *		  출력이미지 주소를 입력할 수 있다.<br>
 */
struct stvideo_address {
	enum edp_window win;/**< window0~4개 선택할 수 있다. window별 각각 다른 메모리 영역이미지를 출력 할 수 있다. */
	enum edp_address_set address_mode;/**< 이미지가 있는 영역의 주소를 가상주소 혹은 직접주소(physical)모두 입력가능*/
	struct stdp_image_address address;/**< dp image address 설정*/
	int duration;
	int timestamp;
};

/**
 * @struct stgrpdisplay
 * @brief dp graphic설정 관련 모든 정보를 갖고 있는 구조체<br>
 */
struct stgrpdisplay {
	enum edp_window win;/**< window0~4개 선택할 수 있다. window별 각각 다른 메모리 영역이미지를 출력 할 수 있다. */
	enum edp_onoff win_onoff;/**< 현재 출력하는 window 켜거나 끌수가 있다. */
	unsigned short stride;/**< graphic stride: 8의 배수만 입력 가능 */
	unsigned int img_width;/**< dp input image width 크기 */
	unsigned int img_height;/**< dp input image height 크기 */
	struct stdp_display_area display;/**<dp display area 설정*/
	enum edp_address_set address_mode;/**< 이미지가 있는 영역의 주소를 가상주소 혹은 직접주소(physical)모두 입력가능*/
	unsigned int address;/**< graphic 시작 address */
};

/**
 * @struct stbb_outline
 * @brief boundary box 라인 설정 구조체<br>
 *		  boundary box 라인의 두께, 투명도,라인의 outline color설정 (보통 검은색으로 설정)<br>
 */
struct stbb_outline {
	unsigned char line_gap_width;/**< boundary box line 좌우 폭  */
	unsigned char ling_gap_height;/**< boudary box line 상하 폭  */
	unsigned char line_alpha;/**< boudary box line 투명도 */
	struct stdp_rgb rgb;/**< boudary box line 테두리 color(boundary box line의 color가 아님)  */
};

/**
 * @struct stgrp_address
 * @brief dp graphic address 설정 관련 모든 정보를 갖고 있는 구조체<br>
 */
struct stgrp_address {
	enum edp_window win;/**< window0~4개 선택할 수 있다. window별 각각 다른 메모리 영역이미지를 출력 할 수 있다. */
	enum edp_address_set address_mode;/**< 이미지가 있는 영역의 주소를 가상주소 혹은 직접주소(physical)모두 입력가능*/
	unsigned int address;/**< graphic 시작 address */
};

/**
 * @struct stbb_info
 * @brief boundary box window별 style설정, table 선택<br>
 *		  boundary box window style 0 = □ , 1 = <br>
 *        boudary box를  설정에 셋팅을 한다.<br>
 */
struct stbb_info {
	enum edp_bb_box bb_win;/**< boundary box window 0~15 선택 */
	enum edp_bb_color_table table; /**< boundary box 4개의 table중 하나 선택 */
	enum edp_bb_style_set style; /**< boundary window style 0 = □ , 1 =  */
};

/**
 * @struct stbb_onoff
 * @brief boundary box window 선택및 위치 on/off
 */
struct stbb_onoff {
	enum edp_bb_box bb_win;/**< boundary box window 0~15 선택 */
	struct stdp_display_area area;/**< boundary box display area  설정*/
	enum edp_onoff onoff; /**< boundary on/off */
};

/**
 * @struct stbb_ctrl
 * @brief boundary box window 선택및 위치 on/off
 */
struct stbb_ctrl {
	enum edp_bb_box bb_win;/**< boundary box window 0~15 선택 */
	enum edp_bb_color_table table; /**< boundary box 4개의 table중 하나 선택 */
	enum edp_bb_style_set style; /**< boundary window style 0 = □ , 1 =  */
	struct stdp_display_area area;/**< boundary box display area  설정*/
	enum edp_onoff onoff; /**< boundary on/off */
};


/**
 * @struct stlcdfilter
 * @brief video/graphic filter설정 구조체
 */
struct stlcdfilter {
	enum ehrz_filter_type type;/**< video, graphic filter 선택  */
	enum edp_filter filer_value;/**< filter type 종류 선택 */
	enum edp_onoff on_off; /**< filter on/off */
};

/**
 * @struct stzebra_set
 * @brief lcd zebra 기능설정 구조체
 */
struct stzebra_set {
	unsigned int Upper_Threshold;/**< zebrra pattern을 동작시키기 위한 하한값  */
	unsigned int Lower_Threshold;/**< zebrra pattern을 동작시키기 위한 상한값  */
	enum ezebra_width Width;/**< zebrra pattern 선 굵기  */
	enum ezebra_angle Angle;/**< zebrra pattern 기울기  */
	enum ezebra_speed Speed; /**< zebrra pattern 속도  */
	struct stdp_argb Pattern_Color; /**< zebrra pattern RGB 색상  */
	enum edp_onoff on_off;/**< zebrra pattern On/Off  */
};

/**
 * @struct stlcd_tcc
 * @brief lcd TCC 기능설정 구조체
 */
struct stlcd_tcc {
	enum edp_onoff onoff;
	enum edp_onoff video_only;
	unsigned int TCC_TABLE[256];
};

/**
 * @struct stfb_info
 * @brief d4_dp_lcd.c에서 framebuffer memory maping시 가져오는 dp virtual address
 */
struct stfb_info {
	unsigned int dp_global_regs;/**< dp global address  */
	unsigned int dp_tv_regs;/**< dp tv address  */
	unsigned int dp_lcd_regs;/**< dp lcd address  */
	unsigned int dp_sublcd_regs;/**< dp sub lcd address  */
};

/**
 * @struct stfb_tv_video_info
 * @brief platform data structure to obtain information of the video
 */
struct stfb_tv_video_info {
	enum edp_window win; /**< dp window 0~3  */
	enum edp_videoformat format; /**< dp video format 4:2:2/4:2:0 */
	enum edp_video_bit bit; /**< dp video bit 8bit,10bit  */
	enum edp_input_img_type scantype; /**< dp input image scantype */
	unsigned int vid_stride; /**< dp video stride */
	struct stdp_image_size image; /**< dp input image size  */
	struct stdp_display_area display; /**< dp input display area  */
	struct stdp_image_address address; /**< dp image address	*/
};

/**
 * @struct stfb_video_info
 * @brief platform data중 video정보를 가져오는 구조체
 */
struct stfb_video_info {
	enum edp_window win;/**< dp window 0~3 선택 */
	enum edp_videoformat format;/**< dp video format 4:2:2/4:2:0 선택  */
	enum edp_video_bit bit; /**< dp video bit 8bit,10bit  선택*/
	unsigned int vid_stride; /**< dp video stride  셋팅*/
	struct stdp_image_size image; /**< dp input image size  */
	struct stdp_display_area display;/**< dp input display area  */
	struct stdp_image_address address;/**< dp image address*/
};

/**
 * @struct stfb_graphic_info
 * @brief platform data중 graphic정보를 가져오는 구조체
 */
struct stfb_graphic_info {
	enum edp_window win;/**< dp window 0~3 */
	unsigned int grp_stride;/**< dp graphic stride */
	struct stdp_image_size image;/**< dp input image size  */
	struct stdp_display_area display;/**< dp input display area  */
	unsigned int address;/**< graphic start address  */
};

/**
 * @struct stnlc_address
 * @brief nlc input image address 설정 구조체
 */
struct stnlc_address {
	unsigned int Addr_Y0;/**< Y address */
	unsigned int Addr_C0;/**< C address */
};

/**
 * @struct sttnlc_video
 * @brief nlc video관련 설정 구조체
 */
struct sttnlc_video {
	enum edp_path path;/**< DP Path, Main LCD/Sub LCD/ TV  */
	enum edp_videoformat format; /**< dp video format 4:2:2/4:2:0 선택  */
	unsigned int inputImage_width;/**< dp input image width 크기 */
	unsigned int inputImage_height;/**< dp input image height 크기 */
	struct stnlc_address address;/**< 시작 address */
	struct stdp_display_area display;/**<dp display area 설정*/
};

/**
 * @struct stnlc_grp_wrap
 * @brief nlc graphic wrap관련 구조체
 *		\image html dp_biblt_nlc.jpg
 * 위의 그림과 같이 Graphic NLC는 Biblt에서만 사용된다. <br>
 * biblt에서 NLC Stream 만들어질 때, 각 tile별 strem size가 가변되어 입력되어 질수 있다. <br>
 * DP에서는 각 tile별 가변된 크기 사이즈를 wrap하여 각타일을 모아 온전한 이미지로 출력 가능하다. <br>
 * wrap이미지를 만들기 위해 biblt에서 만들어진 stream 정보를 이 구조체를 이용하여 입력해야 출력이 가능하다. <br>
 */
struct stnlc_grp_wrap {
	unsigned int tile_size[6];/**< biblt에서 만들어진 tile별 가변 사이즈 입력, 최대 6개 타일 구성*/
	unsigned short tile_num;/**< biblt에서 만들어진 tile별 갯수*/
	unsigned int e_address; /**< stream even address*/
	unsigned int o_address; /**< stream odd address*/
};

/**
 * @struct stnlc_graphic
 * @brief nlc graphic 설정 구조체
 */
struct stnlc_graphic {
	unsigned int image_width; /**< dp input image width 크기 */
	unsigned int image_height; /**< dp input image height 크기 */
	struct stnlc_grp_wrap grp_wrap; /**< nlc graphic wrap관련 구조체 */
	enum edp_window display_win;/**< dp window 0~3 */
	struct stdp_display_area display;/**<dp display area 설정*/
};

/**
 * @struct sttv_graphic_alpha
 * @brief graphic window alpha setting
 */
struct sttv_graphic_alpha {
	enum edp_window win;/**< graphic window */
	unsigned char alpha_value;/**< alpha value */
	enum edp_onoff on_off;/**< to set the window on/off */
};

/**
 * @struct stlcd_graphic_alpha
 * @brief graphic window별 투명도를 조절 할 수 있다.
 */
struct stlcd_graphic_alpha {
	enum edp_window win;/**< graphic 정보 */
	unsigned char alpha_value;/**< 투명도 0~255 값이 높을 수록 투명도가 높다. */
	enum edp_onoff on_off;/**< 투명도 동작 on/off */
};

/**
 * @struct stlcd_graphic_alpha
 * @brief graphic window별 투명도를 조절 할 수 있다.
 */
struct stlcd_gm_lcd {
	enum edp_onoff gm_fict;/**< 투명도 동작 on/off */
	unsigned short gm_value[9];
	enum edp_onoff chroma_expand;
	unsigned char gm_cg_gr;
	unsigned char gm_cg_rb;
	unsigned char gm_cg_gb;
	unsigned char gm_hth_gr;
	unsigned char gm_hth_rb;
	unsigned char gm_hth_gb;
};

struct stlcd_bt656 {
	enum edp_path path;
	enum edp_video_bit bit;
	enum edp_ycbcr_order ycbcr_order;
	enum edp_onoff onoff;
};

struct stlcd_csc_matrix {
	unsigned short matrix_set_00;
	unsigned short matrix_set_01;
	unsigned short matrix_set_02;
	unsigned short matrix_set_10;
	unsigned short matrix_set_11;
	unsigned short matrix_set_12;
	unsigned short matrix_set_20;
	unsigned short matrix_set_21;
	unsigned short matrix_set_22;
	unsigned short ycbcr_offset_y;
	unsigned short ycbcr_offset_cb;
	unsigned short ycbcr_offset_cr;
	unsigned short rgb_upper_range;
	unsigned short rgb_lower_range;
};

#endif
