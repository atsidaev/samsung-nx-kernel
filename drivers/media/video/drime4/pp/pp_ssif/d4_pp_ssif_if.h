/**
 * @file d4_pp_ssif_if.h
 * @brief DRIMe4 PP Sensor Interface Device Driver Header
 * @author Main : DeokEun Cho <de.cho@samsung.com>
 *         MIPI : Gunwoo Nam <gunwoo.nam@samsung.com>
 * 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifndef __PP_SSIF_INTERFACE_H_
#define __PP_SSIF_INTERFACE_H_

#include <linux/completion.h>
#include <media/drime4/pp/pp_ssif/d4_pp_ssif_type.h>

/**<  For POLL using */
#define PP_SSIF_POLL_ENABLE

#ifdef __cplusplus
extern "C" {
#endif

void pp_ssif_com_request_irq(void);
void pp_ssif_com_free_irq(void);
void pp_ssif_k_dd_set_sensor_clock(unsigned int clock_set);

extern struct completion pp_ssif_int_completion[NUM_OF_SSIF_DD_INT];
void pp_ssif_set_callback_func(enum ssif_dd_interrupt_selection selection, void (*callback_func)(int));
void k_pp_ssif_set_interrupt_enable(enum ssif_dd_interrupt_selection selection, enum k_ssif_onoff onoff);

#ifdef __cplusplus
}
#endif

#endif /* __PP_SSIF_INTERFACE_H_ */

