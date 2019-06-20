/**
 * @file	d4_ipcs_ioctl.c
 * @brief	IPCS driver file for Samsung DRIMe4 using ioctl
 *
 * @author	Dongjin Jung <djin81.jung@samsung.com>
 *
 * Copyright (c) 2012 Samsung Electronics
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
#include <linux/interrupt.h>
#include <linux/completion.h>
#include <linux/jiffies.h>

#include <linux/kthread.h>

#include <linux/poll.h>
#include <linux/interrupt.h>

#include <asm/page.h>
#include <asm/cacheflush.h>
#include <mach/d4_mem.h>
#include <media/drime4/usih_type.h>

#include <media/drime4/ipcs/d4_ipcs_ioctl.h>
#include "d4_ipcs_if.h"

#ifdef CONFIG_PMU_SELECT
#include <media/drime4/opener/d4_opener_ioctl.h>
#endif

#define IPCS_K_BUFFER_NUM 80

extern struct dirme4_ipcs *g_ipcs;

#define MAX_IPCS_READ_QUEUE_CNT 512
static DECLARE_WAIT_QUEUE_HEAD(ipcs_wq_read);

static char ipcs_read_queue[MAX_IPCS_READ_QUEUE_CNT];
static unsigned long ipcs_read_q_cnt;
static unsigned long ipcs_read_q_head;
static unsigned long ipcs_read_q_tail;

extern struct completion ipcs_wdma0_completion;
extern struct completion ipcs_wdma1_completion;
extern struct completion ipcs_wdma2_completion;
extern struct completion ipcs_wdma3_completion;

void _ipcs_send_interrupt(unsigned int intr_type)
{
	unsigned long flags;

	local_save_flags(flags);
	local_irq_disable();

	if (ipcs_read_q_cnt < MAX_IPCS_READ_QUEUE_CNT) {
		ipcs_read_queue[ipcs_read_q_head] = intr_type;
		ipcs_read_q_head =
			(ipcs_read_q_head + 1) % MAX_IPCS_READ_QUEUE_CNT;
		ipcs_read_q_cnt++;
	}

	local_irq_restore(flags);

	wake_up_interruptible(&ipcs_wq_read);
}

/*static unsigned int ipcs_buff_phys_addr[IPCS_K_BUFFER_NUM] = {0,};*/

void _ipcs_wdma0_interrupt_set_callback_function(void)
{
	complete(&ipcs_wdma0_completion);
	_ipcs_send_interrupt(INTR_IPCS_B2Y_DONE);
	/*printk("IPCS WDMA0 Interrupt call back : Tile Operation finish\n");*/
}

void _ipcs_wdma1_interrupt_set_callback_function(void)
{
	complete(&ipcs_wdma1_completion);
	_ipcs_send_interrupt(INTR_IPCS_RSZ_DONE);
	/*printk("IPCS WDMA1 Interrupt call back : Tile Operation finish\n");*/
}

void _ipcs_wdma2_interrupt_set_callback_function(void)
{
	complete(&ipcs_wdma2_completion);
	_ipcs_send_interrupt(INTR_IPCS_SRSZ_DONE);
	/*printk("IPCS WDMA2 Interrupt call back : Tile Operation finish\n");*/
}

void _ipcs_wdma3_interrupt_set_callback_function(void)
{
	complete(&ipcs_wdma3_completion);
	_ipcs_send_interrupt(INTR_IPCS_SRSZ2_DONE);
	/*printk("IPCS WDMA3 Interrupt call back : Tile Operation finish\n");*/
}

void _ipcs_wdma0_interrupt_error_callback_function(void)
{
	printk("IPCS WDMA0 Interrupt Error!\n");
}
void _ipcs_wdma1_interrupt_error_callback_function(void)
{
	printk("IPCS WDMA1 Interrupt Error!\n");
}
void _ipcs_wdma2_interrupt_error_callback_function(void)
{
	printk("IPCS WDMA2 Interrupt Error!\n");
}
void _ipcs_wdma3_interrupt_error_callback_function(void)
{
	printk("IPCS WDMA3 Interrupt Error!\n");
}


static int ipcs_k_open(struct inode *inode, struct file *filp)
{
#ifdef CONFIG_PMU_SELECT
	enum kdd_open_flag ret;
#endif

	/* int num = MINOR(inode->i_rdev);*/
	struct drime4_ipcs *ipcs_ctx = g_ipcs;
	filp->private_data = ipcs_ctx;

#ifdef CONFIG_PMU_SELECT
	ret = d4_kdd_open(KDD_IPCS);
	switch (ret) {
	case KDD_OPEN_FIRST:
		ipcs_k_pmu_on_off(IPCS_K_OFF);
		/*printk("IPCS KDD is opened! count: %d\n", d4_get_kdd_open_count(KDD_IPCS));*/
		break;
	case KDD_OPEN:
		/*printk("IPCS KDD is already opened! count: %d\n", d4_get_kdd_open_count(KDD_IPCS));*/
		break;
	case KDD_OPEN_EXCEED_MAXIMUM_OPEN:
		printk("IPCS KDD has exceeded the maximum number of open! count: %d\n", d4_get_kdd_open_count(KDD_IPCS));
		break;
	default:
		break;
	}
#endif

	return 0;
}

static int ipcs_k_release(struct inode *inode, struct file *filp)
{
#ifdef CONFIG_PMU_SELECT
	enum kdd_close_flag ret = d4_kdd_close(KDD_IPCS);

	switch (ret) {
	case KDD_CLOSE_FINAL:
		ipcs_k_pmu_on_off(IPCS_K_ON);
		/*printk("IPCS KDD/fd is closed! count: %d\n", d4_get_kdd_open_count(KDD_IPCS));*/
		break;
	case KDD_CLOSE:
		/*printk("IPCS KDD is closed! count: %d\n", d4_get_kdd_open_count(KDD_IPCS));*/
		break;
	case KDD_CLOSE_ERROR:
	default:
		printk("IPCS KDD close is failed! count: %d\n", d4_get_kdd_open_count(KDD_IPCS));
		break;
	}
#endif
	return 0;
}

ssize_t ipcs_k_read(struct file *filp, char *buf, size_t count, loff_t *f_pos)
{
	unsigned long flags;
	int retstate;

	if ((!ipcs_read_q_cnt) && (filp->f_flags & O_NONBLOCK))
		return -EAGAIN;

	retstate = wait_event_interruptible(ipcs_wq_read, ipcs_read_q_cnt);

	if (retstate)
		return retstate;

	local_save_flags(flags);
	local_irq_disable();

	if (ipcs_read_q_cnt > 0) {
		put_user(ipcs_read_queue[ipcs_read_q_tail], buf);
		ipcs_read_q_tail =
			(ipcs_read_q_tail + 1) % MAX_IPCS_READ_QUEUE_CNT;
		ipcs_read_q_cnt--;
	}

	local_irq_restore(flags);

	return ipcs_read_q_cnt;
}

ssize_t ipcs_k_write(struct file *filp, const char *buf, size_t count,
		loff_t *f_pos)
{
	unsigned int i = 0;

	for (; i < count; i++) {
		/* get_user(dd_data[i],(int*)buf+i); */
	}
	return i;
}

/**
  * @brief		application 접근용 virtual address를 생성해주는 함수
  * @fn			long ipcs_k_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
  * @param 		*file	[in] driver file discripter
  * @param 		cmd		[in] IOCTL command
  * @param 		arg		[in] argument
  * @return 	On error returns a negative error, zero otherwise.
  *
  * @author		Dongjin Jung
  * @note
  */
long ipcs_k_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	int size;
	int err = -1;
	int ret = 0;

	struct ipcs_k_physical_reg_info ipcs_kernel_reg_info;
	unsigned int clk_rate = 100000000;
	struct ipcs_ioctl_wdma enable;
	struct ipcs_ioctl_wdma_done wdma_check;

	enum ipcs_k_on_off pmu_type = IPCS_K_OFF;

#ifdef IPCS_TIMEOUT_CHECK
	unsigned int timeout = 0;
#endif
	memset(&enable, 0, sizeof(struct ipcs_ioctl_wdma));
	memset(&wdma_check, 0, sizeof(struct ipcs_ioctl_wdma_done));

	if (_IOC_TYPE(cmd) != IPCS_MAGIC)
		return -1;

	size = _IOC_SIZE(cmd);

	if (_IOC_DIR(cmd) & _IOC_READ)
		err = access_ok(VERIFY_WRITE, (void *) arg, size);
	else if (_IOC_DIR(cmd) & _IOC_WRITE)
		err = access_ok(VERIFY_READ, (void *) arg, size);

	if (!err)
		return -1;

	switch (cmd) {

	case IPCS_IOCTL_PHYSICAL_REG_INFO:
		ret = copy_from_user((void *)&ipcs_kernel_reg_info, (const void *)arg, size);
		if (ret < 0) {
			printk("ioctl fail: [%d]", cmd);
			return ret;
		}
		ipcs_k_get_physical_reg_info(&ipcs_kernel_reg_info);

		ret = copy_to_user((void *)arg, (const void *)&ipcs_kernel_reg_info, size);
		if (ret < 0) {
			printk("ioctl fail: [%d]", cmd);
			return ret;
		}
		break;

	case IPCS_IOCTL_CLOCK_FREQUENCY:
		ret = copy_from_user((void *)&clk_rate, (const void *)arg, size);
		if (ret < 0) {
			printk("ioctl fail: [%d]", cmd);
			return ret;
		}
		ipcs_k_clk_set_rate(clk_rate);
		break;

	case IPCS_IOCTL_PMU_ON_OFF:
		ret = copy_from_user((void *)&pmu_type, (const void *)arg, size);
		if (ret < 0) {
			printk("ioctl fail: [%d]", cmd);
			return ret;
		}
		ipcs_k_pmu_on_off(pmu_type);
		break;

	case IPCS_IOCTL_OPEN_IRQ:
		err = ipcs_k_interrupt_init();

		ret = copy_to_user((void *)arg, (const void *)&err, size);
		if (ret < 0) {
			printk("ioctl fail: [%d]", cmd);
			return ret;
		}
		break;
	case IPCS_IOCTL_CLOSE_IRQ:
		ipcs_k_interrupt_free();
		break;

	case IPCS_IOCTL_WAIT_1TILE_DONE:
		ret = copy_from_user((void *)&wdma_check, (const void *)arg, size);
		if (ret < 0) {
			printk("ioctl fail: [%d]", cmd);
			return ret;
		}
#ifndef IPCS_TIMEOUT_CHECK
		if (wdma_check.main_out)
			ipcs_k_wdma0_wait_done();
		if (wdma_check.main_rsz)
			ipcs_k_wdma1_wait_done();
		if (wdma_check.sub_rsz1)
			ipcs_k_wdma2_wait_done();
		if (wdma_check.sub_rsz2)
			ipcs_k_wdma3_wait_done();
#else
		if (wdma_check.main_out) {
			wdma_check.err += ipcs_k_wdma0_wait_for_completion(msecs_to_jiffies(wdma_check.timeout));
			if (wdma_check.err < 0)
				wdma_check.main_out = 0;
		}
		if (wdma_check.main_rsz) {
			wdma_check.err += ipcs_k_wdma1_wait_for_completion(msecs_to_jiffies(wdma_check.timeout));
			if (wdma_check.err < 0)
				wdma_check.main_rsz = 0;
		}
		if (wdma_check.sub_rsz1) {
			wdma_check.err += ipcs_k_wdma2_wait_for_completion(msecs_to_jiffies(wdma_check.timeout));
			if (wdma_check.err < 0)
				wdma_check.sub_rsz1 = 0;
		}
		if (wdma_check.sub_rsz2) {
			wdma_check.err += ipcs_k_wdma3_wait_for_completion(msecs_to_jiffies(wdma_check.timeout));
			if (wdma_check.err < 0)
				wdma_check.sub_rsz2 = 0;
		}

#endif
		ret = copy_to_user((void *)arg, (const void *)&wdma_check, size);
		if (ret < 0) {
			printk("ioctl fail: [%d]", cmd);
			return ret;
		}

		break;


	case IPCS_IOCTL_WAIT_MAINOUT_DONE:
#ifndef IPCS_TIMEOUT_CHECK
		ipcs_k_wdma0_wait_done();
#else
		ret = copy_from_user((void *)&timeout, (const void *)arg, size);
		if (ret < 0) {
			printk("ioctl fail: [%d]", cmd);
			return ret;
		}

		err = ipcs_k_wdma0_wait_for_completion(msecs_to_jiffies(timeout));

		ret = copy_to_user((void *)arg, (const void *)&err, size);
		if (ret < 0) {
			printk("ioctl fail: [%d]", cmd);
			return ret;
		}
#endif
		break;
	case IPCS_IOCTL_WAIT_MRSZ_DONE:
#ifndef IPCS_TIMEOUT_CHECK
		ipcs_k_wdma1_wait_done();
#else
		ret = copy_from_user((void *)&timeout, (const void *)arg, size);
		if (ret < 0) {
			printk("ioctl fail: [%d]", cmd);
			return ret;
		}

		err = ipcs_k_wdma1_wait_for_completion(msecs_to_jiffies(timeout));

		ret = copy_to_user((void *)arg, (const void *)&err, size);
		if (ret < 0) {
			printk("ioctl fail: [%d]", cmd);
			return ret;
		}
#endif
		break;
	case IPCS_IOCTL_WAIT_SRSZ1_DONE:
#ifndef IPCS_TIMEOUT_CHECK
		ipcs_k_wdma2_wait_done();
#else
		ret = copy_from_user((void *)&timeout, (const void *)arg, size);
		if (ret < 0) {
			printk("ioctl fail: [%d]", cmd);
			return ret;
		}

		err = ipcs_k_wdma2_wait_for_completion(msecs_to_jiffies(timeout));

		ret = copy_to_user((void *)arg, (const void *)&err, size);
		if (ret < 0) {
			printk("ioctl fail: [%d]", cmd);
			return ret;
		}
#endif
		break;
	case IPCS_IOCTL_WAIT_SRSZ2_DONE:
#ifndef IPCS_TIMEOUT_CHECK
		ipcs_k_wdma3_wait_done();
#else
		ret = copy_from_user((void *)&timeout, (const void *)arg, size);
		if (ret < 0) {
			printk("ioctl fail: [%d]", cmd);
			return ret;
		}

		err = ipcs_k_wdma3_wait_for_completion(msecs_to_jiffies(timeout));

		ret = copy_to_user((void *)arg, (const void *)&err, size);
		if (ret < 0) {
			printk("ioctl fail: [%d]", cmd);
			return ret;
		}
#endif
		break;

	case IPCS_IOCTL_INTERRUPT_ENABLE:
		ret = copy_from_user((void *)&enable, (const void *)arg, size);
		if (ret < 0) {
			printk("ioctl fail: [%d]", cmd);
			return ret;
		}
		if (enable.main_out)
			ipcs_k_interrupt_enable(IPCS_K_INTR_BUSEND_WDMA0,
							_ipcs_wdma0_interrupt_set_callback_function);
		if (enable.main_rsz)
			ipcs_k_interrupt_enable(IPCS_K_INTR_BUSEND_WDMA1,
							_ipcs_wdma1_interrupt_set_callback_function);
		if (enable.sub_rsz1)
			ipcs_k_interrupt_enable(IPCS_K_INTR_BUSEND_WDMA2,
							_ipcs_wdma2_interrupt_set_callback_function);
		if (enable.sub_rsz2)
			ipcs_k_interrupt_enable(IPCS_K_INTR_BUSEND_WDMA3,
							_ipcs_wdma3_interrupt_set_callback_function);

#ifdef IPCS_INTERRUPT_ERROR_CHECK
		if (enable.main_out)
			ipcs_k_interrupt_enable(IPCS_K_INTR_ERROR_WDMA0,
							_ipcs_wdma0_interrupt_error_callback_function);
		if (enable.main_rsz)
			ipcs_k_interrupt_enable(IPCS_K_INTR_ERROR_WDMA1,
							_ipcs_wdma1_interrupt_error_callback_function);
		if (enable.sub_rsz1)
			ipcs_k_interrupt_enable(IPCS_K_INTR_ERROR_WDMA2,
							_ipcs_wdma2_interrupt_error_callback_function);
		if (enable.sub_rsz2)
			ipcs_k_interrupt_enable(IPCS_K_INTR_ERROR_WDMA3,
							_ipcs_wdma3_interrupt_error_callback_function);
#endif
		break;
	case IPCS_IOCTL_INTERRUPT_DISABLE:
		ret = copy_from_user((void *)&enable, (const void *)arg, size);
		if (ret < 0) {
			printk("ioctl fail: [%d]", cmd);
			return ret;
		}
		if (enable.main_out)
			ipcs_k_interrupt_disable(IPCS_K_INTR_BUSEND_WDMA0);
		if (enable.main_rsz)
			ipcs_k_interrupt_disable(IPCS_K_INTR_BUSEND_WDMA1);
		if (enable.sub_rsz1)
			ipcs_k_interrupt_disable(IPCS_K_INTR_BUSEND_WDMA2);
		if (enable.sub_rsz2)
			ipcs_k_interrupt_disable(IPCS_K_INTR_BUSEND_WDMA3);

#ifdef IPCS_INTERRUPT_ERROR_CHECK
		if (enable.main_out)
			ipcs_k_interrupt_disable(IPCS_K_INTR_ERROR_WDMA0);
		if (enable.main_rsz)
			ipcs_k_interrupt_disable(IPCS_K_INTR_ERROR_WDMA1);
		if (enable.sub_rsz1)
			ipcs_k_interrupt_disable(IPCS_K_INTR_ERROR_WDMA2);
		if (enable.sub_rsz2)
			ipcs_k_interrupt_disable(IPCS_K_INTR_ERROR_WDMA3);
#endif
		break;

	default:
		break;
	}

	return 0;
}

/**
  * @brief		application 접근용 virtual address를 생성해주는 함수
  * @fn			int ipcs_k_mmap(struct file *file, struct vm_area_struct *vma)
  * @param 		*file	[in] driver file discripter
  * @param 		*vma	[in/out] virtual memory map을 구성하기 위한 정보 structure
  * @return 	On error returns a negative error, zero otherwise.
  *
  * @author		Dongjin Jung
  * @note
  */
int ipcs_k_mmap(struct file *file, struct vm_area_struct *vma)
{
	unsigned long size = 0;

	size = vma->vm_end - vma->vm_start;

	vma->vm_flags |= VM_IO;
	vma->vm_flags |= VM_RESERVED;

	vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);

	if (io_remap_pfn_range(vma,
			    vma->vm_start,
			    vma->vm_pgoff,
			    size,
			    vma->vm_page_prot)) {
		return -EAGAIN;
	}
	return 0;
}

/**
  * @brief		application could detect ipcs interrupts
  * @fn			int ipcs_k_poll(struct file *file, poll_table *wait)
  * @param 		*file	[in] driver file discripter
  * @param 		*wait	[in/out] poll table
  * @return 	interrupt mask
  *
  * @author		Jangwon Lee
  * @note
  */
static unsigned int ipcs_k_poll(struct file *file, poll_table *wait)
{
	unsigned int mask = 0;

	if (ipcs_read_q_cnt > 0)
		mask |= (POLLIN | POLLRDNORM);
	else {
		poll_wait(file, &ipcs_wq_read, wait);
		/* mask = POLLERR; */
	}

	return mask;
}

const struct file_operations drime4_ipcs_fops = {
		.open = ipcs_k_open,
		.release = ipcs_k_release,
		.read = ipcs_k_read,
		.write = ipcs_k_write,
		.unlocked_ioctl = ipcs_k_ioctl,
		.mmap = ipcs_k_mmap,
		.poll = ipcs_k_poll
};


MODULE_AUTHOR("Dongjin Jung <djin81.jung@samsung.com>");
MODULE_DESCRIPTION("Samsung DRIMe4 IPCS driver using ioctl");
MODULE_LICENSE("GPL");

