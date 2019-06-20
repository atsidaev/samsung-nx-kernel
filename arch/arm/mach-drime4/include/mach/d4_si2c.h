/**
 * @file d4_si2c.h
 * @brief DRIMe4 Slave I2C Platform Driver Header File
 * @author Kyuchun Han <kyuchun.han@samsung.com>
 * 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef D4_SI2C_H_
#define D4_SI2C_H_

#define SI2C_MODULE_NAME		"drime4-si2c"



/* Device context structure */
struct drime4_si2c {
	struct list_head list;
	int id;
	const char *name;
	struct device *dev;
	struct clk *clock;
	int si2c_irq;
	void __iomem *reg_base;
	struct clk *clk;
	wait_queue_head_t i2c_wq;
	struct completion done;
	unsigned int si2c_mode;
	unsigned char addr;
	unsigned int addr_check;
	struct pinctrl *pinctrl;
	struct pinctrl_state	*pinstate;
};

static LIST_HEAD(si2c_list);
static DEFINE_MUTEX(si2c_lock);
#endif

