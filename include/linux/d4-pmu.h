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
#ifndef _D4_PMU_H
#define _D4_PMU_H

#include <mach/d4_pmu_type.h>

struct d4_pmu_device;
int d4_pmu_check(enum pmu_ip_type ip);
int d4_pmu_request(enum pmu_ip_type ip);
void d4_pmu_request_clear(enum pmu_ip_type ip);
void d4_pmu_scpre_set(enum pmu_ip_type ip, enum pmu_ctrl_type value);
void d4_pmu_isoen_set(enum pmu_ip_type ip, enum pmu_ctrl_type value);
void d4_pmu_scall_set(enum pmu_ip_type ip, enum pmu_ctrl_type value);
struct resource *d4_pmu_res_request(void);
int wait_for_stable(enum pmu_ip_type ip);
void d4_pmu_bus_reset(enum pmu_ip_type ip);
int pmu_wait_for_ack(enum pmu_ip_type ip);
/*int d4_pmu_dma_reset(enum pmu_ip_type ip);*/
#endif /* _D4_PMU_H */

