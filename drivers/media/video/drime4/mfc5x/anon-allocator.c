/*
 * linux/drivers/media/video/samsung/mfc5x/mfc_dev.c
 *
 * Copyright (c) 2010 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com/
 *
 * Driver interface for Samsung MFC (Multi Function Codec - FIMV) driver
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/interrupt.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <linux/wait.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/dma-mapping.h>
#include <linux/delay.h>
#include <linux/jiffies.h>
#include <linux/types.h>

#include <linux/sched.h>
#include <linux/firmware.h>

#include <asm/io.h>
#include <asm/uaccess.h>

#include "./mfc_log.h"
#include "anon-allocator_interface.h"

#define ALLOCATOR_DEVICE_FILENAME "/dev/allocator"
#define ALLOCATOR_DEVICE_MINOR 133

enum is_allocated {
	FREE = 1,
	ALLOCATED = 2,
	MMAPED = 4,
};

struct allocator_mem_info {
	unsigned int id;
	unsigned int phy_addr;
	unsigned int mmap_addr;
	unsigned int size;
	enum is_allocated buf_state;
	struct list_head queued_entry;
};

struct allocator_ctx {
	struct mutex lock;
	struct list_head src_queue;
	unsigned int src_queue_cnt;
};

struct allocator_dev {
	struct device *dev;
};

struct allocator_dev *allocator_dev;

static int allocator_open(struct inode *inode, struct file *file)
{
	int ret = 0;
	struct allocator_ctx *allocator_ctx;

	/* prevent invalid reference */
	file->private_data = NULL;

	allocator_ctx = kzalloc(sizeof(struct allocator_ctx), GFP_KERNEL);
	INIT_LIST_HEAD(&allocator_ctx->src_queue);
	if (!allocator_ctx) {
		printk("failed to create ctx\n");
		return -1;
	}

	file->private_data = (struct allocator_ctx *)allocator_ctx;

	return ret;
}

static long allocator_ioctl(struct file *file, unsigned int cmd, unsigned long arg) 
{
	unsigned int ret, ex_ret;
	struct buf_arg in_param;
	struct allocator_mem_info *mem_info;
	struct allocator_ctx *allocator_ctx;
	unsigned char *p;
	unsigned int i=1;

	ret = copy_from_user(&in_param, (struct buf_arg *)arg,
			sizeof(struct buf_arg));
	if (ret < 0) {
		printk("failed to copy parameters\n");
		ret = -EIO;
		goto out_ioctl;
	}
	allocator_ctx = file->private_data;

	switch (cmd) {

	case IOCTL_GET_BUF_P:
			//ret = cma_alloc_from("mfcinput", in_param.size, 2048);
			if (IS_ERR_VALUE(ret))
				goto error;
			/* Need to be fixed */
			in_param.phy_addr = 0xc8200000;

			mem_info = (struct allocator_mem_info *)kzalloc(sizeof(struct \
				allocator_mem_info), GFP_KERNEL);

			mem_info->id = allocator_ctx->src_queue_cnt;
			mem_info->size = PAGE_ALIGN(in_param.size);
			allocator_ctx->src_queue_cnt++;

			mem_info->buf_state = ALLOCATED;
			mem_info->phy_addr = in_param.phy_addr;
			list_add_tail(&mem_info->queued_entry, &allocator_ctx->src_queue);

			break;

	case IOCTL_TEST_KMALLOC:
			while (1) {
			p = kmalloc(in_param.size, GFP_KERNEL);
			if (p == NULL) {
				printk("not enough space\n");
				goto error;
			}
			printk("[%d]th kmalloc call is success size[%d]\n", i, in_param.size);
			i++;
			}

			break;
	default:
		printk("failed to execute ioctl cmd: 0x%08x\n", cmd);
		ret = -EINVAL;
	}

out_ioctl:
	ex_ret = copy_to_user((struct buf_arg *)arg,
			&in_param,
			sizeof(struct buf_arg));
	if (ex_ret < 0) {
		mfc_err("Outparm copy to user error\n");
		ret = -EIO;
	}
	return ret;
error:
	return ret;
}


static int allocator_mmap(struct file *file, struct vm_area_struct *vma)
{
	unsigned int page_frame_no = 0, vm_size = 0;
	struct allocator_mem_info *mem_info = NULL;
	struct allocator_ctx *allocator_ctx;
	allocator_ctx = (struct allocator_ctx *)file->private_data;

	/*mutex_lock(&allocator_ctx->lock);*/

	vma->vm_flags |= VM_RESERVED | VM_IO;
	vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
	vm_size = vma->vm_end - vma->vm_start;
	vm_size =  ALIGN(vm_size, 8);

	/* get mem_info to current index and operation mode. */

	list_for_each_entry(mem_info, &allocator_ctx->src_queue, queued_entry) {
		if (mem_info->buf_state == ALLOCATED) {
			printk("%s:[%d]th buffer is going to be \
				mmaped\n", __func__, mem_info->id);
			break;
		} else {
			mfc_err("there is no free buffer\n");
		}
	}

	if (mem_info == NULL) {
		printk("failed to mmap buffer.\n");
		mutex_unlock(&allocator_ctx->lock);
		return -EFAULT;
	}

	/* reserved memory size should be bigger then vm_size. */
	if (vm_size > mem_info->size) {
		printk(	"reserved memory size(%d) is less then vm_size(%d).\n", \
			mem_info->size, vm_size);
		mutex_unlock(&allocator_ctx->lock);
		return -EINVAL;
	}

	page_frame_no = __phys_to_pfn(mem_info->phy_addr);

	if ((remap_pfn_range(vma, vma->vm_start, page_frame_no,
			vm_size, vma->vm_page_prot)) != 0) {
		printk("%s: failed to mmap.\n", __func__);
		mutex_unlock(&allocator_ctx->lock);
		return -EINVAL;
	}

	mem_info->mmap_addr = vma->vm_start;
	mem_info->buf_state |= MMAPED;

	/*mutex_unlock(&allocator_ctx->lock);*/

	return 0;
}

static struct file_operations allocator_fops = {
	.open		= allocator_open,
	.unlocked_ioctl	= allocator_ioctl,
	.mmap		= allocator_mmap,
};

static struct miscdevice allocator_miscdev = {
	ALLOCATOR_DEVICE_MINOR, "allocator", &allocator_fops
};

static int __init allocator_init(void)
{
	if (misc_register(&allocator_miscdev) != 0) {
		printk(KERN_ERR "allocator device registration failed..\n");
		return -1;
	}

	allocator_dev = kzalloc(sizeof(struct allocator_dev), GFP_KERNEL);
	if(!allocator_dev){
		printk(KERN_ERR "allocator kzalloc failed..\n");
		misc_deregister(&allocator_miscdev);
		return -1;
	}
	allocator_dev->dev = allocator_miscdev.this_device;

	printk("%s: started, device_name: %s\n", __func__, \
			allocator_dev->dev->kobj.name);
	return 0;
}

static void __exit allocator_exit(void)
{
	misc_deregister(&allocator_miscdev);
	printk("allocator driver exit.\n");
}

module_init(allocator_init);
module_exit(allocator_exit);

MODULE_AUTHOR("Sung hwan soon");
MODULE_DESCRIPTION("allocator Device Driver");
MODULE_LICENSE("GPL");
