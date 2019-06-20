///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Samsung Electronics Co., LTD., All Rights Reserved.
//!@file				arch/arm/mach-drime4/version_information.c
//!@brief				System Version Information
//!@note				System Version Information<br>
//!					F/W, H/W, ... etc.
//!@author			Peter Jun
//!@date				Modification<br>
//					2011/01/29	First Draft.<br>
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "include/mach/version_information.h"
#include "include/mach/version.h"
#include "include/mach/gpio.h"

static const char* m_pcnchProject		= CSC_PROJECT;
static const char* m_pcnchFwVerFull 	= CSC_FW_VERSION_FULL;
static const char* m_pcnchFwVerUser 	= CSC_FW_VERSION_USER;

static const char* g_pcnchBdVerJIG2	= "JIG2";
static const char* g_pcnchBdVerDV1		= "DV1";
static const char* g_pcnchBdVerDV2		= "DV2";
static const char* g_pcnchBdVerDV3		= "DV3";
static const char* g_pcnchBdVerDV4		= "DV4";
static const char* g_pcnchBdVerDV5		= "DV5";
static const char* g_pcnchBdVerPV1		= "PV1";
static const char* g_pcnchBdVerPV2		= "PV2";
static const char* g_pcnchBdVerPVR		= "PVR";
static const char* g_pcnchBdVerPR1		= "PR1";
static const char* g_pcnchBdVerRSV		= "N/A";

EBdVersion g_eBoardVersion = eBdVer_JIG2;
static int g_iInitialized = 0;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// ------------------------------------------------	     eBdVer_JIG2				eBdVer_DV1			eBdVer_DV2			eBdVer_DV3			eBdVer_DV4			eBdVer_DV5

const int gpio_ROTATION_INT[eBdVer_Max] 		= { DRIME4_GPIO6(0),	DRIME4_GPIO7(1), 	DRIME4_GPIO7(1), 	DRIME4_GPIO7(1),	DRIME4_GPIO7(1), 	DRIME4_GPIO7(1), 	0,};
const int gpio_SHUTTER_KEY1_DSP[eBdVer_Max] 	= { DRIME4_GPIO23(3),	DRIME4_GPIO6(0), 	DRIME4_GPIO6(0),	DRIME4_GPIO6(0), 	DRIME4_GPIO6(0), 	DRIME4_GPIO6(0), 	0, };
const int gpio_SHUTTER_KEY2[eBdVer_Max]		= { DRIME4_GPIO23(4),	DRIME4_GPIO6(1), 	DRIME4_GPIO6(1),	DRIME4_GPIO6(1),	DRIME4_GPIO6(1),	DRIME4_GPIO6(1),	0, };
const int gpio_JACK_INT[eBdVer_Max] 			= { DRIME4_GPIO6(3),	DRIME4_GPIO6(7), 	DRIME4_GPIO6(7), 	DRIME4_GPIO6(7),	DRIME4_GPIO6(7), 	DRIME4_GPIO6(7), 	0, };
const int gpio_JOG_L[eBdVer_Max] 				= { DRIME4_GPIO6(4),	DRIME4_GPIO7(2), 	DRIME4_GPIO7(2),	DRIME4_GPIO7(2),	DRIME4_GPIO7(2),	DRIME4_GPIO7(2),	0, };
const int gpio_JOG_R[eBdVer_Max] 				= { DRIME4_GPIO6(5),	DRIME4_GPIO7(3), 	DRIME4_GPIO7(3),	DRIME4_GPIO7(3),	DRIME4_GPIO7(3),	DRIME4_GPIO7(3),	0, };
const int gpio_DRS_INV_EN[eBdVer_Max] 		= { DRIME4_GPIO6(7),	DRIME4_GPIO24(3), 	DRIME4_GPIO24(3),	DRIME4_GPIO24(3),	DRIME4_GPIO24(3),	DRIME4_GPIO24(3),	0, };
const int gpio_LENS_RB[eBdVer_Max] 			= { DRIME4_GPIO24(1),	DRIME4_GPIO6(6), 	DRIME4_GPIO6(6),	DRIME4_GPIO6(6),	DRIME4_GPIO6(6),	DRIME4_GPIO6(6),	0, };
const int gpio_CIS_PW1_ON[eBdVer_Max] 		= { DRIME4_GPIO7(0),	DRIME4_GPIO23(4), 	DRIME4_GPIO23(4),	DRIME4_GPIO23(4),	DRIME4_GPIO23(4),	DRIME4_GPIO23(4),	0, };
const int gpio_CIS_PW2_ON[eBdVer_Max] 		= { DRIME4_GPIO7(1),	DRIME4_GPIO23(5), 	DRIME4_GPIO23(5),	DRIME4_GPIO23(5),	DRIME4_GPIO23(5),	DRIME4_GPIO23(5),	0, };
const int gpio_CIS_PW3_ON[eBdVer_Max] 		= { DRIME4_GPIO7(2),	DRIME4_GPIO23(6), 	DRIME4_GPIO23(6), 	DRIME4_GPIO23(6), 	DRIME4_GPIO23(6),	DRIME4_GPIO23(6),	0, };
const int gpio_OSC_CE[eBdVer_Max] 				= { DRIME4_GPIO7(3),	DRIME4_GPIO24(5), 	DRIME4_GPIO24(5), 	DRIME4_GPIO24(5), 	DRIME4_GPIO24(5),	DRIME4_GPIO24(5),	0, };
const int gpio_WHEEL_L[eBdVer_Max] 			= { DRIME4_GPIO7(4),	0, };
const int gpio_SH_PW_ON[eBdVer_Max] 			= { DRIME4_GPIO7(5),	DRIME4_GPIO24(7), 	DRIME4_GPIO24(7),	DRIME4_GPIO24(7),	DRIME4_GPIO24(7),	DRIME4_GPIO24(7),	0, };
const int gpio_LENS_SYNC_EN1[eBdVer_Max] 	= { DRIME4_GPIO8(2),	DRIME4_GPIO23(7), 	DRIME4_GPIO23(7),	DRIME4_GPIO23(7),	DRIME4_GPIO23(7),	DRIME4_GPIO23(7),	0, };
const int gpio_LENS_SYNC_EN2[eBdVer_Max] 	= { DRIME4_GPIO8(3),	DRIME4_GPIO24(0), 	DRIME4_GPIO24(0),	DRIME4_GPIO24(0),	DRIME4_GPIO24(0),	DRIME4_GPIO24(0),	0, };
const int gpio_STR_VCC_ON[eBdVer_Max] 		= { DRIME4_GPIO8(4),	DRIME4_GPIO24(6), 	DRIME4_GPIO24(6), 	DRIME4_GPIO24(6), 	DRIME4_GPIO24(6),	DRIME4_GPIO24(6),	0, };
const int gpio_AF_LED[eBdVer_Max] 				= { DRIME4_GPIO8(6),	DRIME4_GPIO24(1), 	DRIME4_GPIO24(1), 	DRIME4_GPIO24(1), 	DRIME4_GPIO24(1),	DRIME4_GPIO24(1),	0, };
const int gpio_CARD_LED[eBdVer_Max] 			= { DRIME4_GPIO8(7),	DRIME4_GPIO24(2), 	DRIME4_GPIO24(2),	DRIME4_GPIO24(2),	DRIME4_GPIO24(2),	DRIME4_GPIO24(2),	0, };
const int gpio_EFS_RESET[eBdVer_Max] 			= { DRIME4_GPIO9(2),	DRIME4_GPIO25(5), 	DRIME4_GPIO9(0), 	DRIME4_GPIO9(0), 	DRIME4_GPIO9(0), 	-1,					0, };
const int gpio_HDR_RESET[eBdVer_Max] 			= { DRIME4_GPIO9(3),	DRIME4_GPIO25(6), 	DRIME4_GPIO25(6),	DRIME4_GPIO25(6),	DRIME4_GPIO25(6),	DRIME4_GPIO25(6),	0, };
const int gpio_TSP_INT[eBdVer_Max]			= { DRIME4_GPIO12(2),	DRIME4_GPIO6(5),	DRIME4_GPIO6(5),	DRIME4_GPIO6(5),	DRIME4_GPIO6(5),	DRIME4_GPIO6(5),	0, };
const int gpio_TSP_3_3V_ON[eBdVer_Max]		= { DRIME4_GPIO12(3),	DRIME4_GPIO23(3),	DRIME4_GPIO23(3),	DRIME4_GPIO23(3),	DRIME4_GPIO23(3),	DRIME4_GPIO23(3),	0, };

const int gpio_SH_MOT_IN1[eBdVer_Max] 		= { DRIME4_GPIO13(2),	DRIME4_GPIO8(4), 	DRIME4_GPIO8(4), 	DRIME4_GPIO8(4), 	DRIME4_GPIO8(4), 	DRIME4_GPIO8(4),	0, };
const int gpio_SH_MOT_IN2[eBdVer_Max] 		= { DRIME4_GPIO13(3),	DRIME4_GPIO8(5), 	DRIME4_GPIO8(5),	DRIME4_GPIO8(5),	DRIME4_GPIO8(5),	DRIME4_GPIO8(5),	0, };
const int gpio_EXT_STR_DET[eBdVer_Max] 		= { DRIME4_GPIO24(3),	DRIME4_GPIO7(0), 	DRIME4_GPIO7(0),	DRIME4_GPIO7(0),	DRIME4_GPIO7(0),	DRIME4_GPIO7(0),	0, };
const int gpio_WHEEL_R[eBdVer_Max] 			= { DRIME4_GPIO24(2),	0, };
const int gpio_D4C_AN_ON[eBdVer_Max] 			= { -1,					DRIME4_GPIO9(0), 	DRIME4_GPIO8(3), 	DRIME4_GPIO8(3), 	DRIME4_GPIO8(3), 	DRIME4_GPIO8(3), 	0, };
const int gpio_CIS_SCL[eBdVer_Max] 			= { DRIME4_GPIO15(5),	DRIME4_GPIO15(5), 	DRIME4_GPIO15(5),	DRIME4_GPIO15(5),	DRIME4_GPIO15(5),	DRIME4_GPIO15(5),	0, };
const int gpio_CIS_SDA[eBdVer_Max] 			= { DRIME4_GPIO15(6),	DRIME4_GPIO15(6), 	DRIME4_GPIO15(6), 	DRIME4_GPIO15(6), 	DRIME4_GPIO15(6),	DRIME4_GPIO15(6),	0, };
const int gpio_REC_KEY[eBdVer_Max] 			= { -1,					DRIME4_GPIO7(4), 	DRIME4_GPIO7(4), 	DRIME4_GPIO7(4), 	DRIME4_GPIO7(4), 	DRIME4_GPIO7(4),	0, };
const int gpio_WIFI_PWR_CNT[eBdVer_Max] 		= { -1,					DRIME4_GPIO9(1), 	DRIME4_GPIO9(1),	DRIME4_GPIO9(1),	DRIME4_GPIO9(1),	DRIME4_GPIO9(1),	0, };

const int gpio_LCD_DATA_EN[eBdVer_Max] 		= { DRIME4_GPIO1(3),	DRIME4_GPIO1(3), 	-1, 					-1, 					DRIME4_GPIO1(3), 	DRIME4_GPIO1(3),	0, };
const int gpio_LCD_BL_ON[eBdVer_Max] 			= { -1,					DRIME4_GPIO8(7), 	DRIME4_GPIO8(7), 	DRIME4_GPIO8(7), 	-1, 					-1,	 				0, };
const int gpio_SH_KEY1_MUIC[eBdVer_Max] 		= { -1,					-1,				 	-1, 					-1, 					-1,					DRIME4_GPIO5(7), 	0, };
const int gpio_SH_KEY2_MUIC[eBdVer_Max] 		= { -1,					-1,				 	-1, 					-1, 					-1,					DRIME4_GPIO9(0), 	0, };




//--------------------------------------------------------------------------
//!@brief				Version Information Initialize
//!@note				Version Information Initialize
//--------------------------------------------------------------------------
void VersionInfo_Initialize(void)
{
	volatile unsigned int uiPortData;

	*(volatile unsigned int *)(0xf804a400) &= 0xfffffff8;			// VA of 0x3006a400
	uiPortData = *(volatile unsigned int *)(0xf804a3fc);				// VA of 0x3006a3fc

	switch (uiPortData & 0x7)
	{
		case 0:
			g_eBoardVersion = eBdVer_DV1;
			break;
			
		case 1:
			g_eBoardVersion = eBdVer_JIG2;
			break;
			
		case 2:
			g_eBoardVersion = eBdVer_DV2;
			break;
			
		case 3:
			g_eBoardVersion = eBdVer_DV3;
			break;
			
		case 4:
			g_eBoardVersion = eBdVer_DV4;
			break;
			
		case 5:
			g_eBoardVersion = eBdVer_DV5;
			break;
			
		default:
			g_eBoardVersion = eBdVer_RSV;
			break;
	}

	g_iInitialized = 1;
	g_eBoardVersion = eBdVer_DV3;

	printk("\n\n>>>>>>>>>>>>>>>>>>>>>> H/W Board Version is %s!\n\n", GetBoardVersionName());

	
}


//--------------------------------------------------------------------------
//!@brief				Get Project Name
//!@note				Get Project Name
//--------------------------------------------------------------------------
const char * GetProjectName(void) 
{
	if(!g_iInitialized)
	{
		VersionInfo_Initialize();
	}
	
	return m_pcnchProject;
}

//--------------------------------------------------------------------------
//!@brief				Get F/W Version (Full)
//!@note				Get F/W Version (Full)
//--------------------------------------------------------------------------
const char * GetFwVersionFull(void) 
{
	if(!g_iInitialized)
	{
		VersionInfo_Initialize();
	}

	return m_pcnchFwVerFull;
}

//--------------------------------------------------------------------------
//!@brief				Get F/W Version (User)
//!@note				Get F/W Version (User)
//--------------------------------------------------------------------------
const char* GetFwVersionUser(void) 
{
	if(!g_iInitialized)
	{
		VersionInfo_Initialize();
	}

	return m_pcnchFwVerUser;
}

//--------------------------------------------------------------------------
//!@brief				Get Board Version
//!@note				Get Board Version
//--------------------------------------------------------------------------
EBdVersion GetBoardVersion(void) 
{
	if(!g_iInitialized)
	{
		VersionInfo_Initialize();
	}

	return g_eBoardVersion;
}

//--------------------------------------------------------------------------
//!@brief				Get Board Version Name
//!@note				Get Board Version Name
//--------------------------------------------------------------------------
const char* GetBoardVersionName(void) 
{
	if(!g_iInitialized)
	{
		VersionInfo_Initialize();
	}

	if(g_eBoardVersion == eBdVer_JIG2)
	{
		return g_pcnchBdVerJIG2;
	}
	else if(g_eBoardVersion == eBdVer_DV1)
	{
		return g_pcnchBdVerDV1;
	}
	else if(g_eBoardVersion == eBdVer_DV2)
	{
		return g_pcnchBdVerDV2;
	}
	else if(g_eBoardVersion == eBdVer_DV3)
	{
		return g_pcnchBdVerDV3;
	}
	else if(g_eBoardVersion == eBdVer_DV4)
	{
		return g_pcnchBdVerDV4;
	}
	else if(g_eBoardVersion == eBdVer_DV5)
	{
		return g_pcnchBdVerDV5;
	}
	else if(g_eBoardVersion == eBdVer_PV1)
	{
		return g_pcnchBdVerPV1;
	}
	else if(g_eBoardVersion == eBdVer_PV2)
	{
		return g_pcnchBdVerPV2;
	}
	else if(g_eBoardVersion == eBdVer_PVR)
	{
		return g_pcnchBdVerPVR;
	}
	else if(g_eBoardVersion == eBdVer_PR1)
	{
		return g_pcnchBdVerPR1;
	}	
	else
	{
	    return g_pcnchBdVerRSV;
	}
	
}


