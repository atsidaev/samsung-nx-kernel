/**
 * @file d4_pp_3a_regs.h
 * @brief	DRIMe4 PP 3A Register Define for Device Driver
 * @author Kyounghwan Moon <kh.moon@samsung.com>,
 * 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __DRIME4_REGS_PP_3A_H
#define __DRIME4_REGS_PP_3A_H

#include <mach/d4_reg_macro.h>

/******************************************************************************/
/*                          Register Offset Define                            */
/******************************************************************************/
#define PP_3A_REGISTER_000_OFFSET	0x0000
#define PP_3A_REGISTER_004_OFFSET	0x0004
#define PP_3A_REGISTER_170_OFFSET	0x0170
#define PP_3A_REGISTER_1FC_OFFSET	0x01FC

/******************************************************************************/
/*                        Register Structure Define                           */
/******************************************************************************/
/**< PP 3A Interrupt Control Register */
/* offset 0x004 */
#define D4_PP_3A_GET_SEL_BANK_INDICATE(val)						GET_REGISTER_VALUE(val, 24, 1)

/**< PP 3A  Interrupt Flag Clear Register */
/* offset 0x1FC */
#define D4_PP_3A_SET_DMA_INT_CLEAR(val, x)						SET_REGISTER_VALUE(val, x, 8, 1)

#define D4_PP_3A_SET_CTRL_0_3A_BLOCK_ENABLE(val, x)				SET_REGISTER_VALUE(val, x, 1, 1)

#define D4_PP_3A_SET_DMA_INT_EN(val, x)							SET_REGISTER_VALUE(val, x, 8, 1)

#define STATIC_A3_STATUS_DISABLE 0

#endif /* __DRIME4_REGS_PP_3A_H */

