/* linux/arch/arm/mach-drime4/dev-emmc.c
 *
 * Copyright (c) 2011 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com
 *
 * Samsung device definition for DesingWare mobile storage
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/mmc/host.h>
#include <linux/mmc/dw_mmc.h>
#include <linux/dma-mapping.h>
#include <linux/delay.h>
#include <linux/clk.h>
#include <linux/pinctrl/pinmux.h>
#include <linux/pinctrl/pinconf.h>
#include <linux/pinctrl/pinconf-drime4.h>

#include <mach/map.h>
#include <mach/gpio.h>

#define DRIME4_SZ_DW_MMC_SD1	(0x1000)

static struct resource drime4_emmc_resource[] = {
	[0] = {
		.start	= DRIME4_PA_SD1,
		.end	= DRIME4_PA_SD1 + DRIME4_SZ_DW_MMC_SD1 - 1,
		.flags  = IORESOURCE_MEM,
	},
	[1] = {
		.start	= IRQ_SD1,
		.end	= IRQ_SD1,
		.flags	= IORESOURCE_IRQ,
	}
};

static u64 drime4_device_emmc_dmamask = DMA_BIT_MASK(32);

static int emmc_get_ocr(u32 slot_id)
{
	return MMC_VDD_165_195 | MMC_VDD_32_33 | MMC_VDD_33_34;
}

static void emmc_setpower(u32 slot_id, u32 volt)
{
	/* do nothing */
}


static void emmc_set_clk_phase(u32 slot_id, u8 timing)
{
	/* TODO: change ccin clk phase according to emmc mode */
}

static void emmc_set_clk(struct dw_mci *host, u8 timing)
{
	/* TODO: change ccin clk according to emmc mode */
}

static int emmc_init(u32 slot_id, irq_handler_t dummy, void *data)
{
	unsigned int reg;
	struct dw_mci *host = (struct dw_mci *)data;
	struct pinmux *pmx;
	unsigned long conf;
	int ret = 0;

	/* Power pin set */
	pmx = pinmux_get(&host->pdev->dev, NULL);
	if (!IS_ERR(pmx)) {
		ret = pinmux_enable(pmx);
		if (ret) {
			dev_err(&host->pdev->dev,
				"failed to enable pinmux\n");
			return -EFAULT;
		}
		dev_info(&host->pdev->dev, "%s: start pin config\n", __func__);
		conf = to_config_packed(PIN_CONFIG_DRIVE_STRENGTH,
			PIN_CONFIG_DRIVE_STRENGTH_X6);
		ret = pin_config_group_set("drime4-pinmux", "sd1grp", conf);
		if (ret) {
			dev_err(&host->pdev->dev,
				"failed to control sd1grp pingroup\n");
			return -EFAULT;
		}
	}

	/* IO Sharing Controll SDMMC out enable */
	/* SD1 Clock Setting  */
	host->clk = clk_get(&host->pdev->dev, "sd1");
	if (IS_ERR(host->clk)) {
		dev_err(&host->pdev->dev, "No clock is provided.\n");
		host->clk = NULL;
		return -EFAULT;
	} else
		clk_enable(host->clk);

	clk_set_rate(host->clk, 100*1000000);

	/* SD1 clock inversion control register */
	conf = to_config_packed(PIN_CONFIG_VSEL18_CTRL_SD1_CLK_INV,
		PIN_CONFIG_VSEL18_CTRL_SD1_CLK_INV_ENABLE);
	ret = pin_config_set("drime4-pinmux", "VSEL_CTRL", conf);
	if (ret) {
		dev_err(&host->pdev->dev, "failed to control VSEL_CTRL pin\n");
		return -EFAULT;
	}

	/* SD1 Platform clock phase setting
	 * [31]  : SD1 clock enable
	 * [4:5] : SD1 sample clock phase setting
	 *	0: 0, 1: 90, 2: 180, 3: 270
	 * [0:1] : SD1 drive clock phase setting
	 *	0: 0, 1: 90, 2: 180, 3: 270
	 */
	reg = readl(DRIME4_VA_SD_CFG + 0x4);
	reg &= (~(1<<31));
	writel(reg, DRIME4_VA_SD_CFG + 0x4);

	reg = readl(DRIME4_VA_SD_CFG + 0x4);
	reg |= ((1<<31) | (2<<4) | 3);
	writel(reg, DRIME4_VA_SD_CFG + 0x4);

	return ret;
}

static struct dw_mci_board drime4_emmc_def_platdata = {
	.quirks		=
			DW_MCI_QUIRK_BROKEN_CARD_DETECTION |
			0,
	.caps		=
			MMC_CAP_MMC_HIGHSPEED |
			MMC_CAP_4_BIT_DATA |
			MMC_CAP_8_BIT_DATA |
			MMC_CAP_NONREMOVABLE |
			MMC_CAP_MAX_CURRENT_800 |
			MMC_CAP_ERASE |
			0,
	.num_slots	= 1,
	.init		= emmc_init,
	.get_ocr	= emmc_get_ocr,
	.setpower	= emmc_setpower,
	.set_clk	= emmc_set_clk,
	.set_clk_phase	= emmc_set_clk_phase,
	.bus_hz		= 52000000,
	.fifo_depth	= 16,
	.fifo_trans_value = 2,
};

struct platform_device drime4_device_emmc = {
	.name			= "dw_mmc",
	.id			= 0,
	.num_resources		= ARRAY_SIZE(drime4_emmc_resource),
	.resource		= drime4_emmc_resource,
	.dev			= {
		.dma_mask		= &drime4_device_emmc_dmamask,
		.platform_data		= &drime4_emmc_def_platdata,
		.coherent_dma_mask	= DMA_BIT_MASK(32),
	},
};
EXPORT_SYMBOL(drime4_device_emmc);
