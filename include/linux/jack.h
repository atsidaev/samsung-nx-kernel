/*
 *  Copyright (C) 2009 Samsung Electronics
 *  Minkyu Kang <mk7.kang@samsung.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __JACK_H_
#define __JACK_H_

struct jack_platform_data {
	int usb_online;
	int charger_online;
	int charger1_online;
	int hdmi_online;
	int earjack_online;
	int earkey_online;
	int ums_online;
	int cdrom_online;
	int jig_online;
	int host_online;
	int release_online; /** wired release button*/
	int mmc_online;
    int usb_sp_online;
};

int jack_get_data(const char *name);
void jack_event_handler(const char *name, int value);

void usb_drime4_ss_udc_run_stop(unsigned int is_on);

#endif
