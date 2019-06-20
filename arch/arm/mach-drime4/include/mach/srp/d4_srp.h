/**
 * @file d4_srp.h
 * @brief	SRP device header for Samsung DRIMe4 Camera Interface driver
 *
 * @author Geunjae Yu <geunjae.yu@samsung.com>,
 * Copyright (c) 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __D4_SRP_H__
#define __D4_SRP_H__

#define SRP_MODULE_NAME	"d4_srp"


struct drime4_srp_dev_data {
	unsigned int param0;
	unsigned int param1;
	unsigned int param2;
	unsigned int param3;
};

struct d4_srp {
	struct device *dev;
	const char *name;
	int id;
	int irq_num;
	volatile int ref_count;
	int irqlock;

	struct clk *clock;
	struct pinctrl *pmx;
	struct pinctrl_state	*pins_default;
	struct drime4_srp_dev_data *pd;

};


#ifdef __cplusplus
extern "C" {
#endif


#ifdef __cplusplus
}
#endif

#endif /* __D4_SRP_H__ */
