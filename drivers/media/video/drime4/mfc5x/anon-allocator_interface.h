/*
 * Copyright (c) 2011 Samsung Electronics Co., Ltd.
 *              http://www.samsung.com/
 *
 * Global header for dummy driver
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Alternatively, Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __MFC_INTERFACE_H
#define __MFC_INTERFACE_H __FILE__

#define IOCTL_GET_BUF_P			(0x00800001)
#define IOCTL_TEST_KMALLOC		(0x00800002)

struct buf_arg {
	unsigned int index;
	unsigned int phy_addr;
	unsigned int vir_addr;
	unsigned int mmaped_addr;
	unsigned int size;
	unsigned int malloc_opt;
};

#endif /* __MFC_INTERFACE_H */
