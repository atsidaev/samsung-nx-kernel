/* linux/arch/arm/mach-drime4/include/mach/adc.h
 *
 * Copyright (c) 2011 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com/
 *
 * DRIME4 - ADC support
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __MACH_DRIME4_ADC_H
#define __MACH_DRIME4_ADC_H

struct drime4_adc_client;
struct drime4_adc_device;

typedef void (*int_handler)(void *);


enum adc_int_state {
	ADC_INT_ON, ADC_INT_OFF,
};


extern void drime4_adc_request_irq(struct drime4_adc_client *client,
		int_handler uhander, void *dev, unsigned int channel);


extern void drime4_adc_int_set(struct drime4_adc_client *client,
		enum adc_int_state val, unsigned int channel);

extern void drime4_adc_start(struct drime4_adc_client *client,
		unsigned int channel);

extern int drime4_adc_read(struct drime4_adc_client *client,
		unsigned int channel);

extern int drime4_adc_raw_read(struct drime4_adc_client *client,
		unsigned int channel);


extern unsigned int drime4_adc_raw_read_hax(struct drime4_adc_client *client,
		unsigned int channel);

extern struct drime4_adc_client *
drime4_adc_register(struct platform_device *pdev, unsigned int ref_mvolt,
		unsigned int time_margin, unsigned int data_diff);

extern void drime4_adc_release(struct drime4_adc_client *client);
#endif
