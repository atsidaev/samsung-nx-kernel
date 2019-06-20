/* linux/arch/arm/mach-drime4/dev-sdmmc.c
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
#include <linux/pinctrl/pinconf-drime4.h>
#include <linux/pinctrl/pinconf.h>
#include <linux/pinctrl/consumer.h>

#include <mach/map.h>
#include <mach/gpio.h>
#ifdef CONFIG_MMC_DW_SYSFS
#include <mach/dw_mmc_sysfs.h>
#endif

#define DRIME4_SZ_DW_MMC_SD0	(0x1000)

static const u16 tuning_phase_table[] = {
	PHASE_SHIFT_0,
	PHASE_SHIFT_90,
	PHASE_SHIFT_180,
	PHASE_SHIFT_270,
};

static struct resource drime4_sdmmc_resource[] = {
	[0] = {
		.start	= DRIME4_PA_SD0,
		.end	= DRIME4_PA_SD0 + DRIME4_SZ_DW_MMC_SD0 - 1,
		.flags  = IORESOURCE_MEM,
	},
	[1] = {
		.start	= IRQ_SD0,
		.end	= IRQ_SD0,
		.flags	= IORESOURCE_IRQ,
	},
};

static u64 drime4_device_sdmmc_dmamask = DMA_BIT_MASK(32);

static int sdmmc_get_ocr(u32 slot_id)
{
	return MMC_VDD_165_195 | MMC_VDD_32_33 | MMC_VDD_33_34;
}

static void sdmmc_setpower(u32 slot_id, u32 volt)
{
	u32 reg;
	if (volt == MMC_VDD_165_195) {
		/* After 0xFFFF, Change vsel18 */
		writel(0xFFFF, DRIME4_VA_GLOBAL_CTRL + 0x1004);
		/* Decreament counter enable - 10th bit */
		reg = readl(DRIME4_VA_GLOBAL_CTRL + 0x100C);
		writel(reg | 0x400, DRIME4_VA_GLOBAL_CTRL + 0x100C);
		mdelay(10);
	}
	/* code for high frequency. VSEL 1.8 -> 3.3 */
	if (volt == -1) {
		reg = readl(DRIME4_VA_GLOBAL_CTRL + 0x100C);
		writel(reg & (~0x8), DRIME4_VA_GLOBAL_CTRL + 0x100C);
	}
}

static void sdmmc_tune_sample_phase (u32 slot_id, u32 index)
{
	/* SD0 Platform clock phase setting
	 * [31]  : SD0 clock enable
	 * [4:5] : SD0 sample clock phase setting
	 *	0: 0, 1: 90, 2: 180, 3: 270
	 * [0:1] : SD0 drive clock phase setting
	 *	0: 0, 1: 90, 2: 180, 3: 270
	 */
	u32 reg;
	u32 table_index = index % 4;
	u16 sample_phase = tuning_phase_table[table_index];
	u16 drive_phase = PHASE_SHIFT_180;

	reg = readl(DRIME4_VA_SD_CFG);
	reg &= (~(1<<31));
	writel(reg, DRIME4_VA_SD_CFG);

	reg = ((1<<31) | (sample_phase<<4) | drive_phase);

	writel(reg, DRIME4_VA_SD_CFG);

	printk(KERN_INFO "tuning sample phase : %d, drive phase : %d\n",
			sample_phase, drive_phase);
}

static void sdmmc_set_clk_phase(u32 slot_id, u8 timing)
{
	/* SD0 Platform clock phase setting
	 * [31]  : SD0 clock enable
	 * [4:5] : SD0 sample clock phase setting
	 *	0: 0, 1: 90, 2: 180, 3: 270
	 * [0:1] : SD0 drive clock phase setting
	 *	0: 0, 1: 90, 2: 180, 3: 270
	 */
	u32 reg;
	u16 sample_phase, drive_phase;

	switch (timing) {
	case MMC_TIMING_UHS_SDR104:
		sample_phase = PHASE_SHIFT_0;
		drive_phase = PHASE_SHIFT_180;
		break;
	case MMC_TIMING_UHS_SDR50:
	case MMC_TIMING_UHS_DDR50:
		sample_phase = PHASE_SHIFT_180;
		drive_phase = PHASE_SHIFT_180;
		break;
	case MMC_TIMING_SD_HS:
	case MMC_TIMING_SD_NORMAL:
		sample_phase = PHASE_SHIFT_0;
		drive_phase = PHASE_SHIFT_180;
		break;
	case MMC_TIMING_LEGACY:
	default:
		sample_phase = PHASE_SHIFT_0;
		drive_phase = PHASE_SHIFT_270;
		break;
	}

	reg = readl(DRIME4_VA_SD_CFG);
	reg &= (~(1<<31));
	writel(reg, DRIME4_VA_SD_CFG);

#if CONFIG_MMC_DW_SYSFS
	if (g_tunable)
		reg = ((1<<31) | (g_sample_phase<<4) | g_drive_phase);
	else
#endif
		reg = ((1<<31) | (sample_phase<<4) | drive_phase);

	writel(reg, DRIME4_VA_SD_CFG);
#if 0
	printk(KERN_INFO "setting sample phase : %d, drive phase : %d\n",
			sample_phase, drive_phase);
#endif
}

static void sdmmc_set_clk(struct dw_mci *host, u8 timing)
{
	if (host == NULL)
		return ;

	switch (timing) {
	case MMC_TIMING_UHS_SDR104:
	case MMC_TIMING_UHS_SDR50:
		host->bus_hz = 162*1000000;
		clk_set_rate(host->clk, 324*1000000);
		break;
	case MMC_TIMING_SD_HS:
	case MMC_TIMING_SD_NORMAL:
		host->bus_hz = 50*1000000;
		clk_set_rate(host->clk, 100*1000000);
		break;
	case MMC_TIMING_LEGACY:
	default:
		host->bus_hz = 27*1000000/2;
		clk_set_rate(host->clk, 27*1000000);
		break;
	}
}

static int sdmmc_init(u32 slot_id, irq_handler_t dummy, void *data)
{
	struct dw_mci *host = (struct dw_mci *)data;
	unsigned long pin_config;
	int ret = 0;

	/* SD0 Clock Setting  */
	host->clk = clk_get(&host->dev, "sd0");
	if (IS_ERR(host->clk)) {
		dev_err(&host->dev, "No clock is provided.\n");
		host->clk = NULL;
		return -EFAULT;
	} else
		clk_enable(host->clk);

	clk_set_rate(host->clk, 27*1000000);
	sdmmc_set_clk_phase(slot_id, MMC_TIMING_LEGACY);

	pin_config = to_config_packed(PIN_CONFIG_DRIVE_STRENGTH,
			PIN_CONFIG_DRIVE_STRENGTH_X6);
	ret = pin_config_group_set("drime4-pinmux", "sd0grp", pin_config);
#ifdef CONFIG_MMC_DW_SYSFS
	ret = dw_mci_create_node(&host->dev);
	if (ret < 0)
		printk("Fail to create dw_mmc sysfs node");
#endif

	return ret;
}

static void sdmmc_exit(u32 slot_id, void *data)
{
	struct dw_mci *host = (struct dw_mci *)data;
#ifdef CONFIG_MMC_DW_SYSFS
	dw_mci_remove_node(&host->dev);
#endif
}

#ifdef CONFIG_BOOTLOADER_SNAPSHOT
struct dw_mci_board drime4_sdmmc_def_platdata = {
#else
struct dw_mci_board drime4_sdmmc_def_platdata = {
#endif
	.quirks		=
			DW_MCI_PRESET_BOARD_POWER |
			DW_MCI_QUIRK_DELAY_BEFORE_CMD6 |
			0,
	.caps		=
			/*MMC_CAP_UHS_SDR25 |*/
			MMC_CAP_UHS_SDR50 |
			/*MMC_CAP_UHS_DDR50 |*/	/* don't use ddr mode */
			MMC_CAP_UHS_SDR104 |
			/*MMC_CAP_1_8V_DDR |*/
			MMC_CAP_4_BIT_DATA |
			MMC_CAP_SD_HIGHSPEED |
			MMC_CAP_ERASE |
			0,
	.caps2		=
			0,
	.num_slots	= 1,
	.init		= sdmmc_init,
	.exit		= sdmmc_exit,
	.get_ocr	= sdmmc_get_ocr,
	.setpower	= sdmmc_setpower,
	.set_clk	= sdmmc_set_clk,
	.set_clk_phase	= sdmmc_set_clk_phase,
	.tune_sample_phase = sdmmc_tune_sample_phase,
	.bus_hz		= 27*1000000/2,
	.fifo_depth	= 128,
	.fifo_trans_value = 3,
	.blk_settings = 0,
};

struct platform_device drime4_device_sdmmc = {
	.name			= "dw_mmc_sdcard",
	.id			= 0,
	.num_resources		= ARRAY_SIZE(drime4_sdmmc_resource),
	.resource		= drime4_sdmmc_resource,
	.dev			= {
		.dma_mask		= &drime4_device_sdmmc_dmamask,
		.platform_data		= &drime4_sdmmc_def_platdata,
		.coherent_dma_mask	= DMA_BIT_MASK(32),
	},
};
EXPORT_SYMBOL(drime4_device_sdmmc);
