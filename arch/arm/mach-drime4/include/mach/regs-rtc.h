/* linux/arch/arm/mach-drime4/include/mach/regs-rtc.h
 *
 * Copyright (c) 2011 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com/
 *
 * DRIME4 - RTC register definition
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#ifndef __MACH_DRIME4_REGS_RTC_H
#define __MACH_DRIME4_REGS_RTC_H

#define DRIME4_RTCCON			0x0
#define DRIME4_RTCCON_STARTB		(1 << 0)
#define DRIME4_RTCCON_RTCEN		(1 << 1)
#define DRIME4_RTCCON_CLKRST		(1 << 2)
#define DRIME4_RTCCON_OSCEN		(1 << 3)

#define DRIME4_RTCALM			0x4
#define DRIME4_RTCALM_SECEN		(1 << 0)
#define DRIME4_RTCALM_MINEN		(1 << 1)
#define DRIME4_RTCALM_HOUREN		(1 << 2)
#define DRIME4_RTCALM_DATEEN		(1 << 3)
#define DRIME4_RTCALM_DAYEN		(1 << 4)
#define DRIME4_RTCALM_MONEN		(1 << 5)
#define DRIME4_RTCALM_YEAREN		(1 << 6)
#define DRIME4_RTCALM_ALMEN		(1 << 7)

#define DRIME4_ALMSEC			0x8
#define DRIME4_ALMMIN			0xc
#define DRIME4_ALMHOUR			0x10
#define DRIME4_ALMDATE			0x14
#define DRIME4_ALMDAY			0x18
#define DRIME4_ALMMON			0x1c
#define DRIME4_ALMYEAR			0x20

#define DRIME4_BCDSEC			0x24
#define DRIME4_BCDMIN			0x28
#define DRIME4_BCDHOUR			0x2c
#define DRIME4_BCDDATE			0x30
#define DRIME4_BCDDAY			0x34
#define DRIME4_BCDMON			0x38
#define DRIME4_BCDYEAR			0x3c

#define DRIME4_RTCIM			0x40
#define DRIME4_RTCIM_INTMODE_EN_ALM	(1 << 0)
#define DRIME4_RTCIM_INTMODE_LVL_ALMINT	(3 << 0)
#define DRIME4_RTCIM_PEIMODE_PES_EN	(1 << 2)
#define DRIME4_RTCIM_PES_1_PER_256	(1 << 3)
#define DRIME4_RTCIM_PES_1_PER_64	(2 << 3)
#define DRIME4_RTCIM_PES_1_PER_16	(3 << 3)
#define DRIME4_RTCIM_PES_1_PER_4	(4 << 3)
#define DRIME4_RTCIM_PES_1_PER_2	(5 << 3)
#define DRIME4_RTCIM_PES_1		(6 << 3)

#define DRIME4_RTC_PEND			0x44
#define DRIME4_RTC_PEND_CLEAR		(0 << 0)
#define DRIME4_RTC_PEND_PENDING		(1 << 0)

#define DRIME4_RTC_PRIPEND		0x48
#define DRIME4_RTC_PRIPEND_CLEAR		(0 << 0)
#define DRIME4_RTC_PRIPEND_PENDING		(1 << 0)

#define DRIME4_RTC_WUPEND		0x4C
#define DRIME4_RTC_WUPEND_CLEAR		(0 << 0)
#define DRIME4_RTC_WUPEND_PENDING	(1 << 0)

#endif
