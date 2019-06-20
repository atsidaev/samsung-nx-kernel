/* linux/sound/soc/drime4/d4es-hdmi.c
 *
 * Copyright (c) 2011 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com/
 *
 * ALSA SoC Audio for DRIMe4 FGPA Board
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include <sound/soc.h>
#include <sound/jack.h>
#include <asm/mach-types.h>
#include <mach/gpio.h>

#include "drime4-dma.h"
#include "drime4-i2s.h"

#define ALSA_HDMI_DEBUG

#ifdef ALSA_HDMI_DEBUG
#define APRINT(args...)		printk(args)
#else
#define APRINT(args...)
#endif

static struct snd_soc_card d4es;
static struct platform_device *d4es_snd_device;

static void __iomem *audpll_con1_reg;

static int d4es_hdmi_init(struct snd_soc_pcm_runtime *rtd)
{
	return 0;
}

static int d4es_hdmi_hifi_hw_params(struct snd_pcm_substream *substream,
		struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
/*	struct snd_soc_dai *codec_dai = rtd->codec_dai;*/
	struct snd_soc_dai *cpu_dai = rtd->cpu_dai;
	struct drime4_i2s_rate_calc div;
	int ret = 0;
	unsigned long clkrate = 0;
	unsigned int val = 0;

	/* set to AUDPLL_CON1 register */
	val = readl(audpll_con1_reg);
	val = val & (~(0x1FF<<17));	/* M[25:17] clear */

	switch (params_rate(params)) {
	case 11025:
	case 22050:
	case 44100:
		val = val | (0x78<<17);	/* M=120 : 33.8688MHz */
		__raw_writel(val, audpll_con1_reg);
		clkrate = 33868800;
		break;
	case 8000:
	case 12000:
	case 16000:
	case 24000:
	case 32000:
	case 48000:
		val = val | (0x83<<17);	/* M=131 : 36.8640MHz */
		__raw_writel(val, audpll_con1_reg);
		clkrate = 36864000;
		break;
	}

	drime4_i2s_iis_calc_rate(&div, NULL, params_rate(params), clkrate);

	/* set the cpu DAI configuration */
	ret = snd_soc_dai_set_fmt(cpu_dai, SND_SOC_DAIFMT_I2S |
			SND_SOC_DAIFMT_NB_NF | SND_SOC_DAIFMT_CBS_CFS);
	if (ret < 0) {
		APRINT("snd_soc_dai_set_fmt cpu_dai Error!\n");
		return ret;
	}

	/* set the cpu system clock */
	ret = snd_soc_dai_set_sysclk(cpu_dai, DRIME4_I2S_CLKSRC_CDCLK,
			params_rate(params), SND_SOC_CLOCK_OUT);
	if (ret < 0) {
		APRINT("snd_soc_dai_set_sysclk Error!\n");
		return ret;
	}

	/* set the cpu system clock divider */
	ret = snd_soc_dai_set_clkdiv(cpu_dai, DRIME4_I2S_DIV_RCLK, div.fs_div);
	if (ret < 0) {
		APRINT("snd_soc_dai_set_clkdiv RCLK Error!\n");
		return ret;
	}

	/* set the cpu system clock divider */
	ret = snd_soc_dai_set_clkdiv(cpu_dai, DRIME4_I2S_DIV_PRESCALER,
					div.clk_div - 1);
	if (ret < 0) {
		APRINT("snd_soc_dai_set_clkdiv PRESCALER Error!\n");
		return ret;
	}

	/* set codec DAI configuration */
/*	ret = snd_soc_dai_set_fmt(codec_dai, SND_SOC_DAIFMT_I2S |
			SND_SOC_DAIFMT_NB_NF | SND_SOC_DAIFMT_CBS_CFS);
	if (ret < 0) {
		APRINT("snd_soc_dai_set_fmt codec_dai Error!\n");
		return ret;
	}*/

	return 0;
}

static struct snd_soc_ops d4es_hifi_ops = {
	.hw_params = d4es_hdmi_hifi_hw_params,
};

static struct snd_soc_dai_link d4es_dai[] = {
{
	.name = "HDMI",
	.stream_name = "HDMI HiFi",
	.cpu_dai_name = "drime4-i2s.2",
	.codec_dai_name = "lsi-hdmi-hifi",
	.platform_name = "drime4-audio",
	.codec_name = "d4_alsa_hdmi",
	.init = d4es_hdmi_init,
	.ops = &d4es_hifi_ops,
},
};

static struct snd_soc_card d4es = {
	.name = "d4es-hdmi",
	.dai_link = d4es_dai,
	.num_links = ARRAY_SIZE(d4es_dai),
};

static int __init d4es_hdmi_sound_init(void)
{
	int ret;

	d4es_snd_device = platform_device_alloc("soc-audio", 2);
	if (!d4es_snd_device)
		return -ENOMEM;

	platform_set_drvdata(d4es_snd_device, &d4es);
	ret = platform_device_add(d4es_snd_device);

	if (ret) {
		snd_soc_unregister_dai(&d4es_snd_device->dev);
		platform_device_put(d4es_snd_device);
	}

	audpll_con1_reg = ioremap(0x30120058, 0x4);

	return ret;
}

static void __exit d4es_hdmi_sound_exit(void)
{
	iounmap(audpll_con1_reg);

	snd_soc_unregister_dai(&d4es_snd_device->dev);
	platform_device_unregister(d4es_snd_device);
}

module_init(d4es_hdmi_sound_init);
module_exit(d4es_hdmi_sound_exit);

/* Module information */
MODULE_DESCRIPTION("ALSA SoC HDMI D4ES Board(DRIMe4)");
MODULE_AUTHOR("Byungho Ahn");
MODULE_LICENSE("GPL");

