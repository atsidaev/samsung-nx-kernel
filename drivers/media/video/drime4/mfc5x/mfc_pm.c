/*
 * linux/drivers/media/video/samsung/mfc5x/mfc_pm.c
 *
 * Copyright (c) 2010 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com/
 *
 * Power management module for Samsung MFC (Multi Function Codec - FIMV) driver
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/err.h>
#include <linux/clk.h>
#include <linux/clkdev.h>
#include <linux/d4-pmu.h>
#include <linux/d4_rmu.h>
#include <linux/delay.h>

#include "mfc_dev.h"
#include "mfc_log.h"
#include "mfc_reg.h"
#if defined(CONFIG_CPU_DRIME4)
#include <mach/clock.h>

#define MFC_CLK_NAME	"mfc"
#define MFC_PD_NAME	"mfc_pd"

static struct mfc_pm *pm;

int mfc_init_pm(struct mfc_dev *mfcdev)
{
	int ret = 0;

	pm = &mfcdev->pm;

/*	struct mfc_inst_ctx *ctx = mfcdev->inst_ctx[0];  */

	sprintf(pm->clk_name, "%s", MFC_CLK_NAME);
	sprintf(pm->pd_name, "%s", MFC_PD_NAME);

	pm->clock = clk_get(mfcdev->device, pm->clk_name);
	if (pm->clock == -2)
		return -EINVAL;

	if (IS_ERR(pm->clock))
		ret = -ENOENT;

/*	if (ctx->job_status == MFC_DECODING) {
 *		clk_set_rate(pm->clock, 200 * 1000000);
 *	} else {
 */		clk_set_rate(pm->clock, 300 * 1000000);
/*	}      */

	return ret;
}

void mfc_final_pm(struct mfc_dev *mfcdev)
{
	clk_put(pm->clock);
}

int mfc_clock_on(void)
{
	return clk_enable(pm->clock);
}

void mfc_clock_off(void)
{
	clk_disable(pm->clock);
}

static void mfc_k_reset(void)
{
	write_reg(0x80000000, 0xF000);
	udelay(1);
	write_reg(0x00000000, 0xF000);
}

int mfc_power_on(void)
{
#ifdef CONFIG_PMU_SELECT
#if 0
	int ret;
	mfc_dbg("************* MFC_PMU_OFF ************* \n");
		d4_pmu_scpre_set(PMU_CODEC, PMU_CTRL_OFF);
		udelay(100);
		d4_pmu_scall_set(PMU_CODEC, PMU_CTRL_OFF);
		ret = wait_for_stable(PMU_CODEC);
		if (ret) {
			d4_pmu_scpre_set(PMU_CODEC, PMU_CTRL_ON);
			d4_pmu_scall_set(PMU_CODEC, PMU_CTRL_ON);
			return -1;
		}
		mfc_clock_on();
		d4_pmu_dma_reset(PMU_CODEC);
/*
 		d4_pmu_isoen_set(PMU_CODEC, PMU_CTRL_OFF);
 */
		mfc_k_reset();
#else
	mfc_clock_on();
#endif
#endif
	return 0;
}


int mfc_power_off(void)
{
#ifdef CONFIG_PMU_SELECT
#if 0
	int ret;
	struct d4_rmu_device *rmu;
	mfc_dbg("************* MFC_PMU_ON ************* \n");

	/*if (ipcs_pmu_state == 0) {
		mutex_unlock(&ipcs_pmu_mutex);
		return;
	}
	ipcs_pmu_state = 0;*/

	ret = d4_pmu_check(PMU_CODEC);
	if (ret != 0) {
		mfc_k_reset();
		d4_pmu_isoen_set(PMU_CODEC, PMU_CTRL_ON);
		mfc_clock_off();
		d4_pmu_scpre_set(PMU_CODEC, PMU_CTRL_ON);
		d4_pmu_scall_set(PMU_CODEC, PMU_CTRL_ON);
	}
#else
	mfc_clock_off();
#endif
#endif
	return 0;
}


void mfc_bus_disable(void)
{
	int ret;
	ret = d4_pmu_check(PMU_CODEC);
	if (ret != 0) {
		d4_pmu_isoen_set(PMU_CODEC, PMU_CTRL_ON);
		mfc_clock_off();
		d4_pmu_scpre_set(PMU_CODEC, PMU_CTRL_ON);
		d4_pmu_scall_set(PMU_CODEC, PMU_CTRL_ON);
	}
}

#elif defined(CONFIG_CPU_DRIME4)
#ifdef CONFIG_PM_RUNTIME
#include <linux/pm_runtime.h>
#endif

#define MFC_PARENT_CLK_NAME		"mout_mfc0"
#define MFC_CLKNAME			"sclk_mfc"
#define MFC_GATE_CLK_NAME		"mfc"

#undef CLK_DEBUG

static struct mfc_pm *pm;

#ifdef CLK_DEBUG
atomic_t clk_ref;
#endif

#ifdef CONFIG_CPU_FREQ
#include <linux/cpufreq.h>

#define MFC0_BUS_CLK_NAME	"aclk_gdl"
#define MFC1_BUS_CLK_NAME	"aclk_gdr"

static struct clk *bus_clk;
static unsigned int prev_bus_rate;

static int mfc_cpufreq_transition(struct notifier_block *nb,
					unsigned long val, void *data)
{
	unsigned long bus_rate;

	if (val == CPUFREQ_PRECHANGE)
		prev_bus_rate = clk_get_rate(bus_clk);

	if (val == CPUFREQ_POSTCHANGE) {
		bus_rate = clk_get_rate(bus_clk);

		if (bus_rate != prev_bus_rate) {
			mfc_dbg("MFC freq pre: %lu\n",
				clk_get_rate(pm->op_clk));
			clk_set_rate(pm->op_clk, bus_rate);
			mfc_dbg("MFC freq post: %lu\n",
				clk_get_rate(pm->op_clk));
		}
	}

	return 0;
}

static inline int mfc_cpufreq_register(void)
{
	int ret;
	unsigned long rate;

	bus_clk = clk_get(pm->device, MFC0_BUS_CLK_NAME);

	if (IS_ERR(bus_clk)) {
		printk(KERN_ERR "failed to get bus clock\n");
		ret = -ENOENT;
		goto err_bus_clk;
	}

	prev_bus_rate = clk_get_rate(bus_clk);

	rate = clk_get_rate(pm->clock);

	if (rate != prev_bus_rate)
		clk_set_rate(pm->op_clk, prev_bus_rate);

	pm->freq_transition.notifier_call = mfc_cpufreq_transition;

	return cpufreq_register_notifier(&pm->freq_transition,
					 CPUFREQ_TRANSITION_NOTIFIER);
err_bus_clk:
	return ret;
}

static inline void mfc_cpufreq_deregister(void)
{
	clk_put(bus_clk);

	prev_bus_rate = 0;

	cpufreq_unregister_notifier(&pm->freq_transition,
				    CPUFREQ_TRANSITION_NOTIFIER);
}

#else
static inline int mfc_cpufreq_register(void)
{

	return 0;
}

static inline void mfc_cpufreq_deregister(void)
{
}
#endif /* CONFIG_CPU_FREQ */

int mfc_init_pm(struct mfc_dev *mfcdev)
{
	struct clk *parent, *sclk;
	int ret = 0;

	pm = &mfcdev->pm;

	parent = clk_get(mfcdev->device, MFC_PARENT_CLK_NAME);
	if (IS_ERR(parent)) {
		printk(KERN_ERR "failed to get parent clock\n");
		ret = -ENOENT;
		goto err_p_clk;
	}

	sclk = clk_get(mfcdev->device, MFC_CLKNAME);
	if (IS_ERR(sclk)) {
		printk(KERN_ERR "failed to get source clock\n");
		ret = -ENOENT;
		goto err_s_clk;
	}

	clk_set_parent(sclk, parent);
	/* FIXME : */
	clk_set_rate(sclk, 200 * 1000000);

	/* clock for gating */
	pm->clock = clk_get(mfcdev->device, MFC_GATE_CLK_NAME);
	if (IS_ERR(pm->clock)) {
		printk(KERN_ERR "failed to get clock-gating control\n");
		ret = -ENOENT;
		goto err_g_clk;
	}

	atomic_set(&pm->power, 0);

#if defined(CONFIG_PM_RUNTIME) || defined(CONFIG_CPU_FREQ)
	pm->device = mfcdev->device;
#endif

#ifdef CONFIG_PM_RUNTIME
	pm_runtime_enable(pm->device);
#endif

#ifdef CLK_DEBUG
	atomic_set(&clk_ref, 0);
#endif

#ifdef CONFIG_CPU_FREQ
	pm->op_clk = sclk;
	ret = mfc_cpufreq_register();
	if (ret < 0) {
		dev_err(mfcdev->device, "Failed to register cpufreq\n");
		goto err_cpufreq;
	}
#endif

	return 0;

#ifdef CONFIG_CPU_FREQ
err_cpufreq:
	clk_put(pm->clock);
#endif
err_g_clk:
	clk_put(sclk);
err_s_clk:
	clk_put(parent);
err_p_clk:
	return ret;
}

void mfc_final_pm(struct mfc_dev *mfcdev)
{
	clk_put(pm->clock);

#ifdef CONFIG_PM_RUNTIME
	pm_runtime_disable(pm->device);
#endif
}

int mfc_clock_on(void)
{
#ifdef CLK_DEBUG
	atomic_inc(&clk_ref);
	mfc_dbg("+ %d", atomic_read(&clk_ref));
#endif

#ifdef CONFIG_PM_RUNTIME
	return clk_enable(pm->clock);
#else
	return clk_enable(pm->clock);
#endif
}

void mfc_clock_off(void)
{
#ifdef CLK_DEBUG
	atomic_dec(&clk_ref);
	mfc_dbg("- %d", atomic_read(&clk_ref));
#endif

#ifdef CONFIG_PM_RUNTIME
	clk_disable(pm->clock);
#else
	clk_disable(pm->clock);
#endif
}

int mfc_power_on(void)
{
#ifdef CONFIG_PM_RUNTIME
	return pm_runtime_get_sync(pm->device);
#else
	atomic_set(&pm->power, 1);

	return 0;
#endif
}

int mfc_power_off(void)
{
#ifdef CONFIG_PM_RUNTIME
	return pm_runtime_put_sync(pm->device);
#else
	atomic_set(&pm->power, 0);

	return 0;
#endif
}

bool mfc_power_chk(void)
{
	mfc_dbg("%s", atomic_read(&pm->power) ? "on" : "off");

	return atomic_read(&pm->power) ? true : false;
}

#else

int mfc_init_pm(struct mfc_dev *mfcdev)
{
	return -1;
}

void mfc_final_pm(struct mfc_dev *mfcdev)
{
	/* NOP */
}

int mfc_clock_on(void)
{
	return -1;
}

void mfc_clock_off(void)
{
	/* NOP */
}

int mfc_power_on(void)
{
	return -1;
}

int mfc_power_off(void)
{
	return -1;
}
#endif

