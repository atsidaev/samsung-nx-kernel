/*
 *  Switch class driver
 *
 * Copyright (C) 2008 Google, Inc.
 * Author: Mike Lockwood <lockwood@android.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
*/

#ifndef __LINUX_SWITCH_H__
#define __LINUX_SWITCH_H__

#include <linux/notifier.h>

#define SUPPORTED_CABLE_MAX	32
#define CABLE_NAME_MAX		30

struct switch_dev {
	const char	*name;
	struct device	*dev;
	int		index;
	int		state;

	ssize_t	(*print_name)(struct switch_dev *sdev, char *buf);
	ssize_t	(*print_state)(struct switch_dev *sdev, char *buf);

	struct raw_notifier_head nh;
	struct list_head entry;

	const char **supported_cable; /* array of name for supported cables */
};

struct gpio_switch_platform_data {
	const char *name;
	unsigned 	gpio;

	/* if NULL, switch_dev.name will be printed */
	const char *name_on;
	const char *name_off;
	/* if NULL, "0" or "1" will be printed */
	const char *state_on;
	const char *state_off;
};

#ifdef CONFIG_SWITCH
extern int switch_dev_register(struct switch_dev *sdev);
extern void switch_dev_unregister(struct switch_dev *sdev);

static inline int switch_get_state(struct switch_dev *sdev)
{
	return sdev->state;
}

extern void switch_set_state(struct switch_dev *sdev, int state);

extern int switch_get_cable_state(struct switch_dev *sdev,
			const char *cable_name);
extern int switch_set_cable_state(struct switch_dev *sdev,
			const char *cable_name, int cable_state);
extern int switch_get_event_code(struct switch_dev *sdev,
			const char *cable_name);

extern struct switch_dev *switch_get_switch_dev(const char *switch_name);
extern int switch_register_notifier(struct switch_dev *sdev,
			struct notifier_block *bh);
extern int switch_unregister_notifier(struct switch_dev *sdev,
			struct notifier_block *bh);
#else
static inline int switch_dev_register(struct switch_dev *sdev)
{
	return 0;
}
static inline void switch_dev_unregister(struct switch_dev *sdev)
{
	return;
}

static inline int switch_get_state(struct switch_dev *sdev)
{
	return 0;
}

static inline void switch_set_state(struct switch_dev *sdev, int state)
{
	return;
}

static inline int switch_get_cable_state(struct switch_dev *sdev,
			const char *cable_name)
{
	return 0;
}

static inline int switch_set_cable_state(struct switch_dev *sdev,
			const char *cable_name, int state)
{
	return 0;
}

static inline int switch_get_event_code(struct switch_dev *sdev,
			const char *cable_name)
{
	return 0;
}

static inline struct switch_dev *switch_get_switch_dev(const char *switch_name)
{
	return NULL;
}
static inline int switch_register_notifier(struct switch_dev *sdev,
			struct notifier_block *bh)
{
	return 0;
}
static inline int switch_unregister_notifier(struct switch_dev *sdev,
			struct notifier_block *bh)
{
	return 0;
}
#endif
#endif /* __LINUX_SWITCH_H__ */
