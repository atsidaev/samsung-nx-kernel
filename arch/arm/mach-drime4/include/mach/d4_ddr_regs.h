/**
 * arch/arm/mach-drime4/include/mach/d4_ddr_regs.h
 *
 * Copyright (c) 2011 Samsung Electronics Co., Ltd.
 *              http://www.samsung.com/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifndef __DRIME4_DDR_REGS_H
#define __DRIME4_DDR_REGS_H


/*
 * Register offsets
 */
#define DDR_CON_CTRL            (0x00)
#define DDR_MEM_CTRL            (0x04)
#define DDR_DIRECT_CMD          (0x10)
#define DDR_PHYZQ_CTRL          (0x44)
#define DDR_CHIP_STATUS(chip)   (0x48 + (chip)*sizeof(unsigned int))


#endif  // __DRIME4_DDR_REGS_H

