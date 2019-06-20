/* linux/sound/soc/drime4/d4fpga-pcm3793a.c
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

static struct snd_soc_card d4fpga;
static struct platform_device *d4fpga_snd_device;

static int d4fpga_pcm3793a_init(struct snd_soc_pcm_runtime *rtd)
{
	return 0;
}

static int d4fpga_pcm3793a_hifi_hw_params(struct snd_pcm_substream *substream,
		struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *codec_dai = rtd->codec_dai;
	struct snd_soc_dai *cpu_dai = rtd->cpu_dai;
	struct drime4_i2s_rate_calc div;
	int ret = 0;
	unsigned long clkrate = 0;

	switch (params_rate(params)) {
	case 8000:
	case 11025:
	case 12000:
	case 16000:
	case 22050:
	case 32000:
	case 44100:
	case 64000:
		clkrate = 33868800;
		break;
	case 24000:
	case 48000:
	case 96000:
		clkrate = 36864000;
		break;
	}

	drime4_i2s_iis_calc_rate(&div, NULL, params_rate(params), clkrate);

	/* set the cpu DAI configuration */
	ret = snd_soc_dai_set_fmt(cpu_dai, SND_SOC_DAIFMT_I2S |
			SND_SOC_DAIFMT_NB_NF | SND_SOC_DAIFMT_CBS_CFS);
	if (ret < 0)
		return ret;

	/* set the cpu system clock */
	ret = snd_soc_dai_set_sysclk(cpu_dai, DRIME4_I2S_CLKSRC_CDCLK,
			params_rate(params), SND_SOC_CLOCK_OUT);
	if (ret < 0)
		return ret;

	/* set the cpu system clock divider */
	ret = snd_soc_dai_set_clkdiv(cpu_dai, DRIME4_I2S_DIV_RCLK, div.fs_div);
	if (ret < 0)
		return ret;

	/* set the cpu system clock divider */
	ret = snd_soc_dai_set_clkdiv(cpu_dai, DRIME4_I2S_DIV_PRESCALER,
					div.clk_div - 1);
	if (ret < 0)
		return ret;

	/* set codec DAI configuration */
	ret = snd_soc_dai_set_fmt(codec_dai, SND_SOC_DAIFMT_I2S |
			SND_SOC_DAIFMT_NB_NF | SND_SOC_DAIFMT_CBS_CFS);
	if (ret < 0)
		return ret;

	return 0;
}

static struct snd_soc_ops d4fpga_hifi_ops = {
	.hw_params = d4fpga_pcm3793a_hifi_hw_params,
};

static struct snd_soc_dai_link d4fpga_dai[] = {
{
	.name = "PCM3793A",
	.stream_name = "PCM3793A HiFi",
	.cpu_dai_name = "drime4-i2s.0",
	.codec_dai_name = "pcm3793a-hifi",
	.platform_name = "drime4-audio",
	.codec_name = "pcm3793a-codec.1-0047",
	.init = d4fpga_pcm3793a_init,
	.ops = &d4fpga_hifi_ops,
},
};

static struct snd_soc_card d4fpga = {
	.name = "d4fpga",
	.dai_link = d4fpga_dai,
	.num_links = ARRAY_SIZE(d4fpga_dai),
};

static int __init d4fpga_init(void)
{
	int ret;

	d4fpga_snd_device = platform_device_alloc("soc-audio", -1);
	if (!d4fpga_snd_device)
		return -ENOMEM;

	platform_set_drvdata(d4fpga_snd_device, &d4fpga);
	ret = platform_device_add(d4fpga_snd_device);

	if (ret) {
		snd_soc_unregister_dai(&d4fpga_snd_device->dev);
		platform_device_put(d4fpga_snd_device);
	}

	return ret;
}

static void __exit d4fpga_exit(void)
{
	snd_soc_unregister_dai(&d4fpga_snd_device->dev);
	platform_device_unregister(d4fpga_snd_device);
}

module_init(d4fpga_init);
module_exit(d4fpga_exit);

/* Module information */
MODULE_DESCRIPTION("ALSA SoC PCM3793 D4FPGA Board(DRIMe4)");
MODULE_AUTHOR("Chanho Park");
MODULE_LICENSE("GPL");
