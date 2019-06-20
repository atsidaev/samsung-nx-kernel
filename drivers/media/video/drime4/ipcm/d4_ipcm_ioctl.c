/**
 * @file	d4_ipcm_ioctl.c
 * @brief	K_IPCM driver file for Samsung DRIMe4 using ioctl
 *
 * @author	TaeWook Nam <tw.nam@samsung.com>
 *
 * Copyright (c) 2011 Samsung Electronics
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

#include <asm/atomic.h>
#include <asm/page.h>
#include <asm/cacheflush.h>
#include <mach/d4_mem.h>
#include <media/drime4/usih_type.h>

#include <media/drime4/ipcm/d4_ipcm_ioctl.h>
#include "d4_ipcm_if.h"
#include "d4_ipcm_intr.h"


#ifdef CONFIG_PMU_SELECT
#include <media/drime4/opener/d4_opener_ioctl.h>
#include <linux/d4_rmu.h>
#endif

#define USE_RMU_OTF_RESET_IPCM_CLK
#define IPCM_K_BUFFER_NUM 80

extern struct drime4_ipcm *g_ipcm;

#define MAX_IPCM_READ_QUEUE_CNT 3
static char ipcm_read_queue[MAX_IPCM_READ_QUEUE_CNT];
static unsigned long ipcm_read_q_cnt;
static unsigned long ipcm_read_q_head;
static DECLARE_COMPLETION(ipcm_intr_completion);

extern struct completion ipcm_wdma0_completion;
extern struct completion ipcm_wdma1_completion;
extern struct completion ipcm_wdma2_completion;

extern struct completion ipcm_md_gmv_completion;
extern struct completion ipcm_md_rmv_completion;

extern struct completion ipcm_ldcm_completion[LDCM_K_MAX_INTR];

static atomic_t ldcm_use_count = ATOMIC_INIT(0);
static DEFINE_MUTEX(ldcm_lock);

void _ipcm_send_interrupt(unsigned int intr_type)
{
	unsigned long flags;

	local_save_flags(flags);
	local_irq_disable();

	if (ipcm_read_q_cnt < MAX_IPCM_READ_QUEUE_CNT) {
		ipcm_read_queue[ipcm_read_q_head] = intr_type;
		ipcm_read_q_head =
			(ipcm_read_q_head + 1) % MAX_IPCM_READ_QUEUE_CNT;
		ipcm_read_q_cnt++;
	}

	local_irq_restore(flags);

	complete(&ipcm_intr_completion);
}

void _ipcm_wdma0_interrupt_set_callback_function(void)
{
	complete(&ipcm_wdma0_completion);
	_ipcm_send_interrupt(INTR_IPCM_B2Y_DONE);
	/*printk("IPCM WDMA0 Interrupt call back done\n");*/
}

void _ipcm_wdma1_interrupt_set_callback_function(void)
{
	complete(&ipcm_wdma1_completion);
	_ipcm_send_interrupt(INTR_IPCM_RSZ_DONE);
	/*printk("IPCM WDMA1 Interrupt call back done\n");*/
}

void _ipcm_wdma2_interrupt_set_callback_function(void)
{
	complete(&ipcm_wdma2_completion);
	_ipcm_send_interrupt(INTR_IPCM_SRSZ_DONE);
	/*printk("IPCM WDMA2 Interrupt call back done\n");*/
}

void _ipcm_wdma0_error_interrupt_set_callback_function(void)
{
	/*_ipcm_send_interrupt(INTR_IPCM_B2Y_DONE);*/
	printk("IPCM WDMA0 Error interrupt occured\n");
}

void _ipcm_wdma1_error_interrupt_set_callback_function(void)
{
	/*_ipcm_send_interrupt(INTR_IPCM_B2Y_DONE);*/
	printk("IPCM WDMA1 Error interrupt occured\n");
}

void _ipcm_wdma2_error_interrupt_set_callback_function(void)
{
	/*_ipcm_send_interrupt(INTR_IPCM_B2Y_DONE);*/
	printk("IPCM WDMA2 Error interrupt occured\n");
}

void _ipcm_md_gmv_interrupt_set_callback_function(void)
{
	complete(&ipcm_md_gmv_completion);
	/*printk("IPCM WDMA0 Interrupt call back done\n");*/
}

void _ipcm_md_rmv_interrupt_set_callback_function(void)
{
	complete(&ipcm_md_rmv_completion);
	/*printk("IPCM WDMA1 Interrupt call back done\n");*/
}

void _ipcm_ldcm_interrupt_set_callback_function(enum ldcm_k_intr_flags intr)
{
	/* printk("ldcm completion: %d\n", intr); */
	complete(&ipcm_ldcm_completion[intr]);
	/* TODO: _ipcm_send_interrupt(INTR_IPCM_MD_RMD_DONE); */
}

void _ipcm_wdma0_qview_interrupt_set_callback_function(void)
{
	ipcm_k_quickview_done();
	_ipcm_send_interrupt(INTR_IPCM_B2Y_DONE);
	/*printk("IPCM WDMA0 QView Interrupt call back done\n");*/
}

void _ipcm_wdma1_qview_interrupt_set_callback_function(void)
{
	/*printk("IPCM WDMA1 QView Interrupt call back done\n");*/
}

void _ipcm_wdma2_qview_interrupt_set_callback_function(void)
{
	/*printk("IPCM WDMA2 QView Interrupt call back done\n");*/
}

static int ipcm_k_open(struct inode *inode, struct file *filp)
{
#ifdef CONFIG_PMU_SELECT
	enum kdd_open_flag ret;
#endif

	/* int num = MINOR(inode->i_rdev);*/
	struct drime4_ipcm *ipcm_ctx = g_ipcm;
	filp->private_data = ipcm_ctx;


#ifdef CONFIG_PMU_SELECT
	ret = d4_kdd_open(KDD_IPCM);
	switch (ret) {
	case KDD_OPEN_FIRST:
		/* TODO: PMU OFF here...*/
		ipcm_k_pmu_on_off(K_IPCM_OFF);
		ipcm_read_q_cnt = 0;
		ipcm_read_q_head = 0;
		memset(ipcm_read_queue, 0, sizeof(char) * MAX_IPCM_READ_QUEUE_CNT);
		/*printk("IPCM KDD is opened! count: %d\n", d4_get_kdd_open_count(KDD_IPCM));*/
		break;
	case KDD_OPEN:
		/*printk("IPCM KDD is already opened! count: %d\n", d4_get_kdd_open_count(KDD_IPCM));*/
		break;
	case KDD_OPEN_EXCEED_MAXIMUM_OPEN:
		/*printk("IPCM KDD has exceeded the maximum number of open! count: %d\n", d4_get_kdd_open_count(KDD_IPCM));*/
		break;
	default:
		break;
	}
#endif

	return 0;
}

static int ipcm_k_release(struct inode *inode, struct file *filp)
{
#ifdef CONFIG_PMU_SELECT
	enum kdd_close_flag ret = d4_kdd_close(KDD_IPCM);

	switch (ret) {
	case KDD_CLOSE_FINAL:
		/* TODO: PMU ON here...*/
		ipcm_k_pmu_on_off(K_IPCM_ON);
		/*printk("IPCM KDD/fd is closed! count: %d\n", d4_get_kdd_open_count(KDD_IPCM));*/
		break;
	case KDD_CLOSE:
		/*printk("IPCM KDD is closed! count: %d\n", d4_get_kdd_open_count(KDD_IPCM));*/
		break;
	case KDD_CLOSE_ERROR:
	default:
		/*printk("IPCM KDD close is failed! count: %d\n", d4_get_kdd_open_count(KDD_IPCM));*/
		break;
	}
#endif
	return 0;
}

ssize_t ipcm_k_read(struct file *filp, char *buf, size_t count, loff_t *f_pos)
{
	unsigned long flags;
	int i = 0;

	do {
		wait_for_completion_interruptible(&ipcm_intr_completion);
		if (ipcm_read_q_cnt > 0)
			ipcm_read_q_cnt--;
	} while (ipcm_intr_completion.done != 0);

	local_save_flags(flags);
	local_irq_disable();

	for (i = 0; i < MAX_IPCM_READ_QUEUE_CNT; i++) {
		put_user(ipcm_read_queue[i], buf + (i * 4));
		ipcm_read_queue[i] = 0;
	}

	local_irq_restore(flags);

	ipcm_read_q_head = 0;

	return ipcm_read_q_cnt;
}

ssize_t ipcm_k_write(struct file *filp, const char *buf, size_t count,
		loff_t *f_pos)
{
	unsigned int i = 0;

	for (; i < count; i++) {
		/* get_user(dd_data[i],(int*)buf+i); */
	}
	return i;
}

/**
  * @brief		ipcm의 io control을 처리하는 함수
  * @fn			long ipcm_k_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
  * @param		*file	[in] driver file discripter
  * @param		*vma	[in/out] virtual memory map을 구성하기 위한 정보 structure
  * @return		On error returns a negative error, zero otherwise.
  *
  * @author		TaeWook Nam
  * @note
  */
long ipcm_k_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	int size;
	int err = -1;
	int ret = 0;

	struct d4_rmu_device *rmu;

	struct ipcm_k_physical_reg_info ipcm_kernel_reg_info;
	unsigned int clk_rate = 100000000;
	struct ipcm_ioctl_wdma enable = {0, };
	struct ipcm_md_ioctl_intr md_enable = {0, };
	struct ipcm_ldcm_ioctl_intr ldcm_enable = {0, };
	struct ipcm_ldcm_ioctl_intr_wait ldcm_wait = {0, };

#ifdef IPCM_TIMEOUT_CHECK
	unsigned int timeout = 500;
#endif

	if (_IOC_TYPE(cmd) != IPCM_MAGIC)
		return -1;

	size = _IOC_SIZE(cmd);

	if (_IOC_DIR(cmd) & _IOC_READ)
		err = access_ok(VERIFY_WRITE, (void *) arg, size);
	else if (_IOC_DIR(cmd) & _IOC_WRITE)
		err = access_ok(VERIFY_READ, (void *) arg, size);

	if (!err)
		return -1;

	switch (cmd) {

	case IPCM_IOCTL_PHYSICAL_REG_INFO:
#if 0
		ret = copy_from_user((void *)&ipcm_kernel_reg_info, (const void *)arg, size);
		if (ret < 0) {
			printk("ioctl fail: [%d]", cmd);
			return ret;
		}
#endif
		ipcm_k_get_physical_reg_info(&ipcm_kernel_reg_info);

		ret = copy_to_user((void *)arg, (const void *)&ipcm_kernel_reg_info, size);
		if (ret < 0) {
			printk("ioctl fail: [%d]", cmd);
			return ret;
		}
		break;

	case IPCM_IOCTL_CLOCK_FREQUENCY:
		ret = copy_from_user((void *)&clk_rate, (const void *)arg, size);
		if (ret < 0) {
			printk("ioctl fail: [%d]", cmd);
			return ret;
		}
		ipcm_k_clk_set_rate(clk_rate);
#if defined(USE_RMU_OTF_RESET_IPCM_CLK)
		rmu = d4_rmu_request();
		d4_sw_isp_reset(rmu, RST_OTF);
		d4_sw_isp_reset_release(rmu, RST_OTF);
		d4_rmu_release(rmu);
#endif
		break;

	case IPCM_IOCTL_WAIT_MAINOUT_DONE:
#ifndef IPCM_TIMEOUT_CHECK
		ipcm_k_wdma0_wait_done();
#else
		ret = copy_from_user((void *)&timeout, (const void *)arg, size);
		if (ret < 0) {
			printk("ioctl fail: [%d]", cmd);
			return ret;
		}

		err = ipcm_k_wdma0_wait_for_completion(msecs_to_jiffies(timeout));

		ret = copy_to_user((void *)arg, (const void *)&err, size);
		if (ret < 0) {
			printk("ioctl fail: [%d]", cmd);
			return ret;
		}
#endif
		break;
	case IPCM_IOCTL_WAIT_MRSZ_DONE:
#ifndef IPCM_TIMEOUT_CHECK
		ipcm_k_wdma1_wait_done();
#else
		ret = copy_from_user((void *)&timeout, (const void *)arg, size);
		if (ret < 0) {
			printk("ioctl fail: [%d]", cmd);
			return ret;
		}

		err = ipcm_k_wdma1_wait_for_completion(msecs_to_jiffies(timeout));

		ret = copy_to_user((void *)arg, (const void *)&err, size);
		if (ret < 0) {
			printk("ioctl fail: [%d]", cmd);
			return ret;
		}
#endif
		break;
	case IPCM_IOCTL_WAIT_SRSZ_DONE:
#ifndef IPCM_TIMEOUT_CHECK
		ipcm_k_wdma2_wait_done();
#else
		ret = copy_from_user((void *)&timeout, (const void *)arg, size);
		if (ret < 0) {
			printk("ioctl fail: [%d]", cmd);
			return ret;
		}

		err = ipcm_k_wdma2_wait_for_completion(msecs_to_jiffies(timeout));

		ret = copy_to_user((void *)arg, (const void *)&err, size);
		if (ret < 0) {
			printk("ioctl fail: [%d]", cmd);
			return ret;
		}
#endif
		break;
	case IPCM_IOCTL_MD_WAIT_GMV_DONE:
#ifndef IPCM_MD_TIMEOUT_CHECK
		ipcm_k_md_gmv_wait_done();
#else
		ret = copy_from_user((void *)&timeout, (const void *)arg, size);
		if (ret < 0) {
			printk("ioctl fail: [%d]", cmd);
			return ret;
		}

		err = ipcm_k_md_gmv_wait_for_completion(msecs_to_jiffies(timeout));

		ret = copy_to_user((void *)arg, (const void *)&err, size);
		if (ret < 0) {
			printk("ioctl fail: [%d]", cmd);
			return ret;
		}
#endif
		break;
	case IPCM_IOCTL_MD_WAIT_RMV_DONE:
#ifndef IPCM_MD_TIMEOUT_CHECK
		ipcm_k_md_rmv_wait_done();
#else
		ret = copy_from_user((void *)&timeout, (const void *)arg, size);
		if (ret < 0) {
			printk("ioctl fail: [%d]", cmd);
			return ret;
		}

		err = ipcm_k_md_rmv_wait_for_completion(msecs_to_jiffies(timeout));

		ret = copy_to_user((void *)arg, (const void *)&err, size);
		if (ret < 0) {
			printk("ioctl fail: [%d]", cmd);
			return ret;
		}
#endif
		break;
	case IPCM_IOCTL_WAIT_LDCM_DONE:
		ret = copy_from_user((void *)&ldcm_wait, (const void *)arg, size);
		if (ret < 0) {
			printk("ioctl fail: [%d]", cmd);
			return ret;
		}

		ret = ipcm_k_ldcm_wait_for_completion(ldcm_wait.intr, msecs_to_jiffies(ldcm_wait.timeout));
		if (ret < 0) {
			printk("ioctl fail: [%d]", cmd);
			return ret;
		}

		break;
	case IPCM_IOCTL_OPEN_IRQ:
		err = ipcm_k_dma_int_init();
		ret = copy_to_user((void *)arg, (const void *)&err, size);
		if (ret < 0) {
			printk("ioctl fail: [%d]", cmd);
			return ret;
		}
		break;
	case IPCM_IOCTL_CLOSE_IRQ:
		ipcm_k_dma_int_off();
		break;

	case IPCM_IOCTL_INTERRUPT_ENABLE:
		ret = copy_from_user((void *)&enable, (const void *)arg, size);
		if (ret < 0) {
			printk("ioctl fail: [%d]", cmd);
			return ret;
		}
		if (enable.main_out == K_IPCM_ON)
			ipcm_k_dma_int_enable(IPCM_K_INTR_BUSEND_WDMA0,
							_ipcm_wdma0_interrupt_set_callback_function);
		if (enable.main_rsz == K_IPCM_ON) {
			if (enable.main_rsz_nlc_flag == K_IPCM_ON) {
				ipcm_k_dma_int_enable(IPCM_K_INTR_NLCE_DONE,
							_ipcm_wdma1_interrupt_set_callback_function);
			} else {
				ipcm_k_dma_int_enable(IPCM_K_INTR_BUSEND_WDMA1,
							_ipcm_wdma1_interrupt_set_callback_function);
			}
		}
		if (enable.sub_rsz == K_IPCM_ON)
			ipcm_k_dma_int_enable(IPCM_K_INTR_BUSEND_WDMA2,
							_ipcm_wdma2_interrupt_set_callback_function);
		break;

	case IPCM_IOCTL_INTERRUPT_DISABLE:
		ret = copy_from_user((void *)&enable, (const void *)arg, size);
		if (ret < 0) {
			printk("ioctl fail: [%d]", cmd);
			return ret;
		}
		if (enable.main_out == K_IPCM_ON)
			ipcm_k_dma_int_disable(IPCM_K_INTR_BUSEND_WDMA0);
		if (enable.main_rsz == K_IPCM_ON) {
			if (enable.main_rsz_nlc_flag == K_IPCM_ON) {
				ipcm_k_dma_int_disable(IPCM_K_INTR_NLCE_DONE);
			} else {
				ipcm_k_dma_int_disable(IPCM_K_INTR_BUSEND_WDMA1);
			}
		}
		if (enable.sub_rsz == K_IPCM_ON)
			ipcm_k_dma_int_disable(IPCM_K_INTR_BUSEND_WDMA2);
		break;

	case IPCM_IOCTL_ERROR_INTERRUPT_ENABLE:
		ret = copy_from_user((void *)&enable, (const void *)arg, size);
		if (ret < 0) {
			printk("ioctl fail: [%d]", cmd);
			return ret;
		}
		if (enable.main_out == K_IPCM_ON)
			ipcm_k_dma_int_enable(IPCM_K_INTR_ERROR_WDMA0,
							_ipcm_wdma0_error_interrupt_set_callback_function);
		if (enable.main_rsz == K_IPCM_ON)
			ipcm_k_dma_int_enable(IPCM_K_INTR_ERROR_WDMA1,
							_ipcm_wdma1_error_interrupt_set_callback_function);
		if (enable.sub_rsz == K_IPCM_ON)
			ipcm_k_dma_int_enable(IPCM_K_INTR_ERROR_WDMA2,
							_ipcm_wdma2_error_interrupt_set_callback_function);
		break;

	case IPCM_IOCTL_ERROR_INTERRUPT_DISABLE:
		ret = copy_from_user((void *)&enable, (const void *)arg, size);
		if (ret < 0) {
			printk("ioctl fail: [%d]", cmd);
			return ret;
		}
		if (enable.main_out == K_IPCM_ON)
			ipcm_k_dma_int_disable(IPCM_K_INTR_ERROR_WDMA0);
		if (enable.main_rsz == K_IPCM_ON)
			ipcm_k_dma_int_disable(IPCM_K_INTR_ERROR_WDMA1);
		if (enable.sub_rsz == K_IPCM_ON)
			ipcm_k_dma_int_disable(IPCM_K_INTR_ERROR_WDMA2);
		break;

	case IPCM_IOCTL_MD_OPEN_IRQ:
		ipcm_k_md_int_init();
		break;
	case IPCM_IOCTL_MD_CLOSE_IRQ:
		ipcm_k_md_int_off();
		break;

	case IPCM_IOCTL_MD_INTERRUPT_ENABLE:
		ret = copy_from_user((void *)&md_enable, (const void *)arg, size);
		if (ret < 0) {
			printk("ioctl fail: [%d]", cmd);
			return ret;
		}
		if (md_enable.gmv_end == K_IPCM_ON)
			ipcm_k_md_int_enable(MD_K_INTR_GMV_END, _ipcm_md_gmv_interrupt_set_callback_function);
		if (md_enable.rmv_end == K_IPCM_ON)
			ipcm_k_md_int_enable(MD_K_INTR_RMV_END, _ipcm_md_rmv_interrupt_set_callback_function);

		break;
	case IPCM_IOCTL_MD_INTERRUPT_DISABLE:
		ret = copy_from_user((void *)&md_enable, (const void *)arg, size);
		if (ret < 0) {
			printk("ioctl fail: [%d]", cmd);
			return ret;
		}
		if (md_enable.gmv_end == K_IPCM_ON)
			ipcm_k_md_int_disable(MD_K_INTR_GMV_END);
		if (md_enable.rmv_end == K_IPCM_ON)
			ipcm_k_md_int_disable(MD_K_INTR_RMV_END);

		break;

	case IPCM_IOCTL_LDCM_LOCK:
		mutex_lock(&ldcm_lock);
		if (atomic_read(&ldcm_use_count) == 0) {
			atomic_set(&ldcm_use_count, 1);
			mutex_unlock(&ldcm_lock);
		} else {
			mutex_unlock(&ldcm_lock);
			return -1;
		}
		break;

	case IPCM_IOCTL_LDCM_UNLOCK:
		mutex_lock(&ldcm_lock);
		atomic_set(&ldcm_use_count, 0);
		mutex_unlock(&ldcm_lock);
		break;

	case IPCM_IOCTL_LDCM_OPEN_IRQ:
		ipcm_k_ldcm_int_init();
		break;
	case IPCM_IOCTL_LDCM_CLOSE_IRQ:
		ipcm_k_ldcm_int_off();
		break;

	case IPCM_IOCTL_LDCM_INTERRUPT_ENABLE:
		ret = copy_from_user((void *)&ldcm_enable, (const void *)arg, size);
		if (ret < 0) {
			printk("ioctl fail: [%d]", cmd);
			return ret;
		}
		ipcm_k_ldcm_int_enable(ldcm_enable.intr, _ipcm_ldcm_interrupt_set_callback_function);
		break;
	case IPCM_IOCTL_LDCM_INTERRUPT_DISABLE:
		ret = copy_from_user((void *)&ldcm_enable, (const void *)arg, size);
		if (ret < 0) {
			printk("ioctl fail: [%d]", cmd);
			return ret;
		}
		ipcm_k_ldcm_int_disable(ldcm_enable.intr);
		break;

	case IPCM_IOCTL_INTERRUPT_ENABLE_FOR_QVIEW:
		ret = copy_from_user((void *)&enable, (const void *)arg, size);
		if (ret < 0) {
			printk("ioctl fail: [%d]", cmd);
			return ret;
		}
		if (enable.main_out == K_IPCM_ON)
			ipcm_k_dma_int_enable(IPCM_K_INTR_BUSEND_WDMA0,
							_ipcm_wdma0_qview_interrupt_set_callback_function);
		if (enable.main_rsz == K_IPCM_ON) {
			if (enable.main_rsz_nlc_flag == K_IPCM_ON) {
				ipcm_k_dma_int_enable(IPCM_K_INTR_NLCE_DONE,
							_ipcm_wdma1_qview_interrupt_set_callback_function);
			} else {
				ipcm_k_dma_int_enable(IPCM_K_INTR_BUSEND_WDMA1,
							_ipcm_wdma1_qview_interrupt_set_callback_function);
			}
		}
		if (enable.sub_rsz == K_IPCM_ON)
			ipcm_k_dma_int_enable(IPCM_K_INTR_BUSEND_WDMA2,
							_ipcm_wdma2_qview_interrupt_set_callback_function);
		break;

	case IPCM_IOCTL_INTERRUPT_DISABLE_FOR_QVIEW:
		ret = copy_from_user((void *)&enable, (const void *)arg, size);
		if (ret < 0) {
			printk("ioctl fail: [%d]", cmd);
			return ret;
		}
		if (enable.main_out == K_IPCM_ON)
			ipcm_k_dma_int_disable(IPCM_K_INTR_BUSEND_WDMA0);
		if (enable.main_rsz == K_IPCM_ON) {
			if (enable.main_rsz_nlc_flag == K_IPCM_ON) {
				ipcm_k_dma_int_disable(IPCM_K_INTR_NLCE_DONE);
			} else {
				ipcm_k_dma_int_disable(IPCM_K_INTR_BUSEND_WDMA1);
			}
		}
		if (enable.sub_rsz == K_IPCM_ON)
			ipcm_k_dma_int_disable(IPCM_K_INTR_BUSEND_WDMA2);
		break;

	default:
		break;
	}

	return 0;
}

/**
  * @brief	application 접근용 virtual address를 생성해주는 함수
  * @fn		int ipcm_k_mmap(struct file *file, struct vm_area_struct *vma)
  * @param	*file	[in] driver file discripter
  * @param	*vma	[in/out] virtual memory map을 구성하기 위한 정보 structure
  * @return	On error returns a negative error, zero otherwise.
  *
  * @author	TaeWook Nam
  * @note
  */
int ipcm_k_mmap(struct file *file, struct vm_area_struct *vma)
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
  * @brief		application could detect ipcm interrupts
  * @fn			int ipcm_k_poll(struct file *file, poll_table *wait)
  * @param 		*file	[in] driver file discripter
  * @param 		*wait	[in/out] poll table
  * @return 	interrupt mask
  *
  * @author		Jangwon Lee
  * @note
  */
static unsigned int ipcm_k_poll(struct file *file, poll_table *wait)
{
	unsigned int mask = 0;

	return mask;
}

const struct file_operations drime4_ipcm_fops = {
		.open = ipcm_k_open,
		.release = ipcm_k_release,
		.read = ipcm_k_read,
		.write = ipcm_k_write,
		.unlocked_ioctl = ipcm_k_ioctl,
		.mmap = ipcm_k_mmap,
		.poll = ipcm_k_poll
};


MODULE_AUTHOR("TaeWook Nam <tw.nam@samsung.com>");
MODULE_DESCRIPTION("Samsung DRIMe4 K_IPCM driver using ioctl");
MODULE_LICENSE("GPL");

