/**
 * @file d4_cec_regs.h
 * @brief DRIMe4 HDMI CEC Register definitions and corresponding bit values.
 * @author Somabha Bhattacharjya <b.somabha@samsung.com>
 * 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __D4_CEC_REGS_H
#define __D4_CEC_REGS_H

#define CEC_BASE		0x50130000

/* CEC config/status registers */
#define CEC_TX_STATUS_0		0x0000
#define CEC_TX_STATUS_1		0x0004
#define CEC_RX_STATUS_0		0x0008
#define CEC_RX_STATUS_1		0x000C
#define CEC_INTR_MASK		0x0010
#define CEC_INTR_CLEAR		0x0014
#define CEC_LOGIC_ADDR		0x0020
#define CEC_DIVISOR_0		0x0030
#define CEC_DIVISOR_1		0x0034
#define CEC_DIVISOR_2		0x0038
#define CEC_DIVISOR_3		0x003C

/* CEC Tx related registers */
#define CEC_TX_CTRL		0x0040
#define CEC_TX_BYTES		0x0044
#define CEC_TX_STATUS_2		0x0060
#define CEC_TX_STATUS_3		0x0064
#define CEC_TX_BUFF0		0x0080
#define CEC_TX_BUFF1		0x0084
#define CEC_TX_BUFF2		0x0088
#define CEC_TX_BUFF3		0x008C
#define CEC_TX_BUFF4		0x0090
#define CEC_TX_BUFF5		0x0094
#define CEC_TX_BUFF6		0x0098
#define CEC_TX_BUFF7		0x009C
#define CEC_TX_BUFF8		0x00A0
#define CEC_TX_BUFF9		0x00A4
#define CEC_TX_BUFF10		0x00A8
#define CEC_TX_BUFF11		0x00AC
#define CEC_TX_BUFF12		0x00B0
#define CEC_TX_BUFF13		0x00B4
#define CEC_TX_BUFF14		0x00B8
#define CEC_TX_BUFF15		0x00BC

/* CEC Rx related registers  */
#define CEC_RX_CTRL		0x00C0
#define CEC_RX_STATUS_2		0x00E0
#define CEC_RX_STATUS_3		0x00E4
#define CEC_RX_BUFF0		0x0100
#define CEC_RX_BUFF1		0x0104
#define CEC_RX_BUFF2		0x0108
#define CEC_RX_BUFF3		0x010C
#define CEC_RX_BUFF4		0x0110
#define CEC_RX_BUFF5		0x0114
#define CEC_RX_BUFF6		0x0118
#define CEC_RX_BUFF7		0x011C
#define CEC_RX_BUFF8		0x0120
#define CEC_RX_BUFF9		0x0124
#define CEC_RX_BUFF10		0x0128
#define CEC_RX_BUFF11		0x012C
#define CEC_RX_BUFF12		0x0130
#define CEC_RX_BUFF13		0x0134
#define CEC_RX_BUFF14		0x0138
#define CEC_RX_BUFF15		0x013C

#define CEC_FILTER_CTRL		0x0180
#define CEC_FILTER_TH		0x0184

/* Bit values */
#define CEC_STATUS_TX_RUNNING		(1<<0)
#define CEC_STATUS_TX_TRANSFERRING	(1<<1)
#define CEC_STATUS_TX_DONE		(1<<2)
#define CEC_STATUS_TX_ERROR		(1<<3)

#define CEC_STATUS_RX_RUNNING		(1<<0)
#define CEC_STATUS_RX_RECEIVING		(1<<1)
#define CEC_STATUS_RX_DONE		(1<<2)
#define CEC_STATUS_RX_ERROR		(1<<3)
#define CEC_STATUS_RX_BCAST		(1<<4)

#define CEC_IRQ_TX_DONE			(1<<0)
#define CEC_IRQ_TX_ERROR		(1<<1)
#define CEC_IRQ_RX_DONE			(1<<4)
#define CEC_IRQ_RX_ERROR		(1<<5)

#define CEC_TX_CTRL_START		(1<<0)
#define CEC_TX_CTRL_BCAST		(1<<1)
#define CEC_TX_CTRL_RETRY		(0x05<<4)
#define CEC_TX_CTRL_RESET		(1<<7)

#define CEC_RX_CTRL_ENABLE						(1<<0)
#define CEC_RX_CTRL_CHECK_START_BIT_ERROR		(1<<4)
#define CEC_RX_CTRL_CHECK_LOW_TIME_ERROR		(1<<5)
#define CEC_RX_CTRL_CHECK_SAMPLING_ERROR		(1<<6)
#define CEC_RX_CTRL_RESET						(1<<7)

#define CEC_FILTER_EN			(1<<0)
#define CEC_FILTER_CUR_VAL		(1<<7)

/* CEC Rx buffer size */
#define CEC_RX_BUFF_SIZE		16
/* CEC Tx buffer size */
#define CEC_TX_BUFF_SIZE		16

#define CEC_LOGIC_ADDR_MASK		0x0F

#endif /* __D4_CEC_REGS_H */
