/* linux/arch/arm/mach-drime4/clock.h
 *
 * Copyright (c) 2011 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com/
 *
 * DRIME4 - Clock definitions
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#ifndef __MACH_DRIME4_CLOCK_H_
#define __MACH_DRIME4_CLOCK_H_

#include <linux/clkdev.h>

/* flag definitions */
#define CLK_SEL_FREQ		(0x1)
#define CLK_MCLK_CHG_CMD	(0x2)
#define CLK_GCON		(0x4)
#define CLK_PF_GATE		(0x8)
#define CLK_SEL_PATH		(0x10)
#define CLK_PF_RESET		(0x20)

enum clk_type {
	XTAL_PAD,
	CLK_PLL,
	CLK_PLL_DIV,
	CLK_AUD,
	CLK_BUS,
	CLK_PERI,
};

enum clk_pll_type {
	PLL3500X,
	PLL3600X,
};

enum clk_state {
	UNINITIALIZED = 0,
	ON,
	OFF,
};

struct clk;

struct clk_ops {
	void	(*init)(struct clk *clk);
	int	(*enable)(struct clk *clk);
	void	(*disable)(struct clk *clk);
	int	(*set_rate)(struct clk *, unsigned long);
	unsigned long	(*get_rate)(struct clk *);
	long	(*round_rate)(struct clk *, unsigned long);
	int	(*set_parent)(struct clk *, struct clk *);
	struct clk*	(*get_parent)(struct clk *);
};

struct clk_reg {
	u32			reg;
	u32			shift;
	u32			mask;
};

struct clk_pll_freq {
	u32			input_freq;
	u32			output_freq;
	u8			p;
	u16			m;
	u8			s;
	s16			k;
};

struct clk_sel_freq {
	u32			val;
	u32			div;
	struct clk		*src;
};

struct clk {
	/* node for master clocks list */
	struct list_head	node;	/* node for list of all clocks */
	struct clk_lookup	lookup;
#ifdef CONFIG_DEBUG_FS
	struct dentry		*dent;
#endif
	struct clk		*parent;
	enum clk_type		type;
	const char		*name;
	unsigned long		rate;
	struct clk_ops		*ops;
	enum clk_state		state;
	u32			div;

	spinlock_t		lock;

	struct clk_reg		freq_reg;
	struct clk_reg		con_reg;
	struct clk_reg		con2_reg;
	struct clk_reg		gate_reg;
	struct clk_reg		reset_reg;

	u32			flags;
	u32			refcnt;

	union {
		struct {
			const struct clk_pll_freq	*freq_table;
			struct clk_reg			xtlsel_reg;
			enum clk_pll_type		pll_type;
		} pll;
		struct {
			const struct clk_sel_freq	*sel_table;
		} mux;
	} u;
};

void drime4_add_clk_table(void);

#endif
