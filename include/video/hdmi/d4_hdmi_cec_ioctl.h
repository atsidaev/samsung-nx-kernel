/**
 * @file d4_hdmi_cec_ioctl.h
 * @brief DRIMe4 HDMI CEC IOCTL header File
 * @author Somabha Bhattacharjya <b.somabha@samsung.com>
 * 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef _D4_HDMI_CEC_H_
#define _D4_HDMI_CEC_H_

#define CEC_IOC_MAGIC		'c'

/* CEC device request code to set logical address */
#define CEC_IOC_SETLADDR		_IOW(CEC_IOC_MAGIC, 0, unsigned int)
#define CEC_IOC_SETRETRYCNT		_IOW(CEC_IOC_MAGIC, 2, unsigned int)

#endif /* _D4_HDMI_CEC_H_ */
