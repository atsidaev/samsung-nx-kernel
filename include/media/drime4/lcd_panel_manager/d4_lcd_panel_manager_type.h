/**
 * @file d4_lcd_panel_manager_type.h
 * @brief DRIMe4 LCD Panel Manager Structure & Enumeration Define
 * 2010 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/
#ifndef __DRIME4_LCD_PANEL_MANAGER_DATA_TYPE_H__
#define __DRIME4_LCD_PANEL_MANAGER_DATA_TYPE_H__

/******************************************************************************/
/*                                Enumeration                                 */
/******************************************************************************/

/**
 * @enum lpm_onoff
 * @brief LCD Panel Manager on/off 및 동작 설정을 위한 enumeration
 */
enum lpm_onoff {
	LPM_OFF,	/**< 동작 중지 및 설정 OFF */
	LPM_ON		/**< 동작 시작 및 설정 ON*/
};

/**
 * @enum lpm_lcd_ratio
 * @brief LCD Panel 의 ratio 설정을 위한 enumeration
 */
enum lpm_lcd_ratio {
	LPM_RATIO_4_3,
	LPM_RATIO_16_9,
	LPM_RATIO_5_4
};

/**
 * @enum lpm_lcd_bitwidth
 * @brief LCD Panel 의 사용 bit-width 설정을 위한 enumeration
 */
enum lpm_lcd_bitwidth {
	LPM_BIT_WIDTH_8,
	LPM_BIT_WIDTH_24,
	LPM_BIT_WIDTH_16
};

/**
 * @enum lpm_lcd_array_type
 * @brief LCD Panel 의 array type 설정을 위한 enumeration
 */
enum lpm_lcd_array_type {
	LPM_DELTA,
	LPM_STRIPE
};

/**
 * @enum lpm_lcd_out_format
 * @brief LCD Panel 의 output format 설정을 위한 enumeration
 */
enum lpm_lcd_out_format {
	LPM_RGB_OUT,
	LPM_YCC_OUT,
	LPM_CCIR656_OUT
};

/**
 * @enum lpm_lcd_array_type
 * @brief LCD Panel 의 even/odd line 의 R/G/B 배열 순서 설정을 위한 enumeration
 */
enum lpm_lcd_dot_sequence {
	LPM_RGB,
	LPM_GBR,
	LPM_BGR,
	LPM_RBG,
	LPM_GRB,
	LPM_BRG
};

/**
 * @enum lpm_mlcd_select
 * @brief Main LCD 선택을 위한 enumeration, 두 Main LCD를 동시에 사용은 불가능
 */
enum lpm_mlcd_select {
	LPM_MLCD_1_SELECT,
	LPM_MLCD_2_SELECT
};

/**
 * @enum lpm_lcd_brightness_level
 * @brief LCD  brightness level 설정을 위한 enumeration
 */
enum lpm_lcd_brightness_level{
	LPM_BLACK_LEVEL,
	LPM_DARK_LEVEL,
	LPM_NORMAL_DARK_LEVEL,
	LPM_NORMAL_LEVEL,
	LPM_NORMAL_BRIGHT_LEVEL,
	LPM_BRIGHT_LEVEL
};

/**
 * @enum lpm_lcd_rotation
 * @brief LCD rotation control 을 위한 enumeration
 */
enum lpm_lcd_rotation {
	LPM_ROTATION_OFF,
	LPM_ROTATION_HALF,
	LPM_ROTATION_FULL
};

/******************************************************************************/
/*                                Structure                                   */
/******************************************************************************/

/**
 * @struct lcd_panel_ctrl_func
 * @brief LCD panel control function struct
 *
 * @author Kim Sung Hoon
 */
struct lcd_panel_ctrl_func {

	/* Basic Function */

	void (*reset)(void);		    		/**< LCD reset function */

	void (*power_on)(void);		    		/**< LCD power on function */
	void (*power_off)(void);		    	/**< LCD power off function */
	void (*init)(void);			    		/**< LCD initialization function */

	void (*light_on)(void);		    		/**< LCD light on  function, including back light on */
	void (*light_off)(void);		    	/**< LCD light off function, including back light off */

	void (*set_brightness)(int);	    	/**< LCD panel brightness setting function */

	/* Optional Function */

	void (*control_standby_on)(void);       /**< LCD stand-by on Function */
	void (*control_standby_off)(void);      /**< LCD stand-by off Function */

	void (*control_h_flip_on)(void);	    /**< LCD horizontal flip on function */
	void (*control_h_flip_off)(void);	    /**< LCD horizontal flip off function */

	void (*control_v_flip_on)(void);	    /**< LCD vertical flip on function */
	void (*control_v_flip_off)(void);	    /**< LCD vertical flip off function */

	void (*control_rotation)(int);          /**< LCD Rotation control function */
	void (*control_brightness_gamma)(int);  /**< LCD panel brightness gamma setting function */
	void (*control_csc)(void *);	    	/**< LCD CSC control function */
	void (*control_tcc)(void *);	    	/**< LCD TCC control function */
	void (*control_extension)(void *);	    /**< LCD extension control function */


	void (*control_suspend)(void);			/**< suspend function for power management */
	void (*control_resume)(void);			/**< resume  function for power management */
};

/**
 * @struct lcd_panel_timing_info
 * @brief LCD panel timing signal information structure
 *
 * @author Kim Sung Hoon
 */
struct lcd_panel_timing_info {
	unsigned short total_h_size;       /**< Horizontal 전체 Size */
	unsigned short total_v_size;       /**<   Vertical 전체 Size */

	unsigned short hsync_rising;       /**< HSYNC의  Rising 되는 position */
	unsigned short hsync_falling;      /**< HSYNC의 Falling 되는 position */

	unsigned short vsync_rising;       /**< VSYNC의  Rising 되는 position */
	unsigned short vsync_falling;      /**< VSYNC의  Falling 되는 position */

	unsigned short buf_read_h_start;   /**< Buffer 에서 Horizontal 신호를 읽는 시점 선택
									   Tuning 필요: Enable H Start - 8 이 기준 */
	unsigned short enable_h_start;     /**< H Enable 신호의   시작 position :Rising 되는 시점 */
	unsigned short enable_h_size;      /**< H Enable 신호의 마지막 position :Falling 되는 시점 */

	unsigned short enable_v_start;     /**< V Enable 신호의   시작 position */
	unsigned short enable_v_size;      /**< V Enable 신호의 마지막 position */

	enum lpm_onoff inv_dot_clk;         /**< Dot Clock 의 Inversion 설정을 위한 인자 */
	enum lpm_onoff inv_enable_clk;      /**< Enable Clock 의 Inversion 설정을 위한 인자 */
	enum lpm_onoff inv_hsync;           /**< HSYNC 의 Inversion 설정을 위한 인자 */
	enum lpm_onoff inv_vsync;           /**< VSYNC 의 Inversion 설정을 위한 인자 */
};

/**
 * @struct lcd_panel_hw_info
 * @brief LCD panel H/W information structure
 *
 * @author Kim Sung Hoon
 */
struct lcd_panel_hw_info {
	unsigned short  lcd_width_size;
	unsigned short  lcd_height_size;
	unsigned int 	lcd_clock;

	enum lpm_lcd_ratio 			lcd_ratio;
	enum lpm_lcd_array_type 	lcd_dot_array_type;  /**< delta, stripe */
	enum lpm_lcd_dot_sequence 	lcd_dot_even_sequence;
	enum lpm_lcd_dot_sequence 	lcd_dot_odd_sequence;

	enum lpm_lcd_out_format 	lcd_out_format;	    /**< RGB, YCbCr, CCIR656 */
	enum lpm_lcd_bitwidth 		lcd_out_bitwith;	/**< 8, 16, 24 */
};

/**
* @struct lcd_panel_manager_info
* @brief LCD panel managing structure
*
* @author Kim Sung Hoon
*/
struct lcd_panel_manager_info {
	struct lcd_panel_hw_info 		*hw_info;			 /**< LCD Panel H/W information */
	struct lcd_panel_timing_info 	*timing_signal_info; /**< LCD Panel timing signal information */
    struct lcd_panel_ctrl_func 		*control_function;   /**< LCD panel control function information */
    const unsigned char 			*comment;            /**< Comment */
};

/**
* @struct lcd_panel_spi_write_info
* @brief LCD panel SPI write information structure
*
* @author Kim Sung Hoon
*/
struct lcd_panel_spi_write_info {
	unsigned int	data_cnt;
	void			*data_buf;
};

/**
* @struct lcd_panel_spi_rw_info
* @brief LCD panel SPI read/write information structure
*
* @author Kim Sung Hoon
*/
struct lcd_panel_spi_rw_info {
	unsigned int data_cnt;
	void 		 *w_data_buf;
	void 		 *r_data_buf;
};

/**< extension control structure */
/**< 사용자가 용도에 맞게 정의 하여 사용을 하여야 한다. */
/**< Example */
struct lcd_panel_ext_ctrl_info {
	unsigned int ctrl_info_1;
	unsigned int ctrl_info_2;
	char *comment;
};

#endif   /* __DRIME4_LCD_PANEL_MANAGER_DATA_TYPE_H__ */

