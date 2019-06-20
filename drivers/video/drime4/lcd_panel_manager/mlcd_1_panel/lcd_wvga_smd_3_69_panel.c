/**
 * @file lcd_vga_cmd_3_69_panel.c
 * @brief SMD VGA 3.69 TFT LCD panel driver.
 * @author Jo Yonghoon <yh0302.jo@samsung.com>
 * 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/wait.h>
#include <linux/fb.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/gpio.h>
#include <linux/d4_ht_pwm.h>
#include <linux/pinctrl/pinconf-drime4.h>
#include <linux/pinctrl/pinconf.h>
#include <linux/pinctrl/consumer.h>
#include <mach/version_information.h>
#include "../../lcd/d4_dp_lcd_dd.h"
#include "../../lcd_panel_manager/d4_lcd_panel_manager_ctrl.h"

/* Private variable */

#define CALC_PARAM_CNT(x)	((sizeof(x) / 2) - 1)
#define LCD_PWM_CH 	6

/**< define for LCD brightness setting */

#define WVGA_SMD_369_BLACK			5220000 // 13%
#define WVGA_SMD_369_DARK         	4440000	// 26%
#define WVGA_SMD_369_NORMAL_DARK   	4080000	// 32%
#define WVGA_SMD_369_NORMAL        	3720000 // 38% 
#define WVGA_SMD_369_BRIGHT_NORMAL	3360000 // 44%
#define WVGA_SMD_369_BRIGHT        	3000000 // 50%


/**< LCD panel control value */

static const unsigned short panel_init_set_address_mode_table[] = {
		0x036,
		0x10A
};

static const unsigned short panel_init_manufacturer_command_table[] = {
		0x0B0,
		0x100
};

static const unsigned short panel_init_panel_driving_table[] = {
		0x0C0,
		0x128, 0x108
};

static const unsigned short panel_init_score_control_table[] = {
		0x0C1,
		0x101, 0x130, 0x115, 0x105, 0x122
};

static const unsigned short panel_init_gate_interface_table[] = {
		0x0C4,
		0x110, 0x101, 0x100
};

static const unsigned short panel_init_display_h_timming_table[] = {
		0x0C5,
		0x106, 0x155, 0x103, 0x107, 0x10B, 0x133, 0x100, 0x101, 0x103
};

static const unsigned short panel_init_rgb_sync_option_table[] = {
		0x0C6,
		0x101
};

static const unsigned short panel_init_gamma_set_red_table[] = {
		0x0C8,
		0x100, 0x17A, 0x109, 0x111, 0x11F, 0x127, 0x127, 0x12E, 0x12E, 0x138, 
		0x145, 0x146, 0x148, 0x146, 0x14F, 0x15A, 0x15A, 0x15D, 0x13C, 0x100,
		0x17A, 0x109, 0x111, 0x11F, 0x127, 0x127, 0x12E, 0x12E, 0x138, 0x145, 
		0x146, 0x148, 0x146, 0x14F, 0x15A, 0x15A, 0x15D, 0x13C
};

static const unsigned short panel_init_gamma_set_green_table[] = {
		0x0C9,
		0x100, 0x141, 0x113, 0x120, 0x134, 0x141, 0x144, 0x146, 0x14A, 0x154, 
		0x15C, 0x15D, 0x15D, 0x15A, 0x15E, 0x165, 0x164, 0x165, 0x13E, 0x100,
		0x141, 0x113, 0x120, 0x134, 0x141, 0x144, 0x146, 0x14A, 0x154, 0x15C, 
		0x15D, 0x15D, 0x15A, 0x15E, 0x165, 0x164, 0x165, 0x13E
};

static const unsigned short panel_init_gamma_set_blue_table[] = {
		0x0CA,
		0x100, 0x128, 0x115, 0x125, 0x13B, 0x14A, 0x14D, 0x150, 0x154, 0x15E, 
		0x166, 0x166, 0x166, 0x162, 0x167, 0x16B, 0x169, 0x168, 0x140, 0x100,
		0x128, 0x115, 0x125, 0x13B, 0x14A, 0x14D, 0x150, 0x154, 0x15E, 0x166, 
		0x166, 0x166, 0x162, 0x167, 0x16B, 0x169, 0x168, 0x140
};

static const unsigned short panel_init_bias_current_control_table[] = {
		0x0D1,
		0x133, 0x113
};

static const unsigned short panel_init_ddv_control_table[] = {
		0x0D2,
		0x111, 0x100, 0x100
};


static const unsigned short panel_init_gamma_control_ref_table[] = {
		0x0D3,
		0x14F, 0x14E
};

static const unsigned short panel_vcomdc_control_table[] = {
		0x0D4,
		0x157, 0x157
};

static const unsigned short panel_init_dcdc_control_table[] = {
		0x0D5,
		0x12F, 0x111, 0x11E, 0x146
};


static const unsigned short panel_init_vcl_control_table[] = {
		0x0D6,
		0x111, 0x10A	
};

static const unsigned short nvm_load1_table[] = {
		0x0F8,
		0x101, 0x1F5, 0x1F2, 0x171, 0x144	
};

static const unsigned short nvm_load2_table[] = {
		0x0FC,
		0x100, 0x108	
};

static unsigned int uiTcc_Gamma[256] = {
    0x00000000, 0x01010101, 0x02020202, 0x03030303, 0x04040404, 0x05050505, 0x06060606, 0x07070707, 0x08080808, 0x09090909, 0x0A0A0A0A, 0x0B0B0B0B, 0x0C0C0C0C, 0x0D0D0D0D, 0x0E0E0E0E, 0x0F0F0F0F,
    0x10101010, 0x11111111, 0x12121212, 0x13131313, 0x14141414, 0x15151515, 0x16161616, 0x17171717, 0x18181818, 0x19191919, 0x1A1A1A1A, 0x1B1B1B1B, 0x1C1C1C1C, 0x1D1D1D1D, 0x1E1E1E1E, 0x1F1F1F1F,
    0x20202020, 0x21212121, 0x22222222, 0x23232323, 0x24242424, 0x25252525, 0x26262626, 0x27272727, 0x28282828, 0x29292929, 0x2A2A2A2A, 0x2B2B2B2B, 0x2C2C2C2C, 0x2D2D2D2D, 0x2E2E2E2E, 0x2F2F2F2F,
    0x30303030, 0x31313131, 0x32323232, 0x33333333, 0x34343434, 0x35353535, 0x36363636, 0x37373737, 0x38383838, 0x39393939, 0x3A3A3A3A, 0x3B3B3B3B, 0x3C3C3C3C, 0x3D3D3D3D, 0x3E3E3E3E, 0x3F3F3F3F,
    0x40404040, 0x41414141, 0x42424242, 0x43434343, 0x44444444, 0x45454545, 0x46464646, 0x47474747, 0x48484848, 0x49494949, 0x4A4A4A4A, 0x4B4B4B4B, 0x4C4C4C4C, 0x4D4D4D4D, 0x4E4E4E4E, 0x4F4F4F4F,
    0x50505050, 0x51515151, 0x52525252, 0x53535353, 0x54545454, 0x55555555, 0x56565656, 0x57575757, 0x58585858, 0x59595959, 0x5A5A5A5A, 0x5B5B5B5B, 0x5C5C5C5C, 0x5D5D5D5D, 0x5E5E5E5E, 0x5F5F5F5F,
    0x60606060, 0x61616161, 0x62626262, 0x63636363, 0x64646464, 0x65656565, 0x66666666, 0x67676767, 0x68686868, 0x69696969, 0x6A6A6A6A, 0x6B6B6B6B, 0x6C6C6C6C, 0x6D6D6D6D, 0x6E6E6E6E, 0x6F6F6F6F,
    0x70707070, 0x71717171, 0x72727272, 0x73737373, 0x74747474, 0x75757575, 0x76767676, 0x77777777, 0x78787878, 0x79797979, 0x7A7A7A7A, 0x7B7B7B7B, 0x7C7C7C7C, 0x7D7D7D7D, 0x7E7E7E7E, 0x7F7F7F7F,
    0x80808080, 0x81818181, 0x82828282, 0x83838383, 0x84848484, 0x85858585, 0x86868686, 0x87878787, 0x88888888, 0x89898989, 0x8A8A8A8A, 0x8B8B8B8B, 0x8C8C8C8C, 0x8D8D8D8D, 0x8E8E8E8E, 0x8F8F8F8F,
    0x90909090, 0x91919191, 0x92929292, 0x93939393, 0x94949494, 0x95959595, 0x96969696, 0x97979797, 0x98989898, 0x99999999, 0x9A9A9A9A, 0x9B9B9B9B, 0x9C9C9C9C, 0x9D9D9D9D, 0x9E9E9E9E, 0x9F9F9F9F,
    0xA0A0A0A0, 0xA1A1A1A1, 0xA2A2A2A2, 0xA3A3A3A3, 0xA4A4A4A4, 0xA5A5A5A5, 0xA6A6A6A6, 0xA7A7A7A7, 0xA8A8A8A8, 0xA9A9A9A9, 0xAAAAAAAA, 0xABABABAB, 0xACACACAC, 0xADADADAD, 0xAEAEAEAE, 0xAFAFAFAF,
    0xB0B0B0B0, 0xB1B1B1B1, 0xB2B2B2B2, 0xB3B3B3B3, 0xB4B4B4B4, 0xB5B5B5B5, 0xB6B6B6B6, 0xB7B7B7B7, 0xB8B8B8B8, 0xB9B9B9B9, 0xBABABABA, 0xBBBBBBBB, 0xBCBCBCBC, 0xBDBDBDBD, 0xBEBEBEBE, 0xBFBFBFBF,
    0xC0C0C0C0, 0xC1C1C1C1, 0xC2C2C2C2, 0xC3C3C3C3, 0xC4C4C4C4, 0xC5C5C5C5, 0xC6C6C6C6, 0xC7C7C7C7, 0xC8C8C8C8, 0xC9C9C9C9, 0xCACACACA, 0xCBCBCBCB, 0xCCCCCCCC, 0xCDCDCDCD, 0xCECECECE, 0xCFCFCFCF,
    0xD0D0D0D0, 0xD1D1D1D1, 0xD2D2D2D2, 0xD3D3D3D3, 0xD4D4D4D4, 0xD5D5D5D5, 0xD6D6D6D6, 0xD7D7D7D7, 0xD8D8D8D8, 0xD9D9D9D9, 0xDADADADA, 0xDBDBDBDB, 0xDCDCDCDC, 0xDDDDDDDD, 0xDEDEDEDE, 0xDFDFDFDF,
    0xE0E0E0E0, 0xE1E1E1E1, 0xE2E2E2E2, 0xE3E3E3E3, 0xE4E4E4E4, 0xE5E5E5E5, 0xE6E6E6E6, 0xE7E7E7E7, 0xE8E8E8E8, 0xE9E9E9E9, 0xEAEAEAEA, 0xEBEBEBEB, 0xECECECEC, 0xEDEDEDED, 0xEEEEEEEE, 0xEFEFEFEF,
    0xF0F0F0F0, 0xF1F1F1F1, 0xF2F2F2F2, 0xF3F3F3F3, 0xF4F4F4F4, 0xF5F5F5F5, 0xF6F6F6F6, 0xF7F7F7F7, 0xF8F8F8F8, 0xF9F9F9F9, 0xFAFAFAFA, 0xFBFBFBFB, 0xFCFCFCFC, 0xFDFDFDFD, 0xFEFEFEFE, 0xFFFFFFFF,
};

static const int iColorAdjustOffset[15][15][3] = {
//            Y1		    Y2	             Y3		         Y4		         Y5			     Y6				 Y7				 Y8				 Y9				 Y10			 Y11			 Y12			 Y13			 Y14			 Y15
    {{28,-140,112},	{20,-124,104},	{12,-108,96},	{4,-92,88},	    {-4,-76,80},	{-12,-60,72},	{-20,-44,64},	{-56,-56,112},	{-64,-40,104},	{-72,-24,96},	{-80,-8,88},	{-88,8,80}, 	{-96,24,72},	{-104,40,64},	{-112,56,56}},
    {{32,-136,104},	{24,-120,96},	{16,-104,88},	{8,-88,80},	    {0,-72,72},	    {-8,-56,64},	{-16,-40,56},	{-48,-48,96},	{-56,-32,88},	{-64,-16,80},	{-72,0,72}, 	{-80,16,64},	{-88,32,56},	{-96,48,48},	{-104,64,40}},
    {{36,-132,96},	{28,-116,88},	{20,-100,80},	{12,-84,72},	{4,-68,64},	    {-4,-52,56},	{-12,-36,48},	{-40,-40,80},	{-48,-24,72},	{-56,-8,64},	{-64,8,56}, 	{-72,24,48},	{-80,40,40},	{-88,56,32},	{-96,72,24}},
    {{40,-128,88},  {32,-112,80},	{24,-96,72},	{16,-80,64},	{8,-64,56},	    {0,-48,48}, 	{-8,-32,40},	{-32,-32,64},	{-40,-16,56},	{-48,0,48},	    {-56,16,40},	{-64,32,32},	{-72,48,24},	{-80,64,16},	{-88,80,8}},
    {{44,-124,80},	{36,-108,72},	{28,-92,64},	{20,-76,56},	{12,-60,48},	{4,-44,40}, 	{-4,-28,32},	{-24,-24,48},	{-32,-8,40},	{-40,8,32},	    {-48,24,24},	{-56,40,16},	{-64,56,8}, 	{-72,72,0},	    {-80,88,-8}},
    {{48,-120,72},	{40,-104,64},	{32,-88,56},	{24,-72,48},	{16,-56,40},	{8,-40,32},	    {0,-24,24},	    {-16,-16,32},	{-24,0,24}, 	{-32,16,16},	{-40,32,8}, 	{-48,48,0},	    {-56,64,-8},	{-64,80,-16},	{-72,96,-24}},
    {{52,-116,64},	{44,-100,56},	{36,-84,48},	{28,-68,40},	{20,-52,32},	{12,-36,24},	{4,-20,16},	    {-8,-8,16},	    {-16,8,8},	    {-24,24,0}, 	{-32,40,-8},	{-40,56,-16},	{-48,72,-24},	{-56,88,-32},	{-64,104,-40}},
    {{56,-112,56},	{48,-96,48},	{40,-80,40},	{32,-64,32},	{24,-48,24},	{16,-32,16},	{8,-16,8},	    {0,0,0},	    {-8,16,-8},	    {-16,32,-16},	{-24,48,-24},	{-32,64,-32},	{-40,80,-40},	{-48,96,-48},	{-56,112,-56}},
    {{64,-116,52},	{56,-100,44},	{48,-84,36},	{40,-68,28},	{32,-52,20},	{24,-36,12},	{16,-20,4},	    {16,-8,-8},	    {8,8,-16},	    {0,24,-24},	    {-8,40,-32},	{-16,56,-40},	{-24,72,-48},	{-32,88,-56},	{-40,104,-64}},
    {{72,-120,48},	{64,-104,40},	{56,-88,32},	{48,-72,24},	{40,-56,16},	{32,-40,8},	    {24,-24,0},	    {32,-16,-16},	{24,0,-24}, 	{16,16,-32},	{8,32,-40}, 	{0,48,-48},	    {-8,64,-56},	{-16,80,-64},	{-24,96,-72}},
    {{80,-124,44},	{72,-108,36},	{64,-92,28},	{56,-76,20},	{48,-60,12},	{40,-44,4},	    {32,-28,-4},	{48,-24,-24},	{40,-8,-32},	{32,8,-40},	    {24,24,-48},	{16,40,-56},	{8,56,-64}, 	{0,72,-72},	    {-8,88,-80}},
    {{88,-128,40},	{80,-112,32},	{72,-96,24},	{64,-80,16},	{56,-64,8},	    {48,-48,0},	    {40,-32,-8},	{64,-32,-32},	{56,-16,-40},	{48,0,-48},	    {40,16,-56},	{32,32,-64},	{24,48,-72},	{16,64,-80},	{8,80,-88}},
    {{96,-132,36},	{88,-116,28},	{80,-100,20},	{72,-84,12},	{64,-68,4},	    {56,-52,-4},	{48,-36,-12},	{80,-40,-40},	{72,-24,-48},	{64,-8,-56},	{56,8,-64},	    {48,24,-72},	{40,40,-80},	{32,56,-88},	{24,72,-96}},
    {{104,-136,32},	{96,-120,24},	{88,-104,16},	{80,-88,8}, 	{72,-72,0},	    {64,-56,-8},	{56,-40,-16},	{96,-48,-48},	{88,-32,-56},	{80,-16,-64},	{72,0,-72}, 	{64,16,-80},	{56,32,-88},	{48,48,-96},	{40,64,-104}},
    {{112,-140,28},	{104,-124,20},	{96,-108,12},	{88,-92,4},	    {80,-76,-4},	{72,-60,-12},	{64,-44,-20},	{112,-56,-56},	{104,-40,-64},	{96,-24,-72},	{88,-8,-80},	{80,8,-88},	    {72,24,-96},	{64,40,-104},	{56,56,-112}},
};

/* Private function */
static int mlcd_1_set_table_value(const unsigned short *wbuf, unsigned int param_cnt);

/**< LCD panel control functions */
static void mlcd_1_ctrl_power_on(void);
static void mlcd_1_ctrl_reset(void);
static void mlcd_1_ctrl_init(void);
static void mlcd_1_ctrl_display_on(void);
static void mlcd_1_ctrl_display_off(void);
static void mlcd_1_ctrl_standby_on(void);
static void mlcd_1_ctrl_standby_off(void);
static void mlcd_1_ctrl_initializing_sequence(void);
static void mlcd_1_ctrl_vnm_load_sequence(void);
static void mlcd_1_ctrl_sleep_in(void);
static void mlcd_1_ctrl_sleep_out(void);
static void mlcd_1_ctrl_power_off(void);
static void mlcd_1_ctrl_brightness(int level);
static void mlcd_1_ctrl_tcc(void *tcc_info);
static void mlcd_1_ctrl_csc(void *csc_info);
static void mlcd_tcc_manual_set(void *tcc_info);
static void mlcd_1_ctrl_resume(void);
static void mlcd_1_ctrl_extension(void *ext_info);


/******************************************************************************/
/*                     Main LCD 1 Panel Information                           */
/******************************************************************************/

struct lcd_panel_hw_info mlcd_1_panel_hw_info = {
 /**< width/ height/   LCD clock/       LCD ratio/  dot array type */
		480,	800,	25920000,	LPM_RATIO_16_9,	LPM_STRIPE,
 /**< dot even seq/ dot odd seq/ output format/ output bit-width */
		LPM_BGR,	LPM_BGR,	LPM_RGB_OUT,	LPM_BIT_WIDTH_24
};

struct lcd_panel_timing_info mlcd_1_panel_timing_info = {
 /**< total h/  total v/  h rising/ h falling/ v rising/ v falling */
		527,		818,		0,		4,			0,		1,

 /**< read h start/ enable h start/ enable h size/ enable v start/ enable v size */
		34,			44,			480,			8,			800,


 /**< inv_dot clk/ inv_enable/ inv_hsync/ inv_vsync */
	  LPM_ON,		  LPM_OFF,	    LPM_ON,		LPM_ON
};

static struct d4_hs_spi_config spi_config = {
	.speed_hz = 5000000,
	.bpw = 9,
	.mode = SH_SPI_MODE_3,
	.waittime = 100,
	.ss_inven = D4_SPI_TRAN_INVERT_OFF,
	.spi_ttype = D4_SPI_TRAN_BURST_OFF,
	.setup_select = SH_SPI_SPEED|SH_SPI_BPW|SH_SPI_WAVEMODE|SH_SPI_WAITTIME|SH_SPI_INVERT|SH_SPI_BURST
};

struct lcd_panel_ctrl_func mlcd_1_panel_ctrl_func = {

		/* Basic Function */

		mlcd_1_ctrl_reset, 			/**< LCD reset function */

		mlcd_1_ctrl_power_on,		/**< LCD power on function */
		mlcd_1_ctrl_power_off,		/**< LCD power off function */
		mlcd_1_ctrl_init,			/**< LCD initialization function */

		mlcd_1_ctrl_display_on,		/**< LCD light on  function, including back light on */
		mlcd_1_ctrl_display_off,	/**< LCD light off function, including back light off */

		mlcd_1_ctrl_brightness,		/**< LCD panel brightness setting function */

		/* Optional Function */

		mlcd_1_ctrl_standby_on,     /**< LCD stand-by on Function */
		mlcd_1_ctrl_standby_off,   	/**< LCD stand-by off Function */

		NULL,	    				/**< LCD horizontal flip on function */
		NULL,	    				/**< LCD horizontal flip off function */

		NULL,	    				/**< LCD vertical flip on function */
		NULL,	    				/**< LCD vertical flip off function */

		NULL,       				/**< LCD Rotation control function */
		NULL,  						/**< LCD panel brightness gamma setting function */
		mlcd_1_ctrl_csc,    		/**< LCD CSC control function */
		mlcd_1_ctrl_tcc,			/**< LCD TCC control function */
		mlcd_1_ctrl_extension,		/**< LCD extension control function */

		NULL,						/**< suspend function for power management */
		mlcd_1_ctrl_resume			/**< resume  function for power management */
};

struct lcd_panel_manager_info mlcd_1_panel_info = {
    &mlcd_1_panel_hw_info,
    &mlcd_1_panel_timing_info,
    &mlcd_1_panel_ctrl_func,
    " VGA SMD 3.69 Inch LCD Panel "  
};

/******************************************************************************/
/*                  Main LCD 1 Panel Control Functions                        */
/******************************************************************************/

static void mlcd_1_ctrl_power_on(void)
{
	/**< for test */
	mlcd_1_panel_set_gpio(DRIME4_GPIO1(1), LPM_ON);
	mdelay(50); 
}


struct ht_pwm_device *lcd_pwm;

struct ht_pwm_conf_info pwm_setup = {
		.opmode = CONTINUE_PULSE,
		.trigtype = SW_TRIGGER,
		.freq1.period = 6000000,
		.freq1.duty = 3420000,
		.freq1.count = 0	
};

static bool pwm_request_state = false;

void mlcd_1_ctrl_bl_on(void)
{
	/*
	 * LCD_BL_ON
	 */
	
	if (pwm_request_state == false) {
		lcd_pwm = d4_ht_pwm_request(LCD_PWM_CH, "pwmdev");
		if ((lcd_pwm == -16) || (lcd_pwm == -2))
			return;
		pwm_request_state = true;
   }
	d4_ht_pwm_config(lcd_pwm, &pwm_setup);
	d4_ht_pwm_enable(lcd_pwm);
}


static void mlcd_1_ctrl_reset(void)
{
	/**< reset */
	mlcd_1_panel_set_gpio(LPM_GPIO_RESET, LPM_OFF);
	mdelay(1);
	mlcd_1_panel_set_gpio(LPM_GPIO_RESET, LPM_ON);
	mdelay(10);

	mlcd_1_ctrl_bl_on();     //for_PWM
}

static void mlcd_1_ctrl_init(void)
{
	mlcd_1_panel_spi_init();

	mlcd_1_ctrl_initializing_sequence();

	mlcd_1_ctrl_sleep_out();	

	mlcd_1_ctrl_vnm_load_sequence();
	
	mlcd_1_ctrl_display_on();
}

static void mlcd_1_ctrl_initializing_sequence(void)
{
	mlcd_1_set_table_value(panel_init_set_address_mode_table, CALC_PARAM_CNT(panel_init_set_address_mode_table));
	mlcd_1_set_table_value(panel_init_manufacturer_command_table, CALC_PARAM_CNT(panel_init_manufacturer_command_table));
	mlcd_1_set_table_value(panel_init_panel_driving_table, CALC_PARAM_CNT(panel_init_panel_driving_table));
	mlcd_1_set_table_value(panel_init_score_control_table, CALC_PARAM_CNT(panel_init_score_control_table));
	mlcd_1_set_table_value(panel_init_gate_interface_table, CALC_PARAM_CNT(panel_init_gate_interface_table));
	mlcd_1_set_table_value(panel_init_display_h_timming_table, CALC_PARAM_CNT(panel_init_display_h_timming_table));
	mlcd_1_set_table_value(panel_init_rgb_sync_option_table, CALC_PARAM_CNT(panel_init_rgb_sync_option_table));
	mlcd_1_set_table_value(panel_init_gamma_set_red_table, CALC_PARAM_CNT(panel_init_gamma_set_red_table));
	mlcd_1_set_table_value(panel_init_gamma_set_green_table, CALC_PARAM_CNT(panel_init_gamma_set_green_table));
	mlcd_1_set_table_value(panel_init_gamma_set_blue_table, CALC_PARAM_CNT(panel_init_gamma_set_blue_table));
	mlcd_1_set_table_value(panel_init_bias_current_control_table, CALC_PARAM_CNT(panel_init_bias_current_control_table));
	mlcd_1_set_table_value(panel_init_ddv_control_table, CALC_PARAM_CNT(panel_init_ddv_control_table));
	mlcd_1_set_table_value(panel_init_gamma_control_ref_table, CALC_PARAM_CNT(panel_init_gamma_control_ref_table)); 
	mlcd_1_set_table_value(panel_init_dcdc_control_table, CALC_PARAM_CNT(panel_init_dcdc_control_table));
	mlcd_1_set_table_value(panel_init_vcl_control_table, CALC_PARAM_CNT(panel_init_vcl_control_table));
}

static void mlcd_1_ctrl_vnm_load_sequence(void)
{
	mlcd_1_set_table_value(nvm_load1_table, CALC_PARAM_CNT(nvm_load1_table));
	mlcd_1_set_table_value(nvm_load2_table, CALC_PARAM_CNT(nvm_load2_table));
	mdelay(120);
}

static void mlcd_1_ctrl_display_on(void)
{
	unsigned short set_value = 0;
	
	set_value = 0x029;
	mlcd_1_set_table_value(&set_value, 0);	

	d4_ht_pwm_enable(lcd_pwm);	//for_PWM
}

static void mlcd_1_ctrl_display_off(void)
{
	unsigned short set_value = 0;

	set_value = 0x028;
	mlcd_1_set_table_value(&set_value, 0);

	d4_ht_pwm_disable(lcd_pwm);	//for_PWM
}

static void mlcd_1_ctrl_sleep_in(void)
{
	unsigned short set_value = 0;

	set_value = 0x010;
	mlcd_1_set_table_value(&set_value, 0);
	mdelay(120);
}

static void mlcd_1_ctrl_sleep_out(void)
{
	unsigned short set_value = 0;

	set_value = 0x011;
	mlcd_1_set_table_value(&set_value, 0);
	mdelay(10);
}

static void mlcd_1_ctrl_standby_on(void)
{
	mlcd_1_ctrl_sleep_in();		
	mlcd_1_ctrl_display_off();
}

static void mlcd_1_ctrl_standby_off(void)
{
	mlcd_1_ctrl_initializing_sequence();
	mlcd_1_ctrl_display_on();

	mlcd_1_ctrl_sleep_out();
	mdelay(120);	
}

static void mlcd_1_ctrl_power_off(void)
{
	mlcd_1_ctrl_standby_on();
	mlcd_1_panel_set_gpio(LPM_GPIO_POWER_1, LPM_OFF);
}

static void mlcd_1_ctrl_brightness(int level)
{	
	switch(level)
	{
		case LPM_BLACK_LEVEL:
			pwm_setup.freq1.duty = WVGA_SMD_369_BLACK;
			break;
		case LPM_DARK_LEVEL:
			pwm_setup.freq1.duty = WVGA_SMD_369_DARK;
			break;
		case LPM_NORMAL_DARK_LEVEL:
			pwm_setup.freq1.duty = WVGA_SMD_369_NORMAL_DARK;
			break;
		case LPM_NORMAL_LEVEL:
			pwm_setup.freq1.duty = WVGA_SMD_369_NORMAL;
			break;
		case LPM_NORMAL_BRIGHT_LEVEL:
			pwm_setup.freq1.duty = WVGA_SMD_369_BRIGHT_NORMAL;
			break;
		case LPM_BRIGHT_LEVEL:
			pwm_setup.freq1.duty = WVGA_SMD_369_BRIGHT;
			break;
		default:
			break;
	}
	
	d4_ht_pwm_disable(lcd_pwm);
	d4_ht_pwm_config(lcd_pwm, &pwm_setup);
	d4_ht_pwm_enable(lcd_pwm);
}

static void mlcd_1_ctrl_tcc(void *tcc_info)
{
    mlcd_tcc_manual_set(tcc_info);	
}

static void mlcd_1_ctrl_resume(void)
{
//	is_wvga_lcd_resume_case = 1;
}

static void mlcd_1_ctrl_csc(void *csc_info)
{
	unsigned int set_value = 0;
	struct stlcd_csc_matrix matrix;
	
	unsigned short pCsc[9] = {
        0x017,	0x1E0,	0x009,
        0xFE9,	0xFD3,	0x1BC,
        0x1CA,	0xFD3,	0xFF7,
    };
	
	unsigned short x_axis = 0;
	unsigned short y_axis = 0;

	set_value = (unsigned int)csc_info;
	
	x_axis = set_value >> 16;
	y_axis = set_value & 0x00000FF;
	
    matrix.matrix_set_00 = pCsc[0];
	matrix.matrix_set_01 = (unsigned short)((int)pCsc[1] + iColorAdjustOffset[x_axis][y_axis][1]);
    matrix.matrix_set_02 = pCsc[2];
	matrix.matrix_set_10 = pCsc[3];
	matrix.matrix_set_11 = pCsc[4];
	matrix.matrix_set_12 = (unsigned short)((int)pCsc[5] + iColorAdjustOffset[x_axis][y_axis][2]);
	matrix.matrix_set_20 = (unsigned short)((int)pCsc[6] + iColorAdjustOffset[x_axis][y_axis][0]);
	matrix.matrix_set_21 = pCsc[7];
	matrix.matrix_set_22 = pCsc[8];
	matrix.ycbcr_offset_y = 0x00; 
	matrix.ycbcr_offset_cb = 0x00;
	matrix.ycbcr_offset_cr = 0x00;
	matrix.rgb_upper_range = 0xFF;
	matrix.rgb_lower_range= 0x00;

	lcd_csc_manual_set(&matrix);	
}

static void mlcd_tcc_manual_set(void *tcc_info){

    
    int mode = (int)tcc_info;
	struct stlcd_tcc *tcc;
    
	tcc = kmalloc(sizeof(struct stlcd_tcc), GFP_KERNEL);
	if (!tcc)
		return;

    tcc->onoff = DP_ON;
    tcc->video_only = DP_OFF;
    
    switch(mode){
       // case 1: memcpy(tcc->TCC_TABLE,uiTcc_Gamma_Movie,sizeof(uiTcc_Gamma_Movie)); // Movie
       //         break; 
        case 0: 
        default:memcpy(tcc->TCC_TABLE,uiTcc_Gamma,sizeof(uiTcc_Gamma)); //Normal        
                break;
    }
    
    d4_dp_lcd_tcc_set(*tcc);
	kfree(tcc);   
}

static void mlcd_1_ctrl_extension(void *ext_info)
{
	struct lcd_panel_ext_ctrl_info *info;

	info = (struct lcd_panel_ext_ctrl_info *)ext_info;
	printk("Info1: %#010x, Info2: %d \n", info->ctrl_info_1, info->ctrl_info_2);
	printk("Comment: %s \n", info->comment);

	if(info->ctrl_info_1 == 0x9000)
	{
		printk("Info2: %d \n", info->ctrl_info_2);
		pwm_setup.freq1.duty = info->ctrl_info_2;
		d4_ht_pwm_disable(lcd_pwm);
		d4_ht_pwm_config(lcd_pwm, &pwm_setup);
		d4_ht_pwm_enable(lcd_pwm);
	}
}



/******************************************************************************/
/*                  		    Private Functions                        	  */
/******************************************************************************/

static int mlcd_1_set_table_value(const unsigned short *wbuf, unsigned int param_cnt)
{
	int ret = 0;
	struct lcd_panel_spi_write_info spi_set_info;

	spi_set_info.data_cnt = (param_cnt + 1);
	spi_set_info.data_buf = (void *)wbuf;

	ret = mlcd_1_panel_spi_write(&spi_set_info);
	udelay(10);

	return ret;
}

static int mlcd_1_register(void)
{
	unsigned long pin_config = 0;
	int ret = -1;

	/**< Data strength를  높이기 위한 임시 코드, for NX1 proto board */
	pin_config = to_config_packed(PIN_CONFIG_DRIVE_STRENGTH,
			PIN_CONFIG_DRIVE_STRENGTH_X6);
	ret = pin_config_group_set("drime4-pinmux", "mlcdcon", pin_config);
	if (ret < 0) {
		printk("Data strength control: Fail \n");
		return ret;
	}

	mlcd_1_panel_set_spi_init_config(&spi_config);
	
	if (mlcd_1_panel_set_info(&mlcd_1_panel_info) < 0) {
		return -EINVAL;
	}

	printk("Main LCD 1 panel register \n");
	
#if defined(CONFIG_DRM_DRIME4)		/* for drm test */
	mlcd_panel_select(LPM_MLCD_1_SELECT);
	mlcd_panel_power_on();
	mlcd_panel_reset();
	mlcd_panel_init();
	mlcd_panel_light_on();
#endif

	return 0;
}

static void mlcd_1_unregister(void)
{
	printk("Main LCD 1 panel unregister \n");
}

#ifndef CONFIG_SCORE_FAST_RESUME
module_init(mlcd_1_register);
#else
fast_late_initcall(mlcd_1_register);
#endif
module_exit(mlcd_1_unregister)




