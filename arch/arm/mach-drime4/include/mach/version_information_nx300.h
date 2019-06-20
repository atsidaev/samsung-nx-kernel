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


#ifndef __VersionInformation_nx300_h__
#define __VersionInformation_nx300_h__

#define CONFIG_NX300_PR3_BOARD

typedef enum 
{
	eBdVer_PV1 = 0, 
	eBdVer_PV2,
	eBdVer_PV3,
	eBdVer_PV4,
	eBdVer_PV5,
	eBdVer_PR1,
	eBdVer_PR1B,
	eBdVer_PR2,
	eBdVer_PR3,
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
extern const int gpio_SH_KEY2_MUIC[eBdVer_Max];

#endif		// __VersionInformation_nx300_h__

