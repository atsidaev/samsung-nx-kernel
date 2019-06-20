/**
 * @file d4_bma_ioctl.c
 * @brief DRIMe4 BMA Ioctl Control Function File
 * @author Junkwon Choi <junkwon.choi@samsung.com>
 * 2012 Samsung Electronics
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

#include <linux/uaccess.h>
#include <asm/cacheflush.h>

#include <mach/d4_mem.h>
#include "d4_bma_if.h"
#include "media/drime4/bma/d4_bma_ioctl.h"

/**
 * @brief BMA device open
 * @fn int bma_open(struct inode *inode, struct file *filp)
 * @param struct inode *inode, struct file *filp
 * @return int
 * @author Junkwon Choi <junkwon.choi@samsung.com>
 * @note  NONE
 */
int bma_open(struct inode *inode, struct file *filp)
{
	return 0;
}

/**
 * @brief BMA device release
 * @fn int bma_release(struct inode *inode, struct file *filp)
 * @param struct inode *inode, struct file *filp
 * @return int
 * @author Junkwon Choi <junkwon.choi@samsung.com>
 * @note  NONE
 */
static int bma_release(struct inode *inode, struct file *filp)
{
	return 0;
}

/**
 * @brief BMA device read
 * @fn ssize_t bma_read(struct file *filp, char *buf, size_t count, loff_t *f_pos)
 * @param struct file *filp, char *buf, size_t count, loff_t *f_pos
 * @return ssize_t
 * @author Junkwon Choi <junkwon.choi@samsung.com>
 * @note  NONE
 */
ssize_t bma_read(struct file *filp, char *buf, size_t count, loff_t *f_pos)
{
	return 0;
}

/**
 * @brief BMA device write
 * @fn ssize_t bma_write(struct file *filp, char *buf, size_t count, loff_t *f_pos)
 * @param struct file *filp, char *buf, size_t count, loff_t *f_pos
 * @return ssize_t
 * @author Junkwon Choi <junkwon.choi@samsung.com>
 * @note  NONE
 */
ssize_t bma_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos)
{
	return 0;
}

/**
 * @brief BMA DD interface function
 * @fn long bma_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
 * @param struct file *filp, unsigned int cmd, unsigned long arg
 * @return long
 * @author Junkwon Choi <junkwon.choi@samsung.com>
 * @note  NONE
 */
long bma_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	long err = -1;
	int size = 0, ret = 0;
	unsigned int addr = 0;

	struct BMA_Buffer_Info info;

	if (_IOC_TYPE(cmd) != BMA_MAGIC)
		return err;

	size = _IOC_SIZE(cmd);

	if (_IOC_DIR(cmd) & _IOC_READ)
		ret = access_ok(VERIFY_WRITE, (void *)arg, size);
	else if (_IOC_DIR(cmd) & _IOC_WRITE)
		ret = access_ok(VERIFY_READ, (void *)arg, size);

	if (!ret)
		return err;

	switch (cmd) {
	case BMA_ALLOC:
		if (copy_from_user((void *)&info, (const void *)arg, size))
			return -EFAULT;

		if (d4_bma_alloc_buf(&info))
			return -EFAULT;

		if (copy_to_user((void *)arg, (const void *)&info, size))
			return -EFAULT;
		break;

	case BMA_FREE:
		if (copy_from_user((void *)&addr, (const void *)arg, size))
			return -EFAULT;

		addr = d4_uservirt_to_phys(addr);

		if (d4_bma_free_buf(addr))
			return -EFAULT;
		break;

	case BMA_FREE_PHYS:
		if (copy_from_user((void *)&addr, (const void *)arg, size))
			return -EFAULT;

		if (d4_bma_free_buf(addr))
			return -EFAULT;
		break;

	case BMA_VIRT_TO_PHYS:
		if (copy_from_user((void *)&addr, (const void *)arg, size))
			return -EFAULT;

		addr = d4_uservirt_to_phys(addr);

		if (copy_to_user((void *)arg, (const void *)&addr, size))
			return -EFAULT;
		break;
	default:
		break;
	}

   return 0;
}

/**
  * @brief MBA로 할당된 버퍼 영역에 대한 user virtual memory address를 생성함
  * @fn int bma_mmap(struct file *file, struct vm_area_struct *vma)
  * @param *file[in] driver file discripter
  * @param *vma[in/out] virtual memory map을 구성하기 위한 정보 structure<br>
    * @return On error returns a negative error, zero otherwise. <br>
  *
  * @author Junkwon Choi <junkwon.choi@samsung.com>
  * @note
  */
int bma_mmap(struct file *file, struct vm_area_struct *vma)
{
	unsigned int offset;
	size_t size;

	offset = vma->vm_pgoff << PAGE_SHIFT;
	/*offset = virt_to_phys((void *)offset);*/
	/*offset = vma->vm_pgoff;*/

	offset = __phys_to_pfn(offset);
	size = vma->vm_end - vma->vm_start;

/*
	if (!valid_mmap_phys_addr_range(vma->vm_pgoff, size)){
		return -EINVAL;
	}
*/
	vma->vm_flags |= VM_RESERVED;
	vma->vm_flags |= VM_IO;

	vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);

	/* Remap-pfn-range will mark the range VM_IO and VM_RESERVED */
	if (remap_pfn_range(vma,
			    vma->vm_start,
			    offset,
			    size,
			    vma->vm_page_prot)) {
		return -EAGAIN;
	}

	return 0;
}

const struct file_operations bma_ops = {
	.open = bma_open,
	.release = bma_release,
	.read = bma_read,
	.write = bma_write,
	.unlocked_ioctl = bma_ioctl,
	.mmap = bma_mmap
};

MODULE_AUTHOR("Junkwon Choi <junkwon.choi@samsung.com>");
MODULE_DESCRIPTION("Samsung DRIMe4 BMA driver using ioctl");
MODULE_LICENSE("GPL");
