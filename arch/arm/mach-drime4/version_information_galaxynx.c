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
#include "include/mach/version_galaxynx.h"
#include "include/mach/gpio.h"
#include <mach/map.h>
#include <linux/delay.h>
static const char* m_pcnchProject		= CSC_PROJECT;
static const char* m_pcnchFwVerFull 	= CSC_FW_VERSION_FULL;
static const char* m_pcnchFwVerUser 	= CSC_FW_VERSION_USER;

static const char* g_pcnchBdVerES1		= "ES2";
static const char* g_pcnchBdVerES2		= "ES3";
static const char* g_pcnchBdVerEMU0	= "EMU0";
static const char* g_pcnchBdVerPDI1	= "PDI1";
static const char* g_pcnchBdVerPDI2	= "PDI2";
static const char* g_pcnchBdVerDV1		= "DV1";
static const char* g_pcnchBdVerPV1		= "PV1";
static const char* g_pcnchBdVerPV2		= "PV2";
static const char* g_pcnchBdVerPR1		= "PR1";
static const char* g_pcnchBdVerRSV		= "N/A";

EBdVersion g_eBoardVersion = eBdVer_ES2;
static int g_iInitialized = 0;

/* ---------------------------------------------	eBdVer_JIG2			eBdVer_JIG3			eBdVer_PDI2			eBdVer_PDI1       eBdVer_DV1*/

const int gpio_ISP_SCL[eBdVer_Max]			 					= {	DRIME4_GPIO15(5), 	DRIME4_GPIO5(3), 		0, 0, 0};
const int gpio_ISP_SDA[eBdVer_Max]			 					= { DRIME4_GPIO15(6), 	DRIME4_GPIO5(4), 		0, 0, 0};
const int gpio_SHUTTER_KEY1_DSP[eBdVer_Max] 		= { DRIME4_GPIO6(0), 	DRIME4_GPIO5(5), 		0, DRIME4_GPIO9(1), DRIME4_GPIO9(1)};
const int gpio_SHUTTER_KEY2[eBdVer_Max]					= { DRIME4_GPIO6(1),		DRIME4_GPIO5(6), 		0, DRIME4_GPIO9(3), DRIME4_GPIO9(3)};
const int gpio_EXT_STR_CTL_IN[eBdVer_Max]				= {	DRIME4_GPIO5(5), 		DRIME4_GPIO10(4), 	0, 0, 0};
const int gpio_D4C_INT[eBdVer_Max]								= {	DRIME4_GPIO6(4), 		DRIME4_GPIO12(3), 	DRIME4_GPIO12(3), DRIME4_GPIO12(3), DRIME4_GPIO12(3),};
const int gpio_D4C_RESET[eBdVer_Max]						= {	DRIME4_GPIO5(2), 		DRIME4_GPIO12(5),		DRIME4_GPIO12(5),	 DRIME4_GPIO12(5), DRIME4_GPIO9(6),};
const int gpio_D4C_AN_ON[eBdVer_Max]						= {	DRIME4_GPIO8(3), 		DRIME4_GPIO12(6),		DRIME4_GPIO12(6),	 DRIME4_GPIO12(6), DRIME4_GPIO9(7),};
const int gpio_D4C_CORE_ON[eBdVer_Max]					= {	DRIME4_GPIO8(6), 		DRIME4_GPIO13(6), 	DRIME4_GPIO13(6), DRIME4_GPIO13(6), DRIME4_GPIO13(6),};
const int gpio_SH_MOT_IN1[eBdVer_Max] 					= { DRIME4_GPIO8(4), 	DRIME4_GPIO13(7), 	DRIME4_GPIO13(7), DRIME4_GPIO13(7), DRIME4_GPIO13(7),};
const int gpio_SH_MOT_IN2[eBdVer_Max] 					= { DRIME4_GPIO8(5),		DRIME4_GPIO14(0), 	DRIME4_GPIO14(0),	 DRIME4_GPIO14(0), DRIME4_GPIO14(0),};
const int gpio_SH_PI1[eBdVer_Max] 								= { DRIME4_GPIO6(2), 	DRIME4_GPIO15(5), 	DRIME4_GPIO22(3), DRIME4_GPIO22(3), DRIME4_GPIO22(3),};
const int gpio_SH_PI2[eBdVer_Max] 								= { DRIME4_GPIO6(3), 	DRIME4_GPIO15(6), 	DRIME4_GPIO22(5), DRIME4_GPIO22(5), DRIME4_GPIO22(5),};
const int gpio_ISP_POWER_ON[eBdVer_Max] 				= { DRIME4_GPIO1(0), 	DRIME4_GPIO23(3), 	0, 0, 0};
const int gpio_LENS_RB[eBdVer_Max]								= { DRIME4_GPIO6(6), 	DRIME4_GPIO25(5), 	DRIME4_GPIO25(5),		DRIME4_GPIO25(5), DRIME4_GPIO25(5), };
const int gpio_D4C_IO_ON[eBdVer_Max]						= { DRIME4_GPIO9(2), 	DRIME4_GPIO26(5),		DRIME4_GPIO16(1), 	DRIME4_GPIO16(1), DRIME4_GPIO16(1),};
const int gpio_EFS_RESET[eBdVer_Max]						= { DRIME4_GPIO9(0), 	DRIME4_GPIO27(5), 	DRIME4_GPIO25(1),		DRIME4_GPIO9(2), DRIME4_GPIO9(2),};

const int gpio_EXT_STR_DET[eBdVer_Max]					= { DRIME4_GPIO7(0), 	0, 										DRIME4_GPIO10(3),		DRIME4_GPIO10(3), DRIME4_GPIO10(3),};
const int gpio_HDR_RESET[eBdVer_Max] 						= { DRIME4_GPIO8(1),		0, 0,};
const int gpio_AF_LED[eBdVer_Max] 								= { DRIME4_GPIO24(1), 	0, 										DRIME4_GPIO24(1),		DRIME4_GPIO24(1),DRIME4_GPIO24(1),};
const int gpio_CARD_LED[eBdVer_Max] 						= { DRIME4_GPIO24(2),	0, 0, 0, 0,};
const int gpio_DRS_PWM_PLUS_CON[eBdVer_Max] 		= { DRIME4_GPIO26(3),	0, 0, 0, 0,};
const int gpio_DRS_PWM_MINUS_CON[eBdVer_Max] 	= { DRIME4_GPIO26(4),	0, 0, 0, 0,};
/*
const int gpio_WIFI_SDIO_D0[eBdVer_Max] 				= { DRIME4_GPIO26(5),	0, 0, 0, 0,};
const int gpio_WIFI_SDIO_D1[eBdVer_Max] 				= { DRIME4_GPIO26(6),	0, 0, 0, 0,};
const int gpio_WIFI_SDIO_D2[eBdVer_Max] 				= { DRIME4_GPIO26(7),	0, 0, 0, 0,};
const int gpio_WIFI_SDIO_D3[eBdVer_Max] 				= { DRIME4_GPIO27(0),	0, 0, 0, 0,};
const int gpio_WIFI_SDIO_CMD[eBdVer_Max] 				= { DRIME4_GPIO27(5),	0, 0, 0, 0,};
const int gpio_WIFI_SDIO_CLK[eBdVer_Max] 				= { DRIME4_GPIO27(6),	0, 0, 0, 0,};
const int gpio_WIFI_SDIO_RESET[eBdVer_Max] 		= { DRIME4_GPIO27(7),	0, 0, 0, 0,};
*/

/*D4 RB AP_INT define*/
const int gpio_READY_BUSY[eBdVer_Max] 					= {	DRIME4_GPIO12(3), 	DRIME4_GPIO6(7), DRIME4_GPIO27(7), DRIME4_GPIO27(7), DRIME4_GPIO27(7)};
/*D4_INT define for notification*/
const int gpio_ISP_INT[eBdVer_Max]								= {	DRIME4_GPIO12(3), 	DRIME4_GPIO6(7), 		DRIME4_GPIO26(5), DRIME4_GPIO26(5), DRIME4_GPIO26(5)};

const int gpio_LENS_3_3V_ON[eBdVer_Max] 				= { DRIME4_GPIO25(1), DRIME4_GPIO25(1),	DRIME4_GPIO22(7), DRIME4_GPIO22(7), DRIME4_GPIO22(7),};
const int gpio_LENS_5_0V_ON[eBdVer_Max] 				= { DRIME4_GPIO25(2), DRIME4_GPIO25(2),	DRIME4_GPIO22(2), DRIME4_GPIO22(2), DRIME4_GPIO22(2),};

const int gpio_GPIO_EYE_INT[eBdVer_Max] 				= { DRIME4_GPIO12(2), DRIME4_GPIO12(2), DRIME4_GPIO12(2), 0, 0};
const int gpio_GPIO_REC_KEY[eBdVer_Max] 				= { 0, 0, 0, DRIME4_GPIO12(2),DRIME4_GPIO12(2),};
const int gpio_USB_DET_KEY[eBdVer_Max] 					= { DRIME4_GPIO16(1), DRIME4_GPIO16(1), DRIME4_GPIO16(1), 0, 0};

const int gpio_DRS_INV_EN[eBdVer_Max] 					= { DRIME4_GPIO24(3), DRIME4_GPIO24(3), 0, 0, 0};
const int gpio_STR_PU_CNT[eBdVer_Max] 					= { 0, 0, DRIME4_GPIO24(3), DRIME4_GPIO24(3),DRIME4_GPIO24(3),};
const int gpio_CIS_RESET[eBdVer_Max] 						= { DRIME4_GPIO25(4), DRIME4_GPIO25(4), 0, 0, 0};
const int gpio_STR_CHANGE[eBdVer_Max] 					= { 0, 0, DRIME4_GPIO24(4), DRIME4_GPIO24(4),DRIME4_GPIO24(4),};
const int gpio_STR_READY[eBdVer_Max] 						= { 0, 0, DRIME4_GPIO25(7), DRIME4_GPIO25(7),DRIME4_GPIO25(7),};
const int gpio_STR_CTL_IN[eBdVer_Max]						= {0, 0, 	DRIME4_GPIO11(6), DRIME4_GPIO11(6),  DRIME4_GPIO11(6),};

/*
const int gpio_SH_MG1_ON_OUT[eBdVer_Max]				= {DRIME4_GPIO12(7), DRIME4_GPIO12(7),DRIME4_GPIO12(4), DRIME4_GPIO12(7)};
const int gpio_SH_MG2_ON_OUT[eBdVer_Max]				= {DRIME4_GPIO12(0), DRIME4_GPIO12(0),DRIME4_GPIO12(5), DRIME4_GPIO12(0)};*/
//--------------------------------------------------------------------------
//!@brief				Version Information Initialize
//!@note				Version Information Initialize
//--------------------------------------------------------------------------
int temp1 = 0;
#ifdef CONFIG_DRIME4_GALAXYNX_REAL_BOARD
void VersionInfo_Initialize(void)
{

	volatile unsigned int uiPortData;
	unsigned int temp = 0;

	/*BVC0*/
	*(volatile unsigned int *) (DRIME4_VA_GLOBAL_CTRL + 0x3AC) = 0x00020840;

	/*BVC1*/
	*(volatile unsigned int *) (DRIME4_VA_GLOBAL_CTRL + 0x3B4) = 0x00020840;

	/*BVC2*/
		*(volatile unsigned int *) (DRIME4_VA_GLOBAL_CTRL + 0x3BC) = 0x00020840;


	/*BVC GPIO DIRECTION*/
	*(volatile unsigned int *)(DRIME4_VA_GPIO + 0x16400) &= 0xFFFFFFAF;



	/*Read BVC0*/
	temp = *(volatile unsigned int *)(DRIME4_VA_GPIO + 0x163FC);
	uiPortData |= (temp & (0x00000040)) >> 6;


	/*Read BVC1*/
		temp = *(volatile unsigned int *)(DRIME4_VA_GPIO + 0x163FC);
		uiPortData = (temp & (0x00000010)) >> 3;


	/*Read BVC2*/
	temp = *(volatile unsigned int *)(DRIME4_VA_GPIO + 0x173FC);
	uiPortData |= (temp & (0x00000001)) << 2 ;

	/*D4C Set Port*/
	/* 13 6 */
	*(volatile unsigned int *) (DRIME4_VA_GLOBAL_CTRL + 0x3BC) = 0x00020840;
	/* 9  7 */
	*(volatile unsigned int *) (DRIME4_VA_GLOBAL_CTRL + 0x164) = 0x00020840;
	/* 16 1 */
	*(volatile unsigned int *) (DRIME4_VA_GLOBAL_CTRL + 0x254) = 0x00020840;
	/* 9 6*/
	*(volatile unsigned int *) (DRIME4_VA_GLOBAL_CTRL + 0x160) = 0x00020840;
	/* 12 6*/
	*(volatile unsigned int *) (DRIME4_VA_GLOBAL_CTRL + 0x1E8) = 0x00020840;
	/* 12 5*/
	*(volatile unsigned int *) (DRIME4_VA_GLOBAL_CTRL + 0x1E4) = 0x00020840;


	/*DV1, PDI2*/
	/*D4C_CORE_ON  13 6*/
	*(volatile unsigned int *)(DRIME4_VA_GPIO + 0xD400) = 0x0000040;
	*(volatile unsigned int *)(DRIME4_VA_GPIO + 0xD3FC) = 0x0000000;
	/*D4C_AN_ON 9 7, RESET 9 6*/
	*(volatile unsigned int *)(DRIME4_VA_GPIO + 0x9400) = 0x00000C0;
	*(volatile unsigned int *)(DRIME4_VA_GPIO + 0x93FC) = 0x0000000;
	/*D4C_IO_ON 16 1*/
	*(volatile unsigned int *)(DRIME4_VA_GPIO + 0x10400) = 0x0000001;
	*(volatile unsigned int *)(DRIME4_VA_GPIO + 0x103FC) = 0x0000000;

	/*D4C_AN_ON , RESET 12 5-6*/
	*(volatile unsigned int *)(DRIME4_VA_GPIO + 0xC400) = 0x0000060;
	*(volatile unsigned int *)(DRIME4_VA_GPIO + 0xC3FC) = 0x0000000;

	/*DV1 PDI2*/
	/*D4C_CORE_ON  DV1, PDI2 13 6*/
	temp = *(volatile unsigned int *)(DRIME4_VA_GPIO + 0xD3FC);
	temp |= 0x00000040;
	*(volatile unsigned int *)(DRIME4_VA_GPIO + 0xD3FC) = temp;

	/*D4C_AN_ON - DV1 12 6*/
	temp = *(volatile unsigned int *)(DRIME4_VA_GPIO + 0x93FC);
	temp |= 0x00000080;
	*(volatile unsigned int *)(DRIME4_VA_GPIO + 0x93FC) = temp;

	/*D4C_AN_ON - PDI2 9 7*/
	temp = *(volatile unsigned int *)(DRIME4_VA_GPIO + 0x93FC);
	temp |= 0x00000080;
	*(volatile unsigned int *)(DRIME4_VA_GPIO + 0x93FC) = temp;
	/*D4C_IO_ON  DV1, PDI2 16 1*/
	temp = *(volatile unsigned int *)(DRIME4_VA_GPIO + 0x103FC);
	temp |= 0x00000001;
		*(volatile unsigned int *)(DRIME4_VA_GPIO + 0x103FC) = temp;

	/*D4C Reset Low DV1 9 6*/
	temp = *(volatile unsigned int *)(DRIME4_VA_GPIO + 0x93FC);
	temp &= 0x40;
	*(volatile unsigned int *)(DRIME4_VA_GPIO + 0x93FC) = temp;
	/*D4C Reset Low DV1 12 5*/
	temp = *(volatile unsigned int *)(DRIME4_VA_GPIO + 0xC3FC);
	temp &= 0x20;
	*(volatile unsigned int *)(DRIME4_VA_GPIO + 0xC3FC) = temp;

	if (uiPortData == 3) {
		uiPortData = 0x2;
	}
	if (uiPortData == 4) {
		uiPortData = 0x2;
	}

	switch (uiPortData & 0x7) {
	case 0:
		g_eBoardVersion = eBdVer_PDI2;
		break;
	case 1:
		g_eBoardVersion = eBdVer_PDI2;
		break;
	case 2:
		g_eBoardVersion = eBdVer_DV1;
		break;
	case 3:
		g_eBoardVersion = eBdVer_PV1;
		break;
	case 4:
		g_eBoardVersion = eBdVer_PV2;
		break;
	default:
		g_eBoardVersion = eBdVer_PR1;
		break;
	}
	g_iInitialized = 1;
}
#else
void VersionInfo_Initialize(void)
{
	unsigned int  uiBvc0, uiBvc1, uiBvc2;
	volatile unsigned int uiPortData;

	*(volatile unsigned int *)(0xf804a400) &= 0xfffffff8;			// VA of 0x3006a400
	uiPortData = *(volatile unsigned int *)(0xf804a3fc);				// VA of 0x3006a3fc

	uiBvc0 = (uiPortData & 0x1);
	uiBvc1 = (uiPortData & 0x2) >> 1;
	uiBvc2 = (uiPortData & 0x4) >> 2;

	//------------------------------------------------
	// Board	BVC2		BVC1		BVC0
	//------------------------------------------------
	// JIG2		0			0			0
	// JIG3		0			0			1
	// DV1		0			1			0
	// DV2		0			1			1
	// DV3		1			0			0
	// PV1		1			0			1
	// PV2		1			1			0
	// PR1		1			1			1
	//------------------------------------------------

	switch (uiPortData & 0x7)
	{
		case 0:
			g_eBoardVersion = eBdVer_ES1;
			break;

		case 1:
			g_eBoardVersion = eBdVer_ES2;
			break;

		default:
			break;
	}

	g_iInitialized = 1;

	printk("\n\n>>>>>>>>>>>>>>>>>>>>>> H/W Board Version is %s!\n\n", GetBoardVersionName());


}
#endif

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

	if(g_eBoardVersion == eBdVer_ES1)
	{
		return g_pcnchBdVerES1;
	}
	else if(g_eBoardVersion == eBdVer_ES2)
	{
		return g_pcnchBdVerES2;
	}
	else if(g_eBoardVersion == eBdVer_PDI2)
	{
		return g_pcnchBdVerPDI2;
	}
	else if(g_eBoardVersion == eBdVer_PDI1)
	{
		return g_pcnchBdVerPDI1;
	}
		else if(g_eBoardVersion == eBdVer_DV1)
	{
		return g_pcnchBdVerDV1;
	}
	else if(g_eBoardVersion == eBdVer_PV1)
	{
		return g_pcnchBdVerPV1;
	}
	else if(g_eBoardVersion == eBdVer_PV2)
	{
		return g_pcnchBdVerPV2;
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


