/**
 * @file mdma_type.h
 * @brief MDMA config set Data
 * @author kyuchun han <kyuchun.han@samsung.com>
 * 2010 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifndef __LINUX_MDMA_CONF_H
#define __LINUX_MDMA_CONF_H

struct mdma_set_data {
	unsigned long long mdma_src;
	unsigned long long mdma_dst;
	unsigned int mdma_len;
};
#endif
