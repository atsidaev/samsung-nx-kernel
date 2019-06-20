///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Samsung Electronics Co., LTD., All Rights Reserved.
//!@file				Version.h
//!@brief				System Version Definition
//!@note				System Version Definition<br>
//!					Version Definition
//!@author			Peter Jun
//!@date				Modification<br>
//					2011/01/29	First Draft.<br>
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef __Version_galaxynx_simulator_h__
#define __Version_galaxynx_simulator_h__


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define CSC_PROJECT				"GALAXYNX-SIMULATOR"

#define CSC_VER_DATE_YEAR		"12"
#define CSC_VER_DATE_MON		"08"			// DSP Date
#define CSC_VER_DATE_DAY		"07"			// DSP Date
#define CSC_VER_DATE_COUNT		"1"				// DSP Count

#define CSC_FW_VERSION_USER		"00.01"

#define CSC_FW_TYPE				"FW"
#define CSC_REGION			  	"X"
#define CSC_UPDATE_TYPE			"F"


#define CSC_FW_VERSION_FULL	CSC_PROJECT "_" CSC_FW_TYPE "_" CSC_REGION "_"  CSC_UPDATE_TYPE \
								CSC_VER_DATE_YEAR CSC_VER_DATE_MON CSC_VER_DATE_DAY CSC_VER_DATE_COUNT		 //"NX200F_FW_X_F1102111"

#endif		//__Version_galaxynx_simulator_h__

