/**
 * @file d4_opener_type.h
 * @brief DRIMe4 OPENER Structure Define
 * @author Wooram Son <wooram.son@samsung.com>
 * 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/
#ifndef __D4_OPENER_TYPE_H__
#define __D4_OPENER_TYPE_H__

#define MAX_KDD_OPEN_CNT	100
#define KDD_CLOSE_MAGIC 100000

enum kdd_devices {
	KDD_BE,
	KDD_BMA,
	KDD_BWM,
	KDD_EDID,
	KDD_EP,
	KDD_IPCM,
	KDD_IPCS,
	KDD_JPEG,
	KDD_JXR,
	KDD_MIPI,
	KDD_PP,
	KDD_SRP,
	KDD_DEVICES_MAX
};

enum kdd_open_flag {
	KDD_OPEN_ERROR = -1,
	KDD_OPEN_FIRST = 0,
	KDD_OPEN = 1,
	KDD_OPEN_EXCEED_MAXIMUM_OPEN = 2
};

enum kdd_close_flag {
	KDD_CLOSE_ERROR = -1,
	KDD_CLOSE_FINAL = 0,
	KDD_CLOSE = 1
};

struct kdd_open_pid_list {
	enum kdd_devices dev;
	int open_cnt;
	int kdd_open_pid[MAX_KDD_OPEN_CNT];
};

#endif   /* __D4_OPENER_TYPE_H__ */

