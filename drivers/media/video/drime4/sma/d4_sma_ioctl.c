/**
 * @file d4_sma_ioctl.c
 * @brief DRIMe4 SMA Ioctl Control Function File
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
#include <mach/d4_cma.h>
#include "d4_sma_if.h"
#include "media/drime4/sma/d4_sma_ioctl.h"

/**
 * @brief SMA device open
 * @fn int sma_open(struct inode *inode, struct file *filp)
 * @param struct inode *inode, struct file *filp
 * @return int
 * @author Junkwon Choi <junkwon.choi@samsung.com>
 * @note  NONE
 */
int sma_open(struct inode *inode, struct file *filp)
{
	return 0;
}

/**
 * @brief SMA device release
 * @fn int sma_release(struct inode *inode, struct file *filp)
 * @param struct inode *inode, struct file *filp
 * @return int
 * @author Junkwon Choi <junkwon.choi@samsung.com>
 * @note  NONE
 */
static int sma_release(struct inode *inode, struct file *filp)
{
	return 0;
}

/**
 * @brief SMA device read
 * @fn ssize_t sma_read(struct file *filp, char *buf, size_t count, loff_t *f_pos)
 * @param struct file *filp, char *buf, size_t count, loff_t *f_pos
 * @return ssize_t
 * @author Junkwon Choi <junkwon.choi@samsung.com>
 * @note  NONE
 */
ssize_t sma_read(struct file *filp, char *buf, size_t count, loff_t *f_pos)
{
	return 0;
}

/**
 * @brief SMA device write
 * @fn ssize_t sma_write(struct file *filp, char *buf, size_t count, loff_t *f_pos)
 * @param struct file *filp, char *buf, size_t count, loff_t *f_pos
 * @return ssize_t
 * @author Junkwon Choi <junkwon.choi@samsung.com>
 * @note  NONE
 */
ssize_t sma_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos)
{
	return 0;
}

/**
 * @brief SMA DD interface function
 * @fn long sma_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
 * @param struct file *filp, unsigned int cmd, unsigned long arg
 * @return long
 * @author Junkwon Choi <junkwon.choi@samsung.com>
 * @note  NONE
 */
long sma_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	long err = -1;
	int size = 0, ret = 0;
	unsigned int addr = 0, buf_size = 0;
	int flags = 0;

	struct SMA_Buffer_Info info;
	info.addr = 0;
	info.size = 0;

	if (_IOC_TYPE(cmd) != SMA_MAGIC)
		return err;

	size = _IOC_SIZE(cmd);

	if (_IOC_DIR(cmd) & _IOC_READ)
		ret = access_ok(VERIFY_WRITE, (void *)arg, size);
	else if (_IOC_DIR(cmd) & _IOC_WRITE)
		ret = access_ok(VERIFY_READ, (void *)arg, size);

	if (!ret)
		return err;

	switch (cmd) {
	case SMA_GET_REGION_START_ADDR:
		addr = CMA_REGION1_START;
		if (copy_to_user((void *)arg, (const void *)&addr, size))
			return -EFAULT;
		break;
	case SMA_GET_REGION_SIZE:
		buf_size = CMA_REGION1_SMA_SIZE;

		if (copy_to_user((void *)arg, (const void *)&buf_size, size))
			return -EFAULT;
		break;

	case SMA_ALLOC:
		if (copy_from_user((void *)&info, (const void *)arg, size))
			return -EFAULT;

		if (d4_sma_alloc_buf(&info))
			return -EFAULT;

		if (copy_to_user((void *)arg, (const void *)&info, size))
			return -EFAULT;

		break;

	case SMA_FREE:
		if (copy_from_user((void *)&addr, (const void *)arg, size))
			return -EFAULT;

		addr = d4_uservirt_to_phys(addr);

		if (d4_sma_free_buf(addr))
			return -EFAULT;

		break;

	case SMA_FREE_PHYS:
		if (copy_from_user((void *)&addr, (const void *)arg, size))
			return -EFAULT;

		if (d4_sma_free_buf(addr))
			return -EFAULT;

		break;

	case SMA_VIRT_TO_PHYS:
		if (copy_from_user((void *)&addr, (const void *)arg, size))
			return -EFAULT;

		addr = d4_uservirt_to_phys(addr);

		if (copy_to_user((void *)arg, (const void *)&addr, size))
			return -EFAULT;
		break;
	case SMA_SET_CACHE:
		if (copy_from_user((void *)&flags, (const void *)arg, size))
			return -EFAULT;

		if (flags == 0)
			filp->f_flags = O_RDWR | O_SYNC;
		else
			filp->f_flags = O_RDWR;
		break;
	case SMA_GET_ALLOCATED_SIZE:
		buf_size = d4_sma_get_allocated_memory_size();

		if (copy_to_user((void *)arg, (const void *)&buf_size, size))
			return -EFAULT;
		break;
	case SMA_CACHE_FLUSH:
		if (copy_from_user((void *)&info, (const void *)arg, size))
			return -EFAULT;
		dmac_flush_range((void *)info.addr, (void *)(info.addr +  info.size));
		outer_flush_range(info.addr, info.addr +  info.size);
		break;		
	case SMA_SET_REMAIN_BUF:
		if (copy_from_user((void *)&info, (const void *)arg, size))
			return -EFAULT;
#if defined( SMA_TEMP_RESERVED_AREA )
		d4_sma_set_remain_buf(&info);
#endif
		break;
	default:
		break;
	}

	return 0;
}

/**
  * @brief SMA로 할당된 버퍼 영역에 대한 user virtual memory address를 생성함
  * @fn int sma_mmap(struct file *file, struct vm_area_struct *vma)
  * @param *file[in] driver file discripter
  * @param *vma[in/out] virtual memory map을 구성하기 위한 정보 structure<br>
    * @return On error returns a negative error, zero otherwise. <br>
  *
  * @author Junkwon Choi <junkwon.choi@samsung.com>
  * @note
  */
int sma_mmap(struct file *file, struct vm_area_struct *vma)
{
	unsigned int offset;
	size_t size;

	offset = vma->vm_pgoff << PAGE_SHIFT;
	/*offset = virt_to_phys((void *)offset);*/
	/*offset = vma->vm_pgoff;*/

	offset = __phys_to_pfn(offset);
	size = vma->vm_end - vma->vm_start;

	vma->vm_flags |= VM_RESERVED; /* swap out 방지 코드 */

	if (file->f_flags & O_SYNC) {
		/* vma->vm_flags |= VM_IO; */
		vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
	}

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

const struct file_operations sma_ops = {
		.open = sma_open,
		.release = sma_release,
		.read = sma_read,
		.write = sma_write,
		.unlocked_ioctl = sma_ioctl,
		.mmap = sma_mmap
};

MODULE_AUTHOR("Junkwon Choi <junkwon.choi@samsung.com>");
MODULE_DESCRIPTION("Samsung DRIMe4 SMA driver using ioctl");
MODULE_LICENSE("GPL");

