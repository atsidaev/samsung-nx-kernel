/* linux/arch/arm/mach-drime4/dev-sdio.c
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
#include <linux/export.h>
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

#define DRIME4_SZ_DW_MMC_SD1	(0x1000)

static struct resource drime4_sdio_resource[] = {
	[0] = {
		.start	= DRIME4_PA_SD1,
		.end	= DRIME4_PA_SD1 + DRIME4_SZ_DW_MMC_SD1 - 1,
		.flags  = IORESOURCE_MEM,
	},
	[1] = {
		.start	= IRQ_SD1,
		.end	= IRQ_SD1,
		.flags	= IORESOURCE_IRQ,
	},
};

static u64 drime4_device_sdio_dmamask = DMA_BIT_MASK(32);

#if 1 /* 20120820 Matt WiFi on/off */
static struct platform_device msm_wlan_ar6000_pm_device = {
    .name           = "wlan_ar6000_pm_dev",
    .id             = -1,
};

static int __init sdmmc_init_ar6000pm(void)
{
    return platform_device_register(&msm_wlan_ar6000_pm_device);
}
#endif /* CONFIG_ARCH_S5C7380_BCM4325 */

static int sdio_get_ocr(u32 slot_id)
{
	return MMC_VDD_32_33 | MMC_VDD_33_34;
}

static void sdio_setpower(u32 slot_id, u32 volt)
{
	return;
}

static void sdio_set_clk_phase(u32 slot_id, u8 timing)
{
	/* SD1 Platform clock phase setting
	 * [31]  : SD1 clock enable
	 * [4:5] : SD1 sample clock phase setting
	 *	0: 0, 1: 90, 2: 180, 3: 270
	 * [0:1] : SD1 drive clock phase setting
	 *	0: 0, 1: 90, 2: 180, 3: 270
	 */
	unsigned int reg;

	reg = readl(DRIME4_VA_SD_CFG + 0x4);
	reg &= (~(1<<31));
	writel(reg, DRIME4_VA_SD_CFG + 0x4);

	reg = readl(DRIME4_VA_SD_CFG + 0x4);
	reg |= ((1<<31) | (00<<4) | 2);
	writel(reg, DRIME4_VA_SD_CFG + 0x4);

#if 0 /* 20120605 */
	writel((1<<15), DRIME4_VA_GLOBAL_CTRL + 0x1008);
#endif

	return;
}

static void sdio_set_clk(struct dw_mci *host, u8 timing)
{
	return;
}

static int sdio_init(u32 slot_id, irq_handler_t dummy, void *data)
{
	struct dw_mci *host = (struct dw_mci *)data;
	unsigned long pin_config;
	int ret = 0;

	/* SD1 Clock Setting  */
	host->clk = clk_get(&host->dev, "sd1");
	if (IS_ERR(host->clk)) {
		dev_err(&host->dev, "No clock is provided.\n");
		host->clk = NULL;
		return -EFAULT;
	} else
		clk_enable(host->clk);

	clk_set_rate(host->clk, 100000000); /*200000000*/
	sdio_set_clk_phase(slot_id, 0);

	{
	struct pinctrl		*pmx;
	struct pinctrl_state	*pinstate;
	/* setup pad configuration */
	pmx = devm_pinctrl_get(&host->dev);
	if (IS_ERR(pmx))
		dev_dbg(&host->dev, "cannot find pinmux configuration\n");
	else {
		pinstate = pinctrl_lookup_state(pmx, PINCTRL_STATE_DEFAULT);

		ret = pinctrl_select_state(pmx, pinstate);
		if (ret) {
			dev_err(&host->dev, "cannot select pins state\n");
			return -EFAULT;
			}
		}
	}

	pin_config = to_config_packed(PIN_CONFIG_DRIVE_STRENGTH,
			PIN_CONFIG_DRIVE_STRENGTH_X6);
	ret = pin_config_group_set("drime4-pinmux", "sd1grp", pin_config);

#if 1 /* 20120820 Matt WiFi on/off */
	{
#ifdef CONFIG_MACH_D4_NX2000
		pinctrl_request_gpio(GPIO_WIFI_LDO_ON);
		gpio_request(GPIO_WIFI_LDO_ON, "wifi_power");

		gpio_direction_output(GPIO_WIFI_LDO_ON, 1);	
#endif /* CONFIG_MACH_D4_NX2000 */

		pinctrl_request_gpio(GPIO_WIFI_RESET_N);
		gpio_request(GPIO_WIFI_RESET_N, "wifi_reset");

		gpio_direction_output(GPIO_WIFI_RESET_N, 1);
	}
#ifdef CONFIG_MACH_D4_NX2000
	gpio_set_value(GPIO_WIFI_LDO_ON, 0);
#endif /* CONFIG_MACH_D4_NX2000 */
	gpio_set_value(GPIO_WIFI_RESET_N, 0);

	sdmmc_init_ar6000pm();
#endif /* CONFIG_ARCH_S5C7380_BCM4325 */

#ifdef CONFIG_MMC_DW_SYSFS
	ret = dw_mci_create_node(&host->dev);
	if (ret < 0)
		pr_err("Fail to create dw_mmc sysfs node");
#endif

	return ret;
}

static void sdio_exit(u32 slot_id, void *data)
{
	struct dw_mci *host = (struct dw_mci *)data;
#ifdef CONFIG_MMC_DW_SYSFS
	dw_mci_remove_node(&host->dev);
#endif
}

static struct dw_mci_board drime4_sdio_def_platdata = {
	.quirks     =
			0,
	.caps		=
			MMC_CAP_MMC_HIGHSPEED | MMC_CAP_SD_HIGHSPEED |
			MMC_CAP_4_BIT_DATA |
			0,
	.num_slots	= 1,
	.init		= sdio_init,
	.exit		= sdio_exit,
	.get_ocr	= sdio_get_ocr,
	.setpower	= sdio_setpower,
	.set_clk	= sdio_set_clk,
	.set_clk_phase	= sdio_set_clk_phase,
	.bus_hz		= 50000000, /*102000000,*/
	.fifo_depth	= 128,
	.fifo_trans_value = 3,
	.blk_settings = 0,
};


struct platform_device drime4_device_sdio = {
	.name			= "dw_mmc_sdio",
	.id			= 0,
	.num_resources		= ARRAY_SIZE(drime4_sdio_resource),
	.resource		= drime4_sdio_resource,
	.dev			= {
	.dma_mask		= &drime4_device_sdio_dmamask,
	.platform_data		= &drime4_sdio_def_platdata,
	.coherent_dma_mask	= DMA_BIT_MASK(32),
	},
};
EXPORT_SYMBOL(drime4_device_sdio);
