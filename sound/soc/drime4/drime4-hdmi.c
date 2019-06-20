/**
 * @file d4_alsa_hdmi.c
 * @brief DRIMe4 HDMI(alsa) Platform Driver
 * @author Byungho Ahn <bh1212.ahn@samsung.com>
 * 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/version.h>
#include <linux/i2c.h>
#include <linux/irq.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/miscdevice.h>
#include <linux/io.h>


#include <sound/soc.h>
#include <sound/soc-dapm.h>
#include <sound/initval.h>

#include <mach/map.h>
#include <mach/alsa_hdmi/d4_alsa_hdmi.h>
#include "drime4-hdmi.h"

#define HDMI_DEBUG

#ifdef HDMI_DEBUG
#define DPRINTK(args...)	printk(args)
#else
#define DPRINTK(args...)
#endif

#if 0
void __iomem *core_regs;
#endif

void __iomem *i2s_regs;

static void __iomem *audpll_con2_reg;



int hdmi_set_audio_mute(int mute);
int hdmi_set_audio_sample_freq(enum SamplingFreq freq);
	


static int d4_hdmi_dac_mute(struct snd_soc_dai *dai, int mute)
{
	if (mute) {	/* audio mute */
#if 0
		hdmi_set_audio_mute(mute);

		writeb(val &I2S_CLK_CON_DISABLE, i2s_regs + 0x00);
#endif
	} 
	else {
		hdmi_set_audio_mute(mute);
		
		writeb(I2S_CLK_CON_ENABLE, i2s_regs + 0x00);
	}

	return 0;
}


#if 0

/**
 * N value of ACR packet.@n
 * 4096  is the N value for 32 KHz sampling frequency @n
 * 6272  is the N value for 44.1 KHz sampling frequency @n
 * 6144  is the N value for 48 KHz sampling frequency @n
 * 12544 is the N value for 88.2 KHz sampling frequency @n
 * 12288 is the N value for 96 KHz sampling frequency @n
 * 25088 is the N value for 176.4 KHz sampling frequency @n
 * 24576 is the N value for 192 KHz sampling frequency @n
 */
static const unsigned int ACR_N_params[] = { 4096, 6272, 6144, 12544, 12288,
		25088, 24576 };

static int hdmi_set_audio_sample_freq(enum SamplingFreq freq)
{
	unsigned char reg;
	unsigned int n;
	int ret = 1;


	printk("[Enter] hdmi_set_audio_sample_freq:%d\n", freq);

	if (freq > sizeof(ACR_N_params) / sizeof(unsigned int) || freq < 0)
		return 0;

	n = ACR_N_params[freq];
	reg = n & 0xff;
	writeb(0, core_regs + 0x430);	/* HDMI_ACR_N0 */
	writeb(reg, core_regs + 0x430);	/* HDMI_ACR_N0 */

	reg = (n >> 8) & 0xff;
	writeb(reg, core_regs + 0x434);	/* HDMI_ACR_N1 */

	reg = (n >> 16) & 0xff;
	writeb(reg, core_regs + 0x438);	/* HDMI_ACR_N2 */

	/* set as measure cts modei */
	writeb(0x00, core_regs + 0x400);	/* HDMI_ACR_CON */

	/* set AUI packet */
	reg = readb(core_regs + 0x824)
		& ~HDMI_AUI_SF_MASK;	/* HDMI_AUI_BYTE2 */

	switch (freq) {
	case SF_32KHZ:
		reg |= HDMI_AUI_SF_SF_32KHZ;
		break;

	case SF_44KHZ:
		reg |= HDMI_AUI_SF_SF_44KHZ;
		break;

	case SF_88KHZ:
		reg |= HDMI_AUI_SF_SF_88KHZ;
		break;

	case SF_176KHZ:
		reg |= HDMI_AUI_SF_SF_176KHZ;
		break;

	case SF_48KHZ:
		reg |= HDMI_AUI_SF_SF_48KHZ;
		break;

	case SF_96KHZ:
		reg |= HDMI_AUI_SF_SF_96KHZ;
		break;

	case SF_192KHZ:
		reg |= HDMI_AUI_SF_SF_192KHZ;
		break;

	default:
		ret = 0;
		break;
	}

	writeb(0x00, core_regs + 0x824);	/* HDMI_AUI_BYTE2 */


	printk("[Leave] hdmi_set_audio_sample_freq\n");

	return ret;
}

#endif

static int setCUVSampleFreq(enum SamplingFreq freq)
{
	int ret = 1;
	unsigned char reg = readb(i2s_regs + 0x34)
		& ~I2S_CH_ST_3_SF_MASK;	/* HDMI_SS_I2S_CH_ST_3 */


	switch (freq) {
	case SF_32KHZ:
		reg |= I2S_CH_ST_3_SF_32KHZ;
		break;
	case SF_44KHZ:
		reg |= I2S_CH_ST_3_SF_44KHZ;
		break;
	case SF_88KHZ:
		reg |= I2S_CH_ST_3_SF_88KHZ;
		break;
	case SF_176KHZ:
		reg |= I2S_CH_ST_3_SF_176KHZ;
		break;
	case SF_48KHZ:
		reg |= I2S_CH_ST_3_SF_48KHZ;
		break;
	case SF_96KHZ:
		reg |= I2S_CH_ST_3_SF_96KHZ;
		break;
	case SF_192KHZ:
		reg |= I2S_CH_ST_3_SF_192KHZ;
		break;
	case SF_768KHZ:
		reg |= I2S_CH_ST_3_SF_768KHZ;
		break;
	default:
		ret = 0;
	}

	writeb(reg, i2s_regs + 0x34);	/* HDMI_SS_I2S_CH_ST_3 */
	
	writeb(0x1, i2s_regs + 0x24);

	return ret;
}


static int d4_hdmi_hw_params(struct snd_pcm_substream *substream,
			struct snd_pcm_hw_params *params,
			struct snd_soc_dai *dai)
{
	unsigned int val2 = 0;
	printk("[Enter] d4_hdmi_hw_params\n");

	/* set to AUDPLL_CON2 register */

	val2 = readl(audpll_con2_reg);
	val2 = val2 & (~(0xFFFF<<16));	/* M[31:16] clear */

	/* i2s parameter set */
	writeb(I2S_32FS|I2S_CON_DATA_NUM_16|I2S_CON_I2S_MODE_BASIC,
		i2s_regs + 0x08);

	/* audio input port set - i2s */
	writeb(I2S_IN_MUX_ENABLE | I2S_IN_MUX_CUV_ENABLE | I2S_IN_MUX_SELECT_I2S
		| I2S_IN_MUX_IN_ENABLE, i2s_regs + 0x20);
	
	/* sampling rate set */
	switch (params_rate(params)) {
	case 32000:
		setCUVSampleFreq(SF_32KHZ);
		hdmi_set_audio_sample_freq(SF_32KHZ);
		val2 = val2 | (4719<<16);	/* K=4719 */
		__raw_writel(val2, audpll_con2_reg);

		break;
	case 44100:
		setCUVSampleFreq(SF_44KHZ);
		hdmi_set_audio_sample_freq(SF_44KHZ);
		val2 = val2 | (27682<<16);	/* K=27682 */
		__raw_writel(val2, audpll_con2_reg);

		break;
	case 48000:
		setCUVSampleFreq(SF_48KHZ);
		hdmi_set_audio_sample_freq(SF_48KHZ);
		val2 = val2 | (4719<<16);	/* K=4719 */
		__raw_writel(val2, audpll_con2_reg);

		break;
	}

	printk("[Leave] d4_hdmi_hw_params\n");
	return 0;
}

static struct snd_soc_dai_ops d4_hdmi_dai_ops = {
	.digital_mute = d4_hdmi_dac_mute,
	.hw_params = d4_hdmi_hw_params,
};

static struct snd_soc_dai_driver d4es_hdmi_dai = {
	.name = "lsi-hdmi-hifi",
	.playback = {
		.stream_name  = "Playback",
		.channels_min = 2,
		.channels_max = 8,
		.rates = SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_44100  |
			 SNDRV_PCM_RATE_48000 | SNDRV_PCM_RATE_88200  |
			 SNDRV_PCM_RATE_96000 | SNDRV_PCM_RATE_176400 |
			 SNDRV_PCM_RATE_192000,
		.formats = SNDRV_PCM_FMTBIT_S16_LE | SNDRV_PCM_FMTBIT_S24_LE,
	},
	.ops = &d4_hdmi_dai_ops,
};


static int d4es_hdmi_snd_probe(struct snd_soc_codec *codec)
{
	dev_info(codec->dev, "DRIMe4 ES HDMI Audio");

	printk("[Enter] d4es_hdmi_snd_probe\n");

	audpll_con2_reg = ioremap(0x3012005c, 0x4);

	return 0;
}

static struct snd_soc_codec_driver soc_codec_dev_d4es_hdmi = {
	.probe	= d4es_hdmi_snd_probe,
/*	.remove = drime4_alsa_hdmi_remove,
	.read	= d4es_hdmi_snd_read,
	.write	= d4es_hdmi_snd_write,*/
};


static int __devinit drime4_alsa_hdmi_probe(struct platform_device *pdev)
{
	int ret = 0;

	printk("[Enter] drime4_alsa_hdmi_probe\n");

	/* ioremap */

#if 0
	core_regs = ioremap(0x50110000, 0x1000);
#endif

	i2s_regs  = ioremap(DRIME4_PA_HDMI+0x00040000, 0x1000);

	ret = snd_soc_register_codec(&pdev->dev,
		&soc_codec_dev_d4es_hdmi, &d4es_hdmi_dai, 1);

	if (ret < 0) {
		printk("HDMI SOUND INIT ERROR!!!!\n");
		dev_err(&pdev->dev, "hdmi sound codec registration failed\n");
		goto out;
	}

	printk("[Leave] drime4_alsa_hdmi_probe\n");

	return 0;

out:
	return ret;
}


static int drime4_alsa_hdmi_remove(struct platform_device *pdev)
{

	printk("[Enter] drime4_alsa_hdmi_remove\n");

	iounmap(i2s_regs);

	printk("[Leave] drime4_alsa_hdmi_remove\n");
	return 0;
}


static struct platform_driver drime4_alsa_hdmi_driver = {
	.probe  = drime4_alsa_hdmi_probe,
	.remove = drime4_alsa_hdmi_remove,
	.driver = {
		.name  = ALSA_HDMI_MODULE_NAME,
		.owner = THIS_MODULE,
	},
};


static int drime4_alsa_hdmi_register(void)
{
	printk("[Enter] drime4_alsa_hdmi_register\n");

	platform_driver_register(&drime4_alsa_hdmi_driver);

	printk("[Leave] drime4_alsa_hdmi_register\n");
	return 0;
}


static void drime4_alsa_hdmi_unregister(void)
{
	printk("[Enter] drime4_alsa_hdmi_unregister\n");
	
	platform_driver_unregister(&drime4_alsa_hdmi_driver);

	printk("[Leave] drime4_alsa_hdmi_unregister\n");
}


module_init(drime4_alsa_hdmi_register);
module_exit(drime4_alsa_hdmi_unregister);


MODULE_AUTHOR("Byungho Ahn <bh1212.ahn@samung.com>");
MODULE_DESCRIPTION("Samsung DRIMe4 ALSA HDMI driver");
MODULE_LICENSE("GPL");


