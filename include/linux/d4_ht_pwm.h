/**
 * @file d4_ht_pwm.h
 * @brief DRIMe4 Hardware Trigger PWM Interface
 * @author kyuchun han <kyuchun.han@samsung.com>
 * 2010 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifndef _D4_HT_PWM_H
#define _D4_HT_PWM_H

#include <mach/d4_ht_pwm_type.h>

struct ht_pwm_device;


struct ht_pwm_device *d4_ht_pwm_request(int pwm_id, const char *label);

void d4_ht_pwm_int_func_set(struct ht_pwm_device *pwm, void (*callback_func)(int));

void d4_ht_pwm_int_enalbe(struct ht_pwm_device *pwm, enum ht_pwm_int_enable_type int_type);

void d4_ht_pwm_int_disable(struct ht_pwm_device *pwm);

void d4_ht_pwm_free(struct ht_pwm_device *pwm);

int d4_ht_pwm_config(struct ht_pwm_device *pwm, struct ht_pwm_conf_info *pwm_info);

void d4_ht_pwm_clear(struct ht_pwm_device *pwm);

void d4_ht_pwm_enable(struct ht_pwm_device *pwm);

void d4_ht_pwm_disable(struct ht_pwm_device *pwm);

void d4_inc_dic_pulse_set(struct ht_pwm_device *pwm);

void d4_ht_pwm_pad_set(struct ht_pwm_device *pwm, unsigned int val);

void d4_ht_pwm_extinput_set(struct ht_pwm_device *pwm, enum ht_pwm_extint input_num);
void d4_pwm_get_phy_info(struct ht_pwm_device *pwm, struct ht_pwm_phys_reg_info *info);
#endif /* _D4_HT_PWM_H */

