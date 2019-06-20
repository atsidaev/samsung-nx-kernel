/*
 * drivers/rtc/rtx_sysfs.c
 *
 * This driver is to create sysfs node to notify d4key_sysfs status to user area.
 *
 * Copyright (C) 2012 for Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/stat.h>

extern unsigned int rtc_get_boot_info(struct device *dev);
extern void rtc_set_boot_info(struct device *dev);
static ssize_t
show_rtc_boot_info(struct device *dev, struct device_attribute *attr,
				char *buf)
{
	unsigned int data;
	ssize_t size = 0;
	data = rtc_get_boot_info(dev);
	size = sprintf(buf, "%d\n", data);
	return size;
}

static ssize_t
store_rtc_boot_info(struct device *dev, struct device_attribute *attr,
				const char *buf, size_t count)
{
	rtc_set_boot_info(dev);
	return 0;
}

static DEVICE_ATTR(boot_info, S_IRUGO | S_IWUSR,
			show_rtc_boot_info, store_rtc_boot_info);



int rtc_create_node(struct device *dev)
{
	int rc;
	if (!dev)
		return -1;

	rc = device_create_file(dev, &dev_attr_boot_info);
	if (rc) {
		dev_info(dev, "failed to create sysfs d4keys node\n");
		return -1;
	}
	return 0;
}

void rtc_remove_node(struct device *dev)
{
	device_remove_file(dev, &dev_attr_boot_info);
}
