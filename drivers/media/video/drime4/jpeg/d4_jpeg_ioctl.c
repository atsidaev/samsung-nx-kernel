/**
 * @file d4_jpeg_ioctl.c
 * @brief DRIMe4 JPEG Ioctl Control Function File
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
#include <linux/completion.h>
#include <linux/mm.h>
#include <linux/delay.h>
#include <asm/page.h>
#include <asm/cacheflush.h>
#include <linux/dma-mapping.h>

#include "d4_jpeg_if.h"
#include "media/drime4/jpeg/d4_jpeg_ioctl.h"

#ifdef CONFIG_PMU_SELECT
#include <media/drime4/opener/d4_opener_ioctl.h>
#endif

extern struct drime4_jpeg *jpeg_main_ctx;

/**
 * @brief JPEG device open
 * @fn d4_jpeg_open(struct inode *inode, struct file *filp)
 * @param struct inode *inode, struct file *filp
 * @return int
 * @author JinHyoung An <jh0913.an@samsung.com>
 * @note  NONE
 */
int d4_jpeg_open(struct inode *inode, struct file *filp)
{
#ifdef CONFIG_PMU_SELECT
	enum kdd_open_flag ret;
#endif

	/* int num = MINOR(inode->i_rdev);*/
	struct drime4_jpeg *jpeg_ctx = jpeg_main_ctx;
	filp->private_data = jpeg_ctx;

#ifdef CONFIG_PMU_SELECT
	ret = d4_kdd_open(KDD_JPEG);
	switch (ret) {
	case KDD_OPEN_FIRST:
		/* TODO: PMU OFF */
		jpeg_k_pmu_on_off(K_JPEG_OFF);
		//printk("JPEG KDD is opened! count: %d\n", d4_get_kdd_open_count(KDD_JPEG));
		break;
	case KDD_OPEN:
		//printk("JPEG KDD is already opened! count: %d\n", d4_get_kdd_open_count(KDD_JPEG));
		break;
	case KDD_OPEN_EXCEED_MAXIMUM_OPEN:
		printk("JPEG KDD has exceeded the maximum number of open! count: %d\n", d4_get_kdd_open_count(KDD_JPEG));
		break;
	default:
		break;
	}
#endif

	return 0;
}

/**
 * @brief JPEG device release
 * @fn d4_jpeg_release(struct inode *inode, struct file *filp)
 * @param struct inode *inode, struct file *filp
 * @return int
 * @author JinHyoung An <jh0913.an@samsung.com>
 * @note  NONE
 */
static int d4_jpeg_release(struct inode *inode, struct file *filp)
{
#ifdef CONFIG_PMU_SELECT
	enum kdd_close_flag ret = d4_kdd_close(KDD_JPEG);

	switch (ret) {
	case KDD_CLOSE_FINAL:
		/* TODO: PMU ON */
		jpeg_k_pmu_on_off(K_JPEG_ON);
		//printk("JPEG KDD/fd is closed! count: %d\n", d4_get_kdd_open_count(KDD_JPEG));
		break;
	case KDD_CLOSE:
		//printk("JPEG KDD is closed! count: %d\n", d4_get_kdd_open_count(KDD_JPEG));
		break;
	case KDD_CLOSE_ERROR:
	default:
		printk("JPEG KDD close is failed! count: %d\n", d4_get_kdd_open_count(KDD_JPEG));
		break;
	}
#endif
	return 0;
}
/**
 * @brief JPEG device read
 * @fn d4_jpeg_read(struct file *filp, char *buf, size_t count, loff_t *f_pos)
 * @param struct file *filp, char *buf, size_t count, loff_t *f_pos
 * @return ssize_t
 * @author JinHyoung An <jh0913.an@samsung.com>
 * @note  NONE
 */
ssize_t d4_jpeg_read(struct file *filp, char *buf, size_t count, loff_t *f_pos)
{
	return 0;
}

/**
 * @brief JPEG device write
 * @fn d4_jpeg_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos)Kernel dependency한
 * @param struct file *filp, const char *buf, size_t count, loff_t *f_pos
 * @return ssize_t
 * @author JinHyoung An <jh0913.an@samsung.com>
 * @note  NONE
 */
ssize_t d4_jpeg_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos)
{
	return 0;
}

/**
 * @brief JPEG DD ioctl function
 * @fn d4_jpeg_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
 * @param struct file *filp, unsigned int cmd, unsigned long arg
 * @return long
 * @author JinHyoung An <jh0913.an@samsung.com>
 * @note  NONE
 */
long d4_jpeg_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	int size = 0;
	int err = -1;
	unsigned int jpeg_clock_set = 0;
	struct JPEG_GET_REG_INFO jpegReg;
	struct JPEG_IOCTL_SEMA_INFO jpegSem;
	memset((void *)&jpegReg, 0x0, sizeof(struct JPEG_GET_REG_INFO));
	memset((void *)&jpegSem, 0x0, sizeof(struct JPEG_IOCTL_SEMA_INFO));

	if (_IOC_TYPE(cmd) != JPEG_MAGIC)
		return -1;

	size = _IOC_SIZE(cmd);

	if (_IOC_DIR(cmd) & _IOC_READ)
		err = access_ok(VERIFY_WRITE, (void *) arg, size);
	else if (_IOC_DIR(cmd) & _IOC_WRITE)
		err = access_ok(VERIFY_READ, (void *) arg, size);

	if (!err)
		return -1;

	switch (cmd) {
	case JPEG_IOCTL_OPEN_IRQ:
		jpeg_enable_irq();
		break;

	case JPEG_IOCTL_CLOSE_IRQ:
		jpeg_disable_irq();
		break;

	case JPEG_IOCTL_INIT_COMPLETION:
		jpeg_init_completion();
		break;

	case JPEG_IOCTL_SET_CLOCK:
		if (copy_from_user((void *)&jpeg_clock_set, (const void *)arg, size)) {
			printk("ioctl fail: [%d]", cmd);
			return -EFAULT;
		}
		jpeg_set_clock(jpeg_clock_set);
		break;

	case JPEG_IOCTL_REGISTER_INT:
		err = jpeg_register_interrupt();

		if (copy_to_user((void *)arg, (const void *)&err, size)) {
			printk("ioctl fail: [%d]", cmd);
			return -EFAULT;
		}
		break;

	case JPEG_IOCTL_DEREGISTER_INT:
		err = jpeg_deregister_interrupt();

		if (copy_to_user((void *)arg, (const void *)&err, size)) {
			printk("ioctl fail: [%d]", cmd);
			return -EFAULT;
		}
		break;

	case JPEG_IOCTL_SEMAPHORE:
		if (copy_from_user((void *)&jpegSem, (const void *)arg, size)) {
			printk("ioctl fail: [%d]", cmd);
			return -EFAULT;
		}
		err = jpeg_get_operation_sema(jpegSem.timeOut);
		jpegSem.result = err;

		if (copy_to_user((void *)arg, (const void *)&jpegSem, size)) {
			printk("ioctl fail: [%d]", cmd);
			return -EFAULT;
		}
		break;

	case JPEG_IOCTL_GET_PHYS_REG_INFO:
		if (copy_from_user((void *)&jpegReg, (const void *)arg, size)) {
			printk("ioctl fail: [%d]", cmd);
			return -EFAULT;
		}
		jpeg_get_phys_reg_info(&jpegReg);

		if (copy_to_user((void *)arg, (const void *)&jpegReg, size)) {
			printk("ioctl fail: [%d]", cmd);
			return -EFAULT;
		}
		break;

	case JPEG_IOCTL_CHECK_ENC_STATUS:
		err = jpeg_checkEncodeStatus();

		if (copy_to_user((void *)arg, (const void *)&err, size)) {
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
 * @fn int d4_jpeg_mmap(struct file *file, struct vm_area_struct *vma)
 * @param *file[in] driver file discripter
 * @param *vma[in/out] virtual memory map을 구성하기 위한 정보 structure<br>
 * @return On error returns a negative error, zero otherwise. <br>
 *
 * @author Junkwon Choi
 * @note
 */
int d4_jpeg_mmap(struct file *file, struct vm_area_struct *vma)
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

const struct file_operations jpeg_ops = {
	.open = d4_jpeg_open,
	.release = d4_jpeg_release,
	.read = d4_jpeg_read,
	.write = d4_jpeg_write,
	.unlocked_ioctl = d4_jpeg_ioctl,
	.mmap = d4_jpeg_mmap
};

MODULE_AUTHOR("JinHyoung An <jh0913.an@samsung.com>");
MODULE_DESCRIPTION("Samsung DRIMe4 JPEG driver using ioctl");
MODULE_LICENSE("GPL");

