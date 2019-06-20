/**
 * @file d4_ht_pwm_type.h
 * @brief DRIMe4 Hardware Trigger PWM Type
 * @author kyuchun han <kyuchun.han@samsung.com>
 * 2010 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifndef _D4_PTC_TYPE_H
#define _D4_PTC_TYPE_H


enum ptc_input_mode {
    PTC_ONE_INPUT,                      /**< ZPIB */
    PTC_TWO_INPUT                       /**< ZPIB, ZPIC */
};

enum ptc_direct_type {
    DIRECT_TELE,                         /**< ZPIB */
    DIRECT_WIDE                          /**< ZPIB, ZPIC */
};

enum ptc_edge_type {
    PTC_RISING_EDGE  = 1,
    PTC_FALLING_EDGE = 2,
    PTC_BOTH_EDGE    = 3
};

enum ptc_run_type {
	PTC_ZPIB,
	PTC_ZPIC,
	PTC_DF
};

enum ptc_int_type {
	PTC_INT_ENABLE,
	PTC_INT_DISABLE
};

typedef void (*ptc_callback) (void *dev);

struct ptc_oneinput_info {
	ptc_callback	pCallback;
	enum ptc_edge_type edge_type;
	unsigned int time;
};

struct ptc_twoinput_info {
	ptc_callback	pCallback;
	enum ptc_run_type run_type;
	unsigned int time;
};

struct ptc_config {
	enum ptc_edge_type edge_type;
	unsigned int count;
	enum ptc_input_mode mode;
	unsigned int waittime;
};


#endif /* _D4_HT_PWM_TYPE_H */
