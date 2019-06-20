/* linux/sound/soc/drime4/drime4-i2s.h
 *
 * Copyright (c) 2011 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com/
 *
 * DRIME4 I2S definition
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#ifndef __SND_SOC_DRIME4_I2S_H
#define __SND_SOC_DRIME4_I2S_H

#define DRIME4_I2S_DIV_BCLK	(1)
#define DRIME4_I2S_DIV_RCLK	(2)
#define DRIME4_I2S_DIV_PRESCALER	(3)

#define DRIME4_I2S_CLKSRC_PCLK		0
#define DRIME4_I2S_CLKSRC_AUDIOBUS	1
#define DRIME4_I2S_CLKSRC_CDCLK		2

/* Set this flag for I2S controllers that have the bit IISMOD[12]
 * bridge/break RCLK signal and external Xi2sCDCLK pin.
 */
#define DRIME4_FEATURE_CDCLKCON	(1 << 0)

#define DRIME4_I2S_RATES \
	(SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_11025 | SNDRV_PCM_RATE_16000 | \
	SNDRV_PCM_RATE_22050 | SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_44100 | \
	SNDRV_PCM_RATE_48000 | SNDRV_PCM_RATE_88200 | SNDRV_PCM_RATE_96000)

#define DRIME4_I2S_FMTS \
	(SNDRV_PCM_FMTBIT_S8 | SNDRV_PCM_FMTBIT_S16_LE |\
	 SNDRV_PCM_FMTBIT_S24_LE)

extern struct snd_soc_dai drime4_i2s_dai;

/**
 * struct drime4_i2s_info - DRIME4 I2S information
 * @dev: The parent device passed to use from the probe.
 * @regs: The pointer to the device registe block.
 * @feature: Set of bit-flags indicating features of the controller.
 * @master: True if the I2S core is the I2S bit clock master.
 * @dma_playback: DMA information for playback channel.
 * @dma_capture: DMA information for capture channel.
 * @suspend_iismod: PM save for the IISMOD register.
 * @suspend_iiscon: PM save for the IISCON register.
 * @suspend_iispsr: PM save for the IISPSR register.
 *
 * This is the private codec state for the hardware associated with an
 * I2S channel such as the register mappings and clock sources.
 */
struct drime4_i2s_info {
	struct device	*dev;
	void __iomem	*regs;

	u32		feature;

	struct clk	*iis_cclk;

	unsigned char	 master;

	struct drime4_dma_params	*dma_playback;
	struct drime4_dma_params	*dma_capture;

	u32		 suspend_iismod;
	u32		 suspend_iiscon;
	u32		 suspend_iispsr;

	unsigned long	base;
};

struct drime4_i2s_rate_calc {
	unsigned int	clk_div;	/* for prescaler */
	unsigned int	fs_div;		/* for root frame clock */
};

extern int drime4_i2s_iis_calc_rate(struct drime4_i2s_rate_calc *info,
				    unsigned int *fstab,
				    unsigned int rate, unsigned long clkrate);

#endif
