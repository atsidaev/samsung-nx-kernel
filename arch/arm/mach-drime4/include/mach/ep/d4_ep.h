/**
 * @file d4_ep.h
 * @brief DRIMe4 EP Platform Driver Header File
 * @author Wooram Son <wooram.son@samsung.com>
 * 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef D4_EP_H_
#define D4_EP_H_

#define EP_MODULE_NAME		"drime4_ep"

/* #define ENABLE_EP_INTR_DEBUG_MSG TRUE */
#define ENABLE_EP_INTR_ERROR_MSG TRUE

#ifdef ENABLE_EP_INTR_DEBUG_MSG
#define EP_INTR_DEBUG_MSG(format, args...) pr_debug(format, ##args)
#else
#define EP_INTR_DEBUG_MSG(format, args...)
#endif

#ifdef ENABLE_EP_INTR_ERROR_MSG
#define EP_INTR_ERROR_MSG(format, args...) pr_err(format, ##args)
#else
#define EP_INTR_ERROR_MSG(format, args...)
#endif


/* Device context structure */
struct drime4_ep {
	/* Miscellaneous device */
	int id;
	const char *name;
	struct device *dev;
	struct clk *clock;

	int ep_core_irq, ep_dma_irq;

	void __iomem *reg_base_top;
	void __iomem *reg_base_ldc;
	void __iomem *reg_base_hdr;
	void __iomem *reg_base_nrm;
	void __iomem *reg_base_mnf;
	void __iomem *reg_base_fd;
	void __iomem *reg_base_bblt;
	void __iomem *reg_base_lvr;
	void __iomem *reg_base_dma;
};

#endif
