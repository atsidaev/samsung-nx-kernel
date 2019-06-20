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


#ifndef __Version_h__
#define __Version_h__

#if defined(CONFIG_MACH_D4_NX300)
#include "version_nx300.h"
#elif defined(CONFIG_MACH_D4_GALAXYNX)
#include "version_galaxynx.h"
#elif defined(CONFIG_MACH_D4_GALAXYNX_SIMULATOR)
#include "version_galaxynx_simulator.h"
#else
#include "version_nx300.h"	/* default case */
#endif


#endif		// __Version_h__

