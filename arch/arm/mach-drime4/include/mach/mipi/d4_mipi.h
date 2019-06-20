/**
 * @file d4_mipi.h
 * @brief DRIMe4 Mipi Platform Driver Header File
 * @author Gunwoo Nam <gunwoo.nam@samsung.com>
 * 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifndef __MIPI_H_
#define __MIPI_H_

#define MIPI_MODULE_NAME		"d4_mipi"

/******************************************************************************/
/*                                Structure                                   */
/******************************************************************************/

struct drime4_mipi {
	struct device *dev;
	const char	  *name;
	int			  id;
	struct clk 	  *clock;
	atomic_t 	  ref_count;
	struct drime4_mipi_dev_data *pd;
};

struct drime4_mipi_dev_data {
	unsigned int param0;
	unsigned int param1;
	unsigned int param2;
	unsigned int param3;
};

#endif /* __MIPI_H_ */


