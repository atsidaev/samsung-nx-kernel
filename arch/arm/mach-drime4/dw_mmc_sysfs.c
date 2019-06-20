/*
 * drivers/mmc/host/dw_mmc_sysfs.c
 *
 * This driver is to create sysfs node to notify dw_mmc status to user area.
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
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/clk.h>
#include <linux/mmc/host.h>
#include <linux/mmc/dw_mmc.h>
#include <mach/map.h>

int g_media_err;
int g_support_card;
int g_protect_card;
u16 g_sample_phase;
u16 g_drive_phase;
int g_tunable;

extern void starter();
extern void stopper();


ssize_t show_media_node(struct device *dev, struct device_attribute *attr,
				char *buf)
{
	ssize_t size = 0;

	if (g_media_err)
		size = sprintf(buf, "-1\n");
	else
		size = sprintf(buf, "0\n");

	return size;
}

ssize_t store_media_node(struct device *dev, struct device_attribute *attr,
				const char *buf, ssize_t count)
{
	if (!strncmp(buf, "0", 1))
		g_media_err = 0;
	if (!strncmp(buf, "-1", 2))
		g_media_err = -1;
	else
		dev_info(dev, "usage: just input 0 or -1");

	return count;
}

ssize_t show_card_node(struct device *dev, struct device_attribute *attr,
				char *buf)
{
	ssize_t size = 0;

	if (g_support_card == 1)
		size = sprintf(buf, "1\n");
	else
		size = sprintf(buf, "0\n");

	return size;
}

ssize_t store_card_node(struct device *dev, struct device_attribute *attr,
				const char *buf, ssize_t count)
{
	if (!strncmp(buf, "0", 1))
	{
		stopper();
		//g_support_card = 0;
	}
	if (!strncmp(buf, "1", 1))
	{
		starter();
		//g_support_card = 1;
	}
	else
		dev_info(dev, "usage: just input 0 or 1");

	return count;
}

ssize_t show_phase_node(struct device *dev, struct device_attribute *attr,
				char *buf)
{
	ssize_t size = 0;
	u32 reg;

	reg = readl(DRIME4_VA_SD_CFG);

	size = sprintf(buf, "DRIME4_VA_SD_CFG = 0x%x\n", reg);

	return size;
}

ssize_t store_phase_node(struct device *dev, struct device_attribute *attr,
				const char *buf, ssize_t count)
{
	u32 reg = 0;

	if (!strncmp(buf, "0:0", 3)) {
		g_sample_phase = PHASE_SHIFT_0;
		g_drive_phase = PHASE_SHIFT_0;
	} else if (!strncmp(buf, "0:1", 3)) {
		g_sample_phase = PHASE_SHIFT_0;
		g_drive_phase = PHASE_SHIFT_90;
	} else if (!strncmp(buf, "0:2", 3)) {
		g_sample_phase = PHASE_SHIFT_0;
		g_drive_phase = PHASE_SHIFT_180;
	} else if (!strncmp(buf, "0:3", 3)) {
		g_sample_phase = PHASE_SHIFT_0;
		g_drive_phase = PHASE_SHIFT_270;
	} else if (!strncmp(buf, "1:0", 3)) {
		g_sample_phase = PHASE_SHIFT_90;
		g_drive_phase = PHASE_SHIFT_0;
	} else if (!strncmp(buf, "1:1", 3)) {
		g_sample_phase = PHASE_SHIFT_90;
		g_drive_phase = PHASE_SHIFT_90;
	} else if (!strncmp(buf, "1:2", 3)) {
		g_sample_phase = PHASE_SHIFT_90;
		g_drive_phase = PHASE_SHIFT_180;
	} else if (!strncmp(buf, "1:3", 3)) {
		g_sample_phase = PHASE_SHIFT_90;
		g_drive_phase = PHASE_SHIFT_270;
	} else if (!strncmp(buf, "2:0", 3)) {
		g_sample_phase = PHASE_SHIFT_180;
		g_drive_phase = PHASE_SHIFT_0;
	} else if (!strncmp(buf, "2:1", 3)) {
		g_sample_phase = PHASE_SHIFT_180;
		g_drive_phase = PHASE_SHIFT_90;
	} else if (!strncmp(buf, "2:2", 3)) {
		g_sample_phase = PHASE_SHIFT_180;
		g_drive_phase = PHASE_SHIFT_180;
	} else if (!strncmp(buf, "2:3", 3)) {
		g_sample_phase = PHASE_SHIFT_180;
		g_drive_phase = PHASE_SHIFT_270;
	} else if (!strncmp(buf, "3:0", 3)) {
		g_sample_phase = PHASE_SHIFT_270;
		g_drive_phase = PHASE_SHIFT_0;
	} else if (!strncmp(buf, "3:1", 3)) {
		g_sample_phase = PHASE_SHIFT_270;
		g_drive_phase = PHASE_SHIFT_90;
	} else if (!strncmp(buf, "3:2", 3)) {
		g_sample_phase = PHASE_SHIFT_270;
		g_drive_phase = PHASE_SHIFT_180;
	} else if (!strncmp(buf, "3:3", 3)) {
		g_sample_phase = PHASE_SHIFT_270;
		g_drive_phase = PHASE_SHIFT_270;
	} else if (!strncmp(buf, "tunable", 7)) {
		if (g_tunable)
			g_tunable = 0;
		else
			g_tunable = 1;
	} else {
		g_tunable = 0;
		dev_info(dev, "set sample:drive phase [0-3]:[0-3]\n");
		dev_info(dev, "value 0,1,2,3 mean phase 0, 90, 180, 270 degrees\n");
		dev_info(dev, "ex) echo tunable > change; echo 0:0 > change_phase\n");

		return count;
	}

	reg = readl(DRIME4_VA_SD_CFG);
	reg &= (~(1<<31));
	writel(reg, DRIME4_VA_SD_CFG);
	reg = ((1<<31) | (g_sample_phase<<4) | g_drive_phase);
	writel(reg, DRIME4_VA_SD_CFG);

	return count;
}

ssize_t show_detect_node(struct device *dev, struct device_attribute *attr,
				char *buf)
{
	ssize_t size = 0;

	return size;
}

ssize_t store_detect_node(struct device *dev, struct device_attribute *attr,
				const char *buf, ssize_t count)
{
	struct dw_mci *host = dev_get_drvdata(dev);
	struct dw_mci_slot *slot = host->slot[0];

	if (!strncmp(buf, "1", 1))
		mmc_detect_change(slot->mmc, msecs_to_jiffies(0));

	return count;
}

ssize_t show_protect_node(struct device *dev, struct device_attribute *attr,
				char *buf)
{
	ssize_t size = 0;

	if (g_protect_card)
		size = sprintf(buf, "1\n");
	else
		size = sprintf(buf, "0\n");

	return size;
}

ssize_t store_protect_node(struct device *dev, struct device_attribute *attr,
				const char *buf, ssize_t count)
{
	if (!strncmp(buf, "1", 1))
		g_protect_card = 1;
	if (!strncmp(buf, "0", 1))
		g_protect_card = 0;
	else
		dev_info(dev, "usage: just input 0 or 1");

	return count;
}


/*************************** DEVICE ATTRIBUTES ***************************/

static DEVICE_ATTR(media_error, 0644, show_media_node, store_media_node);
static DEVICE_ATTR(support_card, 0644, show_card_node, store_card_node);
static DEVICE_ATTR(change_phase, 0644, show_phase_node, store_phase_node);
static DEVICE_ATTR(manual_detect, 0644, show_detect_node, store_detect_node);
static DEVICE_ATTR(protect_card, 0644, show_protect_node, store_protect_node);

/*************************************************************************/

int dw_mci_create_node(struct device *dev)
{
	int rc;

	/* initialize error check variables */
	g_media_err = 0;
	g_support_card = 0;

	if (!dev)
		return -1;

	rc = device_create_file(dev, &dev_attr_media_error);
	if (rc) {
		dev_info(dev, "failed to create sysfs media_error node\n");
		return -1;
	}

	rc = device_create_file(dev, &dev_attr_support_card);
	if (rc) {
		dev_info(dev, "failed to create sysfs support_card node\n");
		return -1;
	}

	rc = device_create_file(dev, &dev_attr_change_phase);
	if (rc) {
		dev_info(dev, "failed to create sysfs change_phase node\n");
		return -1;
	}

	rc = device_create_file(dev, &dev_attr_manual_detect);
	if (rc) {
		dev_info(dev, "failed to create sysfs menual_detect node\n");
		return -1;
	}

	rc = device_create_file(dev, &dev_attr_protect_card);
	if (rc) {
		dev_info(dev, "failed to create sysfs protect_card node\n");
		return -1;
	}
	return 0;
}

int dw_mci_remove_node(struct device *dev)
{
	device_remove_file(dev, &dev_attr_media_error);
	device_remove_file(dev, &dev_attr_support_card);
	device_remove_file(dev, &dev_attr_change_phase);
	device_remove_file(dev, &dev_attr_manual_detect);
	device_remove_file(dev, &dev_attr_protect_card);	

	return 0;
}
