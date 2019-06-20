#ifndef _DRIME4_HDMI_H_
#define _DRIME4_HDMI_H_


#define HDMI_AUI_SF_MASK			(1<<4|1<<3|1<<2)

#define HDMI_AUI_SF_SF_32KHZ			(1<<2)
#define HDMI_AUI_SF_SF_44KHZ			(1<<3)
#define HDMI_AUI_SF_SF_88KHZ			(1<<4)
#define HDMI_AUI_SF_SF_176KHZ			(1<<4|1<<3)
#define HDMI_AUI_SF_SF_48KHZ			(1<<3|1<<2)
#define HDMI_AUI_SF_SF_96KHZ			(1<<4|1<<2)
#define HDMI_AUI_SF_SF_192KHZ			(1<<4|1<<3|1<<2)

#define I2S_CH_ST_3_SF_MASK			(0x0F)
#define I2S_CH_ST_3_SF_44KHZ			(0)
#define I2S_CH_ST_3_SF_88KHZ			(1<<3)
#define I2S_CH_ST_3_SF_176KHZ			(1<<3|1<<2)
#define I2S_CH_ST_3_SF_48KHZ			(1<<1)
#define I2S_CH_ST_3_SF_96KHZ			(1<<3|1<<1)
#define I2S_CH_ST_3_SF_192KHZ			(1<<3|1<<2|1<<1)
#define I2S_CH_ST_3_SF_768KHZ			(1<<3|1<<0)
#define I2S_CH_ST_3_SF_32KHZ			(1<<1|1<<0)

enum I2SClockPerFrame {
	I2S_32FS,
	I2S_48FS,
	I2S_64FS
};

#define I2S_CON_DATA_NUM_MASK			(3<<2)
#define I2S_CON_DATA_NUM_16			(1<<2)
#define I2S_CON_DATA_NUM_20			(2<<2)
#define I2S_CON_DATA_NUM_24			(3<<2)

#define I2S_CON_I2S_MODE_BASIC			(0)

#define I2S_IN_MUX_IN_ENABLE			(1<<4)
#define I2S_IN_MUX_SELECT_DSD			(2<<2)
#define I2S_IN_MUX_SELECT_I2S			(1<<2)
#define I2S_IN_MUX_SELECT_SPDIF			(0)
#define I2S_IN_MUX_CUV_ENABLE			(1<<1)
#define I2S_IN_MUX_ENABLE			(1)

enum SamplingFreq {
	SF_32KHZ = 0,
	SF_44KHZ,
	SF_48KHZ,
	SF_88KHZ,
	SF_96KHZ,
	SF_176KHZ,
	SF_192KHZ,
	SF_768KHZ
};

#define I2S_CLK_CON_ENABLE			(1)
#define I2S_CLK_CON_DISABLE			(0)


#endif
