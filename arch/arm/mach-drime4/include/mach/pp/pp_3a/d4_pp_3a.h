/**
 * @file d4_pp_3a.h
 * @brief DRIMe4 PP 3A Device Driver Header
 * @author Kyounghwan Moon <kh.moon@samsung.com>,
 * 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __PP_3A_H_
#define __PP_3A_H_

#define PP_3A_MODULE_NAME		"d4_pp_3a"

/* platform device에 추가로 입력할 device data
   구조체를 아래와 같이 정의한다. 필요할 시에만
   정의하면 된다. */
struct drime4_pp_3a_dev_data {
	unsigned int param0;
	unsigned int param1;
	unsigned int param2;
	unsigned int param3;
};

/*
 * Main context
 */
struct drime4_pp_3a {
	struct device			*dev;
	const char			*name;
	int					id;
	void __iomem			*regs_base;
	unsigned long			result_mem_base;
	int					irq;
	struct drime4_pp_3a_dev_data  *pd;
} ;

#endif /* __PP_3A_H_ */
