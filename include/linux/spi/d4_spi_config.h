/**
 * @file d4_spi_config.h
 * @brief DRIMe4 spi Structure&Enumeration Define
 * @author kyuchun han <kyuchun.han@samsung.com>
 * 2010 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifndef _D4_SPI_CONFIG_H
#define _D4_SPI_CONFIG_H


enum d4_spi_mode {
	INTERRUPT_TRANSFER,
	POLLING_TRANSFER,
	DMA_TRANSFER
};


enum d4_spi_tx_ch_on_config {
	D4_SPI_TX_CHANNEL_OFF,
	D4_SPI_TX_CHANNEL_ON
};


enum d4_spi_rx_ch_on_config {
	D4_SPI_RX_CHANNEL_OFF,
	D4_SPI_RX_CHANNEL_ON
};

enum d4_spi_mclk {
    D4_SPI_MST_CLK_81M = 81,
    D4_SPI_MST_CLK_100M = 100,
    D4_SPI_MST_CLK_27M = 27
};

enum d4_spi_tran_type {
    D4_SPI_TRAN_BURST_ON,
    D4_SPI_TRAN_BURST_OFF
};

enum d4_spi_invert_type {
    D4_SPI_TRAN_INVERT_OFF,
    D4_SPI_TRAN_INVERT_ON
};


enum d4_spi_ss_togg_set {
	D4_SPI_TOGG_DISABLE_OFF,
	D4_SPI_TOGG_DISABLE_ON
};


enum hs_spi_mode {
	SH_SPI_MODE_0,
	SH_SPI_MODE_1,
	SH_SPI_MODE_2,
	SH_SPI_MODE_3
};

enum hs_spi_select {
	SH_SPI_SPEED			= 0x1,
	SH_SPI_WAVEMODE 	= 0x2,
	SH_SPI_BPW					= 0x4,
	SH_SPI_INVERT			= 0x8,
	SH_SPI_BURST			= 0x10,
	SH_SPI_WAITTIME		= 0x20,
	HS_SPI_DELAY			= 0x40,
	SH_SPI_ALL					= 0x7F
};


enum hs_spi_fix {
	SH_SPI_FIX_EN,
	SH_SPI_FIX_DIS
};

enum hs_spi_sig_select {
	SH_SPI_CLK		= 0x1,
	SH_SPI_DI		= 0x2,
	SH_SPI_DOUT	= 0x4,
	SH_SPI_CS		= 0x8,
};

/**
 * @pre_delay			: delay time between the clock and SSout start signal(usec)
 * @post_delay		: delay time between the clock and SSout end signal(usec)
 * @hold_delay		: delay time between SSout end and SSout start signal(usec)
*/
struct delay_set {
	unsigned int pre_delay;
	unsigned int post_delay;
	unsigned int hold_delay;
};

/**
 * @speed_hz			: SPI working hz
 * @bpw						: useing bits
 * @ss_inven			: inverting chip selection signal
 * @waittime			: waiting time to complete the transfer of spi
 * @spi_ttype			: Set a burst transfer
 * @mode						: Set waveforms for SPICLK
 * @delayconf			: delay setup set
 * @setup_select	: setup value selection for channel configuration
 * @note						: setup_select example -> setup_select = SH_SPI_SPEED | SH_SPI_WAVEMODE  just setup speed and wavemode
 */
struct d4_hs_spi_config {
	unsigned int speed_hz;
	unsigned int bpw;
	enum d4_spi_invert_type ss_inven;
	unsigned int waittime;
	enum d4_spi_tran_type spi_ttype;
	enum hs_spi_mode mode;
	struct delay_set delayconf;
	unsigned int setup_select;

};


/**
 * @*wbuffer			: write buffer pointer
 * @*rbuffer			: read buffer pointer
 * @data_len			: The number of data
 */
struct spi_data_info {
    void *wbuffer;
    void *rbuffer;
    unsigned int data_len;
};


/**
 * @fix_val			: PAD select (EN:spi, DIS:gpio)
 * @pad_di				: Datainput pin
 * @pad_d0				: Dataoutput pin
 * @pad_clk			: Clock pin
 * @pad_cs				: ChipSelec pin
 */
struct spi_pin_info {
		enum hs_spi_fix fix_val;
		enum hs_spi_fix pad_di;
		enum hs_spi_fix pad_do;
		enum hs_spi_fix pad_clk;
		enum hs_spi_fix pad_cs;
};

struct spi_phys_reg_info {
	unsigned int reg_start_addr; /**< Register physical start address */
	unsigned int reg_size;		 /**< Register size */
};

#endif /* _D4_SPI_CONFIG_H */
