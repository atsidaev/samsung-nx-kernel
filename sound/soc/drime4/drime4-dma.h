/* linux/sound/soc/drime4/drime4-dma.h
 *
 * Copyright (c) 2011 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com/
 *
 * DRIME4 I2S DMA definition
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#ifndef __SND_SOC_DRIME4_DMA_H
#define __SND_SOC_DRIME4_DMA_H

#define ST_RUNNING		(1<<0)
#define ST_OPENED		(1<<1)

struct drime4_dma_params {
	struct drime4_dma_client *client;	/* stream identifier */
	int channel;				/* Channel ID */
	dma_addr_t dma_addr;
	int dma_size;			/* Size of the DMA transfer */
	struct dma_chan *chan;
	struct dma_async_tx_descriptor *desc;
};

#define DRIME4_DAI_I2S			0

/* platform data */
extern struct snd_ac97_bus_ops drime4_ac97_ops;

#endif
