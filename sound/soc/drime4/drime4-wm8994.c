/* linux/sound/soc/drime4/drime4_wm8994.c
 *
 * Copyright (c) 2011 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com/
 *
 * ALSA SoC Audio for DRIMe4
 *
 * Modified by Han Oh <han.oh@samsung.com>
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
#include "../codecs/wm8994.h"

static struct snd_soc_card d4;
static struct platform_device *d4_snd_device;
static struct drime4_i2s_rate_calc div;


static void __iomem *audpll_con1_reg;
static void __iomem *audpll_con2_reg;


static const struct snd_soc_dapm_widget d4_dapm_widgets[] = {
    SND_SOC_DAPM_SPK("Ext Spk", NULL),
    SND_SOC_DAPM_SPK("Ext Spk1", NULL),
    SND_SOC_DAPM_MIC("Stereo Mic1", NULL),
    SND_SOC_DAPM_MIC("Stereo Mic2", NULL),
}; 

static const struct snd_soc_dapm_route d4_dapm_routes[] = {
    {"Ext Spk", NULL, "SPKOUTLP"},
    {"Ext Spk", NULL, "SPKOUTLN"},
    {"Ext Spk1", NULL, "SPKOUTRP"},
    {"Ext Spk1", NULL, "SPKOUTRN"},
    {"IN1LP", NULL, "MICBIAS1"},
	{"IN1LN", NULL, "MICBIAS1"},
	{"IN1RP", NULL, "MICBIAS2"},
	{"IN1RN", NULL, "MICBIAS2"},
	{"MICBIAS1", NULL, "Stereo Mic1"},
	{"MICBIAS2", NULL, "Stereo Mic2"},
};
static int d4_wm8994_init(struct snd_soc_pcm_runtime *rtd)
{
	struct snd_soc_codec *codec = rtd->codec;
	struct snd_soc_dapm_context *dapm = &codec->dapm;

	snd_soc_dapm_new_controls(dapm, d4_dapm_widgets, ARRAY_SIZE(d4_dapm_widgets));
	snd_soc_dapm_add_routes(dapm, d4_dapm_routes, ARRAY_SIZE(d4_dapm_routes));

	snd_soc_dapm_nc_pin(dapm, "IN2LP:VXRN");
	snd_soc_dapm_nc_pin(dapm, "IN2RP:VXRP");
	snd_soc_dapm_nc_pin(dapm, "LINEOUT1N");
	snd_soc_dapm_nc_pin(dapm, "LINEOUT1P");
	snd_soc_dapm_nc_pin(dapm, "LINEOUT2N");
	snd_soc_dapm_nc_pin(dapm, "LINEOUT2P");


	snd_soc_dapm_sync(dapm);
	return 0;
}


static int d4_wm8994_hifi_hw_params(struct snd_pcm_substream *substream,
		struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *codec_dai = rtd->codec_dai;
	struct snd_soc_dai *cpu_dai = rtd->cpu_dai;

	unsigned long clkrate = 0;
	unsigned int val = 0;
	unsigned int val2 = 0;
	int ret = 0;


	unsigned int pll_out = 0;

	/* set to AUDPLL_CON1 register */
	val = readl(audpll_con1_reg);
	val = val & (~(0x1FF<<17));	/* M[25:17] clear */

	val2 = readl(audpll_con2_reg);
	val2 = val2 & (~(0xFFFF<<16));	/* M[31:16] clear */

	

	switch (params_rate(params)) {
	case 11025:
	case 22050:
	case 44100:
	case 88200:
		val = val | (120<<17);	/* M=120 : 33.8688MHz */
		__raw_writel(val, audpll_con1_reg);
		val2 = val2 | (27682<<16);	/* K=27682 */
		__raw_writel(val2, audpll_con2_reg);
		clkrate = 33868800;
		break;
	case 8000:
	case 12000:
	case 16000:
	case 24000:
	case 32000:
	case 48000:
		val = val | (87<<17);	/* M=87 : 24.5760 MHz */
		__raw_writel(val, audpll_con1_reg);
		val2 = val2 | (24991<<16);	/* K=24991 */
		__raw_writel(val2, audpll_con2_reg);
		clkrate = 24576000;
		break;
	case 96000:
		val = val | (131<<17);	/* M=131 : 36.8640 MHz */
		__raw_writel(val, audpll_con1_reg);
		val2 = val2 | (4719<<16);	/* K=4719 */
		__raw_writel(val2, audpll_con2_reg);
		clkrate = 36864000;
		break;
	}

	/* drime4 side setting */
	drime4_i2s_iis_calc_rate(&div, NULL, params_rate(params), clkrate);

	/* set the cpu DAI configuration */
	ret = snd_soc_dai_set_fmt(cpu_dai, SND_SOC_DAIFMT_I2S |
			SND_SOC_DAIFMT_NB_NF | SND_SOC_DAIFMT_CBS_CFS); 

	if (ret < 0)
		return ret;

	/* set codec DAI configuration */
	ret = snd_soc_dai_set_fmt(codec_dai, SND_SOC_DAIFMT_I2S |
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

	pll_out = div.fs_div*params_rate(params);
	
	ret = snd_soc_dai_set_pll(codec_dai, WM8994_FLL1, WM8994_FLL_SRC_MCLK1,
					clkrate, pll_out);

	if (ret < 0)
		return ret;

	ret = snd_soc_dai_set_sysclk(codec_dai, WM8994_SYSCLK_MCLK1,
		pll_out, SND_SOC_CLOCK_IN);

	if (ret < 0) {
		printk("AIF1: snd_soc_dai_set_sysclk failed.\n");
		return ret;
	}

	
	return 0;
}

static int d4_wm8994_hifi2_hw_params(struct snd_pcm_substream *substream, struct snd_pcm_hw_params *params)
{
		struct snd_soc_pcm_runtime *rtd = substream->private_data;
		struct snd_soc_dai *codec_dai = rtd->codec_dai;
		unsigned int pll_out = 0;
		int ret = 0;

		/* set codec DAI configuration */
		ret = snd_soc_dai_set_fmt(codec_dai, SND_SOC_DAIFMT_I2S
			|SND_SOC_DAIFMT_NB_NF
			|SND_SOC_DAIFMT_CBM_CFM);
		
		if (ret < 0)
			return ret;
		
		pll_out = div.fs_div*params_rate(params);

		ret = snd_soc_dai_set_sysclk(codec_dai, WM8994_SYSCLK_MCLK1,
					pll_out, SND_SOC_CLOCK_IN);

		if (ret < 0) {
			printk("AIF2: snd_soc_dai_set_sysclk failed.\n");
			return ret;
		}


		return 0;
}

static int d4_wm8994_hifi3_hw_params(struct snd_pcm_substream *substream, struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *codec_dai = rtd->codec_dai;
	struct snd_soc_codec *codec = codec_dai->codec;


		
	/* GPIO settings for external mic */
	snd_soc_write(codec, 0x700, 0x8101);
	snd_soc_write(codec, 0x702, 0xc100);
	snd_soc_write(codec, 0x703, 0xc100);
	snd_soc_write(codec, 0x704, 0x8100);
	snd_soc_write(codec, 0x706, 0xa101);
	snd_soc_write(codec, 0x707, 0xc300);
	snd_soc_write(codec, 0x708, 0xc100);
	snd_soc_write(codec, 0x709, 0xc300);
	snd_soc_write(codec, 0x70a, 0xa300);


	
	return 0;
}


static struct snd_soc_dai_driver wm8994_aif2_dai[] = {
{
		.name = "drime4-i2s-dummy",
		.id = 0,
		.playback = {
			.channels_min = 1,
			.channels_max = 2,
			.rates = SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_44100 | SNDRV_PCM_RATE_48000,
			.formats = SNDRV_PCM_FMTBIT_S16_LE,
		},
		.capture = {
			.channels_min = 1,
			.channels_max = 2,
			.rates = SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_44100 | SNDRV_PCM_RATE_48000,
			.formats = SNDRV_PCM_FMTBIT_S16_LE,
		},

}
	
};

static struct snd_soc_dai_driver wm8994_aif3_dai[] = {
	{
		.name = "drime4-i2s-dummy",
			.id = 0,
			.playback = {
				.channels_min = 1,
				.channels_max = 2,
				.rates = SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_44100 | SNDRV_PCM_RATE_48000,
				.formats = SNDRV_PCM_FMTBIT_S16_LE,
		},
		.capture = {
			.channels_min = 1,
			.channels_max = 2,
			.rates = SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_44100 | SNDRV_PCM_RATE_48000,
			.formats = SNDRV_PCM_FMTBIT_S16_LE,
		},

	}

};


static struct snd_soc_ops d4_hifi_ops = {
	.hw_params = d4_wm8994_hifi_hw_params,
};

static struct snd_soc_ops d4_hifi2_ops = {
	.hw_params = d4_wm8994_hifi2_hw_params,
};

static struct snd_soc_ops d4_hifi3_ops = {
	.hw_params = d4_wm8994_hifi3_hw_params,
};

static struct snd_soc_dai_link d4_dai[] = {
	{
		.name = "WM8994",
		.stream_name = "WM8994 HiFi Pri",
		.cpu_dai_name = "drime4-i2s.0",
		.codec_dai_name = "wm8994-aif1",
		.platform_name = "drime4-audio",
		.codec_name = "wm8994-codec",
		.init = d4_wm8994_init,
		.ops = &d4_hifi_ops,
	},
	{
		.name = "WM8994 AIF2",
		.stream_name = "WM8994 HiFi Sec",
		.cpu_dai_name = "drime4-i2s-dummy",
		.codec_dai_name = "wm8994-aif2",
		.platform_name = "drime4-audio",
		.codec_name = "wm8994-codec",
		.ops = &d4_hifi2_ops,
	},
	{
		.name = "WM8994 AIF3",
		.stream_name = "WM8994 HiFi Ext",
		.cpu_dai_name = "drime4-i2s-dummy",
		.codec_dai_name = "wm8994-aif3",
		.platform_name = "drime4-audio",
		.codec_name = "wm8994-codec",
		.ops = &d4_hifi3_ops,
	},
};

static struct snd_soc_card d4 = {
	.name = "wm8994",
	.dai_link = d4_dai,
	.num_links = ARRAY_SIZE(d4_dai),
};

static int __init d4_wm8994_sound_init(void)
{
	int ret;

	d4_snd_device = platform_device_alloc("soc-audio", -1);

	if (!d4_snd_device)
		return -ENOMEM;

	/* register aif2 DAI here */
	ret = snd_soc_register_dais(&d4_snd_device->dev, wm8994_aif2_dai, ARRAY_SIZE(wm8994_aif2_dai));
	if (ret) {
		platform_device_put(d4_snd_device);
		return ret;
	}

	/* register aif3 DAI here */
	ret = snd_soc_register_dais(&d4_snd_device->dev, wm8994_aif3_dai, ARRAY_SIZE(wm8994_aif3_dai));
	if (ret) {
		platform_device_put(d4_snd_device);
		return ret;
	}
	
	platform_set_drvdata(d4_snd_device, &d4);
	ret = platform_device_add(d4_snd_device);
	if (ret) {
		snd_soc_unregister_dai(&d4_snd_device->dev);
		platform_device_put(d4_snd_device);
	}
	
	audpll_con1_reg = ioremap(0x30120058, 0x4);
	audpll_con2_reg = ioremap(0x3012005c, 0x4);

	return ret;
}

static void __exit d4_wm8994_sound_exit(void)
{
	iounmap(audpll_con1_reg);

	snd_soc_unregister_dai(&d4_snd_device->dev);
	platform_device_unregister(d4_snd_device);
}

module_init(d4_wm8994_sound_init);
module_exit(d4_wm8994_sound_exit);

/* Module information */
MODULE_DESCRIPTION("ALSA SoC WM8994 DRIMe 4");
MODULE_AUTHOR("Han Oh");
MODULE_LICENSE("GPL");
