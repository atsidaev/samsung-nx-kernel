/**
 * @file sensor_interface.h
 * @brief sensor driver header for Samsung DRIMe4 Camera Interface driver
 * @author Gun Ho Lee <lgh3002@acroem.com>
 * @author Seok Weon Seo <seokweon.seo@samsung.com>
 * 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __EXTERN_STROBE_INTERFACE_H__
#define __EXTERN_STROBE_INTERFACE_H__
#include <linux/ioctl.h>

struct PwmCtrlCmd {
	unsigned int period;
	unsigned int delay;
};

enum EXTSTR_TRGSRC {
	EXTSTR_TRG_SW,
	EXTSTR_TRG_VD,
	EXTSTR_TRG_EXT,
};

struct strobe_spi_ioc_transfer {
	unsigned long long		tx_buf;
	unsigned long long		rx_buf;
	unsigned int			len;
	unsigned short		pre_delay;
	unsigned short		hold_delay;
	unsigned short		post_delay;
};


/*Collision setting */

/* Cmd Of Ssensor IOCTL */
#define STR_IOC_MAGIC	'S'

/* EXTERN STROBE SET TRIGGER ENABLE */
#define PWMIOC_P_PWM_START_TRIGGER	_IO(STR_IOC_MAGIC, 0)

/* EXTERN STROBE SET TRIGGER TIMMING */
#define PWMIOC_P_PWM_SET_TIMMING _IOW(STR_IOC_MAGIC, 1,\
				      struct PwmCtrlCmd)

#define PWMIOC_P_PWM_SET_TRGSRC         _IOW(STR_IOC_MAGIC, 2, unsigned char)

#define HOTSHOE_NORMAL_TRANSFER        _IOW(STR_IOC_MAGIC, 3, unsigned int)
#define HOTSHOE_BURST_TRANSFER         _IOW(STR_IOC_MAGIC, 4, unsigned int)


#define EX_STR_DET_GET				 	_IOR(STR_IOC_MAGIC, 5, unsigned int )
#define EX_STR_VCCON_SET				 _IOW(STR_IOC_MAGIC, 6, unsigned int )
#define EX_STR_TRIG_SET					 _IOW(STR_IOC_MAGIC, 7, unsigned int )


#endif	/* __EXTERN_STROBE_INTERFACE_H__ */

