/**
 * @file d4_spi_regs.h
 * @brief  DRIMe4 HS-SPI Registers Define for Device Driver
 * @author Kyuchun Han <kyuchun.han@samsung.com>
 * 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __DRIME4_SPI_REGS_H
#define __DRIME4_SPI_REGS_H

#include <mach/d4_reg_macro.h>


#define SPI_MODULE_NAME		"drime4_spi"

/**
 * struct drime4_spi_info - SPI Controller defining structure
 * @src_clk_nr: Clock source index for the CLK_CFG[SPI_CLKSEL] field.
 * @src_clk_name: Platform name of the corresponding clock.
 * @clk_from_cmu: If the SPI clock/prescalar control block is present
 *	   by the platform's clock-management-unit and not in SPI controller.
 * @num_cs: Number of CS this controller emulates.
 * @cfg_gpio: Configure pins for this SPI controller.
 * @fifo_lvl_mask: All tx fifo_lvl fields start at offset-6
 * @rx_lvl_offset: Depends on tx fifo_lvl field and bus number
 * @high_speed: If the controller supports HIGH_SPEED_EN bit
 * @valid bit: controller masks valid bits on channel width
 *	the value depends on the characteristic of slave device
 */
struct drime4_spi_info {

	int src_clk_nr;
	char *src_clk_name;
	bool clk_from_cmu;

	int num_cs;

/*	int (*cfg_gpio)(struct platform_device *pdev); */

	/* Following two fields are for future compatibility */
	int fifo_lvl_mask;
	int tx_lvl_offset;
	int rx_lvl_offset;
	int high_speed;
	int valid_bits;
};


/******************************************************************************/
/*							Register Offset Define						*/
/******************************************************************************/

/**< HS SPI Registers */
#define SPI_CH_CFG			(0x00)
#define SPI_CLK_PS			(0x04)
#define SPI_FIFO_CFG		(0x08)
#define SPI_SIG_CTRL		(0x0C)
#define SPI_INT_EN			(0x10)
#define SPI_STATUS			(0x14)
#define SPI_TX_DATA			(0x18)
#define SPI_PACKET_CNT		(0x20)
#define SPI_PENDING_CLR		(0x24)
#define SPI_SWAP_CFG		(0x28)
#define SPI_FB_CLK			(0x2C)
#define SPI_CH_ON			(0x30)
#define SPI_VALID_BITS		(0x34)
#define SPI_RX_DATA			(0xF0)

/**< SPI CHANNEL CONFIGURATION REGISTER */
#define SPI_CH_CFG_CPHA(val, x) \
	SET_REGISTER_VALUE(val, x, 2, 1)
#define SPI_CH_CFG_CPOL(val, x) \
	SET_REGISTER_VALUE(val, x, 3, 1)
#define SPI_CH_CFG_MST_SLV(val, x) \
	SET_REGISTER_VALUE(val, x, 4, 1)
#define SPI_CH_CFG_SW_RST(val, x) \
	SET_REGISTER_VALUE(val, x , 5, 1)
#define SPI_CH_CFG_EARLY_DELI_EN(val, x) \
	SET_REGISTER_VALUE(val, x, 6, 1)


/**< SPI CLOCK PRESCALE REGISTER */
#define SPI_CLK_PRESALE(val, x) \
	SET_REGISTER_VALUE(val, x, 0, 4)
#define SPI_CLK_SEL(val, x) \
	SET_REGISTER_VALUE(val, x, 4, 2)


/**< SPI FIFO CONFIGURATION REGISTER */
#define SPI_FIFO_CFG_DMA_TYPE(val, x) \
	SET_REGISTER_VALUE(val, x, 0, 1)
#define SPI_FIFO_CFG_TX_DMA(val, x) \
	SET_REGISTER_VALUE(val, x, 1, 1)
#define SPI_FIFO_CFG_RX_DMA(val, x) \
	SET_REGISTER_VALUE(val, x, 2, 1)
#define SPI_FIFO_CFG_TX_RDY_LVL(val, x) \
	SET_REGISTER_VALUE(val, x, 4, 6)
#define SPI_FIFO_CFG_RX_RDY_LVL(val, x) \
	SET_REGISTER_VALUE(val, x, 12, 6)
#define SPI_FIFO_CFG_FIFO_WIDTH(val, x) \
	SET_REGISTER_VALUE(val, x, 20, 3)
#define SPI_FIFO_CFG_CH_WIDTH(val, x) \
	SET_REGISTER_VALUE(val, x, 22, 3)
#define SPI_FIFO_CFG_TRAILING_CNT(val, x) \
	SET_REGISTER_VALUE(val, x, 24, 8)


/**< SPI GET FIFO CONFIGURATION REGISTER */
#define SPI_GET_FIFO_CFG_DMA_TYPE(val) \
	GET_REGISTER_VALUE(val, 0, 1)
#define SPI_GET_FIFO_CFG_TX_DMA(val) \
	GET_REGISTER_VALUE(val, 1, 1)
#define SPI_GET_FIFO_CFG_RX_DMA(val) \
	GET_REGISTER_VALUE(val, 2, 1)
#define SPI_GET_FIFO_CFG_TX_RDY_LVL(val) \
	GET_REGISTER_VALUE(val, 4, 6)
#define SPI_GET_FIFO_CFG_RX_RDY_LVL(val) \
	GET_REGISTER_VALUE(val, 12, 6)
#define SPI_GET_FIFO_CFG_FIFO_WIDTH(val) \
	GET_REGISTER_VALUE(val, 20, 3)
#define SPI_GET_FIFO_CFG_CH_WIDTH(val) \
	GET_REGISTER_VALUE(val, 22, 3)
#define SPI_GET_FIFO_CFG_TRAILING_CNT(val) \
	GET_REGISTER_VALUE(val, 24, 8)


/**< SPI SIGNAL CONTROL SIGNAL REGISTER */
#define SPI_SIG_CTRL_SS_SIG(val, x) \
	SET_REGISTER_VALUE(val, x, 0, 1)
#define SPI_SIG_CTRL_SS_MODE(val, x) \
	SET_REGISTER_VALUE(val, x, 1, 1)
#define SPI_SIG_CTRL_SS_INVEN(val, x) \
	SET_REGISTER_VALUE(val, x, 2, 1)
#define SPI_SIG_CTRL_SS_TOGG_DIS(val, x) \
	SET_REGISTER_VALUE(val, x, 3, 1)
#define SPI_SIG_CTRL_CNT_HOLD(val, x) \
	SET_REGISTER_VALUE(val, x, 4, 6)
#define SPI_SIG_CTRL_CNT_POST(val, x) \
	SET_REGISTER_VALUE(val, x, 16, 6)
#define SPI_SIG_CTRL_CNT_PRE(val, x) \
	SET_REGISTER_VALUE(val, x, 24, 6)
#define SPI_SIG_CTRL_DELAY_DIS(val, x) \
	SET_REGISTER_VALUE(val, x, 31, 1)


/**< SPI INTERRUPT ENABLE REGISTER */
#define SPI_INT_EN_TX_UNDERRUN(val, x) \
	SET_REGISTER_VALUE(val, x, 2, 1)
#define SPI_INT_EN_TX_OVERRUN(val, x) \
	SET_REGISTER_VALUE(val, x, 3, 1)
#define SPI_INT_EN_RX_UNDERRUN(val, x) \
	SET_REGISTER_VALUE(val, x, 4, 1)
#define SPI_INT_EN_RX_OVERRUN(val, x) \
	SET_REGISTER_VALUE(val, x, 5, 1)
#define SPI_INT_EN_TRAILING(val, x) \
	SET_REGISTER_VALUE(val, x, 6, 1)
#define SPI_INT_EN_TXDONE(val, x) \
	SET_REGISTER_VALUE(val, x, 7, 1)
#define SPI_INT_EN_TX_FIFO_TRIG(val, x) \
	SET_REGISTER_VALUE(val, x, 8, 1)
#define SPI_INT_EN_RX_FIFO_TRIG(val, x) \
	SET_REGISTER_VALUE(val, x, 9, 1)


/**< SPI STATUS/INTERRUPT REASON REGISTER */
#define SPI_STATUS_TX_FIFO_RDY(val) \
	GET_REGISTER_VALUE(val, 0, 1)
#define SPI_STATUS_RX_FIFO_RDY(val) \
	GET_REGISTER_VALUE(val, 1, 1)
#define SPI_STATUS_TX_UNDERRUN(val) \
	GET_REGISTER_VALUE(val, 2, 1)
#define SPI_STATUS_TX_OVERRUN(val) \
	GET_REGISTER_VALUE(val, 3, 1)
#define SPI_STATUS_RX_UNDERRUN(val) \
	GET_REGISTER_VALUE(val, 4, 1)
#define SPI_STATUS_RX_OVERRUN(val) \
	GET_REGISTER_VALUE(val, 5, 1)
#define SPI_STATUS_TRAILING_BYTE(val) \
	GET_REGISTER_VALUE(val, 6, 1)
#define SPI_STATUS_TX_DONE(val) \
	GET_REGISTER_VALUE(val, 7, 1)
#define SPI_STATUS_TX_FIFO_LVL(val) \
	GET_REGISTER_VALUE(val, 8, 6)
#define SPI_STATUS_RX_FIFO_LVL(val) \
	GET_REGISTER_VALUE(val, 15, 6)


/**< SPI TX DATA FIFO REGISTER */
#define SPI_TX_DATA_FIFO(val, x) \
	SET_REGISTER_VALUE(val, x, 0, 31)

/**< SPI PACKET COUNT REGISTER */
#define SPI_PACKET_COUNT_VALUE(val, x) \
	SET_REGISTER_VALUE(val, x, 0, 16)
#define SPI_PACKET_CNT_EN(val, x) \
	SET_REGISTER_VALUE(val, x, 16, 1)


/**< SPI INTERRRUPT PENDDING CLEAR REGISTER */
#define SPI_PEND_CLR_TX_TRIG_INT(val, x) \
	SET_REGISTER_VALUE(val, x, 0, 1)
#define SPI_PEND_CLR_RX_TRIG_INT(val, x) \
	SET_REGISTER_VALUE(val, x, 1, 1)
#define SPI_PEND_CLR_TX_UNDERRUN_CLR(val, x) \
	SET_REGISTER_VALUE(val, x, 2, 1)
#define SPI_PEND_CLR_TX_OVERRUN_CLR(val, x) \
	SET_REGISTER_VALUE(val, x, 3, 1)
#define SPI_PEND_CLR_RX_UNDERRUN_CLR(val, x) \
	SET_REGISTER_VALUE(val, x, 4, 1)
#define SPI_PEND_CLR_RX_OVERRUN_CLR(val, x) \
	SET_REGISTER_VALUE(val, x, 5, 1)
#define SPI_PEND_CLR_TRAILING_CLR(val, x) \
	SET_REGISTER_VALUE(val, x, 6, 1)
#define SPI_PEND_CLR_TX_DONE_CLR(val, x) \
	SET_REGISTER_VALUE(val, x, 7, 1)

#define SPI_GET_PEND_CLR_TX_TRIG_INT(val) \
	GET_REGISTER_VALUE(val, 0, 1)
#define SPI_GET_PEND_CLR_RX_TRIG_INT(val) \
	GET_REGISTER_VALUE(val, 1, 1)
#define SPI_GET_PEND_CLR_TX_UNDERRUN_CLR(val) \
	GET_REGISTER_VALUE(val, 2, 1)
#define SPI_GET_PEND_CLR_TX_OVERRUN_CLR(val) \
	GET_REGISTER_VALUE(val, 3, 1)
#define SPI_GET_PEND_CLR_RX_UNDERRUN_CLR(val) \
	GET_REGISTER_VALUE(val, 4, 1)
#define SPI_GET_PEND_CLR_RX_OVERRUN_CLR(val) \
	GET_REGISTER_VALUE(val, 5, 1)
#define SPI_GET_PEND_CLR_TRAILING_CLR(val) \
	GET_REGISTER_VALUE(val, 6, 1)
#define SPI_GET_PEND_CLR_TX_DONE_CLR(val) \
	GET_REGISTER_VALUE(val, 7, 1)


/**< SPI SWAP CONFIG REGISTER */
#define SPI_SWAP_CFG_TX_SWAP_EN(val, x) \
	SET_REGISTER_VALUE(val, x, 0, 1)
#define SPI_SWAP_CFG_TX_BIT_SWAP(val, x) \
	SET_REGISTER_VALUE(val, x, 1, 1)
#define SPI_SWAP_CFG_TX_BYTE_SWAP(val, x) \
	SET_REGISTER_VALUE(val, x, 2, 1)
#define SPI_SWAP_CFG_TX_HWORD_SWAP(val, x) \
	SET_REGISTER_VALUE(val, x, 3, 1)
#define SPI_SWAP_CFG_RX_SWAP_EN(val, x) \
	SET_REGISTER_VALUE(val, x, 4, 1)
#define SPI_SWAP_CFG_RX_BIT_SWAP(val, x) \
	SET_REGISTER_VALUE(val, x, 5, 1)
#define SPI_SWAP_CFG_RX_BYTE_SWAP(val, x) \
	SET_REGISTER_VALUE(val, x, 6, 1)
#define SPI_SWAP_CFG_RX_HWORD_SWAP(val, x) \
	SET_REGISTER_VALUE(val, x, 7, 1)


/**< SPI FEEDBACK CLOCK SELECTION REGISTER */
#define SPI_FB_CLK_SEL_FB_SEL(val, x) \
	SET_REGISTER_VALUE(val, x, 0, 2)


/**< SPI CHANNEL ON REGISTER */
#define SPI_CH_ON_TX_CH(val, x) \
	SET_REGISTER_VALUE(val, x, 0, 1)
#define SPI_CH_ON_RX_CH(val, x) \
	SET_REGISTER_VALUE(val, x, 1, 1)


/**< SPI VALID BITS REGISTER */
#define SPI_VALID_BITS_SET(val, x) \
	SET_REGISTER_VALUE(val, x, 0, 6)
#define SPI_VALID_BITS_EN(val, x) \
	SET_REGISTER_VALUE(val, x, 8, 1)


/**< SPI RX DATA FIFO REGISTER */
#define SPI_RX_DATA_FIFO(val, x) \
	SET_REGISTER_VALUE(val, x, 0, 31)

#endif /* __DRIME4_SPI_REGS_H */

