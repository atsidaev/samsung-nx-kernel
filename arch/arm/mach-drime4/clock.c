/* linux/arch/arm/mach-drime4/clock.c
 *
 * Copyright (c) 2011 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com/
 *
 * DRIME4 - Clock support
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/err.h>
#include <linux/clk.h>
#include <linux/io.h>
#include <linux/debugfs.h>
#include <linux/seq_file.h>
#include <linux/sched.h>
#include <linux/uaccess.h>

#include <mach/clock.h>
#include <mach/map.h>
#include <mach/common.h>
#include <mach/d4_cmu_regs.h>

#define CLK_REG(r, s, m)			\
	{ .reg = r, .shift = s, .mask = m, }	\

#define PLL3500X_M_MASK		0x3FF
#define PLL3500X_M_SHIFT	19
#define PLL3500X_P_MASK		0x3F
#define PLL3500X_P_SHIFT	13
#define PLL3500X_S_MASK		0x7
#define PLL3500X_S_SHIFT	10

#define PLL3600X_M_MASK		0x3FF
#define PLL3600X_M_SHIFT	17
#define PLL3600X_P_MASK		0x3F
#define PLL3600X_P_SHIFT	11
#define PLL3600X_S_MASK		0x7
#define PLL3600X_S_SHIFT	8
#define PLL3600X_K_MASK		0xFFFF
#define PLL3600X_K_SHIFT	16

#define PLL_PMS3500X_CLEAR(val)	\
	(val &= ~((PLL_P_MASK << PLL_P_SHIFT) |	\
		(PLL_M_MASK << PLL_M_SHIFT) |	\
		(PLL_S_MASK << PLL_S_SHIFT)))	\

#define PLL_PMS3500X(val, p, m, s)	\
	(val |= ((p & PLL_P_MASK) << PLL_P_SHIFT) |	\
		((m & PLL_M_MASK) << PLL_M_SHIFT) |	\
		((s & PLL_S_MASK) << PLL_S_SHIFT))	\

#define AUDPLL_M_MASK	0x1FF
#define AUDPLL_M_SHIFT	17
#define AUDPLL_P_MASK	0x3F
#define AUDPLL_P_SHIFT	11
#define AUDPLL_S_MASK	7
#define AUDPLL_S_SHIFT	8
#define AUDPLL_K_MASK	0xFFFFFF
#define AUDPLL_K_SHIFT	16

#define AUDPLL_PMS_CLEAR(val)	\
	(val &= ~((AUDPLL_P_MASK << AUDPLL_P_SHIFT) |	\
		(AUDPLL_M_MASK << AUDPLL_M_SHIFT) |	\
		(AUDPLL_S_MASK << AUDPLL_S_SHIFT)))	\

#define AUDPLL_PMS(val, p, m, s)	\
	(val |= ((p & PLL_P_MASK) << PLL_P_SHIFT) |	\
		((m & PLL_M_MASK) << PLL_M_SHIFT) |	\
		((s & PLL_S_MASK) << PLL_S_SHIFT))	\

#define XTL_HDMI_SEL	0
#define XTL_DPHY_SEL	1

#define NOR_CLK_GATE	0x200
#define PWM_CLK_GATE	0x208
#define PTC_CLK_GATE	0x20C
#define UART_CLK_GATE	0x210
#define GPIO_CLK_GATE	0x214
#define SPI_CLK_GATE	0x21C
#define I2C_CLK_GATE	0x220
#define I2S_CLK_GATE	0x224
#define ADC_CLK_GATE	0x228
#define TIMER_CLK_GATE	0x22C
#define EFS_CLK_GATE	0x230
#define SD_CLK_GATE	0x234
#define NAND_CLK_GATE	0x238
#define USB3_CLK_GATE	0x300
#define HSIC_CLK_GATE	0x304
#define ATA_CLK_GATE	0x308

#define NOR_CLK_RESET	0x400
#define PWM_CLK_RESET	0x408
#define PTC_CLK_RESET	0x40C
#define UART_CLK_RESET	0x410
#define GPIO_CLK_RESET	0x414
#define SPI_CLK_RESET	0x41C
#define I2C_CLK_RESET	0x420
#define I2S_CLK_RESET	0x424
#define ADC_CLK_RESET	0x428
#define TIMER_CLK_RESET	0x42C
#define EFS_CLK_RESET	0x430
#define SD_CLK_RESET	0x434
#define NAND_CLK_RESET	0x438
#define USB3_CLK_RESET	0x500
#define HSIC_CLK_RESET	0x504
#define ATA_CLK_RESET	0x508

static void __iomem *reg_clk_base = DRIME4_VA_CLOCK_CTRL;
static void __iomem *reg_platform_base = DRIME4_VA_PLATFORM_CTRL;

#define clk_writel(value, reg) \
	__raw_writel(value, (u32)reg_clk_base + (reg))
#define clk_readl(reg) \
	__raw_readl((u32)reg_clk_base + (reg))
#define pf_writel(value, reg) \
	__raw_writel(value, (u32)reg_platform_base + (reg))
#define pf_readl(reg) \
	__raw_readl((u32)reg_platform_base + (reg))

#define CLK(dev, con, ck)	\
	{			\
		.dev_id = dev,	\
		.con_id = con,	\
		.clk = ck,	\
	}

static DEFINE_MUTEX(clock_list_lock);
static LIST_HEAD(clocks);

static struct clk clk_xtl_hdmi = {
	.name = "xtalhdmi",
	.rate = 27000000,
	.type = XTAL_PAD,
};

static struct clk clk_xtl_dphy = {
	.name = "xtaldphy",
	.rate = 24000000,
	.type = XTAL_PAD,
};

void clk_init(struct clk *c)
{
	spin_lock_init(&c->lock);
	if (c->div == 0)
		c->div = 1;

	if (c->ops && c->ops->init)
		c->ops->init(c);

	if (!c->ops || !c->ops->enable) {
		c->refcnt++;
		if (c->parent)
			c->state = c->parent->state;
		else
			c->state = ON;
	}

	mutex_lock(&clock_list_lock);
	list_add(&c->node, &clocks);
	mutex_unlock(&clock_list_lock);
}

int clk_enable(struct clk *clk)
{
	int ret = 0;
	unsigned long flags;

	if (clk == NULL) 
		return -1;

	spin_lock_irqsave(&clk->lock, flags);

	if (clk->refcnt == 0) {
		if (clk->parent) {
			ret = clk_enable(clk->parent);
			if (ret)
				goto out;
		}

		if (clk->ops && clk->ops->enable) {
			ret = clk->ops->enable(clk);
			if (ret) {
				if (clk->parent)
					clk_disable(clk->parent);
				goto out;
			}
			clk->state = ON;
		}
	}
	clk->refcnt++;
out:
	spin_unlock_irqrestore(&clk->lock, flags);

	pr_debug("%s: %s clock enable ref:%d state:%d\n",
		__func__, clk->name, clk->refcnt, clk->state);
	return ret;
}
EXPORT_SYMBOL(clk_enable);

void clk_disable(struct clk *clk)
{
	unsigned long flags;

	if (clk == NULL)
		return;

	spin_lock_irqsave(&clk->lock, flags);

	if (clk->refcnt == 0) {
		WARN(1, "Attempting to disable clock %s with refcnt 0",
			clk->name);
		spin_unlock_irqrestore(&clk->lock, flags);
		return;
	}
	if (clk->refcnt == 1) {
		if (clk->ops && clk->ops->disable)
			clk->ops->disable(clk);

		if (clk->parent)
			clk_disable(clk->parent);

		clk->state = OFF;
	}
	clk->refcnt--;

	pr_debug("%s: %s clock disable ref:%d state:%d\n",
		__func__, clk->name, clk->refcnt, clk->state);

	spin_unlock_irqrestore(&clk->lock, flags);
}
EXPORT_SYMBOL(clk_disable);

unsigned long clk_get_rate(struct clk *clk)
{
	unsigned long rate;
	unsigned long flags;

	spin_lock_irqsave(&clk->lock, flags);

	if (clk->ops && clk->ops->get_rate)
		rate = clk->ops->get_rate(clk);
	else {
		if (clk->parent)
			rate = clk_get_rate(clk->parent) / clk->div;
		else
			rate = clk->rate / clk->div;
	}

	spin_unlock_irqrestore(&clk->lock, flags);

	pr_debug("%s: %s clock get_rate ref:%d state:%d rate:%lu\n",
		__func__, clk->name, clk->refcnt, clk->state, rate);

	return rate;
}
EXPORT_SYMBOL(clk_get_rate);

long clk_round_rate(struct clk *clk, unsigned long rate)
{
	unsigned long flags;
	long ret = -EIO;

	if (clk == NULL)
		return ret;

	spin_lock_irqsave(&clk->lock, flags);

	if (clk->ops && clk->ops->round_rate)
		ret = clk->ops->round_rate(clk, rate);

	spin_unlock_irqrestore(&clk->lock, flags);

	return ret;
}
EXPORT_SYMBOL(clk_round_rate);

int clk_set_rate(struct clk *clk, unsigned long rate)
{
	unsigned long flags;
	long new_rate;
	int ret = -EIO;

	if (clk == NULL) 
		return ret;

	pr_debug("%s: %s clock set_rate ref:%d state:%d setrate:%lu\n",
		__func__, clk->name, clk->refcnt, clk->state, rate);

	spin_lock_irqsave(&clk->lock, flags);

	if (clk->ops && clk->ops->round_rate) {
		new_rate = clk->ops->round_rate(clk, rate);
		if (new_rate < 0)
			goto out;
		rate = new_rate;
	}

	if (clk->ops && clk->ops->set_rate)
		ret = clk->ops->set_rate(clk, rate);

out:
	spin_unlock_irqrestore(&clk->lock, flags);

	return ret;
}
EXPORT_SYMBOL(clk_set_rate);

int clk_set_parent(struct clk *clk, struct clk *parent)
{
	int ret = -EIO;
	unsigned long flags;

	spin_lock_irqsave(&clk->lock, flags);

	if (clk->ops && clk->ops->set_parent)
		ret = clk->ops->set_parent(clk, parent);

	spin_unlock_irqrestore(&clk->lock, flags);

	return ret;
}
EXPORT_SYMBOL(clk_set_parent);

struct clk *clk_get_parent(struct clk *clk)
{
	return clk->parent;
}
EXPORT_SYMBOL(clk_get_parent);

static inline void extract_pll3500x_pms(u32 reg,
	struct clk_pll_freq *freq)
{
	freq->p = (reg >> PLL3500X_P_SHIFT) & PLL3500X_P_MASK;
	freq->m = (reg >> PLL3500X_M_SHIFT) & PLL3500X_M_MASK;
	freq->s = (reg >> PLL3500X_S_SHIFT) & PLL3500X_S_MASK;
}

static inline void extract_pll3600x_pms(u32 reg, u32 reg2,
	struct clk_pll_freq *freq)
{
	freq->p = (reg >> PLL3600X_P_SHIFT) & PLL3600X_P_MASK;
	freq->m = (reg >> PLL3600X_M_SHIFT) & PLL3600X_M_MASK;
	freq->s = (reg >> PLL3600X_S_SHIFT) & PLL3600X_S_MASK;
	freq->k = (reg2 >> PLL3600X_K_SHIFT) & PLL3600X_K_MASK;
}

static inline void set_pll3500x_pms(u32 *reg, const struct clk_pll_freq *freq)
{
	*reg &= ~((PLL3500X_P_MASK << PLL3500X_P_SHIFT) |
		(PLL3500X_M_MASK << PLL3500X_M_SHIFT) |
		(PLL3500X_S_MASK << PLL3500X_S_SHIFT));
	*reg |= ((freq->p & PLL3500X_P_MASK) << PLL3500X_P_SHIFT) |
		((freq->m & PLL3500X_M_MASK) << PLL3500X_M_SHIFT) |
		((freq->s & PLL3500X_S_MASK) << PLL3500X_S_SHIFT);
}

static inline void set_pll3600x_pmsk(u32 *reg, u32 *reg2,
	const struct clk_pll_freq *freq)
{
	*reg &= ~((PLL3600X_P_MASK << PLL3600X_P_SHIFT) |
		(PLL3600X_M_MASK << PLL3600X_M_SHIFT) |
		(PLL3600X_S_MASK << PLL3600X_S_SHIFT));
	*reg |= ((freq->p & PLL3600X_P_MASK) << PLL3600X_P_SHIFT) |
		((freq->m & PLL3600X_M_MASK) << PLL3600X_M_SHIFT) |
		((freq->s & PLL3600X_S_MASK) << PLL3600X_S_SHIFT);
	*reg2 &= ~(PLL3600X_K_MASK << PLL3600X_K_SHIFT);
	*reg2 |= (freq->k & PLL3600X_K_MASK) << PLL3600X_K_SHIFT;
}

static unsigned long find_pllclk_rate(const struct clk_pll_freq *freq_table,
	struct clk_pll_freq *freq)
{
	const struct clk_pll_freq *sel;

	for (sel = freq_table; sel->input_freq != 0; sel++) {
		if (sel->input_freq == freq->input_freq &&
			sel->p == freq->p &&
			sel->m == freq->m &&
			sel->s == freq->s &&
			sel->k == freq->k)
			return sel->output_freq;
	}
	return 0;
}

static void drime4_pll_clk_init(struct clk *clk)
{
	u32 reg, reg2;
	struct clk_pll_freq pll_freq = {0,};
	struct clk_reg *xtl_reg = &clk->u.pll.xtlsel_reg;

	reg = clk_readl(xtl_reg->reg);

	/* Check pll source xtal */
	if (reg & (xtl_reg->mask << xtl_reg->shift))
		clk->parent = &clk_xtl_dphy;

	/* Retrive p/m/s value from curent register setting */
	pll_freq.input_freq = clk_get_rate(clk->parent);
	reg = clk_readl(clk->con_reg.reg);
	if (clk->u.pll.pll_type == PLL3500X)
		extract_pll3500x_pms(reg, &pll_freq);
	else {
		reg2 = clk_readl(clk->con2_reg.reg);
		extract_pll3600x_pms(reg, reg2, &pll_freq);
	}
	clk->rate = find_pllclk_rate(clk->u.pll.freq_table,
			&pll_freq);
	clk->state = ON;

	pr_debug("%s: %s input_freq:%u con:0x%x init rate:%lu\n",
		__func__, clk->name, pll_freq.input_freq, reg,
		clk->rate);
}

static int drime4_pll_clk_setrate(struct clk *clk, unsigned long rate)
{
	unsigned long input_rate;
	const struct clk_pll_freq *sel;
	u32 reg, reg2;

	input_rate = clk_get_rate(clk->parent);

	for (sel = clk->u.pll.freq_table; sel->input_freq != 0; sel++) {
		if (sel->input_freq == input_rate &&
			sel->output_freq == rate) {
			reg = clk_readl(clk->con_reg.reg);
			if (clk->u.pll.pll_type == PLL3500X)
				set_pll3500x_pms(&reg, sel);
			else {
				reg2 = clk_readl(clk->con2_reg.reg);
				set_pll3600x_pmsk(&reg, &reg2, sel);
				clk_writel(reg2, clk->con2_reg.reg);
			}
			clk_writel(reg, clk->con_reg.reg);
			clk->rate = rate;
			return 0;
		}
	}

	return -EINVAL;
}

static long drime4_pll_clk_roundrate(struct clk *clk, unsigned long rate)
{
	u32 dist, min_dist = 100000000;
	long sel_rate = -EINVAL;

	if (clk->flags & CLK_SEL_FREQ) {
		const struct clk_pll_freq *sel = clk->u.pll.freq_table;
		for (; sel->input_freq != 0; sel++) {
			dist = abs(rate - sel->output_freq);
			if (dist < min_dist) {
				min_dist = dist;
				sel_rate = sel->output_freq;
			}
		}
	} else
		sel_rate = clk_get_rate(clk);
	return sel_rate;
}

static unsigned long drime4_pll_clk_getrate(struct clk *clk)
{
	return clk->rate;
}

static struct clk_ops drime4_clk_pll_ops = {
	.init		= drime4_pll_clk_init,
	.set_rate	= drime4_pll_clk_setrate,
	.round_rate	= drime4_pll_clk_roundrate,
	.get_rate	= drime4_pll_clk_getrate,
};

static const struct clk_sel_freq *find_source_clk(
	const struct clk_sel_freq *table,
	struct clk *src)
{
	const struct clk_sel_freq *sel;

	for (sel = table; sel->src != NULL; sel++) {
		if (sel->src == src)
			return sel;
	}
	return NULL;
}

static const struct clk_sel_freq *find_source_clk_by_value(
	const struct clk_sel_freq *table,
	u32 val)
{
	const struct clk_sel_freq *sel;

	for (sel = table; sel->src != NULL; sel++) {
		if (sel->val == val)
			return sel;
	}
	return NULL;
}

static const struct clk_sel_freq *find_source_clk_by_rate(
	const struct clk_sel_freq *table,
	unsigned long rate)
{
	const struct clk_sel_freq *sel;
	unsigned long sel_rate;

	for (sel = table; sel->src != NULL; sel++) {
		sel_rate = clk_get_rate(sel->src) / sel->div;
		if (sel_rate == rate)
			return sel;
	}
	return NULL;
}

static inline u32 get_clk_reg_val(u32 reg, struct clk_reg *c_reg)
{
	return (reg >> c_reg->shift) & c_reg->mask;
}

int drime4_peri_clk_check(struct clk *clk)
{
	u32 reg = 0;
	u32 r_reg = 0;

	if (clk->flags & CLK_GCON) {
		reg = clk_readl(clk->con_reg.reg);
		r_reg = get_clk_reg_val(reg, &clk->con_reg);
	}

	if (clk->flags & CLK_PF_GATE) {
		reg = pf_readl(clk->gate_reg.reg);
		r_reg = get_clk_reg_val(reg, &clk->gate_reg);
	}

	if (clk->flags & CLK_PF_RESET) {
		reg = pf_readl(clk->reset_reg.reg);
		r_reg = get_clk_reg_val(reg, &clk->reset_reg);
	}

	return (int)(r_reg == 0x00000001);
}

static inline void set_clk_reg_val(u32 *reg, struct clk_reg *c_reg, u32 val)
{
	*reg &= ~(c_reg->mask << c_reg->shift);
	*reg |= (val << c_reg->shift);
}

static inline void set_path_reg_val(u32 *reg, struct clk_reg *c_reg)
{
	*reg |= (c_reg->mask << c_reg->shift);
}

static inline void clear_clk_reg_val(u32 *reg, struct clk_reg *c_reg)
{
	*reg &= ~(c_reg->mask << c_reg->shift);
}

static void drime4_peri_clk_init(struct clk *clk)
{
	u32 reg, val;
	const struct clk_sel_freq *sel_freq;

	if (clk->parent)
		clk->state = clk->parent->state;

	if (clk->flags & CLK_SEL_FREQ || clk->flags & CLK_SEL_PATH) {
		reg = clk_readl(clk->freq_reg.reg);
		val = get_clk_reg_val(reg, &clk->freq_reg);
		sel_freq = find_source_clk_by_value(clk->u.mux.sel_table, val);
		if (!sel_freq) {
			pr_err("Cannot find source clock:%s\n",
				clk->name);
			return;
		}
		clk->parent = sel_freq->src;
		clk->div = sel_freq->div;
		clk->state = clk->parent->state;
	}

	if (clk->flags & CLK_GCON) {
		reg = clk_readl(clk->con_reg.reg);
		if (get_clk_reg_val(reg, &clk->con_reg))
			clk->state = ON;
		else
			clk->state = OFF;
	}

	if (clk->flags & CLK_PF_GATE) {
		reg = pf_readl(clk->gate_reg.reg);
		if (get_clk_reg_val(reg, &clk->gate_reg))
			clk->state = ON;
		else
			clk->state = OFF;
	}
	pr_debug("%s: %s init state:%d\n", __func__, clk->name,
		clk->state);
}

static int drime4_peri_clk_enable(struct clk *clk)
{
	u32 reg;
	pr_debug("%s: %s %d disable\n", __func__, clk->name, clk->flags);

	if (clk->flags & CLK_GCON) {
		reg = clk_readl(clk->con_reg.reg);
		set_clk_reg_val(&reg, &clk->con_reg, 1);
		clk_writel(reg, clk->con_reg.reg);
	}

	if (clk->flags & CLK_PF_GATE) {
		reg = pf_readl(clk->gate_reg.reg);
		set_clk_reg_val(&reg, &clk->gate_reg, 1);
		pf_writel(reg, clk->gate_reg.reg);
	}

	if (clk->flags & CLK_PF_RESET) {
		reg = pf_readl(clk->reset_reg.reg);
		set_clk_reg_val(&reg, &clk->reset_reg, 1);
		pf_writel(reg, clk->reset_reg.reg);
	}

	return 0;
}

void drime4_peri_clk_disable(struct clk *clk)
{
	u32 reg;

	pr_debug("%s: %s disable\n", __func__, clk->name);

	if (clk->flags & CLK_GCON) {
		reg = clk_readl(clk->con_reg.reg);
		clear_clk_reg_val(&reg, &clk->con_reg);
		clk_writel(reg, clk->con_reg.reg);
	}

	if (clk->flags & CLK_PF_GATE) {
		reg = pf_readl(clk->gate_reg.reg);
		clear_clk_reg_val(&reg, &clk->gate_reg);
		pf_writel(reg, clk->gate_reg.reg);
	}
}

static int drime4_peri_clk_setrate(struct clk *clk, unsigned long rate)
{
	u32 reg;
	const struct clk_sel_freq *sel_freq;

	pr_debug("%s: %s setrate:%lu\n", __func__, clk->name, rate);

	if (clk->flags & CLK_SEL_FREQ) {
		sel_freq = find_source_clk_by_rate(clk->u.mux.sel_table, rate);
		if (!sel_freq)
			return -EINVAL;

		reg = clk_readl(clk->freq_reg.reg);
		set_clk_reg_val(&reg, &clk->freq_reg, sel_freq->val);
		clk_writel(reg, clk->freq_reg.reg);

		clk->rate = rate;
		clk->div = sel_freq->div;
		clk->parent = sel_freq->src;
	}

	return 0;
}

static long drime4_peri_clk_roundrate(struct clk *clk, unsigned long rate)
{
	u32 dist, min_dist = 100000000;
	long sel_rate = -EINVAL, calc_rate;

	if (clk->flags & CLK_SEL_FREQ) {
		const struct clk_sel_freq *sel = clk->u.mux.sel_table;
		for (; sel->src != NULL; sel++) {
			calc_rate = clk_get_rate(sel->src) / sel->div;
			dist = abs(rate - calc_rate);
			if (dist < min_dist) {
				min_dist = dist;
				sel_rate = calc_rate;
			}
		}
	} else
		sel_rate = clk_get_rate(clk);

	return sel_rate;
}

static int drime4_peri_clk_setparent(struct clk *clk, struct clk *parent)
{
	u32 reg;
	const struct clk_sel_freq *sel_freq;

	if (clk->flags & CLK_SEL_PATH) {
		sel_freq = find_source_clk(clk->u.mux.sel_table, parent);
		if (!sel_freq)
			return -EINVAL;

		reg = clk_readl(clk->freq_reg.reg);
		set_clk_reg_val(&reg, &clk->freq_reg, sel_freq->val);
		clk_writel(reg, clk->freq_reg.reg);

		clk->parent = parent;
		clk->div = sel_freq->div;
	}

	return 0;
}

static struct clk_ops drime4_clk_peri_ops = {
	.init		= drime4_peri_clk_init,
	.enable		= drime4_peri_clk_enable,
	.disable	= drime4_peri_clk_disable,
	.set_rate	= drime4_peri_clk_setrate,
	.round_rate	= drime4_peri_clk_roundrate,
	.set_parent	= drime4_peri_clk_setparent,
};

static void drime4_i2s_clk_init(struct clk *clk)
{
	u32 reg;
	reg = clk_readl(clk->con_reg.reg);
	if (get_clk_reg_val(reg, &clk->con_reg))
		clk->state = ON;
	else
		clk->state = OFF;
}

static int drime4_i2s_clk_setrate(struct clk *clk, unsigned long rate)
{
	return clk_set_rate(clk->parent, rate);
}

static long drime4_i2s_clk_roundrate(struct clk *clk, unsigned long rate)
{
	return clk_round_rate(clk->parent, rate);
}

static struct clk_ops drime4_clk_i2s_ops = {
	.init		= drime4_i2s_clk_init,
	.enable		= drime4_peri_clk_enable,
	.disable	= drime4_peri_clk_disable,
	.set_rate	= drime4_i2s_clk_setrate,
	.round_rate	= drime4_i2s_clk_roundrate,
};

static struct clk_pll_freq syspll1_freq_table[] = {
	{27000000, 1066000000, 4, 158, 0, 0},
	{27000000, 1000000000, 5, 185, 0, 0},
	{27000000, 800000000, 8, 237, 0, 0},
	{27000000, 667000000, 6, 296, 1, 0},
	{24000000, 1066000000, 12, 533, 0, 0},
	{24000000, 1000000000, 3, 251, 0, 0},
	{24000000, 800000000, 3, 100, 0, 0},
	{24000000, 667000000, 4, 222, 1, 0},
	{0, 0, 0, 0, 0, 0},
};

static struct clk_pll_freq syspll2_freq_table[] = {
	{27000000, 1200000000, 9, 400, 0, 0},
	{24000000, 1200000000, 4, 200, 0, 0},
	{0, 0, 0, 0, 0, 0},
};

static struct clk_pll_freq lcdpll_freq_table[] = {
	{27000000, 1296000000, 5, 240, 0, 0},
	{24000000, 1296000000, 4, 216, 0, 0},
	{0, 0, 0, 0, 0, 0},
};

static struct clk_pll_freq armpll_freq_table[] = {
	{27000000, 800000000, 8, 237, 0, 0},
	{27000000, 700000000, 13, 674, 1, 0},
	{27000000, 600000000, 9, 400, 1, 0},
	{27000000, 500000000, 5, 185, 1, 0},
	{27000000, 400000000, 8, 237, 1, 0},
	{24000000, 800000000, 3, 100, 0, 0},
	{24000000, 700000000, 3, 175, 1, 0},
	{24000000, 600000000, 4, 200, 1, 0},
	{24000000, 500000000, 3, 125, 1, 0},
	{0, 0, 0, 0, 0, 0},
};

static struct clk_pll_freq audpll_freq_table[] = {
	{27000000, 24576000, 3, 87, 5, 24991},
	{27000000, 33868800, 3, 120, 5, 27682},
	{27000000, 36864000, 3, 131, 5, 4719},
	{27000000, 45158400, 3, 80, 4, 18455},
	{27000000, 49152000, 3, 87, 4, 24991},
	{24000000, 24576000, 2, 66, 5, -30409},
	{24000000, 33868800, 2, 90, 5, 20762},
	{24000000, 36864000, 2, 98, 5, 19923},
	{24000000, 45158400, 2, 60, 4, 13841},
	{24000000, 49152000, 2, 66, 4, -30409},
	{0, 0, 0, 0, 0, 0},
};

static struct clk syspll1_clk = {
	.parent = &clk_xtl_hdmi,
	.name = "syspll1",
	.type = CLK_PLL,
	.flags = CLK_SEL_FREQ,
	.u.pll = {
		.freq_table = syspll1_freq_table,
		.xtlsel_reg = CLK_REG(XTLSRC_SEL, 1, 1),
	},
	.ops = &drime4_clk_pll_ops,
	.con_reg = CLK_REG(SYSPLL1_CON1, 0, 0),
	.con2_reg = CLK_REG(SYSPLL1_CON2, 0, 0),
};

static struct clk syspll2_clk = {
	.parent = &clk_xtl_hdmi,
	.name = "syspll2",
	.type = CLK_PLL,
	.u.pll = {
		.freq_table = syspll2_freq_table,
		.xtlsel_reg = CLK_REG(XTLSRC_SEL, 2, 1),
	},
	.ops = &drime4_clk_pll_ops,
	.con_reg = CLK_REG(SYSPLL2_CON1, 0, 0),
	.con2_reg = CLK_REG(SYSPLL2_CON2, 0, 0),
};

static struct clk lcdpll_clk = {
	.parent = &clk_xtl_hdmi,
	.name = "lcdpll",
	.type = CLK_PLL,
	.u.pll = {
		.freq_table = lcdpll_freq_table,
		.xtlsel_reg = CLK_REG(XTLSRC_SEL, 3, 1),
	},
	.ops = &drime4_clk_pll_ops,
	.con_reg = CLK_REG(LCDPLL_CON1, 0, 0),
	.con2_reg = CLK_REG(LCDPLL_CON2, 0, 0),
};

static struct clk armpll_clk = {
	.parent = &clk_xtl_hdmi,
	.name = "armpll",
	.type = CLK_PLL,
	.flags = CLK_SEL_FREQ,
	.u.pll = {
		.freq_table = armpll_freq_table,
		.xtlsel_reg = CLK_REG(XTLSRC_SEL, 4, 1),
	},
	.ops = &drime4_clk_pll_ops,
	.con_reg = CLK_REG(ARMPLL_CON1, 0, 0),
	.con2_reg = CLK_REG(ARMPLL_CON2, 0, 0),
};

static struct clk audpll_clk = {
	.parent = &clk_xtl_hdmi,
	.name = "audpll",
	.type = CLK_PLL,
	.flags = CLK_SEL_FREQ,
	.u.pll = {
		.freq_table = audpll_freq_table,
		.xtlsel_reg = CLK_REG(XTLSRC_SEL, 0, 1),
		.pll_type = PLL3600X,
	},
	.ops = &drime4_clk_pll_ops,
	.con_reg = CLK_REG(AUDPLL_CON1, 0, 0),
	.con2_reg = CLK_REG(AUDPLL_CON2, 0, 0),
};

#define DEF_PLLDIV_CLK(p, n, d)		\
		.parent = p,		\
		.name = n,		\
		.type = CLK_PLL_DIV,	\
		.div = d,

static struct clk syspll1_fout_div2 = {
	DEF_PLLDIV_CLK(&syspll1_clk, "syspll1_fout_div2", 2)
};

static struct clk syspll1_fout_div4 = {
	DEF_PLLDIV_CLK(&syspll1_clk, "syspll1_fout_div4", 4)
};

static struct clk syspll1_fout_div8 = {
	DEF_PLLDIV_CLK(&syspll1_clk, "syspll1_fout_div8", 8)
};

static struct clk syspll1_fout_div12 = {
	DEF_PLLDIV_CLK(&syspll1_clk, "syspll1_fout_div12", 12)
};

static struct clk syspll2_fout_div3 = {
	DEF_PLLDIV_CLK(&syspll2_clk, "syspll2_fout_div3", 3)
};

static struct clk syspll2_fout_div4 = {
	DEF_PLLDIV_CLK(&syspll2_clk, "syspll2_fout_div4", 4)
};

static struct clk lcdpll_fout_div2 = {
	DEF_PLLDIV_CLK(&lcdpll_clk, "lcdpll_fout_div2", 2)
};

static struct clk lcdpll_fout_div5 = {
	DEF_PLLDIV_CLK(&lcdpll_clk, "lcdpll_fout_div5", 5)
};

static struct clk bclk = {
	DEF_PLLDIV_CLK(&syspll2_fout_div3, "bclk", 3)
};

static struct clk clk_100 = {
	DEF_PLLDIV_CLK(&syspll2_fout_div3, "clk_100", 4)
};

#define DEF_PERI_CLK(_name, _dname, _gr, _gr_s, _gr_m, _p)	\
	{							\
		.name	= _name,				\
		.lookup	= { .dev_id = _dname },			\
		.type	= CLK_PERI,				\
		.flags	= CLK_PF_GATE,				\
		.ops	= &drime4_clk_peri_ops,			\
		.gate_reg	= { _gr, _gr_s, _gr_m },	\
		.parent	= _p,					\
	}

#define DEF_PERI_CLK_RESET(_name, _dname, _gr, _gr_s, _gr_m,	\
			_rr, _rr_s, _rr_m, _p)			\
	{							\
		.name	= _name,				\
		.lookup	= { .dev_id = _dname },			\
		.type	= CLK_PERI,				\
		.flags	= CLK_PF_GATE | CLK_PF_RESET,		\
		.ops	= &drime4_clk_peri_ops,			\
		.gate_reg	= { _gr, _gr_s, _gr_m },	\
		.reset_reg	= { _rr, _rr_s, _rr_m },	\
		.parent	= _p,					\
	}

#define DEF_PERI_FIXED_CLK(_name, _dname, _p)	\
	{					\
		.name	= _name,		\
		.lookup	= { .dev_id = _dname },	\
		.type	= CLK_PERI,		\
		.ops	= &drime4_clk_peri_ops,	\
		.parent	= _p,			\
	}

#define CLK(dev, con, ck)	\
	{			\
		.dev_id = dev,	\
		.con_id = con,	\
		.clk = ck,	\
	}

static struct clk uart0_clk = {
	.parent = &clk_100,
	.name = "uart0",
	.lookup = { .dev_id = "uart0" },
	.type = CLK_PERI,
	.flags = CLK_GCON | CLK_PF_GATE,
	.ops = &drime4_clk_peri_ops,
	.con_reg = CLK_REG(GCLKCON1, 5, 1),
	.gate_reg = CLK_REG(UART_CLK_GATE, 0, 1),
};

#if 1
static struct clk uart1_clk = {
	.parent = &clk_100,
	.name = "uart1",
	.lookup = { .dev_id = "uart1" },
	.type = CLK_PERI,
	.flags = CLK_GCON | CLK_PF_GATE,
	.ops = &drime4_clk_peri_ops,
	.con_reg = CLK_REG(GCLKCON1, 23, 1),
	.gate_reg = CLK_REG(UART_CLK_GATE, 1, 1),
};
#endif

static struct clk usb3_clk = {
	.parent = &clk_100,
	.name = "usb3",
	.lookup = { .dev_id = "drime4-dwc3" },
	.type = CLK_PERI,
	.flags = CLK_GCON | CLK_PF_GATE | CLK_PF_RESET,
	.ops = &drime4_clk_peri_ops,
	.con_reg = CLK_REG(GCLKCON1, 6, 1),
	.gate_reg = CLK_REG(USB3_CLK_GATE, 0, 1),
	.reset_reg = CLK_REG(USB3_CLK_RESET, 0, 1),
};
/* GCLKSEL1 clocks */

static struct clk_sel_freq sd0_freq_table[] = {
	{0, 4, &syspll2_fout_div3},
	{1, 2, &syspll2_fout_div3},
	{2, 2, &lcdpll_fout_div2},
	{3, 1, &syspll2_fout_div3},
	{4, 1, &clk_xtl_hdmi},
	{0, 0, NULL},
};

static struct clk sd0_clk = {
	.name = "sd0",
	.lookup = { .con_id = "sd0", .dev_id = "dw_mmc_sdcard.0" },
	.type = CLK_PERI,
	.flags = CLK_SEL_FREQ | CLK_GCON | CLK_PF_GATE,
	.u.mux = { .sel_table = sd0_freq_table, },
	.ops = &drime4_clk_peri_ops,
	.freq_reg = CLK_REG(GCLKSEL1, 12, 0x7),
	.con_reg = CLK_REG(GCLKCON1, 0, 1),
	.gate_reg = CLK_REG(SD_CLK_GATE, 0, 1),
};

static struct clk_sel_freq sd1_freq_table[] = {
	{0, 8, &lcdpll_fout_div2},
	{1, 4, &syspll2_fout_div3},
	{2, 1, &clk_xtl_hdmi},
	{3, 1, &clk_xtl_hdmi},
	{0, 0, NULL},
};

static struct clk sd1_clk = {
	.name = "sd1",
	.lookup = { .con_id = "sd1", .dev_id = "dw_mmc_sdio.0" },
	.type = CLK_PERI,
	.flags = CLK_SEL_FREQ | CLK_GCON | CLK_PF_GATE,
	.u.mux = { .sel_table = sd1_freq_table, },
	.ops = &drime4_clk_peri_ops,
	.freq_reg = CLK_REG(GCLKSEL1, 16, 0x7),
	.con_reg = CLK_REG(GCLKCON1, 1, 1),
	.gate_reg = CLK_REG(SD_CLK_GATE, 1, 1),
};

static struct clk_sel_freq flash_freq_table[] = {
	{0, 6, &lcdpll_fout_div2},
	{1, 4, &syspll2_fout_div3},
	{2, 7, &lcdpll_fout_div2},
	{3, 8, &lcdpll_fout_div2},
	{4, 9, &lcdpll_fout_div2},
	{5, 10, &lcdpll_fout_div2},
	{6, 16, &lcdpll_fout_div2},
	{7, 18, &lcdpll_fout_div2},
	{8, 5, &lcdpll_fout_div2},
	{8, 5, &lcdpll_fout_div2},
	{0, 0, NULL},
};

static struct clk flash_clk = {
	.name = "flash",
	.type = CLK_PERI,
	.flags = CLK_SEL_FREQ | CLK_GCON,
	.u.mux = { .sel_table = flash_freq_table, },
	.ops = &drime4_clk_peri_ops,
	.freq_reg = CLK_REG(GCLKSEL1, 8, 0xF),
	.con_reg = CLK_REG(GCLKCON1, 7, 1),
};

static struct clk_sel_freq ddr_freq_table[] = {
	{0, 1, &syspll1_fout_div2},
	{1, 16, &lcdpll_fout_div2},
	{2, 1, &syspll1_fout_div4},
	{0, 0, NULL},
};

static struct clk ddr_clk = {
	.name = "ddr",
	.type = CLK_PERI,
	.flags = CLK_SEL_FREQ | CLK_GCON,
	.u.mux = { .sel_table = ddr_freq_table, },
	.ops = &drime4_clk_peri_ops,
	.freq_reg = CLK_REG(GCLKSEL1, 1, 3),
	.con_reg = CLK_REG(GCLKCMD1, 0, 1),
};

static struct clk_sel_freq mbus_freq_table[] = {
	{0, 2, &ddr_clk},
	{1, 3, &lcdpll_fout_div2},
	{0, 0, NULL},
};

static struct clk mclk = {
	.name = "mclk",
	.type = CLK_PERI,
	.flags = CLK_SEL_FREQ | CLK_GCON,
	.u.mux = { .sel_table = mbus_freq_table, },
	.ops = &drime4_clk_peri_ops,
	.freq_reg = CLK_REG(GCLKSEL1, 0, 1),
	.con_reg = CLK_REG(GCLKCMD1, 0, 1),
};

static struct clk peri81_clk = {
	.parent = &lcdpll_fout_div2,
	.name = "peri81",
	.type = CLK_PERI,
	.ops = &drime4_clk_peri_ops,
	.flags = CLK_GCON,
	.con_reg = CLK_REG(GCLKCON1, 29, 1),
	.div = 8,
};

static struct clk peri100_clk = {
	.parent = &syspll2_fout_div3,
	.name = "peri100",
	.type = CLK_PERI,
	.ops = &drime4_clk_peri_ops,
	.flags = CLK_GCON,
	.con_reg = CLK_REG(GCLKCON1, 30, 1),
	.div = 4,
};

static struct clk peri27_clk = {
	.parent = &clk_xtl_hdmi,
	.name = "peri27",
	.type = CLK_PERI,
	.ops = &drime4_clk_peri_ops,
	.flags = CLK_GCON,
	.con_reg = CLK_REG(GCLKCON1, 30, 1),
};

/* GCLKSEL2 clocks */
static struct clk_sel_freq ipcs_freq_table[] = {
	{0, 1, &lcdpll_fout_div5},
	{1, 2, &syspll2_fout_div3},
	{2, 3, &syspll2_fout_div3},
	{3, 8, &syspll2_fout_div3},
	{4, 3, &lcdpll_fout_div2},
	{5, 4, &lcdpll_fout_div2},
	{6, 6, &lcdpll_fout_div2},
	{7, 7, &lcdpll_fout_div2},
	{8, 8, &lcdpll_fout_div2},
	{9, 9, &lcdpll_fout_div2},
	{10, 10, &lcdpll_fout_div2},
	{11, 11, &lcdpll_fout_div2},
	{12, 15, &lcdpll_fout_div2},
	{13, 18, &lcdpll_fout_div2},
	{14, 1, &clk_xtl_hdmi},
	{15, 36, &lcdpll_fout_div2},
	/*{16, 1, &mclk},*/
	{17, 2, &syspll2_fout_div4},
	{18, 4, &syspll2_fout_div3},
	/*{19, 1, &syspll1_fout_div8},*/
	/*{20, 1, &syspll1_fout_div12},*/
	{0, 0, NULL},
};

static struct clk ipcs_clk = {
	.name = "ipcs",
	.lookup = { .dev_id = "d4_ipcs" },
	.type = CLK_PERI,
	.ops = &drime4_clk_peri_ops,
	.flags = CLK_SEL_FREQ | CLK_GCON,
	.u.mux = { .sel_table = ipcs_freq_table, },
	.freq_reg = CLK_REG(GCLKSEL2, 24, 0x1F),
	.con_reg = CLK_REG(GCLKCON1, 18, 1),
};

static struct clk ipcm_clk = {
	.name = "ipcm",
	.lookup = { .dev_id = "d4_ipcm" },
	.type = CLK_PERI,
	.ops = &drime4_clk_peri_ops,
	.flags = CLK_SEL_FREQ | CLK_GCON,
	.u.mux = { .sel_table = ipcs_freq_table, },
	.freq_reg = CLK_REG(GCLKSEL2, 16, 0x1F),
	.con_reg = CLK_REG(GCLKCON1, 17, 1),
};

static struct clk pp_clk = {
	.name = "pp",
	.type = CLK_PERI,
	.lookup = { .dev_id = "d4_pp_core" },
	.ops = &drime4_clk_peri_ops,
	.flags = CLK_SEL_FREQ | CLK_GCON,
	.u.mux = { .sel_table = ipcs_freq_table, },
	.freq_reg = CLK_REG(GCLKSEL2, 8, 0x1F),
	.con_reg = CLK_REG(GCLKCON1, 16, 1),
};

static struct clk_sel_freq jpeg_freq_table[] = {
	{0, 3, &lcdpll_fout_div2},
	{1, 2, &syspll2_fout_div3},
	{2, 4, &lcdpll_fout_div2},
	{3, 3, &syspll2_fout_div3},
	{4, 4, &syspll2_fout_div3},
	{5, 8, &lcdpll_fout_div2},
	{6, 8, &syspll2_fout_div3},
	{7, 8, &clk_xtl_hdmi},
	{8, 1, &mclk},
};

static struct clk jpeg_clk = {
	.name = "jpeg",
	.lookup = { .dev_id = "d4_jpeg" },
	.type = CLK_PERI,
	.ops = &drime4_clk_peri_ops,
	.flags = CLK_SEL_FREQ | CLK_GCON,
	.u.mux = { .sel_table = jpeg_freq_table, },
	.freq_reg = CLK_REG(GCLKSEL2, 4, 0xF),
	.con_reg = CLK_REG(GCLKCON1, 9, 1),
};

static struct clk_sel_freq cod_freq_table[] = {
	{0, 1, &lcdpll_fout_div5},		/* 260Mhz */
	{1, 3, &lcdpll_fout_div2},		/* 216Mhz */
	{2, 2, &syspll2_fout_div3},		/* 200Mhz */
	{3, 4, &lcdpll_fout_div2},		/* 162Mhz */
	{4, 5, &lcdpll_fout_div2},		/* 130Mhz */
	{5, 4, &syspll2_fout_div3},		/* 100Mhz */
	{6, 8, &lcdpll_fout_div2},		/* 81Mhz */
	{7, 8, &syspll2_fout_div3},		/* 50Mhz */
	{8, 8, &clk_xtl_hdmi},			/* 27Mhz */
	{9, 1, &syspll2_fout_div4},		/* 300Mhz */
	{10, 1, &mclk},				/* MBUS clock */
	{0, 0, NULL},
};

static struct clk cod_clk = {
	.name = "cod",
	.lookup = { .dev_id = "s5p-mfc", .con_id = "mfc" },
	.type = CLK_PERI,
	.ops = &drime4_clk_peri_ops,
	.flags = CLK_SEL_FREQ | CLK_GCON,
	.u.mux = { .sel_table = cod_freq_table, },
	.freq_reg = CLK_REG(GCLKSEL2, 0, 0xF),
	.con_reg = CLK_REG(GCLKCON1, 8, 1),
};

static struct clk srp_clk = {
	.name = "srp",
	.lookup = { .dev_id = "d4_srp"},
	.type = CLK_PERI,
	.ops = &drime4_clk_peri_ops,
	.flags = CLK_SEL_FREQ | CLK_GCON,
	.u.mux = { .sel_table = cod_freq_table, },
	.freq_reg = CLK_REG(GCLKSEL2, 0, 0xF),
	.con_reg = CLK_REG(GCLKCON1, 8, 1),
};


static struct clk_sel_freq be_freq_table[] = {
	{0, 2, &syspll2_fout_div3},
	{1, 4, &lcdpll_fout_div2},
	{2, 3, &syspll2_fout_div3},
	{3, 4, &syspll2_fout_div3},
	{4, 8, &lcdpll_fout_div2},
	{5, 8, &syspll2_fout_div3},
	{6, 8, &clk_xtl_hdmi},
	{10, 1, &mclk},
	{0, 0, NULL},
};


static struct clk_sel_freq dp_freq_table[] = {
	{0, 1, &lcdpll_fout_div5},
	{1, 3, &lcdpll_fout_div2},
	{2, 2, &syspll2_fout_div3},
	{3, 4, &lcdpll_fout_div2},
	{4, 3, &syspll2_fout_div3},
	{5, 4, &syspll2_fout_div3},
	{6, 8, &lcdpll_fout_div2},
	{7, 8, &syspll2_fout_div3},
	{8, 1, &clk_xtl_hdmi},
	{10, 1, &mclk},
	{0, 0, NULL},
};

#ifdef CONFIG_SCORE_SUSPEND
struct clk dp_clk = {
#else
static struct clk dp_clk = {
#endif
	.name = "dp",
	.type = CLK_PERI,
	.ops = &drime4_clk_peri_ops,
	.flags = CLK_SEL_FREQ | CLK_GCON,
	.u.mux = { .sel_table = dp_freq_table, },
	.freq_reg = CLK_REG(GCLKSEL3, 12, 0xF),
	.con_reg = CLK_REG(GCLKCON1, 22, 1),
};

static struct clk gpu_clk = {
	.name = "gpu",
	.type = CLK_PERI,
	.ops = &drime4_clk_peri_ops,
	.flags = CLK_SEL_FREQ | CLK_GCON,
	.u.mux = { .sel_table = dp_freq_table, },
	.freq_reg = CLK_REG(GCLKSEL3, 8, 0xF),
	.con_reg = CLK_REG(GCLKCON1, 21, 1),
};

static struct clk be_clk = {
	.name = "be",
	.type = CLK_PERI,
	.ops = &drime4_clk_peri_ops,
	.flags = CLK_SEL_FREQ | CLK_GCON,
	.u.mux = { .sel_table = be_freq_table, },
	.freq_reg = CLK_REG(GCLKSEL3, 4, 0xF),
	.con_reg = CLK_REG(GCLKCON1, 20, 1),
};

static struct clk ep_clk = {
	.name = "ep",
	.type = CLK_PERI,
	.ops = &drime4_clk_peri_ops,
	.flags = CLK_SEL_FREQ | CLK_GCON,
	.u.mux = { .sel_table = dp_freq_table, },
	.freq_reg = CLK_REG(GCLKSEL3, 0, 0xF),
	.con_reg = CLK_REG(GCLKCON1, 19, 1),
};

static struct clk i2s_clk = {
	.parent = &audpll_clk,
	.name = "i2s",
	.type = CLK_AUD,
	.ops = &drime4_clk_i2s_ops,
	.flags = CLK_SEL_FREQ | CLK_GCON,
	.con_reg = CLK_REG(GCLKCON1, 2, 1),
};

static struct clk_sel_freq ext_tg_freq_table[] = {
	{0, 8, &lcdpll_fout_div2},
	{1, 9, &lcdpll_fout_div2},
	{2, 10, &lcdpll_fout_div2},
	{3, 11, &lcdpll_fout_div2},
	{4, 12, &lcdpll_fout_div2},
	{5, 16, &lcdpll_fout_div2},
	{6, 18, &lcdpll_fout_div2},
	{7, 24, &lcdpll_fout_div2},
	{8, 30, &lcdpll_fout_div2},
	{9, 36, &lcdpll_fout_div2},
	{10, 42, &lcdpll_fout_div2},
	{11, 48, &lcdpll_fout_div2},
	{12, 54, &lcdpll_fout_div2},
	{13, 128, &lcdpll_fout_div2},
	{14, 48, &lcdpll_fout_div2},
	{15, 48, &lcdpll_fout_div2},
	{0, 0, NULL},
};

static struct clk ext_tg_clk = {
	.name = "ext_tg",
	.type = CLK_PERI,
	.ops = &drime4_clk_peri_ops,
	.flags = CLK_SEL_FREQ,
	.u.mux = { .sel_table = ext_tg_freq_table, },
	.freq_reg = CLK_REG(GCLKSEL4, 20, 0xF),
};

static struct clk byte_clk0 = {
	.name = "byte_clk0",
};

static struct clk inv_byte_clk0 = {
	.name = "inv_byte_clk0",
};

static struct clk byte_clk1 = {
	.name = "byte_clk1",
};

static struct clk inv_byte_clk1 = {
	.name = "inv_byte_clk1",
};

static struct clk byte_clk2 = {
	.name = "byte_clk2",
};

static struct clk inv_byte_clk2 = {
	.name = "inv_byte_clk2",
};

static struct clk byte_clk3 = {
	.name = "byte_clk3",
};

static struct clk inv_byte_clk3 = {
	.name = "inv_byte_clk3",
};

static struct clk ccd_clk1 = {
	.name = "ccd_clk1",
};

static struct clk inv_ccd_clk1 = {
	.name = "inv_ccd_clk1",
};

static struct clk ccd_clk2 = {
	.name = "ccd_clk2",
};

static struct clk inv_ccd_clk2 = {
	.name = "inv_ccd_clk2",
};

static struct clk inv_ext_tg_clk = {
	.name = "inv_ext_tg",
};

static struct clk_sel_freq ssif_path_table[] = {
	{0, 1, &byte_clk0},
	{1, 1, &inv_byte_clk0},
	{2, 1, &byte_clk1},
	{3, 1, &inv_byte_clk1},
	{4, 1, &byte_clk2},
	{5, 1, &inv_byte_clk2},
	{6, 1, &byte_clk3},
	{7, 1, &inv_byte_clk3},
	{8, 1, &ccd_clk1},
	{9, 1, &inv_ccd_clk1},
	{10, 1, &ccd_clk2},
	{11, 1, &inv_ccd_clk2},
	{12, 1, &ext_tg_clk},
	{13, 1, &inv_ext_tg_clk},
	{0, 0, NULL},
};

static struct clk ssif_sg_clk = {
	.name = "ssif_sg",
	.type = CLK_PERI,
	.ops = &drime4_clk_peri_ops,
	.flags = CLK_GCON | CLK_SEL_PATH,
	.u.mux = { .sel_table = ssif_path_table, },
	.freq_reg = CLK_REG(GCLKSEL4, 16, 0xF),
	.con_reg = CLK_REG(GCLKCON1, 14, 1),
};

static struct clk ssif_s3_clk = {
	.name = "ssif_s3",
	.type = CLK_PERI,
	.ops = &drime4_clk_peri_ops,
	.flags = CLK_GCON | CLK_SEL_PATH,
	.u.mux = { .sel_table = ssif_path_table, },
	.freq_reg = CLK_REG(GCLKSEL4, 12, 0xF),
	.con_reg = CLK_REG(GCLKCON1, 13, 1),
};

static struct clk ssif_s2_clk = {
	.name = "ssif_s2",
	.type = CLK_PERI,
	.ops = &drime4_clk_peri_ops,
	.flags = CLK_GCON | CLK_SEL_PATH,
	.u.mux = { .sel_table = ssif_path_table, },
	.freq_reg = CLK_REG(GCLKSEL4, 8, 0xF),
	.con_reg = CLK_REG(GCLKCON1, 12, 1),
};

static struct clk ssif_s1_clk = {
	.name = "ssif_s1",
	.type = CLK_PERI,
	.ops = &drime4_clk_peri_ops,
	.flags = CLK_GCON | CLK_SEL_PATH,
	.u.mux = { .sel_table = ssif_path_table, },
	.freq_reg = CLK_REG(GCLKSEL4, 4, 0xF),
	.con_reg = CLK_REG(GCLKCON1, 11, 1),
};

static struct clk ssif_s0_clk = {
	.name = "ssif_s0",
	.type = CLK_PERI,
	.ops = &drime4_clk_peri_ops,
	.flags = CLK_GCON | CLK_SEL_PATH,
	.u.mux = { .sel_table = ssif_path_table, },
	.freq_reg = CLK_REG(GCLKSEL4, 0, 0xF),
	.con_reg = CLK_REG(GCLKCON1, 10, 1),
};

static struct clk ssif_sensor_clk = {
	.parent = &ext_tg_clk,
	.name = "ssif_sensor",
	.type = CLK_PERI,
	.ops = &drime4_clk_peri_ops,
	.flags = CLK_GCON,
	.con_reg = CLK_REG(GCLKCON1, 15, 1),
};

static struct clk_sel_freq lcd_freq_table[] = {
	{0, 160, &lcdpll_fout_div2},
	{1, 150, &lcdpll_fout_div2},
	{2, 144, &lcdpll_fout_div2},
	{3, 135, &lcdpll_fout_div2},
	{4, 130, &lcdpll_fout_div2},
	{5, 128, &lcdpll_fout_div2},
	{6, 125, &lcdpll_fout_div2},
	{7, 120, &lcdpll_fout_div2},
	{8, 114, &lcdpll_fout_div2},
	{9, 108, &lcdpll_fout_div2},
	{10, 100, &lcdpll_fout_div2},
	{11, 96, &lcdpll_fout_div2},
	{12, 90, &lcdpll_fout_div2},
	{13, 81, &lcdpll_fout_div2},
	{14, 48, &lcdpll_fout_div2},
	{15, 45, &lcdpll_fout_div2},
	{16, 40, &lcdpll_fout_div2},
	{17, 36, &lcdpll_fout_div2},
	{18, 32, &lcdpll_fout_div2},
	{19, 30, &lcdpll_fout_div2},
	{20, 27, &lcdpll_fout_div2},
	{21, 26, &lcdpll_fout_div2},
	{22, 25, &lcdpll_fout_div2},
	{23, 24, &lcdpll_fout_div2},
	{24, 22, &lcdpll_fout_div2},
	{25, 21, &lcdpll_fout_div2},
	{26, 20, &lcdpll_fout_div2},
	{27, 19, &lcdpll_fout_div2},
	{28, 18, &lcdpll_fout_div2},
	{29, 16, &lcdpll_fout_div2},
	{30, 15, &lcdpll_fout_div2},
	{31, 14, &lcdpll_fout_div2},
	{32, 13, &lcdpll_fout_div2},
	{33, 12, &lcdpll_fout_div2},
	{34, 11, &lcdpll_fout_div2},
	{35, 10, &lcdpll_fout_div2},
	{36, 9, &lcdpll_fout_div2},
	{37, 8, &lcdpll_fout_div2},
	{38, 7, &lcdpll_fout_div2},
	{39, 6, &lcdpll_fout_div2},
	{0, 0, NULL},
};

static struct clk mlcd_ext_evf_clk = {
	.name = "mlcd_ext_evf",
};

static struct clk mlcd_ext_clk = {
	.name = "mlcd_ext",
};

static struct clk mlcd_sel1_clk = {
	.name = "mlcd_sel1",
	.type = CLK_PERI,
	.ops = &drime4_clk_peri_ops,
	.flags = CLK_SEL_FREQ,
	.u.mux = { .sel_table = lcd_freq_table, },
	.freq_reg = CLK_REG(GCLKSEL5, 0, 0x3F),
};

static struct clk_sel_freq mlcd_parent_table[] = {
	{0, 1, &mlcd_sel1_clk},
	{1, 1, &mlcd_ext_clk},
	{2, 1, &mlcd_ext_evf_clk},
	{0, 0, NULL},
};

static struct clk mlcd_sel_clk = {
	.name = "mlcd_sel",
	.type = CLK_PERI,
	.ops = &drime4_clk_peri_ops,
	.flags = CLK_SEL_PATH,
	.u.mux = { .sel_table = mlcd_parent_table, },
	.freq_reg = CLK_REG(GCLKSEL5, 11, 0x3),
};

static struct clk mlcd_div2_clk = {
	.parent = &mlcd_sel_clk,
	.name = "mlcd_div2",
	.type = CLK_PERI,
	.ops = &drime4_clk_peri_ops,
	.div = 2,
};

static struct clk mlcd_div3_clk = {
	.parent = &mlcd_sel_clk,
	.name = "mlcd_div3",
	.type = CLK_PERI,
	.ops = &drime4_clk_peri_ops,
	.div = 3,
};

static struct clk mlcd_div4_clk = {
	.parent = &mlcd_sel_clk,
	.name = "mlcd_div4",
	.type = CLK_PERI,
	.ops = &drime4_clk_peri_ops,
	.div = 4,
};

static struct clk_sel_freq mlcd_sel2_table[] = {
	{0, 1, &mlcd_div2_clk},
	{1, 1, &mlcd_div3_clk},
	{2, 1, &mlcd_div4_clk},
	{0, 0, NULL},
};

static struct clk mlcd_sel2_clk = {
	.name = "mlcd_sel2",
	.type = CLK_PERI,
	.ops = &drime4_clk_peri_ops,
	.flags = CLK_SEL_PATH,
	.u.mux = { .sel_table = mlcd_sel2_table, },
	.freq_reg = CLK_REG(GCLKSEL5, 6, 0x3),
};

static struct clk_sel_freq mlcd_path_table[] = {
	{0, 1, &mlcd_sel2_clk},
	{1, 1, &mlcd_sel_clk},
	{0, 0, NULL},
};

static struct clk mlcd_clk = {
	.name = "mlcd",
	.type = CLK_PERI,
	.ops = &drime4_clk_peri_ops,
	.flags = CLK_GCON | CLK_SEL_PATH,
	.u.mux = { .sel_table = mlcd_path_table, },
	.freq_reg = CLK_REG(GCLKSEL5, 10, 1),
	.con_reg = CLK_REG(GCLKCON1, 24, 1),
};

static struct clk mlcd_out_clk = {
	.name = "mlcd_out",
	.type = CLK_PERI,
	.ops = &drime4_clk_peri_ops,
	.flags = CLK_GCON | CLK_SEL_PATH,
	.u.mux = { .sel_table = mlcd_path_table, },
	.freq_reg = CLK_REG(GCLKSEL5, 9, 1),
	.con_reg = CLK_REG(GCLKCON1, 25, 1),
};

static struct clk slcd_ext_clk = {
	.name = "slcd_ext",
};

static struct clk slcd_sel1_clk = {
	.name = "slcd_sel1",
	.type = CLK_PERI,
	.ops = &drime4_clk_peri_ops,
	.flags = CLK_SEL_FREQ,
	.u.mux = { .sel_table = lcd_freq_table, },
	.freq_reg = CLK_REG(GCLKSEL5, 13, 0x1F),
};

static struct clk_sel_freq slcd_parent_table[] = {
	{0, 1, &slcd_sel1_clk},
	{1, 1, &slcd_ext_clk},
	{2, 1, &mlcd_ext_evf_clk},
	{0, 0, NULL},
};

static struct clk slcd_sel_clk = {
	.name = "slcd_sel",
	.type = CLK_PERI,
	.ops = &drime4_clk_peri_ops,
	.flags = CLK_SEL_PATH,
	.u.mux = { .sel_table = slcd_parent_table, },
	.freq_reg = CLK_REG(GCLKSEL5, 24, 0x3),
};

static struct clk slcd_div2_clk = {
	.parent = &slcd_sel_clk,
	.name = "slcd_div2",
	.type = CLK_PERI,
	.ops = &drime4_clk_peri_ops,
	.div = 2,
};

static struct clk slcd_div3_clk = {
	.parent = &slcd_sel_clk,
	.name = "slcd_div3",
	.type = CLK_PERI,
	.ops = &drime4_clk_peri_ops,
	.div = 3,
};

static struct clk slcd_div4_clk = {
	.parent = &slcd_sel_clk,
	.name = "slcd_div4",
	.type = CLK_PERI,
	.ops = &drime4_clk_peri_ops,
	.div = 4,
};

static struct clk_sel_freq slcd_sel2_table[] = {
	{0, 1, &slcd_div2_clk},
	{1, 1, &slcd_div3_clk},
	{2, 1, &slcd_div4_clk},
	{0, 0, NULL},
};

static struct clk slcd_sel2_clk = {
	.name = "slcd_sel2",
	.type = CLK_PERI,
	.ops = &drime4_clk_peri_ops,
	.flags = CLK_SEL_PATH,
	.u.mux = { .sel_table = slcd_sel2_table, },
	.freq_reg = CLK_REG(GCLKSEL5, 19, 0x3),
};

static struct clk_sel_freq slcd_path_table[] = {
	{0, 1, &slcd_sel2_clk},	/* Bypass */
	{1, 1, &slcd_sel_clk},
	{0, 0, NULL},
};

static struct clk slcd_clk = {
	.name = "slcd",
	.type = CLK_PERI,
	.ops = &drime4_clk_peri_ops,
	.flags = CLK_GCON | CLK_SEL_PATH,
	.u.mux = { .sel_table = slcd_path_table, },
	.freq_reg = CLK_REG(GCLKSEL5, 23, 1),
	.con_reg = CLK_REG(GCLKCON1, 26, 1),
};

static struct clk slcd_out_clk = {
	.name = "slcd_out",
	.type = CLK_PERI,
	.ops = &drime4_clk_peri_ops,
	.flags = CLK_GCON | CLK_SEL_PATH,
	.u.mux = { .sel_table = slcd_path_table, },
	.freq_reg = CLK_REG(GCLKSEL5, 22, 1),
	.con_reg = CLK_REG(GCLKCON1, 27, 1),
};

static struct clk *drime4_global_clocks[] = {
	&uart0_clk,
	&uart1_clk,
	&usb3_clk,
	&sd0_clk,
	&sd1_clk,
	&flash_clk,
	&ddr_clk,
	&mclk,
	&peri81_clk,
	&peri100_clk,
	&peri27_clk,
	&ipcs_clk,
	&ipcm_clk,
	&pp_clk,
	&jpeg_clk,
	&cod_clk,
	&dp_clk,
	&gpu_clk,
	&be_clk,
	&ep_clk,
	&i2s_clk,
	&mlcd_ext_evf_clk,
	&mlcd_ext_clk,
	&mlcd_sel1_clk,
	&mlcd_sel_clk,
	&mlcd_div2_clk,
	&mlcd_div3_clk,
	&mlcd_div4_clk,
	&mlcd_sel2_clk,
	&mlcd_clk,
	&mlcd_out_clk,
	&slcd_ext_clk,
	&slcd_sel1_clk,
	&slcd_sel_clk,
	&slcd_div2_clk,
	&slcd_div3_clk,
	&slcd_div4_clk,
	&slcd_sel2_clk,
	&slcd_clk,
	&slcd_out_clk,
	&ext_tg_clk,
	&byte_clk0,
	&inv_byte_clk0,
	&byte_clk1,
	&inv_byte_clk1,
	&byte_clk2,
	&inv_byte_clk2,
	&byte_clk3,
	&inv_byte_clk3,
	&ccd_clk1,
	&inv_ccd_clk1,
	&ccd_clk2,
	&inv_ccd_clk2,
	&inv_ext_tg_clk,
	&ssif_sg_clk,
	&ssif_s3_clk,
	&ssif_s2_clk,
	&ssif_s1_clk,
	&ssif_s0_clk,
	&ssif_sensor_clk,
	&srp_clk,
};

/* Platform Peripherals Clock */
static struct clk drime4_peri_clocks[] = {
	DEF_PERI_CLK("pwm", NULL, PWM_CLK_GATE, 0, 1, &bclk),
	DEF_PERI_CLK("nand", "drime4-nand", NAND_CLK_GATE, 0, 1, &bclk),
	DEF_PERI_CLK("ptc0", "drime4-ptc.0", PTC_CLK_GATE, 0, 1, &bclk),
	DEF_PERI_CLK("ptc1", "drime4-ptc.1", PTC_CLK_GATE, 1, 1, &bclk),
	DEF_PERI_CLK("gpio0", "gpio0", GPIO_CLK_GATE, 0, 1, &bclk),
	DEF_PERI_CLK("gpio1", "gpio1", GPIO_CLK_GATE, 1, 1, &bclk),
	DEF_PERI_CLK("gpio2", "gpio2",  GPIO_CLK_GATE, 2, 1, &bclk),
	DEF_PERI_CLK("gpio3", "gpio3", GPIO_CLK_GATE, 3, 1, &bclk),
	DEF_PERI_CLK("gpio4", "gpio4", GPIO_CLK_GATE, 4, 1, &bclk),
	DEF_PERI_CLK("gpio5", "gpio5", GPIO_CLK_GATE, 5, 1, &bclk),
	DEF_PERI_CLK("gpio6", "gpio6", GPIO_CLK_GATE, 6, 1, &bclk),
	DEF_PERI_CLK("gpio7", "gpio7", GPIO_CLK_GATE, 7, 1, &bclk),
	DEF_PERI_CLK("gpio8", "gpio8", GPIO_CLK_GATE, 8, 1, &bclk),
	DEF_PERI_CLK("gpio9", "gpio9", GPIO_CLK_GATE, 9, 1, &bclk),
	DEF_PERI_CLK("gpio10", "gpio10", GPIO_CLK_GATE, 10, 1, &bclk),
	DEF_PERI_CLK("gpio11", "gpio11", GPIO_CLK_GATE, 11, 1, &bclk),
	DEF_PERI_CLK("gpio12", "gpio12", GPIO_CLK_GATE, 12, 1, &bclk),
	DEF_PERI_CLK("gpio13", "gpio13", GPIO_CLK_GATE, 13, 1, &bclk),
	DEF_PERI_CLK("gpio14", "gpio14", GPIO_CLK_GATE, 14, 1, &bclk),
	DEF_PERI_CLK("gpio15", "gpio15", GPIO_CLK_GATE, 15, 1, &bclk),
	DEF_PERI_CLK("gpio16", "gpio16", GPIO_CLK_GATE, 16, 1, &bclk),
	DEF_PERI_CLK("gpio17", "gpio17", GPIO_CLK_GATE, 17, 1, &bclk),
	DEF_PERI_CLK("gpio18", "gpio18", GPIO_CLK_GATE, 18, 1, &bclk),
	DEF_PERI_CLK("gpio19", "gpio19", GPIO_CLK_GATE, 19, 1, &bclk),
	DEF_PERI_CLK("gpio20", "gpio20", GPIO_CLK_GATE, 20, 1, &bclk),
	DEF_PERI_CLK("gpio21", "gpio21", GPIO_CLK_GATE, 21, 1, &bclk),
	DEF_PERI_CLK("gpio22", "gpio22", GPIO_CLK_GATE, 22, 1, &bclk),
	DEF_PERI_CLK("gpio23", "gpio23", GPIO_CLK_GATE, 23, 1, &bclk),
	DEF_PERI_CLK("gpio24", "gpio24", GPIO_CLK_GATE, 24, 1, &bclk),
	DEF_PERI_CLK("gpio25", "gpio25", GPIO_CLK_GATE, 25, 1, &bclk),
	DEF_PERI_CLK("gpio26", "gpio26", GPIO_CLK_GATE, 26, 1, &bclk),
	DEF_PERI_CLK("gpio27", "gpio27", GPIO_CLK_GATE, 27, 1, &bclk),
	DEF_PERI_CLK("spi0", "drime4_spi.0", SPI_CLK_GATE, 0, 1, &bclk),
	DEF_PERI_CLK("spi1", "drime4_spi.1", SPI_CLK_GATE, 1, 1, &bclk),
	DEF_PERI_CLK("spi2", "drime4_spi.2", SPI_CLK_GATE, 2, 1, &bclk),
	DEF_PERI_CLK("spi3", "drime4_spi.3", SPI_CLK_GATE, 3, 1, &bclk),
	DEF_PERI_CLK("spi4", "drime4_spi.4", SPI_CLK_GATE, 4, 1, &bclk),
	DEF_PERI_CLK("spi5", "drime4_spi.5", SPI_CLK_GATE, 5, 1, &bclk),
	DEF_PERI_CLK("spi6", "drime4_spi.6", SPI_CLK_GATE, 6, 1, &bclk),
	DEF_PERI_CLK("spi7", "drime4_spi.7", SPI_CLK_GATE, 7, 1, &bclk),
//	DEF_PERI_CLK("uart1", "drime4_uart.1", UART_CLK_GATE, 1, 1, &bclk),
#ifdef CONFIG_DRIME4_SI2C0
	DEF_PERI_CLK_RESET("i2c0", "drime4-si2c.0", I2C_CLK_GATE, 0, 1,
				I2C_CLK_RESET, 0, 1, &bclk),
#else
	DEF_PERI_CLK_RESET("i2c0", "drime4-i2c.0", I2C_CLK_GATE, 0, 1,
				I2C_CLK_RESET, 0, 1, &bclk),
#endif
	DEF_PERI_CLK_RESET("i2c1", "drime4-i2c.1", I2C_CLK_GATE, 1, 1,
				I2C_CLK_RESET, 1, 1, &bclk),
	DEF_PERI_CLK_RESET("i2c2", "drime4-i2c.2", I2C_CLK_GATE, 2, 1,
				I2C_CLK_RESET, 2, 1, &bclk),
	DEF_PERI_CLK_RESET("i2c3", "drime4-i2c.3", I2C_CLK_GATE, 3, 1,
				I2C_CLK_RESET, 3, 1, &bclk),
#ifdef CONFIG_DRIME4_SI2C4
	DEF_PERI_CLK_RESET("i2c4", "drime4-si2c.4", I2C_CLK_GATE, 4, 1,
				I2C_CLK_RESET, 4, 1, &bclk),
#else
	DEF_PERI_CLK_RESET("i2c4", "drime4-i2c.4", I2C_CLK_GATE, 4, 1,
				I2C_CLK_RESET, 4, 1, &bclk),
#endif
	DEF_PERI_CLK_RESET("i2c5", "drime4-i2c.5", I2C_CLK_GATE, 5, 1,
				I2C_CLK_RESET, 5, 1, &bclk),
	DEF_PERI_CLK_RESET("i2c6", "drime4-i2c.6", I2C_CLK_GATE, 6, 1,
				I2C_CLK_RESET, 6, 1, &bclk),
	DEF_PERI_CLK_RESET("i2c7", "drime4-i2c.7", I2C_CLK_GATE, 7, 1,
				I2C_CLK_RESET, 7, 1, &bclk),
	DEF_PERI_CLK("i2s0", "drime4-i2s.0", I2S_CLK_GATE, 0, 1, &i2s_clk),
	DEF_PERI_CLK("i2s1", "drime4-i2s.1", I2S_CLK_GATE, 1, 1, &i2s_clk),
	DEF_PERI_CLK("i2s2", "drime4-i2s.2", I2S_CLK_GATE, 2, 1, &i2s_clk),
	DEF_PERI_CLK("adc", "drime4-adc", ADC_CLK_GATE, 0, 1, &bclk),
	DEF_PERI_CLK("timer", "drime4-timer", TIMER_CLK_GATE, 0, 1, &bclk),
	DEF_PERI_CLK("efs", "drime4-efs", EFS_CLK_GATE, 0, 1, &bclk),
	DEF_PERI_CLK("hsic", "drime4-hsic", HSIC_CLK_GATE, 0, 1, &bclk),
	DEF_PERI_CLK("ata", "drime4-ata", ATA_CLK_GATE, 0, 1, &bclk),
	DEF_PERI_FIXED_CLK("watchdog", "drime4-wdt", &bclk),
};

static struct clk *drime4_ptr_clocks[] = {
	&clk_xtl_hdmi,
	&clk_xtl_dphy,
	&syspll1_clk,
	&syspll2_clk,
	&lcdpll_clk,
	&armpll_clk,
	&audpll_clk,
	&syspll1_fout_div2,
	&syspll1_fout_div4,
	&syspll1_fout_div8,
	&syspll1_fout_div12,
	&syspll2_fout_div3,
	&syspll2_fout_div4,
	&lcdpll_fout_div2,
	&lcdpll_fout_div5,
	&bclk,
	&clk_100,
};

static struct clk dummy_dma = {
};

static struct clk dummy_apb_pclk = {
};

static struct clk_lookup lookups[] = {
	CLK(NULL, "apb_pclk", &dummy_apb_pclk),
	CLK(NULL, "dma", &dummy_dma),
};

void drime4_add_clk_table(void)
{
	clkdev_add_table(lookups, ARRAY_SIZE(lookups));
}
EXPORT_SYMBOL(drime4_add_clk_table);

static void drime4_init_one_clock(struct clk *c)
{
	clk_init(c);
	if (!c->lookup.dev_id && !c->lookup.con_id)
		c->lookup.con_id = c->name;
	c->lookup.clk = c;
	clkdev_add(&c->lookup);
}

int __init drime4_clk_init(void)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(drime4_ptr_clocks); i++)
		drime4_init_one_clock(drime4_ptr_clocks[i]);

	for (i = 0; i < ARRAY_SIZE(drime4_global_clocks); i++)
		drime4_init_one_clock(drime4_global_clocks[i]);

	for (i = 0; i < ARRAY_SIZE(drime4_peri_clocks); i++)
		drime4_init_one_clock(&drime4_peri_clocks[i]);

	return 0;
}

#ifdef CONFIG_DEBUG_FS

static struct dentry *clk_debugfs_root;

static int clk_avail_rate_show(struct seq_file *s, void *data)
{
	struct clk *c = s->private;

	seq_printf(s, "avail_freq\n");

	if (c->flags & CLK_SEL_FREQ) {
		if (c->type == CLK_PLL) {
			const struct clk_pll_freq *sel = c->u.pll.freq_table;
			for (; sel->input_freq != 0; sel++)
				seq_printf(s, "%u\n", sel->output_freq);
		} else if (c->type == CLK_AUD) {
			const struct clk_pll_freq *sel =
				c->parent->u.pll.freq_table;
			for (; sel->input_freq != 0; sel++)
				seq_printf(s, "%u\n", sel->output_freq);
		} else if (c->type == CLK_PERI) {
			const struct clk_sel_freq *sel = c->u.mux.sel_table;
			for (; sel->src != NULL; sel++) {
				seq_printf(s, "%lu\n",
					clk_get_rate(sel->src) / sel->div);
			}
		}
	} else
		seq_printf(s, "%lu\n", clk_get_rate(c));
	return 0;
}

static int clk_debugfs_avail_rate_open(struct inode *inode,
	struct file *file)
{
	return single_open(file, clk_avail_rate_show, inode->i_private);
}

static const struct file_operations clk_avail_rate_fops = {
	.open	= clk_debugfs_avail_rate_open,
	.read	= seq_read,
	.llseek	= seq_lseek,
	.release	= single_release,
};

static int clk_debugfs_rate_show(struct seq_file *s, void *data)
{
	struct clk *c = s->private;
	seq_printf(s, "%lu\n", clk_get_rate(c));
	return 0;
}

static int clk_debugfs_rate_open(struct inode *inode,
	struct file *file)
{
	return single_open(file, clk_debugfs_rate_show, inode->i_private);
}

static ssize_t clk_debugfs_rate_write(struct file *file,
	const char __user *user_buf,
	size_t count, loff_t *ppos)
{
	u32 rate;
	struct seq_file *s_file = (struct seq_file *)file->private_data;
	struct clk *c = s_file->private;
	char buf[32];
	unsigned int len = 0;
	len = min(count, sizeof(buf) - 1);
	if (copy_from_user(buf, user_buf, len))
		return -EFAULT;
	buf[len] = '\0';
	if (kstrtou32(buf, 0, &rate))
		return -EINVAL;

	clk_set_rate(c, (unsigned long)rate);

	return len;
}

static const struct file_operations clk_rate_fops = {
	.open	= clk_debugfs_rate_open,
	.read	= seq_read,
	.write	= clk_debugfs_rate_write,
	.llseek	= seq_lseek,
	.release	= single_release,
};

static int clk_debugfs_state_show(struct seq_file *s, void *data)
{
	struct clk *c = s->private;
	if (c->state == ON)
		seq_printf(s, "on\n");
	else if (c->state == OFF)
		seq_printf(s, "off\n");
	else
		seq_printf(s, "uninitialized\n");
	return 0;
}

static int clk_debugfs_state_open(struct inode *inode,
	struct file *file)
{
	return single_open(file, clk_debugfs_state_show, inode->i_private);
}

static ssize_t clk_debugfs_state_write(struct file *file,
	const char __user *user_buf,
	size_t count, loff_t *ppos)
{
	struct seq_file *s_file = (struct seq_file *)file->private_data;
	struct clk *c = s_file->private;
	char buf[32];
	unsigned int len = 0;
	len = min(count, sizeof(buf) - 1);
	if (copy_from_user(buf, user_buf, len))
		return -EFAULT;
	buf[len] = '\0';

	if (!strncmp(buf, "on", 2))
		clk_enable(c);
	else if (!strncmp(buf, "off", 3))
		clk_disable(c);

	return len;
}

static const struct file_operations clk_state_fops = {
	.open	= clk_debugfs_state_open,
	.read	= seq_read,
	.write	= clk_debugfs_state_write,
	.llseek	= seq_lseek,
	.release	= single_release,
};

static int clk_debugfs_parent_show(struct seq_file *s, void *data)
{
	struct clk *c = s->private;

	if (c->parent)
		seq_printf(s, "%s\n", c->parent->name);
	else
		seq_printf(s, "none\n");
	return 0;
}

static int clk_debugfs_parent_open(struct inode *inode,
	struct file *file)
{
	return single_open(file, clk_debugfs_parent_show, inode->i_private);
}

static ssize_t clk_debugfs_parent_write(struct file *file,
	const char __user *user_buf,
	size_t count, loff_t *ppos)
{
	struct seq_file *s_file = (struct seq_file *)file->private_data;
	struct clk *c = s_file->private;
	char buf[32];
	unsigned int len = 0;
	struct clk *cp;

	len = min(count, sizeof(buf) - 1);
	if (copy_from_user(buf, user_buf, len))
		return -EFAULT;
	buf[len] = '\0';
	strim(buf);

	list_for_each_entry(cp, &clocks, node) {
		if (!strcmp(cp->name, buf)) {
			clk_set_parent(c, cp);
			break;
		}
	}

	return len;
}

static const struct file_operations clk_parent_fops = {
	.open	= clk_debugfs_parent_open,
	.read	= seq_read,
	.write	= clk_debugfs_parent_write,
	.llseek	= seq_lseek,
	.release	= single_release,
};

static int clk_debugfs_register_one(struct clk *c)
{
	struct dentry *d;

	d = debugfs_create_dir(c->name, clk_debugfs_root);
	if (!d)
		return -ENOMEM;
	c->dent = d;

	d = debugfs_create_u8("refcnt", S_IRUGO, c->dent, (u8 *)&c->refcnt);
	if (!d)
		goto err_out;

	d = debugfs_create_file("rate", S_IRUGO, c->dent, c,
		&clk_rate_fops);
	if (!d)
		goto err_out;

	d = debugfs_create_x32("flags", S_IRUGO, c->dent, (u32 *)&c->flags);
	if (!d)
		goto err_out;

	d = debugfs_create_file("avail_rate", S_IRUGO, c->dent,
		c, &clk_avail_rate_fops);
	if (!d)
		goto err_out;

	d = debugfs_create_file("state", S_IRUGO, c->dent,
		c, &clk_state_fops);
	if (!d)
		goto err_out;

	d = debugfs_create_file("parent", S_IRUGO, c->dent,
		c, &clk_parent_fops);
	if (!d)
		goto err_out;

	return 0;

err_out:
	debugfs_remove_recursive(c->dent);
	return -ENOMEM;
}

static int clk_debugfs_register(struct clk *c)
{
	int err;
	struct clk *pa = c->parent;

	if (pa && !pa->dent) {
		err = clk_debugfs_register(pa);
		if (err)
			return err;
	}

	if (!c->dent) {
		err = clk_debugfs_register_one(c);
		if (err)
			return err;
	}
	return 0;
}

static int __init clk_debugfs_init(void)
{
	struct clk *c;
	struct dentry *d;
	int err = -ENOMEM;

	d = debugfs_create_dir("clock", NULL);
	if (!d)
		return -ENOMEM;
	clk_debugfs_root = d;

	list_for_each_entry(c, &clocks, node) {
		err = clk_debugfs_register(c);
		if (err)
			goto err_out;
	}
	return 0;
err_out:
	debugfs_remove_recursive(clk_debugfs_root);
	return err;
}

late_initcall(clk_debugfs_init);
#endif
