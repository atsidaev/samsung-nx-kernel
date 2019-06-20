/**
 * @file d4_pp_core.h
 * @brief DRIMe4 PP Core Platform Driver Header File
 * @author Sunghoon Kim <bluesay.kim@samsung.com>
 * 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifndef __PP_CORE_H_
#define __PP_CORE_H_

#define PP_CORE_MODULE_NAME		"d4_pp_core"

#include <media/v4l2-device.h>

/******************************************************************************/
/*                                Structure                                   */
/******************************************************************************/

/*
 * Main context
 */
struct drime4_pp_core {
	struct device *dev;
	const char	  *name;
	int			  id;
	struct clk 	  *clock;
	atomic_t 	  ref_count;
	struct drime4_pp_core_dev_data *pd;
};

/* platform device에 추가로 입력할 device data
 구조체를 아래와 같이 정의한다. 필요할 시에만
 정의하면 된다. */
struct drime4_pp_core_dev_data {
	unsigned int param0;
	unsigned int param1;
	unsigned int param2;
	unsigned int param3;
};

#endif /* __PP_CORE_H_ */

