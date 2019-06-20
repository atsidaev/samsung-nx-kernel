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
#ifndef _D4_PMU_TYPE_H
#define _D4_PMU_TYPE_H


enum pmu_ip_type {
	PMU_GPU,
	PMU_PP,
	PMU_IPCM,
	PMU_IPCS,
	PMU_EP,
	PMU_BAYER,
	PMU_DP,
	PMU_JPEG,
	PMU_CODEC
};

enum pmu_ip_delay {
	PMU_D1,
	PMU_D2,
	PMU_D3
};

enum pmu_ctrl_type {
	PMU_CTRL_OFF,
	PMU_CTRL_ON,
};

#endif /* _D4_PMU_TYPE_H */
