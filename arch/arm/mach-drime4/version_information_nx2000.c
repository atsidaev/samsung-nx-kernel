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

static const char* g_pcnchBdVerJIG		= "JIG";
static const char* g_pcnchBdVerDV1		= "DV1";
static const char* g_pcnchBdVerDV2		= "DV2";
static const char* g_pcnchBdVerPV1		= "PV1";
static const char* g_pcnchBdVerPV1B	= "PV1B";
static const char* g_pcnchBdVerPV2		= "PV2";
static const char* g_pcnchBdVerPVR		= "PVR";
static const char* g_pcnchBdVerPR1		= "PR1";
static const char* g_pcnchBdVerRSV		= "N/A";

EBdVersion g_eBoardVersion = eBdVer_RSV;
static int g_iInitialized = 0;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// ----------------------------------------- ---  	 eBdVer_JIG			eBdVer_DV1			eBdVer_DV2			eBdVer_PV1			eBdVer_PV1B			eBdVer_PV2			eBdVer_PVR			eBdVer_PR1,			eBdVer_res	
const int gpio_LCD_BL_ON[eBdVer_Max] 		= { DRIME4_GPIO12(5),	DRIME4_GPIO12(5),	DRIME4_GPIO13(3),	DRIME4_GPIO13(3),	DRIME4_GPIO13(3),	DRIME4_GPIO13(3),	DRIME4_GPIO13(3),	DRIME4_GPIO13(3),	DRIME4_GPIO13(3),	};
const int gpio_EFS_RESET[eBdVer_Max] 		= { DRIME4_GPIO8(1),	DRIME4_GPIO9(0),	DRIME4_GPIO9(0),	DRIME4_GPIO13(6),	DRIME4_GPIO13(6),	DRIME4_GPIO13(6),	DRIME4_GPIO13(6),	DRIME4_GPIO13(6),	DRIME4_GPIO13(6),	};
const int gpio_LCD_ESD_RESET[eBdVer_Max] = { DRIME4_GPIO12(3),	DRIME4_GPIO12(3),	DRIME4_GPIO12(3),	DRIME4_GPIO12(4),	DRIME4_GPIO12(4),	DRIME4_GPIO12(3),	DRIME4_GPIO12(3),	DRIME4_GPIO12(3),	DRIME4_GPIO12(3),	};


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
		case 2:
			g_eBoardVersion = eBdVer_JIG;
			break;

		case 3:
			g_eBoardVersion = eBdVer_DV1;
			break;

		case 4:
			g_eBoardVersion = eBdVer_DV2;
			break;
			
		case 5:
			g_eBoardVersion = eBdVer_PV1;
			break;

		case 6:
			g_eBoardVersion = eBdVer_PV1B;
			break;
			
		case 7:
			g_eBoardVersion = eBdVer_PV2;
			break;

		default:
			g_eBoardVersion = eBdVer_RSV;
			break;
	}

	g_iInitialized = 1;

	printk("\n\n>>>>>>>>>>>>>>>>>>>>>> NX2000 Board Version is %s!\n\n", GetBoardVersionName());

	
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

	if(g_eBoardVersion == eBdVer_JIG)
	{
		return g_pcnchBdVerJIG;
	}
	else if(g_eBoardVersion == eBdVer_DV1)
	{
		return g_pcnchBdVerDV1;
	}
	else if(g_eBoardVersion == eBdVer_DV2)
	{
		return g_pcnchBdVerDV2;
	}
	else if(g_eBoardVersion == eBdVer_PV1)
	{
		return g_pcnchBdVerPV1;
	}
	else if(g_eBoardVersion == eBdVer_PV1B)
	{
		return g_pcnchBdVerPV1B;
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


