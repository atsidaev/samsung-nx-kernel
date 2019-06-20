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


#ifndef __VersionInformation_galaxynx_simulator_h__
#define __VersionInformation_galaxynx_simulator_h__


typedef enum 
{
	eBdVer_JIG2 = 0,
	eBdVer_DV1,
	eBdVer_DV2,
	eBdVer_DV3,	
	eBdVer_DV4, 
	eBdVer_DV5, 
	eBdVer_PV1,
	eBdVer_PV2,
	eBdVer_PVR,
	eBdVer_PR1,
	eBdVer_RSV,
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
extern const int gpio_ROTATION_INT[eBdVer_Max];
extern const int gpio_SHUTTER_KEY1_DSP[eBdVer_Max];
extern const int gpio_SHUTTER_KEY2[eBdVer_Max];
extern const int gpio_JACK_INT[eBdVer_Max];
extern const int gpio_JOG_L[eBdVer_Max];
extern const int gpio_JOG_R[eBdVer_Max];
extern const int gpio_DRS_INV_EN[eBdVer_Max];
extern const int gpio_LENS_RB[eBdVer_Max];
extern const int gpio_CIS_PW1_ON[eBdVer_Max];
extern const int gpio_CIS_PW2_ON[eBdVer_Max];
extern const int gpio_CIS_PW3_ON[eBdVer_Max];
extern const int gpio_OSC_CE[eBdVer_Max];
extern const int gpio_WHEEL_L[eBdVer_Max];
extern const int gpio_SH_PW_ON[eBdVer_Max];
extern const int gpio_LENS_SYNC_EN1[eBdVer_Max];
extern const int gpio_LENS_SYNC_EN2[eBdVer_Max];
extern const int gpio_STR_VCC_ON[eBdVer_Max];
extern const int gpio_AF_LED[eBdVer_Max];
extern const int gpio_CARD_LED[eBdVer_Max];
extern const int gpio_EFS_RESET[eBdVer_Max];
extern const int gpio_HDR_RESET[eBdVer_Max];
extern const int gpio_TSP_INT[eBdVer_Max];
extern const int gpio_TSP_3_3V_ON[eBdVer_Max];
extern const int gpio_SH_MOT_IN1[eBdVer_Max];
extern const int gpio_SH_MOT_IN2[eBdVer_Max];
extern const int gpio_EXT_STR_DET[eBdVer_Max];
extern const int gpio_WHEEL_R[eBdVer_Max];
extern const int gpio_REC_KEY[eBdVer_Max];
extern const int gpio_WIFI_PWR_CNT[eBdVer_Max];
extern const int gpio_LCD_DATA_EN[eBdVer_Max];
extern const int gpio_LCD_BL_ON[eBdVer_Max];
extern const int gpio_SH_KEY1_MUIC[eBdVer_Max];
extern const int gpio_SH_KEY2_MUIC[eBdVer_Max];

#endif		// __VersionInformation_galaxynx_simulator_h__

