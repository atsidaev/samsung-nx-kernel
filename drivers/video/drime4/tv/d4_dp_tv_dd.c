#include <linux/delay.h>

#include <asm/io.h>
#include <asm/uaccess.h>
#include <asm/page.h>
#include <linux/platform_device.h>
#include <mach/map.h>

#include "d4_dp_tv_reg.h"
#include "../global/d4_dp_global_reg.h"
#include "d4_dp_tv_dd.h"

#include "../global/d4_dp_global.h"
#include <linux/pinctrl/pinmux.h>
#include <asm/mmu_context.h>


struct stfb_info tv_info;
struct stfb_tv_video_info tv_video;
struct stfb_graphic_info tv_graphic;
struct i2c_client *i2client;
static enum edp_tv_mode g_mode; /* temp global */

static struct stdp_tv_filter_set hrz_filter_set[DP_BYPASS + 1] = {
		{ 0, 1, 2, 1, 0, 2 }, /* LPF1 */
		{ 0, 2, 3, 2, 0, 3 }, /* LPF2 */
		{ 1, 2, 2, 2, 1, 3 }, /* LPF3 */
		{ 0, -1, 3, -1, 0, 1 }, /* HPF1 */
		{ -1, -1, 4, -1, -1, 2 }, /* HPF2 */
		{ -1, -2, 4, -2, -1, 1 }, /* HPF3 */
		{ 0, 0, 1, 0, 0, 0 } /* Bypass */
};

static const enum edp_hdmi_clock phy_freq[] = { clock27, /*	NTSC=0 */
		clock27, /*	PAL */
		clock27, /*	RES_480P */
		clock27, /*	RES_576P */
		clock74_25, /*	RES_720P_60	*/
		clock74_176, /*	RES_720P_50	*/
		clock74_25, /*	RES_1080I_60 */
		clock74_176, /*	RES_1080I_50	*/
		clock148_5, /*	RES_1080P_60	*/
		clock148_352, /*	RES_1080P_50	*/
		clock74_25, /*	RES_1080p_30	*/
		clock74_25, /*	RES_1080P_24	*/
		clock148_5, /*	RES_720P_60_3D_FP	*/
		clock74_25, /*	1280x720p@60Hz_3D_SideHalf	*/
		clock74_25, /*	1280x720p@60Hz_3D_TOP	*/
		clock148_352, /*	RES_720_50_3D_FP (problem)	*/
		clock74_176, /*	1280x720p@50Hz_3D_SideHalf	*/
		clock74_176, /*	1280x720p@50Hz_3D_TOP	*/
		clock74_25, /*	RES_1080I_60_3D_SBS	*/
		clock74_176, /*	1920x1080i@50Hz_3D_SideHalf	*/
		clock148_5, /*	1920x1080p@60Hz_3D_TopBottom	*/
		clock148_352, /*	1920x1080p@50Hz_3D_TopBottom	*/
		clock148_5, /*	1920x1080p@30Hz_3D_FramePacking	*/
		clock74_25, /*	1920x1080p@30Hz_3D_TopBottom	*/
		clock148_5, /*	RES_1080P_24_3D_FP	*/
		clock74_25, /*	1920x1080p@24Hz_3D_SideHalf	*/
		clock74_25, /*	1920x1080p@24Hz_3D_TOP	*/
		clock27 };

static int M_TV_Mode = RES_1080I_60;

/* TV TG data table*/
static struct tv_tg tv_tg_table[res_max] = {
/*	NTSC	*/
{ 0x020C, 0x06B3, 0x0000, 0x0114, 0x0000, 0x0016, 0x0106, 0x011D, 0x0000,
		0x0003, 0x0109, 0x0000, 0x020C, 0x0010, 0x010c, 0x0116, 0x06B3, 0x0014,
		0x0000, 0x0114, 0x0105, 0x0016, 0x020c, 0x011D, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000 },
/*	PAL	*/
{ 0x0270, 0x06bf, 0x0000, 0x0120, 0x0000, 0x0018, 0x0138, 0x0151, 0x0000,
		0x0002, 0x013a, 0x0000, 0x0270, 0x0010, 0x0118, 0x014a, 0x06bf, 0x0014,
		0x0000, 0x0120, 0x0137, 0x0018, 0x0270, 0x0151, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000 },
/*	RES_480_60p	*/
{ 0x020c, 0x0359, 0x0000, 0x003e, 0x0006, 0x000c, 0x0006, 0x000c, 0x0000,
		0x0000, 0x0000, 0x0000, 0x020c, 0x0010, 0x0072, 0x0010, 0x0359, 0x0014,
		0x034a, 0x007a, 0x0209, 0x002a, 0x0209, 0x002a, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000 },
/*	RES_576_50p	*/
{ 0x0270, 0x035f, 0x0000, 0x0040, 0x0000, 0x0005, 0x0000, 0x0005, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0270, 0x0010, 0x007c, 0x0010, 0x035e, 0x0010,
		0x0354, 0x0084, 0x026b, 0x002c, 0x026b, 0x002c, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000 },
/*	RES_720P_60	*/
{ 0x02ED, 0x0671, 0x0000, 0x0028, 0x0000, 0x0005, 0x0000, 0x0005, 0x0000,
		0x0000, 0x0000, 0x0000, 0x02ED, 0x0010, 0x00fc, 0x0010, 0x0671, 0x0014,
		0x0604, 0x0104, 0x02E8, 0x0019, 0x02E8, 0x0019, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000 },
/*	RES_720_50p	*/
{ 0x02ED, 0x07BB, 0x0000, 0x0028, 0x0000, 0x0005, 0x0000, 0x0005, 0x0000,
		0x0000, 0x0000, 0x0000, 0x02ED, 0x0010, 0x00fc, 0x0010, 0x07BB, 0x0014,
		0x0604, 0x0104, 0x02E8, 0x0019, 0x02E8, 0x0019, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000 },
/*	RES_1080_60i	*/
{ 0x0464, 0x0897, 0x0000, 0x002c, 0x0000, 0x0005, 0x0232, 0x0237, 0x0232,
		0x0000, 0x0232, 0x044C, 0x0464, 0x0010, 0x00b8, 0x023a, 0x0897, 0x0014,
		0x0840, 0x00c0, 0x022f, 0x0014, 0x0462, 0x0247, 0x022F, 0x0461, 0x0000,
		0x0000, 0x0000 },
/*	RES_1080_50i	*/
{ 0x0464, 0x0A4F, 0x0000, 0x002c, 0x0000, 0x0005, /*0x0000, 0x0005*/
		0x0232, 0x0237, 0x0232,
		0x0000, 0x0232, 0x0528, 0x0464, 0x0010, 0x00b8, 0x023a, 0x0A4F,
		/*0x0010*/0x0014,
		0x0840, 0x00c0, 0x022f, 0x0014, 0x0462, 0x0247, 0x022F, 0x0461, 0x0000,
		0x0000, 0x0000 },
/*	RES_1080_60p	*/
{ 0x0464, 0x0897, 0x0000, 0x002c, 0x0000, 0x0005, 0x0000, 0x0005, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0464, 0x0010, 0x00b8, 0x0010, 0x0897, 0x0014,
		0x0840, 0x00c0, 0x0460, 0x0029, 0x0460, 0x0029, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000 },
/*	RES_1080_50P	*/
{ 0x0464, 0x0A4F, 0x0000, 0x002c, 0x0000, 0x0005, 0x0000, 0x0005, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0464, 0x0010, 0x00b8, 0x0010, 0x0A4F, 0x0014,
		0x0840, 0x00c0, 0x0460, 0x0029, 0x0460, 0x0029, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000 },
/*	RES_1080_30p	*/
{ 0x0464, 0x0897, 0x0000, 0x002c, 0x0000, 0x0005, 0x0000, 0x0005, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0464, 0x0010, 0x00b8, 0x0010, 0x0897, 0x0014,
		0x0840, 0x00c0, 0x0460, 0x0029, 0x0460, 0x0029, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000 },
/*	RES_1080_24P	*/
{ 0x0464, 0x0ABD, 0x0000, 0x002c, 0x0000, 0x0005, 0x0000, 0x0005, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0464, 0x0010, 0x00b8, 0x0010, 0x0A4F, 0x0014,
		0x0840, 0x00c0, 0x0460, 0x0029, 0x0460, 0x0029, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000 },

/*	3D	*/
/*	RES_720P_60_3D_FP	*/
{ 0x05db, 0x0671, 0x0000, 0x0028, 0x0000, 0x0005, 0x0000, 0x0005, 0x0000,
		0x0000, 0x0000, 0x0000, 0x05db, 0x0010, 0x00fc, 0x0010, 0x0671, 0x0010,
		0x0604, 0x0104, 0x05d6, 0x0019, 0x05d6, 0x0019, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000 },
/*	RES_720P_60_3D_SBS	*/
{ 0x02ED, 0x0671, 0x0000, 0x0028, 0x0000, 0x0005, 0x0000, 0x0005, 0x0000,
		0x0000, 0x0000, 0x0000, 0x02ED, 0x0010, 0x00fc, 0x0010, 0x0671, 0x0014,
		0x0604, 0x0104, 0x02E8, 0x0019, 0x02E8, 0x0019, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000 },
/*	RES_720P_60_3D_TB	*/
{ 0x02ED, 0x0671, 0x0000, 0x0028, 0x0000, 0x0005, 0x0000, 0x0005, 0x0000,
		0x0000, 0x0000, 0x0000, 0x02ED, 0x0010, 0x00fc, 0x0010, 0x0671, 0x0014,
		0x0604, 0x0104, 0x02E8, 0x0019, 0x02E8, 0x0019, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000 },
/*	RES_720P_50_3D_FP	*/
{ 0x05DB, 0x07BB, 0x0000, 0x0028, 0x0000, 0x0005, 0x0000, 0x0005, 0x0000,
		0x0000, 0x0000, 0x0000, 0x05DB, 0x0010, 0x0EB, 0x0010, 0x07BB, 0x0010,
		0x0604, 0x0104, 0x05d6, 0x0019, 0x05d6, 0x0019, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000 },
/*	RES_720P_50_3D_SBS	*/
{ 0x02ED, 0x07BB, 0x0000, 0x0028, 0x0000, 0x0005, 0x0000, 0x0005, 0x0000,
		0x0000, 0x0000, 0x0000, 0x02ED, 0x0010, 0x00fc, 0x0010, 0x07BB, 0x0014,
		0x0604, 0x0104, 0x02E8, 0x0019, 0x02E8, 0x0019, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000 },
/*	RES_720P_50_3D_TB	*/
{ 0x02ED, 0x07BB, 0x0000, 0x0028, 0x0000, 0x0005, 0x0000, 0x0005, 0x0000,
		0x0000, 0x0000, 0x0000, 0x02ED, 0x0010, 0x00fc, 0x0010, 0x07BB, 0x0014,
		0x0604, 0x0104, 0x02E8, 0x0019, 0x02E8, 0x0019, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000 },
/*	RES_1080I_60_3D_SBS	*/
{ 0x0464, 0x0897, 0x0000, 0x002c, 0x0000, 0x0005, 0x0232, 0x0237, 0x0232,
		0x0000, 0x0232, 0x044C, 0x0464, 0x0010, 0x00b8, 0x023a, 0x0897, 0x0014,
		0x0840, 0x00c0, 0x022f, 0x0014, 0x0462, 0x0247, 0x022F, 0x0461, 0x0000,
		0x0000, 0x0000 },
/*	RES_1080I_50_3D_SBS	*/
{ 0x0464, 0x0A4F, 0x0000, 0x002c, 0x0000, 0x0005, 0x0232, 0x0237, 0x0232,
		0x0000, 0x0232, 0x0528, 0x0464, 0x0010, 0x00b8, 0x023a, 0x0A4F, 0x0014,
		0x0840, 0x00c0, 0x022f, 0x0014, 0x0462, 0x0247, 0x022F, 0x0461, 0x0000,
		0x0000, 0x0000 },
/*	RES_1080P_60_3D_TB	*/
{ 0x0464, 0x0897, 0x0000, 0x002c, 0x0000, 0x0005, 0x0000, 0x0005, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0464, 0x0010, 0x00b8, 0x0010, 0x0897, 0x0014,
		0x0840, 0x00c0, 0x0460, 0x0029, 0x0460, 0x0029, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000 },
/*	RES_1080P_50_3D_TB	*/
{ 0x0464, 0x0A4F, 0x0000, 0x002c, 0x0000, 0x0005, 0x0000, 0x0005, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0464, 0x0010, 0x00b8, 0x0010, 0x0A4F, 0x0014,
		0x0840, 0x00c0, 0x0460, 0x0029, 0x0460, 0x0029, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000 },
/*	RES_1080P_30_3D_FP	*/
{ 0x08C9, 0x0897, 0x0000, 0x002C, 0x0000, 0x0005, 0x0000, 0x0005, 0x0000,
		0x0000, 0x0000, 0x0000, 0x08C9, 0x0010, 0x00B7, 0x0010, 0x0897, 0x0010,
		0x0840, 0x00C0, 0x08C5, 0x0029, 0x08C6, 0x0029, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000 },
/*	RES_1080P_30_3D_TB	*/
{ 0x0464, 0x0897, 0x0000, 0x002c, 0x0000, 0x0005, 0x0000, 0x0005, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0464, 0x0010, 0x00b8, 0x0010, 0x0897, 0x0014,
		0x0840, 0x00c0, 0x0460, 0x0029, 0x0460, 0x0029, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000 },
/*	RES_1080P_24_3D_FP	*/
{ 0x08C9, 0x0ABD, 0x0000, 0x002C, 0x0000, 0x0005, 0x0000, 0x0005, 0x0000,
		0x0000, 0x0000, 0x0000, 0x08C9, 0x0010, 0x00B8, 0x0010, 0x0A4F, 0x0014,
		0x0840, 0x00C0, 0x08C5, 0x0029, 0x08C6, 0x0029, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000 },
/*	RES_1080P_24_3D_SBS	*/
{ 0x0464, 0x0ABD, 0x0000, 0x002c, 0x0000, 0x0005, 0x0000, 0x0005, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0464, 0x0010, 0x00b8, 0x0010, 0x0A4F, 0x0014,
		0x0840, 0x00c0, 0x0460, 0x0029, 0x0460, 0x0029, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000 },
/*	RES_1080P_24_3D_TB	*/
{ 0x0464, 0x0ABD, 0x0000, 0x002c, 0x0000, 0x0005, 0x0000, 0x0005, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0464, 0x0010, 0x00b8, 0x0010, 0x0A4F, 0x0014,
		0x0840, 0x00c0, 0x0460, 0x0029, 0x0460, 0x0029, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000 },
/*	RES_1080P_30_3D_SBS	*/
{ 0x0464, 0x0897, 0x0000, 0x002c, 0x0000, 0x0005, 0x0000, 0x0005, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0464, 0x0010, 0x00b8, 0x0010, 0x0897, 0x0014,
		0x0840, 0x00c0, 0x0460, 0x0029, 0x0460, 0x0029, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000 },
};

/**< For Phy control */
#define HDMI_PHY_LAST 0x1E
static int dp_tv_i2c_write(unsigned char addr, unsigned char value);

/**< Phy setting table */
unsigned char clock_27_8_bit[] = { 0x11, 0x1E, 0x55, 0x40, 0x01, 0x00, 0x08,
		0x82, 0x00, 0xB4, 0xD8, 0x45, 0xA0, 0xAC, 0x80, 0x06, 0x80, 0x11, 0x04,
		0x02, 0x22, 0x44, 0x86, 0x54, 0x00, 0x25, 0x00, 0x00, 0x00, 0x01, 0x80 };
unsigned char clock_27_10_bit[] = { 0x12, 0x4B, 0x55, 0x55, 0x01, 0x00, 0x08,
		0x82, 0x00, 0xE1, 0xD8, 0x45, 0xA0, 0xAC, 0x80, 0x06, 0x80, 0x11, 0x04,
		0x02, 0x22, 0x44, 0x86, 0x54, 0xCD, 0x24, 0x00, 0x00, 0x00, 0x01, 0x80 };
unsigned char clock_27_12_bit[] = { 0x11, 0x2D, 0x55, 0x65, 0x01, 0x00, 0x08,
		0x82, 0x00, 0x0E, 0xD9, 0x45, 0xA0, 0xAC, 0x80, 0x06, 0x80, 0x11, 0x04,
		0x02, 0x22, 0x44, 0x86, 0x54, 0xAB, 0x24, 0x00, 0x00, 0x00, 0x01, 0x80 };
unsigned char clock_74_25_8_bit[] = { 0x11, 0x37, 0x35, 0x40, 0x01, 0x00, 0x08,
		0x82, 0x00, 0x4A, 0xD9, 0x45, 0xA0, 0xAC, 0x80, 0x56, 0x80, 0x11, 0x04,
		0x02, 0x22, 0x44, 0x86, 0x54, 0xBA, 0x24, 0x01, 0x00, 0x00, 0x01, 0x80 };
unsigned char clock_74_25_10_bit[] = { 0x91, 0x22, 0x11, 0x51, 0x40, 0x30, 0x08,
		0x86, 0x20, 0xCE, 0xD8, 0x45, 0xA0, 0xAC, 0x80, 0x56, 0x80, 0x11, 0x04,
		0x02, 0x22, 0x44, 0x86, 0x54, 0x95, 0x24, 0x01, 0x00, 0x00, 0x01, 0x80 };
unsigned char clock_74_25_12_bit[] = { 0x91, 0x29, 0x12, 0x61, 0x40, 0x20, 0x08,
		0x87, 0x20, 0xF8, 0xD8, 0x45, 0xA0, 0xAC, 0x80, 0x56, 0x80, 0x11, 0x04,
		0x02, 0x22, 0x44, 0x86, 0x54, 0xF0, 0x25, 0x03, 0x00, 0x00, 0x01, 0x80 };
unsigned char clock_74_176_8_bit[] = { 0x91, 0x37, 0x34, 0x40, 0x5B, 0xF6, 0x08,
		0x81, 0x20, 0x4A, 0xD9, 0x45, 0xA0, 0xAC, 0x80, 0x56, 0x80, 0x11, 0x04,
		0x02, 0x22, 0x44, 0x86, 0x54, 0xBA, 0x24, 0x01, 0x00, 0x00, 0x01, 0x80 };
unsigned char clock_74_176_10_bit[] = { 0x91, 0x22, 0x11, 0x51, 0x5B, 0x3E,
		0x08, 0x86, 0x20, 0xCE, 0xD8, 0x45, 0xA0, 0xAC, 0x80, 0x56, 0x80, 0x11,
		0x04, 0x02, 0x22, 0x44, 0x86, 0x54, 0x95, 0x24, 0x01, 0x00, 0x00, 0x01,
		0x80 };
unsigned char clock_74_176_12_bit[] = { 0x91, 0x29, 0x12, 0x61, 0x5B, 0x26,
		0x08, 0x87, 0x20, 0xF7, 0xD8, 0x45, 0xA0, 0xAC, 0x80, 0x56, 0x80, 0x11,
		0x04, 0x02, 0x22, 0x44, 0x86, 0x54, 0xF1, 0x25, 0x03, 0x00, 0x00, 0x01,
		0x80 };
unsigned char clock_148_5_8_bit[] = { 0x11, 0x37, 0x15, 0x40, 0x01, 0x00, 0x08,
		0x82, 0x00, 0x4A, 0xD9, 0x45, 0xA0, 0xAC, 0x80, 0x66, 0x80, 0x11, 0x04,
		0x02, 0x22, 0x44, 0x86, 0x54, 0x74, 0x25, 0x03, 0x00, 0x00, 0x01, 0x80 };
unsigned char clock_148_5_10_bit[] = { 0x91, 0x22, 0x01, 0x50, 0x40, 0x30, 0x08,
		0x86, 0x20, 0xCE, 0xD8, 0x45, 0xA0, 0xAC, 0xA0, 0x66, 0x80, 0x11, 0x04,
		0x02, 0x22, 0x44, 0x86, 0x54, 0x2A, 0x25, 0x03, 0x00, 0x00, 0x01, 0x80 };
unsigned char clock_148_5_12_bit[] = { 0x91, 0x29, 0x02, 0x60, 0x40, 0x20, 0x08,
		0x87, 0x20, 0xF8, 0xD8, 0x45, 0xA0, 0xAC, 0xA0, 0x66, 0x80, 0x11, 0x04,
		0x02, 0x22, 0x44, 0x86, 0x54, 0xF8, 0x24, 0x03, 0x00, 0x00, 0x01, 0x80 };
unsigned char clock_148_352_8_bit[] = { 0x91, 0x37, 0x14, 0x40, 0x5B, 0xF6,
		0x08, 0x81, 0x20, 0x4A, 0xD9, 0x45, 0xA0, 0xAC, 0x80, 0x66, 0x80, 0x11,
		0x04, 0x02, 0x22, 0x44, 0x86, 0x54, 0x75, 0x25, 0x03, 0x00, 0x00, 0x01,
		0x80 };
unsigned char clock_148_352_10_bit[] = { 0x91, 0x22, 0x01, 0x50, 0x5B, 0x3E,
		0x08, 0x86, 0x20, 0xCE, 0xD8, 0x45, 0xA0, 0xAC, 0xA0, 0x66, 0x80, 0x11,
		0x04, 0x02, 0x22, 0x44, 0x86, 0x54, 0x2A, 0x25, 0x03, 0x00, 0x00, 0x01,
		0x80 };
unsigned char clock_148_352_12_bit[] = { 0x91, 0x29, 0x02, 0x60, 0x5B, 0x26,
		0x08, 0x87, 0x20, 0xF7, 0xD8, 0x45, 0xA0, 0xAC, 0xA0, 0x66, 0x80, 0x11,
		0x04, 0x02, 0x22, 0x44, 0x86, 0x54, 0xF8, 0x24, 0x03, 0x00, 0x00, 0x01,
		0x80 };

static int g_dp_tv_interrupt_state;
static int g_dp_tv_interrupt_arg[MAX_INTERRUPT];

/**
 * @brief Set  Graphic Ch1 DMA Controller
 * @fn void DP_TV_GRP1DMA_OnOff(eDPOnOff OnOff)
 * @param mode   [in] Graphic1 DMA On/Off
 *
 * @return None.
 *
 */
void dp_tv_grp1dma_onoff(struct stfb_info *info, enum edp_onoff onoff)
{
	unsigned int reg = 0;
	reg = __raw_readl(info->dp_tv_regs + DP_TV_PATH_CONTROL);
	if (onoff)
		reg |= GRP1DMAC;
	else
		reg &= ~GRP1DMAC;
	__raw_writel(reg, info->dp_tv_regs + DP_TV_PATH_CONTROL);

}

/**
 * @brief  Set Alpha blending operation On/Off
 * @fn     void DP_TV_Alphablend1_OnOff(eDPOnOff OnOff)
 * @param mode    [in] Set On/Off
 *
 * @return None.
 *
 */
void dp_tv_alphablend1_onoff(struct stfb_info *info, enum edp_onoff onoff)
{
	unsigned int reg = 0;
	reg = __raw_readl(info->dp_tv_regs + DP_TV_PATH_VIDGRPMIX_ON);
	if (onoff)
		reg |= ALPBLD_ON;
	else
		reg &= ~ALPBLD_ON;
	__raw_writel(reg, info->dp_tv_regs + DP_TV_PATH_VIDGRPMIX_ON);

}

void dp_tv_rangedetect_onoff(struct stfb_info *info, enum edp_layer layer,
		enum edp_onoff onoff)
{
	unsigned int reg = 0;
	reg = __raw_readl(info->dp_tv_regs + DP_TV_PATH_VIDGRPMIX_ON);
	if (onoff) {
		if (layer == DP_VIDEO) {
			reg |= RANGE_DET_ON(1);

		} else if (layer == DP_GRP) {
			reg |= RANGE_DET_ON(2);
		}
	} else {
		reg |= RANGE_DET_ON(0);
	}
	__raw_writel(reg, info->dp_tv_regs + DP_TV_PATH_VIDGRPMIX_ON);
}

void dp_tv_rangetarget_onoff(struct stfb_info *info, enum edp_layer layer,
		enum edp_onoff onoff)
{
	unsigned int reg = 0;
	reg = __raw_readl(info->dp_tv_regs + DP_TV_PATH_VIDGRPMIX_ON);
	if (onoff) {
		if (layer == DP_VIDEO) {
			reg |= RANGEDET_TARGET;

		} else if (layer == DP_GRP) {
			reg |= RANGEDET_TARGET;
		}
	} else {
		reg &= ~RANGEDET_TARGET;
	}
	__raw_writel(reg, info->dp_tv_regs + DP_TV_PATH_VIDGRPMIX_ON);
}

void dp_tv_vid_rangedetect_flag(struct stfb_info *info, enum edp_onoff rflag,
		enum edp_onoff gflag, enum edp_onoff bflag, enum edp_onoff ranginv)
{
	unsigned int reg = 0;
	reg = __raw_readl(info->dp_tv_regs + DP_TV_PATH_VIDGRPMIX_ON);

	if (1 == rflag) {
		reg |= FLAG0_R_ON;
	} else if (0 == rflag) {
		reg &= ~FLAG0_R_ON;
	}
	if (1 == gflag) {
		reg |= FLAG0_G_ON;
	} else if (0 == gflag) {
		reg &= ~FLAG0_G_ON;
	}
	if (1 == bflag) {
		reg |= FLAG0_B_ON;
	} else if (0 == bflag) {
		reg &= ~FLAG0_B_ON;
	}
	if (1 == ranginv) {
		reg |= RANGE0_INV;
	} else if (0 == ranginv) {
		reg &= ~RANGE0_INV;
	}
	__raw_writel(reg, info->dp_tv_regs + DP_TV_PATH_VIDGRPMIX_ON);

}

void dp_tv_grp1_rangedetect_flag(struct stfb_info *info, enum edp_onoff rflag,
		enum edp_onoff gflag, enum edp_onoff bflag, enum edp_onoff ranginv)
{
	unsigned int reg = 0;
	reg = __raw_readl(info->dp_tv_regs + DP_TV_PATH_VIDGRPMIX_ON);

	if (1 == rflag) {
		reg |= FLAG0_R_ON;
	} else if (0 == rflag) {
		reg &= ~FLAG0_R_ON;
	}
	if (1 == gflag) {
		reg |= FLAG0_G_ON;
	} else if (0 == gflag) {
		reg &= ~FLAG0_G_ON;
	}
	if (1 == bflag) {
		reg |= FLAG0_B_ON;
	} else if (0 == bflag) {
		reg &= ~FLAG0_B_ON;
	}
	__raw_writel(reg, info->dp_tv_regs + DP_TV_PATH_VIDGRPMIX_ON);

}

/**
 * @brief graphic alpha control
 * @fn static void dp_lcd_set_grp_alpha_ctrl_onoff(struct stfb_info *info, enum egrp_alpha_ctrl selection, enum edp_onoff on_off)
 * @param *info[in]  stfb_info reference linux/include/vide/drime4/d4_dp_type.h
 * @param selection[in] set graphic alpah/R/G/B
 * @return
 *
 * @note
 */
void dp_tv_set_grp_alpha_ctrl_onoff(struct stfb_info *info,
		enum egrp_alpha_control selection, enum edp_onoff on_off)
{
	unsigned int alpha_ctrl_onoff = 0;

	alpha_ctrl_onoff = __raw_readl(
			info->dp_tv_regs + DP_TV_PATH_VIDGRPMIX0_ALPCTRL);

	if (selection == ALPHA_CTRL_ON)
		D4_DP_TV_GRP_ALP_ON(alpha_ctrl_onoff, on_off);
	else if (selection == ALPHA_CTRL_INV)
		D4_DP_TV_GRP_ALP_INV_ON(alpha_ctrl_onoff, on_off);
	else if (selection == ALPHA_CTRL_OFS)
		D4_DP_TV_GRP_ALP_OFS(alpha_ctrl_onoff, on_off);

	__raw_writel(alpha_ctrl_onoff,
			info->dp_tv_regs + DP_TV_PATH_VIDGRPMIX0_ALPCTRL);
}

void dp_tv_alphablend_inv(struct stfb_info *info, enum edp_onoff onoff)
{
	unsigned int reg = 0;
	reg = __raw_readl(info->dp_tv_regs + DP_TV_PATH_VIDGRPMIX0_ALPCTRL);
	if (onoff)
		reg |= CHx_ALP_INV;
	else
		reg &= ~CHx_ALP_INV;
	__raw_writel(reg, info->dp_tv_regs + DP_TV_PATH_VIDGRPMIX0_ALPCTRL);

}

void dp_tv_alphablend1_offset_ctrl(struct stfb_info *info, enum edp_onoff onoff,
		unsigned short value)
{
	unsigned int reg = 0;
	reg = __raw_readl(info->dp_tv_regs + DP_TV_PATH_VIDGRPMIX0_ALPCTRL);
	if (onoff) {
		reg |= CHx_ALP_OFS;
		reg |= CHx_ALP_OFS_4BIT(value);
	} else {
		reg &= ~CHx_ALP_OFS;
		reg &= ~CHx_ALP_OFS_4BIT(value);
	}

	__raw_writel(reg, info->dp_tv_regs + DP_TV_PATH_VIDGRPMIX0_ALPCTRL);

}

void dp_tv_alphablend1_value_set(struct stfb_info *info, unsigned short value)
{
	unsigned int reg = 0;
	reg = __raw_readl(info->dp_tv_regs + DP_TV_PATH_VIDGRPMIX0_ALPCTRL);
	reg |= C_KEY_ALP_8BIT(value);

	__raw_writel(reg, info->dp_tv_regs + DP_TV_PATH_VIDGRPMIX0_ALPCTRL);

}

void dp_tv_grpmix0_ch0_onoff(struct stfb_info *info, enum edp_onoff rOnOff,
		enum edp_onoff gOnOff, enum edp_onoff bOnOff)
{
	unsigned int reg = 0;
	reg = __raw_readl(info->dp_tv_regs + DP_TV_PATH_VIDGRPMIX0_CH0);

	if (1 == rOnOff) {
		D4_DP_TV_GRP_MIX_R_ON(reg, rOnOff);
	} else if (0 == rOnOff) {
		D4_DP_TV_GRP_MIX_R_ON(reg, rOnOff);
	}

	if (1 == gOnOff) {
		D4_DP_TV_GRP_MIX_G_ON(reg, gOnOff);
	} else if (0 == gOnOff) {
		D4_DP_TV_GRP_MIX_G_ON(reg, gOnOff);
	}

	if (1 == bOnOff) {
		D4_DP_TV_GRP_MIX_B_ON(reg, bOnOff);
	} else if (0 == bOnOff) {
		D4_DP_TV_GRP_MIX_B_ON(reg, bOnOff);
	}
	__raw_writel(reg, info->dp_tv_regs + DP_TV_PATH_VIDGRPMIX0_CH0);

}

void dp_tv_grpmix0_ch0_inv_onoff(struct stfb_info *info, enum edp_onoff rOnOff,
		enum edp_onoff gOnOff, enum edp_onoff bOnOff)
{
	unsigned int reg = 0;
	reg = __raw_readl(info->dp_tv_regs + DP_TV_PATH_VIDGRPMIX0_CH0);

	if (1 == rOnOff) {
		reg |= CH_R_INV_ON;
	} else if (0 == rOnOff) {
		reg &= ~CH_R_INV_ON;
	}
	if (1 == gOnOff) {
		reg |= CH_G_INV_ON;
	} else if (0 == gOnOff) {
		reg &= ~CH_G_INV_ON;
	}
	if (1 == bOnOff) {
		reg |= CH_B_INV_ON;
	} else if (0 == bOnOff) {
		reg &= ~CH_B_INV_ON;
	}
	__raw_writel(reg, info->dp_tv_regs + DP_TV_PATH_VIDGRPMIX0_CH0);

}

void dp_tv_grpmix0_ch1_inv_onoff(struct stfb_info *info, enum edp_onoff rOnOff,
		enum edp_onoff gOnOff, enum edp_onoff bOnOff)
{
	unsigned int reg = 0;
	reg = __raw_readl(info->dp_tv_regs + DP_TV_PATH_VIDGRPMIX0_CH1);

	if (1 == rOnOff) {
		reg |= CH_R_INV_ON;
	} else if (0 == rOnOff) {
		reg &= ~CH_R_INV_ON;
	}
	if (1 == gOnOff) {
		reg |= CH_G_INV_ON;
	} else if (0 == gOnOff) {
		reg &= ~CH_G_INV_ON;
	}
	if (1 == bOnOff) {
		reg |= CH_B_INV_ON;
	} else if (0 == bOnOff) {
		reg &= ~CH_B_INV_ON;
	}
	__raw_writel(reg, info->dp_tv_regs + DP_TV_PATH_VIDGRPMIX0_CH1);

}

void dp_tv_grpmix0_ch0_offset_ctrl(struct stfb_info *info,
		enum edp_onoff OffsetOnOff, unsigned int rOffset, unsigned int gOffset,
		unsigned int bOffset, unsigned short sign)
{
	unsigned int reg = 0;
	reg = __raw_readl(info->dp_tv_regs + DP_TV_PATH_VIDGRPMIX0_CH0);

	if (OffsetOnOff) {
		reg |= CH_R_OFS_ON;
		reg |= CH_G_OFS_ON;
		reg |= CH_B_OFS_ON;

		reg = OFS_SIGN(sign) | R_OFS_5BIT(rOffset) | G_OFS_5BIT(gOffset)
				| B_OFS_5BIT(bOffset);
	} else {
		reg &= ~CH_R_OFS_ON;
		reg &= ~CH_G_OFS_ON;
		reg &= ~CH_B_OFS_ON;
	}
	__raw_writel(reg, info->dp_tv_regs + DP_TV_PATH_VIDGRPMIX0_CH0);

}

void dp_tv_grpmix0_ch1_offset_ctrl(struct stfb_info *info,
		enum edp_onoff OffsetOnOff, unsigned int rOffset, unsigned int gOffset,
		unsigned int bOffset, unsigned short sign)
{
	unsigned int reg = 0;
	reg = __raw_readl(info->dp_tv_regs + DP_TV_PATH_VIDGRPMIX0_CH1);

	if (OffsetOnOff) {
		reg |= CH_R_OFS_ON;
		reg |= CH_G_OFS_ON;
		reg |= CH_B_OFS_ON;

		reg = OFS_SIGN(sign) | R_OFS_5BIT(rOffset) | G_OFS_5BIT(gOffset)
				| B_OFS_5BIT(bOffset);
	} else {
		reg &= ~CH_R_OFS_ON;
		reg &= ~CH_G_OFS_ON;
		reg &= ~CH_B_OFS_ON;
	}
	__raw_writel(reg, info->dp_tv_regs + DP_TV_PATH_VIDGRPMIX0_CH1);

}

void dp_tv_grpmix0_ch1_onoff(struct stfb_info *info, enum edp_onoff rOnOff,
		enum edp_onoff gOnOff, enum edp_onoff bOnOff)
{
	unsigned int reg = 0;
	reg = __raw_readl(info->dp_tv_regs + DP_TV_PATH_VIDGRPMIX0_CH1);

	if (1 == rOnOff) {
		D4_DP_TV_GRP_MIX_R_ON(reg, rOnOff);
	} else if (0 == rOnOff) {
		D4_DP_TV_GRP_MIX_R_ON(reg, rOnOff);
	}
	if (1 == gOnOff) {
		D4_DP_TV_GRP_MIX_G_ON(reg, gOnOff);
	} else if (0 == gOnOff) {
		D4_DP_TV_GRP_MIX_G_ON(reg, gOnOff);
	}
	if (1 == bOnOff) {
		D4_DP_TV_GRP_MIX_B_ON(reg, bOnOff);
	} else if (0 == bOnOff) {
		D4_DP_TV_GRP_MIX_B_ON(reg, bOnOff);
	}
	__raw_writel(reg, info->dp_tv_regs + DP_TV_PATH_VIDGRPMIX0_CH1);

}

void dp_tv_ch_rangedetect_set(struct stfb_info *info,
		struct stdp_rgb_range range)
{
	unsigned int reg0 = 0, reg1 = 0, reg2 = 0;
	reg0 = __raw_readl(info->dp_tv_regs + DP_TV_PATH_GRPMIX_RANGE_R);
	reg1 = __raw_readl(info->dp_tv_regs + DP_TV_PATH_GRPMIX_RANGE_G);
	reg2 = __raw_readl(info->dp_tv_regs + DP_TV_PATH_GRPMIX_RANGE_B);

	reg0 |= RANGEDET_R_UPP(range.R_Range.Upper_Range)
			| RANGEDET_R_LOW(range.R_Range.Lower_Range);
	reg1 |= RANGEDET_G_UPP(range.G_Range.Upper_Range)
			| RANGEDET_G_LOW(range.G_Range.Lower_Range);
	reg2 |= RANGEDET_B_UPP(range.B_Range.Upper_Range)
			| RANGEDET_B_LOW(range.B_Range.Lower_Range);

	__raw_writel(reg0, info->dp_tv_regs + DP_TV_PATH_GRPMIX_RANGE_R);
	__raw_writel(reg1, info->dp_tv_regs + DP_TV_PATH_GRPMIX_RANGE_G);
	__raw_writel(reg2, info->dp_tv_regs + DP_TV_PATH_GRPMIX_RANGE_B);

}

void dp_tv_grp_bkg_argb_order(struct stfb_info *info, enum edp_layer layer,
		enum edp_onoff onoff)
{
	unsigned int reg = 0;
	if (layer == DP_VIDEO)
		return;
	else if (layer == DP_GRP) {
		reg = __raw_readl(info->dp_tv_regs + DP_TV_GRP_PATH1_DMA_CTRL);
		if (onoff)
			reg |= GRP_ARGB_ORDER;
		else
			reg &= ~GRP_ARGB_ORDER;
	}
	__raw_writel(reg, info->dp_tv_regs + DP_TV_GRP_PATH1_DMA_CTRL);

}

void dp_tv_grp_flat_alpha_set(struct stfb_info *info, enum dp_window win,
		enum edp_onoff onoff, unsigned short value)
{
	unsigned int reg0 = 0, reg1 = 0;
	reg0 = __raw_readl(info->dp_tv_regs + DP_TV_GRP_PATH1_DMA_CTRL);
	reg1 = __raw_readl(info->dp_tv_regs + DP_TV_GRP_PATH1_FLAT_ALPHA);

	switch (win) {
	case DP_WIN0:
		D4_DP_TV_GRP_WIN0_ALPHA(reg1, value);
		if (onoff)
			reg0 |= WD0_ALPHA_ON;
		else
			reg0 &= ~WD0_ALPHA_ON;
		break;
	case DP_WIN1:
		D4_DP_TV_GRP_WIN1_ALPHA(reg1, value);
		if (onoff)
			reg0 |= WD1_ALPHA_ON;
		else
			reg0 &= ~WD1_ALPHA_ON;
		break;
	case DP_WIN2:
		D4_DP_TV_GRP_WIN2_ALPHA(reg1, value);
		if (onoff)
			reg0 |= WD2_ALPHA_ON;
		else
			reg0 &= ~WD2_ALPHA_ON;
		break;
	case DP_WIN3:
		D4_DP_TV_GRP_WIN3_ALPHA(reg1, value);
		if (onoff)
			reg0 |= WD3_ALPHA_ON;
		else
			reg0 &= ~WD3_ALPHA_ON;
		break;
	}
	__raw_writel(reg0, info->dp_tv_regs + DP_TV_GRP_PATH1_DMA_CTRL);
	__raw_writel(reg1, info->dp_tv_regs + DP_TV_GRP_PATH1_FLAT_ALPHA);

}

/**
 * @brief graphic scale set
 * @fn void d4_dp_tv_graphics_set_scale(enum egrp_scale scale)
 * @param scale[in] graphic scale, original/double
 * @return
 *
 * @note
 */
void d4_dp_tv_graphics_set_scale(enum egrp_scale scale)
{
	dp_tv_grp1_set_scaler(&tv_info, scale);
}

void d4_dp_tv_graphics_set_vertical_scale(enum egrp_scale scale)
{
	dp_tv_grp_vertical_zoom_set(&tv_info, scale);
}


void dp_tv_grp1_set_scaler(struct stfb_info *info, enum egrp_scale scale)
{
	dp_tv_grp_zoom_set(info, scale);
}

void dp_tv_grp_bkg_set(struct stfb_info *info, enum edp_layer layer,
		struct stdp_argb *argb)
{
	unsigned int reg = 0;
	if (layer == DP_GRP) {
		D4_DP_TV_DMA_BKG_R(reg, argb->DP_R);
		D4_DP_TV_DMA_BKG_G(reg, argb->DP_G);
		D4_DP_TV_DMA_BKG_B(reg, argb->DP_B);
		D4_DP_TV_DMA_BKG_A(reg, argb->DP_A);

		__raw_writel(reg, info->dp_tv_regs + DP_TV_GRP_PATH1_BKG);
	}

}

int dp_tv_graphic_display_area(struct stfb_info *info,
		struct stgraphic_display_area *area)
{
	unsigned int reg = 0, reg1 = 0, index = 0;
	unsigned int h_start = area->display.H_Start;
	unsigned int h_end;
	unsigned int v_start = area->display.V_Start;
	unsigned int v_end;

#if 0
	reg1 = __raw_readl(info->dp_tv_regs + DP_TV_GRP_PATH1_DMA_SCL);

	if ((reg1 & V_ZOOM) == 1) {
		if (v_start % 2 != 0
				|| (area->display.V_Size - area->display.V_Start) % 2 != 0)
			return -1;
	}

	if ((reg1 & H_ZOOM) == 1) {
		if (h_start % 2 != 0
				|| (area->display.H_Size - area->display.H_Start) % 2 != 0)
			return -1;
	}
#endif
	
	if((v_start % 4) < 3)
		v_start = v_start - (v_start % 4);
	else
		v_start = v_start + (4-(v_start % 4));

	if (area->display.H_Size >= 1920)
		area->display.H_Size = 1920;
	if (area->display.V_Size >= 1080)
		area->display.V_Size = 1080;

	v_end = area->display.V_Size + v_start;
	h_end = area->display.H_Size + h_start;

	index = (area->win) * 0x10;

	D4_DP_TV_POS_H_START(reg, h_start);
	D4_DP_TV_POS_H_END(reg, h_end);
	__raw_writel(reg, info->dp_tv_regs + DP_TV_GRP_PATH1_CH0_HPOS + index);

	reg = 0;
	D4_DP_TV_POS_V_START(reg, v_start);
	D4_DP_TV_POS_V_END(reg, v_end);
	__raw_writel(reg, info->dp_tv_regs + DP_TV_GRP_PATH1_CH0_VPOS + index);

	return 0;
}

void dp_tv_graphics_window_onoff(struct stfb_info *info, enum edp_window window,
		enum edp_onoff on_off)
{
	unsigned int reg = 0;
	reg = __raw_readl(info->dp_tv_regs + DP_TV_GRP_PATH1_DMA_CTRL);

	switch (window) {
		case DP_WIN0:
			if (on_off)
				reg |= Win0_On;
			else
				reg &= ~Win0_On;
			break;

		case DP_WIN1:
			if (on_off)
				reg |= Win1_On;
			else
				reg &= ~Win1_On;
			break;

		case DP_WIN2:
			if (on_off)
				reg |= Win2_On;
			else
				reg &= ~Win2_On;
			break;

		case DP_WIN3:
			if (on_off)
				reg |= Win3_On;
			else
				reg &= ~Win3_On;
			break;
		default:
			break;
	}
	__raw_writel(reg, info->dp_tv_regs + DP_TV_GRP_PATH1_DMA_CTRL);

}

void dp_tv_set_graphics_image_address(struct stfb_info *info,
		enum edp_window window, unsigned int address)
{
	unsigned int index = 0, reg0 = 0, reg1 = 0, reg2 = 0, stride = 0;

	reg2 = __raw_readl(info->dp_tv_regs + DP_TV_GRP_PATH1_DMA_SCL);

	index = window * 0x10;

	reg0 = __raw_readl(info->dp_tv_regs + DP_TV_PATH_STRIDE);
	reg0 = (reg0 >> 16) & 0x7ff;
	stride = reg0 * 8;

	reg1 = address;
	reg2 = address + stride;
	__raw_writel(reg1, info->dp_tv_regs + DP_TV_GRP_PATH1_CH0_ADDR_F0 + index);
	__raw_writel(reg2, info->dp_tv_regs + DP_TV_GRP_PATH1_CH0_ADDR_F1 + index);
}

void dp_tv_grp1_win_prority(struct stfb_info *info, unsigned char win0,
		unsigned char win1, unsigned char win2, unsigned char win3)
{
	unsigned int reg = 0;
	reg |= WD0_PRIORITY(win0) | WD1_PRIORITY(win1) | WD2_PRIORITY(win2)
			| WD3_PRIORITY(win3);
	__raw_writel(reg, info->dp_tv_regs + DP_TV_GRP_PATH1_DMA_PRIORITY);

}

void dp_tv_grp_vertical_zoom_set(struct stfb_info *info, enum egrp_scale scale)
{
	unsigned int grp_scaler_set = 0;

	grp_scaler_set = __raw_readl(info->dp_tv_regs + DP_TV_GRP_PATH1_DMA_SCL);

	if (scale == SCL_X2) {
		D4_DP_TV_DMA_ZOOM_V(grp_scaler_set, DP_ON);
	} else {
		D4_DP_TV_DMA_ZOOM_V(grp_scaler_set, DP_OFF);
	}
	__raw_writel(grp_scaler_set, info->dp_tv_regs + DP_TV_GRP_PATH1_DMA_SCL);

}


void dp_tv_grp_zoom_set(struct stfb_info *info, enum egrp_scale scale)
{
	unsigned int grp_scaler_set = 0;

	if (scale == SCL_X2) {
		D4_DP_TV_DMA_ZOOM_V(grp_scaler_set, DP_ON);
		D4_DP_TV_DMA_ZOOM_H(grp_scaler_set, DP_ON);
	} else {
		D4_DP_TV_DMA_ZOOM_V(grp_scaler_set, DP_OFF);
		D4_DP_TV_DMA_ZOOM_H(grp_scaler_set, DP_OFF);
	}
	__raw_writel(grp_scaler_set, info->dp_tv_regs + DP_TV_GRP_PATH1_DMA_SCL);

}

void d4_dp_tv_change_layer(enum edp_layer layer)
{
	if (layer == DP_VIDEO)
		dp_tv_channelswap(&tv_info, 0);
	else
		dp_tv_channelswap(&tv_info, 1);
}

void dp_tv_channelswap(struct stfb_info *info, unsigned int type)
{
	unsigned int reg = 0;
	reg = __raw_readl(info->dp_tv_regs + DP_TV_PATH_VIDGRPMIX_ON);
	if (type == 0)
		D4_DP_TV_CH_SWAP(reg, 0);
	else if (type == 1)
		D4_DP_TV_CH_SWAP(reg, 1);
	else if (type == 2)
		D4_DP_TV_CH_SWAP(reg, 2);
	__raw_writel(reg, info->dp_tv_regs + DP_TV_PATH_VIDGRPMIX_ON);

}

void d4_dp_tv_set_info(struct tvfb_get_info *tvgetinfo)
{
	dp_tv_set_info(tvgetinfo);
}

/* fb driver calls this function to initialize
 * is fb driver a common utility? if yes then it can initialize tv fb_info too
 */
void dp_tv_set_info(struct tvfb_get_info *tvgetinfo)
{
	tv_info.dp_global_regs = tvgetinfo->dp_global;
	tv_info.dp_tv_regs = tvgetinfo->dp_tv;
	tv_video = tvgetinfo->video;
	tv_graphic = tvgetinfo->graphic;
	i2client = tvgetinfo->tv_i2c_client;
}

/**
 * @brief video stride set
 * @fn int d4_dp_tv_video_stride(unsigned int stride)
 * @param stride[in]  stride value, it must be a multiple of 8
 * @return
 *
 * @note
 */
int d4_dp_tv_video_stride(unsigned int stride)
{
	return dp_tv_set_stride(&tv_info, DP_VIDEO, stride);
}

/**
 * @brief graphics stride set
 * @fn int void d4_dp_tv_graphics_set_stride(unsigned int stride)
 * @param stride[in]  stride value, it must be a multiple of 8
 * @return
 *
 * @note
 */
int d4_dp_tv_graphics_set_stride(unsigned int stride)
{
	return dp_tv_set_stride(&tv_info, DP_GRP, stride);
}

/**
 * @brief graphic background set
 * @fn void d4_dp_tv_graphics_background(struct stdp_argb *argb)
 * @param *argb[in] struct stdp_argb reference linux/include/video/drime4/d4_dp_type.h
 * @return
 *
 * @note
 */
void d4_dp_tv_graphics_background(struct stdp_argb *argb)
{
	dp_tv_grp_bkg_set(&tv_info, DP_GRP, argb);
}

/**
 * @brief graphic display area set
 * @fn void d4_dp_tv_graphic_display_area(struct stgraphic_display_area *area)
 * @param *area[in] struct stgraphic_display_area reference linux/include/video/drime4/d4_dp_type.h
 * @return
 *
 * @note
 */
void d4_dp_tv_graphics_display_area(struct stgraphic_display_area *area)
{
	if (dp_tv_graphic_display_area(&tv_info, area) < 0)
		printk(
				"[Warning] graphic display H position the multiple of 2:[%s:%d]:%s\n",
				__FUNCTION__, __LINE__, __FILE__);
}

/**
 * @brief  Set Graphic1 window On/Off
 * @fn void d4_dp_tv_graphics_window_onoff(enum edp_window window, enum edp_onoff on_off)
 * @param mode[in] Set window
 * @param mode[in] Set On/Off
 *
 * @return None.
 *
 * @note
 */
void d4_dp_tv_graphics_window_onoff(enum edp_window window,
		enum edp_onoff on_off)
{
	dp_tv_graphics_window_onoff(&tv_info, window, on_off);
}

/**
 * @brief graphic display window,display area, address, format, stride set, viedeo with relation all part set
 * @fn void d4_dp_tv_graphics_set(struct stgrpdisplay graphic)
 * @param graphic[in] struct stgrpdisplay reference linux/include/video/drime4/d4_dp_type.h
 * @return
 *
 * @note
 */
void d4_dp_tv_graphics_set(struct stgrpdisplay graphic)
{
	struct stgraphic_display_area area;
	unsigned int address;

#if 0
	if (graphic.win_onoff == DP_OFF)
		return;
#endif

	area.win = graphic.win;
	area.display = graphic.display;
	address = graphic.address;

	if (dp_tv_graphic_display_area(&tv_info, &area) < 0)
		printk(
				"[Warning] graphic display H position the multiple of 2:[%s:%d]:%s\n",
				__FUNCTION__, __LINE__, __FILE__);

	dp_tv_auto_update_onoff(&tv_info, DP_OFF);
	dp_tv_set_stride(&tv_info, DP_GRP, graphic.stride);
	dp_tv_set_graphics_image_address(&tv_info, graphic.win, address);
	dp_tv_auto_update_onoff(&tv_info, DP_ON);
}

/**
 * @brief graphic window display prority set
 * @fn void d4_dp_tv_graphics_window_priority(struct stdp_grp_prority *priority)
 * @param *priority[in] struct stdp_grp_prority reference linux/include/video/drime4/d4_dp_type.h
 * @return
 *
 * @note
 */
void d4_dp_tv_graphics_window_priority(struct stdp_grp_prority *priority)
{
	dp_tv_graphics_window_priority(&tv_info, priority);
}

void dp_tv_graphics_window_priority(struct stfb_info *info,
		struct stdp_grp_prority *priority)
{
	unsigned char win_num, Sequence[4] = { 0, }, Max = 3;

	for (win_num = 0; win_num < 4; win_num++) {
		if (priority->First_Priority == win_num)
			Sequence[win_num] = Max;
		if (priority->Second_Priority == win_num)
			Sequence[win_num] = Max - 1;
		if (priority->Third_Priotrity == win_num)
			Sequence[win_num] = Max - 2;
		if (priority->Fourth_Priority == win_num)
			Sequence[win_num] = Max - 3;
	}
	dp_tv_grp1_win_prority(info, Sequence[0], Sequence[1], Sequence[2],
			Sequence[3]);

}

/**
 * @brief Set the TV Mode
 * @fn void d4_dp_tv_set_mode(struct stdp_tv_modes modes)
 * @param modes[in] Tv Mode (resolution)
 *
 * @return none
 *
 */
void d4_dp_tv_set_mode(enum edp_tv_mode mode)
{
	dp_tv_mode_set(mode, DP_ON, input8_out8);
}

/**
 * @brief dp graphic flat alpha set
 * @fn void d4_dp_tv_graphic_window_alpha(struct sttv_graphic_alpha alpha)
 * @param alpha[in] struct sttv_graphic_alpha reference linux/include/vide/drime4/d4_dp_type.h
 * @return none
 *
 */
void d4_dp_tv_graphic_window_alpha(struct sttv_graphic_alpha alpha)
{
	dp_tv_grp_flat_alpha_set(&tv_info, alpha.win, alpha.on_off,
			alpha.alpha_value);
}

/**
 * @brief d4_dp_tv_graphic_order_change
 * @fn void d4_dp_tv_graphic_order_change(enum edp_onoff on_off)
 * @param on_off[in] DP_ON or DP_OFF
 * @return none
 *
 */
void d4_dp_tv_graphic_order_change(enum edp_onoff on_off)
{
	dp_tv_grp_bkg_argb_order(&tv_info, DP_GRP, on_off);
}


void d4_dp_link_set(enum edp_tv_mode mode, enum edp_onoff onoff)
{
	dp_tv_mode_set(mode, onoff, input8_out8);
}


void d4_dp_tv_mode_set(void)
{
	dp_tv_mode_set(RES_1080I_60, DP_ON, input8_out8);
}

void d4_dp_tv_vid_init(void)
{
	dp_tv_vid_init();
}

/**
 * @brief To finalize the TV path
 * @fn void d4_dp_tv_turn_video_on(void)
 * @param none
 *
 * @return none
 *
 * @note
 */
void d4_dp_tv_turn_video_on(void)
{
	struct stdp_video_layer_info vidinfo;
	struct grp_layer_info grpinfo;
	struct stvideo_on video;


	/* video init*/
	d4_dp_tv_vid_init();

	/* graphic init */
	d4_dp_tv_grp_init();

#if 0	/* is this required as HDMI mode set called it already */
	d4_dp_tv_mode_set();
#endif
	/* video layer set */
	vidinfo.stride = 4096;
	vidinfo.type = d4_dp_tv_vid_scan_get();
	vidinfo.background_color.DP_Y = 0x00;
	vidinfo.background_color.DP_Cb = 0x80;
	vidinfo.background_color.DP_Cr = 0x80;
	d4_dp_tv_vid_set_layer(&vidinfo);

	/* graphic layer set */
	grpinfo.stride = 4096;
	grpinfo.scan_type = d4_dp_tv_grp_scan_get();
	grpinfo.background_color.DP_A = 0;
	grpinfo.background_color.DP_R = 0x00;
	grpinfo.background_color.DP_G = 0x00;
	grpinfo.background_color.DP_B = 0x00;

	d4_dp_tv_grp1_set_layer_info(&grpinfo);

	video.window = tv_video.win;
	video.format = tv_video.format;
	video.bit = tv_video.bit;
	video.stride = tv_video.vid_stride;
	video.img_width = tv_video.image.image_width;
	video.img_height = tv_video.image.image_height;
	video.mode.type = none;
	video.mode.window = l_window;

	video.image_addr.y0_address = tv_video.address.y0_address;
	video.image_addr.y1_address = tv_video.address.y1_address;
	video.image_addr.c0_address = tv_video.address.c1_address;
	video.image_addr.c1_address = tv_video.address.c1_address;

	video.display_area.H_Start = tv_video.display.H_Start;
	video.display_area.H_Size = tv_video.display.H_Size;
	video.display_area.V_Start = tv_video.display.V_Start;
	video.display_area.V_Size = tv_video.display.V_Size;

	//d4_dp_tv_video_on(video);

#if 1
	video.window = tv_video.win;
	video.format = tv_video.format;
	video.bit = tv_video.bit;
	video.stride = tv_video.vid_stride;
	video.img_width = tv_video.image.image_width;
	video.img_height = tv_video.image.image_height;

	video.mode.window = r_window;
	video.image_addr.y0_address = tv_video.address.y0_address;
	video.image_addr.y1_address = tv_video.address.y1_address;
	video.image_addr.c0_address = tv_video.address.c0_address;
	video.image_addr.c1_address = tv_video.address.c1_address;

	video.display_area.H_Start = tv_video.display.H_Start;
	video.display_area.H_Size = tv_video.display.H_Size;
	video.display_area.V_Start = tv_video.display.V_Start;
	video.display_area.V_Size = tv_video.display.V_Size;

	//d4_dp_tv_video_on(video);
#endif
}

/**
 * @brief DP TV Clock enable/disable function
 * @fn void d4_dp_tv_clock_onoff(enum edp_tv_clock clock, enum edp_onoff on_off)
 * @param on_off[in]	Turn On/Off Tv Clock & sub-component clocks
 *
 * @return none
 *
 * @note
 */
void d4_dp_tv_clock_onoff(enum edp_onoff on_off)
{
	if (on_off == DP_ON) {
		/* hdmi phy control */
		dp_tv_hdmi_phy_onoff(&tv_info, on_off);
		
		/* TV System Start */
		dp_tv_clock_onoff(&tv_info, tv_clock, on_off);
		dp_tv_clock_onoff(&tv_info, tv_sys_clock, on_off);
		dp_tv_clock_onoff(&tv_info, hdmi_clock, on_off);
		dp_tv_clock_onoff(&tv_info, hdmi_sys_clock, on_off);
		dp_tv_clock_onoff(&tv_info, hdmi_pixel_clock, on_off);
		dp_tv_clock_onoff(&tv_info, hdmi_pixel_clock, on_off);
		dp_tv_clock_onoff(&tv_info, hdmi_tmds_clock, on_off);
		dp_tv_clock_onoff(&tv_info, hdmi_reference_clock, on_off);
		dp_global_nlc_clock_onoff(&tv_info, on_off);
	} else {
		/* TV System Stop */
		dp_tv_clock_onoff(&tv_info, tv_clock, on_off);
		dp_tv_clock_onoff(&tv_info, tv_sys_clock, on_off);
		dp_tv_clock_onoff(&tv_info, hdmi_clock, on_off);
		dp_tv_clock_onoff(&tv_info, hdmi_sys_clock, on_off);
		dp_tv_clock_onoff(&tv_info, hdmi_pixel_clock, on_off);
		dp_tv_clock_onoff(&tv_info, hdmi_pixel_clock, on_off);
		dp_tv_clock_onoff(&tv_info, hdmi_tmds_clock, on_off);
		dp_tv_clock_onoff(&tv_info, hdmi_reference_clock, on_off);
		dp_global_nlc_clock_onoff(&tv_info, on_off);

		/* hdmi phy control */
		dp_tv_hdmi_phy_onoff(&tv_info, on_off);
	}
}

void d4_dp_hdmi_clock_onoff(enum edp_tv_clock clock, enum edp_onoff on_off)
{
	dp_tv_clock_onoff(&tv_info, clock, on_off);
}

/**
 * @brief DP TV Video Block
 * @fn void DP_TV_VID_Init(void)
 * @param none
 *
 * @return none
 *
 * @note  DP TV Video .<br>
 */
void dp_tv_vid_init(void)
{
	struct stdp_ycbcr bkg_color;

	/* TV System Start */
	dp_tv_global_clock_onoff(&tv_info, DP_ON);
	dp_tv_clock_onoff(&tv_info, tv_clock, DP_ON);
	dp_tv_clock_onoff(&tv_info, tv_sys_clock, DP_ON);
	dp_tv_clock_onoff(&tv_info, hdmi_clock, DP_ON);
	dp_tv_clock_onoff(&tv_info, hdmi_sys_clock, DP_ON);
	dp_tv_clock_onoff(&tv_info, hdmi_pixel_clock, DP_ON);
	dp_tv_clock_onoff(&tv_info, hdmi_pixel_clock, DP_ON);
	dp_tv_clock_onoff(&tv_info, hdmi_tmds_clock, DP_ON);
	dp_tv_clock_onoff(&tv_info, hdmi_reference_clock, DP_ON);
	dp_global_nlc_clock_onoff(&tv_info, DP_ON);

	/* DP Data Update */
	dp_tv_register_update(&tv_info);

	dp_tv_vidmac_onoff(&tv_info, DP_ON);

	/*	Video Data initialize */
	/* background color set : black */
	bkg_color.DP_Y = 0x00;
	bkg_color.DP_Cb = 0x80;
	bkg_color.DP_Cr = 0x80;
	dp_tv_bkg_color_set(&tv_info, &bkg_color);

	dp_tv_bb_init(&tv_info);
	dp_tv_y_8burst_onoff(&tv_info, 0);
	dp_tv_y_moalen_set(&tv_info, 7);
	dp_tv_c_8burst_onoff(&tv_info, 0);
	dp_tv_c_moalen_set(&tv_info, 7);
	dp_tv_g1_8burst_onoff(&tv_info, 0);
	dp_tv_g1_moalen_set(&tv_info, 7);

	dp_tv_out_hrz_filter_on(&tv_info, DP_VIDEO, DP_BYPASS);
	dp_tv_out_hrz_filter_off(&tv_info, DP_VIDEO);
	dp_tv_grpmix_onoff(&tv_info, DP_OFF);

	/* TV Path Out default */
	dp_tv_out_default(&tv_info);

}

void d4_dp_tv_grp_init(void)
{
	dp_tv_grp_init();
}

/**
 * @brief DP TV Graphic block
 * @fn void DP_TV_GRP_Init(void)
 * @param none
 *
 * @return none
 *
 * @note   Graphic data default
 *         Default data: Graphic DMA ON, BMT/CSC/filter Off, Graphic Mix off,
 *         Range detect off, graphic scale original size,
 *         graphic priority window3> window2> window1> window0
 */
void dp_tv_grp_init(void)
{
	struct stdp_argb grpbkg;
	struct stdp_rgb_range range;
	range.R_Range.Lower_Range = 0;
	range.R_Range.Upper_Range = 0;
	range.G_Range.Lower_Range = 0;
	range.G_Range.Upper_Range = 0;
	range.B_Range.Lower_Range = 0;
	range.B_Range.Upper_Range = 0;

	grpbkg.DP_A = 0;
	grpbkg.DP_R = 0;
	grpbkg.DP_G = 0;
	grpbkg.DP_B = 0;

	/*Graphic Default*/
	dp_tv_grp1dma_onoff(&tv_info, DP_ON);
	dp_tv_grpmix_onoff(&tv_info, DP_ON);

	dp_tv_out_hrz_filter_on(&tv_info, DP_GRP, DP_BYPASS);
	dp_tv_out_hrz_filter_off(&tv_info, DP_GRP);

	/* GRP Mix */
	dp_tv_alphablend1_onoff(&tv_info, DP_ON);
	dp_tv_rangedetect_onoff(&tv_info, DP_VIDEO, DP_OFF);
	dp_tv_channelswap(&tv_info, 0);
	dp_tv_rangetarget_onoff(&tv_info, DP_VIDEO, DP_OFF);
	dp_tv_vid_rangedetect_flag(&tv_info, 0, 0, 0, 0);
	dp_tv_grp1_rangedetect_flag(&tv_info, 0, 0, 0, 0);

	/* alpha ctrl value */
	dp_tv_set_grp_alpha_ctrl_onoff(&tv_info, ALPHA_CTRL_ON, DP_ON);

	dp_tv_alphablend_inv(&tv_info, 0);
	dp_tv_alphablend1_offset_ctrl(&tv_info, DP_OFF, 0);
	dp_tv_alphablend1_value_set(&tv_info, 0);

	/* TVGRPMIX0_CH0 */
	dp_tv_grpmix0_ch0_onoff(&tv_info, DP_ON, DP_ON, DP_ON);
	dp_tv_grpmix0_ch0_inv_onoff(&tv_info, DP_OFF, DP_OFF, DP_OFF);
	dp_tv_grpmix0_ch0_offset_ctrl(&tv_info, DP_OFF, 0, 0, 0, 0);

	/* TVGRPMIX0_CH1 */
	dp_tv_grpmix0_ch1_onoff(&tv_info, DP_ON, DP_ON, DP_ON);
	dp_tv_grpmix0_ch1_inv_onoff(&tv_info, DP_OFF, DP_OFF, DP_OFF);
	dp_tv_grpmix0_ch1_offset_ctrl(&tv_info, DP_OFF, 0, 0, 0, 0);

	/* TV Y RANGE CH: 0 set */
	dp_tv_ch_rangedetect_set(&tv_info, range);

	/* Graphic1 DMA Ctrl*/
	dp_tv_grp_bkg_argb_order(&tv_info, DP_GRP, DP_ON);
	dp_tv_grp_flat_alpha_set(&tv_info, DP_WIN0, DP_OFF, 0xff);
	dp_tv_grp_flat_alpha_set(&tv_info, DP_WIN1, DP_OFF, 0xff);
	dp_tv_grp_flat_alpha_set(&tv_info, DP_WIN2, DP_ON, 0xff);
	dp_tv_grp_flat_alpha_set(&tv_info, DP_WIN3, DP_ON, 0xff);
	/* Graphic1 zoom scale */
	dp_tv_grp1_set_scaler(&tv_info, GRP_SCL_X1);
	dp_tv_grp_bkg_set(&tv_info, DP_GRP, &grpbkg);
	dp_tv_grp1_win_prority(&tv_info, 0, 1, 2, 3);
}

/**
 * @brief Set TV TimeGenerate.
 * @fn void DP_TV_TG_Set(etvMode mode)
 * @param mode   [in] Specify for TV Resolution mode
 *
 * @return None.
 *
 */
void dp_tv_timegenerate_set(struct stfb_info *info, enum edp_tv_mode mode)
{
	unsigned int reg = 0;

	reg |= Pixel_Full_M1(tv_tg_table[mode].pixel_full_m1)
			| Line_Full_M1(tv_tg_table[mode].line_full_m1);
	__raw_writel(reg, info->dp_tv_regs + DP_TV_TG_SIZE);

	reg = 0;
	reg |= Hsync_BLK(tv_tg_table[mode].h_sync_blk)
			| Hsync_ACT(tv_tg_table[mode].h_sync_act);
	__raw_writel(reg, info->dp_tv_regs + DP_TV_TG_HSYNC);

	reg = 0;
	reg |= Vsync_BLK0(tv_tg_table[mode].v_sync_blk0)
			| Vsync_ACT0(tv_tg_table[mode].v_sync_act0);
	__raw_writel(reg, info->dp_tv_regs + DP_TV_TG_F0VSYNC);

	reg = 0;
	reg |= Vsync_BLK1(tv_tg_table[mode].v_sync_blk1)
			| Vsync_ACT1(tv_tg_table[mode].v_sync_act1);
	__raw_writel(reg, info->dp_tv_regs + DP_TV_TG_F1VSYNC);

	reg = 0;
	reg |= Fsync_BLK0(tv_tg_table[mode].f_sync_blk0)
			| Fsync_ACT0(tv_tg_table[mode].f_sync_act0);
	__raw_writel(reg, info->dp_tv_regs + DP_TV_TG_FSYNC);

	reg = 0;
	reg |= Pixel_Start(tv_tg_table[mode].pixel_start)
			| Fsync_ACT1(tv_tg_table[mode].f_sync_act1);
	__raw_writel(reg, info->dp_tv_regs + DP_TV_TG_FVSNC);

	reg = 0;
	reg |= Frame_Init(tv_tg_table[mode].frame_init)
			| Frame_Finalize(tv_tg_table[mode].frame_finalize);
	__raw_writel(reg, info->dp_tv_regs + DP_TV_TG_INFO_0);

	reg = 0;
	reg |= Field_Init(tv_tg_table[mode].field_init)
			| BUF_Read_Start(tv_tg_table[mode].buf_read_start);
	__raw_writel(reg, info->dp_tv_regs + DP_TV_TG_INFO_1);

	reg = 0;
	reg |= Pre_Line_Start(tv_tg_table[mode].pre_line_start)
			| Pre_Line_End(tv_tg_table[mode].pre_line_end);
	__raw_writel(reg, info->dp_tv_regs + DP_TV_TG_INFO_2);

	reg = 0;
	reg |= De_Pixel_Start(tv_tg_table[mode].de_pixel_start)
			| De_Pixel_End(tv_tg_table[mode].de_pixel_end);
	__raw_writel(reg, info->dp_tv_regs + DP_TV_TG_ACTIVE_PIXEL);

	reg = 0;
	reg |= Active_Line_Start0(tv_tg_table[mode].active_line_start0)
			| Active_Line_End0(tv_tg_table[mode].active_line_end0);
	__raw_writel(reg, info->dp_tv_regs + DP_TV_TG_ACTIVE_0_LINE);

	reg = 0;
	reg |= Active_Line_Start1(tv_tg_table[mode].active_line_start1)
			| Active_Line_End1(tv_tg_table[mode].active_line_end1);
	__raw_writel(reg, info->dp_tv_regs + DP_TV_TG_ACTIVE_1_LINE);

	reg = 0;
	reg |= Field_BLK(tv_tg_table[mode].field_blk)
			| Field_ACT(tv_tg_table[mode].field_act);
	__raw_writel(reg, info->dp_tv_regs + DP_TV_TG_FIELD);

	reg = 0;
	reg |= v_offset0(tv_tg_table[mode].v_offset0)
			| v_offset1(tv_tg_table[mode].v_offset1);
	__raw_writel(reg, info->dp_tv_regs + DP_TV_TG_V_OFFSET);

	reg = 0;
	reg |= h_offset0(tv_tg_table[mode].h_offset);
	__raw_writel(reg, info->dp_tv_regs + DP_TV_TG_H_OFFSET);

}

void dp_tv_path_out_mode(struct stfb_info *info, enum edp_tv_mode mode)
{
	dp_tv_out_default(info);
	dp_tv_vid_colorspace_conversion(info, HD_Std_Range);

	switch (mode) {
	case NTSC:
		dp_tv_hdmi_enc_onoff(info, DP_ON);
		dp_tv_cvbs_enc_onoff(info, DP_OFF);
		dp_tv_bt656_enc_onoff(info, DP_OFF);
		dp_tv_cvbs_lsi_select(info, DP_OFF);
		dp_tv_hv_resolution(info, 720, 240);
		dp_tv_cvbs_format(info, NTSC_M);
		dp_tv_vid_scan_set(info, DP_INTERLACE);
		dp_tv_grp1_scan_set(info, DP_INTERLACE);
		dp_tv_sd_pattern_onoff(info, DP_OFF);
		dp_tv_hd_pattern_interace_progressive(info, 1);
		dp_tv_hd_pattern_framerate(info, 0);
		break;
	case PAL:
		dp_tv_hdmi_enc_onoff(info, DP_ON);
		dp_tv_cvbs_enc_onoff(info, DP_OFF);
		dp_tv_bt656_enc_onoff(info, DP_OFF);
		dp_tv_cvbs_lsi_select(info, DP_OFF);

		dp_tv_hv_resolution(info, 720, 288);
		dp_tv_cvbs_format(info, PAL_BDGH);
		dp_tv_vid_scan_set(info, DP_INTERLACE);
		dp_tv_grp1_scan_set(info, DP_INTERLACE);
		dp_tv_hd_pattern_interace_progressive(info, 1);
		dp_tv_hd_pattern_framerate(info, 1);
		break;
	case RES_480P:
		dp_tv_hdmi_enc_onoff(info, DP_ON);
		dp_tv_cvbs_enc_onoff(info, DP_OFF);
		dp_tv_bt656_enc_onoff(info, DP_OFF);
		dp_tv_hv_resolution(info, 720, 480);
		dp_tv_vid_scan_set(info, DP_PROGRESSIVE);
		dp_tv_grp1_scan_set(info, DP_PROGRESSIVE);
		dp_tv_hd_pattern_interace_progressive(info, 1);
		dp_tv_hd_pattern_framerate(info, 0);
		break;

	case RES_576P:
		dp_tv_hdmi_enc_onoff(info, DP_ON);
		dp_tv_cvbs_enc_onoff(info, DP_OFF);
		dp_tv_bt656_enc_onoff(info, DP_OFF);
		dp_tv_hv_resolution(info, 720, 576);
		dp_tv_vid_scan_set(info, DP_PROGRESSIVE);
		dp_tv_grp1_scan_set(info, DP_PROGRESSIVE);
		dp_tv_hd_pattern_interace_progressive(info, 1);
		dp_tv_hd_pattern_framerate(info, 1);
		break;
	case RES_720P_60:
		dp_tv_hdmi_enc_onoff(info, DP_ON);
		dp_tv_cvbs_enc_onoff(info, DP_OFF);
		dp_tv_bt656_enc_onoff(info, DP_OFF);
		dp_tv_hv_resolution(info, 1280, 720);
		dp_tv_vid_scan_set(info, DP_PROGRESSIVE);

		dp_tv_grp1_scan_set(info, DP_PROGRESSIVE);
		dp_tv_hd_pattern_interace_progressive(info, 1);
		dp_tv_hd_pattern_framerate(info, 0);
		break;
	case RES_720P_60_3D_FP:
		dp_tv_hdmi_enc_onoff(info, DP_ON);
		dp_tv_cvbs_enc_onoff(info, DP_OFF);
		dp_tv_bt656_enc_onoff(info, DP_OFF);
		dp_tv_hv_resolution(info, 1280, 1470);
		dp_tv_vid_scan_set(info, DP_PROGRESSIVE);
		dp_tv_grp1_scan_set(info, DP_PROGRESSIVE);
		dp_tv_hd_pattern_interace_progressive(info, 1);
		dp_tv_hd_pattern_framerate(info, 0);
		break;
	case RES_720P_50:
		dp_tv_hdmi_enc_onoff(info, DP_ON);
		dp_tv_cvbs_enc_onoff(info, DP_OFF);
		dp_tv_bt656_enc_onoff(info, DP_OFF);
		dp_tv_hv_resolution(info, 1280, 720);
		dp_tv_vid_scan_set(info, DP_PROGRESSIVE);
		dp_tv_grp1_scan_set(info, DP_PROGRESSIVE);
		dp_tv_hd_pattern_interace_progressive(info, 1);
		dp_tv_hd_pattern_framerate(info, 1);
		break;
	case RES_1080I_60:
		dp_tv_hdmi_enc_onoff(info, DP_ON);
		dp_tv_cvbs_enc_onoff(info, DP_OFF);
		dp_tv_bt656_enc_onoff(info, DP_OFF);
		dp_tv_hv_resolution(info, 1920, 540);
		dp_tv_vid_scan_set(info, DP_INTERLACE);
		dp_tv_grp1_scan_set(info, DP_INTERLACE);

		dp_tv_bbox_2x(info, DP_ON);

		dp_tv_hd_pattern_interace_progressive(info, 0);
		dp_tv_hd_pattern_framerate(info, 0);
		break;
	case RES_1080I_50:
		dp_tv_hdmi_enc_onoff(info, DP_ON);
		dp_tv_cvbs_enc_onoff(info, DP_OFF);
		dp_tv_hv_resolution(info, 1920, 540);
		dp_tv_vid_scan_set(info, DP_INTERLACE);
		dp_tv_grp1_scan_set(info, DP_INTERLACE);

		dp_tv_bbox_2x(info, DP_ON);

		dp_tv_hd_pattern_interace_progressive(info, 0);
		dp_tv_hd_pattern_framerate(info, 1);
		break;
	case RES_1080P_60:
	case RES_1080P_50:
		dp_tv_hdmi_enc_onoff(info, DP_ON);
		dp_tv_cvbs_enc_onoff(info, DP_OFF);
		dp_tv_bt656_enc_onoff(info, DP_OFF);
		dp_tv_hv_resolution(info, 1920, 1080);
		dp_tv_vid_scan_set(info, DP_PROGRESSIVE);
		dp_tv_grp1_scan_set(info, DP_PROGRESSIVE);

		dp_tv_hd_pattern_interace_progressive(info, 1);
		dp_tv_hd_pattern_framerate(info, 0);
		break;
	case RES_1080P_30:
		dp_tv_hdmi_enc_onoff(info, DP_ON);
		dp_tv_cvbs_enc_onoff(info, DP_OFF);
		dp_tv_bt656_enc_onoff(info, DP_OFF);
		dp_tv_hv_resolution(info, 1920, 1080);
		dp_tv_vid_scan_set(info, DP_PROGRESSIVE);
		dp_tv_grp1_scan_set(info, DP_PROGRESSIVE);

		dp_tv_hd_pattern_interace_progressive(info, 1);
		dp_tv_hd_pattern_framerate(info, 2);
		break;
	case RES_1080P_24:
		dp_tv_hdmi_enc_onoff(info, DP_ON);
		dp_tv_cvbs_enc_onoff(info, DP_OFF);
		dp_tv_bt656_enc_onoff(info, DP_OFF);
		dp_tv_hv_resolution(info, 1920, 1080);
		dp_tv_vid_scan_set(info, DP_PROGRESSIVE);
		dp_tv_grp1_scan_set(info, DP_PROGRESSIVE);

		dp_tv_hd_pattern_interace_progressive(info, 1);
		dp_tv_hd_pattern_framerate(info, 3);
		break;
	case RES_1080P_24_3D_FP:
		dp_tv_hdmi_enc_onoff(info, DP_ON);
		dp_tv_cvbs_enc_onoff(info, DP_OFF);
		dp_tv_bt656_enc_onoff(info, DP_OFF);
		dp_tv_hv_resolution(info, 1920, 2205);
		dp_tv_vid_scan_set(info, DP_PROGRESSIVE);
		dp_tv_grp1_scan_set(info, DP_PROGRESSIVE);

		dp_tv_hd_pattern_interace_progressive(info, 1);
		dp_tv_hd_pattern_framerate(info, 3);
		break;
	case RES_720P_50_3D_FP:
		dp_tv_hdmi_enc_onoff(info, DP_ON);
		dp_tv_cvbs_enc_onoff(info, DP_OFF);
		dp_tv_bt656_enc_onoff(info, DP_OFF);
		dp_tv_hv_resolution(info, 1280, 1470);
		dp_tv_vid_scan_set(info, DP_PROGRESSIVE);
		dp_tv_grp1_scan_set(info, DP_PROGRESSIVE);

		dp_tv_hd_pattern_interace_progressive(info, 1);
		dp_tv_hd_pattern_framerate(info, 0);
		break;
	case RES_720P_50_3D_SBS:
		dp_tv_hdmi_enc_onoff(info, DP_ON);
		dp_tv_cvbs_enc_onoff(info, DP_OFF);
		dp_tv_bt656_enc_onoff(info, DP_OFF);
		dp_tv_hv_resolution(info, 1280, 720);
		dp_tv_vid_scan_set(info, DP_PROGRESSIVE);
		dp_tv_grp1_scan_set(info, DP_PROGRESSIVE);

		dp_tv_hd_pattern_interace_progressive(info, 1);
		dp_tv_hd_pattern_framerate(info, 2);
		break;
	case RES_720P_50_3D_TB:
		dp_tv_hdmi_enc_onoff(info, DP_ON);
		dp_tv_cvbs_enc_onoff(info, DP_OFF);
		dp_tv_bt656_enc_onoff(info, DP_OFF);
		dp_tv_hv_resolution(info, 1280, 720);
		dp_tv_vid_scan_set(info, DP_PROGRESSIVE);
		dp_tv_grp1_scan_set(info, DP_PROGRESSIVE);

		dp_tv_hd_pattern_interace_progressive(info, 1);
		dp_tv_hd_pattern_framerate(info, 1);
		break;
	case RES_720P_60_3D_SBS:
		dp_tv_hdmi_enc_onoff(info, DP_ON);
		dp_tv_cvbs_enc_onoff(info, DP_OFF);
		dp_tv_bt656_enc_onoff(info, DP_OFF);
		dp_tv_hv_resolution(info, 1280, 720);
		dp_tv_vid_scan_set(info, DP_PROGRESSIVE);
		dp_tv_grp1_scan_set(info, DP_PROGRESSIVE);

		dp_tv_hd_pattern_interace_progressive(info, 1);
		dp_tv_hd_pattern_framerate(info, 0);

		break;
	case RES_720P_60_3D_TB:
		dp_tv_hdmi_enc_onoff(info, DP_ON);
		dp_tv_cvbs_enc_onoff(info, DP_OFF);
		dp_tv_bt656_enc_onoff(info, DP_OFF);
		dp_tv_hv_resolution(info, 1280, 720);
		dp_tv_vid_scan_set(info, DP_PROGRESSIVE);
		dp_tv_grp1_scan_set(info, DP_PROGRESSIVE);

		dp_tv_hd_pattern_interace_progressive(info, 1);
		dp_tv_hd_pattern_framerate(info, 0);
		break;
	case RES_1080I_60_3D_SBS:
		dp_tv_hdmi_enc_onoff(info, DP_ON);
		dp_tv_cvbs_enc_onoff(info, DP_OFF);
		dp_tv_bt656_enc_onoff(info, DP_OFF);
		dp_tv_hv_resolution(info, 1920, 540);
		dp_tv_vid_scan_set(info, DP_INTERLACE);
		dp_tv_grp1_scan_set(info, DP_INTERLACE);
		dp_tv_hd_pattern_interace_progressive(info, 0);
		dp_tv_hd_pattern_framerate(info, 0);
		break;
	case RES_1080I_50_3D_SBS:
		dp_tv_hdmi_enc_onoff(info, DP_ON);
		dp_tv_cvbs_enc_onoff(info, DP_OFF);
		dp_tv_bt656_enc_onoff(info, DP_OFF);
		dp_tv_hv_resolution(info, 1920, 540);
		dp_tv_vid_scan_set(info, DP_INTERLACE);
		dp_tv_grp1_scan_set(info, DP_INTERLACE);
		dp_tv_hd_pattern_interace_progressive(info, 0);
		dp_tv_hd_pattern_framerate(info, 1);
		break;
	case RES_1080P_30_3D_SBS:
		dp_tv_hdmi_enc_onoff(info, DP_ON);
		dp_tv_cvbs_enc_onoff(info, DP_OFF);
		dp_tv_bt656_enc_onoff(info, DP_OFF);
		dp_tv_hv_resolution(info, 1920, 1080);
		dp_tv_vid_scan_set(info, DP_PROGRESSIVE);
		dp_tv_grp1_scan_set(info, DP_PROGRESSIVE);
		dp_tv_hd_pattern_interace_progressive(info, 1);
		dp_tv_hd_pattern_framerate(info, 2);
		break;
	case RES_1080P_24_3D_SBS:
		dp_tv_hdmi_enc_onoff(info, DP_ON);
		dp_tv_cvbs_enc_onoff(info, DP_OFF);
		dp_tv_bt656_enc_onoff(info, DP_OFF);
		dp_tv_hv_resolution(info, 1920, 1080);
		dp_tv_vid_scan_set(info, DP_PROGRESSIVE);
		dp_tv_grp1_scan_set(info, DP_PROGRESSIVE);
		dp_tv_hd_pattern_interace_progressive(info, 1);
		dp_tv_hd_pattern_framerate(info, 3);
		break;
	case RES_1080P_24_3D_TB:
		dp_tv_hdmi_enc_onoff(info, DP_ON);
		dp_tv_cvbs_enc_onoff(info, DP_OFF);
		dp_tv_bt656_enc_onoff(info, DP_OFF);
		dp_tv_hv_resolution(info, 1920, 1080);
		dp_tv_vid_scan_set(info, DP_PROGRESSIVE);
		dp_tv_grp1_scan_set(info, DP_PROGRESSIVE);
		dp_tv_hd_pattern_interace_progressive(info, 1);
		dp_tv_hd_pattern_framerate(info, 3);

		break;
	case RES_1080P_30_3D_FP:
		dp_tv_hdmi_enc_onoff(info, DP_ON);
		dp_tv_cvbs_enc_onoff(info, DP_OFF);
		dp_tv_bt656_enc_onoff(info, DP_OFF);
		dp_tv_hv_resolution(info, 1920, 2205);
		dp_tv_vid_scan_set(info, DP_PROGRESSIVE);
		dp_tv_grp1_scan_set(info, DP_PROGRESSIVE);
		dp_tv_hd_pattern_interace_progressive(info, 1);
		dp_tv_hd_pattern_framerate(info, 2);
		break;
	case RES_1080P_30_3D_TB:
		dp_tv_hdmi_enc_onoff(info, DP_ON);
		dp_tv_cvbs_enc_onoff(info, DP_OFF);
		dp_tv_bt656_enc_onoff(info, DP_OFF);
		dp_tv_hv_resolution(info, 1920, 1080);
		dp_tv_vid_scan_set(info, DP_PROGRESSIVE);
		dp_tv_grp1_scan_set(info, DP_PROGRESSIVE);
		dp_tv_hd_pattern_interace_progressive(info, 1);
		dp_tv_hd_pattern_framerate(info, 2);
		break;
	case RES_1080P_50_3D_TB:
		dp_tv_hdmi_enc_onoff(info, DP_ON);
		dp_tv_cvbs_enc_onoff(info, DP_OFF);
		dp_tv_bt656_enc_onoff(info, DP_OFF);
		dp_tv_hv_resolution(info, 1920, 1080);
		dp_tv_vid_scan_set(info, DP_PROGRESSIVE);
		dp_tv_grp1_scan_set(info, DP_PROGRESSIVE);
		dp_tv_hd_pattern_interace_progressive(info, 1);
		dp_tv_hd_pattern_framerate(info, 1);
		break;
	case RES_1080P_60_3D_TB:
		dp_tv_hdmi_enc_onoff(info, DP_ON);
		dp_tv_cvbs_enc_onoff(info, DP_OFF);
		dp_tv_bt656_enc_onoff(info, DP_OFF);
		dp_tv_hv_resolution(info, 1920, 1080);
		dp_tv_vid_scan_set(info, DP_PROGRESSIVE);
		dp_tv_grp1_scan_set(info, DP_PROGRESSIVE);
		dp_tv_hd_pattern_interace_progressive(info, 1);
		dp_tv_hd_pattern_framerate(info, 0);
		break;
	case RES_MAX:
		break;
	}

}

void dp_tv_rgb_ycbcr(struct stfb_info *info, enum edp_onoff onoff)
{
	unsigned int reg = 0;
	reg = __raw_readl(info->dp_tv_regs + DP_TV_PATH_CONTROL);
	if (onoff)
		reg |= RGB2YCBCR;
	else
		reg &= ~RGB2YCBCR;
	__raw_writel(reg, info->dp_tv_regs + DP_TV_PATH_CONTROL);

	reg = 0;
	reg = __raw_readl(info->dp_tv_regs + DP_TV_PATH_TVOUT);
	if (onoff)
		reg |= HDMI_Ycbcr;
	else
		reg &= ~HDMI_Ycbcr;
	__raw_writel(reg, info->dp_tv_regs + DP_TV_PATH_TVOUT);

}

void hdmi_link(struct stfb_info *info, enum edp_tv_mode mode,
		enum tv_10bit_mode inout)
{
	/* SLSI HDMI Tx */
	hdmi_out_select(info, 0);
	hdmi_phy_clock_set(mode, inout);
}

void hdmi_out_select(struct stfb_info *info, unsigned short phy)
{
	unsigned int reg = 0;
	reg = __raw_readl(info->dp_tv_regs + DP_TV_PATH_TVOUT);
	if (phy == 1)
		reg |= HDMIChipSelect;
	else if (phy == 0)
		reg &= ~HDMIChipSelect;
	__raw_writel(reg, info->dp_tv_regs + DP_TV_PATH_TVOUT);

}

void hdmi_phy_clock_set(enum edp_tv_mode mode, enum tv_10bit_mode inout)
{
	enum edp_hdmi_clock freq = phy_freq[mode];
	if (inout == input8_out8 || inout == input10_out8)
		dp_hdmi_phy_clock_set_32n_8bit(freq);
	else
		dp_hdmi_phy_clock_set_32n_10bit(freq);

}

void dp_hdmi_phy_clock_set_32n_8bit(enum edp_hdmi_clock mode)
{
	int iResult = 0;
	unsigned char i = 0, regNo = 0;
	iResult = dp_tv_i2c_write(0x1F, 0x00);
	if (iResult < 0) {
		return;
	}

	if (mode == clock27) {
		for (i = 0, regNo = 1; i <= HDMI_PHY_LAST; i++, regNo++) {
			iResult = dp_tv_i2c_write(regNo, clock_27_8_bit[i]);
			if (iResult < 0) {
				return;
			}
		}
	}

	if (mode == clock74_25) {
		for (i = 0, regNo = 1; i <= HDMI_PHY_LAST; i++, regNo++) {
			iResult = dp_tv_i2c_write(regNo, clock_74_25_8_bit[i]);
			if (iResult < 0) {
				return;
			}
		}
	}

	if (mode == clock74_176) {
		for (i = 0, regNo = 1; i <= HDMI_PHY_LAST; i++, regNo++) {
			iResult = dp_tv_i2c_write(regNo, clock_74_176_8_bit[i]);
			if (iResult < 0) {
				return;
			}
		}
	}

	if (mode == clock148_352) {
		for (i = 0, regNo = 1; i <= HDMI_PHY_LAST; i++, regNo++) {
			iResult = dp_tv_i2c_write(regNo, clock_148_352_8_bit[i]);
			if (iResult < 0) {
				return;
			}
		}
	}

	if (mode == clock148_5) {
		for (i = 0, regNo = 1; i <= HDMI_PHY_LAST; i++, regNo++) {
			iResult = dp_tv_i2c_write(regNo, clock_148_5_8_bit[i]);
			if (iResult < 0) {
				return;
			}
		}

	}
}

void dp_hdmi_phy_clock_set_32n_10bit(enum edp_hdmi_clock mode)
{
	int iResult = 0;
	unsigned char i = 0, regNo = 0;
	iResult = dp_tv_i2c_write(0x1F, 0x00);
	if (iResult < 0) {
		return;
	}

	if (mode == clock27) {
		for (i = 0, regNo = 1; i <= HDMI_PHY_LAST; i++, regNo++) {
			iResult = dp_tv_i2c_write(regNo, clock_27_10_bit[i]);
			if (iResult < 0) {
				return;
			}
		}
	}
	if (mode == clock74_25) {
		for (i = 0, regNo = 1; i <= HDMI_PHY_LAST; i++, regNo++) {
			iResult = dp_tv_i2c_write(regNo, clock_74_25_10_bit[i]);
			if (iResult < 0) {
				return;
			}
		}
	}

	if (mode == clock74_176) {
		for (i = 0, regNo = 1; i <= HDMI_PHY_LAST; i++, regNo++) {
			iResult = dp_tv_i2c_write(regNo, clock_74_176_10_bit[i]);
			if (iResult < 0) {
				return;
			}
		}
	}

	if (mode == clock148_352) {
		for (i = 0, regNo = 1; i <= HDMI_PHY_LAST; i++, regNo++) {
			iResult = dp_tv_i2c_write(regNo, clock_148_352_10_bit[i]);
			if (iResult < 0) {
				return;
			}
		}
	}

	if (mode == clock148_5) {
		for (i = 0, regNo = 1; i <= HDMI_PHY_LAST; i++, regNo++) {
			iResult = dp_tv_i2c_write(regNo, clock_148_5_10_bit[i]);
			if (iResult < 0) {
				return;
			}
		}

	}

}

void dp_10bit_select(struct stfb_info *info, enum tv_10bit_mode mode,
		unsigned short range)
{
	if (mode == input8_out10) {
		dp_tv_vid_10bit_dma_set(info, DP_OFF);
		dp_tv_expan_on(info, DP_ON);
		dp_tv_expan_bitrange(info, range);
		dp_tv_hdmi_10bit_sel(info, DP_ON);

		dp_tv_bkg_chroma_order(info, DP_OFF);
	} else if (mode == input10_out10) {
		dp_tv_vid_10bit_dma_set(info, DP_ON);
		dp_tv_expan_on(info, DP_OFF);
		dp_tv_hdmi_10bit_sel(info, DP_ON);

		dp_tv_bkg_chroma_order(info, DP_ON);
	} else if (mode == input8_out8) {
		dp_tv_vid_10bit_dma_set(info, DP_OFF);
		dp_tv_expan_on(info, DP_OFF);
		dp_tv_hdmi_10bit_sel(info, DP_OFF);

		dp_tv_bkg_chroma_order(info, DP_OFF);
	} else {

		dp_tv_dither_on(info, DP_OFF);
		dp_tv_dither_sel(info, DP_ON);
		dp_tv_hdmi_10bit_sel(info, DP_OFF);

		dp_tv_vid_10bit_dma_set(info, DP_ON);
		dp_tv_expan_on(info, DP_OFF);
		dp_tv_bkg_chroma_order(info, DP_ON);
	}

}

void dp_tv_dither_on(struct stfb_info *info, enum edp_onoff onoff)
{
	unsigned int reg = 0;
	reg = __raw_readl(info->dp_tv_regs + DP_TV_PATH_CONTROL);
	if (onoff)
		reg |= Diter_ON;
	else
		reg &= ~Diter_ON;
	__raw_writel(reg, info->dp_tv_regs + DP_TV_PATH_CONTROL);
}

void dp_tv_dither_sel(struct stfb_info *info, enum edp_onoff onoff)
{
	unsigned int reg = 0;
	reg = __raw_readl(info->dp_tv_regs + DP_TV_PATH_EXPAN);
	if (onoff)
		reg |= Dither_sel;
	else
		reg &= ~Dither_sel;
	__raw_writel(reg, info->dp_tv_regs + DP_TV_PATH_EXPAN);
}

void dp_tv_vid_10bit_dma_set(struct stfb_info *info, enum edp_onoff onoff)
{
	unsigned int reg = 0;
	reg = __raw_readl(info->dp_tv_regs + DP_TV_PATH_CONTROL);
	if (onoff)
		reg |= VID_10BIT;
	else
		reg &= ~VID_10BIT;
	__raw_writel(reg, info->dp_tv_regs + DP_TV_PATH_CONTROL);
}

void dp_tv_expan_on(struct stfb_info *info, enum edp_onoff onoff)
{
	unsigned int reg = 0;
	reg = __raw_readl(info->dp_tv_regs + DP_TV_PATH_CONTROL);
	if (onoff)
		reg |= EXPAN_ON;
	else
		reg &= ~EXPAN_ON;
	__raw_writel(reg, info->dp_tv_regs + DP_TV_PATH_CONTROL);
}

void dp_tv_expan_bitrange(struct stfb_info *info, unsigned short range)
{
	unsigned int reg = 0;
	reg = __raw_readl(info->dp_tv_regs + DP_TV_PATH_EXPAN);
	reg |= EXPAN_BIT_RANGE(range);
	__raw_writel(reg, info->dp_tv_regs + DP_TV_PATH_EXPAN);
}

void dp_tv_hdmi_10bit_sel(struct stfb_info *info, enum edp_onoff onoff)
{
	unsigned int reg = 0;
	reg = __raw_readl(info->dp_tv_regs + DP_TV_PATH_TVOUT);
	if (onoff)
		reg |= BIT10_SEL;
	else
		reg &= ~BIT10_SEL;
	__raw_writel(reg, info->dp_tv_regs + DP_TV_PATH_TVOUT);
}

void dp_tv_bkg_chroma_order(struct stfb_info *info, enum edp_onoff onoff)
{
	unsigned int reg = 0;
	reg = __raw_readl(info->dp_tv_regs + DP_TV_PATH_VIDDMA_M0);
	if (onoff)
		reg |= CHROMA_ORDER;
	else
		reg &= ~CHROMA_ORDER;
	__raw_writel(reg, info->dp_tv_regs + DP_TV_PATH_VIDDMA_M0);
}

void dp_tv_hdmi_enc_onoff(struct stfb_info *info, enum edp_onoff onoff)
{
	unsigned int reg = 0;
	reg = __raw_readl(info->dp_tv_regs + DP_TV_PATH_CONTROL);
	if (onoff)
		reg |= HDMI_ENC;
	else
		reg &= ~HDMI_ENC;
	__raw_writel(reg, info->dp_tv_regs + DP_TV_PATH_CONTROL);
}

void dp_tv_cvbs_enc_onoff(struct stfb_info *info, enum edp_onoff onoff)
{
	unsigned int reg = 0;
	reg = __raw_readl(info->dp_tv_regs + DP_TV_PATH_CONTROL);
	if (onoff)
		reg |= CVBS_ENC;
	else
		reg &= ~CVBS_ENC;
	__raw_writel(reg, info->dp_tv_regs + DP_TV_PATH_CONTROL);
}

void dp_tv_bt656_enc_onoff(struct stfb_info *info, enum edp_onoff onoff)
{
	unsigned int reg = 0;
	reg = __raw_readl(info->dp_tv_regs + DP_TV_PATH_CONTROL);
	if (onoff)
		reg |= BT656_ENC;
	else
		reg &= ~BT656_ENC;
	__raw_writel(reg, info->dp_tv_regs + DP_TV_PATH_CONTROL);
}

void dp_tv_cvbs_lsi_select(struct stfb_info *info, enum edp_onoff onoff)
{
	unsigned int reg = 0;
	reg = __raw_readl(info->dp_tv_regs + DP_TV_PATH_TVOUT);
	if (onoff)
		reg |= CVBS_ENC_SEL;
	else
		reg &= ~CVBS_ENC_SEL;
	__raw_writel(reg, info->dp_tv_regs + DP_TV_PATH_TVOUT);
}

void dp_tv_hv_resolution(struct stfb_info *info, unsigned short xsize,
		unsigned short ysize)
{
	unsigned int reg = 0;
	reg = __raw_readl(info->dp_tv_regs + DP_TV_PATH_VIDIN);
	reg |= (SizeX(xsize) | SizeY(ysize));
	__raw_writel(reg, info->dp_tv_regs + DP_TV_PATH_VIDIN);
}

void dp_tv_vid_scan_set(struct stfb_info *info,
		enum edp_input_img_type scan_type)
{
	unsigned int reg = 0;
	reg = __raw_readl(info->dp_tv_regs + DP_TV_PATH_CONTROL);
	if (scan_type == DP_PROGRESSIVE) {
		reg |= VideoSCAN;
	} else {
		reg &= ~VideoSCAN;
	}

	__raw_writel(reg, info->dp_tv_regs + DP_TV_PATH_CONTROL);
}

void dp_tv_grp1_scan_set(struct stfb_info *info,
		enum edp_input_img_type scan_type)
{
	unsigned int reg = 0;
	reg = __raw_readl(info->dp_tv_regs + DP_TV_PATH_CONTROL);
	if (scan_type == DP_PROGRESSIVE) {
		reg |= GRP1SCAN;
	} else {
		reg &= ~GRP1SCAN;
	}

	__raw_writel(reg, info->dp_tv_regs + DP_TV_PATH_CONTROL);
}

void dp_tv_hd_pattern_interace_progressive(struct stfb_info *info,
		unsigned short mode)
{
	unsigned int reg = 0;
	reg = __raw_readl(info->dp_tv_regs + DP_TV_PATH_TVOUT_PAT_GEN);
	if (mode == DP_PROGRESSIVE) {
		reg |= HD_PAT_INT_PROG;
	} else {
		reg &= ~HD_PAT_INT_PROG;
	}

	__raw_writel(reg, info->dp_tv_regs + DP_TV_PATH_TVOUT_PAT_GEN);
}

void dp_tv_hd_pattern_framerate(struct stfb_info *info, unsigned short data)
{
	unsigned int reg = 0;
	reg = __raw_readl(info->dp_tv_regs + DP_TV_PATH_TVOUT_PAT_GEN);
	reg |= HD_PAT_FRAME_RATE(data);

	__raw_writel(reg, info->dp_tv_regs + DP_TV_PATH_TVOUT_PAT_GEN);
}

void dp_tv_vid_colorspace_conversion(struct stfb_info *info,
		enum tv_cscmode mode)
{
	unsigned int TV_YCbCr2Rgb0 = 0;
	unsigned int TV_YCbCr2Rgb1 = 0;
	unsigned int TV_YCbCr2Rgb2 = 0;
	unsigned int TV_YCbCr2Rgb3 = 0;
	unsigned int TV_YCbCr2Rgb4 = 0;
	unsigned int TV_YCbCr2Rgb5 = 0;
	unsigned int TV_YCbCr2Rgb6 = 0;
	unsigned int TV_YCbCr2Rgb7 = 0;

	unsigned int TV_RGB2YCbCr8 = 0;
	unsigned int TV_RGB2YCbCr9 = 0;
	unsigned int TV_RGB2YCbCr10 = 0;
	unsigned int TV_RGB2YCbCr11 = 0;
	unsigned int TV_RGB2YCbCr12 = 0;
	unsigned int TV_RGB2YCbCr13 = 0;
	unsigned int TV_RGB2YCbCr14 = 0;
	unsigned int TV_RGB2YCbCr15 = 0;
	unsigned int TV_RGB2YCbCr16 = 0;

	if (mode == HD_Std_Range) {

		/* YCbCr to RGB */
		TV_YCbCr2Rgb0 |= MTX33_00(0x200) | MTX33_01(0x314);
		TV_YCbCr2Rgb1 |= MTX33_02(0x000) | MTX33_10(0x200);
		TV_YCbCr2Rgb2 |= MTX33_11(0x0eb) | MTX33_12(0x05e);
		TV_YCbCr2Rgb3 |= MTX33_20(0x200) | MTX33_21(0x000);
		TV_YCbCr2Rgb4 |= MTX33_22(0x3a2);
		TV_YCbCr2Rgb5 |= MTX33_Y_OFFSET(0) | MTX33_CB_OFFSET(0x200);
		TV_YCbCr2Rgb6 |= MTX33_CR_OFFSET(0x200);
		TV_YCbCr2Rgb7 |= underflow_rgb(0x40) | overflow_rgb(0x3ac);

		/* RGB to YCbCr */
		TV_RGB2YCbCr8 |= MTX33_00(0x6d) | MTX33_01(0x16e);
		TV_RGB2YCbCr9 |= MTX33_02(0x24) | MTX33_10(0x3B);
		TV_RGB2YCbCr10 |= MTX33_11(0xC9) | MTX33_12(0x105);
		TV_RGB2YCbCr11 |= MTX33_20(0x105) | MTX33_21(0xED);

		TV_RGB2YCbCr12 |= MTX33_22(0x18);
		TV_RGB2YCbCr13 |= MTX33_Y_OFFSET(0x000) | MTX33_CB_OFFSET(0x200);
		TV_RGB2YCbCr14 |= MTX33_CR_OFFSET(0x200);
		TV_RGB2YCbCr15 |= UNDERFLOW_Y(0x40) | OVERFLOW_Y(0x3AC);
		TV_RGB2YCbCr16 |= UNDERFLOW_C(0x40) | OVERFLOW_C(0x3C0);
	} else if (mode == xvYcc_Range) {

		/* YCbCr to RGB */
		TV_YCbCr2Rgb0 |= MTX33_00(0x254) | MTX33_01(0x396);
		TV_YCbCr2Rgb1 |= MTX33_02(0x000) | MTX33_10(0x254);
		TV_YCbCr2Rgb2 |= MTX33_11(0x111) | MTX33_12(0x06D);
		TV_YCbCr2Rgb3 |= MTX33_20(0x254) | MTX33_21(0x000);
		TV_YCbCr2Rgb4 |= MTX33_22(0x43B);
		TV_YCbCr2Rgb5 |= MTX33_Y_OFFSET(0x40) | MTX33_CB_OFFSET(0x200);
		TV_YCbCr2Rgb6 |= MTX33_CR_OFFSET(0x200);
		TV_YCbCr2Rgb7 |= underflow_rgb(0x800) | overflow_rgb(0x7ff);

		/* RGB to YCbCr */
		TV_RGB2YCbCr8 |= MTX33_00(0x5D) | MTX33_01(0x134);
		TV_RGB2YCbCr9 |= MTX33_02(0x1F) | MTX33_10(0x33);
		TV_RGB2YCbCr10 |= MTX33_11(0xAD) | MTX33_12(0xE0);
		TV_RGB2YCbCr11 |= MTX33_20(0xE0) | MTX33_21(0xCC);

		TV_RGB2YCbCr12 |= MTX33_22(0x14);
		TV_RGB2YCbCr13 |= MTX33_Y_OFFSET(0x40) | MTX33_CB_OFFSET(0x200);
		TV_RGB2YCbCr14 |= MTX33_CR_OFFSET(0x200);
		TV_RGB2YCbCr15 |= UNDERFLOW_Y(0x000) | OVERFLOW_Y(0x3FF);
		TV_RGB2YCbCr16 |= UNDERFLOW_C(0x00) | OVERFLOW_C(0x3FF);
	}
	__raw_writel(TV_YCbCr2Rgb0, info->dp_tv_regs + DP_TV_PATH_VID_CSC0);
	__raw_writel(TV_YCbCr2Rgb1, info->dp_tv_regs + DP_TV_PATH_VID_CSC1);
	__raw_writel(TV_YCbCr2Rgb2, info->dp_tv_regs + DP_TV_PATH_VID_CSC2);
	__raw_writel(TV_YCbCr2Rgb3, info->dp_tv_regs + DP_TV_PATH_VID_CSC3);
	__raw_writel(TV_YCbCr2Rgb4, info->dp_tv_regs + DP_TV_PATH_VID_CSC4);
	__raw_writel(TV_YCbCr2Rgb5, info->dp_tv_regs + DP_TV_PATH_VID_CSC5);
	__raw_writel(TV_YCbCr2Rgb6, info->dp_tv_regs + DP_TV_PATH_VID_CSC6);
	__raw_writel(TV_YCbCr2Rgb7, info->dp_tv_regs + DP_TV_PATH_VID_CSC7);

	__raw_writel(TV_RGB2YCbCr8, info->dp_tv_regs + DP_TV_PATH_GRP_CSC0);
	__raw_writel(TV_RGB2YCbCr9, info->dp_tv_regs + DP_TV_PATH_GRP_CSC1);
	__raw_writel(TV_RGB2YCbCr10, info->dp_tv_regs + DP_TV_PATH_GRP_CSC2);
	__raw_writel(TV_RGB2YCbCr11, info->dp_tv_regs + DP_TV_PATH_GRP_CSC3);
	__raw_writel(TV_RGB2YCbCr12, info->dp_tv_regs + DP_TV_PATH_GRP_CSC4);
	__raw_writel(TV_RGB2YCbCr13, info->dp_tv_regs + DP_TV_PATH_GRP_CSC5);
	__raw_writel(TV_RGB2YCbCr14, info->dp_tv_regs + DP_TV_PATH_GRP_CSC6);
	__raw_writel(TV_RGB2YCbCr15, info->dp_tv_regs + DP_TV_PATH_GRP_CSC7);
	__raw_writel(TV_RGB2YCbCr16, info->dp_tv_regs + DP_TV_PATH_GRP_CSC8);

}

void dp_tv_mode_global_variable_set(enum edp_tv_mode mode)
{
	g_mode = mode;	
}

/**
 * @brief   to set TV Mode
 * @param   mode[in] NTSC,PAL,480P,576P,720P_60,720P_50,1080I_60,1080I_50·Î<br>
 *
 * @return   void
 *
 * @note     DP_TV_VID_Init(void) should be called
 *
 */
void dp_tv_mode_set(enum edp_tv_mode mode, enum edp_onoff onoff, enum tv_10bit_mode inout)
{
	g_mode = mode;	
	dp_tv_timegenerate_set(&tv_info, mode);
	dp_tv_path_out_mode(&tv_info, mode);

	/*DP RGB/Ycbcr Select*/
	dp_tv_rgb_ycbcr(&tv_info, onoff);

	/*DP bit select*/
	dp_10bit_select(&tv_info, inout, 0);

	hdmi_link(&tv_info, mode, inout);
}

void d4_dp_tv_grp1_set_layer_info(struct grp_layer_info *grpinfo)
{
	dp_tv_grp1_set_layer_info(&tv_info, grpinfo);
}

void dp_tv_grp1_set_layer_info(struct stfb_info *info,
		struct grp_layer_info *grpinfo)
{
	struct stdp_argb bkg;

	bkg.DP_A = grpinfo->background_color.DP_A;
	bkg.DP_R = grpinfo->background_color.DP_R;
	bkg.DP_G = grpinfo->background_color.DP_G;
	bkg.DP_B = grpinfo->background_color.DP_B;

	dp_tv_set_stride(info, DP_GRP, grpinfo->stride);
	dp_tv_grp_bkg_set(info, DP_GRP, &bkg);

}

/**
 * @brief video layer settings
 * @fn void d4_dp_tv_vid_set_layer(struct stdp_video_layer_info *layer)
 * @param vidinfo[in], struct stdp_video_layer_info(for details please refer linux/include/video/drime4/d4_dp_type.h
 * @return
 *
 * @note	sets the stride & background color
 */
void d4_dp_tv_vid_set_layer(struct stdp_video_layer_info *vidinfo)
{
	dp_tv_vid_set_layer(vidinfo);
}

void dp_tv_vid_set_layer(struct stdp_video_layer_info *vidinfo)
{
	struct stdp_ycbcr vidBkg;

	if (M_TV_Mode < RES_480P) {
		vidBkg.DP_Y = 0x0;
		vidBkg.DP_Cb = 0x80;
		vidBkg.DP_Cr = 0x80;
	} else {
		vidBkg.DP_Y = vidinfo->background_color.DP_Y;
		vidBkg.DP_Cb = vidinfo->background_color.DP_Cb;
		vidBkg.DP_Cr = vidinfo->background_color.DP_Cr;

	}

	dp_tv_set_stride(&tv_info, DP_VIDEO, vidinfo->stride);
	dp_tv_bkg_color_set(&tv_info, &vidBkg);

}

int dp_tv_set_stride(struct stfb_info *info, enum edp_layer layer,
		unsigned int stride_value)
{
	unsigned int reg = 0;
	unsigned int stride_set_val = 0;

	if ((stride_value % 8) != 0) {
		return -1;
	}

	if (stride_value == 0) {
		stride_value = 16384;
	}

	if (stride_value > 16384) {
		return -1;
	}

	stride_set_val = stride_value / 8;

	reg = __raw_readl(info->dp_tv_regs + DP_TV_PATH_STRIDE);

	if (layer == DP_VIDEO)
		D4_DP_TV_PATH_VID_STRIDE(reg, stride_set_val);
	else
		D4_DP_TV_PATH_GRP_STRIDE(reg, stride_set_val);

	__raw_writel(reg, info->dp_tv_regs + DP_TV_PATH_STRIDE);

	return 0;
}

void dp_sd_ycbcr_range_check(struct ycbcr input, struct stdp_ycbcr *output)
{
	output->DP_Y = input.y;
	if (input.y > 0x90)
		output->DP_Y = 0x90;

	if (input.cb >= 0x80 && input.cr >= 0x80) {
		if (input.y < 0x20)
			output->DP_Y = 0x20;
	} else {
		if (input.y < 0x40)
			output->DP_Y = 0x40;
	}

	output->DP_Cb = input.cb;
	if (input.cb > 0xb0)
		output->DP_Cb = 0xb0;
	if (input.cb < 0x20)
		output->DP_Cb = 0x20;

	output->DP_Cr = input.cr;
	if (input.cr > 0xc0)
		output->DP_Cr = 0xc0;
	if (input.cr < 0x20)
		output->DP_Cr = 0x20;

}

enum scan_type d4_dp_tv_vid_scan_get(void)
{
	return dp_tv_vid_scan_get();
}

enum scan_type dp_tv_vid_scan_get(void)
{
	unsigned int reg = 0;
	reg = __raw_readl(tv_info.dp_tv_regs + DP_TV_PATH_CONTROL);
	if (reg & 0x01)
		return 1;
	else
		return 0;
}

enum scan_type d4_dp_tv_grp_scan_get()
{
	return dp_tv_grp_scan_get();
}

enum scan_type dp_tv_grp_scan_get(void)
{
	unsigned int reg = 0;
	reg = __raw_readl(tv_info.dp_tv_regs + DP_TV_PATH_CONTROL);
	if (reg & 0x01)
		return 1;
	else
		return 0;
}

/**
 * @brief   TV outputpath clock enable
 * @        TV system On/Off (Power On/Off)
 * @fn
 * @param   edp_onoff: On/Off
 * @return  void
 *
 * @note
 */
void dp_tv_global_clock_onoff(struct stfb_info *info, enum edp_onoff onoff)
{

}

/**
 * @brief   TV outputpath clock enable
 * @        TV system On/Off (Power On/Off)
 * @fn
 * @param   edp_tv_clock: TV Clock,Composite Clock, HDMI Clock
 * @param   edp_onoff: On/Off
 * @return  void
 *
 * @note
 */
void dp_tv_clock_onoff(struct stfb_info *info, enum edp_tv_clock clock,
		enum edp_onoff onoff)
{
	unsigned int reg = 0;

	reg = __raw_readl(info->dp_global_regs + dp_clk_en);

	if (clock == tv_clock) {
		D4_DP_COMMON_TV_CLK_EN(reg, onoff);
	} else if (clock == tv_sys_clock) {
		D4_DP_COMMON_SYS_CLK_EN(reg, onoff);
	} else if (clock == hdmi_clock) {
		D4_DP_COMMON_HDMI_VID_CLK_EN(reg, onoff);
	} else if (clock == hdmi_sys_clock) {
		D4_DP_COMMON_HDMI_SYS_CLK_EN(reg, onoff);
	} else if (clock == hdmi_pixel_clock) {
		D4_DP_COMMON_HDMI_PIXEL_CLK_EN(reg, onoff);
	} else if (clock == hdmi_tmds_clock) {
		D4_DP_COMMON_HDMI_TMDS_CLK_EN(reg, onoff);
	} else if (clock == hdmi_reference_clock) {
		D4_DP_COMMON_HDMI_REF_CLK_EN(reg, onoff);
	}

	__raw_writel(reg, info->dp_global_regs + dp_clk_en);
}

void dp_tv_hdmi_phy_onoff(struct stfb_info *info, enum edp_onoff onoff)
{
	unsigned char set_value;

	if (onoff) {
		/* HDMI PHY Power */
		__raw_writel(0x00, info->dp_global_regs + dp_pwr_ctrl);

		/* Phy reset release */
		set_value = __raw_readl(DRIME4_VA_RESET_CTRL);

		set_value |= (0x1 << 16);
		__raw_writel(set_value, DRIME4_VA_RESET_CTRL);

		set_value = __raw_readl(DRIME4_VA_RESET_CTRL);

		set_value &= ~(0x1 << 16);

		__raw_writel(set_value, DRIME4_VA_RESET_CTRL);	
	} else {
		/* HDMI PHY Power */
		__raw_writel(0x01, info->dp_global_regs + dp_pwr_ctrl);
	}
		
}

void dp_global_nlc_clock_onoff(struct stfb_info *info, enum edp_onoff onoff)
{
	unsigned int reg = 0;
	reg = __raw_readl(info->dp_global_regs + dp_clk_en);
	D4_DP_COMMON_NLC_VID_CLK_EN(reg, onoff);
	D4_DP_COMMON_NLC_GRP_CLK_EN(reg, onoff);
	__raw_writel(reg, info->dp_global_regs + dp_clk_en);
}

/**
 * @brief Set TV operation start
 * @fn void DP_TV_Start_OnOff(eDPOnOff OnOff)
 * @param mode   [in] Tv Start On/Off
 *
 * @return None.
 *
 */
void dp_tv_start_onoff(struct stfb_info *info, enum edp_onoff onoff)
{
	unsigned int reg = 0;
	reg = __raw_readl(info->dp_global_regs + dp_start_update);
	D4_DP_COMMON_TV_ON(reg, onoff);
	__raw_writel(reg, info->dp_global_regs + dp_start_update);

}

/**
 * @brief Set TV Register auto update
 * @fn void DP_TV_Auto_Update_OnOff(eDPOnOff OnOff)
 * @param mode   [in] Set On/Off
 *
 * @return None.
 *
 */
void dp_tv_auto_update_onoff(struct stfb_info *info, enum edp_onoff onoff)
{
	unsigned int reg = 0;
	reg = __raw_readl(info->dp_global_regs + dp_start_update);
	D4_DP_COMMON_TV_AUTO(reg, onoff);
	__raw_writel(reg, info->dp_global_regs + dp_start_update);

}

/**
 * @brief Set TV Register manual update
 * @fn void DP_TV_Manual_Update_OnOff(eDPOnOff OnOff)
 * @param mode   [in] Set On/Off
 *
 * @return None.
 *
 */
void dp_tv_manual_update_onoff(struct stfb_info *info, enum edp_onoff onoff)
{
	unsigned int reg = 0;
	reg = __raw_readl(info->dp_global_regs + dp_start_update);
	D4_DP_COMMON_TV_MANUAL(reg, onoff);
	__raw_writel(reg, info->dp_global_regs + dp_start_update);

}

/**
 * @brief Set TV Register update
 * @fn void DP_TV_Register_Update(void)
 * @param None.
 *
 * @return None.
 *
 */
void dp_tv_register_update(struct stfb_info *info)
{
	dp_tv_auto_update_onoff(info, DP_ON);
	dp_tv_manual_update_onoff(info, DP_OFF);
	dp_tv_start_onoff(info, DP_ON);
	dp_tv_manual_update_onoff(info, DP_ON);
	dp_tv_manual_update_onoff(info, DP_OFF);
}

/**
 * @brief Set Video DMA Controller
 * @fn void DP_TV_VIDMAC_OnOff(eDPOnOff OnOff)
 * @param mode   [in] DMA On/Off
 *
 * @return None.
 */
void dp_tv_vidmac_onoff(struct stfb_info *info, enum edp_onoff onoff)
{
	unsigned int reg = 0;
	reg = __raw_readl(info->dp_tv_regs + DP_TV_PATH_CONTROL);
	if (onoff)
		reg |= DMAC;
	else
		reg &= ~DMAC;

	__raw_writel(reg, info->dp_tv_regs + DP_TV_PATH_CONTROL);

}

/**
 * @brief video background set
 * @fn void d4_dp_tv_video_background(struct stdp_ycbcr *color)
 * @param *color[in] struct stdp_ycbcr,  for details please refer linux/include/video/drime4/d4_dp_type.h
 * @return
 *
 * @note
 */
void d4_dp_tv_video_background(struct stdp_ycbcr *color)
{
	dp_tv_bkg_color_set(&tv_info, color);
}

void dp_tv_bkg_color_set(struct stfb_info *info, struct stdp_ycbcr *color)
{
	unsigned int reg = 0;
	D4_DP_TV_PATH_BKG_Y(reg, color->DP_Y);
	D4_DP_TV_PATH_BKG_Cb(reg, color->DP_Cb);
	D4_DP_TV_PATH_BKG_Cr(reg, color->DP_Cr);
	__raw_writel(reg, info->dp_tv_regs + DP_TV_PATH_VIDDMA_M2);
}

/**
 * @brief the dp tv boundary box colortable select and the table set a color
 * @fn void d4_dp_tv_boundarybox_table_color(enum edp_bb_color_table table,    struct stdp_rgb *rgb_info)
 * @param table[in] boundary box color table select
 * @param *rgb_info[in] table color set
 * @return
 *
 * @note
 */
void d4_dp_tv_boundarybox_table_color(enum edp_bb_color_table table,
		struct stdp_rgb *rgb_info)
{
	dp_tv_vid_set_bb_color_table(&tv_info, table, rgb_info);
}

void d4_dp_tv_boundarybox_info_set(struct stbb_info *bb_info)
{
	dp_tv_boundarybox_info_set(&tv_info, bb_info);

}

void dp_tv_boundarybox_info_set(struct stfb_info *info,
		struct stbb_info *bb_info)
{
	struct stdp_rgb bb_out_color;
	bb_out_color.DP_R = 0;
	bb_out_color.DP_G = 0;
	bb_out_color.DP_B = 0;

	dp_tv_set_bb_width(&tv_info, 4, 4);
	dp_tv_set_bb_alpha(&tv_info, 0xf0);
	dp_tv_set_bb_outline_color(&tv_info, &bb_out_color);

}

void dp_tv_set_bb_width(struct stfb_info *info, unsigned char h_width,
		unsigned char v_width)
{
	unsigned int reg = 0;
	reg = __raw_readl(info->dp_tv_regs + DP_TV_VID_PATH_BB_ALPHA);
	reg |= (BB_H_WIDTH(h_width) | BB_V_WIDTH(v_width));
	__raw_writel(reg, info->dp_tv_regs + DP_TV_VID_PATH_BB_ALPHA);
}

void dp_tv_set_bb_alpha(struct stfb_info *info, unsigned char alpha)
{
	unsigned int reg = 0;
	reg = __raw_readl(info->dp_tv_regs + DP_TV_VID_PATH_BB_ALPHA);
	reg |= BB_ALPHA(alpha);
	__raw_writel(reg, info->dp_tv_regs + DP_TV_VID_PATH_BB_ALPHA);
}

void dp_tv_set_bb_outline_color(struct stfb_info *info,
		struct stdp_rgb *bb_out_color)
{
	unsigned int reg = 0;
	reg = __raw_readl(info->dp_tv_regs + DP_TV_VID_PATH_BB_BOUNDARY_COLOR);
	reg |= BB_BOUNDAR_COLOR_B(bb_out_color->DP_B)
			| BB_BOUNDAR_COLOR_G(bb_out_color->DP_G)
			| BB_BOUNDAR_COLOR_R(bb_out_color->DP_R);
	__raw_writel(reg, info->dp_tv_regs + DP_TV_VID_PATH_BB_BOUNDARY_COLOR);
}

void dp_tv_set_bb_mode(struct stfb_info *info, enum edp_bb_box window,
		enum edp_bb_style_set style)
{
	unsigned int reg = 0;
	reg = __raw_readl(info->dp_tv_regs + DP_TV_VID_PATH_BB_ON);

	if (window == 0)
		reg |= BB_MODE_ON0 | (style);
	else if (window == 1)
		reg |= BB_MODE_ON1 | (style);
	else if (window == 2)
		reg |= BB_MODE_ON2 | (style);
	else if (window == 3)
		reg |= BB_MODE_ON3 | (style);
	else if (window == 4)
		reg |= BB_MODE_ON4 | (style);
	else if (window == 5)
		reg |= BB_MODE_ON5 | (style);
	else if (window == 6)
		reg |= BB_MODE_ON6 | (style);
	else if (window == 7)
		reg |= BB_MODE_ON7 | (style);
	else if (window == 8)
		reg |= BB_MODE_ON8 | (style);
	else if (window == 9)
		reg |= BB_MODE_ON9 | (style);
	else if (window == 10)
		reg |= BB_MODE_ON10 | (style);
	else if (window == 11)
		reg |= BB_MODE_ON11 | (style);
	else if (window == 12)
		reg |= BB_MODE_ON12 | (style);
	else if (window == 13)
		reg |= BB_MODE_ON13 | (style);
	else if (window == 14)
		reg |= BB_MODE_ON14 | (style);
	else if (window == 15)
		reg |= BB_MODE_ON15 | (style);

	__raw_writel(reg, info->dp_tv_regs + DP_TV_VID_PATH_BB_ON);
}

void dp_tv_vid_set_bb_color_table(struct stfb_info *info,
		enum edp_bb_color_table table, struct stdp_rgb *rgb_info)
{
	unsigned int index, reg = 0;
	reg = __raw_readl(info->dp_tv_regs + DP_TV_VID_PATH_BB_COLOR00);
	reg |= (BB_COLOR0_B(rgb_info->DP_B) | BB_COLOR0_G(rgb_info->DP_G)
			| BB_COLOR0_R(rgb_info->DP_R));
	index = table * 0x04;
	__raw_writel(reg, info->dp_tv_regs + DP_TV_VID_PATH_BB_COLOR00 + index);
}

void d4_dp_tv_boundarybox_onoff(struct stbb_onoff *bb_onoff)
{
	dp_tv_set_bb_display_area(&tv_info, bb_onoff->bb_win, bb_onoff->area);
	dp_tv_boundarybox_onoff(&tv_info, bb_onoff->bb_win, bb_onoff->onoff);
}

void dp_tv_set_bb_display_area(struct stfb_info *info, enum edp_bb_box window,
		struct stdp_display_area area)
{
	unsigned int index, reg1 = 0, reg2 = 0;

	reg1 |= ((START0(area.H_Start)) | (END0(area.H_Start + area.H_Size)));
	reg2 |= ((START0(area.V_Start)) | (END0(area.V_Start + area.V_Size)));

	if (window < 4) {
		index = window * 0x20;
		__raw_writel(reg1,
				info->dp_tv_regs + DP_TV_VID_PATH_BB_H_STARTEND0 + index);
		__raw_writel(reg2,
				info->dp_tv_regs + DP_TV_VID_PATH_BB_V_STARTEND0 + index);
	} else {
		index = (window - 4) * 0x08;
		__raw_writel(reg1,
				info->dp_tv_regs + DP_TV_VID_PATH_BB_H_STARTEND4 + index);
		__raw_writel(reg2,
				info->dp_tv_regs + DP_TV_VID_PATH_BB_V_STARTEND4 + index);

	}

}

void dp_tv_boundarybox_onoff(struct stfb_info *info, enum edp_bb_box boxwin,
		enum edp_onoff onoff)
{
	unsigned int reg = 0;
	reg = __raw_readl(info->dp_tv_regs + DP_TV_VID_PATH_BB_ON);

	if (boxwin == DP_BB_00) {
		if (onoff)
			reg |= BOUNDARY_ON0;
		else
			reg &= ~BOUNDARY_ON0;
	} else if (boxwin == DP_BB_01) {
		if (onoff)
			reg |= BOUNDARY_ON1;
		else
			reg &= ~BOUNDARY_ON1;
	} else if (boxwin == DP_BB_02) {
		if (onoff)
			reg |= BOUNDARY_ON2;
		else
			reg &= ~BOUNDARY_ON2;

	} else if (boxwin == DP_BB_03) {
		if (onoff)
			reg |= BOUNDARY_ON3;
		else
			reg &= ~BOUNDARY_ON3;
	} else if (boxwin == DP_BB_04) {
		if (onoff)
			reg |= BOUNDARY_ON4;
		else
			reg &= ~BOUNDARY_ON4;
	} else if (boxwin == DP_BB_05) {
		if (onoff)
			reg |= BOUNDARY_ON5;
		else
			reg &= ~BOUNDARY_ON5;
	} else if (boxwin == DP_BB_06) {
		if (onoff)
			reg |= BOUNDARY_ON6;
		else
			reg &= ~BOUNDARY_ON6;
	} else if (boxwin == DP_BB_07) {
		if (onoff)
			reg |= BOUNDARY_ON7;
		else
			reg &= ~BOUNDARY_ON7;
	} else if (boxwin == DP_BB_08) {
		if (onoff)
			reg |= BOUNDARY_ON8;
		else
			reg &= ~BOUNDARY_ON8;
	} else if (boxwin == DP_BB_09) {
		if (onoff)
			reg |= BOUNDARY_ON9;
		else
			reg &= ~BOUNDARY_ON9;
	} else if (boxwin == DP_BB_10) {
		if (onoff)
			reg |= BOUNDARY_ON10;
		else
			reg &= ~BOUNDARY_ON10;
	} else if (boxwin == DP_BB_11) {
		if (onoff)
			reg |= BOUNDARY_ON11;
		else
			reg &= ~BOUNDARY_ON11;
	} else if (boxwin == DP_BB_12) {
		if (onoff)
			reg |= BOUNDARY_ON12;
		else
			reg &= ~BOUNDARY_ON12;
	} else if (boxwin == DP_BB_13) {
		if (onoff)
			reg |= BOUNDARY_ON13;
		else
			reg &= ~BOUNDARY_ON13;
	} else if (boxwin == DP_BB_14) {
		if (onoff)
			reg |= BOUNDARY_ON14;
		else
			reg &= ~BOUNDARY_ON14;
	} else if (boxwin == DP_BB_15) {
		if (onoff)
			reg |= BOUNDARY_ON15;
		else
			reg &= ~BOUNDARY_ON15;
	}
	__raw_writel(reg, info->dp_tv_regs + DP_TV_VID_PATH_BB_ON);

}

void dp_tv_bb_init(struct stfb_info *info)
{
	unsigned int reg = 0;
	reg = __raw_readl(info->dp_tv_regs + DP_TV_VID_PATH_BB_ON);
	reg |= 0;
	__raw_writel(reg, info->dp_tv_regs + DP_TV_VID_PATH_BB_ON);
}

void dp_tv_y_8burst_onoff(struct stfb_info *info, enum edp_onoff onoff)
{
	unsigned int reg = 0;
	reg = __raw_readl(info->dp_tv_regs + DP_TV_PATH_VIDDMA_M1);
	if (onoff)
		reg |= Y_8BURST;
	else
		reg &= ~Y_8BURST;

	__raw_writel(reg, info->dp_tv_regs + DP_TV_PATH_VIDDMA_M1);

}

void dp_tv_y_moalen_set(struct stfb_info *info, unsigned int value)
{
	unsigned int reg = 0;
	reg = __raw_readl(info->dp_tv_regs + DP_TV_PATH_VIDDMA_M1);
	reg |= Y_MOALEN(value);
	__raw_writel(reg, info->dp_tv_regs + DP_TV_PATH_VIDDMA_M1);

}

void dp_tv_c_8burst_onoff(struct stfb_info *info, enum edp_onoff onoff)
{
	unsigned int reg = 0;
	reg = __raw_readl(info->dp_tv_regs + DP_TV_PATH_VIDDMA_M1);
	if (onoff)
		reg |= C_8BURST;
	else
		reg &= ~C_8BURST;

	__raw_writel(reg, info->dp_tv_regs + DP_TV_PATH_VIDDMA_M1);

}

void dp_tv_c_moalen_set(struct stfb_info *info, unsigned int value)
{
	unsigned int reg = 0;
	reg = __raw_readl(info->dp_tv_regs + DP_TV_PATH_VIDDMA_M1);
	reg |= C_MOALEN(value);
	__raw_writel(reg, info->dp_tv_regs + DP_TV_PATH_VIDDMA_M1);

}

void dp_tv_g1_8burst_onoff(struct stfb_info *info, enum edp_onoff onoff)
{
	unsigned int reg = 0;
	reg = __raw_readl(info->dp_tv_regs + DP_TV_PATH_VIDDMA_M1);
	if (onoff)
		reg |= G1_8BURST;
	else
		reg &= ~G1_8BURST;

	__raw_writel(reg, info->dp_tv_regs + DP_TV_PATH_VIDDMA_M1);

}

void dp_tv_g1_moalen_set(struct stfb_info *info, unsigned int value)
{
	unsigned int reg = 0;
	reg = __raw_readl(info->dp_tv_regs + DP_TV_PATH_VIDDMA_M1);
	reg |= G1_MOALEN(value);
	__raw_writel(reg, info->dp_tv_regs + DP_TV_PATH_VIDDMA_M1);

}

void dp_tv_out_hrz_filter_on(struct stfb_info *info, enum edp_layer layer,
		enum edp_filter filer_value)
{
	unsigned int reg = 0;

	if (hrz_filter_set[filer_value].Tap0_Coef < 0) {
		reg |= Sign0;
		reg |= PRECOEF0(-1 * hrz_filter_set[filer_value].Tap0_Coef);
	} else {
		reg &= ~Sign0;
		reg |= PRECOEF0(hrz_filter_set[filer_value].Tap0_Coef);
	}

	if (hrz_filter_set[filer_value].Tap1_Coef < 0) {
		reg |= Sign1;
		reg |= PRECOEF1(-1 * hrz_filter_set[filer_value].Tap1_Coef);
	} else {
		reg &= ~Sign1;
		reg |= PRECOEF1(hrz_filter_set[filer_value].Tap1_Coef);
	}

	reg |= PRECOEF2(hrz_filter_set[filer_value].Tap2_Coef);

	if (hrz_filter_set[filer_value].Tap3_Coef < 0) {
		reg |= Sign3;
		reg |= PRECOEF3(-1 * hrz_filter_set[filer_value].Tap3_Coef);
	} else {
		reg &= ~Sign3;
		reg |= PRECOEF3(hrz_filter_set[filer_value].Tap3_Coef);
	}

	if (hrz_filter_set[filer_value].Tap4_Coef < 0) {
		reg |= Sign4;
		reg |= PRECOEF4(-1 * hrz_filter_set[filer_value].Tap4_Coef);
	} else {
		reg &= ~Sign4;
		reg |= PRECOEF4(hrz_filter_set[filer_value].Tap4_Coef);
	}

	reg |= POSTCOEF(hrz_filter_set[filer_value].Post_Coef);

	if (layer == DP_VIDEO) {
		dp_tv_hrzflt_onoff(info, DP_ON);
		__raw_writel(reg, info->dp_tv_regs + DP_TV_PATH_VIDHRZFLT);
	} else if (layer == DP_GRP) {
		dp_tv_grp1hrzflt_onoff(info, DP_ON);
		__raw_writel(reg, info->dp_tv_regs + DP_TV_GRP_PATH1_HRZFILT);

	}

}

void dp_tv_hrzflt_onoff(struct stfb_info *info, enum edp_onoff onoff)
{
	unsigned int reg = 0;
	reg = __raw_readl(info->dp_tv_regs + DP_TV_PATH_CONTROL);
	if (onoff)
		reg |= HRZFLT;
	else
		reg &= ~HRZFLT;
	__raw_writel(reg, info->dp_tv_regs + DP_TV_PATH_CONTROL);

}

void dp_tv_grp1hrzflt_onoff(struct stfb_info *info, enum edp_onoff onoff)
{
	unsigned int reg = 0;
	reg = __raw_readl(info->dp_tv_regs + DP_TV_PATH_CONTROL);
	if (onoff)
		reg |= GRP1HRZFLT;
	else
		reg &= ~GRP1HRZFLT;
	__raw_writel(reg, info->dp_tv_regs + DP_TV_PATH_CONTROL);

}

void dp_tv_out_hrz_filter_off(struct stfb_info *info, enum edp_layer layer)
{
	if (layer == DP_GRP)
		dp_tv_grp1hrzflt_onoff(info, DP_OFF);
	else if (layer == DP_VIDEO)
		dp_tv_hrzflt_onoff(info, DP_OFF);

}

void d4_tv_hrzflt_onoff(enum edp_layer layer, enum edp_filter filter)
{
	if (filter == DP_BYPASS) {
		dp_tv_out_hrz_filter_on(&tv_info, layer, DP_BYPASS);
		
		if (layer == DP_GRP) {
			dp_tv_grp1hrzflt_onoff(&tv_info, DP_OFF);
		} else {
			dp_tv_hrzflt_onoff(&tv_info, DP_OFF);
		}
	} else {
		dp_tv_out_hrz_filter_on(&tv_info, layer, filter);
	}
}

void dp_tv_grpmix_onoff(struct stfb_info *info, enum edp_onoff onoff)
{
	unsigned int reg = 0;
	reg = __raw_readl(info->dp_tv_regs + DP_TV_PATH_CONTROL);
	if (onoff)
		reg |= GRPMIX0;
	else
		reg &= ~GRPMIX0;
	__raw_writel(reg, info->dp_tv_regs + DP_TV_PATH_CONTROL);

}

/**
 * @brief  TV OUTpath
 * @fn     void DP_TV_Out_Default(void)
 * @param
 *
 * @return none
 *
 * @note
 */
void dp_tv_out_default(struct stfb_info *info)
{

	dp_tv_cy_delay_adj(info, 0, 0);
	dp_tv_pedestal_onoff(info, DP_OFF);
	dp_tv_mono_onoff(info, DP_OFF);
	dp_tv_c_filter_onoff(info, DP_ON);
	dp_tv_y_filter_onoff(info, DP_ON);
	dp_tv_lock_mode_onoff(info, DP_OFF);
	dp_tv_sd_pattern_sync_match(info, DP_OFF);
	dp_tv_sd_pattern_onoff(info, DP_OFF);
	dp_tv_cvbs_format(info, PAL_BDGH);
	dp_tv_sync_inv_onoff(info, 0, DP_OFF);
	dp_tv_sync_inv_onoff(info, 1, DP_OFF);
	dp_tv_sync_inv_onoff(info, 2, DP_OFF);
	dp_tv_bbox_2x(info, DP_OFF);
}

/**
 * @brief Set CVBS Y/C Delay Time
 * @fn  void DP_TV_CY_Delay_ADJ(Uint8 cDelay, Uint8 yDelay)
 * @param mode   [in] set C Delay
 * @param mode   [in] set Y Delay
 *
 * @return None.
 *
 */
void dp_tv_cy_delay_adj(struct stfb_info *info, unsigned int cdelay,
		unsigned int ydelay)
{
	unsigned int reg = 0;
	reg = __raw_readl(info->dp_tv_regs + DP_TV_PATH_TVOUT);
	reg |= CdelayADJ(cdelay) | YdelayADJ(ydelay);
	__raw_writel(reg, info->dp_tv_regs + DP_TV_PATH_TVOUT);

}

/**
 * @brief  Pedestal on
 * @fn  void DP_TV_Pedestal_OnOff(eDPOnOff OnOff)
 * @param mode   [in] pedestal On/Off
 *
 * @return None.
 *
 */
void dp_tv_pedestal_onoff(struct stfb_info *info, enum edp_onoff onoff)
{
	unsigned int reg = 0;
	reg = __raw_readl(info->dp_tv_regs + DP_TV_PATH_TVOUT);
	if (onoff)
		reg |= PEDESTAL;
	else
		reg &= ~PEDESTAL;
	__raw_writel(reg, info->dp_tv_regs + DP_TV_PATH_TVOUT);

}

/**
 * @brief  Set CVBS Mono / Color Display :0 = Color, 1=mono
 * @fn  void DP_TV_Mono_OnOff(eDPOnOff OnOff)
 * @param mode   [in] Specify mono or color
 *
 * @return None.
 *
 */
void dp_tv_mono_onoff(struct stfb_info *info, enum edp_onoff onoff)
{
	unsigned int reg = 0;
	reg = __raw_readl(info->dp_tv_regs + DP_TV_PATH_TVOUT);
	if (onoff)
		reg |= OnlyY;
	else
		reg &= ~OnlyY;
	__raw_writel(reg, info->dp_tv_regs + DP_TV_PATH_TVOUT);

}

/**
 * @brief  Set CVBS chroma Filter on
 * @fn   void DP_TV_C_FILTER_OnOff(eDPOnOff OnOff)
 * @param mode   [in] C Filter On/Off
 *
 * @return None.
 *
 */
void dp_tv_c_filter_onoff(struct stfb_info *info, enum edp_onoff onoff)
{
	unsigned int reg = 0;
	reg = __raw_readl(info->dp_tv_regs + DP_TV_PATH_TVOUT);
	if (onoff)
		reg |= SelCFilter;
	else
		reg &= ~SelCFilter;
	__raw_writel(reg, info->dp_tv_regs + DP_TV_PATH_TVOUT);

}

/**
 * @brief  Set CVBS Luminous Filter on
 * @fn   void DP_TV_Y_FILTER_OnOff(eDPOnOff OnOff)
 * @param mode   [in] Y Filter On/Off
 *
 * @return None.
 *
 */
void dp_tv_y_filter_onoff(struct stfb_info *info, enum edp_onoff onoff)
{
	unsigned int reg = 0;
	reg = __raw_readl(info->dp_tv_regs + DP_TV_PATH_TVOUT);
	if (onoff)
		reg |= SelYFilter;
	else
		reg &= ~SelYFilter;
	__raw_writel(reg, info->dp_tv_regs + DP_TV_PATH_TVOUT);

}

/**
 * @brief  Set CVBS ColorSubCarrier Genlock: 0=Free run mode, 1=Lock Mode
 * @fn   void DP_TV_LOCK_MODE_OnOff(eDPOnOff OnOff)
 * @param mode   [in] Specify Lock or free run
 *
 * @return None.
 *
 */
void dp_tv_lock_mode_onoff(struct stfb_info *info, enum edp_onoff onoff)
{
	unsigned int reg = 0;
	reg = __raw_readl(info->dp_tv_regs + DP_TV_PATH_TVOUT);
	if (onoff)
		reg |= LockMode;
	else
		reg &= ~LockMode;
	__raw_writel(reg, info->dp_tv_regs + DP_TV_PATH_TVOUT);

}

/**
 * @brief  Set CCVBS Patern Generate Synchronize with input sync
 * @fn   void DP_TV_SD_Pattern_Sync_Match(eDPOnOff OnOff)
 * @param mode   [in] Sync match On/Off
 *
 * @return None.
 *
 */
void dp_tv_sd_pattern_sync_match(struct stfb_info *info, enum edp_onoff onoff)
{
	unsigned int reg = 0;
	reg = __raw_readl(info->dp_tv_regs + DP_TV_PATH_TVOUT);
	if (onoff)
		reg |= SDPaternGENSyncMatch;
	else
		reg &= ~SDPaternGENSyncMatch;
	__raw_writel(reg, info->dp_tv_regs + DP_TV_PATH_TVOUT);

}

/**
 * @brief  Set CVBS Pattern Generator on
 * @fn   void DP_TV_SD_Pattern_OnOff(eDPOnOff OnOff)
 * @param mode   [in] SD pattern On/Off
 *
 * @return None.
 *
 */
void dp_tv_sd_pattern_onoff(struct stfb_info *info, enum edp_onoff onoff)
{
	unsigned int reg = 0;
	reg = __raw_readl(info->dp_tv_regs + DP_TV_PATH_TVOUT);
	if (onoff)
		reg |= SDPaternGENON;
	else
		reg &= ~SDPaternGENON;
	__raw_writel(reg, info->dp_tv_regs + DP_TV_PATH_TVOUT);

}

/**
 * @brief  Set CVBS Video Format:0= PAL-B,D,G,H,I, 1=NTSC-M, 2= PAL-N, 3=PAL60
 * @fn   void DP_TV_CVBS_Format(eTvCVBSFormat mode)
 * @param mode   [in] Specify Composite Video mode
 *
 * @return None.
 *
 */
void dp_tv_cvbs_format(struct stfb_info *info, enum tv_cvbs_format mode)
{
	unsigned int reg = 0;
	reg = __raw_readl(info->dp_tv_regs + DP_TV_PATH_TVOUT);
	reg |= SelNTPAL(mode);
	__raw_writel(reg, info->dp_tv_regs + DP_TV_PATH_TVOUT);

}

/**
 * @brief  Specify  TV output Sync inverse:0=Vsync, 1=Hsync, 2=Fsync(field sync, Only use composite)
 * @fn     void DP_SYNC_INV_OnOff(Uint8 Sync, eDPOnOff OnOff)
 * @param mode    [in] Select Sync type
 * @param mode    [in] Sync Inverse On/Off
 *
 * @return None.
 *
 */
void dp_tv_sync_inv_onoff(struct stfb_info *info, unsigned int sync,
		enum edp_onoff onoff)
{
	unsigned int reg = 0;
	reg = __raw_readl(info->dp_tv_regs + DP_TV_PATH_TVOUT);
	if (0 == sync) {
		if (onoff)
			reg |= VSYNC_INV;
		else
			reg &= ~VSYNC_INV;
	} else if (1 == sync) {
		if (onoff)
			reg |= HSYNC_INV;
		else
			reg &= ~HSYNC_INV;
	} else if (2 == sync) {
		if (onoff)
			reg |= FSYNC_INV;
		else
			reg &= ~FSYNC_INV;
	}
	__raw_writel(reg, info->dp_tv_regs + DP_TV_PATH_TVOUT);

}

void dp_tv_bbox_2x(struct stfb_info *info, enum edp_onoff onoff)
{
	unsigned int reg = 0;
	reg = __raw_readl(info->dp_tv_regs + DP_TV_PATH_CONTROL);
	if (onoff)
		reg |= BBOX_2X;
	else
		reg &= ~BBOX_2X;
	__raw_writel(reg, info->dp_tv_regs + DP_TV_PATH_CONTROL);

}

/**
 * @brief video on setting
 * @fn void d4_dp_tv_video_on(struct stvideo_on video)
 * @param video_on[in] struct stvideo_on, for details please refer linux/include/video/drime4/d4_dp_type.h
 * @return
 *
 * @note	Features: Address adjust, video Boundary On/Off, Image Display Position, Video Format (4:2:2, 4:2:0), Video Window On/Off
 */
void d4_dp_tv_video_on(struct stvideo_on video)
{

	enum edp_window window;
	struct stdp_image_address addr;
	struct stdp_display_area area;
	enum edp_tv_mode mode;

	window = video.window;
	addr = video.image_addr;
	area = video.display_area;

	dp_tv_auto_update_onoff(&tv_info, DP_OFF);
	d4_dp_tv_video_stride(video.stride);
	dp_tv_vid_set_format(&tv_info, video.format);
	//dp_tv_3d_onoff(&tv_info, video.mode.type);

	/* 3D R window */
	if (video.mode.window == r_window) {

		dp_tv_mode_get(&mode);
		if (video.mode.type == fp_progressive
				|| video.mode.type == fp_interlace) {
			if (mode == RES_720P_60_3D_FP || mode == RES_720P_50_3D_FP) {
				area.V_Start += 750;
			} else if (mode == RES_1080P_24_3D_FP
					|| mode == RES_1080P_30_3D_FP) {
				area.V_Start += (1080 + 45);
			}
		} else if (video.mode.type == ssh) {
			if (mode == RES_720P_60_3D_SBS || mode == RES_720P_50_3D_SBS) {
				area.H_Start += 1280 / 2;
			} else if (mode == RES_1080I_60_3D_SBS
					|| mode == RES_1080I_50_3D_SBS
					|| mode == RES_1080P_24_3D_SBS
					|| mode == RES_1080P_30_3D_SBS) {
				area.H_Start += 1920 / 2;
			}
		} else if (video.mode.type == tb) {
			if (mode == RES_720P_60_3D_TB || mode == RES_720P_50_3D_TB) {
				area.V_Start += 720 / 2;
			} else if (mode == RES_1080P_60_3D_TB || mode == RES_1080P_50_3D_TB
					|| mode == RES_1080P_30_3D_TB
					|| mode == RES_1080P_24_3D_TB) {
				area.V_Start += 1080 / 2;

			}
		}
		dp_tv_set_vid_display_area(&tv_info, video.window, video.bit, &area);
	} else
		dp_tv_set_vid_display_area(&tv_info, video.window, video.bit, &area);
	
	dp_tv_set_vid_image_address(&tv_info, window, &addr);
	dp_tv_auto_update_onoff(&tv_info, DP_ON);		
}

/**
 * @brief 3D video setting
 * @fn void d4_dp_tv_3d_video_on(struct stvideo_on_3d video_3d)
 * @param video_on_3d[in] struct stvideo_on_3d, for details please refer linux/include/video/drime4/d4_dp_type.h
 * @return
 *
 * @note	Features: Address adjust, video Boundary On/Off, Image Display Position, Video Format (4:2:2, 4:2:0), Video Window On/Off
 */
void d4_dp_tv_3d_video_on(struct stvideo_on_3d video_3d)
{
	struct stdp_display_area left_window_area;
	struct stdp_display_area right_window_area;
	enum edp_tv_mode mode;

	d4_dp_tv_video_stride(video_3d.stride);
	dp_tv_vid_set_format(&tv_info, video_3d.format);
	//dp_tv_3d_onoff(&tv_info, video_3d.mode.type);

	left_window_area = video_3d.left_window_display_area;
	right_window_area = video_3d.right_window_display_area;

	/* Set 3D Right window */
	dp_tv_mode_get(&mode);
	if (video_3d.mode.type == fp_progressive
			|| video_3d.mode.type == fp_interlace) {
		if (mode == RES_720P_60_3D_FP || mode == RES_720P_50_3D_FP) {
			right_window_area.V_Start += 750;
		} else if (mode == RES_1080P_24_3D_FP || mode == RES_1080P_30_3D_FP) {
			right_window_area.V_Start += (1080 + 45);
		}
	} else if (video_3d.mode.type == ssh) {
		if (mode == RES_720P_60_3D_SBS || mode == RES_720P_50_3D_SBS) {
			right_window_area.H_Start += 1280 / 2;
		} else if (mode == RES_1080I_60_3D_SBS || mode == RES_1080I_50_3D_SBS
				|| mode == RES_1080P_24_3D_SBS || mode == RES_1080P_30_3D_SBS) {
			right_window_area.H_Start += 1920 / 2;
		}
	} else if (video_3d.mode.type == tb) {
		if (mode == RES_720P_60_3D_TB || mode == RES_720P_50_3D_TB) {
			right_window_area.V_Start += 720 / 2;
		} else if (mode == RES_1080P_60_3D_TB || mode == RES_1080P_50_3D_TB
				|| mode == RES_1080P_30_3D_TB || mode == RES_1080P_24_3D_TB) {
			right_window_area.V_Start += 1080 / 2;
		}
	}

	/* set 3D Left window area */
	dp_tv_set_vid_display_area(&tv_info, 0, video_3d.bit, &left_window_area);
	/* set 3D Right window area */
	dp_tv_set_vid_display_area(&tv_info, 1, video_3d.bit, &right_window_area);
}



/**
 * @brief
 *
 * @param mode
 */
void dp_tv_mode_get(enum edp_tv_mode *mode)
{
	*mode = g_mode;
}

/**
 * @brief
 *
 * @param window, addr
 */
void dp_tv_set_vid_image_address(struct stfb_info *info, enum edp_window window,
		struct stdp_image_address *addr)
{
	unsigned int index;
	index = (window) * (0x20);

	__raw_writel(addr->y0_address,
			info->dp_tv_regs + DP_TV_VID_PATH_VIDCH0_ADDR_F0Y + index);

	__raw_writel(addr->c0_address,
			info->dp_tv_regs + DP_TV_VID_PATH_VIDCH0_ADDR_F0C + index);

	__raw_writel(addr->y1_address,
			info->dp_tv_regs + DP_TV_VID_PATH_VIDCH0_ADDR_F1Y + index);

	__raw_writel(addr->c1_address,
			info->dp_tv_regs + DP_TV_VID_PATH_VIDCH0_ADDR_F1C + index);

}

void d4_dp_tv_set_vid_image_address(enum edp_window window,
		struct stdp_image_address *addr)
{
	dp_tv_set_vid_image_address(&tv_info, window, addr);
}

void d4_dp_tv_set_3d_vid_image_address(struct stdp_image_address *laddr, struct stdp_image_address *raddr)
{
	dp_tv_auto_update_onoff(&tv_info, DP_OFF);
	dp_tv_set_vid_image_address(&tv_info, DP_WIN0, laddr);
	dp_tv_set_vid_image_address(&tv_info, DP_WIN1, raddr);
	dp_tv_auto_update_onoff(&tv_info, DP_ON);
}


/**
 * @brief video display area setting
 * @fn void d4_dp_tv_set_vid_display_area(enum edp_window win,
 enum edp_video_bit vid_bit, struct stdp_display_area *display)
 * @param win[in] display window set
 * @param vid_bit[in] video bit 8/10 bit
 * @param *display[in] video display area set, for details please refer linux/include/video/drime4/d4_dp_type.h
 * @return
 *
 * @note
 */
void d4_dp_tv_set_vid_display_area(enum edp_window win,
		enum edp_video_bit vid_bit, struct stdp_display_area *display)
{
	dp_tv_set_vid_display_area(&tv_info, win, vid_bit, display);
}

/**
 * @brief
 *
 * @param info,win,vid_bit,display
 */
void dp_tv_set_vid_display_area(struct stfb_info *info, enum edp_window win,
		enum edp_video_bit vid_bit, struct stdp_display_area *display)
{
	unsigned int index, reg = 0, temp1 = 0, temp2 = 0;
	unsigned int h_start, h_end, v_start, v_end;
	enum edp_tv_mode mode;
	
	dp_tv_mode_get(&mode);
	
	if (display->H_Size >= 1920)
		display->H_Size = 1920;
	
	h_start = display->H_Start;
	h_end = h_start + display->H_Size;

	if (display->V_Size >= 1080)
		display->V_Size = 1080;

	if (mode == RES_1080I_50_3D_SBS ||mode == RES_1080I_60_3D_SBS) {
		display->V_Start /= 2;
		display->V_Size /= 2;
	}

	v_start = display->V_Start;
	// handle video noise except 3D mode
	// if(mode < RES_720P_60_3D_FP)
	{
		if((display->V_Start % 4) < 3)
			v_start = v_start - (v_start % 4);
		else
			v_start = v_start + (4-(v_start % 4));
	}
	
	v_end = v_start + display->V_Size;

	index = (win) * (0x20);
	temp1 = temp2 = 0;
	D4_DP_TV_POS_H_START(temp1, h_start);
	D4_DP_TV_POS_H_END(temp2, h_end);
	reg = temp1 | temp2;
	__raw_writel(reg, info->dp_tv_regs + DP_TV_VID_PATH_VIDCH0_HPOS + index);

	reg = 0;
	temp1 = temp2 = 0;
	D4_DP_TV_POS_V_START(temp1, v_start);
	D4_DP_TV_POS_V_END(temp2, v_end);
	reg = temp1 | temp2;
	__raw_writel(reg, info->dp_tv_regs + DP_TV_VID_PATH_VIDCH0_VPOS + index);

}

/**
 * @brief video display window on/off
 * @fn void d4_dp_tv_video_window_onoff(enum edp_window window, enum edp_onoff on_off)
 * @param window[in] display window set
 * @param on_off[in] on/off
 * @return
 *
 * @note
 */
void d4_dp_tv_video_window_onoff(enum edp_window window, enum edp_onoff on_off)
{
	dp_tv_vid_window_onoff(&tv_info, window, on_off);
}

/**
 * @brief
 *
 * @param window, DP_ON
 *
 */
void dp_tv_vid_window_onoff(struct stfb_info *info, enum edp_window window,
		enum edp_onoff on_off)
{
	unsigned int reg = 0;
	reg = __raw_readl(info->dp_tv_regs + DP_TV_PATH_VIDDMA_M0);

	switch (window) {
		case DP_WIN0:
			if (on_off)
				reg |= Win0_On;
			else
				reg &= ~Win0_On;
			break;

		case DP_WIN1:
			if (on_off)
				reg |= Win1_On;
			else
				reg &= ~Win1_On;
			break;

		case DP_WIN2:
			if (on_off)
				reg |= Win2_On;
			else
				reg &= ~Win2_On;
			break;

		case DP_WIN3:
			if (on_off)
				reg |= Win3_On;
			else
				reg &= ~Win3_On;
			break;
			
		default:
			break;
	}
	__raw_writel(reg, info->dp_tv_regs + DP_TV_PATH_VIDDMA_M0);

}

void d4_dp_tv_vid_set_format(enum edp_videoformat format)
{
	dp_tv_vid_set_format(&tv_info, format);
}

void dp_tv_vid_set_format(struct stfb_info *info, enum edp_videoformat format)
{
	unsigned int reg = 0;
	reg = __raw_readl(info->dp_tv_regs + DP_TV_PATH_CONTROL);

	if (Ycbcr_422 == format)
		reg |= YCBCR422;
	else if (Ycbcr_420 == format)
		reg &= ~YCBCR422;
	__raw_writel(reg, info->dp_tv_regs + DP_TV_PATH_CONTROL);
}

/**
 * @brief  3D Display on/off
 *
 * @param 3D Type
 * @return None
 *
 */
void dp_tv_3d_onoff(struct stfb_info *info, enum scan_type type)
{
	if (none == type) {
		dp_tv_3d_set(info, DP_OFF);
	} else {
		dp_tv_3d_set(info, DP_ON);
		dp_tv_3d_mode_set(info, type);
	}
}

/**
 * @brief Set 3D Video
 * @fn  void DP_TV_3D_Set(eDPOnOff OnOff)
 * @param OnOff [in] 3D On/Off
 *
 * @return None.
 *
 */
void dp_tv_3d_set(struct stfb_info *info, enum edp_onoff onoff)
{
	unsigned int reg = 0;
	reg = __raw_readl(info->dp_tv_regs + DP_TV_PATH_CONTROL);

	if (onoff)
		reg |= VID_3D;
	else
		reg &= ~VID_3D;
	__raw_writel(reg, info->dp_tv_regs + DP_TV_PATH_CONTROL);
}

/**
 * @brief Set 3D Type
 * @fn  void DP_TV_3D_Mode_Set(e3D_TYPE type)
 * @param type [in] 3D Type
 *
 * @return None.
 *
 */
void dp_tv_3d_mode_set(struct stfb_info *info, enum scan_type type)
{
	unsigned int reg = 0;
	reg = __raw_readl(info->dp_tv_regs + DP_TV_PATH_CONTROL);

	reg |= FORMAT_3D(type);
	__raw_writel(reg, info->dp_tv_regs + DP_TV_PATH_CONTROL);
}

int dptv_interrupt_check(void)
{
	if (dp_get_interrupt_status(&tv_info, INT_TV)) {
		clear_dp_interrupt(&tv_info, INT_TV);
		return 0;
	} else
		return -1;

}

/**
 * @brief dp tv interrupt on/off
 * @fn void d4_dp_tv_interrupt_onoff(enum edp_onoff onoff)
 * @param onoff[in] interrupt on/off
 * @return
 *
 * @note
 */
void d4_dp_tv_interrupt_onoff(enum edp_onoff onoff, enum einterrupt_type sel, int arg)
{
	DpPRINTK("%s(),%d,onoff =%d\n", __FUNCTION__, __LINE__, onoff);

	if (onoff == DP_ON) {
		g_dp_tv_interrupt_state |= (1<<sel);
		g_dp_tv_interrupt_arg[sel] = arg;
		dp_interrupt_on_off(&tv_info, INT_TV, onoff);
	} else {
		g_dp_tv_interrupt_state &= ~(1<<sel);
		g_dp_tv_interrupt_arg[sel] = 0;

		if (g_dp_tv_interrupt_state == 0) {
			dp_interrupt_on_off(&tv_info, INT_TV, onoff);
		}
	}
	
}

int d4_dp_tv_interrupt_state(void)
{
	return g_dp_tv_interrupt_state;
}

int d4_dp_tv_interrupt_arg(enum einterrupt_type sel)
{
	return g_dp_tv_interrupt_arg[sel];
}

static int dp_tv_i2c_write(unsigned char addr, unsigned char value)
{
	int ret = 0;
	if(NULL == i2client) {
		printk("\n [Warning] i2client is null\n");
	}

	ret = i2c_smbus_write_byte_data(i2client, addr, value);
	if (ret < 0) {
		printk("[Error] TV I2C Error[error number: %d ]: Addr[%#02x], Value[%#02x]\n",
				ret, addr, value);
		return -1;
	}
	return 0;
}

void dp_tv_off(void)
{
	d4_dp_tv_video_window_onoff(0, DP_OFF);
	d4_dp_tv_video_window_onoff(1, DP_OFF);	
	d4_dp_tv_video_window_onoff(2, DP_OFF);	
	d4_dp_tv_video_window_onoff(3, DP_OFF);	
	d4_dp_tv_graphics_window_onoff(0, DP_OFF);
	d4_dp_tv_graphics_window_onoff(1, DP_OFF);
	d4_dp_tv_graphics_window_onoff(2, DP_OFF);
	d4_dp_tv_graphics_window_onoff(3, DP_OFF);
	dp_tv_start_onoff(&tv_info, DP_OFF);
}
