///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Samsung Electronics Co., LTD., All Rights Reserved.
//!@file				arch/arm/mach-drime4/include/mach/version_information.h
//!@brief				System Version Information
//!@note				System Version Information<br>
//!					F/W, H/W (Board), ... etc. Version
//!@author			Peter Jun
//!@date				Modification<br>
//					2011/01/29	First Draft.<br>
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#ifndef __VersionInformation_galaxynx_h__
#define __VersionInformation_galaxynx_h__


typedef enum 
{
	eBdVer_ES1 = 0,
	eBdVer_ES2,
	eBdVer_PDI2,
	eBdVer_PDI1,
	eBdVer_DV1,
	eBdVer_DV2,
	eBdVer_PV1,
	eBdVer_PV2,
	eBdVer_PR1,
	eBdVer_Max
} EBdVersion;


//--------------------------------------------------------------------------
//!@brief				Version Information Initialize
//!@note				Version Information Initialize
//--------------------------------------------------------------------------
extern void VersionInfo_Initialize(void);

//--------------------------------------------------------------------------
//!@brief				Get Project Name
//!@note				Get Project Name
//--------------------------------------------------------------------------
extern const char * GetProjectName(void);

//--------------------------------------------------------------------------
//!@brief				Get F/W Version (Full)
//!@note				Get F/W Version (Full)
//--------------------------------------------------------------------------
extern const char * GetFwVersionFull(void);

//--------------------------------------------------------------------------
//!@brief				Get F/W Version (User)
//!@note				Get F/W Version (User)
//--------------------------------------------------------------------------
extern const char * GetFwVersionUser(void);

//--------------------------------------------------------------------------
//!@brief				Get Board Version
//!@note				Get Board Version
//--------------------------------------------------------------------------
extern EBdVersion GetBoardVersion(void);

//--------------------------------------------------------------------------
//!@brief				Get Board Version Name
//!@note				Get Board Version Name
//--------------------------------------------------------------------------
extern const char * GetBoardVersionName(void);


/*
 * gpio definition arry
 */
extern EBdVersion g_eBoardVersion;

extern const int gpio_ISP_SCL[eBdVer_Max];
extern const int gpio_ISP_SDA[eBdVer_Max];
extern const int gpio_SHUTTER_KEY1_DSP[eBdVer_Max];
extern const int gpio_SHUTTER_KEY2[eBdVer_Max];
extern const int gpio_ISP_INT[eBdVer_Max];
extern const int gpio_EXT_STR_CTL_IN[eBdVer_Max];
extern const int gpio_D4C_INT[eBdVer_Max];
extern const int gpio_D4C_RESET[eBdVer_Max];
extern const int gpio_D4C_AN_ON[eBdVer_Max];
extern const int gpio_D4C_CORE_ON[eBdVer_Max];
extern const int gpio_SH_MOT_IN1[eBdVer_Max];
extern const int gpio_SH_MOT_IN2[eBdVer_Max];
extern const int gpio_SH_PI1[eBdVer_Max];
extern const int gpio_SH_PI2[eBdVer_Max];
extern const int gpio_ISP_POWER_ON[eBdVer_Max];
extern const int gpio_LENS_RB[eBdVer_Max];
extern const int gpio_D4C_IO_ON[eBdVer_Max];

extern const int gpio_EXT_STR_DET[eBdVer_Max];
extern const int gpio_HDR_RESET[eBdVer_Max];
extern const int gpio_EFS_RESET[eBdVer_Max];
extern const int gpio_AF_LED[eBdVer_Max];
extern const int gpio_CARD_LED[eBdVer_Max];
extern const int gpio_DRS_PWM_PLUS_CON[eBdVer_Max];
extern const int gpio_DRS_PWM_MINUS_CON[eBdVer_Max];
/*
extern const int gpio_WIFI_SDIO_D0[eBdVer_Max];
extern const int gpio_WIFI_SDIO_D1[eBdVer_Max];
extern const int gpio_WIFI_SDIO_D2[eBdVer_Max];
extern const int gpio_WIFI_SDIO_D3[eBdVer_Max];
extern const int gpio_WIFI_SDIO_CMD[eBdVer_Max];
extern const int gpio_WIFI_SDIO_CLK[eBdVer_Max];
extern const int gpio_WIFI_SDIO_RESET[eBdVer_Max];
*/
extern const int gpio_READY_BUSY[eBdVer_Max];

extern const int gpio_LENS_3_3V_ON[eBdVer_Max];
extern const int gpio_LENS_5_0V_ON[eBdVer_Max];
extern const int gpio_GPIO_EYE_INT[eBdVer_Max];
extern const int gpio_GPIO_REC_KEY[eBdVer_Max];
extern const int gpio_USB_DET_KEY[eBdVer_Max];
extern const int gpio_DRS_INV_EN[eBdVer_Max];
extern const int gpio_STR_PU_CNT[eBdVer_Max];
extern const int gpio_CIS_RESET[eBdVer_Max];
extern const int gpio_STR_CHANGE[eBdVer_Max];
extern const int gpio_STR_READY[eBdVer_Max];
extern const int gpio_STR_CTL_IN[eBdVer_Max];
extern const int gpio_SH_MG1_ON_OUT[eBdVer_Max];
extern const int gpio_SH_MG2_ON_OUT[eBdVer_Max];
#endif		// __VersionInformation_galaxynx_h__

