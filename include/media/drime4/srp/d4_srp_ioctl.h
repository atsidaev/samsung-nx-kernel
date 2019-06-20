/**
 * @file drime4_srp_ioctl.h
 * @brief DRIMe4 SRP IOCTL Interface Header File
 * @author Geunjae Yu <geunjae.yu@samsung.com>
 * 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */


#ifndef __D4_SRP_IOCTL_H__
#define __D4_SRP_IOCTL_H__

/*
#include "media/drime4/srp/d4_srp_type.h"
*/
#include "d4_srp_type.h"


struct srp_ioctl_get_reg_info {
	enum srp_reg_selection phys_reg_selection;
	struct srp_phys_reg_info phys_reg_info;
};

struct srp_ioctl_sema_info {
	int timeout;
	int result;
};

enum srp_ioctl_pinmux_info {
	MODE_CTL_SRP,
	MODE_CTL_CODEC,
};

enum srp_clock_gate {
	SRP_CLOCK_ON,
	SRP_CLOCK_OFF,
};

struct srp_ioctl_clock_info {
	unsigned int clock_change;
	unsigned int clock_freq;
	enum srp_clock_gate clock_on;
	int result;
};

/*
 * SRP structure define section
 */
struct srp_reg_ctrl_base_info {
	struct device *dev;
	unsigned int srp_reg_base;
	unsigned int srp_dap_base;
	unsigned int srp_commbox_base;
	unsigned int srp_ssram_base;
	int irq_num;

	struct srp_phys_reg_info srp_reg_info;
	struct srp_phys_reg_info srp_dap_info;
	struct srp_phys_reg_info srp_commbox_info;
	struct srp_phys_reg_info srp_ssram_info;
};


extern void d4_srp_set_reg_ctrl_base_info(struct srp_reg_ctrl_base_info *info);

#define SRP_MAGIC 'r'



#define SRP_IOCTL_OPEN_IRQ 			_IO(SRP_MAGIC, 1)
#define SRP_IOCTL_CLOSE_IRQ 			_IO(SRP_MAGIC, 2)
#define SRP_IOCTL_INIT_COMPLETION		_IO(SRP_MAGIC, 3)

#define SRP_IOCTL_SET_MODECTL			_IOW(SRP_MAGIC, 6, unsigned int)
#define SRP_IOCTL_SET_CLOCK			_IOW(SRP_MAGIC, 7, struct srp_ioctl_clock_info)
#define SRP_IOCTL_SET_DELAY			_IOW(SRP_MAGIC, 8, unsigned int)

#define SRP_IOCTL_SEMAPHORE 			_IOWR(SRP_MAGIC, 10, struct srp_ioctl_sema_info)
#define SRP_IOCTL_GET_PHYS_REG_INFO		_IOWR(SRP_MAGIC, 11, struct srp_ioctl_get_reg_info)


#endif /* __D4_SRP_IOCTL_H__ */
