/* linux/drivers/mtd/nand/drime4.c
 *
 * Samsung drime4 NAND driver
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifdef CONFIG_MTD_NAND_DRIME4_DEBUG
#define DEBUG
#endif

#include <linux/module.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/ioport.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/clk.h>
#include <linux/cpufreq.h>
#include <linux/mutex.h>
#include <linux/irq.h>
#include <linux/interrupt.h>

#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/nand_ecc.h>
#include <linux/mtd/partitions.h>
#include "../../media/video/drime4/bma/d4_bma_if.h"
#include <asm/io.h>

#include <mach/regs-nand.h>
#include <mach/nand.h>
#include <mach/map.h>
#include <mach/d4_mem.h>

#ifdef CONFIG_MTD_NAND_DRIME4_HWECC
static int hardware_ecc = 1;
#else
static int hardware_ecc = 0;
#endif

#ifdef CONFIG_MTD_NAND_DRIME4_CLKSTOP
static int clock_stop = 1;
#else
static const int clock_stop = 0;
#endif

#if 0
#ifndef CONFIG_BLK_DEV_INITRD
#define __USE_NAND_DMA_READ__
#endif
#endif

#ifdef __USE_NAND_DMA_READ__
#define __NDMA_INT_ISR_ENALBE__

static dma_addr_t g_nand_dma_paddr = 0;
static void * g_nand_dma_vaddr = 0;
static volatile uint32_t g_rnDmaAdr = 0, g_rnDmaStat = 0;

#ifdef __NDMA_INT_ISR_ENALBE__
static struct mutex nDmaMutex = {0, };
static int g_inDmaIrq;
#endif

#endif


#define OFFSET_SCT_PARITY	16
#define NUM_SECTOR			8


extern int add_mtd_device(struct mtd_info *mtd);

/** for test message
#define TEST_MESSAGE	0
*/
/* new oob placement block for use with hardware ecc generation
 */
/* TODO */
static struct nand_ecclayout nand_2K_hw_eccoob = {
	.eccbytes = 4*8,
	.eccpos = {
	/* ecc data location in spare area 2K page 4bit ecc case */
		32, 33, 34, 35, 36, 37, 38, 39,
		40, 41, 42, 43, 44, 45, 46, 47,
		48, 49, 50, 51, 52, 53, 54, 55,
		56, 57, 58, 59, 60, 61, 62, 63 },
	.oobfree = {
		{.offset = 1,
		 .length = 31} }
};

static struct nand_ecclayout nand_4K_hw_eccoob = {
#if 0
	.eccbytes = 16*8,
	.eccpos = {
	/* ecc data location in spare area 4K page 4bit ecc case */
		  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23,
		 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51,
		 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
		 92, 93, 94, 95, 96, 97, 98, 99,100,101,102,103,104,105,106,107,
		120,121,122,123,124,125,126,127,128,129,130,131,132,133,134,135,
		148,149,150,151,152,153,154,155,156,157,158,159,160,161,162,163,
		176,177,178,179,180,181,182,183,184,185,186,187,188,189,190,191,
		204,205,206,207,208,209,210,211,212,213,214,215,216,217,218,219
		},
	.oobfree = {
		{.offset = 1,
		 .length = 95} }
#else
	.eccbytes = OFFSET_SCT_PARITY*NUM_SECTOR,
	.eccpos = {
	/* ecc data location in spare area 4K page 4bit ecc case */
		 96, 97, 98, 99,100,101,102,103,104,105,106,107,108,109,110,111,
		112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,
		128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,
		144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159,
		160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,
		176,177,178,179,180,181,182,183,184,185,186,187,188,189,190,191,
		192,193,194,195,196,197,198,199,200,201,202,203,204,205,206,207,
		208,209,210,211,212,213,214,215,216,217,218,219,220,221,222,223
		},
	.oobfree = {
		{.offset = 1,
		 .length = 95} }
#endif
};

static struct nand_ecclayout nand_hw_eccoob = {
	.eccbytes = 4,
	.eccpos = {
		60, 61, 62, 63 },
	.oobfree = {
		{.offset = 1,
		 .length = 59} }
};

/* controller and mtd information */

struct drime4_nand_info;

/**
 * struct drime4_nand_mtd - driver MTD structure
 * @mtd: The MTD instance to pass to the MTD layer.
 * @chip: The NAND chip information.
 * @set: The platform information supplied for this set of NAND chips.
 * @info: Link back to the hardware information.
 * @scan_res: The result from calling nand_scan_ident().
*/
struct drime4_nand_mtd {
	struct mtd_info			mtd;
	struct nand_chip		chip;
	struct drime4_nand_set		*set;
	struct drime4_nand_info		*info;
	int				scan_res;
};

/* overview of the drime4 nand state */

/**
 * struct drime4_nand_info - NAND controller state.
 * @mtds: An array of MTD instances on this controoler.
 * @platform: The platform data for this board.
 * @device: The platform device we bound to.
 * @area: The IO area resource that came from request_mem_region().
 * @clk: The clock resource for this controller.
 * @regs: The area mapped for the hardware registers described by @area.
 * @sel_reg: Pointer to the register controlling the NAND selection.
 * @sel_bit: The bit in @sel_reg to select the NAND chip.
 * @mtd_count: The number of MTDs created from this controller.
 * @save_sel: The contents of @sel_reg to be saved over suspend.
 * @clk_rate: The clock rate from @clk.
 * @cpu_type: The exact type of this controller.
 */
struct drime4_nand_info {
	/* mtd info */
	struct nand_hw_control		controller;
	struct drime4_nand_mtd		*mtds;
	struct drime4_platform_nand	*platform;

	/* device info */
	struct device			*device;
	struct resource			*area;
	struct clk			*clk;
	void __iomem			*regs;
	void __iomem			*regs_ndma;
	void __iomem			*sel_reg;
	int				sel_bit;
	int				mtd_count;
	unsigned long			save_sel;
	unsigned long			clk_rate;

#ifdef CONFIG_CPU_FREQ
	struct notifier_block	freq_transition;
#endif
};


/* conversion functions */

static struct drime4_nand_mtd *drime4_nand_mtd_toours(struct mtd_info *mtd)
{
	return container_of(mtd, struct drime4_nand_mtd, mtd);
}

static struct drime4_nand_info *drime4_nand_mtd_toinfo(struct mtd_info *mtd)
{
	return drime4_nand_mtd_toours(mtd)->info;
}

static struct drime4_nand_info *to_nand_info(struct platform_device *dev)
{
	return platform_get_drvdata(dev);
}

static struct drime4_platform_nand *to_nand_plat(struct platform_device *dev)
{
	return dev->dev.platform_data;
}

static inline int allow_clk_stop(struct drime4_nand_info *info)
{
	return clock_stop;
}

/* timing calculations */

#define NS_IN_KHZ 1000000

/**
 * drime4_nand_calc_rate - calculate timing data.
 * @wanted: The cycle time in nanoseconds.
 * @clk: The clock rate in kHz.
 * @max: The maximum divider value.
 *
 * Calculate the timing value from the given parameters.
 */
static int drime4_nand_calc_rate(int wanted, unsigned long clk, int max)
{
	int result;

	result = DIV_ROUND_UP((wanted * clk), NS_IN_KHZ);

	pr_debug("result %d from %ld, %d\n", result, clk, wanted);

	if (result > max) {
		printk("%d ns is too big for current clock rate %ld\n", wanted, clk);
		return -1;
	}

	if (result < 1)
		result = 1;

	return result;
}

#define to_ns(ticks,clk) (((ticks) * NS_IN_KHZ) / (unsigned int)(clk))

/* controller setup */

/**
 * drime4_nand_setrate - setup controller timing information.
 * @info: The controller instance.
 *
 * Given the information supplied by the platform, calculate and set
 * the necessary timing registers in the hardware to generate the
 * necessary timing cycles to the hardware.
 */
static int drime4_nand_setrate(struct drime4_nand_info *info)
{
	struct drime4_platform_nand *plat = info->platform;
	int tacls_max = 8;
	int tacls, twrph0, twrph1;
	unsigned long clkrate = clk_get_rate(info->clk);
	unsigned long uninitialized_var(set), cfg, uninitialized_var(mask);
	unsigned long flags;

	/* calculate the timing information for the controller */
	dev_dbg(info->device, "drime4_nand_setrate()\n");

	info->clk_rate = clkrate;
	clkrate /= 1000;	/* turn clock into kHz for ease of use */

	if (plat != NULL) {
		/* TODO: check this point when drime4-es-board bring up */
		/* 0: not available, available value is 1~8 -> real value is -1 */
		tacls = 1;  /*drime4_nand_calc_rate(plat->tacls, clkrate, tacls_max);*/
		twrph0 = 4; /* drime4_nand_calc_rate(plat->twrph0, clkrate, 8);*/
		twrph1 = 2; /*drime4_nand_calc_rate(plat->twrph1, clkrate, 8);*/
	} else {
		/* default timings */
		tacls = 8;
		twrph0 = 8;
		twrph1 = 8;
	}

	if (tacls < 0 || twrph0 < 0 || twrph1 < 0) {
		dev_err(info->device, "cannot get suitable timings\n");
		return -EINVAL;
	}

	dev_info(info->device, "Tacls=%d, %dns Twrph0=%d %dns, Twrph1=%d %dns\n",
	       tacls, to_ns(tacls, clkrate), twrph0, to_ns(twrph0, clkrate), twrph1, to_ns(twrph1, clkrate));

	mask = (DRIME4_NFCONF_TACLS(tacls_max - 1) |
		DRIME4_NFCONF_PAGE_SIZE |
		DRIME4_NFCONF_NANDBOOT |
		DRIME4_NFCONF_BUSWIDTH |
		DRIME4_NFCONF_LSG_LENGTH |
		DRIME4_NFCONF_ECC_TYPE_MASK |
		DRIME4_NFCONF_TWRPH0(7) |
		DRIME4_NFCONF_TWRPH1(7));

	set = DRIME4_NFCONF_TACLS(tacls - 1);
	set |= DRIME4_NFCONF_TWRPH0(twrph0 - 1);
	set |= DRIME4_NFCONF_TWRPH1(twrph1 - 1);
	set |= DRIME4_NFCONF_ADDR_CYCLE;

	local_irq_save(flags);

	cfg = __raw_readl(info->regs + DRIME4_NFCONF);
	cfg &= ~mask;
	cfg |= set;

	__raw_writel(cfg, info->regs + DRIME4_NFCONF);

	local_irq_restore(flags);

	dev_dbg(info->device, "NF_CONF is 0x%lx\n", cfg);

	return 0;
}

/**
 * drime4_nand_inithw - basic hardware initialisation
 * @info: The hardware state.
 *
 * Do the basic initialisation of the hardware, using drime4_nand_setrate()
 * to setup the hardware access speeds and set the controller to be enabled.
*/
static int drime4_nand_inithw(struct drime4_nand_info *info)
{
	int ret;
	uint32_t conf;
	uint32_t ctrl;

	dev_dbg(info->device, "drime4_nand_inithw()\n");

	ret = drime4_nand_setrate(info);

	if (ret < 0)
		return ret;

	conf = __raw_readl(info->regs + DRIME4_NFCONF);
	ctrl = __raw_readl(info->regs + DRIME4_NFCONT);

#if defined(CONFIG_MTD_NAND_DRIME4_8BIT_ECC)
	conf |= (DRIME4_NFCONF_ECC_TYPE(DRIME4_NFECC_MLC8BIT)
#else
	conf |= (DRIME4_NFCONF_ECC_TYPE(DRIME4_NFECC_MLC4BIT)
#endif
		| DRIME4_NFCONF_MLCFLASH
		| DRIME4_NFCONF_BUSWIDTH_8);


	ctrl = (DRIME4_NFCONT_ECCEN_INT
		| DRIME4_NFCONT_ILLEGALACC_EN
		| DRIME4_NFCONT_RNBINT_EN
		| DRIME4_NFCONT_nFCE3
		| DRIME4_NFCONT_nFCE2
		| DRIME4_NFCONT_nFCE1
		| DRIME4_NFCONT_ENABLE);

	__raw_writel(conf, info->regs + DRIME4_NFCONF);
	__raw_writel(ctrl, info->regs + DRIME4_NFCONT);

	dev_dbg(info->device, "NFCONF:%08lx, NFCONT:%08lx\n", conf, ctrl);

	return 0;
}

/**
 * drime4_nand_select_chip - select the given nand chip
 * @mtd: The MTD instance for this chip.
 * @chip: The chip number.
 *
 * This is called by the MTD layer to either select a given chip for the
 * @mtd instance, or to indicate that the access has finished and the
 * chip can be de-selected.
 *
 * The routine ensures that the nFCE line is correctly setup, and any
 * platform specific selection code is called to route nFCE to the specific
 * chip.
 */
static void drime4_nand_select_chip(struct mtd_info *mtd, int chip)
{
	struct drime4_nand_info *info;
	struct drime4_nand_mtd *nmtd;
	struct nand_chip *this = mtd->priv;
	unsigned long cur;

	nmtd = this->priv;
	info = nmtd->info;

	if (chip != -1 && allow_clk_stop(info))
		clk_enable(info->clk);

	cur = __raw_readl(info->sel_reg);

	if (chip == -1) {
		cur |= info->sel_bit;
	} else {
		if (nmtd->set != NULL && chip > nmtd->set->nr_chips) {
			dev_err(info->device, "invalid chip %d\n", chip);
			return;
		}

		if (info->platform != NULL) {
			if (info->platform->select_chip != NULL)
				(info->platform->select_chip) (nmtd->set, chip);
		}

		cur &= ~info->sel_bit;
	}

	__raw_writel(cur, info->sel_reg);

	if (chip == -1 && allow_clk_stop(info))
		clk_disable(info->clk);
}

/* command and control functions */
static void drime4_nand_hwcontrol(struct mtd_info *mtd, int cmd,
				   unsigned int ctrl)
{
	struct drime4_nand_info *info = drime4_nand_mtd_toinfo(mtd);

	if (cmd == NAND_CMD_NONE)
		return;

	if (ctrl & NAND_CLE)
		__raw_writeb(cmd, info->regs + DRIME4_NFCMD);
	else
		__raw_writeb(cmd, info->regs + DRIME4_NFADDR);

}

/* drime4_nand_devready()
 *
 * returns 0 if the nand is busy, 1 if it is ready
*/

static int drime4_nand_devready(struct mtd_info *mtd)
{
	struct drime4_nand_info *info = drime4_nand_mtd_toinfo(mtd);
	return __raw_readb(info->regs + DRIME4_NFSTAT) & DRIME4_NFSTAT_READY;
}

/* ECC handling functions */
static int drime4_nand_correct_data(struct mtd_info *mtd, u_char *dat,
				     u_char *read_ecc, u_char *calc_ecc)
{
	struct drime4_nand_info *info = drime4_nand_mtd_toinfo(mtd);
	uint8_t err_cnt;
	uint8_t err_pattern[4];
	uint32_t err_loc[4];

	uint32_t meccerr0, meccerr1, bitpattern;
	u_char rd_byte, tmp;

	meccerr0 = __raw_readl(info->regs + DRIME4_NFMECC_ERR0);
	meccerr1 = __raw_readl(info->regs + DRIME4_NFMECC_ERR1);
	bitpattern = __raw_readl(info->regs + DRIME4_NFMLCBITPT);

	dev_dbg(info->device, "drime4_nand_correct_data() base_addr:%08lx, meccerr0/1:%08lx/%08lx, bitpattern:%08lx\n",
		dat, meccerr0, meccerr1, bitpattern);

	err_cnt = (uint8_t)((meccerr0 >> 26) & 0x7);
	if (err_cnt > 4) {
		return -1;
	}
	err_loc[0] = (meccerr0) & 0x3ff;
	err_loc[1] = (meccerr0 >> 16) & 0x3ff;
	err_loc[2] = (meccerr1) & 0x3ff;
	err_loc[3] = (meccerr1 >> 16) & 0x3ff;

	err_pattern[0] = (uint8_t)(bitpattern >> 0);
	err_pattern[1] = (uint8_t)(bitpattern >> 8);
	err_pattern[2] = (uint8_t)(bitpattern >> 16);
	err_pattern[3] = (uint8_t)(bitpattern >> 24);

	/**
	dev_dbg(info->device, "__/err_num:%d, err_loc:%d/%d/%d/%d, err_ptrn:%02x %02x %02x %02x\n",
		err_cnt, err_loc[0], err_loc[1], err_loc[2], err_loc[3], err_pattern[0], err_pattern[1], err_pattern[2], err_pattern[3]);
	*/

	while (err_cnt--) {
		rd_byte = __raw_readb(dat + err_loc[err_cnt]);

		if ((rd_byte & err_pattern[err_cnt]) == 0) {
			tmp = rd_byte | err_pattern[err_cnt];
			__raw_writeb(tmp, dat + err_loc[err_cnt]);
		} else {
			tmp = rd_byte & (~err_pattern[err_cnt]);
			__raw_writeb(tmp , dat + err_loc[err_cnt]);
		}
		/**
		dev_dbg(info->device,"__/err_cnt:%d, err_loc:%x+%x, %02x -> %02x, real:%02x\n",
			err_cnt, dat, err_loc[err_cnt], rd_byte, tmp, __raw_readb(dat + err_loc[err_cnt]) );
		*/
	}

	return 0;

}
static int drime4_nand_correct_data_8b_ecc(struct mtd_info *mtd, u_char *dat,
				     u_char *read_ecc, u_char *calc_ecc)
{
	struct drime4_nand_info *info = drime4_nand_mtd_toinfo(mtd);
	uint8_t err_cnt;
	uint8_t err_pattern[8];
	uint32_t err_loc[8];

	uint32_t meccerr0, meccerr1,meccerr2, bitpattern0, bitpattern1;
	u_char rd_byte, tmp;

	meccerr0 = __raw_readl(info->regs + DRIME4_NF8ECCERR0);
	meccerr1 = __raw_readl(info->regs + DRIME4_NF8ECCERR1);
	meccerr2 = __raw_readl(info->regs + DRIME4_NF8ECCERR2);
	bitpattern0 = __raw_readl(info->regs + DRIME4_NFM8BITPT0);
	bitpattern1 = __raw_readl(info->regs + DRIME4_NFM8BITPT1);

	dev_dbg(info->device, "drime4_nand_correct_data() base_addr:%08lx, meccerr:%08lx/%08lx/%08lx, bitpattern:%08lx/%08lx\n",
		dat, meccerr0, meccerr1, meccerr1, bitpattern0, bitpattern1);

	err_cnt = (uint8_t)((meccerr0 >> 25) & 0xf);
	if (err_cnt > 8) {
		return -1;
	}

	err_loc[0] = (meccerr0) & 0x3ff;
	err_loc[1] = (meccerr0>>15) & 0x3ff;
	err_loc[2] = (meccerr1) & 0x3ff;
	err_loc[3] = (meccerr1>>11) & 0x3ff;
	err_loc[4] = (meccerr1>>22) & 0x3ff;
	err_loc[5] = (meccerr2) & 0x3ff;
	err_loc[6] = (meccerr2>>11) & 0x3ff;
	err_loc[7] = (meccerr2>>22) & 0x3ff;

	err_pattern[0] = (unsigned char)(bitpattern0 >>  0);
	err_pattern[1] = (unsigned char)(bitpattern0 >>  8);
	err_pattern[2] = (unsigned char)(bitpattern0 >> 16);
	err_pattern[3] = (unsigned char)(bitpattern0 >> 24);
	err_pattern[4] = (unsigned char)(bitpattern1 >>  0);
	err_pattern[5] = (unsigned char)(bitpattern1 >>  8);
	err_pattern[6] = (unsigned char)(bitpattern1 >> 16);
	err_pattern[7] = (unsigned char)(bitpattern1 >> 24);

	/**
	dev_dbg(info->device, "__/err_num:%d, err_loc:%d/%d/%d/%d, err_ptrn:%02x %02x %02x %02x\n",
		err_cnt, err_loc[0], err_loc[1], err_loc[2], err_loc[3], err_pattern[0], err_pattern[1], err_pattern[2], err_pattern[3]);
	*/

	while (err_cnt--) {
		rd_byte = __raw_readb(dat + err_loc[err_cnt]);

		if ((rd_byte & err_pattern[err_cnt]) == 0) {
			tmp = rd_byte | err_pattern[err_cnt];
			__raw_writeb(tmp, dat + err_loc[err_cnt]);
		} else {
			tmp = rd_byte & (~err_pattern[err_cnt]);
			__raw_writeb(tmp , dat + err_loc[err_cnt]);
		}
		/**
		dev_dbg(info->device,"__/err_cnt:%d, err_loc:%x+%x, %02x -> %02x, real:%02x\n",
			err_cnt, dat, err_loc[err_cnt], rd_byte, tmp, __raw_readb(dat + err_loc[err_cnt]) );
		*/
	}

	return 0;

}

/* ECC functions
 *
 * These allow the drime4 to use the controller's ECC
 * generator block to ECC the data as it passes through
*/

static void drime4_nand_enable_hwecc(struct mtd_info *mtd, int mode)
{
	struct drime4_nand_info *info = drime4_nand_mtd_toinfo(mtd);
	uint32_t ctrl;
	uint32_t stat;

	ctrl = __raw_readl(info->regs + DRIME4_NFCONT);
	stat = __raw_readl(info->regs + DRIME4_NFSTAT);

	if (mode == NAND_ECC_WRITE) {
		ctrl |= (DRIME4_NFCONT_ECCDIR_ENC | DRIME4_NFCONT_ECCEN_INT);
		ctrl &= ~DRIME4_NFCONT_ECCDE_INT;
	} else {
		ctrl |= DRIME4_NFCONT_ECCDE_INT;
		ctrl &= ~(DRIME4_NFCONT_ECCDIR_ENC | DRIME4_NFCONT_ECCEN_INT);
	}

	/*ctrl |= (DRIME4_NFCONT_ILLEGALACC_EN | DRIME4_NFCONT_RNBINT_EN);*/

	__raw_writel(ctrl, info->regs + DRIME4_NFCONT);
	__raw_writel(stat | DRIME4_NFSTAT_RnB_CHANGE, info->regs + DRIME4_NFSTAT);

}
static void drime4_nand_enable_8b_hwecc(struct mtd_info *mtd, int mode)
{
	struct drime4_nand_info *info = drime4_nand_mtd_toinfo(mtd);
	uint32_t ctrl;
	uint32_t stat;

	ctrl = __raw_readl(info->regs + DRIME4_NFCONT);
	stat = __raw_readl(info->regs + DRIME4_NFSTAT);

	if (mode == NAND_ECC_WRITE) {
		ctrl |= (DRIME4_NFCONT_ECCDIR_ENC | DRIME4_NFCONT_ECCEN_INT);
		ctrl &= ~DRIME4_NFCONT_ECCDE_INT;
	} else {
		ctrl |= DRIME4_NFCONT_ECCDE_INT;
		ctrl &= ~(DRIME4_NFCONT_ECCDIR_ENC | DRIME4_NFCONT_ECCEN_INT);
	}

	ctrl |= DRIME4_NFCONT_ECC_STOP;
	/*ctrl |= (DRIME4_NFCONT_ILLEGALACC_EN | DRIME4_NFCONT_RNBINT_EN);*/

	__raw_writel(ctrl, info->regs + DRIME4_NFCONT);
	__raw_writel(stat | DRIME4_NFSTAT_RnB_CHANGE, info->regs + DRIME4_NFSTAT);

}

static void drime4_nand_start_hwmecc(struct mtd_info *mtd)
{
	struct drime4_nand_info *info = drime4_nand_mtd_toinfo(mtd);
	uint32_t ctrl;

	ctrl = __raw_readl(info->regs + DRIME4_NFCONT);

	ctrl &= ~DRIME4_NFCONT_MAIN_ECCLOCK;

	__raw_writel(ctrl | DRIME4_NFCONT_INITMECC, info->regs + DRIME4_NFCONT);

}

/* ECC check for error or not
*/
static int drime4_nand_calculate_ecc(struct mtd_info *mtd, const u_char *dat, u_char *ecc_code)
{
	struct drime4_nand_info *info = drime4_nand_mtd_toinfo(mtd);
	uint32_t ctrl;
	uint32_t stat;
	uint32_t mecc[2];

	/* check ecc decord done */
	mecc[0] = ecc_code;
	mecc[1] = ecc_code + 4;

	dev_dbg(info->device, "drime4_nand_calculate_ecc(), meccerr0 meccerr1 : %08lx %08lx, &ecc_code:%08lx, %08lx\n",
			mecc[0], mecc[1], ecc_code, ecc_code+4);

	if ((mecc[0] >> 29) & 0x01 == 0x01) {
		dev_dbg(info->device, "        No ERROR\n");
	} else if (((mecc[0] >> 26) & 0x7) == 0x0) {
		dev_dbg(info->device, "        No ERROR\n");
	} else if (((mecc[0] >> 26) & 0x7) < 5) {
		dev_dbg(info->device, "        ECC ERROR(correctable)\n");
	} else {
		dev_err(info->device, "        ECC Uncorrectable  ERROR\n");
	}

	pr_debug("calculate_ecc: returning mecc1,mecc0 : %08lx,%08lx\n",
		mecc[1], mecc[0]);

	return 0;
}

static int drime4_nand_calculate_8b_ecc(struct mtd_info *mtd, const u_char *dat, u_char *ecc_code)
{
	struct drime4_nand_info *info = drime4_nand_mtd_toinfo(mtd);
	uint32_t ctrl;
	uint32_t stat;
	uint32_t mecc[3];

	/* check ecc decord done */
	mecc[0] = ecc_code;
	mecc[1] = ecc_code + 4;
	mecc[2] = ecc_code + 8;

	dev_dbg(info->device, "drime4_nand_calculate_ecc(), meccerr0/1/2 : %08lx %08lx %08lx, &ecc_code:%08lx %08lx %08lx\n",
			mecc[0], mecc[1], mecc[2], ecc_code, ecc_code+4, ecc_code+12);

	if ((mecc[0] >> 29) & 0x01 == 0x01) {
		dev_dbg(info->device, " 	   No ERROR\n");
	} else if (((mecc[0] >> 25) & 0xf) == 0x0) {
		dev_dbg(info->device, " 	   No ERROR\n");
	} else if (((mecc[0] >> 25) & 0xf) < 9) {
		dev_dbg(info->device, " 	   ECC ERROR(correctable)\n");
	} else {
		dev_err(info->device, " 	   ECC Uncorrectable  ERROR\n");
	}

	pr_debug("calculate_ecc: returning mecc0,mecc1,mecc2 : %08lx,%08lx,%08lx\n",
		mecc[0], mecc[1], mecc[2]);

	return 0;
}

/* over-ride the standard functions for a little more speed. We can
 * use read/write block to move the data buffers to/from the controller
*/

static void drime4_nand_read_buf(struct mtd_info *mtd, u_char *buf, int len)
{
	struct drime4_nand_info *info = drime4_nand_mtd_toinfo(mtd);

	__raw_readsl(info->regs + DRIME4_NFDATA, buf, len >> 2);

	/* cleanup if we've got less than a word to do */
	if (len & 3) {
		buf += len & ~3;

		for (; len & 3; len--)
			*buf++ = __raw_readb(info->regs + DRIME4_NFDATA);
	}
}

static void drime4_nand_write_buf(struct mtd_info *mtd, const u_char *buf, int len)
{
	struct drime4_nand_info *info = drime4_nand_mtd_toinfo(mtd);

	__raw_writesl(info->regs + DRIME4_NFDATA, buf, len >> 2);

	/* cleanup any fractional write */
	if (len & 3) {
		buf += len & ~3;

		for (; len & 3; len--, buf++)
			__raw_writeb(*buf, info->regs + DRIME4_NFDATA);
	}
}

#ifdef __USE_NAND_DMA_READ__

static int drime4_nanddma_err_correct(struct mtd_info *mtd, struct nand_chip *chip, int stat)
{
	struct drime4_nand_info *info = drime4_nand_mtd_toinfo(mtd);
	uint8_t err_cnt;
	uint8_t err_pattern[4];
	uint32_t err_loc[4];
	uint32_t page_size;
	uint32_t err_baseAddr;

	uint32_t meccerr0, meccerr1, bitpattern, dbgStat, dbgPg, dbgSct;
	uint32_t ndma_conf;
	unsigned char rd_byte, tmp;

	uint32_t sts_errpage;
	unsigned char sts_errsct;
	unsigned char sts_errecc;
	uint8_t *read_ecc;
	uint8_t *calc_ecc;
	int err;

	sts_errpage = stat >> 8;
	sts_errsct = (stat >> 4) & 0x7;

	meccerr0 = __raw_readl(info->regs + DRIME4_NFMECC_ERR0);
	meccerr1 = __raw_readl(info->regs + DRIME4_NFMECC_ERR1);
	bitpattern = __raw_readl(info->regs + DRIME4_NFMLCBITPT);

	dbgStat = __raw_readl(info->regs_ndma + DRIME4_NDMA_STS);
	dbgPg = __raw_readl(info->regs_ndma + DRIME4_NDMA_ROW_ADDR);
	dbgSct = __raw_readl(info->regs_ndma + DRIME4_NDMA_SCT_COUNT);

	//ndma_conf = __raw_readl(info->regs_ndma + DRIME4_NDMA_CONF);
	//page_size = (ndma_conf & 0x1) ? 4096 : 2048;
	page_size = 2048;
	err_baseAddr = __raw_readl(info->regs_ndma + DRIME4_NDMA_DST_ADDR);
	err_baseAddr += (sts_errpage*page_size + sts_errsct*512);
	
	//printk("\n>>>>>>> nDMA Ecc [buf:0x%x, pgs:%d, stat:0x%x, dat:0x%x, eccer0:0x%x, eccer1:0x%x, eccptn:0x%x, mon(stat:0x%x, pg:%d, sct:%d)]", err_baseAddr, dbgPg, stat, *(uint32_t *)err_baseAddr, meccerr0, meccerr1, bitpattern, dbgStat, dbgPg, dbgSct);


	// Indicates the page data red from NAND Flash has all 0xFF
	if ((meccerr0>>29) & 0x01 == 0x01)
	{
		//printk("\n>>>>>>> nDMA Read ECC_CORRECT: Indicates the page data red from NAND Flash has all 0xFF");
		return DRIME4_NDMA_OK;
	}

	err_cnt = (uint8_t)((meccerr0 >> 26) & 0x7);

	if(err_cnt == 0)
	{
		return DRIME4_NDMA_OK;
	}
	else if(err_cnt > 4)
	{
		printk("\n>>>>>>> nDMA Read DRIME4_NDMA_ECC_UNCORRECT");
		return DRIME4_NDMA_ECC_UNCORRECT;
	}
	else
	{}

#if 0
	err = chip->ecc.correct(mtd, (uint8_t *)(err_baseAddr), read_ecc, calc_ecc);
#else

	err_loc[0] = (meccerr0) & 0x3ff;
	err_loc[1] = (meccerr0 >> 16) & 0x3ff;
	err_loc[2] = (meccerr1) & 0x3ff;
	err_loc[3] = (meccerr1 >> 16) & 0x3ff;

	err_pattern[0] = (uint8_t)(bitpattern >> 0);
	err_pattern[1] = (uint8_t)(bitpattern >> 8);
	err_pattern[2] = (uint8_t)(bitpattern >> 16);
	err_pattern[3] = (uint8_t)(bitpattern >> 24);

	while(err_cnt)
	{
		err_cnt--;
		
		rd_byte = __raw_readb(err_baseAddr + err_loc[err_cnt]);

		if((rd_byte & err_pattern[err_cnt]) == 0)
		{
			tmp = rd_byte | err_pattern[err_cnt];
			__raw_writeb(tmp, err_baseAddr + err_loc[err_cnt]);
		}
		else
		{
			tmp = rd_byte & (~err_pattern[err_cnt]);
			__raw_writeb(tmp, err_baseAddr + err_loc[err_cnt]);
		}

		//printk("\n>>>>>>> nDMA Ecc [err_pg:%d, err_sct:%d, dst:0x%x, loc:0x%x(%d), res:0x%02x -> 0x%02x]", sts_errpage, sts_errsct, (err_baseAddr + err_loc[err_cnt]), err_loc[err_cnt], err_loc[err_cnt], rd_byte, tmp);

	}
#endif

	return DRIME4_NDMA_OK;

}


/* page read with nanddma
*/
#ifdef __NDMA_INT_ISR_ENALBE__
static irqreturn_t drime4_nand_dma_isr(int irq, void *dev_id)
{
	disable_irq_nosync(irq);
	
	g_rnDmaStat = __raw_readl(g_rnDmaAdr + DRIME4_NDMA_INT_STS);

	//printk("\n>>>>>>> drime4_nand_dma_isr (stat:0x%x)", g_rnDmaStat);

	__raw_writel(DRIME4_NDMA_CTL_INTCLR, g_rnDmaAdr + DRIME4_NDMA_CONT);

	mutex_unlock(&nDmaMutex);

	enable_irq(irq);

	return IRQ_HANDLED;
}
#endif


/* page read with nanddma
*/
int drime4_nand_read_withdma(struct mtd_info *mtd, struct nand_chip *chip,	uint8_t *buf, int oob_required, int page)
{
	struct drime4_nand_info *info = drime4_nand_mtd_toinfo(mtd);
	volatile uint32_t gclkcon1, dst, conf, cont, stat, vic1enb, vic1clr, vic1Pri, vic1RawStat, vic1IrqStat, dbgStat, dbgPg, dbgSct;
	int ret =  DRIME4_NDMA_OK;

	//stat = __raw_readl(info->regs + DRIME4_NFSTAT);
	//__raw_writel((stat | DRIME4_NFSTAT_ECC_DECDONE), info->regs + DRIME4_NFSTAT);

	// nDMA INT Setting -------------------------------------------------------------------------------------------------------------------------
	// nDMA INT Enalbe
	//vic1enb = __raw_readl(DRIME4_VA_VIC1 + 0x10);
	//__raw_writel((vic1enb | (0x1 << 31)), DRIME4_VA_VIC1 + 0x10);
	//__raw_writel(0x3, DRIME4_VA_VIC1 + 0x028);
	//__raw_writel(0x3, DRIME4_VA_VIC1 + 0x27c);
	//__raw_writel(drime4_nand_dma_isr, DRIME4_VA_VIC1 + 0x17c);

	g_rnDmaAdr = info->regs_ndma;
	g_rnDmaStat = 0;

	vic1IrqStat = __raw_readl(DRIME4_VA_VIC1 + 0x0);
	vic1RawStat = __raw_readl(DRIME4_VA_VIC1 + 0x08);
	vic1enb = __raw_readl(DRIME4_VA_VIC1 + 0x10);
	
	// NFC Setting ------------------------------------------------------------------------------------------------------------------------------
	conf = __raw_readl(info->regs + DRIME4_NFCONF);
	conf |= (DRIME4_NFCONF_ECC_TYPE(DRIME4_NFECC_MLC4BIT));
	conf &= ~(DRIME4_NFCONF_MLC_CLKCON);
	__raw_writel(conf, info->regs + DRIME4_NFCONF);

	cont = __raw_readl(info->regs + DRIME4_NFCONT);
	cont |= (DRIME4_NFCONT_ECCDE_INT|DRIME4_NFCONT_ILLEGALACC_EN|DRIME4_NFCONT_RNBINT_EN|DRIME4_NFCONT_INITMECC);
	cont &= ~(DRIME4_NFCONT_ECCDIR_ENC|DRIME4_NFCONT_ECCEN_INT|DRIME4_NFCONT_RN_FALLING|DRIME4_NFCONT_MAIN_ECCLOCK);
	__raw_writel(cont, info->regs + DRIME4_NFCONT);

	// nDMA Setting -----------------------------------------------------------------------------------------------------------------------------
	__raw_writel(info->regs, info->regs_ndma + DRIME4_NDMA_BASE_ADDR);
	__raw_writel(page , info->regs_ndma + DRIME4_NDMA_SRC_ADDR);
	__raw_writel(g_nand_dma_paddr, info->regs_ndma + DRIME4_NDMA_DST_ADDR);
	__raw_writel(0, info->regs_ndma + DRIME4_NDMA_PG_SIZE);

	//conf = (chip->ecc.size == 2048) ? 0 : 1;
	conf = (DRIME4_NDMA_INT_COMPLETE | DRIME4_NDMA_INT_ECCERR);
	__raw_writel(conf, info->regs_ndma + DRIME4_NDMA_CONF);

#if 0
	if(page >= 5312 && page <= 5316)
	{
		uint32_t nDmaReg[6];
		uint32_t nfcReg[17];

		nfcReg[0] = __raw_readl(info->regs + DRIME4_NFCONF);
		nfcReg[1] = __raw_readl(info->regs + DRIME4_NFCONT);
		nfcReg[2] = __raw_readl(info->regs + DRIME4_NFCMD);
		nfcReg[3] = __raw_readl(info->regs + DRIME4_NFADDR);
		nfcReg[4] = __raw_readl(info->regs + DRIME4_NFDATA);
		nfcReg[5] = __raw_readl(info->regs + DRIME4_NFECCD0);
		nfcReg[6] = __raw_readl(info->regs + DRIME4_NFECCD1);
		nfcReg[7] = __raw_readl(info->regs + DRIME4_NFECCD);
		nfcReg[8] = __raw_readl(info->regs + DRIME4_NFSBLK);
		nfcReg[9] = __raw_readl(info->regs + DRIME4_NFEBLK);
		nfcReg[10] = __raw_readl(info->regs + DRIME4_NFSTAT);
		nfcReg[11] = __raw_readl(info->regs + DRIME4_NFMECC_ERR0);
		nfcReg[12] = __raw_readl(info->regs + DRIME4_NFMECC_ERR1);
		nfcReg[13] = __raw_readl(info->regs + DRIME4_NFMECC0);
		nfcReg[14] = __raw_readl(info->regs + DRIME4_NFMECC1);
		nfcReg[15] = __raw_readl(info->regs + DRIME4_NFSECC);
		nfcReg[16] = __raw_readl(info->regs + DRIME4_NFMLCBITPT);
			
		nDmaReg[0] = __raw_readl(info->regs_ndma + DRIME4_NDMA_DST_ADDR);
		nDmaReg[1] = __raw_readl(info->regs_ndma + DRIME4_NDMA_SRC_ADDR);
		nDmaReg[2] = __raw_readl(info->regs_ndma + DRIME4_NDMA_INT_STS);
		nDmaReg[3] = __raw_readl(info->regs_ndma + DRIME4_NDMA_BASE_ADDR);
		nDmaReg[4] = __raw_readl(info->regs_ndma + DRIME4_NDMA_CONF);
		nDmaReg[5] = __raw_readl(info->regs_ndma + DRIME4_NDMA_PG_SIZE);
		
		printk("\n>>>>>>> rNFC Get [cfg:0x%x, ctl:0x%x, cmd:0x%x, adr:0x%x, dat:0x%x, meccd0:0x%x, meccd1:0x%x, seccd:0x%x]", nfcReg[0], nfcReg[1], nfcReg[2], nfcReg[3], nfcReg[4], nfcReg[5], nfcReg[6], nfcReg[7]);
		printk("\n>>>>>>> rNFC Get [sblk:0x%x, eblk:0x%x, stat:0x%x, eccer0:0x%x, eccer1:0x%x, gecc0:0x%x, gecc1:0x%x, geccs:0x%x, eccptn:0x%x]", nfcReg[8], nfcReg[9], nfcReg[10], nfcReg[11], nfcReg[12], nfcReg[13], nfcReg[14], nfcReg[15], nfcReg[16]);
		printk("\n>>>>>>> nDMA Set [dst:0x%x, src:%d, stat:0x%x, nfc:0x%x, cfg:0x%x, pgn:%d, INT(enb:0x%x, stat:0x%x, rawSt:0x%x)]", nDmaReg[0], nDmaReg[1], nDmaReg[2], nDmaReg[3], nDmaReg[4], nDmaReg[5], vic1enb, vic1IrqStat, vic1RawStat);

	}
#endif
	
	cont = (DRIME4_NDMA_CTL_INTCLR | DRIME4_NDMA_CTL_START);
	__raw_writel(cont, info->regs_ndma + DRIME4_NDMA_CONT);

	while(1)
	{		
		#ifdef __NDMA_INT_ISR_ENALBE__
		mutex_lock(&nDmaMutex);
		//printk("\n>>>>>>> nDMA Read Page(%d) Wait nDMA Int. released!", page);
		#else
		g_rnDmaStat= __raw_readl(info->regs_ndma + DRIME4_NDMA_INT_STS);
		#endif

		if((g_rnDmaStat & DRIME4_NDMA_STS_COMPLETE) == DRIME4_NDMA_STS_COMPLETE)
		{
			//printk("\n>>>>>>> nDMA Read Page(%d) Completed!", page);
			break;
		}
		else if((g_rnDmaStat & DRIME4_NDMA_STS_ECCERR) == DRIME4_NDMA_STS_ECCERR)
		{
			ret = drime4_nanddma_err_correct(mtd, chip, g_rnDmaStat);

			if (ret == DRIME4_NDMA_ECC_UNCORRECT)
			{
				cont = (DRIME4_NDMA_CTL_INTCLR | DRIME4_NDMA_CTL_STOP);
				__raw_writel(cont, info->regs_ndma + DRIME4_NDMA_CONT);

				return -EIO;

			}
			
			g_rnDmaStat = 0;
			
			// NFC Setting ------------------------------------------------------------------------------------------------------------------------------
			conf = __raw_readl(info->regs + DRIME4_NFCONF);
			conf |= (DRIME4_NFCONF_MLC_CLKCON |DRIME4_NFCONF_ECC_TYPE(DRIME4_NFECC_MLC4BIT));
			conf &= ~(DRIME4_NFCONF_MLC_CLKCON);
			__raw_writel(conf, info->regs + DRIME4_NFCONF);
			
			cont = __raw_readl(info->regs + DRIME4_NFCONT);
			cont |= (DRIME4_NFCONT_ECCDE_INT|DRIME4_NFCONT_ILLEGALACC_EN|DRIME4_NFCONT_RNBINT_EN|DRIME4_NFCONT_INITMECC);
			cont &= ~(DRIME4_NFCONT_ECCDIR_ENC|DRIME4_NFCONT_ECCEN_INT|DRIME4_NFCONT_RN_FALLING|DRIME4_NFCONT_MAIN_ECCLOCK);
			__raw_writel(cont, info->regs + DRIME4_NFCONT);

			// nDMA Setting -----------------------------------------------------------------------------------------------------------------------------
			cont = (DRIME4_NDMA_CTL_INTCLR | DRIME4_NDMA_CTL_RESTART);
			__raw_writel(cont, info->regs_ndma + DRIME4_NDMA_CONT);
			
		}
		else
		{}

	}

	#if 0
	if(page >= 5312 && page <= 5316)
	{
		uint32_t nfcReg[17];

		nfcReg[0] = __raw_readl(info->regs + DRIME4_NFCONF);
		nfcReg[1] = __raw_readl(info->regs + DRIME4_NFCONT);
		nfcReg[2] = __raw_readl(info->regs + DRIME4_NFCMD);
		nfcReg[3] = __raw_readl(info->regs + DRIME4_NFADDR);
		nfcReg[4] = __raw_readl(info->regs + DRIME4_NFDATA);
		nfcReg[5] = __raw_readl(info->regs + DRIME4_NFECCD0);
		nfcReg[6] = __raw_readl(info->regs + DRIME4_NFECCD1);
		nfcReg[7] = __raw_readl(info->regs + DRIME4_NFECCD);
		nfcReg[8] = __raw_readl(info->regs + DRIME4_NFSBLK);
		nfcReg[9] = __raw_readl(info->regs + DRIME4_NFEBLK);
		nfcReg[10] = __raw_readl(info->regs + DRIME4_NFSTAT);
		nfcReg[11] = __raw_readl(info->regs + DRIME4_NFMECC_ERR0);
		nfcReg[12] = __raw_readl(info->regs + DRIME4_NFMECC_ERR1);
		nfcReg[13] = __raw_readl(info->regs + DRIME4_NFMECC0);
		nfcReg[14] = __raw_readl(info->regs + DRIME4_NFMECC1);
		nfcReg[15] = __raw_readl(info->regs + DRIME4_NFSECC);
		nfcReg[16] = __raw_readl(info->regs + DRIME4_NFMLCBITPT);

		vic1IrqStat = __raw_readl(DRIME4_VA_VIC1 + 0x0);
		vic1RawStat = __raw_readl(DRIME4_VA_VIC1 + 0x08);
		vic1enb = __raw_readl(DRIME4_VA_VIC1 + 0x10);
	
		dbgStat = __raw_readl(info->regs_ndma + DRIME4_NDMA_STS);
		dbgPg = __raw_readl(info->regs_ndma + DRIME4_NDMA_ROW_ADDR);
		dbgSct = __raw_readl(info->regs_ndma + DRIME4_NDMA_SCT_COUNT);
		
		stat = __raw_readl(info->regs_ndma + DRIME4_NDMA_INT_STS);

		//__cpuc_flush_dcache_area((void *)buf, 2048);
			
		//printk("\n>>>>>>> nDMA Res [buf:0x%x, pgs:%d, stat:0x%x, dat:0x%x, mon(stat:0x%x, pg:%d, sct:%d), INT(enb:0x%x, stat:0x%x, rawSt:0x%x)]", buf, page, stat, *(uint32_t *)buf, dbgStat, dbgPg, dbgSct, vic1enb, vic1IrqStat, vic1RawStat);
		printk("\n>>>>>>> nDMA Res [buf:0x%x, pgs:%d, stat:0x%x, dat(s):0x%x, dat(e):0x%x, INT(enb:0x%x, stat:0x%x, rawSt:0x%x)]", g_nand_dma_vaddr, page, stat, *(uint32_t *)g_nand_dma_vaddr, *(uint32_t *)(g_nand_dma_vaddr+2043), vic1enb, vic1IrqStat, vic1RawStat);
		printk("\n>>>>>>> rNFC Get [cfg:0x%x, ctl:0x%x, cmd:0x%x, adr:0x%x, dat:0x%x, meccd0:0x%x, meccd1:0x%x, seccd:0x%x]", nfcReg[0], nfcReg[1], nfcReg[2], nfcReg[3], nfcReg[4], nfcReg[5], nfcReg[6], nfcReg[7]);
		printk("\n>>>>>>> rNFC Get [sblk:0x%x, eblk:0x%x, stat:0x%x, eccer0:0x%x, eccer1:0x%x, gecc0:0x%x, gecc1:0x%x, geccs:0x%x, eccptn:0x%x]\n", nfcReg[8], nfcReg[9], nfcReg[10], nfcReg[11], nfcReg[12], nfcReg[13], nfcReg[14], nfcReg[15], nfcReg[16]);

		
	}
	#endif
	
	cont = (DRIME4_NDMA_CTL_INTCLR | DRIME4_NDMA_CTL_STOP);
	__raw_writel(cont, info->regs_ndma + DRIME4_NDMA_CONT);

	memcpy(buf, g_nand_dma_vaddr, 2048);	

	return ret;

}
#endif

/**
 * drime4_nand_read_oob_std - [REPLACABLE] the most common OOB data read function
 * @mtd:	mtd info structure
 * @chip:	nand chip info structure
 * @page:	page number to read
 * @sndcmd:	flag whether to issue read command or not
 */
static int drime4_nand_read_oob_std(struct mtd_info *mtd, struct nand_chip *chip,
			     int page, int sndcmd)
{
	if (sndcmd) {
		chip->cmdfunc(mtd, NAND_CMD_READOOB, 0, page);
		sndcmd = 0;
	}
	chip->read_buf(mtd, chip->oob_poi, mtd->oobsize);
	return sndcmd;
}

/**
 * drime4_nand_read_page_hwecc - hardware ecc based page read function
 * @mtd:	mtd info structure
 * @chip:	nand chip info structure
 * @buf: buffer to store read data
 * @oob_required: caller requires OOB data read to chip->oob_poi
 * @page: page number to read
 *
 * Not for syndrome calculating ecc controllers which need a special oob layout
 */
static int drime4_nand_read_page_hwecc(struct mtd_info *mtd, struct nand_chip *chip,
				uint8_t *buf, int oob_required, int page)
{
	struct drime4_nand_info *info = drime4_nand_mtd_toinfo(mtd);
	uint8_t *read_ecc;
	uint8_t *calc_ecc;

	int i, col_oob, col_data;

	/*uint8_t *p = buf;*/
	uint32_t parity[2];

	int sctsize =  512;
	int sctperpg = chip->ecc.size / sctsize;

	uint32_t ctrl;
	uint32_t stat;
	uint32_t meccerr[2];
	int err;

	col_data = 0;
	col_oob = (int)(mtd->ecclayout->eccpos[0]);

	chip->ecc.hwctl(mtd, NAND_ECC_READ);

	for (i = 0 ; i < sctperpg ; i++) {

		dev_dbg(info->device, "page:%d, sector:%04x, col_data:%08x, buf:%08x\n",
				page, i, col_data, buf+col_data);

		/*drime4_nand_start_hwmecc(mtd);*/
		ctrl = __raw_readl(info->regs + DRIME4_NFCONT);
		ctrl &= ~DRIME4_NFCONT_MAIN_ECCLOCK;
		__raw_writel(ctrl | DRIME4_NFCONT_INITMECC, info->regs + DRIME4_NFCONT);

		/* read 512(1sector)byte */
		chip->read_buf(mtd, (uint8_t *)(buf + col_data), sctsize);


		/* read 8byte(parity data of 1 sector) in spare area*/
		chip->cmdfunc(mtd, NAND_CMD_RNDOUT, mtd->writesize + col_oob, -1);
		chip->read_buf(mtd, (uint8_t *)parity, 8);

		/* Nand Main ECC Lock */
		ctrl = __raw_readl(info->regs + DRIME4_NFCONT);
		__raw_writel(ctrl | DRIME4_NFCONT_MAIN_ECCLOCK, info->regs + DRIME4_NFCONT);
		/* check ecc encord done */
		while (1) {
			stat = __raw_readl(info->regs + DRIME4_NFSTAT);
			if (stat & DRIME4_NFSTAT_ECC_DECDONE)
				break;
		}
		/* clear state */
		__raw_writel(stat | DRIME4_NFSTAT_ECC_DECDONE, info->regs + DRIME4_NFSTAT);

		meccerr[0] = __raw_readl(info->regs + DRIME4_NFMECC_ERR0);
		meccerr[1] = __raw_readl(info->regs + DRIME4_NFMECC_ERR1);

		dev_dbg(info->device, "sct:%d, col_data:%04x, buf:%08x, col_oob:%02d, meccerr1 meccerr0:%08x %08x, parity:%08x %08x\n",
				i, col_data, buf+col_data, col_oob, meccerr[1], meccerr[0], parity[1], parity[0]);

		/*
		ecc check then correct
		*/
		if ((meccerr[0]>>29) & 0x01 == 0x01) {
			/*
			dev_dbg(info->device, " No ERROR\n");
			*/
		} else if (((meccerr[0] >> 26) & 0x7) == 0x0) {
			/*
			dev_dbg(info->device, " No ERROR\n");
			*/
		} else if (((meccerr[0] >> 26) & 0x7) < 5) {
			/*
			dev_dbg(info->device, " %dpage Correctable Error\n",page);
			*/
			err = chip->ecc.correct(mtd, (uint8_t *)(buf + col_data), read_ecc, calc_ecc);
			if (err < 0) {
				return -EIO;
			}

			/*
			dev_dbg(info->device, " Error Corrected\n");
			*/
		} else {
			dev_err(info->device, " %dpage Uncorrectable Error\n", page);
			return -EIO;
		}

		/* over 1 page, exit loop */
		if (col_data >= mtd->writesize)
			break;

		/*p += sctsize;*/
		col_data += sctsize;
		col_oob += 8;

		/* Command latch cycle */
		chip->cmdfunc(mtd, NAND_CMD_READ0, col_data, page);

	}

	return 0;
}

static int drime4_nand_read_page_8b_hwecc(struct mtd_info *mtd, struct nand_chip *chip,
				uint8_t *buf, int oob_required, int page)
{
	struct drime4_nand_info *info = drime4_nand_mtd_toinfo(mtd);
	uint8_t *read_ecc;
	uint8_t *calc_ecc;

	int i, col_oob, col_data;

	/*uint8_t *p = buf;*/
	uint32_t parity[4];

	int sctsize =  512;
	int sctperpg = chip->ecc.size / sctsize;

	uint32_t ctrl;
	uint32_t stat;
	uint32_t meccerr[3];
	int err;

	col_data = 0;
	col_oob = (int)(mtd->ecclayout->eccpos[0]);

	chip->ecc.hwctl(mtd, NAND_ECC_READ);

	for (i = 0 ; i < sctperpg ; i++) {

		dev_dbg(info->device, "page:%d, sector:%04x, col_data:%08x, buf:%08x\n",
				page, i, col_data, buf+col_data);

		/*drime4_nand_start_hwmecc(mtd);*/
		ctrl = __raw_readl(info->regs + DRIME4_NFCONT);
		ctrl &= ~DRIME4_NFCONT_MAIN_ECCLOCK;
		__raw_writel(ctrl | DRIME4_NFCONT_INITMECC, info->regs + DRIME4_NFCONT);

		/* read 512(1sector)byte */
		chip->read_buf(mtd, (uint8_t *)(buf + col_data), sctsize);


		/* read 8byte(parity data of 1 sector) in spare area*/
		chip->cmdfunc(mtd, NAND_CMD_RNDOUT, mtd->writesize + col_oob, -1);
 		chip->read_buf(mtd, (uint8_t *)parity, OFFSET_SCT_PARITY);

		/* Nand Main ECC Lock */
		ctrl = __raw_readl(info->regs + DRIME4_NFCONT);
		__raw_writel(ctrl | DRIME4_NFCONT_MAIN_ECCLOCK, info->regs + DRIME4_NFCONT);

		/* check ecc encord done */
		while (1) {
			stat = __raw_readl(info->regs + DRIME4_NFSTAT);
			if (stat & DRIME4_NFSTAT_ECC_DECDONE)
				break;
		}
		/* clear state */
		__raw_writel(stat | DRIME4_NFSTAT_ECC_DECDONE, info->regs + DRIME4_NFSTAT);

		meccerr[0] = __raw_readl(info->regs + DRIME4_NF8ECCERR0);
		meccerr[1] = __raw_readl(info->regs + DRIME4_NF8ECCERR1);
		meccerr[2] = __raw_readl(info->regs + DRIME4_NF8ECCERR2);

		dev_dbg(info->device, "sct:%d, col_data:%04x, buf:%08x, col_oob:%02d, meccerr0/1/2:%08x %08x %08x, parity:%08x %08x %08x %08x\n",
				i, col_data, buf+col_data, col_oob, meccerr[0], meccerr[1], meccerr[2],
				parity[0], parity[1], parity[2], parity[3]);
#if 0
		printk("___sct:%d, col_data:%04x, col_oob:%02d, meccerr0/1/2:%08x %08x %08x, parity:%08x %08x %08x %08x\n",
				i, col_data, col_oob, meccerr[0], meccerr[1], meccerr[2],
				parity[0], parity[1], parity[2], parity[3]);
#endif

		/*
		ecc check then correct
		*/
		if ((meccerr[0]>>29) & 0x01 == 0x01) {
			/*
			dev_dbg(info->device, " No ERROR\n");
			*/
		} else if (((meccerr[0] >> 25) & 0xf) == 0x0) {
			/*
			dev_dbg(info->device, " No ERROR\n");
			*/
		} else if (((meccerr[0] >> 25) & 0xf) < 9) {
			/*
			dev_dbg(info->device, " %dpage Correctable Error\n",page);
			*/
			err = chip->ecc.correct(mtd, (uint8_t *)(buf + col_data), read_ecc, calc_ecc);
			if (err < 0) {
				return -EIO;
			}

			/*
			dev_dbg(info->device, " Error Corrected\n");
			*/
		} else if (((meccerr[0] >> 25) & 0xf) > 8){
//			printk("%dpage, oob:%d \n", page, mtd->writesize + col_oob);
			dev_err(info->device, "___ Uncorrectable Error\n");
			return -EIO;
		}

		/* over 1 page, exit loop */
		if (col_data >= mtd->writesize)
			break;

		/*p += sctsize;*/
		col_data += sctsize;
		col_oob += OFFSET_SCT_PARITY;

		/* Command latch cycle */
		chip->cmdfunc(mtd, NAND_CMD_READ0, col_data, page);

	}

	return 0;
}

/**
 * drime4_nand_write_oob_std - [REPLACABLE] the most common OOB data write function
 * @mtd:	mtd info structure
 * @chip:	nand chip info structure
 * @page:	page number to write
 */
static int drime4_nand_write_oob_std(struct mtd_info *mtd, struct nand_chip *chip,
			      int page)
{
	int status = 0;
	const uint8_t *buf = chip->oob_poi;
	int length = mtd->oobsize;

	chip->cmdfunc(mtd, NAND_CMD_RNDIN, mtd->writesize, page);
	chip->write_buf(mtd, buf, length);
	/* Send command to program the OOB data */
	chip->cmdfunc(mtd, NAND_CMD_PAGEPROG, -1, -1);

	status = chip->waitfunc(mtd, chip);

	return status & NAND_STATUS_FAIL ? -EIO : 0;
}

/**
 * drime4_nand_write_oob_ecc - ecc parity data write to oob area
 * @mtd:	mtd info structure
 * @chip:	nand chip info structure
 */
static int drime4_nand_write_oob_ecc(struct mtd_info *mtd, struct nand_chip *chip)
{
	int status = 0;
	const uint8_t *buf = chip->oob_poi;
	int length = chip->ecc.total;
	int column = (int)(mtd->ecclayout->eccpos[0]);

//	printk("__write_oob pos:%d+%d, buf:%d  length:%d\n", mtd->writesize, column,buf,  length);
//	printk("__%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
//		chip->oob_poi[0],chip->oob_poi[1],chip->oob_poi[2],chip->oob_poi[3],
//		chip->oob_poi[4],chip->oob_poi[5],chip->oob_poi[6],chip->oob_poi[7],
//		chip->oob_poi[8],chip->oob_poi[9],chip->oob_poi[10],chip->oob_poi[11],
//		chip->oob_poi[12],chip->oob_poi[13],chip->oob_poi[14],chip->oob_poi[15]);

	chip->cmdfunc(mtd, NAND_CMD_RNDIN, mtd->writesize + column, -1);
	chip->write_buf(mtd, buf+column, length);
	/* Send command to program the OOB data */
	chip->cmdfunc(mtd, NAND_CMD_PAGEPROG, -1, -1);

	status = chip->waitfunc(mtd, chip);

	return status & NAND_STATUS_FAIL ? -EIO : 0;
}

/**
 * drime4_nand_write_page_hwecc - hardware ecc based page write function
 * @mtd:	mtd info structure
 * @chip:	nand chip info structure
 * @buf:	data buffer
 */
static void drime4_nand_write_page_hwecc(struct mtd_info *mtd, struct nand_chip *chip,
				  const uint8_t *buf, int oob_required)
{
	struct drime4_nand_info *info = drime4_nand_mtd_toinfo(mtd);
	int i;
	const uint8_t *p = buf;
	uint8_t *ecc_calc = chip->buffers->ecccalc;
	uint32_t *eccpos = chip->ecc.layout->eccpos;

	int sctsize =  512;
	int sctperpg = chip->ecc.size / sctsize;
	int cnt;

	uint32_t ctrl;
	uint32_t stat;
	uint32_t mecc0;
	uint32_t mecc1;

	dev_dbg(info->device, "drime4_nand_write_page_hwecc()\n");

	for (i = 0; i < sctperpg; i++, p += sctsize) {

		/*drime4_nand_start_hwmecc(mtd);*/
		ctrl = __raw_readl(info->regs + DRIME4_NFCONT);
		ctrl &= ~DRIME4_NFCONT_MAIN_ECCLOCK;
		__raw_writel(ctrl | DRIME4_NFCONT_INITMECC, info->regs + DRIME4_NFCONT);

		chip->write_buf(mtd, p, sctsize);

		mecc0 = __raw_readl(info->regs + DRIME4_NFMECC0);
		mecc1 = __raw_readl(info->regs + DRIME4_NFMECC1);

		/* Nand Main ECC Lock */
		ctrl = __raw_readl(info->regs + DRIME4_NFCONT);
		__raw_writel(ctrl | DRIME4_NFCONT_MAIN_ECCLOCK, info->regs + DRIME4_NFCONT);

		/* check ecc encord done & clear*/
		while (1) {
			stat = __raw_readl(info->regs + DRIME4_NFSTAT);
			if (stat & DRIME4_NFSTAT_ECC_ENCDONE)
				break;
		}
		__raw_writel(stat | DRIME4_NFSTAT_ECC_ENCDONE, info->regs + DRIME4_NFSTAT);

		ecc_calc[i*8+0] = (uint8_t)((mecc0 >> 0) & 0xff);
		ecc_calc[i*8+1] = (uint8_t)((mecc0 >> 8) & 0xff);
		ecc_calc[i*8+2] = (uint8_t)((mecc0 >> 16) & 0xff);
		ecc_calc[i*8+3] = (uint8_t)((mecc0 >> 24) & 0xff);
		ecc_calc[i*8+4] = (uint8_t)((mecc1 >> 0) & 0xff);
		ecc_calc[i*8+5] = (uint8_t)((mecc1 >> 8) & 0xff);
		ecc_calc[i*8+6] = (uint8_t)((mecc1 >> 16) & 0xff);
		ecc_calc[i*8+7] = (uint8_t)((mecc1 >> 24) & 0xff);

		dev_dbg(info->device, "sct:%d,p:%08lx,mecc1/mecc0:%08lx %08lx\n",
				i, p, mecc1, mecc0);

	}

	/* write ecc parity data onto oob(spare) area
	*/
	for (i = 0; i < chip->ecc.total; i++)
		chip->oob_poi[eccpos[i]] = ecc_calc[i];

	/* ecc parity write */
	drime4_nand_write_oob_ecc(mtd, chip);

	/**
	for(i=0;i<mtd->oobsize;i++) {
		if(i%16 == 0) printk("		  [%2d:%2d] ",i, i+15);
		printk("%02x",chip->oob_poi[i]);
		if(i%4 == 3) printk(" ");
		if(i%16 == 15) printk("\n");
	}
	*/

}

static void drime4_nand_write_page_8b_hwecc(struct mtd_info *mtd, struct nand_chip *chip,
				  const uint8_t *buf, int oob_required)
{
	struct drime4_nand_info *info = drime4_nand_mtd_toinfo(mtd);
	int i;
	const uint8_t *p = buf;
	uint8_t *ecc_calc = chip->buffers->ecccalc;
	uint32_t *eccpos = chip->ecc.layout->eccpos;

	int sctsize =  512;
	int sctperpg = chip->ecc.size / sctsize;
	int cnt;

	uint32_t ctrl;
	uint32_t stat;
	uint32_t mecc0;
	uint32_t mecc1;
	uint32_t mecc2;
	uint32_t mecc3;

	dev_dbg(info->device, "drime4_nand_write_page_hwecc()\n");

	for (i = 0; i < sctperpg; i++, p += sctsize) {

		/*drime4_nand_start_hwmecc(mtd);*/
		ctrl = __raw_readl(info->regs + DRIME4_NFCONT);
		ctrl &= ~DRIME4_NFCONT_MAIN_ECCLOCK;
		__raw_writel(ctrl | DRIME4_NFCONT_INITMECC, info->regs + DRIME4_NFCONT);

		chip->write_buf(mtd, p, sctsize);

		mecc0 = __raw_readl(info->regs + DRIME4_NFM8ECC0);
		mecc1 = __raw_readl(info->regs + DRIME4_NFM8ECC1);
		mecc2 = __raw_readl(info->regs + DRIME4_NFM8ECC2);
		mecc3 = __raw_readl(info->regs + DRIME4_NFM8ECC3);

		/* Nand Main ECC Lock */
		ctrl = __raw_readl(info->regs + DRIME4_NFCONT);
		__raw_writel(ctrl | DRIME4_NFCONT_MAIN_ECCLOCK, info->regs + DRIME4_NFCONT);

		/* check ecc encord done & clear*/
		while (1) {
			stat = __raw_readl(info->regs + DRIME4_NFSTAT);
			if (stat & DRIME4_NFSTAT_ECC_ENCDONE)
				break;
		}
		__raw_writel(stat | DRIME4_NFSTAT_ECC_ENCDONE, info->regs + DRIME4_NFSTAT);

		ecc_calc[i*OFFSET_SCT_PARITY+0] = (uint8_t)((mecc0 >> 0) & 0xff);
		ecc_calc[i*OFFSET_SCT_PARITY+1] = (uint8_t)((mecc0 >> 8) & 0xff);
		ecc_calc[i*OFFSET_SCT_PARITY+2] = (uint8_t)((mecc0 >> 16) & 0xff);
		ecc_calc[i*OFFSET_SCT_PARITY+3] = (uint8_t)((mecc0 >> 24) & 0xff);
		ecc_calc[i*OFFSET_SCT_PARITY+4] = (uint8_t)((mecc1 >> 0) & 0xff);
		ecc_calc[i*OFFSET_SCT_PARITY+5] = (uint8_t)((mecc1 >> 8) & 0xff);
		ecc_calc[i*OFFSET_SCT_PARITY+6] = (uint8_t)((mecc1 >> 16) & 0xff);
		ecc_calc[i*OFFSET_SCT_PARITY+7] = (uint8_t)((mecc1 >> 24) & 0xff);
		ecc_calc[i*OFFSET_SCT_PARITY+8] = (uint8_t)((mecc2 >> 0) & 0xff);
		ecc_calc[i*OFFSET_SCT_PARITY+9] = (uint8_t)((mecc2 >> 8) & 0xff);
		ecc_calc[i*OFFSET_SCT_PARITY+10] = (uint8_t)((mecc2 >> 16) & 0xff);
		ecc_calc[i*OFFSET_SCT_PARITY+11] = (uint8_t)((mecc2 >> 24) & 0xff);
		ecc_calc[i*OFFSET_SCT_PARITY+12] = (uint8_t)((mecc3 >> 0) & 0xff);
		ecc_calc[i*OFFSET_SCT_PARITY+13] = (uint8_t)((mecc3 >> 8) & 0xff);
		ecc_calc[i*OFFSET_SCT_PARITY+14] = (uint8_t)((mecc3 >> 16) & 0xff);
		ecc_calc[i*OFFSET_SCT_PARITY+15] = (uint8_t)((mecc3 >> 24) & 0xff);

		dev_dbg(info->device, "___sct:%d,p:%08lx,mecc0/1/2/3:%08lx %08lx %08lx %08lx\n",
				i, p, mecc0, mecc1, mecc2, mecc3);
//		printk("___sct:%d,p:%08lx,mecc0/1/2/3:%08lx %08lx %08lx %08lx\n",
//				i, p, mecc0, mecc1, mecc2, mecc3);

	}

	/* write ecc parity data onto oob(spare) area
	*/
	for (i = 0; i < chip->ecc.total; i++)
		chip->oob_poi[eccpos[i]] = ecc_calc[i];
	//	printk("oob_poi[eccpos[%d]]:%d   ",i, chip->oob_poi[eccpos[i]]);

	/* ecc parity write */
	drime4_nand_write_oob_ecc(mtd, chip);

	/**
	for(i=0;i<mtd->oobsize;i++) {
		if(i%16 == 0) printk("        [%2d:%2d] ",i, i+15);
		printk("%02x",chip->oob_poi[i]);
		if(i%4 == 3) printk(" ");
		if(i%16 == 15) printk("\n");
	}
	*/

}

/**
 * drime4_nand_write_page - [REPLACEABLE] write one page
 * @mtd:	MTD device structure
 * @chip:	NAND chip descriptor
 * @buf:	the data to write
 * @page:	page number to write
 * @cached:	cached programming
 * @raw:	use _raw version of write_page
 */
static int drime4_nand_write_page(struct mtd_info *mtd, struct nand_chip *chip,
				const uint8_t *buf, int oob_required, int page,
				int cached, int raw)
{
	int status;

	chip->ecc.hwctl(mtd, NAND_ECC_WRITE);
	chip->cmdfunc(mtd, NAND_CMD_SEQIN, 0, page);

	if (unlikely(raw))
		chip->ecc.write_page_raw(mtd, chip, buf, oob_required);
	else
		chip->ecc.write_page(mtd, chip, buf, oob_required);

	/*
	 * Cached progamming disabled for now, Not sure if its worth the
	 * trouble. The speed gain is not very impressive. (2.3->2.6Mib/s)
	 */
	cached = 0;

	if (!cached || !(chip->options & NAND_CACHEPRG)) {

		chip->cmdfunc(mtd, NAND_CMD_PAGEPROG, -1, -1);
		status = chip->waitfunc(mtd, chip);
		/*
		 * See if operation failed and additional status checks are
		 * available
		 */
		if ((status & NAND_STATUS_FAIL) && (chip->errstat))
			status = chip->errstat(mtd, chip, FL_WRITING, status,
					       page);

		if (status & NAND_STATUS_FAIL)
			return -EIO;
	} else {
		chip->cmdfunc(mtd, NAND_CMD_CACHEDPROG, -1, -1);
		status = chip->waitfunc(mtd, chip);
	}

#ifdef CONFIG_MTD_NAND_VERIFY_WRITE
	/* Send command to read back the data */
	chip->cmdfunc(mtd, NAND_CMD_READ0, 0, page);

	if (chip->verify_buf(mtd, buf, mtd->writesize))
		return -EIO;
#endif
	return 0;
}


/* cpufreq driver support */

#ifdef CONFIG_CPU_FREQ

static int drime4_nand_cpufreq_transition(struct notifier_block *nb,
					  unsigned long val, void *data)
{
	struct drime4_nand_info *info;
	unsigned long newclk;

	info = container_of(nb, struct drime4_nand_info, freq_transition);
	newclk = clk_get_rate(info->clk);

	if ((val == CPUFREQ_POSTCHANGE && newclk < info->clk_rate) ||
	    (val == CPUFREQ_PRECHANGE && newclk > info->clk_rate)) {
		drime4_nand_setrate(info);
	}

	return 0;
}

static inline int drime4_nand_cpufreq_register(struct drime4_nand_info *info)
{
	info->freq_transition.notifier_call = drime4_nand_cpufreq_transition;

	return cpufreq_register_notifier(&info->freq_transition,
					 CPUFREQ_TRANSITION_NOTIFIER);
}

static inline void drime4_nand_cpufreq_deregister(struct drime4_nand_info *info)
{
	cpufreq_unregister_notifier(&info->freq_transition,
				    CPUFREQ_TRANSITION_NOTIFIER);
}

#else
static inline int drime4_nand_cpufreq_register(struct drime4_nand_info *info)
{
	return 0;
}

static inline void drime4_nand_cpufreq_deregister(struct drime4_nand_info *info)
{
}
#endif

/* device management functions */

static int drime4_nand_remove(struct platform_device *pdev)
{
	struct drime4_nand_info *info = to_nand_info(pdev);

	platform_set_drvdata(pdev, NULL);

	if (info == NULL)
		return 0;

	drime4_nand_cpufreq_deregister(info);

	/* Release all our mtds  and their partitions, then go through
	 * freeing the resources used
	 */

	if (info->mtds != NULL) {
		struct drime4_nand_mtd *ptr = info->mtds;
		int mtdno;

		for (mtdno = 0; mtdno < info->mtd_count; mtdno++, ptr++) {
			pr_debug("releasing mtd %d (%p)\n", mtdno, ptr);
			nand_release(&ptr->mtd);
		}

		kfree(info->mtds);
	}

	/* free the common resources */

	if (info->clk != NULL && !IS_ERR(info->clk)) {
		if (!allow_clk_stop(info))
			clk_disable(info->clk);
		clk_put(info->clk);
	}

	if (info->regs != NULL) {
		iounmap(info->regs);
		info->regs = NULL;
	}

	if (info->regs_ndma != NULL) {
		iounmap(info->regs_ndma);
		info->regs_ndma = NULL;
	}

	if (info->area != NULL) {
		release_resource(info->area);
		kfree(info->area);
		info->area = NULL;
	}

#ifdef __USE_NAND_DMA_READ__
	if (g_nand_dma_vaddr)
	{
		if (d4_bma_free_buf(g_nand_dma_paddr) < 0)
		{
			printk("nDMA buffer dealloc ERROR!\n");
			return -1;
		}

		g_nand_dma_paddr =0;
		g_nand_dma_vaddr = 0;
	}

	#ifdef __NDMA_INT_ISR_ENALBE__
	free_irq(g_inDmaIrq, pdev);
	#endif

#endif

	kfree(info);

	return 0;
}

static int drime4_nand_add_partition(struct drime4_nand_info *info,
				      struct drime4_nand_mtd *mtd,
				      struct drime4_nand_set *set)
{
        if (set == NULL) 
                return add_mtd_device(&mtd->mtd);

	if (set)
		mtd->mtd.name = set->name;

	return mtd_device_parse_register(&mtd->mtd, NULL, NULL,
					 set->partitions, set->nr_partitions);
}



/**
 * drime4_nand_init_chip - initialise a single instance of an chip
 * @info: The base NAND controller the chip is on.
 * @nmtd: The new controller MTD instance to fill in.
 * @set: The information passed from the board specific platform data.
 *
 * Initialise the given @nmtd from the information in @info and @set. This
 * readies the structure for use with the MTD layer functions by ensuring
 * all pointers are setup and the necessary control routines selected.
 */
static void drime4_nand_init_chip(struct drime4_nand_info *info,
				   struct drime4_nand_mtd *nmtd,
				   struct drime4_nand_set *set)
{
	struct nand_chip *chip = &nmtd->chip;
	void __iomem *regs = info->regs;

	chip->write_buf    = drime4_nand_write_buf;
	chip->read_buf     = drime4_nand_read_buf;
	chip->select_chip  = drime4_nand_select_chip;
	chip->chip_delay   = 10;
	chip->priv	   = nmtd;
	chip->options	   = set->options;
	chip->controller   = &info->controller;

	chip->IO_ADDR_R = regs + DRIME4_NFDATA;
	chip->IO_ADDR_W = regs + DRIME4_NFDATA;
	info->sel_reg   = regs + DRIME4_NFCONT;
	info->sel_bit	= DRIME4_NFCONT_nCE;

	chip->cmd_ctrl  = drime4_nand_hwcontrol;
	chip->dev_ready = drime4_nand_devready;

	if (__raw_readl(regs + DRIME4_NFCONF) & DRIME4_NFCONF_NANDBOOT)
		dev_info(info->device, "System booted from NAND\n");

	nmtd->info	   = info;
	nmtd->mtd.priv	   = chip;
	nmtd->mtd.owner    = THIS_MODULE;
	nmtd->set	   = set;

	if (hardware_ecc) {
		chip->ecc.mode	    = NAND_ECC_HW;
#if defined(CONFIG_MTD_NAND_DRIME4_8BIT_ECC)
		chip->ecc.correct   = drime4_nand_correct_data_8b_ecc;
		chip->ecc.hwctl     = drime4_nand_enable_8b_hwecc;
		chip->ecc.calculate = drime4_nand_calculate_8b_ecc;
		chip->ecc.strength	= 8;	/* 8bit correctable every 512byte */
#else
		chip->ecc.correct   = drime4_nand_correct_data;
		chip->ecc.hwctl     = drime4_nand_enable_hwecc;
		chip->ecc.calculate = drime4_nand_calculate_ecc;
		chip->ecc.strength = 4;	/* 4bit correctable every 512byte */
#endif

	} else {
		chip->ecc.mode	    = NAND_ECC_SOFT;
	}

	if (set->ecc_layout != NULL)
		chip->ecc.layout = set->ecc_layout;

	if (set->disable_ecc)
		chip->ecc.mode	= NAND_ECC_NONE;

	switch (chip->ecc.mode) {
	case NAND_ECC_NONE:
		dev_info(info->device, "NAND ECC disabled\n");
		break;
	case NAND_ECC_SOFT:
		dev_info(info->device, "NAND soft ECC\n");
		break;
	case NAND_ECC_HW:
		dev_info(info->device, "NAND hardware ECC\n");
		break;
	default:
		dev_info(info->device, "NAND ECC UNKNOWN\n");
		break;
	}

	/* If you use u-boot BBT creation code, specifying this flag will
	 * let the kernel fish out the BBT from the NAND, and also skip the
	 * full NAND scan that can take 1/2s or so. Little things... */
	if (set->flash_bbt) {
		chip->bbt_options |= NAND_BBT_USE_FLASH;
		chip->options |= NAND_SKIP_BBTSCAN;
	}
}

/**
 * drime4_nand_update_chip - post probe update
 * @info: The controller instance.
 * @nmtd: The driver version of the MTD instance.
 *
 * This routine is called after the chip probe has successfully completed
 * and the relevant per-chip information updated. This call ensure that
 * we update the internal state accordingly.
 *
 * The internal state is currently limited to the ECC state information.
*/
static void drime4_nand_update_chip(struct drime4_nand_info *info,
				     struct drime4_nand_mtd *nmtd)
{
	struct nand_chip *chip = &nmtd->chip;

	dev_dbg(info->device, "chip %p => page shift %d\n",
		chip, chip->page_shift);

	if (chip->ecc.mode != NAND_ECC_HW)
		return;

	/* change the behaviour depending on wether we are using
	 * the large or small page nand device */

	if (chip->page_shift > 11) {
		chip->ecc.size	    = 4096;
		chip->ecc.bytes	    = OFFSET_SCT_PARITY*NUM_SECTOR;
		chip->ecc.layout    = &nand_4K_hw_eccoob;
	} else if (chip->page_shift == 11) {
		chip->ecc.size	    = 2048;
		chip->ecc.bytes	    = 8*4;
		chip->ecc.layout    = &nand_2K_hw_eccoob;
	} else {
		chip->ecc.size	    = 512;
		chip->ecc.bytes	    = 4;
		chip->ecc.layout    = &nand_hw_eccoob;
	}

}


static int nand_register_iomap(struct platform_device *pdev,
		struct drime4_nand_info *nand_info, int res_num)
{
	struct resource *res = NULL;
	void __iomem *regs;
	int ret = 0;

	/* get resource for io memory  */
	res = platform_get_resource(pdev, IORESOURCE_MEM, res_num);
	if (!res) {
		dev_err(&pdev->dev, "failed to get io memory region\n");
		ret = -ENODEV;
		goto out;
	}

	/* request mem region  */
	res = request_mem_region(res->start, resource_size(res), pdev->name);
	if (!res) {
		dev_err(&pdev->dev, "failed to request io memory region\n");
		ret = -ENOMEM;
		goto out;
	}

	/* ioremap for register block */
	regs = ioremap(res->start, resource_size(res));
	if (!regs) {
		dev_err(&pdev->dev, "failed to remap io region\n");
		ret = -ENOMEM;
		goto err_no_ioremap;
	}

	if (res_num == 0) {
		nand_info->regs   = (unsigned int)regs;
	} else if (res_num == 1) {
		nand_info->regs_ndma = (unsigned int)regs;
	}
	return 0;

err_no_ioremap:
	release_mem_region(res->start, resource_size(res));
out:
	return ret;
}

/* drime4_nand_probe
 *
 * called by device layer when it finds a device matching
 * one our driver can handled. This code checks to see if
 * it can allocate all necessary resources then calls the
 * nand layer to look for devices
*/
static int drime4_nand_probe(struct platform_device *pdev)
{
	struct drime4_platform_nand *plat = to_nand_plat(pdev);
	struct drime4_nand_info *info;
	struct drime4_nand_mtd *nmtd;
	struct drime4_nand_set *sets;
	struct resource *res;
	struct resource *res_ndma;
	struct BMA_Buffer_Info buf_info;
	int err = 0;
	int size, size_ndma;
	int nr_sets;
	int setno;
	int ret;

	pr_debug("drime4_nand_probe(%p)\n", pdev);

	info = kzalloc(sizeof(*info), GFP_KERNEL);
	if (info == NULL) {
		dev_err(&pdev->dev, "no memory for flash info\n");
		err = -ENOMEM;
		goto exit_error;
	}
	platform_set_drvdata(pdev, info);

	spin_lock_init(&info->controller.lock);
	init_waitqueue_head(&info->controller.wq);

	/* get the clock source and enable it */

	info->clk = clk_get(&pdev->dev, "nand");
	if (IS_ERR(info->clk)) {
		dev_err(&pdev->dev, "failed to get clock\n");
		err = -ENOENT;
		kfree(info);

		/* fix 'use after free' bug */
		platform_set_drvdata(pdev, NULL);
		goto exit_error;
	}

	clk_enable(info->clk);

	/* allocate and map the resource */

	/* currently we assume we have the one resource */
	res  = &pdev->resource[0];
	size = resource_size(res);

	info->area = request_mem_region(res->start, size, pdev->name);

	if (info->area == NULL) {
		dev_err(&pdev->dev, "cannot reserve register region\n");
		err = -ENOENT;
		kfree(info);

		/* fix 'use after free' bug */
		platform_set_drvdata(pdev, NULL);
		goto exit_error;
	}

	info->device     = &pdev->dev;
	info->platform   = plat;
	info->regs       = ioremap(res->start, size);

	if (info->regs == NULL) {
		dev_err(&pdev->dev, "cannot reserve register region\n");
		err = -EIO;
		kfree(info);

		/* fix 'use after free' bug */
		platform_set_drvdata(pdev, NULL);
		goto exit_error;
	}

	dev_dbg(&pdev->dev, "mapped registers at %p\n", info->regs);

	/* ioremap for nanddma
	*/
	res_ndma  = &pdev->resource[1];
	size_ndma = resource_size(res_ndma);

	info->regs_ndma = ioremap(res_ndma->start, size_ndma);

	if (info->regs_ndma == NULL) {
		dev_err(&pdev->dev, "cannot reserve register region\n");
		err = -EIO;
		kfree(info);

		/* fix 'use after free' bug */
		platform_set_drvdata(pdev, NULL);
		goto exit_error;
	}

	dev_dbg(&pdev->dev, "mapped ndma registers at %p\n", info->regs_ndma);

#ifdef __USE_NAND_DMA_READ__
	if (!g_nand_dma_vaddr)
	{
		buf_info.size = 2048;
		buf_info.addr = 0;

		if (d4_bma_alloc_buf(&buf_info) < 0)
		{
			printk("nDMA buffer alloc ERROR!\n");
			goto exit_error;
		}

		g_nand_dma_paddr = buf_info.addr;
		g_nand_dma_vaddr = phys_to_virt(g_nand_dma_paddr);
	}

	#ifdef __NDMA_INT_ISR_ENALBE__
	/* nDma interrupt  */
	res = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
	if (!res) {
		dev_err(pdev, "irq request failed.\n");
		goto exit_error;
	}

	g_inDmaIrq = res->start;
	
	ret = request_irq(g_inDmaIrq, drime4_nand_dma_isr, IRQF_DISABLED, "nDMA IRQ", pdev);
	
	if (ret < 0) {
		dev_err(pdev, "irq request failed.\n");
		goto exit_error;
	}

	mutex_init(&nDmaMutex);
	#endif

#endif

	/* initialise the hardware */

	err = drime4_nand_inithw(info);
	if (err != 0) {
		kfree(info);

		/* fix 'use after free' bug */
		platform_set_drvdata(pdev, NULL);
		goto exit_error;
	}

	/*sets = (plat != NULL) ? plat->sets : NULL;*/
	if ((plat != NULL) && (plat->sets != NULL))
		sets = plat->sets;
	else {
		kfree(info);

		/* fix 'use after free' bug */
		platform_set_drvdata(pdev, NULL);
		goto exit_error;
	}
	nr_sets = (plat != NULL) ? plat->nr_sets : 1;

	info->mtd_count = nr_sets;

	/* allocate our information */

	size = nr_sets * sizeof(*info->mtds);
	info->mtds = kzalloc(size, GFP_KERNEL);
	if (info->mtds == NULL) {
		dev_err(&pdev->dev, "failed to allocate mtd storage\n");
		err = -ENOMEM;
		kfree(info);

		/* fix 'use after free' bug */
		platform_set_drvdata(pdev, NULL);
		goto exit_error;
	}

	/* initialise all possible chips */

	nmtd = info->mtds;

	for (setno = 0; setno < nr_sets; setno++, nmtd++) {
		pr_debug("initialising set %d (%p, info %p)\n", setno, nmtd, info);

		drime4_nand_init_chip(info, nmtd, sets);

		nmtd->scan_res = nand_scan_ident(&nmtd->mtd,
						 (sets) ? sets->nr_chips : 1,
						 NULL);

		if (nmtd->scan_res == 0) {
			drime4_nand_update_chip(info, nmtd);
			nand_scan_tail(&nmtd->mtd);

			/* for drime4 configuration
			*/
			nmtd->chip.write_page = drime4_nand_write_page;

#if defined(CONFIG_MTD_NAND_DRIME4_8BIT_ECC)
			nmtd->chip.ecc.write_page = drime4_nand_write_page_8b_hwecc;
			nmtd->chip.ecc.read_page = drime4_nand_read_page_8b_hwecc;
#else
			nmtd->chip.ecc.write_page = drime4_nand_write_page_hwecc;
			#ifdef __USE_NAND_DMA_READ__	
			nmtd->chip.ecc.read_page = drime4_nand_read_withdma;
			#else
			nmtd->chip.ecc.read_page = drime4_nand_read_page_hwecc;
			#endif
#endif
			nmtd->chip.ecc.write_oob = drime4_nand_write_oob_std;
			nmtd->chip.ecc.read_oob = drime4_nand_read_oob_std;

			drime4_nand_add_partition(info, nmtd, sets);
		}

		if (sets != NULL)
			sets++;
	}

	err = drime4_nand_cpufreq_register(info);
	if (err < 0) {
		dev_err(&pdev->dev, "failed to init cpufreq support\n");
		kfree(info->mtds);
		kfree(info);

		/* fix 'use after free' bug */
		platform_set_drvdata(pdev, NULL);
		goto exit_error;
	}

	if (allow_clk_stop(info)) {
		dev_info(&pdev->dev, "clock idle support enabled\n");
		clk_disable(info->clk);
	}

	pr_debug("initialised ok\n");
	return 0;

 exit_error:
	drime4_nand_remove(pdev);

	if (err == 0)
		err = -EINVAL;
	return err;
}

/* PM Support */
#ifdef CONFIG_PM

static int drime4_nand_suspend(struct platform_device *dev, pm_message_t pm)
{
	struct drime4_nand_info *info = platform_get_drvdata(dev);

	if (info) {
		info->save_sel = __raw_readl(info->sel_reg);

		/* For the moment, we must ensure nFCE is high during
		 * the time we are suspended. This really should be
		 * handled by suspending the MTDs we are using, but
		 * that is currently not the case. */

		__raw_writel(info->save_sel | info->sel_bit, info->sel_reg);

		if (!allow_clk_stop(info))
			clk_disable(info->clk);
	}

	return 0;
}

static int drime4_nand_resume(struct platform_device *dev)
{
	struct drime4_nand_info *info = platform_get_drvdata(dev);
	unsigned long sel;

	if (info) {
		clk_enable(info->clk);
		drime4_nand_inithw(info);

		/* Restore the state of the nFCE line. */

		sel = readl(info->sel_reg);
		sel &= ~info->sel_bit;
		sel |= info->save_sel & info->sel_bit;
		writel(sel, info->sel_reg);

		if (allow_clk_stop(info))
			clk_disable(info->clk);
	}

	return 0;
}

#else
#define drime4_nand_suspend NULL
#define drime4_nand_resume NULL
#endif

/* driver device registration */

static struct platform_device_id drimeX_driver_ids[] = {
	{
		.name		= "drime4-nand",
		.driver_data	= 0,
	},
};

static struct platform_driver drime4_nand_driver = {
	.probe		= drime4_nand_probe,
	.remove		= drime4_nand_remove,
	.suspend	= drime4_nand_suspend,
	.resume		= drime4_nand_resume,
	.id_table	= drimeX_driver_ids,
	.driver		= {
		.name	= "drime4-nand",
		.owner	= THIS_MODULE,
		},
};

static int __init drime4_nand_init(void)
{
	printk("DRIME4 NAND Driver, (c) 2011 Samsung Electronics\n");

	return platform_driver_register(&drime4_nand_driver);
}

static void __exit drime4_nand_exit(void)
{
	platform_driver_unregister(&drime4_nand_driver);
}

#ifndef CONFIG_SCORE_FAST_RESUME
module_init(drime4_nand_init);
#else
fast_dev_initcall(drime4_nand_init);
#endif
module_exit(drime4_nand_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("DRIME4 MTD NAND driver");
