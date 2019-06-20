/**
 * @file d4_ep_top.c
 * @brief DRIMe4 EP Platform Driver Header File
 * @author Wooram Son <wooram.son@samsung.com>
 * 2012 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef D4_BE_H_
#define D4_BE_H_

#define BE_MODULE_NAME		"drime4_be"
/*************************************************/
/*                Structure					     */
/************************************************/

struct drime4_be {
	/* Miscellaneous device */
	int id;
	const char *name;
	struct device *dev;
	struct clk 	  *clock;
	/* Register base address of Sub-Block IPs */
	void __iomem *reg_be_top; /*5008-0000*/
	void __iomem *reg_be_ghost;/*5008-1000*/
	void __iomem *reg_be_snr; /*5008-2000*/
	void __iomem *reg_be_sg; /*5008-3000*/
	void __iomem *reg_be_blend;/*5008-4000*/
	void __iomem *reg_be_fme; /*5008-5000*/
	void __iomem *reg_be_3dme; /*5008-6000*/
	void __iomem *reg_be_dma; /*5008-7000*/

	int be_irq;
};


#endif


