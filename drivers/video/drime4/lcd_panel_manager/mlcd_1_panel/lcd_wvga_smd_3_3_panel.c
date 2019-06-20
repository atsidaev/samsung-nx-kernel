/**
 * @file lcd_vga_cmd_3_0panel.c
 * @brief SMD VGA 3.0 AMOLED LCD panel driver.
 * @author Kim Sunghoon <bluesay.kim@samsung.com>
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
#include <linux/pinctrl/pinconf-drime4.h>
#include <linux/pinctrl/pinconf.h>
#include <linux/pinctrl/consumer.h>
#include <mach/version_information.h>
#include <video/drime4/d4_dp_type.h>
#include <video/drime4/d4_dp_ioctl.h>
#include "../../lcd/d4_dp_lcd_dd.h"
#include "../../lcd_panel_manager/d4_lcd_panel_manager_ctrl.h"

#define CALC_PARAM_CNT(x)	((sizeof(x) / 2) - 1)

/* Private variable */
static unsigned long is_wvga_lcd_resume_case;

/**< LCD panel control value */

static const unsigned short panel_condition_set_table[] = {
		0x0F8,
		0x101, 0x127, 0x127, 0x107, 0x107, 0x154, 0x19F, 0x163, 0x186, 0x11A,
		0x133, 0x10D, 0x100, 0x100
};

static const unsigned short display_condition_1_set_table[] = {
		0x0F2,
		0x102, 0x103, 0x136, 0x11C, 0x110 /**< VBP:3, VHB:54, HBP:28, HFP:16 */
};

static const unsigned short display_condition_2_set_table[] = {
		0x0F7,
		0x100, 0x100, 0x120
};

static const unsigned short gamma_set_table[] = {
		0x0FA,
		0x102, 0x117, 0x113, 0x115,	0x1CF, 0x1C9, 0x1C7, 0x1CA,	0x1CC, 0x1C4, 0x1BE,
        0x1BF, 0x1BA, 0x1C8, 0x1CA,	0x1C5, 0x100, 0x1B5, 0x100,	0x1AB, 0x100, 0x1D5
};

static const unsigned short gamma_set_brightness_table[6][23] = {
       //1st ver. by jungmi.yun
	/*{   0x0FA, //80ccd
        0x102, 0x117, 0x113, 0x115,	0x1CF, 0x1C9, 0x1C7, 0x1CA,	0x1CC, 0x1C4, 0x1BE,
        0x1BF, 0x1BA, 0x1C8, 0x1CA,	0x1C5, 0x100, 0x17B, 0x100,	0x171, 0x100, 0x190 },
	{   0x0FA, //180ccd
        0x102, 0x117, 0x113, 0x115,	0x1CF, 0x1C9, 0x1C7, 0x1CA,	0x1CC, 0x1C4, 0x1BE,
        0x1BF, 0x1BA, 0x1C8, 0x1CA,	0x1C5, 0x100, 0x1A0, 0x100,	0x196, 0x100, 0x1BD },
	{   0x0FA, //210ccd
        0x102, 0x117, 0x113, 0x115,	0x1CF, 0x1C9, 0x1C7, 0x1CA,	0x1CC, 0x1C4, 0x1BE,
        0x1BF, 0x1BA, 0x1C8, 0x1CA,	0x1C5, 0x100, 0x1A7, 0x100,	0x19D, 0x100, 0x1C6 }, 
	{   0x0FA, //240ccd 
        0x102, 0x117, 0x113, 0x115,	0x1CF, 0x1C9, 0x1C7, 0x1CA,	0x1CC, 0x1C4, 0x1BE,	
        0x1BF, 0x1BA, 0x1C8, 0x1CA,	0x1C5, 0x100, 0x1AF, 0x100,	0x1A5, 0x100, 0x1CF }, 
	{   0x0FA, //270ccd 
        0x102, 0x117, 0x113, 0x115,	0x1CF, 0x1C9, 0x1C7, 0x1CA,	0x1CC, 0x1C4, 0x1BE,
        0x1BF, 0x1BA, 0x1C8, 0x1CA,	0x1C5, 0x100, 0x1B5, 0x100,	0x1AB, 0x100, 0x1D5 }, 
	{   0x0FA, //300ccd 
        0x102, 0x117, 0x113, 0x115,	0x1CF, 0x1C9, 0x1C7, 0x1CA,	0x1CC, 0x1C4, 0x1BE,
        0x1BF, 0x1BA, 0x1C8, 0x1CA,	0x1C5, 0x100, 0x1BD, 0x100,	0x1B3, 0x100, 0x1DF	}*/

       //2nd ver. brightness up by jungmi.yun
	{   0x0FA, //140ccd
        0x102, 0x117, 0x113, 0x115,	0x1CF, 0x1CC, 0x1C9, 0x1CB,	0x1CE, 0x1C4, 0x1CB,
        0x1C7, 0x1C4, 0x1D0, 0x1D4,	0x1CF, 0x100, 0x18E, 0x100,	0x18B, 0x100, 0x1AB },
	{   0x0FA, //210ccd
        0x102, 0x117, 0x113, 0x115,	0x1CF, 0x1C9, 0x1C7, 0x1CA,	0x1CC, 0x1C4, 0x1BE,
        0x1BF, 0x1BA, 0x1C8, 0x1CA,	0x1C5, 0x100, 0x1A7, 0x100,	0x19D, 0x100, 0x1C6 },
	{   0x0FA, //240ccd 
        0x102, 0x117, 0x113, 0x115,	0x1CF, 0x1C9, 0x1C7, 0x1CA,	0x1CC, 0x1C4, 0x1BE,	
        0x1BF, 0x1BA, 0x1C8, 0x1CA,	0x1C5, 0x100, 0x1AF, 0x100,	0x1A5, 0x100, 0x1CF }, 
	{   0x0FA, //270ccd 
        0x102, 0x117, 0x113, 0x115,	0x1CF, 0x1C9, 0x1C7, 0x1CA,	0x1CC, 0x1C4, 0x1BE,
        0x1BF, 0x1BA, 0x1C8, 0x1CA,	0x1C5, 0x100, 0x1B5, 0x100,	0x1AB, 0x100, 0x1D5 }, 
	{   0x0FA, //280ccd 
        0x102, 0x117, 0x113, 0x115,	0x1CF, 0x1C9, 0x1C7, 0x1CA,	0x1CC, 0x1C4, 0x1BE,
        0x1BF, 0x1BA, 0x1C8, 0x1CA,	0x1C5, 0x100, 0x1B8, 0x100,	0x1AD, 0x100, 0x1D8 },
	{   0x0FA, //300ccd 
        0x102, 0x117, 0x113, 0x115,	0x1CF, 0x1C9, 0x1C7, 0x1CA,	0x1CC, 0x1C4, 0x1BE,
        0x1BF, 0x1BA, 0x1C8, 0x1CA,	0x1C5, 0x100, 0x1BD, 0x100,	0x1B3, 0x100, 0x1DF	}       
};

static const unsigned short gamma_update_table[] = {
		0x0FA,
		0x103
};

static const unsigned short etc_condition_1_table[] = {
		0x0F6,
		0x100, 0x18E, 0x10F
};

static const unsigned short etc_condition_2_table[] = {
		0x0B3,
		0x10C
};

static const unsigned short etc_condition_3_table[] = {
		0x0B5,
		0x108, 0x108, 0x108, 0x108, 0x110, 0x110, 0x120, 0x120, 0x12E, 0x119,
		0x12E, 0x127, 0x121, 0x11E, 0x11A, 0x119, 0x12C, 0x127, 0x124, 0x121,
		0x13B, 0x134, 0x130, 0x12C, 0x128, 0x126, 0x124, 0x122, 0x121, 0x11F,
		0x11E, 0x11C
};

static const unsigned short etc_condition_4_table[] = {
		0x0B6,
		0x100, 0x100, 0x111, 0x122, 0x133, 0x144, 0x144, 0x144, 0x155, 0x155,
		0x166, 0x166, 0x166, 0x166, 0x166, 0x166
};

static const unsigned short etc_condition_5_table[] = {
		0x0B7,
		0x108, 0x108, 0x108, 0x108, 0x110, 0x110, 0x120, 0x120, 0x12E, 0x119,
		0x12E, 0x127, 0x121, 0x11E, 0x11A, 0x119, 0x12C, 0x127, 0x124, 0x121,
		0x13B, 0x134, 0x130, 0x12C, 0x128, 0x126, 0x124, 0x122, 0x121, 0x11F,
		0x11E, 0x11C
};

static const unsigned short etc_condition_6_table[] = {
		0x0B8,
		0x100, 0x100, 0x111, 0x122, 0x133, 0x144, 0x144, 0x144, 0x155, 0x155,
		0x166, 0x166, 0x166, 0x166, 0x166, 0x166
};

static const unsigned short etc_condition_7_table[] = {
		0x0B9,
		0x108, 0x108, 0x108, 0x108, 0x110, 0x110, 0x120, 0x120, 0x12E, 0x119,
		0x12E, 0x127, 0x121, 0x11E, 0x11A, 0x119, 0x12C, 0x127, 0x124, 0x121,
		0x13B, 0x134, 0x130, 0x12C, 0x128, 0x126, 0x124, 0x122, 0x121, 0x11F,
		0x11E, 0x11C
};

static const unsigned short etc_condition_8_table[] = {
		0x0BA,
		0x100, 0x100, 0x111, 0x122, 0x133, 0x144, 0x144, 0x144, 0x155, 0x155,
		0x166, 0x166, 0x166, 0x166, 0x166, 0x166
};

static const unsigned short dynamic_elvss_on[] = {
		0x0B1,
		0x10D
};

static const unsigned short dynamic_elvss_off[] = {
		0x0B1,
		0x10A
};
static unsigned int uiTcc_Gamma[256] = {
    0x000A0A0A, 0x010B0B0B, 0x020D0D0D, 0x030E0E0E, 0x04101010, 0x05111111, 0x06121212, 0x07131313, 0x08141414, 0x09151515, 0x0A161616, 0x0B171717, 0x0C181818, 0x0D191919, 0x0E1A1A1A, 0x0F1B1B1B,
    0x101B1B1B, 0x111C1C1C, 0x121D1D1D, 0x131E1E1E, 0x141F1F1F, 0x15202020, 0x16202020, 0x17212121, 0x18222222, 0x19232323, 0x1A232323, 0x1B242424, 0x1C252525, 0x1D262626, 0x1E272727, 0x1F272727,
    0x20282828, 0x21292929, 0x222A2A2A, 0x232B2B2B, 0x242C2C2C, 0x252D2D2D, 0x262D2D2D, 0x272E2E2E, 0x282F2F2F, 0x29303030, 0x2A313131, 0x2B323232, 0x2C333333, 0x2D343434, 0x2E353535, 0x2F363636,
    0x30373737, 0x31383838, 0x32393939, 0x333A3A3A, 0x343B3B3B, 0x353C3C3C, 0x363D3D3D, 0x373E3E3E, 0x383F3F3F, 0x39404040, 0x3A414141, 0x3B414141, 0x3C424242, 0x3D434343, 0x3E444444, 0x3F454545,
    0x40464646, 0x41474747, 0x42484848, 0x43494949, 0x444A4A4A, 0x454B4B4B, 0x464C4C4C, 0x474D4D4D, 0x484E4E4E, 0x494F4F4F, 0x4A505050, 0x4B515151, 0x4C525252, 0x4D535353, 0x4E545454, 0x4F565656,
    0x50575757, 0x51585858, 0x52595959, 0x535A5A5A, 0x545B5B5B, 0x555C5C5C, 0x565D5D5D, 0x575E5E5E, 0x585F5F5F, 0x59606060, 0x5A616161, 0x5B626262, 0x5C636363, 0x5D646464, 0x5E656565, 0x5F666666,
    0x60676767, 0x61686868, 0x62696969, 0x636A6A6A, 0x646B6B6B, 0x656C6C6C, 0x666D6D6D, 0x676E6E6E, 0x686F6F6F, 0x69707070, 0x6A717171, 0x6B727272, 0x6C737373, 0x6D747474, 0x6E767676, 0x6F777777,
    0x70787878, 0x71797979, 0x727A7A7A, 0x737B7B7B, 0x747C7C7C, 0x757D7D7D, 0x767E7E7E, 0x777F7F7F, 0x78808080, 0x79818181, 0x7A828282, 0x7B838383, 0x7C848484, 0x7D858585, 0x7E868686, 0x7F878787,
    0x80888888, 0x81898989, 0x828A8A8A, 0x838B8B8B, 0x848D8D8D, 0x858E8E8E, 0x868F8F8F, 0x87909090, 0x88919191, 0x89929292, 0x8A939393, 0x8B949494, 0x8C959595, 0x8D969696, 0x8E979797, 0x8F989898,
    0x90999999, 0x919A9A9A, 0x929B9B9B, 0x939C9C9C, 0x949D9D9D, 0x959E9E9E, 0x969F9F9F, 0x97A0A0A0, 0x98A1A1A1, 0x99A2A2A2, 0x9AA4A4A4, 0x9BA5A5A5, 0x9CA6A6A6, 0x9DA7A7A7, 0x9EA8A8A8, 0x9FA9A9A9,
    0xA0AAAAAA, 0xA1ABABAB, 0xA2ACACAC, 0xA3ADADAD, 0xA4AEAEAE, 0xA5AFAFAF, 0xA6B1B1B1, 0xA7B2B2B2, 0xA8B3B3B3, 0xA9B4B4B4, 0xAAB5B5B5, 0xABB6B6B6, 0xACB7B7B7, 0xADB8B8B8, 0xAEB9B9B9, 0xAFBABABA,
    0xB0BCBCBC, 0xB1BDBDBD, 0xB2BEBEBE, 0xB3BFBFBF, 0xB4C0C0C0, 0xB5C1C1C1, 0xB6C2C2C2, 0xB7C3C3C3, 0xB8C5C5C5, 0xB9C6C6C6, 0xBAC7C7C7, 0xBBC8C8C8, 0xBCC9C9C9, 0xBDCACACA, 0xBECBCBCB, 0xBFCCCCCC,
    0xC0CDCDCD, 0xC1CFCFCF, 0xC2D0D0D0, 0xC3D1D1D1, 0xC4D2D2D2, 0xC5D3D3D3, 0xC6D4D4D4, 0xC7D5D5D5, 0xC8D6D6D6, 0xC9D7D7D7, 0xCAD8D8D8, 0xCBD9D9D9, 0xCCDADADA, 0xCDDBDBDB, 0xCEDDDDDD, 0xCFDEDEDE,
    0xD0DFDFDF, 0xD1E0E0E0, 0xD2E1E1E1, 0xD3E2E2E2, 0xD4E3E3E3, 0xD5E4E4E4, 0xD6E4E4E4, 0xD7E5E5E5, 0xD8E6E6E6, 0xD9E7E7E7, 0xDAE8E8E8, 0xDBE9E9E9, 0xDCEAEAEA, 0xDDEBEBEB, 0xDEECECEC, 0xDFECECEC,
    0xE0EDEDED, 0xE1EEEEEE, 0xE2EFEFEF, 0xE3EFEFEF, 0xE4F0F0F0, 0xE5F1F1F1, 0xE6F2F2F2, 0xE7F2F2F2, 0xE8F3F3F3, 0xE9F4F4F4, 0xEAF4F4F4, 0xEBF5F5F5, 0xECF5F5F5, 0xEDF6F6F6, 0xEEF7F7F7, 0xEFF7F7F7,
    0xF0F8F8F8, 0xF1F8F8F8, 0xF2F9F9F9, 0xF3F9F9F9, 0xF4FAFAFA, 0xF5FAFAFA, 0xF6FBFBFB, 0xF7FBFBFB, 0xF8FCFCFC, 0xF9FCFCFC, 0xFAFDFDFD, 0xFBFDFDFD, 0xFCFEFEFE, 0xFDFEFEFE, 0xFEFFFFFF, 0xFFFFFFFF
};

static unsigned int uiTcc_Gamma_Movie[256] = {
    0x000A0A0A, 0x010A0A0A, 0x020A0A0A, 0x030A0A0A, 0x040A0A0A, 0x050A0A0A, 0x060A0A0A, 0x070A0A0A, 0x080A0A0A, 0x090A0A0A, 0x0A0A0A0A, 0x0B0A0A0A, 0x0C0A0A0A, 0x0D0A0A0A, 0x0E0A0A0A, 0x0F0A0A0A,
    0x100A0A0A, 0x110B0B0B, 0x120C0C0C, 0x130D0D0D, 0x140E0E0E, 0x15101010, 0x16111111, 0x17121212, 0x18131313, 0x19141414, 0x1A151515, 0x1B161616, 0x1C171717, 0x1D191919, 0x1E1A1A1A, 0x1F1B1B1B,
    0x201C1C1C, 0x211D1D1D, 0x221E1E1E, 0x231F1F1F, 0x24202020, 0x25212121, 0x26232323, 0x27242424, 0x28252525, 0x29262626, 0x2A272727, 0x2B282828, 0x2C292929, 0x2D2A2A2A, 0x2E2C2C2C, 0x2F2D2D2D,
    0x302E2E2E, 0x312F2F2F, 0x32303030, 0x33313131, 0x34323232, 0x35333333, 0x36353535, 0x37363636, 0x38373737, 0x39383838, 0x3A393939, 0x3B3A3A3A, 0x3C3B3B3B, 0x3D3C3C3C, 0x3E3D3D3D, 0x3F3F3F3F,
    0x40404040, 0x41414141, 0x42424242, 0x43434343, 0x44444444, 0x45454545, 0x46464646, 0x47484848, 0x48494949, 0x494A4A4A, 0x4A4B4B4B, 0x4B4C4C4C, 0x4C4D4D4D, 0x4D4E4E4E, 0x4E4F4F4F, 0x4F505050,
    0x50525252, 0x51535353, 0x52545454, 0x53555555, 0x54565656, 0x55575757, 0x56585858, 0x57595959, 0x585B5B5B, 0x595C5C5C, 0x5A5D5D5D, 0x5B5E5E5E, 0x5C5F5F5F, 0x5D606060, 0x5E616161, 0x5F626262,
    0x60636363, 0x61656565, 0x62666666, 0x63676767, 0x64686868, 0x65696969, 0x666A6A6A, 0x676B6B6B, 0x686C6C6C, 0x696E6E6E, 0x6A6F6F6F, 0x6B707070, 0x6C717171, 0x6D727272, 0x6E737373, 0x6F747474,
    0x70757575, 0x71777777, 0x72787878, 0x73797979, 0x747A7A7A, 0x757B7B7B, 0x767C7C7C, 0x777D7D7D, 0x787E7E7E, 0x797F7F7F, 0x7A818181, 0x7B828282, 0x7C838383, 0x7D848484, 0x7E858585, 0x7F868686,
    0x80878787, 0x81888888, 0x828A8A8A, 0x838B8B8B, 0x848C8C8C, 0x858D8D8D, 0x868E8E8E, 0x878F8F8F, 0x88909090, 0x89919191, 0x8A929292, 0x8B949494, 0x8C959595, 0x8D969696, 0x8E979797, 0x8F989898,
    0x90999999, 0x919A9A9A, 0x929B9B9B, 0x939D9D9D, 0x949E9E9E, 0x959F9F9F, 0x96A0A0A0, 0x97A1A1A1, 0x98A2A2A2, 0x99A3A3A3, 0x9AA4A4A4, 0x9BA6A6A6, 0x9CA7A7A7, 0x9DA8A8A8, 0x9EA9A9A9, 0x9FAAAAAA,
    0xA0ABABAB, 0xA1ACACAC, 0xA2ADADAD, 0xA3AEAEAE, 0xA4B0B0B0, 0xA5B1B1B1, 0xA6B2B2B2, 0xA7B3B3B3, 0xA8B4B4B4, 0xA9B5B5B5, 0xAAB6B6B6, 0xABB7B7B7, 0xACB9B9B9, 0xADBABABA, 0xAEBBBBBB, 0xAFBCBCBC,
    0xB0BDBDBD, 0xB1BEBEBE, 0xB2BFBFBF, 0xB3C0C0C0, 0xB4C1C1C1, 0xB5C3C3C3, 0xB6C4C4C4, 0xB7C5C5C5, 0xB8C6C6C6, 0xB9C7C7C7, 0xBAC8C8C8, 0xBBC9C9C9, 0xBCCACACA, 0xBDCCCCCC, 0xBECDCDCD, 0xBFCECECE,
    0xC0CFCFCF, 0xC1D0D0D0, 0xC2D1D1D1, 0xC3D2D2D2, 0xC4D3D3D3, 0xC5D4D4D4, 0xC6D6D6D6, 0xC7D7D7D7, 0xC8D8D8D8, 0xC9D9D9D9, 0xCADADADA, 0xCBDBDBDB, 0xCCDCDCDC, 0xCDDDDDDD, 0xCEDFDFDF, 0xCFE0E0E0,
    0xD0E1E1E1, 0xD1E2E2E2, 0xD2E3E3E3, 0xD3E4E4E4, 0xD4E5E5E5, 0xD5E6E6E6, 0xD6E8E8E8, 0xD7E9E9E9, 0xD8EAEAEA, 0xD9EBEBEB, 0xDAECECEC, 0xDBEDEDED, 0xDCEEEEEE, 0xDDEFEFEF, 0xDEF0F0F0, 0xDFF2F2F2,
    0xE0F3F3F3, 0xE1F4F4F4, 0xE2F5F5F5, 0xE3F6F6F6, 0xE4F7F7F7, 0xE5F8F8F8, 0xE6F9F9F9, 0xE7FBFBFB, 0xE8FCFCFC, 0xE9FDFDFD, 0xEAFEFEFE, 0xEBFFFFFF, 0xECFFFFFF, 0xEDFFFFFF, 0xEEFFFFFF, 0xEFFFFFFF,
    0xF0FFFFFF, 0xF1FFFFFF, 0xF2FFFFFF, 0xF3FFFFFF, 0xF4FFFFFF, 0xF5FFFFFF, 0xF6FFFFFF, 0xF7FFFFFF, 0xF8FFFFFF, 0xF9FFFFFF, 0xFAFFFFFF, 0xFBFFFFFF, 0xFCFFFFFF, 0xFDFFFFFF, 0xFEFFFFFF, 0xFFFFFFFF,
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
static void mlcd_1_ctrl_power_off(void);
static void mlcd_1_ctrl_brightness(int level);
static void mlcd_1_ctrl_tcc(void *tcc_info);
static void mlcd_1_ctrl_csc(void *csc_info);
static void mlcd_tcc_manual_set(void *tcc_info);
static void mlcd_1_ctrl_resume(void);

/******************************************************************************/
/*                     Main LCD 1 Panel Information                           */
/******************************************************************************/

struct lcd_panel_hw_info mlcd_1_panel_hw_info = {
 /**< width/ height/   LCD clock/       LCD ratio/  dot array type */
		480,	800,	27000000,	LPM_RATIO_16_9,	LPM_STRIPE,
 /**< dot even seq/ dot odd seq/ output format/ output bit-width */
		LPM_BGR,	LPM_BGR,	LPM_RGB_OUT,	LPM_BIT_WIDTH_24
};

struct lcd_panel_timing_info mlcd_1_panel_timing_info = {
 /**< total h/  total v/  h rising/ h falling/ v rising/ v falling */
		524,		857,		0,		10,			0,		2,

 /**< read h start/ enable h start/ enable h size/ enable v start/ enable v size */
		20,			28,			480,			3,			800,

 /**< inv_dot clk/ inv_enable/ inv_hsync/ inv_vsync */
	  LPM_OFF,		  LPM_ON,	    LPM_ON,		LPM_ON
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

		NULL,						/**< LCD horizontal flip on function */
		NULL,						/**< LCD horizontal flip off function */

		NULL,						/**< LCD vertical flip on function */
		NULL,						/**< LCD vertical flip off function */

		NULL,       				/**< LCD Rotation control function */
		NULL,  						/**< LCD panel brightness gamma setting function */
		mlcd_1_ctrl_csc,    		/**< LCD CSC control function */
		mlcd_1_ctrl_tcc,			/**< LCD TCC control function */
		NULL,						/**< LCD extension control function */

		NULL,						/**< suspend function for power management */
		mlcd_1_ctrl_resume			/**< resume  function for power management */
};

struct lcd_panel_manager_info mlcd_1_panel_info = {
    &mlcd_1_panel_hw_info,
    &mlcd_1_panel_timing_info,
    &mlcd_1_panel_ctrl_func,
    " VGA SMD 3.31 Inch AM-OLED Panel "
};

/******************************************************************************/
/*                  Main LCD 1 Panel Control Functions                        */
/******************************************************************************/

static void mlcd_1_ctrl_power_on(void)
{
	/**< for test */
	mlcd_1_panel_set_gpio(LPM_GPIO_POWER_1, LPM_ON);
	mdelay(25);
}

static void mlcd_1_ctrl_reset(void)
{
	/**< reset */
	mlcd_1_panel_set_gpio(LPM_GPIO_RESET, LPM_ON);
	mdelay(10);
	mlcd_1_panel_set_gpio(LPM_GPIO_RESET, LPM_OFF);
	mdelay(10);
	mlcd_1_panel_set_gpio(LPM_GPIO_RESET, LPM_ON);
	mdelay(10);
}

static void mlcd_1_ctrl_init(void)
{
	mlcd_1_panel_spi_init();

	mlcd_1_set_table_value(panel_condition_set_table, CALC_PARAM_CNT(panel_condition_set_table));

	mlcd_1_set_table_value(display_condition_1_set_table, CALC_PARAM_CNT(display_condition_1_set_table));
	mlcd_1_set_table_value(display_condition_2_set_table, CALC_PARAM_CNT(display_condition_2_set_table));

	// normal gamma setting
	mlcd_1_set_table_value(gamma_set_brightness_table[3], CALC_PARAM_CNT(gamma_set_brightness_table[3]));
	mlcd_1_set_table_value(gamma_update_table, CALC_PARAM_CNT(gamma_update_table));

	mlcd_1_set_table_value(etc_condition_1_table, CALC_PARAM_CNT(etc_condition_1_table));
	mlcd_1_set_table_value(etc_condition_2_table, CALC_PARAM_CNT(etc_condition_2_table));
	mlcd_1_set_table_value(etc_condition_3_table, CALC_PARAM_CNT(etc_condition_3_table));
	mlcd_1_set_table_value(etc_condition_4_table, CALC_PARAM_CNT(etc_condition_4_table));
	mlcd_1_set_table_value(etc_condition_5_table, CALC_PARAM_CNT(etc_condition_5_table));
	mlcd_1_set_table_value(etc_condition_6_table, CALC_PARAM_CNT(etc_condition_6_table));
	mlcd_1_set_table_value(etc_condition_7_table, CALC_PARAM_CNT(etc_condition_7_table));
	mlcd_1_set_table_value(etc_condition_8_table, CALC_PARAM_CNT(etc_condition_8_table));

	/**< ELVSS Setting */
	#if 1
		/**< ELVSS OFF */
	mlcd_1_set_table_value(dynamic_elvss_off, CALC_PARAM_CNT(dynamic_elvss_off));

	#else
		/**< ELVSS ON */
	mlcd_1_set_table_value(dynamic_elvss_210_300_table, CALC_PARAM_CNT(dynamic_elvss_210_300_table));
	mlcd_1_set_table_value(dynamic_elvss_on, CALC_PARAM_CNT(dynamic_elvss_on));
	#endif

	mdelay(10);
	mlcd_1_ctrl_standby_off();


	mdelay(10);
#if 0 // init at d4_dp_lcd_dd not here.
	/**< csc matrix init */
	int adjust_value = 0;
	short x_axis = 7;
	short y_axis = 7;
	adjust_value = ((x_axis << 16) | (y_axis & 0x0000FFFF));
	mlcd_1_ctrl_extension(adjust_value);

    /** tcc lut init */
    mlcd_tcc_manual_set();  
#endif
}

static void mlcd_1_ctrl_standby_on(void)
{
	unsigned short set_value = 0;

	set_value = 0x010;
	mlcd_1_set_table_value(&set_value, 0);
	mdelay(120);
}

static void mlcd_1_ctrl_standby_off(void)
{
#if 0	
	unsigned short set_value = 0;

	set_value = 0x011;
	mlcd_1_set_table_value(&set_value, 0);
	mdelay(120);
#endif	
}

static void mlcd_1_ctrl_display_on(void)
{
	unsigned short set_value = 0;
	if (is_wvga_lcd_resume_case) {
		is_wvga_lcd_resume_case = 0;
	} else {
		set_value = 0x011;
		mlcd_1_set_table_value(&set_value, 0);
		mdelay(120);
	}
	set_value = 0x029;
	mlcd_1_set_table_value(&set_value, 0);
}

static void mlcd_1_ctrl_display_off(void)
{
	unsigned short set_value = 0;

	set_value = 0x010;
	mlcd_1_set_table_value(&set_value, 0);
	mdelay(120);	
}

static void mlcd_1_ctrl_power_off(void)
{
	mlcd_1_panel_set_gpio(LPM_GPIO_POWER_1, LPM_OFF);
}

static void mlcd_1_ctrl_brightness(int level)
{
	mlcd_1_set_table_value(gamma_set_brightness_table[level], CALC_PARAM_CNT(gamma_set_brightness_table[level]));
	mlcd_1_set_table_value(gamma_update_table, CALC_PARAM_CNT(gamma_update_table));
}

static void mlcd_1_ctrl_tcc(void *tcc_info)
{
    mlcd_tcc_manual_set(tcc_info);	
}

static void mlcd_1_ctrl_resume(void)
{
	is_wvga_lcd_resume_case = 1;
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
        case 1: memcpy(tcc->TCC_TABLE,uiTcc_Gamma_Movie,sizeof(uiTcc_Gamma_Movie)); // Movie
                break; 
        case 0: 
        default:memcpy(tcc->TCC_TABLE,uiTcc_Gamma,sizeof(uiTcc_Gamma)); //Normal        
                break;
    }
    
    d4_dp_lcd_tcc_set(*tcc);
	kfree(tcc);
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

	printk("Main LCD 3 panel register \n");
	
#if defined(CONFIG_DRM_DRIME4)		/* for drm test */
	mlcd_panel_select(LPM_MLCD_1_SELECT);
	mlcd_panel_power_on();
	mlcd_panel_reset();
	mlcd_panel_init();
//		mlcd_panel_light_on();
#endif
	return 0;
}

static void mlcd_1_unregister(void)
{
	printk("Main LCD 3 panel unregister \n");
}

#ifndef CONFIG_SCORE_FAST_RESUME
module_init(mlcd_1_register);
#else
fast_late_initcall(mlcd_1_register);
#endif
module_exit(mlcd_1_unregister)



