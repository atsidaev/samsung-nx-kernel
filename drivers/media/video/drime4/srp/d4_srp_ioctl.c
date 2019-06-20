/* linux\drivers\media\video\drime4\srp\d4_srp_ioctl.c
 *
 * @file d4_srp_ioctl.c
 * @brief DRIMe4 SRP Ioctl Control Function File
 * @author Geunjae Yu <geunjae.yu@samsung.com>
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
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/clk.h>
#include <asm/page.h>
#include <asm/cacheflush.h>

/*
#include "media/drime4/srp/d4_srp_type.h"
*/
//#include "media/video/drime4/mfc5x/mfc_pm.h"
#include "../mfc5x/mfc_pm.h"
#include "media/drime4/srp/d4_srp_ioctl.h"

#include <mach/d4_cma.h>
#include <mach/d4_mem.h>
#include <mach/srp/d4_srp.h>
#include "d4_srp_if.h"

#include <linux/pinctrl/pinconf-drime4.h>
#include <linux/pinctrl/pinconf.h>
#include <linux/pinctrl/consumer.h>



extern struct d4_srp *g_srp;


/* IRQ */
static struct completion SrpOpCompletion;
static int d4_srp_irq_num;

/*  Device Info */
static struct device *d4_srp_dev;

/* physical register information */
static struct srp_phys_reg_info phys_reg_info[4];

/* SRP Clock */
static struct clk *srp_clk_set;


static unsigned int  __iomem d4_srp_reg_base;
static unsigned int  __iomem d4_srp_dap_base;
static unsigned int  __iomem d4_srp_commbox_base;
static unsigned int  __iomem d4_srp_ssram_base;


inline void write_srp_register(unsigned int offset, unsigned int val)
{
	/*
	printk("[SRP] write srp reg [0x%08x] = 0x%x(%d)\n"
				, offset, val, val);
	*/
	__raw_writel(val, offset);
}


/**
 * @brief  set resigter address
 * @fn     srp_set_reg_ctrl_base_info(struct srp_reg_ctrl_base_info *info)
 * @param  struct srp_reg_ctrl_base_info *info
 * @return void
 * @author <geunjae.yu@samsung.com>
 * @note   NONE
 */
void d4_srp_set_reg_ctrl_base_info(struct srp_reg_ctrl_base_info *info)
{
	d4_srp_dev		= info->dev;

	d4_srp_reg_base		= info->srp_reg_base;
	d4_srp_dap_base		= info->srp_dap_base ;
	d4_srp_commbox_base	= info->srp_commbox_base;
	d4_srp_ssram_base 	= info->srp_ssram_base;

	d4_srp_irq_num		= info->irq_num;

	/* physical register information */
	phys_reg_info[SRP_REG].reg_start_addr = info->srp_reg_info.reg_start_addr;
	phys_reg_info[SRP_REG].reg_size = info->srp_reg_info.reg_size;

	phys_reg_info[DAP_REG].reg_start_addr = info->srp_dap_info.reg_start_addr;
	phys_reg_info[DAP_REG].reg_size = info->srp_dap_info.reg_size;

	phys_reg_info[COMMBOX_REG].reg_start_addr = info->srp_commbox_info.reg_start_addr;
	phys_reg_info[COMMBOX_REG].reg_size = info->srp_commbox_info.reg_size;

	phys_reg_info[SSRAM_REG].reg_start_addr = info->srp_ssram_info.reg_start_addr;
	phys_reg_info[SSRAM_REG].reg_size = info->srp_ssram_info.reg_size;
}


void d4_srp_get_phys_reg_info(enum srp_reg_selection selection,
				struct srp_phys_reg_info *reg_info)
{
	switch (selection) {
	case SRP_REG:
		reg_info->reg_start_addr = phys_reg_info[SRP_REG].reg_start_addr;
		reg_info->reg_size = phys_reg_info[SRP_REG].reg_size;
		/*
		printk("[D4_SRP] SRP Base : 0x%08x, size:%x\n", reg_info->reg_start_addr, reg_info->reg_size);
		*/
		break;
	case DAP_REG:
		reg_info->reg_start_addr = phys_reg_info[DAP_REG].reg_start_addr;
		reg_info->reg_size = phys_reg_info[DAP_REG].reg_size;
		/*
		printk("[D4_SRP] DAP Base : 0x%08x, size:%x\n", reg_info->reg_start_addr, reg_info->reg_size);
		*/
		break;
	case COMMBOX_REG:
		reg_info->reg_start_addr = phys_reg_info[COMMBOX_REG].reg_start_addr;
		reg_info->reg_size = phys_reg_info[COMMBOX_REG].reg_size;
		/*
		printk("[D4_SRP] COMMBOX Base : 0x%08x, size:%x\n", reg_info->reg_start_addr, reg_info->reg_size);
		*/
		break;
	case SSRAM_REG:
		reg_info->reg_start_addr = phys_reg_info[SSRAM_REG].reg_start_addr;
		reg_info->reg_size = phys_reg_info[SSRAM_REG].reg_size;
		/*
		printk("[D4_SRP] SRAM Base : 0x%08x, size:%x\n", reg_info->reg_start_addr, reg_info->reg_size);
		*/
		break;
	}
}


/**
 * @brief  set srp clock on/off
 * @fn     d4_srp_set_clock(unsigned int clock_set)
 * @param  onoff
 * @return void
 * @author <geunjae.yu@samsung.com>
 * @note   NONE
 */
void d4_srp_set_clock(struct srp_ioctl_clock_info clock_info)
{
	/*
	printk("[D4_SRP] d4_srp_set_clock()\n");
	*/

	/* check need to chagne clock freq. */
	if (clock_info.clock_change == 1) {
		srp_clk_set = clk_get(d4_srp_dev, "srp");
		if (srp_clk_set == -2)
				return;
		clk_set_rate(srp_clk_set, clock_info.clock_freq);
	}

	/**< Clock Enable -> already enable in probe function */
	/**disable in probe fuction */
	if (clock_info.clock_on == SRP_CLOCK_ON)
		clk_enable(g_srp->clock);
	else
		clk_disable(g_srp->clock);

}

/**
 * @brief  set srp/codec mode select(pinmux)
 * @fn     d4_srp_set_clock(unsigned int clock_set)
 * @param  mode_ctl 0:SRP, 1:CODEC(default)
 * @return int
 * @author <geunjae.yu@samsung.com>
 * @note   NONE
 */
int d4_srp_set_modectl(unsigned int mode_ctl)
{
	unsigned long conf;
	int ret;

	/*
	printk("[D4_SRP] d4_srp_set_modectl : %d(0:SRP, 1:CODEC)\n",mode_ctl);
	*/

	/**< Mode Select CODEC->SRP */
 	/* unblock after update pinmux fuction */
	/*d4_rmu_modecon_set(RMU_MODECON_CODEC_SRP,RMU_SEL_SRP);*/
	/*pinmux_enable(g_srp->pmx);*/
	conf = to_config_packed(PIN_CONFIG_MODE_CTRL_CODEC_SRP_SEL, mode_ctl);
	ret = pin_config_set("drime4-pinmux", "MODE_CTRL", conf);

	if (ret)
		return -EINVAL;

	return ret;
}



/**
 * @brief  get operatoin semaphore
 * @fn     d4_srp_Get_OpSema(int timeOut)
 * @param  int timeOut
 * @return int
 * @author <geunjae.yu@samsung.com>
 * @note   NONE
 */
int d4_srp_Get_OpSema(unsigned int timeout)
{
	int semStatus = SRP_SUCCESS;

	semStatus = wait_for_completion_timeout(&SrpOpCompletion, timeout);

	return semStatus;
}

/**
 * @brief  srp interrupt serive routain
 * @fn     d4_srp_intr_isr(void)
 * @param  void
 * @return IRQ_HANDLED
 * @author <geunjae.yu@samsung.com>
 * @note   NONE
 */
irqreturn_t d4_srp_intr_isr(void)
{
	complete(&SrpOpCompletion);
	write_srp_register(d4_srp_commbox_base + D4_SRP_HOST_IRQ*sizeof(unsigned int), 0x0);
	/*
	printk("[D4_SRP] ISR\n");
	*/
	return IRQ_HANDLED;
}


/**
 * @brief  Enable srp IRQ
 * @fn     d4_srp_int_init(void)
 * @param  void
 * @return int
 * @author <geunjae.yu@samsung.com>
 * @note   NONE
 */
int d4_srp_int_init(void)
{
	int result = SRP_SUCCESS;

	/*
	printk("[D4_SRP] d4_srp_int_init()\n");
	*/

	/**< Top Interrupt Registeration */
	if (request_irq(d4_srp_irq_num, (void *)d4_srp_intr_isr, IRQF_DISABLED, SRP_MODULE_NAME, d4_srp_dev)) {
		result = SRP_IRQ_FAIL;
		printk(KERN_INFO "failed to srp request_irq failed\n");
	}

	/**< completion initialization for synchronization */
	init_completion(&SrpOpCompletion);

	return result;
}

/**
 * @brief  free ipq resource
 * @fn     d4_srp_int_Deinit(void)
 * @param  void
 * @return void
 * @author <geunjae.yu@samsung.com>
 * @note   NONE
 */
void d4_srp_int_Deinit(void)
{
	/*
	printk("[D4_SRP] d4_srp_int_Deinit()\n");
	*/
	free_irq(d4_srp_irq_num, d4_srp_dev);
}

void d4_srp_init_completion(void)
{
	init_completion(&SrpOpCompletion);
}


/**
 * @brief  SRP device open
 * @fn     d4_srp_open(struct inode *inode, struct file *filp)
 * @param  struct inode *inode, struct file *filp
 * @return int
 * @author <geunjae.yu@samsung.com>
 * @note   NONE
 */
int d4_srp_open(struct inode *inode, struct file *filp)
{
	struct d4_srp *srp_ctx = g_srp;
	/*
	printk("[D4_SRP] d4_srp_open()\n");
	*/
	filp->private_data = srp_ctx;
	return 0;
}

/**
 * @brief  SRP device release
 * @fn     d4_srp_release(struct inode *inode, struct file *filp)
 * @param  struct inode *inode, struct file *filp
 * @return int
 * @author <geunjae.yu@samsung.com>
 * @note   NONE
 */
static int d4_srp_release(struct inode *inode, struct file *filp)
{
	/*
	printk("[D4_SRP] d4_srp_release()\n");
	*/
	return 0;
}

/**
 * @brief  SRP device read
 * @fn     d4_srp_read(struct file *filp, char *buf, size_t count, loff_t *f_pos)
 * @param  struct file *filp, char *buf, size_t count, loff_t *f_pos
 * @return ssize_t
 * @author <geunjae.yu@samsung.com>
 * @note   NONE
 */
ssize_t d4_srp_read(struct file *filp, char *buf, size_t count, loff_t *f_pos)
{
	/*
	printk("[D4_SRP] d4_srp_read()\n");
	*/
	return 0;
}

/**
 * @brief  SRP device write
 * @fn     d4_srp_write(struct file *filp, char *buf, size_t count, loff_t *f_pos)Kernel dependency한
 * @param  struct file *filp, char *buf, size_t count, loff_t *f_pos
 * @return ssize_t
 * @author <geunjae.yu@samsung.com>
 * @note   NONE
 */
ssize_t d4_srp_write(struct file *filp, const char *buf, size_t count, off_t *f_pos)
{
	/*
	printk("[D4_SRP] d4_srp_write()\n");
	*/
	return 0;
}



/**
 * @brief  SRP DD interface function
 * @fn     d4_srp_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
 * @param  struct file *filp, unsigned int cmd, unsigned long arg
 * @return long
 * @author <geunjae.yu@samsung.com>
 * @note   NONE
 */
long d4_srp_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	int size = 0;
	int err = -1;

	unsigned int srp_modectl_set = 0;
	unsigned int set_delay = 0;

	struct srp_ioctl_clock_info clk_info = {0,};
	struct srp_ioctl_get_reg_info reg_info = {0,};
	struct srp_ioctl_sema_info sem_info = {0,};

	if (_IOC_TYPE(cmd) != SRP_MAGIC)
		return -1;

	size = _IOC_SIZE(cmd);

	if (_IOC_DIR(cmd) & _IOC_READ)
		err = access_ok(VERIFY_WRITE, (void *) arg, size);
	else if (_IOC_DIR(cmd) & _IOC_WRITE)
		err = access_ok(VERIFY_READ, (void *) arg, size);

	if (!err)
		return -1;

	/*
	printk("[D4_SRP] srp_ioctl() - cmd : %d", cmd);
	*/

	switch (cmd) {

	case SRP_IOCTL_OPEN_IRQ:
		d4_srp_int_init();
		break;
	case SRP_IOCTL_CLOSE_IRQ:
		d4_srp_int_Deinit();
		break;
	case SRP_IOCTL_INIT_COMPLETION:
		d4_srp_init_completion();
		break;

	case SRP_IOCTL_SET_CLOCK:
		if (copy_from_user((void *) &clk_info, (const void *) arg, size)) {
			printk("ioctl fail: [%d]", cmd);
			return -EFAULT;
		}
		d4_srp_set_clock(clk_info);
		break;

	case SRP_IOCTL_SET_MODECTL:
		if (copy_from_user((void *) &srp_modectl_set, (const void *) arg, size)) {
			printk("ioctl fail: [%d]", cmd);
			return -EFAULT;
		}
		//codec power on/off when srp init/end for power management 	
		if(srp_modectl_set == MODE_CTL_SRP)	
			mfc_power_on();	
		else
			mfc_power_off();	
		d4_srp_set_modectl(srp_modectl_set);
		break;

	case SRP_IOCTL_SET_DELAY:
		if (copy_from_user((void *)&set_delay, (const void *)arg, size)) {
			printk("ioctl fail: [%d]", cmd);
			return -EFAULT;
		}
		mdelay(set_delay);
		break;

	case SRP_IOCTL_SEMAPHORE:
		if (copy_from_user((void *) &sem_info, (const void *) arg, size)) {
			printk("ioctl fail: [%d]", cmd);
			return -EFAULT;
		}
		err = d4_srp_Get_OpSema(sem_info.timeout);
		sem_info.result = err;

		if (copy_to_user((void *) arg, (const void *) &sem_info, size)) {
			printk("ioctl fail: [%d]", cmd);
			return -EFAULT;
		}
		break;

	case SRP_IOCTL_GET_PHYS_REG_INFO:
		if (copy_from_user((void *) &reg_info, (const void *) arg, size)) {
			printk("ioctl fail: [%d]", cmd);
			return -EFAULT;
		}
		d4_srp_get_phys_reg_info(reg_info.phys_reg_selection, &reg_info.phys_reg_info);

		if (copy_to_user((void *) arg, (const void *) &reg_info, size)) {
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
  * @brief  cma로 할당된 영역에 대한 application 접근용 virtual address를 생성해주는 함수
  * @fn     int srp_mmap(struct file *file, struct vm_area_struct *vma)
  * @param  *file[in] driver file discripter
  * @param  *vma[in/out] virtual memory map을 구성하기 위한 정보 structure<br>
  * @return On error returns a negative error, zero otherwise. <br>
  *
  * @author Junkwon Choi
  * @note
  */
int d4_srp_mmap(struct file *file, struct vm_area_struct *vma)
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

const struct file_operations drime4_srp_ops = {
		.open 		= d4_srp_open,
		.release 	= d4_srp_release,
		.read 		= d4_srp_read,
		.write 		= d4_srp_write,
		.unlocked_ioctl	= d4_srp_ioctl,
		.mmap 		= d4_srp_mmap
};

MODULE_DESCRIPTION("Samsung DRIMe4 SRP driver using ioctl");
MODULE_LICENSE("GPL");

