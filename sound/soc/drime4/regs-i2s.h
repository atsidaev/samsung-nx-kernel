/* linux/sound/soc/drime4/regs-i2s.h
 *
 * Copyright (c) 2011 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com/
 *
 * DRIME4 I2S register definition
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#ifndef __ASM_ARCH_REGS_DRIME4_I2S_H
#define __ASM_ARCH_REGS_DRIME4_I2S_H

#define DRIME4_I2SCON			(0x00)
#define DRIME4_I2SMOD			(0x04)
#define DRIME4_I2SFIC			(0x08)
#define DRIME4_I2SPSR			(0x0C)
#define DRIME4_I2STXD			(0x10)
#define DRIME4_I2SRXD			(0x14)

#define DRIME4_I2SCON_FRXOFSTATUS	(1 << 19)
#define DRIME4_I2SCON_FRXORINTEN	(1 << 18)
#define DRIME4_I2SCON_FTXURSTATUS	(1 << 17)
#define DRIME4_I2SCON_FTXURINTEN	(1 << 16)

#define DRIME4_I2SCON_LRINDEX		(1 << 11)
#define DRIME4_I2SCON_TXFIFO_EMPTY	(1 << 10)
#define DRIME4_I2SCON_RXFIFO_EMPTY	(1 << 9)
#define DRIME4_I2SCON_TXFIFO_FULL	(1 << 8)
#define DRIME4_I2SCON_RXFIFO_FULL	(1 << 7)
#define DRIME4_I2SCON_TXDMA_PAUSE	(1 << 6)
#define DRIME4_I2SCON_RXDMA_PAUSE	(1 << 5)
#define DRIME4_I2SCON_TXCH_PAUSE	(1 << 4)
#define DRIME4_I2SCON_RXCH_PAUSE	(1 << 3)
#define DRIME4_I2SCON_TXDMA_ACTIVE	(1 << 2)
#define DRIME4_I2SCON_RXDMA_ACTIVE	(1 << 1)
#define DRIME4_I2SCON_I2S_ACTIVE	(1 << 0)

#define DRIME4_I2SMOD_BLC_16BIT		(0 << 13)
#define DRIME4_I2SMOD_BLC_8BIT		(1 << 13)
#define DRIME4_I2SMOD_BLC_24BIT		(2 << 13)
#define DRIME4_I2SMOD_BLC_MASK		(3 << 13)
#define DRIME4_I2SMOD_CDCLKCON		(1 << 12)

#define DRIME4_I2SMOD_SLAVE		(1 << 11)
#define DRIME4_I2SMOD_MODE_TXONLY	(0 << 8)
#define DRIME4_I2SMOD_MODE_RXONLY	(1 << 8)
#define DRIME4_I2SMOD_MODE_TXRX		(2 << 8)
#define DRIME4_I2SMOD_MODE_MASK		(3 << 8)
#define DRIME4_I2SMOD_LR_LLOW		(0 << 7)
#define DRIME4_I2SMOD_LR_RLOW		(1 << 7)
#define DRIME4_I2SMOD_SDF_IIS		(0 << 5)
#define DRIME4_I2SMOD_SDF_MSB		(1 << 5)
#define DRIME4_I2SMOD_SDF_LSB		(2 << 5)
#define DRIME4_I2SMOD_SDF_MASK		(3 << 5)
#define DRIME4_I2SMOD_RCLK_256FS	(0 << 3)
#define DRIME4_I2SMOD_RCLK_512FS	(1 << 3)
#define DRIME4_I2SMOD_RCLK_384FS	(2 << 3)
#define DRIME4_I2SMOD_RCLK_768FS	(3 << 3)
#define DRIME4_I2SMOD_RCLK_MASK		(3 << 3)
#define DRIME4_I2SMOD_BCLK_32FS		(0 << 1)
#define DRIME4_I2SMOD_BCLK_48FS		(1 << 1)
#define DRIME4_I2SMOD_BCLK_16FS		(2 << 1)
#define DRIME4_I2SMOD_BCLK_24FS		(3 << 1)
#define DRIME4_I2SMOD_BCLK_MASK		(3 << 1)

#define DRIME4_I2SPSR_PSREN		(1 << 16)

#define DRIME4_I2SFIC_TXFLUSH		(1 << 15)
#define DRIME4_I2SFIC_TXCOUNT(x)	(((x) >> 8) & 0x7f)
#define DRIME4_I2SFIC_RXFLUSH		(1 << 7)
#define DRIME4_I2SFIC_RXCOUNT(x)	((x) & 0x7f)

#endif
