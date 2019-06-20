/* linux/sound/soc/drime4/drime4-dma.c
 *
 * Copyright (c) 2011 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com/
 *
 * ALSA SoC Audio Layer - DRIME4 ALSA DMA support
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/module.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/dma-mapping.h>

#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>

#include <linux/dmaengine.h>
#include <linux/amba/pl330.h>

#include "drime4-dma.h"

static const struct snd_pcm_hardware dma_hardware = {
	.info			= SNDRV_PCM_INFO_INTERLEAVED |
				    SNDRV_PCM_INFO_BLOCK_TRANSFER |
				    SNDRV_PCM_INFO_MMAP |
				    SNDRV_PCM_INFO_MMAP_VALID |
				    SNDRV_PCM_INFO_PAUSE |
				    SNDRV_PCM_INFO_RESUME,
	.formats		= SNDRV_PCM_FMTBIT_S16_LE |
				    SNDRV_PCM_FMTBIT_U16_LE |
				    SNDRV_PCM_FMTBIT_U8 |
				    SNDRV_PCM_FMTBIT_S8,
	.channels_min		= 2,
	.channels_max		= 2,
	.buffer_bytes_max	= 128*1024,
	.period_bytes_min	= PAGE_SIZE,
	.period_bytes_max	= PAGE_SIZE*2,
	.periods_min		= 2,
	.periods_max		= 128,
	.fifo_size		= 32,
};

struct runtime_data {
	spinlock_t lock;
	int state;
	unsigned int dma_loaded;
	unsigned int dma_limit;
	unsigned int dma_period;
	dma_addr_t dma_start;
	dma_addr_t dma_pos;
	dma_addr_t dma_end;
	struct drime4_dma_params *params;
};

static void audio_buffdone(void *data);

/* dma_enqueue
 *
 * place a dma buffer onto the queue for the dma system
 * to handle.
*/
static void dma_enqueue(struct snd_pcm_substream *substream)
{
	struct runtime_data *prtd = substream->runtime->private_data;
	dma_addr_t pos = prtd->dma_pos;
	unsigned int limit;
	struct dma_chan *chan = prtd->params->chan;

	pr_debug("Entered %s\n", __func__);

	limit = (prtd->dma_end - prtd->dma_start) / prtd->dma_period;

	pr_debug("%s: loaded %d, limit %d\n",
				__func__, prtd->dma_loaded, limit);

	chan = prtd->params->chan;

	while (prtd->dma_loaded < limit) {
		unsigned long len = prtd->dma_period;

		pr_debug("dma_loaded: %d\n", prtd->dma_loaded);

		if ((pos + len) > prtd->dma_end) {
			len  = prtd->dma_end - pos;
			pr_debug("%s: corrected dma len %ld\n", __func__, len);
		}

		prtd->params->desc =
			dmaengine_prep_dma_cyclic(chan,
				pos, len * limit, len,
				substream->stream == SNDRV_PCM_STREAM_PLAYBACK ?
				DMA_TO_DEVICE : DMA_FROM_DEVICE);
		if (!prtd->params->desc) {
			dev_err(&chan->dev->device, "cannot prepare cyclic dma\n");
			break;
		}

		prtd->params->desc->callback = audio_buffdone;
		prtd->params->desc->callback_param = substream;
		dmaengine_submit(prtd->params->desc);

		prtd->dma_loaded++;
		pos += prtd->dma_period;
		if (pos >= prtd->dma_end)
			pos = prtd->dma_start;
	}

	prtd->dma_pos = pos;
}

static void audio_buffdone(void *data)
{
	struct snd_pcm_substream *substream = data;
	struct runtime_data *prtd;

	pr_debug("Entered %s\n", __func__);

	if (!substream || !substream->runtime)
		return;

	prtd = substream->runtime->private_data;

	if (prtd->state & ST_RUNNING) {
		prtd->dma_pos += prtd->dma_period;
		if (prtd->dma_pos >= prtd->dma_end)
			prtd->dma_pos = prtd->dma_start;

		snd_pcm_period_elapsed(substream);
	}
}

static int dma_hw_params(struct snd_pcm_substream *substream,
	struct snd_pcm_hw_params *params)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct runtime_data *prtd = runtime->private_data;
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	unsigned long totbytes = params_buffer_bytes(params);
	struct drime4_dma_params *dma =
		snd_soc_dai_get_dma_data(rtd->cpu_dai, substream);
	dma_cap_mask_t mask;
	struct dma_slave_config slave_config;

	pr_debug("Entered %s\n", __func__);

	/* return if this is a bufferless transfer e.g.
	 * codec <--> BT codec or GSM modem -- lg FIXME */
	if (!dma)
		return 0;

	/* this may get called several times by oss emulation
	 * with different params -HW */
	if (prtd->params == NULL) {
		/* prepare DMA */
		prtd->params = dma;

		pr_debug("params %p, client %p, channel %d\n", prtd->params,
			prtd->params->client, prtd->params->channel);

		dma_cap_zero(mask);
		dma_cap_set(DMA_SLAVE, mask);
		dma_cap_set(DMA_CYCLIC, mask);

		prtd->params->chan = dma_request_channel(mask,
						pl330_filter,
						(void *)prtd->params->channel);
		if (!prtd->params->chan) {
			pr_err("failed to get dma channel\n");
			return -ENODEV;
		}

		memset(&slave_config, 0, sizeof(struct dma_slave_config));

		/* channel needs configuring for mem=>device,
		* increment memory addr,
		* sync to pclk, half-word transfers to the IIS-FIFO. */
		if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
			slave_config.direction = DMA_TO_DEVICE;
			slave_config.dst_addr = prtd->params->dma_addr;
			slave_config.dst_addr_width = prtd->params->dma_size;
		} else {
			slave_config.direction = DMA_FROM_DEVICE;
			slave_config.src_addr = prtd->params->dma_addr;
			slave_config.src_addr_width = prtd->params->dma_size;
		}
		dmaengine_slave_config(prtd->params->chan, &slave_config);
	}

	snd_pcm_set_runtime_buffer(substream, &substream->dma_buffer);

	runtime->dma_bytes = totbytes;

	spin_lock_irq(&prtd->lock);
	prtd->dma_loaded = 0;
	prtd->dma_limit = runtime->hw.periods_min;
	prtd->dma_period = params_period_bytes(params);
	prtd->dma_start = runtime->dma_addr;
	prtd->dma_pos = prtd->dma_start;
	prtd->dma_end = prtd->dma_start + totbytes;
	spin_unlock_irq(&prtd->lock);

	return 0;
}

static int dma_hw_free(struct snd_pcm_substream *substream)
{
	struct runtime_data *prtd = substream->runtime->private_data;

	pr_debug("Entered %s\n", __func__);

	/* TODO - do we need to ensure DMA flushed */
	snd_pcm_set_runtime_buffer(substream, NULL);

	if (prtd->params) {
		dma_release_channel(prtd->params->chan);
		prtd->params = NULL;
	}

	return 0;
}

static int dma_prepare(struct snd_pcm_substream *substream)
{
	struct dma_chan *chan;
	struct runtime_data *prtd;
	int ret = 0;

	if (substream == NULL)
		return 0;

	prtd = substream->runtime->private_data;

	/* return if this is a bufferless transfer e.g.
	 * codec <--> BT codec or GSM modem -- lg FIXME */
	if (!prtd || !prtd->params || !prtd->params->chan)
		return 0;

	chan = prtd->params->chan;

	pr_debug("Entered %s\n", __func__);

	ret = dmaengine_terminate_all(chan);
	if (ret)
		dev_err(&chan->dev->device, "cannot flush dma channel\n");

	prtd->dma_loaded = 0;
	prtd->dma_pos = prtd->dma_start;

	/* enqueue dma buffers */
	dma_enqueue(substream);

	return ret;
}

static int dma_trigger(struct snd_pcm_substream *substream, int cmd)
{
	struct runtime_data *prtd = substream->runtime->private_data;
	int ret = 0;
	struct dma_chan *chan = prtd->params->chan;

	pr_debug("Entered %s\n", __func__);

	spin_lock(&prtd->lock);

	switch (cmd) {
	case SNDRV_PCM_TRIGGER_START:
	case SNDRV_PCM_TRIGGER_RESUME:
	case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
		prtd->state |= ST_RUNNING;
		dma_async_issue_pending(prtd->params->chan);
		break;

	case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
		prtd->state |= ST_RUNNING;
		break;

	case SNDRV_PCM_TRIGGER_STOP:
	case SNDRV_PCM_TRIGGER_SUSPEND:
		prtd->state &= ~ST_RUNNING;
		ret = dmaengine_terminate_all(chan);
		if (ret)
			dev_err(&chan->dev->device, "cannot flush dma channel\n");
		break;

	default:
		ret = -EINVAL;
		break;
	}

	spin_unlock(&prtd->lock);

	return ret;
}

static snd_pcm_uframes_t
dma_pointer(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct runtime_data *prtd = runtime->private_data;
	unsigned long res;

	pr_debug("Entered %s\n", __func__);

	res = prtd->dma_pos - prtd->dma_start;

	pr_debug("Pointer %lu\n", res);

	/* we seem to be getting the odd error from the pcm library due
	 * to out-of-bounds pointers. this is maybe due to the dma engine
	 * not having loaded the new values for the channel before being
	 * callled... (todo - fix )
	 */

	if (res >= snd_pcm_lib_buffer_bytes(substream)) {
		if (res == snd_pcm_lib_buffer_bytes(substream))
			res = 0;
	}

	return bytes_to_frames(substream->runtime, res);
}

static int dma_open(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct runtime_data *prtd;

	pr_debug("Entered %s\n", __func__);

	snd_pcm_hw_constraint_integer(runtime, SNDRV_PCM_HW_PARAM_PERIODS);
	snd_soc_set_runtime_hwparams(substream, &dma_hardware);

	prtd = kzalloc(sizeof(struct runtime_data), GFP_KERNEL);
	if (prtd == NULL)
		return -ENOMEM;

	spin_lock_init(&prtd->lock);

	runtime->private_data = prtd;
	return 0;
}

static int dma_close(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct runtime_data *prtd = runtime->private_data;

	pr_debug("Entered %s\n", __func__);

	if (!prtd) {
		pr_debug("dma_close called with prtd == NULL\n");
		return -1;
	}

	kfree(prtd);

	return 0;
}

static int dma_mmap(struct snd_pcm_substream *substream,
	struct vm_area_struct *vma)
{
	struct snd_pcm_runtime *runtime = substream->runtime;

	pr_debug("Entered %s\n", __func__);

	return dma_mmap_writecombine(substream->pcm->card->dev, vma,
				     runtime->dma_area,
				     runtime->dma_addr,
				     runtime->dma_bytes);
}

static struct snd_pcm_ops dma_ops = {
	.open		= dma_open,
	.close		= dma_close,
	.ioctl		= snd_pcm_lib_ioctl,
	.hw_params	= dma_hw_params,
	.hw_free	= dma_hw_free,
	.prepare	= dma_prepare,
	.trigger	= dma_trigger,
	.pointer	= dma_pointer,
	.mmap		= dma_mmap,
};

static int preallocate_dma_buffer(struct snd_pcm *pcm, int stream)
{
	struct snd_pcm_substream *substream = pcm->streams[stream].substream;
	struct snd_dma_buffer *buf = &substream->dma_buffer;
	size_t size = dma_hardware.buffer_bytes_max;

	pr_debug("Entered %s\n", __func__);

	buf->dev.type = SNDRV_DMA_TYPE_DEV;
	buf->dev.dev = pcm->card->dev;
	buf->private_data = NULL;
	buf->area = dma_alloc_writecombine(pcm->card->dev, size,
					   &buf->addr, GFP_KERNEL);
	if (!buf->area)
		return -ENOMEM;
	buf->bytes = size;
	return 0;
}

static void dma_free_dma_buffers(struct snd_pcm *pcm)
{
	struct snd_pcm_substream *substream;
	struct snd_dma_buffer *buf;
	int stream;

	pr_debug("Entered %s\n", __func__);

	for (stream = 0; stream < 2; stream++) {
		substream = pcm->streams[stream].substream;
		if (!substream)
			continue;

		buf = &substream->dma_buffer;
		if (!buf->area)
			continue;

		dma_free_writecombine(pcm->card->dev, buf->bytes,
				      buf->area, buf->addr);
		buf->area = NULL;
	}
}

static u64 dma_mask = DMA_BIT_MASK(32);

static int dma_new(struct snd_soc_pcm_runtime *rtd)
{
	struct snd_card *card = rtd->card->snd_card;
	struct snd_pcm *pcm = rtd->pcm;
	int ret = 0;

	pr_debug("Entered %s\n", __func__);

	if (!card->dev->dma_mask)
		card->dev->dma_mask = &dma_mask;
	if (!card->dev->coherent_dma_mask)
		card->dev->coherent_dma_mask = 0xffffffff;

	if (pcm->streams[SNDRV_PCM_STREAM_PLAYBACK].substream) {
		ret = preallocate_dma_buffer(pcm,
			SNDRV_PCM_STREAM_PLAYBACK);
		if (ret)
			goto out;
	}

	if (pcm->streams[SNDRV_PCM_STREAM_CAPTURE].substream) {
		ret = preallocate_dma_buffer(pcm,
			SNDRV_PCM_STREAM_CAPTURE);
		if (ret)
			goto out;
	}

out:
	return ret;
}

static struct snd_soc_platform_driver drime4_asoc_platform = {
	.ops		= &dma_ops,
	.pcm_new	= dma_new,
	.pcm_free	= dma_free_dma_buffers,
};

static int __devinit drime4_asoc_platform_probe(struct platform_device *pdev)
{
	return snd_soc_register_platform(&pdev->dev, &drime4_asoc_platform);
}

static int __devexit drime4_asoc_platform_remove(struct platform_device *pdev)
{
	snd_soc_unregister_platform(&pdev->dev);
	return 0;
}

static struct platform_driver asoc_dma_driver = {
	.driver = {
		.name = "drime4-audio",
		.owner = THIS_MODULE,
	},

	.probe = drime4_asoc_platform_probe,
	.remove = __devexit_p(drime4_asoc_platform_remove),
};

static int __init drime4_asoc_init(void)
{
	return platform_driver_register(&asoc_dma_driver);
}
module_init(drime4_asoc_init);

static void __exit drime4_asoc_exit(void)
{
	platform_driver_unregister(&asoc_dma_driver);
}
module_exit(drime4_asoc_exit);

MODULE_AUTHOR("Chanho Park, <chanho61.park@samsung.com>");
MODULE_DESCRIPTION("DRIME4 ASoC DMA Driver");
MODULE_LICENSE("GPL");
