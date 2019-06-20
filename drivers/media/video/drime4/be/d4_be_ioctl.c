/**
 * @file d4_be_ioctl.c
 * @brief DRIMe4 BE IOCTL Interface
 * @author Niladri Mukherjee <n.mukherjee@samsung.com>
 * 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

/*=============================
 * Bayer SYSTEM INCLUCES
 =============================*/
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/io.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/wait.h>
#include <linux/slab.h>
#include <linux/mm.h>
//#include <linux/cma.h>
#include <mach/d4_mem.h>
#include <linux/uaccess.h>
#include <asm/cacheflush.h>
#include <mach/d4_cma.h>
/*=============================
 * Bayer USER INCLUCES
 =============================*/
#include <media/drime4/be/d4_be_ioctl.h>
#include "d4_be_if.h"

#ifdef CONFIG_PMU_SELECT
#include <media/drime4/opener/d4_opener_ioctl.h>
#endif

/*=============================
 * Bayer Global Variables
 =============================*/
extern struct drime4_be *g_be;




void d4_be_hdrlls_deregister_dmaint(unsigned int fun_blk)
{
	switch(fun_blk){
	case BLD_DMA_INTR:
	{
		be_dma_deregister_isr_clbk(BE_DMA_INT_LEVEL_WDMA2_INTBUSEND);
	}
	break;

	case R2S_DMA_INTR:
	{
		be_dma_deregister_isr_clbk(BE_DMA_INT_LEVEL_WDMA0_INTBUSEND);
	}
	break;
	case LMC_DMA_INTR:
	{
		be_dma_deregister_isr_clbk(BE_DMA_INT_LEVEL_WDMA0_INTBUSEND);
	}
	break;
	default:
		printk("DE-REGISTER DMA INT::ERROR COMMAND BLOCK\n");
		break;
	}
}




void d4_be_hdrlls_deregister_int(unsigned int fun_blk)
{
	switch(fun_blk){
	case BE_LLSHDR_NR_WT_ALL:
	{
		be_top_isr_deregister_clbk(BE_INT_LEVEL_SNR_FWT);
		be_top_isr_deregister_clbk(BE_INT_LEVEL_SNR_IWT);
		be_top_isr_deregister_clbk(BE_INT_LEVEL_SNR_STRIP);
	}
	break;

	case BE_INT_LEVEL_SNR_SNR:
	case BE_INT_LEVEL_SNR_FWT:
	case BE_INT_LEVEL_SNR_IWT:
	case BE_INT_LEVEL_SNR_STRIP:
	case BE_INT_LEVEL_SG_PRF:
	case BE_INT_LEVEL_SG_R2S:
	case BE_INT_LEVEL_SG_GME:
	case BE_INT_LEVEL_SG_LME:
	case BE_INT_LEVEL_SG_LMC:
	case BE_INT_LEVEL_GD_GD_DIFF_GF:
	case BE_INT_LEVEL_GD_FW_PRE_LVR:
	case BE_INT_LEVEL_GD_FW_WEIGHT:
	case BE_INT_LEVEL_GD_DB_BLEND:
	case BE_INT_LEVEL_FC_DT_PRE_GF3:
	case BE_INT_LEVEL_FC_DT_GF3:
	case BE_INT_LEVEL_FC_OS_LOW_GF:
	case BE_INT_LEVEL_FC_OS_COUNT:
	case BE_INT_LEVEL_FC_CC_FCHK2:
	case BE_INT_LEVEL_BLD_LUT:
	case BE_INT_LEVEL_BLD_MAX:
	case BE_INT_LEVEL_BLD_BLD:
	case BE_INT_LEVEL_DMA:
	{
		be_top_isr_deregister_clbk(fun_blk);
	}
	break;

	default:
		printk("DE-REGISTER INTR::ERROR COMMAND BLOCK\n");
		break;
	}
}



void d4_be_hdrlls_register_int(unsigned int fun_blk)
{
	switch(fun_blk){
	case BE_INT_LEVEL_SNR_SNR:
	{
		be_snr_register_kisr_clbk();
	}
	break;

	case BE_INT_LEVEL_SNR_FWT:
	{
		be_nr_register_fwt();
	}
	break;

	case BE_INT_LEVEL_SNR_IWT:
	{
		be_nr_register_iwt();
	}
	break;

	case BE_INT_LEVEL_SNR_STRIP:
	{
		be_nr_register_strip();
	}
	break;

	case BE_LLSHDR_NR_WT_ALL:
	{
		be_nr_block_fiwlet();
	}
	break;

	case BE_INT_LEVEL_SG_PRF:
	case BE_INT_LEVEL_SG_R2S:
	case BE_INT_LEVEL_SG_GME:
	case BE_INT_LEVEL_SG_LME:
	case BE_INT_LEVEL_SG_LMC:
	{
		be_ksg_register_isr_clbk(fun_blk);
	}
	break;

	case BE_INT_LEVEL_BLD_LUT:
	case BE_INT_LEVEL_BLD_MAX:
	case BE_INT_LEVEL_BLD_BLD:
	{
		be_kblend_register_isr_clbk(fun_blk);
	}
	break;

	case BE_INT_LEVEL_GD_GD_DIFF_GF:
	case BE_INT_LEVEL_GD_FW_PRE_LVR:
	case BE_INT_LEVEL_GD_FW_WEIGHT:
	case BE_INT_LEVEL_GD_DB_BLEND:
	case BE_INT_LEVEL_FC_DT_PRE_GF3:
	case BE_INT_LEVEL_FC_DT_GF3:
	case BE_INT_LEVEL_FC_OS_LOW_GF:
	case BE_INT_LEVEL_FC_OS_COUNT:
	case BE_INT_LEVEL_FC_CC_FCHK2:
	{
		be_kghost_fchk_register_isr_clbk(fun_blk);
	}
	break;


	default:
		printk("REGISTER INTR::ERROR COMMAND BLOCK\n");
		break;
	}

}

int d4_be_hdrlls_wait_for_event(struct be_hdrlls_completion_event comp_event)
{
	int result = 0;

	switch(comp_event.hdrlls_ctrl){

	case BE_INT_LEVEL_SNR_SNR:
	case BE_INT_LEVEL_SNR_FWT:
	case BE_INT_LEVEL_SNR_IWT:
	case BE_INT_LEVEL_SNR_STRIP:
	{
		result = be_nr_wait_for_kcompletion(comp_event.hdrlls_ctrl, comp_event.timeout);
	}
	break;

	case BE_INT_LEVEL_SG_PRF:
	case BE_INT_LEVEL_SG_R2S:
	case BE_INT_LEVEL_SG_GME:
	case BE_INT_LEVEL_SG_LME:
	case BE_INT_LEVEL_SG_LMC:
	{
		result = be_sg_wait_for_kcompletion(comp_event.hdrlls_ctrl, comp_event.timeout);

	}
	break;

	case BE_INT_LEVEL_BLD_LUT:
	case BE_INT_LEVEL_BLD_MAX:
	case BE_INT_LEVEL_BLD_BLD:
	{
		result = be_kblend_wait_for_completion(comp_event.hdrlls_ctrl, comp_event.timeout);
	}
	break;

	case BE_INT_LEVEL_GD_GD_DIFF_GF:
	case BE_INT_LEVEL_GD_FW_PRE_LVR:
	case BE_INT_LEVEL_GD_FW_WEIGHT:
	case BE_INT_LEVEL_GD_DB_BLEND:
	case BE_INT_LEVEL_FC_DT_PRE_GF3:
	case BE_INT_LEVEL_FC_DT_GF3:
	case BE_INT_LEVEL_FC_OS_LOW_GF:
	case BE_INT_LEVEL_FC_OS_COUNT:
	case BE_INT_LEVEL_FC_CC_FCHK2:
	{
		result = be_kghost_kfchk_wait_for_result(comp_event.hdrlls_ctrl, comp_event.timeout);
		}
	break;


	default:
		printk("D4_BE_HDRLLS_WAIT_FOR_EVENT::ERROR COMMAND BLOCK\n");
		break;
	}

	return result;

}

static int d4_be_open(struct inode *inode, struct file *filp)
{
#ifdef CONFIG_PMU_SELECT
	enum kdd_open_flag ret;
#endif

	struct drime4_be *be = g_be;
	filp->private_data = be;
	BE_IOCTL_DEBUG("bayer ioctl open\n");

#ifdef CONFIG_PMU_SELECT
	ret = d4_kdd_open(KDD_BE);
	switch (ret) {
	case KDD_OPEN_FIRST:
		be_pmu_on_off(BE_PMU_OFF);
		BE_IOCTL_DEBUG("BE KDD is opened! count: %d\n", d4_get_kdd_open_count(KDD_BE));
		break;
	case KDD_OPEN:
		BE_IOCTL_DEBUG("BE KDD is already opened! count: %d\n", d4_get_kdd_open_count(KDD_BE));
		break;
	case KDD_OPEN_EXCEED_MAXIMUM_OPEN:
		BE_IOCTL_DEBUG("BE KDD has exceeded the maximum number of open! count: %d\n", d4_get_kdd_open_count(KDD_BE));
		break;
	default:
		break;
	}
#endif
	return 0;
}

static int d4_be_release(struct inode *inode, struct file *filp)
{
#ifdef CONFIG_PMU_SELECT
	enum kdd_close_flag ret = d4_kdd_close(KDD_BE);

	switch (ret) {
	case KDD_CLOSE_FINAL:
		be_pmu_on_off(BE_PMU_ON);
		BE_IOCTL_DEBUG("BE KDD/fd is closed! count: %d\n", d4_get_kdd_open_count(KDD_BE));
		break;
	case KDD_CLOSE:
		BE_IOCTL_DEBUG("BE KDD is closed! count: %d\n", d4_get_kdd_open_count(KDD_BE));
		break;
	case KDD_CLOSE_ERROR:
	default:
		BE_IOCTL_DEBUG("BE KDD close is failed! count: %d\n", d4_get_kdd_open_count(KDD_BE));
		break;
	}
#endif
	return 0;
}

static long d4_be_ioctl(struct file *filp, unsigned int cmd,
		unsigned long arg)
{
	int ret = 0;
	int size = 0;
	int err = -1;
	struct be_ioctl_get_reg_info be_reg_info = {0,};
	unsigned int be_clock_set = 0;
	unsigned int usrptr = 0;
	struct be_hdrlls_completion_event comp_event = {0,};
	struct be_nr_iwt_info iwt_info = {0,};
	enum be_interrupt_level be_core_intr = {0,};
	enum be_dma_interrupt_level be_dma_intr = {0,};
	struct be_ioctl_ip_wait be_ip_sem = {0,};
	struct be_ioctl_dma_wait be_dma_sem = {0,};

	if (_IOC_TYPE(cmd) != BE_IOCTL_CMD_MAGIC)
		return -ENOTTY;
	if (_IOC_NR(cmd) > BE_IOCTL_CMD_MAGIC)
		return -ENOTTY;
	if (_IOC_DIR(cmd) & _IOC_READ) {
		err = access_ok(VERIFY_WRITE, (void *)arg,
				size = _IOC_SIZE(cmd));
	}
	if (_IOC_DIR(cmd) & _IOC_WRITE) {
		err = access_ok(VERIFY_READ, (void *)arg,
				size = _IOC_SIZE(cmd));
	}
	if ((_IOC_DIR(cmd) != _IOC_NONE)) {
		err = access_ok(VERIFY_READ, (void *)arg,
				size = _IOC_SIZE(cmd));
	}

	if (!err)
		return -EFAULT;

	switch(cmd) {
	case BE_IOCTL_GET_PHYS_REG_INFO:
    {
		ret = copy_from_user((void *) &be_reg_info, (const void *) arg, size);
		if (ret < 0) {
			BE_IOCTL_DEBUG("[BE IOCTL, %s, %d] copy_from_user fail", __FILE__,
					__LINE__);
			return ret;
		}
		be_get_phys_reg_info(be_reg_info.phys_reg_selection,	&be_reg_info.phys_reg_info);

		ret = copy_to_user((void *) arg, (const void *) &be_reg_info, size);
		if (ret < 0) {
			BE_IOCTL_DEBUG("[BE IOCTL, %s, %d] copy_to_user fail", __FILE__, __LINE__);
			return ret;
		}
     }
 	break;
	case BE_IOCTL_OPEN_IRQ  :
	{
		err = be_enable_irq();
		ret = copy_to_user((void *) arg, (const void *) &err, size);
		if (ret < 0) {
			BE_IOCTL_DEBUG("[BE IOCTL, %s, %d] copy_to_user fail", __FILE__, __LINE__);
			return ret;
		}
	}
	break;

	case BE_IOCTL_CLOSE_IRQ :
		be_deinit_irq();
		break;

	case BE_IOCTL_SET_CLOCK :
		ret = copy_from_user((void *) &be_clock_set, (const void *) arg, size);
		if (ret < 0) {
			BE_IOCTL_DEBUG("[BE IOCTL, %s, %d] copy_from_user fail", __FILE__,
					__LINE__);
			return ret;
		}
		be_set_clock(be_clock_set);
		break;

	case BE_IOCTL_HDRLLS_REG_INTR:
	{
		ret = copy_from_user((void*) &usrptr, (const void*) arg, size);
		if(ret < 0){
			BE_IOCTL_DEBUG("[BE IOCTL, %s %d] copy_from_user failed", __FILE__, __LINE__);
			return ret;
		}
	d4_be_hdrlls_register_int(usrptr);
	}
	break;

	case BE_IOCTL_HDRLLS_DEREG_INTR:

		ret = copy_from_user((void*) &usrptr, (const void*) arg, size);
		if(ret < 0){
			BE_IOCTL_DEBUG("[BE IOCTL, %s %d] copy_from_user failed", __FILE__, __LINE__);
			return ret;
		}
	d4_be_hdrlls_deregister_int(usrptr);
	break;

	case BE_IOCTL_HDRLLS_DEREG_DMAINTR:
		ret = copy_from_user((void*) &usrptr, (const void*) arg, size);
		if(ret < 0){
			BE_IOCTL_DEBUG("[BE IOCTL, %s %d] copy_from_user failed", __FILE__, __LINE__);
			return ret;
		}
	d4_be_hdrlls_deregister_dmaint(usrptr);
	break;


	case BE_IOCTL_HDRLLS_EVENT_WAIT:
	{
		ret = copy_from_user((void*)&comp_event, (const void*) arg, size);
		if (ret < 0){
			BE_IOCTL_DEBUG("[BE IOCTL, %s %d] copy_from_user failed", __FILE__, __LINE__);
			return ret;
		}
		ret = d4_be_hdrlls_wait_for_event(comp_event);
		comp_event.event_result = ret;

		ret = copy_to_user((void *) arg, (const void *) &comp_event, size);
		if (ret < 0) {
			BE_IOCTL_DEBUG("[BE IOCTL, %s, %d] copy_to_user fail", __FILE__, __LINE__);
			return ret;
		}
	}
	break;


	case BE_IOCTL_HDRLLS_NR_IWT:
	{
		ret = copy_from_user((void*)&iwt_info, (const void*) arg, size);
		if (ret < 0) {
			BE_IOCTL_DEBUG("[BE IOCTL, %s %d] copy_from_user failed", __FILE__, __LINE__);
		}

		BE_IOCTL_DEBUG("INV ADDR = %#08x", iwt_info.inv_addr);
		BE_IOCTL_DEBUG("INV_SIZE = %d\n", iwt_info.inv_size);
		BE_IOCTL_DEBUG("LW = %d\n", iwt_info.lw);
		BE_IOCTL_DEBUG("INV ADDR = %#08x", iwt_info.inv_addr);
		ret = be_kwlet_iwt(&iwt_info);
	}
	break;

	case BE_IOCTL_INIT_COMPLELETION :
		be_init_intr_compeletion();
		break;

	case BE_IOCTL_INIT_ALL_IP_COMPLETETION :
		be_init_ip_intr_compeletion();
		break;

	case BE_IOCTL_INIT_ALL_DMA_COMPLETETION :
		be_init_dma_intr_compeletion();
		break;

	case BE_IOCTL_INIT_AN_IP_COMPLETETION :
		ret = copy_from_user((void *) &be_core_intr, (const void *) arg, size);
		if (ret < 0) {
			BE_IOCTL_DEBUG("[BE IOCTL, %s, %d] copy_from_user fail", __FILE__,
					__LINE__);
			return ret;
		}
		be_init_an_ip_intr_compeletion(be_core_intr);
		break;

	case BE_IOCTL_INIT_A_DMA_COMPLETETION :
		ret = copy_from_user((void *) &be_dma_intr, (const void *) arg, size);
		if (ret < 0) {
			BE_IOCTL_DEBUG("[BE IOCTL, %s, %d] copy_from_user fail", __FILE__,
					__LINE__);
			return ret;
		}
		be_init_a_dma_intr_compeletion(be_dma_intr);
		break;

	case BE_IOCTL_LOCK_MUTEX:
		be_mutex_lock();
		break;

	case BE_IOCTL_UNLOCK_MUTEX:
		be_mutex_unlock();
		break;

	case BE_IOCTL_WAIT_IP_INTERRUPT :
		ret = copy_from_user((void *) &be_ip_sem, (const void *) arg, size);
		if (ret < 0) {
			BE_IOCTL_DEBUG("[BE IOCTL, %s, %d] copy_from_user fail", __FILE__,
					__LINE__);
			return ret;
		}
		err = be_wait_core_intr_timeout(be_ip_sem.be_core_intr, be_ip_sem.timeout);
		be_ip_sem.result = err;

		ret = copy_to_user((void *) arg, (const void *) &be_ip_sem, size);
		if (ret < 0) {
			BE_IOCTL_DEBUG("[BE IOCTL, %s, %d] copy_to_user fail", __FILE__, __LINE__);
			return ret;
		}
		break;

	case BE_IOCTL_WAIT_DMA_INTERRUPT :
		ret = copy_from_user((void *) &be_dma_sem, (const void *) arg, size);
		if (ret < 0) {
			BE_IOCTL_DEBUG("[BE IOCTL, %s, %d] copy_from_user fail", __FILE__,
					__LINE__);
			return ret;
		}
		err = be_wait_dma_intr_timeout(be_dma_sem.be_dma_intr, be_dma_sem.timeout);
		be_dma_sem.result = err;

		ret = copy_to_user((void *) arg, (const void *) &be_dma_sem, size);
		if (ret < 0) {
			BE_IOCTL_DEBUG("[BE IOCTL, %s, %d] copy_to_user fail", __FILE__, __LINE__);
			return ret;
		}
		break;

	case BE_IOCTL_3DME_KERNEL_DO:
		be_3dme_ktest();
		break;

	default :
		printk("ERROR!! IOCTL COMMAND\n");
		break;
	}
	return ret;
}

/**
 * @brief cma in the area allocated to the virtual address to create an application that functions
 * @fn int be_mmap(struct file *file, struct vm_area_struct *vma)
 * @param *file[in] driver file discripter
 * @param *vma[in/out] To configure virtual memory map information structure <br>
 * @return On error returns a negative error, zero otherwise. <br>
 *
 * @author Niladri Mukherjee
 * @note
 */

int d4_be_mmap(struct file *file, struct vm_area_struct *vma)
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

const struct file_operations drime4_be_ops = {
		.owner = THIS_MODULE,
		.open = d4_be_open,
		.release = d4_be_release,
		.unlocked_ioctl =	d4_be_ioctl,
		.mmap = d4_be_mmap
};

MODULE_AUTHOR("Niel Mukherjee <n.mukherjee@samsung.com>");
MODULE_DESCRIPTION("Samsung Drime IV BE Driver");
MODULE_LICENSE("GPL");

