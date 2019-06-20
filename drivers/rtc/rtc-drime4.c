/* linux/drivers/rtc/rtc-drime4.c
 *
 * Copyright (c) 2011 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com/
 *
 * DRIME4 RTC Driver
 *
 * This code is based on linux/drivers/rtc/rtc-s3c.c
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

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/rtc.h>
#include <linux/bcd.h>
#include <linux/clk.h>
#include <linux/log2.h>
#include <linux/slab.h>
#include <linux/io.h>

#include <mach/hardware.h>
#include <mach/rtc_sysfs.h>
#include <asm/irq.h>
#include <mach/regs-rtc.h>
/*
static DEFINE_SPINLOCK(drime4_rtc_lock);
*/
struct drime4_rtc {
	struct rtc_device		*rtc;
	void __iomem			*base;
	struct resource			*mem;
	resource_size_t			pbase;
	resource_size_t			base_size;
	int				irq_alm;
	int				irq_pri;
	struct clk			*clk;
};

static inline int bcd2year(u16 bcd_year)
{
	return (bcd2bin((bcd_year & 0xff00) >> 8) * 100) +
		(bcd2bin(bcd_year & 0xff));
}

static inline u16 year2bcd(int year)
{
	return (bin2bcd(year / 100) << 8) |
		(bin2bcd(year % 100));
}

static irqreturn_t drime4_rtc_alarmirq(int irq, void *id)
{
	struct drime4_rtc *d4_rtc = id;

	rtc_update_irq(d4_rtc->rtc, 1, RTC_AF | RTC_IRQF);

	writeb(DRIME4_RTC_PEND_CLEAR, d4_rtc->base + DRIME4_RTC_PEND);

	return IRQ_HANDLED;
}

static irqreturn_t drime4_rtc_priirq(int irq, void *id)
{
	struct drime4_rtc *d4_rtc = id;

	rtc_update_irq(d4_rtc->rtc, 1, RTC_AF | RTC_IRQF);

	writeb(DRIME4_RTC_PEND_CLEAR, d4_rtc->base + DRIME4_RTC_PEND);

	return IRQ_HANDLED;
}

static void drime4_rtc_set_enable(struct platform_device *pdev, int en)
{
	struct drime4_rtc *d4_rtc = platform_get_drvdata(pdev);
	unsigned int tmp;

	if(!d4_rtc)
		return;
	
	tmp = readb(d4_rtc->base + DRIME4_RTCCON);

	if (!en) {
		tmp &= ~DRIME4_RTCCON_RTCEN;
		tmp &= ~DRIME4_RTCCON_STARTB;
//		tmp &= ~DRIME4_RTCCON_CLKRST;

		writeb(tmp, d4_rtc->base + DRIME4_RTCCON);
	} else {
		/* re-enable the device, and check it is ok */
		tmp |= DRIME4_RTCCON_RTCEN;
		tmp |= DRIME4_RTCCON_STARTB;
//		tmp |= DRIME4_RTCCON_CLKRST;

		writeb(tmp, d4_rtc->base + DRIME4_RTCCON);
	}
}

/* Update control registers */
static int drime4_rtc_setaie(struct device *dev, unsigned int enabled)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct drime4_rtc *d4_rtc = platform_get_drvdata(pdev);
	unsigned int tmp;

	if(!d4_rtc)
		return 0;

	pr_debug("%s: aie=%d\n", __func__, enabled);

	tmp = readb(d4_rtc->base + DRIME4_RTCALM) & ~DRIME4_RTCALM_ALMEN;

	if (enabled)
		tmp |= DRIME4_RTCALM_ALMEN;

	writeb(tmp, d4_rtc->base + DRIME4_RTCALM);
	writeb(0x1, d4_rtc->base + DRIME4_RTCIM);
	return 0;
}
/*
static int drime4_rtc_setfreq(struct device *dev, int freq)
{
	if (!is_power_of_2(freq))
		return -EINVAL;

	spin_lock_irq(&drime4_rtc_lock);


	spin_unlock_irq(&drime4_rtc_lock);

	return 0;
}
*/
static int drime4_rtc_open(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct drime4_rtc *d4_rtc = platform_get_drvdata(pdev);
	int ret;

	if(!d4_rtc)
		return 0;
	
	ret = request_irq(d4_rtc->irq_alm, drime4_rtc_alarmirq,
			  IRQF_DISABLED,  "drime4-rtc alarm", d4_rtc);

	if (ret) {
		dev_err(dev, "IRQ%d error %d\n", d4_rtc->irq_alm, ret);
		return ret;
	}

	ret = request_irq(d4_rtc->irq_pri, drime4_rtc_priirq,
			  IRQF_DISABLED,  "drime4-rtc pri", d4_rtc);

	if (ret) {
		dev_err(dev, "IRQ%d error %d\n", d4_rtc->irq_pri, ret);
		goto fail_priirq;
	}

	return ret;

fail_priirq:
	free_irq(d4_rtc->irq_alm, d4_rtc);
	return ret;
}

static void drime4_rtc_release(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct drime4_rtc *d4_rtc = platform_get_drvdata(pdev);

	if(!d4_rtc)
		return;
	/* do not clear AIE here, it may be needed for wake */

	free_irq(d4_rtc->irq_alm, d4_rtc);
	free_irq(d4_rtc->irq_pri, d4_rtc);
}

static int drime4_rtc_gettime(struct device *dev, struct rtc_time *rtc_tm)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct drime4_rtc *d4_rtc = platform_get_drvdata(pdev);
	unsigned int have_retried = 0;

	if(!d4_rtc)
		return -1;

 retry_get_time:
	rtc_tm->tm_min  = readb(d4_rtc->base + DRIME4_BCDMIN);
	rtc_tm->tm_hour = readb(d4_rtc->base + DRIME4_BCDHOUR);
	rtc_tm->tm_mday = readb(d4_rtc->base + DRIME4_BCDDATE);
	rtc_tm->tm_mon  = readb(d4_rtc->base + DRIME4_BCDMON);
	rtc_tm->tm_year = readw(d4_rtc->base + DRIME4_BCDYEAR);
	rtc_tm->tm_sec  = readb(d4_rtc->base + DRIME4_BCDSEC);

	/* the only way to work out wether the system was mid-update
	 * when we read it is to check the second counter, and if it
	 * is zero, then we re-try the entire read
	 */

	if (rtc_tm->tm_sec == 0 && !have_retried) {
		have_retried = 1;
		goto retry_get_time;
	}

	rtc_tm->tm_sec = bcd2bin(rtc_tm->tm_sec);
	rtc_tm->tm_min = bcd2bin(rtc_tm->tm_min);
	rtc_tm->tm_hour = bcd2bin(rtc_tm->tm_hour);
	rtc_tm->tm_mday = bcd2bin(rtc_tm->tm_mday);
	rtc_tm->tm_mon = bcd2bin(rtc_tm->tm_mon) - 1;
	rtc_tm->tm_year = bcd2year(rtc_tm->tm_year) - 1900;

	pr_debug("read time %04d.%02d.%02d %02d:%02d:%02d\n",
		 1900 + rtc_tm->tm_year, rtc_tm->tm_mon+1, rtc_tm->tm_mday,
		 rtc_tm->tm_hour, rtc_tm->tm_min, rtc_tm->tm_sec);

	return rtc_valid_tm(rtc_tm);
}

static int drime4_rtc_settime(struct device *dev, struct rtc_time *tm)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct drime4_rtc *d4_rtc = platform_get_drvdata(pdev);

	if(!d4_rtc)
		return 0;
	
	pr_debug("set time %04d.%02d.%02d %02d:%02d:%02d\n",
		 1900 + tm->tm_year, tm->tm_mon + 1, tm->tm_mday,
		 tm->tm_hour, tm->tm_min, tm->tm_sec);

	drime4_rtc_set_enable(pdev, 1);

	writeb(bin2bcd(tm->tm_sec), d4_rtc->base + DRIME4_BCDSEC);
	writeb(bin2bcd(tm->tm_min), d4_rtc->base + DRIME4_BCDMIN);
	writeb(bin2bcd(tm->tm_hour), d4_rtc->base + DRIME4_BCDHOUR);
	writeb(bin2bcd(tm->tm_mday), d4_rtc->base + DRIME4_BCDDATE);
	writeb(bin2bcd(tm->tm_mon+1), d4_rtc->base + DRIME4_BCDMON);

	writew(year2bcd(tm->tm_year + 1900), d4_rtc->base + DRIME4_BCDYEAR);

	drime4_rtc_set_enable(pdev, 0);

	return 0;
}

static int drime4_rtc_getalarm(struct device *dev, struct rtc_wkalrm *alrm)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct drime4_rtc *d4_rtc = platform_get_drvdata(pdev);
	struct rtc_time *alm_tm = &alrm->time;
	unsigned int alm_en;

	if(!d4_rtc)
		return 0;

	alm_tm->tm_sec  = readb(d4_rtc->base + DRIME4_ALMSEC);
	alm_tm->tm_min  = readb(d4_rtc->base + DRIME4_ALMMIN);
	alm_tm->tm_hour = readb(d4_rtc->base + DRIME4_ALMHOUR);
	alm_tm->tm_mon  = readb(d4_rtc->base + DRIME4_ALMMON);
	alm_tm->tm_mday = readb(d4_rtc->base + DRIME4_ALMDATE);
	alm_tm->tm_year = readw(d4_rtc->base + DRIME4_ALMYEAR);

	alm_en = readb(d4_rtc->base + DRIME4_RTCALM);

	alrm->enabled = (alm_en & DRIME4_RTCALM_ALMEN) ? 1 : 0;

	/* decode the alarm enable field */

	if (alm_en & DRIME4_RTCALM_SECEN)
		alm_tm->tm_sec = bcd2bin(alm_tm->tm_sec);
	else
		alm_tm->tm_sec = -1;

	if (alm_en & DRIME4_RTCALM_MINEN)
		alm_tm->tm_min = bcd2bin(alm_tm->tm_min);
	else
		alm_tm->tm_min = -1;

	if (alm_en & DRIME4_RTCALM_HOUREN)
		alm_tm->tm_hour = bcd2bin(alm_tm->tm_hour);
	else
		alm_tm->tm_hour = -1;

	if (alm_en & DRIME4_RTCALM_DATEEN)
		alm_tm->tm_mday = bcd2bin(alm_tm->tm_mday);
	else
		alm_tm->tm_mday = -1;

	if (alm_en & DRIME4_RTCALM_MONEN)
		alm_tm->tm_mon = bcd2bin(alm_tm->tm_mon) - 1;
	else
		alm_tm->tm_mon = -1;

	if (alm_en & DRIME4_RTCALM_YEAREN)
		alm_tm->tm_year = bcd2year(alm_tm->tm_year) - 1900;
	else
		alm_tm->tm_year = -1;

	pr_debug("read alarm %d, %04d.%02d.%02d %02d:%02d:%02d\n",
		 alm_en,
		 1900 + alm_tm->tm_year, alm_tm->tm_mon+1, alm_tm->tm_mday,
		 alm_tm->tm_hour, alm_tm->tm_min, alm_tm->tm_sec);

	return 0;
}

static int drime4_rtc_setalarm(struct device *dev, struct rtc_wkalrm *alrm)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct drime4_rtc *d4_rtc = platform_get_drvdata(pdev);
	struct rtc_time *tm = &alrm->time;
	unsigned int alrm_en;

	if(!d4_rtc)
		return 0;
	
	pr_debug("drime4_rtc_setalarm: %d, %04d.%02d.%02d %02d:%02d:%02d\n",
		 alrm->enabled,
		 1900 + tm->tm_year, tm->tm_mon+1, tm->tm_mday,
		 tm->tm_hour, tm->tm_min, tm->tm_sec);

	drime4_rtc_set_enable(pdev, 1);

	alrm_en = readb(d4_rtc->base + DRIME4_RTCALM)
			& DRIME4_RTCALM_ALMEN;
	writeb(0x00, d4_rtc->base + DRIME4_RTCALM);

	if (tm->tm_sec < 60 && tm->tm_sec >= 0) {
		alrm_en |= DRIME4_RTCALM_SECEN;
		writeb(bin2bcd(tm->tm_sec), d4_rtc->base + DRIME4_ALMSEC);
	}

	if (tm->tm_min < 60 && tm->tm_min >= 0) {
		alrm_en |= DRIME4_RTCALM_MINEN;
		writeb(bin2bcd(tm->tm_min), d4_rtc->base + DRIME4_ALMMIN);
	}

	if (tm->tm_hour < 24 && tm->tm_hour >= 0) {
		alrm_en |= DRIME4_RTCALM_HOUREN;
		writeb(bin2bcd(tm->tm_hour), d4_rtc->base + DRIME4_ALMHOUR);
	}
/*
	if (tm->tm_mday <= 31 && tm->tm_mday >= 1) {
		alrm_en |= DRIME4_RTCALM_DAYEN;
		writeb(bin2bcd(tm->tm_mday), d4_rtc->base + DRIME4_ALMDATE);
	}

	if (tm->tm_mon < 12 && tm->tm_mon >= 0) {
		alrm_en |= DRIME4_RTCALM_MONEN;
		writeb(bin2bcd(tm->tm_mon+1), d4_rtc->base + DRIME4_ALMMON);
	}

	if (tm->tm_year >= 0) {
		alrm_en |= DRIME4_RTCALM_YEAREN;
		writew(year2bcd(tm->tm_year + 1900), d4_rtc->base + DRIME4_ALMYEAR);
	}
*/
	pr_debug("setting DRIME4_RTCALM to %08x\n", alrm_en);

	writeb(alrm_en, d4_rtc->base + DRIME4_RTCALM);

	drime4_rtc_setaie(dev, alrm->enabled);
	writeb(0x0, d4_rtc->base + DRIME4_RTC_WUPEND);
	drime4_rtc_set_enable(pdev, 0);

	return 0;
}

unsigned int rtc_get_boot_info(struct device *dev)
{
	unsigned int data;
	struct platform_device *pdev = to_platform_device(dev);
	struct drime4_rtc *d4_rtc = platform_get_drvdata(pdev);

	if(!d4_rtc)
		return 0;

	drime4_rtc_set_enable(pdev, 1);

	data = readb(d4_rtc->base + DRIME4_ALMDAY);
	drime4_rtc_set_enable(pdev, 0);
	return data;
}

void rtc_set_boot_info(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct drime4_rtc *d4_rtc = platform_get_drvdata(pdev);

	if(!d4_rtc)
		return;

	drime4_rtc_set_enable(pdev, 1);
	writeb(0x0, d4_rtc->base + DRIME4_ALMDAY);
	drime4_rtc_set_enable(pdev, 0);
}

static const struct rtc_class_ops drime4_rtcops = {
	.open		= drime4_rtc_open,
	.release	= drime4_rtc_release,
	.read_time	= drime4_rtc_gettime,
	.set_time	= drime4_rtc_settime,
	.read_alarm	= drime4_rtc_getalarm,
	.set_alarm	= drime4_rtc_setalarm,
	.alarm_irq_enable = drime4_rtc_setaie,
};

static int __devinit drime4_rtc_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct drime4_rtc *d4_rtc;
	struct rtc_time rtc_tm;
	struct resource *res;
	int ret = 0;

	d4_rtc = devm_kzalloc(&pdev->dev, sizeof(struct drime4_rtc),
			GFP_KERNEL);
	if (!d4_rtc) {
		dev_dbg(dev, "could not allocate memory\n");
		return -ENOMEM;
	}

	/* find the IRQs */

	d4_rtc->irq_alm = platform_get_irq(pdev, 0);
	if (d4_rtc->irq_alm < 0) {
		dev_err(dev, "no irq for rtc alarm\n");
		ret = -ENXIO;
		goto err_free;
	}

	d4_rtc->irq_pri = platform_get_irq(pdev, 1);
	if (d4_rtc->irq_pri < 0) {
		dev_err(dev, "no irq for pri\n");
		ret = -ENXIO;
		goto err_free;
	}

	pr_debug("drime4_rtc: alarm irq %d, pri irq %d\n",
		 d4_rtc->irq_alm, d4_rtc->irq_pri);

	/* get the memory region */

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (res == NULL) {
		dev_err(dev, "failed to get memory region resource\n");
		ret = -EINVAL;
		goto err_free;
	}

	d4_rtc->pbase = res->start;
	d4_rtc->base_size = res->end - res->start + 1;
	d4_rtc->mem = devm_request_mem_region(&pdev->dev, d4_rtc->pbase,
					 d4_rtc->base_size,
					 pdev->name);

	if (d4_rtc->mem == NULL) {
		dev_err(dev, "failed to reserve memory region\n");
		ret = -ENOENT;
		goto err_free;
	}

	d4_rtc->base = devm_ioremap(&pdev->dev, res->start,
				res->end - res->start + 1);
	if (d4_rtc->base == NULL) {
		dev_err(dev, "failed ioremap()\n");
		ret = -EINVAL;
		goto err_free;
	}

	platform_set_drvdata(pdev, d4_rtc);
	/* check to see if everything is setup correctly */

	drime4_rtc_set_enable(pdev, 0);

	pr_debug("drime4_rtc: RTCCON=%01x\n",
		 readb(d4_rtc->base + DRIME4_RTCCON));

	device_init_wakeup(dev, 1);

	/* register RTC and exit */

	d4_rtc->rtc = rtc_device_register("drime4", dev, &drime4_rtcops,
				  THIS_MODULE);

	if (IS_ERR(d4_rtc->rtc)) {
		dev_err(dev, "cannot attach rtc\n");
		ret = PTR_ERR(d4_rtc->rtc);
		goto fail_nortc;
	}

	/* Check RTC Time */

	if(drime4_rtc_gettime(dev, &rtc_tm) == -1)
		goto fail_nortc;

	if (rtc_valid_tm(&rtc_tm)) {
		rtc_tm.tm_year	= 111;
		rtc_tm.tm_mon	= 6;
		rtc_tm.tm_mday	= 7;
		rtc_tm.tm_hour	= 12;
		rtc_tm.tm_min	= 59;
		rtc_tm.tm_sec	= 55;

		drime4_rtc_settime(dev, &rtc_tm);

		dev_warn(&pdev->dev, "warning: invalid RTC value so initializing it\n");
	}

	d4_rtc->rtc->max_user_freq = 32768;
	rtc_create_node(dev);
	return 0;

fail_nortc:
	drime4_rtc_set_enable(pdev, 0);
	clk_disable(d4_rtc->clk);
	clk_put(d4_rtc->clk);

err_free:
	devm_kfree(&pdev->dev, d4_rtc);

	return ret;
}

static int __devexit drime4_rtc_remove(struct platform_device *pdev)
{
	struct drime4_rtc *d4_rtc = platform_get_drvdata(pdev);

	if(!d4_rtc)
		return 0;

	device_init_wakeup(&pdev->dev, 0);

	drime4_rtc_setaie(&pdev->dev, 0);

	clk_disable(d4_rtc->clk);
	clk_put(d4_rtc->clk);

	rtc_device_unregister(d4_rtc->rtc);

	platform_set_drvdata(pdev, NULL);

	return 0;
}

#ifdef CONFIG_PM
static int drime4_rtc_suspend(struct platform_device *pdev, pm_message_t state)
{
	/* TODO: */
	return 0;
}

static int drime4_rtc_resume(struct platform_device *pdev)
{
	/* TODO: */
	return 0;
}
#else
#define drime4_rtc_suspend	NULL
#define drime4_rtc_resume	NULL
#endif

static struct platform_driver drime4_rtc_driver = {
	.probe		= drime4_rtc_probe,
	.remove		= __devexit_p(drime4_rtc_remove),
	.suspend	= drime4_rtc_suspend,
	.resume		= drime4_rtc_resume,
	.driver		= {
		.name	= "drime4-rtc",
		.owner	= THIS_MODULE,
	},
};

static int __init drime4_rtc_init(void)
{
	return platform_driver_register(&drime4_rtc_driver);
}
module_init(drime4_rtc_init);

static void __exit drime4_rtc_exit(void)
{
	platform_driver_unregister(&drime4_rtc_driver);
}
module_exit(drime4_rtc_exit);

MODULE_DESCRIPTION("Samsung DRIME4 RTC Driver");
MODULE_AUTHOR("Chanho Park <chanho61.park@samsung.com>");
