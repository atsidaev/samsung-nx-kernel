/**
 * @file d4_pp_core_ioctl.c
 * @brief DRIMe4 PP Core Ioctl Control Function File
 * @author Main : Sunghoon Kim <bluesay.kim@samsung.com>
 *         MIPI : Gunwoo Nam <gunwoo.nam@samsung.com>
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
#include <linux/poll.h>
#include <linux/sched.h>

#include <media/drime4/usih_type.h>
#include <media/drime4/pp/pp_core/d4_pp_core_ioctl.h>
#include "d4_pp_core_if.h"

#ifdef CONFIG_PMU_SELECT
#include <media/drime4/opener/d4_opener_ioctl.h>
#include <linux/d4_rmu.h>
#endif

#define USE_RMU_OTF_RESET_PP_CLK
extern struct drime4_pp_core *g_pp_core;

/**< For Test */
inline void write_pp_core_ctrl_register(unsigned int offset, unsigned int val);

void pp_core_wdma_callback_func(void);
void pp_core_rdma_callback_func(void);
void mipi_rx_wdma_callback_func(void);
void mipi_tx_rdma_callback_func(void);

#ifdef PP_CORE_POLL_ENABLE
static DECLARE_WAIT_QUEUE_HEAD(PP_WaitQueue_Read);
#define MAX_PP_QUEUE_CNT 512

static char PP_ReadQ[MAX_PP_QUEUE_CNT];
static unsigned long PP_ReadQCount;
static unsigned long PP_ReadQHead;
static unsigned long PP_ReadQTail;

int pp_send_interrupt(unsigned int intr_type)
{
	unsigned long flags;

	local_save_flags(flags);
	local_irq_disable();

	if (PP_ReadQCount < MAX_PP_QUEUE_CNT) {
		PP_ReadQ[PP_ReadQHead] = intr_type;
		PP_ReadQHead = (PP_ReadQHead + 1) % MAX_PP_QUEUE_CNT;
		PP_ReadQCount++;
	}

	local_irq_restore(flags);

	wake_up_interruptible(&PP_WaitQueue_Read);

	return 0;
}
#endif

static int d4_pp_core_open(struct inode *inode, struct file *filp)
{
#ifdef CONFIG_PMU_SELECT
	enum kdd_open_flag ret;
#endif

	/* int num = MINOR(inode->i_rdev);*/
	struct drime4_pp_core *pp_core_ctx = g_pp_core;
	filp->private_data = pp_core_ctx;

#ifdef CONFIG_PMU_SELECT
	ret = d4_kdd_open(KDD_PP);
	switch (ret) {
	case KDD_OPEN_FIRST:
		pp_com_pmu_on_off(PP_DD_OFF);
		pr_debug("PP KDD is opened! count: %d\n", d4_get_kdd_open_count(KDD_PP));
		break;
	case KDD_OPEN:
		pr_debug("PP KDD is already opened! count: %d\n", d4_get_kdd_open_count(KDD_PP));
		break;
	case KDD_OPEN_EXCEED_MAXIMUM_OPEN:
		pr_err("PP KDD has exceeded the maximum number of open! count: %d\n", d4_get_kdd_open_count(KDD_PP));
		break;
	default:
		break;
	}
#endif
	return 0;
}

static int d4_pp_core_release(struct inode *inode, struct file *filp)
{
#ifdef CONFIG_PMU_SELECT
	enum kdd_close_flag ret = d4_kdd_close(KDD_PP);

	switch (ret) {
	case KDD_CLOSE_FINAL:
		pp_com_pmu_on_off(PP_DD_ON);
		pr_debug("PP KDD/fd is closed! count: %d\n", d4_get_kdd_open_count(KDD_PP));
		break;
	case KDD_CLOSE:
		pr_debug("PP KDD is closed! count: %d\n", d4_get_kdd_open_count(KDD_PP));
		break;
	case KDD_CLOSE_ERROR:
	default:
		pr_err(KERN_ERR "PP KDD close is failed! count: %d\n", d4_get_kdd_open_count(KDD_PP));
		break;
	}
#endif
	return 0;
}

ssize_t pp_core_read(struct file *filp, char *buf, size_t count, loff_t *f_pos)
{
	unsigned int i = 0;
	return i;
}

ssize_t pp_core_write(struct file *filp, const char *buf, size_t count,	loff_t *f_pos)
{
	unsigned int i = 0;
	return i;
}

long pp_core_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	int size;
	int err = -1;
	long ret = -1;
	unsigned int pp_clock_set = 0;
	unsigned int pp_phys_start_addr = 0;
	unsigned int set_timeout = 0;
    struct d4_rmu_device *rmu;

	enum pp_dd_onoff pp_core_onoff;

	if (_IOC_TYPE(cmd) != PP_CORE_MAGIC)
		return -1;

	size = _IOC_SIZE(cmd);

	if (_IOC_DIR(cmd) & _IOC_READ)
		err = access_ok(VERIFY_WRITE, (void *) arg, size);
	else if (_IOC_DIR(cmd) & _IOC_WRITE)
		err = access_ok(VERIFY_READ, (void *) arg, size);

	if (!err)
		return -1;

	switch (cmd) {
	case PP_CORE_IOCTL_GET_PHYS_REG_INFO:
		pp_core_get_phys_reg_info(&pp_phys_start_addr);

		ret = copy_to_user((void *)arg, (const void *)&pp_phys_start_addr, size);
		if (ret < 0) {
			pr_err("ioctl fail: [%d]\n", cmd);
			return ret;
		}
		break;
	case PP_CORE_IOCTL_OPEN_IRQ:
		pp_core_com_request_irq();
		break;
	case PP_CORE_IOCTL_CLOSE_IRQ:
		pp_core_com_free_irq();
		break;
	case PP_CORE_IOCTL_WAIT_LUT_LOAD_DONE:
		pp_core_com_wait_lut_load_done();
		break;
	case PP_CORE_IOCTL_WAIT_LUT_LOAD_DONE_WITH_TIMEOUT:
		ret = copy_from_user((void *)&set_timeout, (const void *)arg, size);
		if (ret < 0) {
			pr_err("ioctl fail: [%d]\n", cmd);
			return ret;
		}

		if (pp_core_com_wait_lut_load_done_with_timeout(set_timeout) < 0)
			return -1;

		break;
	case PP_CORE_IOCTL_WAIT_LUT_GENERATION_DONE:
		pp_core_com_wait_lut_generation_done();
		break;
	case PP_CORE_IOCTL_WAIT_VFPN_DONE:
		pp_core_com_wait_vfpn_done();
		break;
	case PP_CORE_IOCTL_WAIT_INIT_WDMA:
		pp_core_com_wait_init_wdma();
		break;
	case PP_CORE_IOCTL_WAIT_INIT_RDMA:
		pp_core_com_wait_init_rdma();
		break;
	case PP_CORE_IOCTL_WAIT_INIT_LUT_LOAD:
		pp_core_com_wait_init_lut_load();
		break;
	case PP_CORE_IOCTL_WAIT_WDMA_DONE:
		pp_core_com_wait_wdma_done();
		break;
	case PP_CORE_IOCTL_WAIT_RDMA_DONE:
		pp_core_com_wait_rdma_done();
		break;
	case PP_CORE_IOCTL_CORE_RESET:
		pp_core_com_core_reset();
		break;
	case MIPI_IOCTL_WAIT_WDMA_DONE:
		mipi_com_wait_wdma_done();
		break;
	case MIPI_IOCTL_WAIT_RDMA_DONE:
		mipi_com_wait_rdma_done();
		break;
	case MIPI_IOCTL_WAIT_WDMA_CALLBACK:
		mipi_wdma_set_callback_func(mipi_rx_wdma_callback_func);
		break;
	case MIPI_IOCTL_WAIT_WDMA_CALLBACK_CLEAR:
		mipi_wdma_set_callback_func(NULL);
		break;
	case MIPI_IOCTL_WAIT_RDMA_CALLBACK:
		mipi_rdma_set_callback_func(mipi_tx_rdma_callback_func);
		break;
	case PP_CORE_IOCTL_SET_PP_CLOCK:
		ret = copy_from_user((void *)&pp_clock_set, (const void *)arg, size);
		if (ret < 0) {
			pr_err("ioctl fail: [%d]\n", cmd);
			return ret;
		}
		pp_core_com_set_pp_clock(pp_clock_set);
#if defined(USE_RMU_OTF_RESET_PP_CLK)
        rmu = d4_rmu_request();
					if (rmu == NULL)
						return -1;
        d4_sw_isp_reset(rmu, RST_OTF);
        d4_sw_isp_reset_release(rmu, RST_OTF);
        d4_rmu_release(rmu);
#endif
		break;
	case PP_CORE_IOCTL_PP_PMU_ONOFF:
		ret = copy_from_user((void *)&pp_core_onoff, (const void *)arg, size);
		if (ret < 0) {
			pr_err("ioctl fail: [%d]\n", cmd);
			return ret;
		}
		pp_com_pmu_on_off(pp_core_onoff);
		break;
	default:
		break;
	}
	return 0;
}

void pp_core_wdma_callback_func(void)
{
#ifdef PP_CORE_POLL_ENABLE
	pp_send_interrupt(INTR_PP_WDMA_WRITE_DONE);
#else
	complete(&pp_core_wdma_completion);
	pp_wdma_set_callback_func(NULL);
#endif
}

void pp_core_rdma_callback_func(void)
{
#ifdef PP_CORE_POLL_ENABLE
	pp_send_interrupt(INTR_PP_RDMA_READ_DONE);
#else
	complete(&pp_core_rdma_completion);
	pp_rdma_set_callback_func(NULL);
#endif
}

void mipi_rx_wdma_callback_func(void)
{
#ifdef PP_MIPI_POLL_ENABLE
	pp_send_interrupt(INTR_PP_MIPI_RX_DONE);
#else
	complete(&mipi_rx_wdma_completion);
#endif
}

void mipi_tx_rdma_callback_func(void)
{
	complete(&mipi_tx_rdma_completion);
	mipi_rdma_set_callback_func(NULL);
}


int pp_core_mmap(struct file *file, struct vm_area_struct *vma)
{
	unsigned long size = 0;

	size     = (vma->vm_end - vma->vm_start);

	vma->vm_flags |= VM_IO;
	vma->vm_flags |= VM_RESERVED;

	vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);

	if (io_remap_pfn_range(vma, vma->vm_start, vma->vm_pgoff, size,
			vma->vm_page_prot)) {
		return -EAGAIN;
	}
	return 0;
}

#ifdef PP_CORE_POLL_ENABLE
static u32 pp_core_poll_wait(struct file *filp, poll_table *wait)

{
	unsigned int mask = 0;

	poll_wait(filp, &PP_WaitQueue_Read, wait);

	if (PP_ReadQCount > 0) {
		mask |= POLLIN | POLLRDNORM;
	}

	return mask;
}

ssize_t pp_core_poll_read(struct file *filp, char __user *num, size_t count, loff_t *f_pos)
{
	unsigned long flags;
	int retstate;

	if ((!PP_ReadQCount) && (filp->f_flags & O_NONBLOCK))
		return -EAGAIN;

	retstate = wait_event_interruptible(PP_WaitQueue_Read, PP_ReadQCount);

	if (retstate)
		return retstate;

	local_save_flags(flags);
	local_irq_disable();

	if (PP_ReadQCount > 0) {
		put_user(PP_ReadQ[PP_ReadQTail], num);
		PP_ReadQTail = (PP_ReadQTail + 1) % MAX_PP_QUEUE_CNT;
		PP_ReadQCount--;
	}

	local_irq_restore(flags);

	return PP_ReadQCount;
}

const struct file_operations pp_core_ops = {
		.open = d4_pp_core_open,
		.release = d4_pp_core_release,
		.write = pp_core_write,
		.mmap = pp_core_mmap,
		.unlocked_ioctl = pp_core_ioctl,
		.poll = pp_core_poll_wait,
		.read = pp_core_poll_read,
};
#else
const struct file_operations pp_core_ops = {
		.open = d4_pp_core_open,
		.release = d4_pp_core_release,
		.read = pp_core_read,
		.write = pp_core_write,
		.mmap = pp_core_mmap,
		.unlocked_ioctl = pp_core_ioctl,
};
#endif

MODULE_AUTHOR("SungHoon Kim <bluesay.kim@samsung.com>");
MODULE_DESCRIPTION("Samsung DRIMe4 PP Core driver using ioctl");
MODULE_LICENSE("GPL");

