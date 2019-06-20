/**
 * @file d4_pp_3a_ioctl.c
 * @brief DRIMe4 PP 3a Ioctl Control Function File
 * @author Kyounghwan Moon <kh.moon@samsung.com>,
 * 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */


#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/io.h>
#include <linux/interrupt.h>
#include <linux/poll.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/slab.h>

#include <media/drime4/pp/pp_3a/d4_pp_3a_ioctl.h>
#include "d4_pp_3a_if.h"

static DECLARE_WAIT_QUEUE_HEAD(waitqueue_3a_dma_intr);

extern struct drime4_pp_3a *g_pp_3a;
extern int pp_3a_irq_num;


/**
 * brief 3A device open 함수
 * fn int pp_3a_open(struct inode *inode, struct file *filp)
 * param  3A device file 관련 정보
 * return error state
 * author Kyounghwan Moon
 */
int pp_3a_open(struct inode *inode, struct file *filp)
{
	struct drime4_pp_3a *pp_3a_ctx = g_pp_3a;
	filp->private_data = pp_3a_ctx;

	return 0;
}

/**
 * brief 3A device release 함수
 * fn static int pp_3a_release(struct inode *inode, struct file *filp)
 * param  3A device file 관련 정보
 * return 0
 * author Kyounghwan Moon
 */
static int pp_3a_release(struct inode *inode, struct file *filp)
{
	return 0;
}

/**
 * brief 3A device read 함수
 * fn ssize_t pp_3a_read(struct file *filp, char *buf, size_t count, loff_t *f_pos)
 * param  struct file *filp, char *buf, size_t count, loff_t *f_pos
 * return 0
 * author Kyounghwan Moon
 * note - 현재까지 사용 용도 없음
 */
ssize_t pp_3a_read(struct file *filp, char *buf, size_t count, loff_t *f_pos)
{
	return 0;
}

/**
 * brief 3A device write 함수
 * fn ssize_t pp_3a_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos)
 * param  struct file *filp, char *buf, size_t count, loff_t *f_pos
 * return 0
 * author Kyounghwan Moon
 * note - 현재까지 사용 용도 없음
 */
ssize_t pp_3a_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos)
{
	return 0;
}


/**
 * brief 3A device ioctl 함수
 * fn long pp_3a_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
 * param  3A device file pointer, command, argument
 * return error state
 * author Kyounghwan Moon
 */
long pp_3a_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	int size;
	int err = -1;
	unsigned int val;
	int ret = -1;

	if (_IOC_TYPE(cmd) != PP_3A_MAGIC)
		return ret;

	size = _IOC_SIZE(cmd);

	if (_IOC_DIR(cmd) & _IOC_READ)
		err = access_ok(VERIFY_WRITE, (void *) arg, size);
	else if (_IOC_DIR(cmd) & _IOC_WRITE)
		err = access_ok(VERIFY_READ, (void *) arg, size);

	if (!err)
		return ret;

	switch (cmd) {
	case PP_3A_IOCTL_OPEN_IRQ:
		pp_3a_request_irq();
		break;
	case PP_3A_IOCTL_CLOSE_IRQ:
		pp_3a_free_irq();
		break;
	case PP_3A_IOCTL_WAIT_DMA_DONE:
		pp_3a_wait_dma_done();
		break;
	case PP_3A_IOCTL_RESULT_MEM_SET:
		pp_3a_set_result_mem_addr_info();
		break;
	case PP_3A_IOCTL_3A_DISABLE_AFTER_CAPTURE:
		pp_3a_disable_after_capture();
		break;

	default:
		break;
	}
	return ret;
}

/**
 * brief 3A device mmap 함수
 * fn int pp_3a_mmap(struct file *file, struct vm_area_struct *vma)
 * param  struct file *file, struct vm_area_struct *vma
 * return error state
 * author Kyounghwan Moon
 */
int pp_3a_mmap(struct file *file, struct vm_area_struct *vma)
{
	unsigned int offset;
	size_t size;

	offset = vma->vm_pgoff << PAGE_SHIFT;

	offset = __phys_to_pfn(offset);
	size = vma->vm_end - vma->vm_start;

	vma->vm_flags |= VM_RESERVED; /* swap out 방지 코드 */

	vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);

	/* Remap-pfn-range will mark the range VM_RESERVED */
	if (remap_pfn_range(vma,
			    vma->vm_start,
			    offset,
			    size,
			    vma->vm_page_prot)) {
		return -EAGAIN;
	}

	return 0;
}

const struct file_operations pp_3a_ops = {
	.open				= pp_3a_open,
	.release				= pp_3a_release,
	.read				= pp_3a_read,
	.write				= pp_3a_write,
	.unlocked_ioctl			= pp_3a_ioctl,
	.mmap 				= pp_3a_mmap,
};

MODULE_AUTHOR("Kyounghwan Moon <kh.moon@samsung.com>");
MODULE_DESCRIPTION("Samsung DRIMe4 PP 3A driver using ioctl");
MODULE_LICENSE("GPL");

