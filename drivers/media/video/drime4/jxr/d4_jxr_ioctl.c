/**
 * @file d4_jxr_ioctl.c
 * @brief DRIMe4 JPEG XR Ioctl Control Function File
 * @author JinHyoung An <jh0913.an@samsung.com>
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
#include <linux/slab.h>
#include <asm/page.h>

#include <linux/uaccess.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <asm/cacheflush.h>

#include <mach/d4_mem.h>
#include "d4_jxr_if.h"
#include "media/drime4/jxr/d4_jxr_ioctl.h"

extern struct drime4_jxr *jxr_main_ctx;

/**
 * @brief JPEG_XR device open
 * @fn d4_jxr_open(struct inode *inode, struct file *filp)
 * @param struct file *filp, char *buf, size_t count, loff_t *f_pos
 * @return int
 * @author JinHyoung An <jh0913.an@samsung.com>
 * @note  NONE
 */
int d4_jxr_open(struct inode *inode, struct file *filp)
{
   struct drime4_jxr *jxr_core_ctx = jxr_main_ctx;
   filp->private_data = jxr_core_ctx;

   return 0;
}

/**
 * @brief JPEG_XR device release
 * @fn d4_jxr_release(struct inode *inode, struct file *filp)
 * @param struct file *filp, char *buf, size_t count, loff_t *f_pos
 * @return ssize_t
 * @author JinHyoung An <jh0913.an@samsung.com>
 * @note  NONE
 */
static int d4_jxr_release(struct inode *inode, struct file *filp)
{
   return 0;
}

/**
 * @brief JPEG_XR device read
 * @fn d4_jxr_read(struct file *filp, char *buf, size_t count, loff_t *f_pos)
 * @param struct file *filp, char *buf, size_t count, loff_t *f_pos
 * @return ssize_t
 * @author JinHyoung An <jh0913.an@samsung.com>
 * @note  NONE
 */
ssize_t d4_jxr_read(struct file *filp, char *buf, size_t count, loff_t *f_pos)
{
   return 0;
}

/**
 * @brief JPEG_XR device write
 * @fn d4_jxr_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos)
 * @param struct file *filp, char *buf, size_t count, loff_t *f_pos
 * @return ssize_t
 * @author JinHyoung An <jh0913.an@samsung.com>
 * @note  NONE
 */
ssize_t d4_jxr_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos)
{
   return 0;
}

/**
 * @brief JPEG_XR DD interface function
 * @fn d4_jxr_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
 * @param struct file *filp, unsigned int cmd, unsigned long arg
 * @return long
 * @author JinHyoung An <jh0913.an@samsung.com>
 * @note  NONE
 */
long d4_jxr_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	int size = 0;
	int err = JXR_FAIL;
	struct JXR_SEMA_INFO jxrSema;
	struct JXR_PHYS_REG_INFO jxrReg;
	signed int result = JXR_FAIL;

	if (_IOC_TYPE(cmd) != JXR_MAGIC)
		return JXR_FAIL;

	size = _IOC_SIZE(cmd);

	if (_IOC_DIR(cmd) & _IOC_READ)
		err = access_ok(VERIFY_WRITE, (void *)arg, size);
	else if (_IOC_DIR(cmd) & _IOC_WRITE)
		err = access_ok(VERIFY_READ, (void *)arg, size);

	if (!err)
		return JXR_FAIL;

	switch (cmd) {
	case JXR_ENABLE_IRQ:
		result = jxr_initIRQ();

		if (copy_to_user((void *) arg, (const void *)&result, size)) {
			printk("ioctl fail: [%d]", cmd);
			return -EFAULT;
		}
		break;

	case JXR_DISABLE_IRQ:
		jxr_deInitIRQ();
		break;

	case JXR_INIT_COMPLETION:
		jxr_init_completion();
		break;

	case JXR_SEMAPHORE:
		if (copy_from_user((void *) &jxrSema, (const void *) arg, size)) {
			printk("ioctl fail: [%d]", cmd);
			return -EFAULT;
		}
		jxrSema.timeOut = (signed int)jxr_getSema(jxrSema.timeOut);

		jxrSema.result = jxrSema.timeOut ? JXR_SUCCESS : JXR_SEMA_TIMEOUT;

		if (copy_to_user((void *) arg, (const void *)&jxrSema, size)) {
			printk("ioctl fail: [%d]", cmd);
			return -EFAULT;
		}
		break;

	case JXR_REGISTER_INT:
		result = jxr_registerInterrupt();

		if (copy_to_user((void *)arg, (const void *)&result, size)) {
			printk("ioctl fail: [%d]", cmd);
			return -EFAULT;
		}
		break;

	case JXR_DEREGISTER_INT:
		result = jxr_deRegisterInterrupt();

		if (copy_to_user((void *)arg, (const void *)&result, size)) {
			printk("ioctl fail: [%d]", cmd);
			return -EFAULT;
		}
		break;

	case JXR_GET_PHYS_REG_INFO:
		if (copy_from_user((void *) &jxrReg, (const void *)arg, size)) {
			printk("ioctl fail: [%d]", cmd);
			return -EFAULT;
		}
		jxr_get_phys_reg_info(&jxrReg);

		if (copy_to_user((void *)arg, (const void *)&jxrReg, size)) {
			printk("ioctl fail: [%d]", cmd);
			return -EFAULT;
		}
		break;


	default:
		break;
	}

   return 0;
}

/**
  * @brief cma로 할당된 영역에 대한 application 접근용 virtual address를 생성해주는 함수
  * @fn int jxr_mmap(struct file *file, struct vm_area_struct *vma)
  * @param struct file *, struct vm_area_struct *
  * @param *vma[in/out] virtual memory map을 구성하기 위한 정보 structure<br>
    * @return On error returns a negative error, zero otherwise. <br>
  *
  * @author JinHyoung An <jh0913.an@samsung.com>
  * @note
  */
int d4_jxr_mmap(struct file *file, struct vm_area_struct *vma)
{
	unsigned long size = 0;

	size = (vma->vm_end - vma->vm_start);

	vma->vm_flags |= VM_IO;
	vma->vm_flags |= VM_RESERVED;
	vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);

	/* Remap-pfn-range will mark the range VM_IO and VM_RESERVED */
	if (io_remap_pfn_range(vma, vma->vm_start, vma->vm_pgoff, size,
					vma->vm_page_prot)) {
		return -EAGAIN;
	}

	return 0;
}


const struct file_operations jxr_ops = {
			.open = d4_jxr_open,
			.release = d4_jxr_release,
			.read = d4_jxr_read,
			.write = d4_jxr_write,
			.unlocked_ioctl = d4_jxr_ioctl,
			.mmap = d4_jxr_mmap
};

MODULE_AUTHOR("JinHyoung An <jh0913.an@samsung.com>");
MODULE_DESCRIPTION("Samsung DRIMe4 JXR driver using ioctl");
MODULE_LICENSE("GPL");

