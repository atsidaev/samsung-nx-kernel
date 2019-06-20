/**
 * @file d4_reg_macro.h
 * @brief DRIMe4 register macro
 * @author Junkwon Choi <junkwon.choi@samsung.com>
 * 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __D4_REG_MACRO_H__
#define __D4_REG_MACRO_H__

#define DATA_BIT_1		(0x1)
#define DATA_BIT_2		(0x3)
#define DATA_BIT_3		(0x7)
#define DATA_BIT_4		(0xF)
#define DATA_BIT_5		(0x1F)
#define DATA_BIT_6		(0x3F)
#define DATA_BIT_7		(0x7F)
#define DATA_BIT_8		(0xFF)
#define DATA_BIT_9		(0x1FF)
#define DATA_BIT_10		(0x3FF)
#define DATA_BIT_11		(0x7FF)
#define DATA_BIT_12		(0xFFF)
#define DATA_BIT_13		(0x1FFF)
#define DATA_BIT_14		(0x3FFF)
#define DATA_BIT_15		(0x7FFF)
#define DATA_BIT_16		(0xFFFF)
#define DATA_BIT_17		(0x1FFFF)
#define DATA_BIT_18		(0x3FFFF)
#define DATA_BIT_19		(0x7FFFF)
#define DATA_BIT_20		(0xFFFFF)
#define DATA_BIT_21		(0x1FFFFF)
#define DATA_BIT_22		(0x3FFFFF)
#define DATA_BIT_23		(0x7FFFFF)
#define DATA_BIT_24		(0xFFFFFF)
#define DATA_BIT_25		(0x1FFFFFF)
#define DATA_BIT_26		(0x3FFFFFF)
#define DATA_BIT_27		(0x7FFFFFF)
#define DATA_BIT_28		(0xFFFFFFF)
#define DATA_BIT_29		(0x1FFFFFFF)
#define DATA_BIT_30		(0x3FFFFFFF)
#define DATA_BIT_31		(0x7FFFFFFF)
#define DATA_BIT_32		(0xFFFFFFFF)

/*	val	: 레지스터에 입력할 값
 *	x 	: 레지스터 중 특정 비트에 기록할 값
 *	bit	: LSB로부터 몇번째 비트인지 표시함
 *	size	: 값이 몇비트에 걸쳐 기록되는지 설정하는 값
 */
#define SET_REGISTER_VALUE(val, x, bit, size) \
		do { \
			val = ((val & ~(DATA_BIT_##size << bit)) |\
			(((x) & DATA_BIT_##size) << bit)); \
		}  while (0)

/*	val	: 레지스터에 입력할 값
 *	bit	: LSB로부터 몇번째 비트인지 표시함
 *	size	: 값이 몇비트에 걸쳐 기록되는지 설정하는 값
 */
#define GET_REGISTER_VALUE(val, bit, size) \
		((val & (DATA_BIT_##size << bit)) >> bit)

#endif /* __D4_REG_MACRO_H__ */

