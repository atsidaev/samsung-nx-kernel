/* linux/sound/soc/drime4/drime4-i2s.c
 *
 * Copyright (c) 2011 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com/
 *
 * ALSA SoC Audio Layer - DRIME4 I2S core
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/delay.h>
#include <linux/clk.h>
#include <linux/io.h>

#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>

#include <mach/dma.h>
#include <mach/audio.h>

#include <linux/dmaengine.h>

#include "regs-i2s.h"
#include "drime4-i2s.h"
#include "drime4-dma.h"

#define DRIME4_I2S_DEBUG_CON 0

/* warning for null pointer dereferences */
/* Added by Han Oh <han.oh@samsung.com> on 9/11/2012 */
#define NULL_TEST(ptr, returncode) \
	if ((ptr) == NULL) { \
		printk("[drime4-i2s.c] Null pointer dereference-=-=-=-=-=-=-=-=-=-=\n"); \
		return (returncode); }

#define NULL_TEST_VOID(ptr) \
	if ((ptr) == NULL) { \
		printk("[drime4-i2s.c] Null pointer dereference-=-=-=-=-=-=-=-=-=-=.\n"); \
		return; }	
static inline struct drime4_i2s_info *to_info(struct snd_soc_dai *cpu_dai)
{
	return snd_soc_dai_get_drvdata(cpu_dai);
}

#define bit_set(v, b) (((v) & (b)) ? 1 : 0)

#if DRIME4_I2S_DEBUG_CON
static void dbg_showcon(const char *fn, u32 con)
{
	pr_debug("%s: LRI=%d, TXFEMPT=%d, RXFEMPT=%d, TXFFULL=%d, RXFFULL=%d\n",
	       fn,
	       bit_set(con, DRIME4_I2SCON_LRINDEX),
	       bit_set(con, DRIME4_I2SCON_TXFIFO_EMPTY),
	       bit_set(con, DRIME4_I2SCON_RXFIFO_EMPTY),
	       bit_set(con, DRIME4_I2SCON_TXFIFO_FULL),
	       bit_set(con, DRIME4_I2SCON_RXFIFO_FULL));

	pr_debug("%s: PAUSE: TXDMA=%d, RXDMA=%d, TXCH=%d, RXCH=%d\n",
	       fn,
	       bit_set(con, DRIME4_I2SCON_TXDMA_PAUSE),
	       bit_set(con, DRIME4_I2SCON_RXDMA_PAUSE),
	       bit_set(con, DRIME4_I2SCON_TXCH_PAUSE),
	       bit_set(con, DRIME4_I2SCON_RXCH_PAUSE));
	pr_debug("%s: ACTIVE: TXDMA=%d, RXDMA=%d, IIS=%d\n", fn,
	       bit_set(con, DRIME4_I2SCON_TXDMA_ACTIVE),
	       bit_set(con, DRIME4_I2SCON_RXDMA_ACTIVE),
	       bit_set(con, DRIME4_I2SCON_I2S_ACTIVE));
}
#else
static inline void dbg_showcon(const char *fn, u32 con)
{
}
#endif

static struct drime4_dma_client drime4_dma_client_out = {
	.name		= "I2S PCM Stereo out"
};

static struct drime4_dma_client drime4_dma_client_in = {
	.name		= "I2S PCM Stereo in"
};


static struct drime4_i2s_info drime4_i2s[3];
static struct drime4_dma_params drime4_i2s_pcm_stereo_out[3];
static struct drime4_dma_params drime4_i2s_pcm_stereo_in[3];

/* Turn on or off the transmission path. */
static void drime4_snd_txctrl(struct drime4_i2s_info *i2s, int on)
{
	void __iomem *regs;
	u32 fic, con, mod;

	pr_debug("%s(%d)\n", __func__, on);
	NULL_TEST_VOID(i2s); 
	regs = i2s->regs;
	fic = readl(regs + DRIME4_I2SFIC);
	con = readl(regs + DRIME4_I2SCON);
	mod = readl(regs + DRIME4_I2SMOD);

	pr_debug("%s: IIS: CON=%x MOD=%x FIC=%x\n", __func__, con, mod, fic);

	if (on) {
		con |= DRIME4_I2SCON_TXDMA_ACTIVE | DRIME4_I2SCON_I2S_ACTIVE;
		con &= ~DRIME4_I2SCON_TXDMA_PAUSE;
		con &= ~DRIME4_I2SCON_TXCH_PAUSE;

		switch (mod & DRIME4_I2SMOD_MODE_MASK) {
		case DRIME4_I2SMOD_MODE_TXONLY:
		case DRIME4_I2SMOD_MODE_TXRX:
			/* do nothing, we are in the right mode */
			break;

		case DRIME4_I2SMOD_MODE_RXONLY:
			mod &= ~DRIME4_I2SMOD_MODE_MASK;
			mod |= DRIME4_I2SMOD_MODE_TXRX;
			break;

		default:
			dev_err(i2s->dev, "TXEN: Invalid MODE %x in IISMOD\n",
				mod & DRIME4_I2SMOD_MODE_MASK);
			break;
		}

		writel(con, regs + DRIME4_I2SCON);
		writel(mod, regs + DRIME4_I2SMOD);
	} else {
		/* Note, we do not have any indication that the FIFO problems
		 * tha the S3C2410/2440 had apply here, so we should be able
		 * to disable the DMA and TX without resetting the FIFOS.
		 */

		con |=  DRIME4_I2SCON_TXDMA_PAUSE;
		con |=  DRIME4_I2SCON_TXCH_PAUSE;
		con &= ~DRIME4_I2SCON_TXDMA_ACTIVE;

		switch (mod & DRIME4_I2SMOD_MODE_MASK) {
		case DRIME4_I2SMOD_MODE_TXRX:
			mod &= ~DRIME4_I2SMOD_MODE_MASK;
			mod |= DRIME4_I2SMOD_MODE_RXONLY;
			break;

		case DRIME4_I2SMOD_MODE_TXONLY:
			mod &= ~DRIME4_I2SMOD_MODE_MASK;
			con &= ~DRIME4_I2SCON_I2S_ACTIVE;
			break;

		default:
			dev_err(i2s->dev, "TXDIS: Invalid MODE %x in IISMOD\n",
				mod & DRIME4_I2SMOD_MODE_MASK);
			break;
		}

		writel(mod, regs + DRIME4_I2SMOD);
		writel(con, regs + DRIME4_I2SCON);
	}

	fic = readl(regs + DRIME4_I2SFIC);
	dbg_showcon(__func__, con);
	pr_debug("%s: IIS: CON=%x MOD=%x FIC=%x\n", __func__, con, mod, fic);
}

static void drime4_snd_rxctrl(struct drime4_i2s_info *i2s, int on)
{
	void __iomem *regs;
	u32 fic, con, mod;

	pr_debug("%s(%d)\n", __func__, on);

	NULL_TEST_VOID(i2s); 
	regs = i2s->regs;
	fic = readl(regs + DRIME4_I2SFIC);
	con = readl(regs + DRIME4_I2SCON);
	mod = readl(regs + DRIME4_I2SMOD);

	pr_debug("%s: IIS: CON=%x MOD=%x FIC=%x\n", __func__, con, mod, fic);

	if (on) {
		con |= DRIME4_I2SCON_RXDMA_ACTIVE | DRIME4_I2SCON_I2S_ACTIVE;
		con &= ~DRIME4_I2SCON_RXDMA_PAUSE;
		con &= ~DRIME4_I2SCON_RXCH_PAUSE;

		switch (mod & DRIME4_I2SMOD_MODE_MASK) {
		case DRIME4_I2SMOD_MODE_TXRX:
		case DRIME4_I2SMOD_MODE_RXONLY:
			/* do nothing, we are in the right mode */
			break;

		case DRIME4_I2SMOD_MODE_TXONLY:
			mod &= ~DRIME4_I2SMOD_MODE_MASK;
			mod |= DRIME4_I2SMOD_MODE_TXRX;
			break;

		default:
			dev_err(i2s->dev, "RXEN: Invalid MODE %x in IISMOD\n",
				mod & DRIME4_I2SMOD_MODE_MASK);
		}

		writel(mod, regs + DRIME4_I2SMOD);
		writel(con, regs + DRIME4_I2SCON);
	} else {
		/* See txctrl notes on FIFOs. */

		con &= ~DRIME4_I2SCON_RXDMA_ACTIVE;
		con |=  DRIME4_I2SCON_RXDMA_PAUSE;
		con |=  DRIME4_I2SCON_RXCH_PAUSE;

		switch (mod & DRIME4_I2SMOD_MODE_MASK) {
		case DRIME4_I2SMOD_MODE_RXONLY:
			con &= ~DRIME4_I2SCON_I2S_ACTIVE;
			mod &= ~DRIME4_I2SMOD_MODE_MASK;
			break;

		case DRIME4_I2SMOD_MODE_TXRX:
			mod &= ~DRIME4_I2SMOD_MODE_MASK;
			mod |= DRIME4_I2SMOD_MODE_TXONLY;
			break;

		default:
			dev_err(i2s->dev, "RXDIS: Invalid MODE %x in IISMOD\n",
				mod & DRIME4_I2SMOD_MODE_MASK);
		}

		writel(con, regs + DRIME4_I2SCON);
		writel(mod, regs + DRIME4_I2SMOD);
	}

	fic = readl(regs + DRIME4_I2SFIC);
	pr_debug("%s: IIS: CON=%x MOD=%x FIC=%x\n", __func__, con, mod, fic);
}

#define msecs_to_loops(t) (loops_per_jiffy / 1000 * HZ * t)

/*
 * Wait for the LR signal to allow synchronisation to the L/R clock
 * from the codec. May only be needed for slave mode.
 */
static int drime4_snd_lrsync(struct drime4_i2s_info *i2s)
{
	u32 iiscon;
	unsigned long loops = msecs_to_loops(5);

	pr_debug("Entered %s\n", __func__);

	while (--loops) {
		iiscon = readl(i2s->regs + DRIME4_I2SCON);
		if (iiscon & DRIME4_I2SCON_LRINDEX)
			break;

		cpu_relax();
	}

	if (!loops) {
		pr_err("%s: timeout\n", __func__);
		return -ETIMEDOUT;
	}

	return 0;
}

/*
 * Set DRIME4 I2S DAI format
 */
static int drime4_i2s_set_fmt(struct snd_soc_dai *cpu_dai,
			       unsigned int fmt)
{
	struct drime4_i2s_info *i2s = to_info(cpu_dai);
	u32 iismod;

	pr_debug("Entered %s\n", __func__);
	NULL_TEST(i2s, 1);

	iismod = readl(i2s->regs + DRIME4_I2SMOD);
	pr_debug("hw_params r: IISMOD: %x\n", iismod);

	switch (fmt & SND_SOC_DAIFMT_MASTER_MASK) {
	case SND_SOC_DAIFMT_CBM_CFM:
		i2s->master = 0;
		iismod |= DRIME4_I2SMOD_SLAVE;
		break;
	case SND_SOC_DAIFMT_CBS_CFS:
		i2s->master = 1;
		iismod &= ~DRIME4_I2SMOD_SLAVE;
		break;
	default:
		pr_err("unknwon master/slave format\n");
		return -EINVAL;
	}

	iismod &= ~DRIME4_I2SMOD_SDF_MASK;

	switch (fmt & SND_SOC_DAIFMT_FORMAT_MASK) {
	case SND_SOC_DAIFMT_RIGHT_J:
		iismod |= DRIME4_I2SMOD_LR_RLOW;
		iismod |= DRIME4_I2SMOD_SDF_MSB;
		break;
	case SND_SOC_DAIFMT_LEFT_J:
		iismod |= DRIME4_I2SMOD_LR_RLOW;
		iismod |= DRIME4_I2SMOD_SDF_LSB;
		break;
	case SND_SOC_DAIFMT_I2S:
		iismod &= ~DRIME4_I2SMOD_LR_RLOW;
		iismod |= DRIME4_I2SMOD_SDF_IIS;
		break;
	default:
		pr_err("Unknown data format\n");
		return -EINVAL;
	}

	writel(iismod, i2s->regs + DRIME4_I2SMOD);
	pr_debug("hw_params w: IISMOD: %x\n", iismod);
	return 0;
}

static int drime4_i2s_hw_params(struct snd_pcm_substream *substream,
				 struct snd_pcm_hw_params *params,
				 struct snd_soc_dai *dai)
{
	struct drime4_i2s_info *i2s = to_info(dai);
	struct drime4_dma_params *dma_data;
	u32 iismod;

	pr_debug("Entered %s\n", __func__);
	NULL_TEST(i2s, 1);
	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
		dma_data = i2s->dma_playback;
	else
		dma_data = i2s->dma_capture;

	snd_soc_dai_set_dma_data(dai, substream, dma_data);

	/* Working copies of register */
	iismod = readl(i2s->regs + DRIME4_I2SMOD);
	pr_debug("%s: r: IISMOD: %x\n", __func__, iismod);

	iismod &= ~DRIME4_I2SMOD_BLC_MASK;
	/* Sample size */
	switch (params_format(params)) {
	case SNDRV_PCM_FORMAT_S8:
		iismod |= DRIME4_I2SMOD_BLC_8BIT;
		break;
	case SNDRV_PCM_FORMAT_S16_LE:
		break;
	case SNDRV_PCM_FORMAT_S24_LE:
		iismod |= DRIME4_I2SMOD_BLC_24BIT;
		break;
	}

	writel(iismod, i2s->regs + DRIME4_I2SMOD);
	pr_debug("%s: w: IISMOD: %x\n", __func__, iismod);

	return 0;
}

static int drime4_i2s_set_sysclk(struct snd_soc_dai *cpu_dai,
				  int clk_id, unsigned int freq, int dir)
{
	struct drime4_i2s_info *i2s = to_info(cpu_dai);
	u32 iismod;
	NULL_TEST(i2s, 1);
	iismod = readl(i2s->regs + DRIME4_I2SMOD);

	pr_debug("Entered %s\n", __func__);
	pr_debug("%s r: IISMOD: %x\n", __func__, iismod);

	switch (clk_id) {
	case DRIME4_I2S_CLKSRC_CDCLK:
		/* Error if controller doesn't have the CDCLKCON bit */
		if (!(i2s->feature & DRIME4_FEATURE_CDCLKCON))
			return -EINVAL;

		switch (dir) {
		case SND_SOC_CLOCK_IN:
			iismod |= DRIME4_I2SMOD_CDCLKCON;
			break;
		case SND_SOC_CLOCK_OUT:
			iismod &= ~DRIME4_I2SMOD_CDCLKCON;
			break;
		default:
			return -EINVAL;
		}
		break;

	default:
		return -EINVAL;
	}

	writel(iismod, i2s->regs + DRIME4_I2SMOD);
	pr_debug("%s w: IISMOD: %x\n", __func__, iismod);

	return 0;
}

static int drime4_i2s_trigger(struct snd_pcm_substream *substream, int cmd,
			       struct snd_soc_dai *dai)
{
	struct snd_soc_pcm_runtime *rtd;
	struct drime4_i2s_info *i2s;
	int capture; 
	unsigned long irqs;
	int ret = 0;
	struct drime4_dma_params *dma_data;
		
	NULL_TEST(substream, 1);
	rtd = substream->private_data;
	NULL_TEST(rtd, 1);
	i2s = to_info(rtd->cpu_dai);
	NULL_TEST(i2s, 1);
	capture = (substream->stream == SNDRV_PCM_STREAM_CAPTURE);
	dma_data = snd_soc_dai_get_dma_data(rtd->cpu_dai, substream);
	
	pr_debug("Entered %s\n", __func__);

	switch (cmd) {
	case SNDRV_PCM_TRIGGER_START:
		/* On start, ensure that the FIFOs are cleared and reset. */

		writel(capture ? DRIME4_I2SFIC_RXFLUSH : DRIME4_I2SFIC_TXFLUSH,
		       i2s->regs + DRIME4_I2SFIC);

		/* clear again, just in case */
		writel(0x0, i2s->regs + DRIME4_I2SFIC);

	case SNDRV_PCM_TRIGGER_RESUME:
	case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
		if (!i2s->master) {
			ret = drime4_snd_lrsync(i2s);
			if (ret)
				goto exit_err;
		}

		local_irq_save(irqs);

		if (capture)
			drime4_snd_rxctrl(i2s, 1);
		else
			drime4_snd_txctrl(i2s, 1);

		local_irq_restore(irqs);

		/*
		 * Load the next buffer to DMA to meet the reqirement
		 * of the auto reload mechanism of DRIME4.
		 */
		dma_async_issue_pending(dma_data->chan);

		break;

	case SNDRV_PCM_TRIGGER_STOP:
	case SNDRV_PCM_TRIGGER_SUSPEND:
	case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
		local_irq_save(irqs);

		if (capture)
			drime4_snd_rxctrl(i2s, 0);
		else
			drime4_snd_txctrl(i2s, 0);

		local_irq_restore(irqs);
		break;
	default:
		ret = -EINVAL;
		break;
	}

exit_err:
	return ret;
}

/*
 * Set DRIME4 Clock dividers
 */
static int drime4_i2s_set_clkdiv(struct snd_soc_dai *cpu_dai,
				  int div_id, int div)
{
	struct drime4_i2s_info *i2s;
	u32 reg;

	pr_debug("%s(%p, %d, %d)\n", __func__, cpu_dai, div_id, div);
	i2s = to_info(cpu_dai);
	NULL_TEST(i2s, 1);
	
	switch (div_id) {
	case DRIME4_I2S_DIV_BCLK:
		switch (div) {
		case 16:
			div = DRIME4_I2SMOD_BCLK_16FS;
			break;

		case 32:
			div = DRIME4_I2SMOD_BCLK_32FS;
			break;

		case 24:
			div = DRIME4_I2SMOD_BCLK_24FS;
			break;

		case 48:
			div = DRIME4_I2SMOD_BCLK_48FS;
			break;

		default:
			return -EINVAL;
		}

		reg = readl(i2s->regs + DRIME4_I2SMOD);
		reg &= ~DRIME4_I2SMOD_BCLK_MASK;
		writel(reg | div, i2s->regs + DRIME4_I2SMOD);

		pr_debug("%s: MOD=%08x\n", __func__,
			readl(i2s->regs + DRIME4_I2SMOD));
		break;

	case DRIME4_I2S_DIV_RCLK:
		switch (div) {
		case 256:
			div = DRIME4_I2SMOD_RCLK_256FS;
			break;

		case 384:
			div = DRIME4_I2SMOD_RCLK_384FS;
			break;

		case 512:
			div = DRIME4_I2SMOD_RCLK_512FS;
			break;

		case 768:
			div = DRIME4_I2SMOD_RCLK_768FS;
			break;

		default:
			return -EINVAL;
		}

		reg = readl(i2s->regs + DRIME4_I2SMOD);
		reg &= ~DRIME4_I2SMOD_RCLK_MASK;
		writel(reg | div, i2s->regs + DRIME4_I2SMOD);
		pr_debug("%s: MOD=%08x\n", __func__,
			readl(i2s->regs + DRIME4_I2SMOD));
		break;

	case DRIME4_I2S_DIV_PRESCALER:
		if (div >= 0) {
			writel(div << 8,
			       i2s->regs + DRIME4_I2SPSR);
		} else {
			writel(0x0, i2s->regs + DRIME4_I2SPSR);
		}
		pr_debug("%s: PSR=%08x\n", __func__,
			readl(i2s->regs + DRIME4_I2SPSR));
		break;

	default:
		return -EINVAL;
	}

	return 0;
}

static snd_pcm_sframes_t drime4_i2s_delay(struct snd_pcm_substream *substream,
					   struct snd_soc_dai *dai)
{
	struct drime4_i2s_info *i2s;
	u32 reg;
    snd_pcm_sframes_t delay;
	i2s = to_info(dai);
	NULL_TEST(i2s, 0);
	reg = readl(i2s->regs + DRIME4_I2SFIC);
	
	NULL_TEST(substream, 0);
	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
		delay = DRIME4_I2SFIC_TXCOUNT(reg);
	else
		delay = DRIME4_I2SFIC_RXCOUNT(reg);

	return delay;
}

struct clk *drime4_i2s_get_clock(struct snd_soc_dai *cpu_dai)
{
	struct drime4_i2s_info *i2s;
	i2s	= to_info(cpu_dai);
	NULL_TEST(i2s, 0);
	return i2s->iis_cclk;
}
EXPORT_SYMBOL_GPL(drime4_i2s_get_clock);

/* default table of all avaialable root fs divisors */
static unsigned int iis_fs_tab[] = { 256, 512, 384, 768 };

int drime4_i2s_iis_calc_rate(struct drime4_i2s_rate_calc *info,
			    unsigned int *fstab,
			    unsigned int rate, unsigned long clkrate)
{
	unsigned int div;
	unsigned int fsclk;
	unsigned int actual;
	unsigned int fs;
	unsigned int fsdiv;
	signed int deviation = 0;
	unsigned int best_fs = 0;
	unsigned int best_div = 0;
	unsigned int best_rate = 0;
	unsigned int best_deviation = INT_MAX;

	pr_debug("Input clock rate %ldHz\n", clkrate);

	if (fstab == NULL)
		fstab = iis_fs_tab;

	for (fs = 0; fs < ARRAY_SIZE(iis_fs_tab); fs++) {
		fsdiv = iis_fs_tab[fs];

		fsclk = clkrate / fsdiv;
		div = fsclk / rate;

		if ((fsclk % rate) > (rate / 2))
			div++;

		if (div == 0)
			continue;

		actual = clkrate / (fsdiv * div);
		deviation = actual - rate;

		pr_debug("%ufs: div %u => result %u, deviation %d\n",
		       fsdiv, div, actual, deviation);

		deviation = abs(deviation);

		if (deviation < best_deviation) {
			best_fs = fsdiv;
			best_div = div;
			best_rate = actual;
			best_deviation = deviation;
		}

		if (deviation == 0)
			break;
	}

	pr_debug("best: fs=%u, div=%u, rate=%u\n",
	       best_fs, best_div, best_rate);

	info->fs_div = best_fs;
	info->clk_div = best_div;

	return 0;
}
EXPORT_SYMBOL_GPL(drime4_i2s_iis_calc_rate);

static struct snd_soc_dai_ops drime4_i2s_dai_ops = {
	.hw_params	= drime4_i2s_hw_params,
	.delay		= drime4_i2s_delay,
	.trigger	= drime4_i2s_trigger,
	.set_fmt	= drime4_i2s_set_fmt,
	.set_clkdiv	= drime4_i2s_set_clkdiv,
	.set_sysclk	= drime4_i2s_set_sysclk,
};

static __devinit int drime4_i2s_probe(struct snd_soc_dai *dai)
{
	snd_soc_dai_set_drvdata(dai, &drime4_i2s[dai->id]);
	return 0;
}

static struct snd_soc_dai_driver drime4_i2s_dai_driver = {
	.probe = drime4_i2s_probe,
	.playback = {
		.channels_min = 2,
		.channels_max = 2,
		.rates = DRIME4_I2S_RATES,
		.formats = DRIME4_I2S_FMTS, },
	.capture = {
		.channels_min = 1,
		.channels_max = 2,
		.rates = DRIME4_I2S_RATES,
		.formats = DRIME4_I2S_FMTS, },
	.ops = &drime4_i2s_dai_ops,
	.symmetric_rates = 1,
};


static __devinit int drime4_i2s_dev_probe(struct platform_device *pdev)
{
	struct drime4_audio_pdata *i2s_pdata;
	struct drime4_i2s_info *i2s;
	struct resource *res;
	int ret;

	i2s = &drime4_i2s[pdev->id];

	i2s->feature |= DRIME4_FEATURE_CDCLKCON;

	i2s->dma_capture = &drime4_i2s_pcm_stereo_in[pdev->id];
	i2s->dma_playback = &drime4_i2s_pcm_stereo_out[pdev->id];

	res = platform_get_resource(pdev, IORESOURCE_DMA, 0);
	if (!res) {
		dev_err(&pdev->dev, "Unable to get I2S-TX dma resource\n");
		return -ENXIO;
	}
	i2s->dma_playback->channel = res->start;

	res = platform_get_resource(pdev, IORESOURCE_DMA, 1);
	if (!res) {
		dev_err(&pdev->dev, "Unable to get I2S-RX dma resource\n");
		return -ENXIO;
	}
	i2s->dma_capture->channel = res->start;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		dev_err(&pdev->dev, "Unable to get I2S SFR address\n");
		return -ENXIO;
	}

	res = devm_request_mem_region(&pdev->dev, res->start,
		resource_size(res), pdev->name);

	if (!res) {
		dev_err(&pdev->dev, "Unable to request SFR region\n");
		return -EBUSY;
	}
	i2s->dma_capture->dma_addr = res->start + DRIME4_I2SRXD;
	i2s->dma_playback->dma_addr = res->start + DRIME4_I2STXD;

	i2s->dma_capture->client = &drime4_dma_client_in;
	i2s->dma_capture->dma_size = 4;
	i2s->dma_playback->client = &drime4_dma_client_out;
	i2s->dma_playback->dma_size = 4;

	i2s_pdata = pdev->dev.platform_data;
	if (i2s_pdata && i2s_pdata->cfg_gpio && i2s_pdata->cfg_gpio(pdev)) {
		dev_err(&pdev->dev, "Unable to configure gpio\n");
		return -EINVAL;
	}

	i2s->regs = devm_ioremap(&pdev->dev, res->start, resource_size(res));
	if (!i2s->regs) {
		dev_err(&pdev->dev, "cannot ioremap registers\n");
		return -ENXIO;
	}

	i2s->iis_cclk = clk_get(&pdev->dev, "i2s");
	if (IS_ERR(i2s->iis_cclk)) {
		dev_err(&pdev->dev, "failed to get i2s clock\n");
		ret = PTR_ERR(i2s->iis_cclk);
		goto err;
	}

	clk_enable(i2s->iis_cclk);

	ret = snd_soc_register_dai(&pdev->dev, &drime4_i2s_dai_driver);
	if (ret != 0)
		goto err_clk;

	return 0;

err_clk:
	clk_disable(i2s->iis_cclk);
	clk_put(i2s->iis_cclk);
err:
	return ret;
}

static __devexit int drime4_i2s_dev_remove(struct platform_device *pdev)
{
	snd_soc_unregister_dai(&pdev->dev);

	clk_disable(drime4_i2s[pdev->id].iis_cclk);
	clk_put(drime4_i2s[pdev->id].iis_cclk);

	return 0;
}

static struct platform_driver drime4_i2s_driver = {
	.probe  = drime4_i2s_dev_probe,
	.remove = drime4_i2s_dev_remove,
	.driver = {
		.name = "drime4-i2s",
		.owner = THIS_MODULE,
	},
};

static int __init drime4_i2s_init(void)
{
	return platform_driver_register(&drime4_i2s_driver);
}
#ifndef CONFIG_SCORE_FAST_RESUME
module_init(drime4_i2s_init);
#else
fast_dev_initcall(drime4_i2s_init);
#endif

static void __exit drime4_i2s_exit(void)
{
	platform_driver_unregister(&drime4_i2s_driver);
}
module_exit(drime4_i2s_exit);

/* Module information */
MODULE_AUTHOR("Chanho Park, <chanho61.park@samsung.com>, \
               Byungho Ahn <bh1212.ahn@samsnug.com>");
MODULE_DESCRIPTION("DRIME4 I2S SoC Interface");
MODULE_LICENSE("GPL");
