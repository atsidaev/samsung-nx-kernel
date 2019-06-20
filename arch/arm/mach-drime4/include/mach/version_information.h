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


#ifndef __VersionInformation_h__
#define __VersionInformation_h__

#define CONFIG_BOOT_TIME_CHECK

#ifdef CONFIG_BOOT_TIME_CHECK
	#if defined(CONFIG_MACH_D4_NX300)
	#define SW_DBG_P0_HIGH()	*(volatile unsigned int *)(0xf80483fc) |= 0x10;			// TP30 : GPIO_24(4)
	#define SW_DBG_P0_LOW() 	*(volatile unsigned int *)(0xf80483fc) &= 0xffffffef;	// TP30 : GPIO_24(4)
	#define SW_DBG_P1_HIGH()	*(volatile unsigned int *)(0xf803c3fc) |= 0x10;			// TP15 : GPIO_12(4)
	#define SW_DBG_P1_LOW()     	*(volatile unsigned int *)(0xf803c3fc) &= 0xffffffef;	// TP15 : GPIO_12(4)
	#elif defined(CONFIG_MACH_D4_NX2000)
	#define SW_DBG_P0_HIGH()	*(volatile unsigned int *)(0xf80483fc) |= 0x10; 		// TP30 : GPIO_24(4)
	#define SW_DBG_P0_LOW() 	*(volatile unsigned int *)(0xf80483fc) &= 0xffffffef;	// TP30 : GPIO_24(4)
	#define SW_DBG_P1_HIGH()
	#define SW_DBG_P1_LOW()
	#else
	#define SW_DBG_P0_HIGH()
	#define SW_DBG_P0_LOW()
	#define SW_DBG_P1_HIGH()
	#define SW_DBG_P1_LOW()
	#endif
#else
	#define SW_DBG_P0_HIGH()
	#define SW_DBG_P0_LOW()
	#define SW_DBG_P1_HIGH()
	#define SW_DBG_P1_LOW()
#endif

#if defined(CONFIG_MACH_D4_NX300)
#include "version_information_nx300.h"
#elif defined(CONFIG_MACH_D4_NX2000)
#include "version_information_nx2000.h"
#elif defined(CONFIG_MACH_D4_GALAXYNX)
#include "version_information_galaxynx.h"
#elif defined(CONFIG_MACH_D4_GALAXYNX_SIMULATOR)
#include "version_information_galaxynx_simulator.h"
#else
#include "version_information_nx300.h"	/* default case */
#endif


#endif		// __VersionInformation_h__

