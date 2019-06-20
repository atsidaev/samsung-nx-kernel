/**
 * @file d4_pp_ssif.h
 * @brief DRIMe4 PP Sensor Platform Driver Header File
 * @author DeokEun Cho <de.cho@samsung.com>
 * 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifndef _PP_SSIF_H_
#define _PP_SSIF_H_

#define PP_SSIF_MODULE_NAME		"d4_pp_ssif"

/******************************************************************************/
/*                                Structure                                   */
/******************************************************************************/

/*
 * Main context
 */
struct drime4_pp_ssif {
	struct device	*dev;
	const char 		*name;
	int 			id;
	atomic_t 		ref_count;
	struct drime4_pp_ssif_dev_data *pd;
	struct pinctrl 			*pmx;
	struct pinctrl_state	*pins_default;
};

/* platform device에 추가로 입력할 device data
   구조체를 아래와 같이 정의한다. 필요할 시에만
   정의하면 된다. */
struct drime4_pp_ssif_dev_data {
	unsigned int param0;
	unsigned int param1;
	unsigned int param2;
	unsigned int param3;
};

struct drimet_pp_ssif_ext_tg_data {
	unsigned int ext_tg_clk;
	char *sg_cksel;
	char *s0_cksel;
	char *s1_cksel;
	char *s2_cksel;
	char *s3_cksel;
};
#endif /* _PP_SSIF_H_ */

