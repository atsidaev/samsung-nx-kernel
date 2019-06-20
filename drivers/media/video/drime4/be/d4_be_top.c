/**
 * @file d4_be_top.c
 * @brief DRIMe4 BE TOP Interface / Control
 * @author Niladri Mukherjee <niladri.mukherjee@samsung.com>
 * 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

/*=============================
 * Bayer SYSTEM INCLUCES
 =============================*/
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/io.h>
#include <linux/sched.h>
#include <linux/device.h>
#include <mach/d4_cma.h>
#include <linux/clk.h>
#include <linux/interrupt.h>
#include <linux/uaccess.h>
#include <linux/delay.h>
#include <linux/d4-pmu.h>
#include <linux/d4_rmu.h>
/*=============================
 * Bayer USER INCLUCES
 =============================*/
#include <mach/be/d4_be.h>
#include "d4_be_top.h"
#include "d4_be_regs.h"
/**< Register Base Address */
unsigned int be_top_reg_base;
/**< IRQ Number */
unsigned int be_irq_num;
/*=============================
 * static variables
 =============================*/
/**< Register Base Address */
static unsigned int be_ghost_reg_base;
static unsigned int be_snr_reg_base;
static unsigned int be_sg_reg_base;
static unsigned int be_blend_reg_base;
static unsigned int be_fme_reg_base;
static unsigned int be_3dme_reg_base;
static unsigned int be_dma_reg_base;
/**< BE Device Info */
static struct device *be_dev;
/**< physical register information */
static struct be_phys_reg_info be_phys_reg_info[8];
/**< BE mutex */
static DEFINE_MUTEX(be_mutex_0);
/**
 *	@brief	Bayer	IWT	Global	Variables
 *				image_lock:	ï¿½ï¿½ï¿½ï¿½	ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½	Critical	Section	ï¿½ï¿½È£ï¿½ï¿½	ï¿½ï¿½ï¿½ï¿½	ï¿½ï¿½ï¿½ï¿½Ï´ï¿?Mutex
 */
static DEFINE_MUTEX(image_lock);
/**< BE completion */
static struct completion be_ip_complete[BE_INT_LEVEL_MAX];
static struct completion be_dma_complete[BE_DMA_INT_LEVEL_MAX];
/*************************************************************/
/**< Completion event instances */
/**!! <NR Completion instances * */
static struct completion be_snr_cmd;
static	struct completion be_wlet_snr_fwt_cmd;
static	struct completion be_wlet_snr_iwt_cmd;
static	struct completion be_wlet_snr_strip_iwtc_cmd;
static int bayer_wlet_isr_iwt_complete;
/*============================================== * */
/**!! < YCC BLEND Completion instances * */
static struct completion be_blend_cmd_lut;
static struct completion be_blend_cmd_max;
static struct completion be_blend_cmd_bld;
static struct completion be_blend_wdma_ch2;
static int be_ycc_blend;
/*============================================== * */
/**!! < GHOST Completion instances * */
static struct completion be_fchk_mode_dth_fwd_obj;
static struct completion be_fchk_mode_dth_rev_obj;

static struct completion be_fchk_mode_osmc_lgd_obj;
static struct completion be_fchk_mode_osmc_lgc_obj;
static struct completion be_fchk_mode_fchk2_obj;

static struct completion be_fchk_mode_corner_case;
static struct completion be_fchk_mode_over_spec;
//static int be_fchk_mode_corner_case_flag;
//static int be_fchk_mode_over_spec_flag;

static struct completion be_ghost_cmd_gd;
static struct completion be_ghost_deghost_blend;
static struct completion be_ghost_cmd_fw_pre_compl;
static struct completion be_ghost_cmd_fw_weight_compl;
/*====================================== * */
/**!! < SG Completion instances * */
static struct completion be_sg_mode_cmd_prf_obj;
static struct completion be_sg_mode_cmd_r2s_obj;
static struct completion be_sg_mode_cmd_gme_obj;
static struct completion be_sg_mode_cmd_lmc_obj;
static struct completion be_sg_mode_cmd_lme_obj;
static struct completion be_sg_dma_wdma_ch0_obj;
static int r2s_interrupt_received;
static int lmc_interrupt_received;
/* ====================================== * */
/**************************************************/
/**
 * @brief set resigter address
 * @fn   be_set_reg_ctrl_base_info(struct be_reg_ctrl_base_info *info)
 * @param struct be_reg_ctrl_base_info *info
 * @return void
 * @author Yunmi Lee <ym4404.lee@samsung.com>
 * @note  -
 */
void be_set_reg_ctrl_base_info(struct be_reg_ctrl_base_info *info)
{
	be_dev = 			info->dev_info;
	be_top_reg_base = 	info->top_reg_base;
	be_ghost_reg_base = info->ghost_reg_base;
	be_snr_reg_base = 	info->snr_reg_base;
	be_sg_reg_base = 	info->sg_reg_base;
	be_blend_reg_base = info->blend_reg_base;
	be_fme_reg_base = 	info->fme_reg_base;
	be_3dme_reg_base = 	info->cme_reg_base;
	be_dma_reg_base = 	info->dma_reg_base;
	be_irq_num = 		info->irq_num;

	/**< physical register information */
	be_phys_reg_info[BE_TOP_REG].reg_start_addr	= info->phys_top_reg_info.reg_start_addr;
	be_phys_reg_info[BE_TOP_REG].reg_size		= info->phys_top_reg_info.reg_size;

	be_phys_reg_info[BE_GHOST_REG].reg_start_addr = info->phys_ghost_reg_info.reg_start_addr;
	be_phys_reg_info[BE_GHOST_REG].reg_size		 = info->phys_ghost_reg_info.reg_size;

	be_phys_reg_info[BE_SNR_REG].reg_start_addr	 = info->phys_snr_reg_info.reg_start_addr;
	be_phys_reg_info[BE_SNR_REG].reg_size   	 = info->phys_snr_reg_info.reg_size;

	be_phys_reg_info[BE_SG_REG].reg_start_addr 	 = info->phys_sg_reg_info.reg_start_addr;
	be_phys_reg_info[BE_SG_REG].reg_size	     = info->phys_sg_reg_info.reg_size;

	be_phys_reg_info[BE_BLEND_REG].reg_start_addr = info->phys_blend_reg_info.reg_start_addr;
	be_phys_reg_info[BE_BLEND_REG].reg_size 	  = info->phys_blend_reg_info.reg_size;

	be_phys_reg_info[BE_FME_REG].reg_start_addr	  = info->phys_fme_reg_info.reg_start_addr;
	be_phys_reg_info[BE_FME_REG].reg_size		  = info->phys_fme_reg_info.reg_size;

	be_phys_reg_info[BE_3DME_REG].reg_start_addr  = info->phys_3dme_reg_info.reg_start_addr;
	be_phys_reg_info[BE_3DME_REG].reg_size		  = info->phys_3dme_reg_info.reg_size;

	be_phys_reg_info[BE_DMA_REG].reg_start_addr	  = info->phys_dma_reg_info.reg_start_addr;
	be_phys_reg_info[BE_DMA_REG].reg_size		  = info->phys_dma_reg_info.reg_size;
}

/**
 * @brief get resiter physical address
 * @fn  void be_get_phys_reg_info(enum be_reg_selection selection,
 *		     		struct be_phys_reg_info *reg_info)
 * @param enum be_reg_selection selection
 * @param struct be_phy_reg_info *reg_info
 * @return void
 * @author Yunmi Lee <ym4404.lee@samsung.com>
 * @note -
 */
void be_get_phys_reg_info(enum be_reg_selection selection,
				struct be_phys_reg_info *reg_info)
{
	switch (selection) {
	case BE_TOP_REG:
		reg_info->reg_start_addr = be_phys_reg_info[BE_TOP_REG].reg_start_addr;
		reg_info->reg_size 		 = be_phys_reg_info[BE_TOP_REG].reg_size;
		break;

	case BE_GHOST_REG:
		reg_info->reg_start_addr = be_phys_reg_info[BE_GHOST_REG].reg_start_addr;
		reg_info->reg_size = 	   be_phys_reg_info[BE_GHOST_REG].reg_size;
		break;

	case BE_SNR_REG:
		reg_info->reg_start_addr = be_phys_reg_info[BE_SNR_REG].reg_start_addr;
		reg_info->reg_size = 	   be_phys_reg_info[BE_SNR_REG].reg_size;
		break;

	case BE_SG_REG:
		reg_info->reg_start_addr = be_phys_reg_info[BE_SG_REG].reg_start_addr;
		reg_info->reg_size =  	   be_phys_reg_info[BE_SG_REG].reg_size;
		break;

	case BE_BLEND_REG:
		reg_info->reg_start_addr = be_phys_reg_info[BE_BLEND_REG].reg_start_addr;
		reg_info->reg_size = 	   be_phys_reg_info[BE_BLEND_REG].reg_size;
		break;

	case BE_FME_REG:
		reg_info->reg_start_addr = be_phys_reg_info[BE_FME_REG].reg_start_addr;
		reg_info->reg_size = 	   be_phys_reg_info[BE_FME_REG].reg_size;
		break;

	case BE_3DME_REG:
		reg_info->reg_start_addr = be_phys_reg_info[BE_3DME_REG].reg_start_addr;
		reg_info->reg_size = 	   be_phys_reg_info[BE_3DME_REG].reg_size;
		break;

	case BE_DMA_REG:
		reg_info->reg_start_addr = be_phys_reg_info[BE_DMA_REG].reg_start_addr;
		reg_info->reg_size = 	   be_phys_reg_info[BE_DMA_REG].reg_size;
		break;

	default:
		/*never reached*/
		printk("[%s, %d]invalid parameter. selection %d\n", __func__, __LINE__, selection);
		break;
	}
}

/**
 * @brief set be clock
 * @fn   void be_set_clock(unsigned int clock_set)
 * @param unsigned int clock_set
 * @return void
 * @author Yunmi Lee <ym4404.lee@samsung.com>
 * @note -
 */
void be_set_clock(unsigned int clock_set)
{
	/**<BE Clock*/
	static struct clk *be_clk_set;
	/* Clock Enable*/
	be_clk_set = clk_get(be_dev, "be");
	/*Clock frequency*/
	if (be_clk_set == -2)
		return;

	clk_set_rate(be_clk_set, clock_set);
	BE_KERN_DEBUG_MSG("\n=======================================\n");
	BE_KERN_DEBUG_MSG("Function : %s\n", __FUNCTION__);
	BE_KERN_DEBUG_MSG("Clock Rate : %d Hz\n", clock_set);
	BE_KERN_DEBUG_MSG("=======================================\n");
}



/**
 * @brief This locks the BE mutex.
 * @fn   void be_mutex_lock(void)
 * @param void
 * @return void
 * @author Yunmi Lee <ym4404.lee@samsung.com>
 * @note -
 */
void be_mutex_lock(void)
{
	mutex_lock(&be_mutex_0);
}

/**
 * @brief This unlocks the BE mutex.
 * @fn   void be_unmutex_lock(void)
 * @param void
 * @return void
 * @author Yunmi Lee <ym4404.lee@samsung.com>
 * @note -
 */
void be_mutex_unlock(void)
{
	mutex_unlock(&be_mutex_0);
}

/*==============================NR CALLBACK======================================== * */
void __be_snr_isr_clbk(void)
{
	unsigned int mask = 0;
	BE_KERN_DEBUG_MSG("be_snr_isr_clbk received!!!!\n");
	complete(&be_snr_cmd);
	mask = READ_BE_REG(be_snr_reg_base, D4_BE_SNR_STATUS);
	/*Enable	the	interrupt*/
	mask |= (1 << 16);
	WRITE_BE_REG(be_snr_reg_base, D4_BE_SNR_STATUS, mask);
	mask = READ_BE_REG(be_snr_reg_base, D4_BE_SNR_STATUS);
	/*Enable	the	interrupt*/
	mask &= ~(1 << 16);
	WRITE_BE_REG(be_snr_reg_base, D4_BE_SNR_STATUS, mask);
}

void __be_wlet_isr_fwt(void)
{
	unsigned int mask = READ_BE_REG(be_snr_reg_base, D4_BE_SNR_WLET_STATUS);
	BE_KERN_DEBUG_MSG("FWT Received!!!!!!\n");
	mask |= STAT_FWT;
	WRITE_BE_REG(be_snr_reg_base, D4_BE_SNR_WLET_STATUS, mask);
	complete(&be_wlet_snr_fwt_cmd);

}

void __be_wlet_isr_inv(void)
{
	unsigned int mask = 0;
	complete(&be_wlet_snr_iwt_cmd);
	complete(&be_wlet_snr_strip_iwtc_cmd);
	bayer_wlet_isr_iwt_complete = 1;
	BE_KERN_DEBUG_MSG("be_wlet_isr_inv received!!!!!!!\n");
	mask = READ_BE_REG(be_snr_reg_base, D4_BE_SNR_WLET_STATUS);
	mask |= STAT_IWT;
	WRITE_BE_REG(be_snr_reg_base, D4_BE_SNR_WLET_STATUS, mask);

}

void __be_wlet_isr_inv_c(void)
{
	unsigned int mask = 0;
	complete(&be_wlet_snr_strip_iwtc_cmd);
	//bayer_wlet_isr_iwt_continue = 1;
	BE_KERN_DEBUG_MSG("be_wlet_isr_inv_c received !!!!\n");
	mask = READ_BE_REG(be_snr_reg_base, D4_BE_SNR_WLET_STATUS);
	mask |= STAT_IWTC;
	WRITE_BE_REG(be_snr_reg_base, D4_BE_SNR_WLET_STATUS, mask);
}

/*===========================INT/CALLBACK REGISTRATION========================== * */
void be_snr_register_kisr_clbk()
{
	init_completion(&be_snr_cmd);
	BE_KERN_DEBUG_MSG("be_snr_cmd registered !!!!\n");
	be_top_isr_register_clbk(BE_INT_LEVEL_SNR_SNR, __be_snr_isr_clbk);
}

void be_nr_register_fwt()
{
	init_completion(&be_wlet_snr_fwt_cmd);
	BE_KERN_DEBUG_MSG("be_wlet_snr_fwt_cmd registered !!!!\n");
	be_top_isr_register_clbk(BE_INT_LEVEL_SNR_FWT, __be_wlet_isr_fwt);
}

void be_nr_register_iwt()
{
	init_completion(&be_wlet_snr_iwt_cmd);
	BE_KERN_DEBUG_MSG("be_wlet_snr_iwt_cmd registered !!!!\n");
	be_top_isr_register_clbk(BE_INT_LEVEL_SNR_IWT, __be_wlet_isr_inv);
}

void be_nr_register_strip()
{
	init_completion(&be_wlet_snr_strip_iwtc_cmd);
	BE_KERN_DEBUG_MSG("be_wlet_snr_strip_iwtc_cmd registered !!!!\n");
	be_top_isr_register_clbk(BE_INT_LEVEL_SNR_STRIP, __be_wlet_isr_inv_c);
}

void be_nr_block_fiwlet()
{
	be_nr_register_fwt();
	be_nr_register_iwt();
	be_nr_register_strip();
}



BE_RESULT be_nr_wait_for_kcompletion(unsigned int cmd, int timeout)
{

	BE_RESULT result = D4_BE_SUCCESS;
	switch(cmd){
	case BE_INT_LEVEL_SNR_SNR:
		BE_KERN_DEBUG_MSG("WAIT FOR BE_LLSHDR_SNR_BLK!!!\n");
		/*Wait	Until	Done*/
		if (wait_for_completion_timeout(&be_snr_cmd,
			msecs_to_jiffies(timeout)) == 0) {
		pr_err("BAYER:	be_snr_callback	timeout	while	interrupt	wait\n");
		result = -D4_BE_ERR_INTR_TIME_OUT;
		goto EXIT_FUNC;
		}
		result = 0;
		BE_KERN_DEBUG_MSG("be_snr_cmd Done\n");
	break;

	case 	BE_INT_LEVEL_SNR_FWT:
		BE_KERN_DEBUG_MSG("WAIT FOR SG::PRF!!!\n");
		/*Wait Until Done*/
		if (wait_for_completion_timeout(&be_wlet_snr_fwt_cmd, msecs_to_jiffies(timeout))
				== 0) {
			pr_err("BAYER:	be_wlet_fwt	timeout	while	interrupt	wait\n");
			result = -D4_BE_ERR_INTR_TIME_OUT;
			goto EXIT_FUNC;
		}
		result = D4_BE_SUCCESS;
		break;

	case	BE_INT_LEVEL_SNR_IWT:
		BE_KERN_DEBUG_MSG("WAIT FOR BE_LLSHDR_NR_IWT_BLK!!!\n");
		/*Wait Until Done*/
		if (wait_for_completion_timeout(&be_wlet_snr_iwt_cmd, msecs_to_jiffies(timeout))
				== 0) {
			pr_err("BAYER:	be_wlet_snr_iwt_cmd	timeout	while	interrupt	wait\n");
			result = -D4_BE_ERR_INTR_TIME_OUT;
			goto EXIT_FUNC;
		}
		result = D4_BE_SUCCESS;
		break;

	case	BE_INT_LEVEL_SNR_STRIP:
		BE_KERN_DEBUG_MSG("WAIT FOR BE_LLSHDR_NR_IWT_BLK!!!\n");
		/*Wait Until Done*/
		if (wait_for_completion_timeout(&be_wlet_snr_strip_iwtc_cmd, msecs_to_jiffies(timeout))
				== 0) {
			pr_err("BAYER:	be_wlet_snr_strip_iwtc_cmd	timeout	while	interrupt	wait\n");
			result = -D4_BE_ERR_INTR_TIME_OUT;
			goto EXIT_FUNC;
		}
		result = D4_BE_SUCCESS;
		break;
	}
EXIT_FUNC:
		return result;
}

/* ======================================================== * */
/*=====================IWT OPERATION======================= * */

static void d4_be_kvdma_set_write_port_iwt(int ch, int port, unsigned int addr,
		int stride, int bit_format)
{
	BE_KERN_DEBUG_MSG("DMA WRITE Channel = %d and Port=%d\n", ch, port);

	BE_KERN_DEBUG_MSG("**set_WRITE_port**channel = %d\n", ch);
	BE_KERN_DEBUG_MSG("**set_WRITE_port**ch_port = %d\n", port);
	BE_KERN_DEBUG_MSG("**set_WRITE_port**address = %x\n", addr);
	BE_KERN_DEBUG_MSG("**set_WRITE_port**stride = %d\n", stride);
	BE_KERN_DEBUG_MSG("**set_WRITE_port**bit_fmt = %x\n", bit_format);
	BE_KERN_DEBUG_MSG("be_dma_reg_base = %#08x\n", be_dma_reg_base);


	WRITE_BE_REG(be_dma_reg_base, D4_BE_KVDMA_W1_IFMT10,	(bit_format << 8) | (bit_format << 4));
	WRITE_BE_REG(be_dma_reg_base, D4_BE_KVDMA_W1_EVN0, addr);
	WRITE_BE_REG(be_dma_reg_base, D4_BE_KVDMA_W1_ODD0, addr + stride);
	WRITE_BE_REG(be_dma_reg_base, D4_BE_KVDMA_W1_FOFF0, stride * 2);

	WRITE_BE_REG(be_dma_reg_base, D4_BE_KVDMA_W1_XOFF10,	0x10000);
	WRITE_BE_REG(be_dma_reg_base, D4_BE_KVDMA_W1_BGNX10, 0);
	WRITE_BE_REG(be_dma_reg_base, D4_BE_KVDMA_W1_XOFF32, 0x10000);
	WRITE_BE_REG(be_dma_reg_base, D4_BE_KVDMA_W1_BGNX32, 0);
	WRITE_BE_REG(be_dma_reg_base, D4_BE_KVDMA_W1_TIMETHR, 0);
	WRITE_BE_REG(be_dma_reg_base, D4_BE_KVDMA_W1_BURSTLEN, 0xF);
}

static void wavelet_kiwt_oneshot(int cmd)
{
	int mask = 0;
	if (cmd == CMD_IWTC) {
		WRITE_BE_REG(be_snr_reg_base, D4_BE_KSNR_WLET_IWT_CONTINUE, 1);
		WRITE_BE_REG(be_snr_reg_base, D4_BE_KSNR_WLET_IWT_CONTINUE, 0);
	} else if (cmd == CMD_IWT) {
		mask = 1 << 16;
		WRITE_BE_REG(be_snr_reg_base, D4_BE_KSNR_WLET_START_IWT, mask);
		mask = READ_BE_REG (be_snr_reg_base, D4_BE_KSNR_WLET_START_IWT);
		mask = ~(1 << 16);
		WRITE_BE_REG(be_snr_reg_base, D4_BE_KSNR_WLET_START_IWT, mask);
	}
}


BE_RESULT be_kwlet_iwt(struct be_nr_iwt_info *info)
{
	BE_RESULT result = D4_BE_SUCCESS;
	BE_KERN_DEBUG_MSG("bayer_wlet_isr_iwt_complete = %d\n", bayer_wlet_isr_iwt_complete);

	be_nr_register_strip();
	be_nr_register_iwt();
	wavelet_kiwt_oneshot(CMD_IWT);
	if (wait_for_completion_timeout(&be_wlet_snr_iwt_cmd,
			msecs_to_jiffies(50)) == 0) {
		do {
			init_completion(&be_wlet_snr_strip_iwtc_cmd);
			BE_KERN_DEBUG_MSG("********IWT_C EXECUTING*******\n");
			BE_KERN_DEBUG_MSG("inv_addr = %x, inv_size = %x\n", info->inv_addr, info->inv_size);
			info->inv_addr = info->inv_addr + info->inv_size;
			BE_KERN_DEBUG_MSG("After Addition inv_addr = %x\n", info->inv_addr);
			BE_KERN_DEBUG_MSG("lw = %d\n", info->lw);
			d4_be_kvdma_set_write_port_iwt(BE_WLET_KWDMA_CH_IWT, 0, info->inv_addr, info->stride, info->dma_bit);
			mutex_lock(&image_lock);
				wavelet_kiwt_oneshot(CMD_IWTC);
			mutex_unlock(&image_lock);
			wait_for_completion(&be_wlet_snr_strip_iwtc_cmd);
		} while (!bayer_wlet_isr_iwt_complete);
	}
	bayer_wlet_isr_iwt_complete = 0;
	BE_KERN_DEBUG_MSG ("!!!!!!!!!IWT Complete!!!!!!!!\n");

	return result;
}

/* ================================================================================ * */
/*=====================SG-BLOCK CALLBACK/ INT REG================================== * */
/*================================================================================= * */
void __be_sg_isr_wdma_done_ch0(void)
{
	BE_KERN_DEBUG_MSG("BAYER BE_SG_ISR_WDMA_DONE_CH0 INTERRUPT OCCURED!!!\n");
	complete(&be_sg_dma_wdma_ch0_obj);
}
void __be_sg_isr_lmc(void)
{
	unsigned int status, mask;
	BE_KERN_DEBUG_MSG("BAYER LMC INTERRUPT OCCURED!!!\n");
	complete(&be_sg_mode_cmd_lmc_obj);
	lmc_interrupt_received = 1;
	status = READ_BE_REG(be_sg_reg_base, D4_BE_SGTOP_STATUS);
	mask = (status |= CMD_LMC);
	WRITE_BE_REG(be_sg_reg_base, D4_BE_SGTOP_STATUS, mask);
}

void __be_sg_isr_lme(void)
{
	unsigned int status, mask;
	BE_KERN_DEBUG_MSG("BAYER LME INTERRUPT OCCURED!!!\n");
	complete(&be_sg_mode_cmd_lme_obj);
	status = READ_BE_REG(be_sg_reg_base, D4_BE_SGTOP_STATUS);
	mask = (status |= CMD_LME);
	WRITE_BE_REG(be_sg_reg_base, D4_BE_SGTOP_STATUS, mask);
}
void __be_sg_isr_gme(void)
{
	unsigned int status, mask;
	BE_KERN_DEBUG_MSG("BAYER GME INTERRUPT OCCURED!!!\n");
	complete(&be_sg_mode_cmd_gme_obj);
	status = READ_BE_REG(be_sg_reg_base, D4_BE_SGTOP_STATUS);
	mask = (status |= CMD_GME);
	WRITE_BE_REG(be_sg_reg_base, D4_BE_SGTOP_STATUS, mask);

}
void __be_sg_isr_r2s(void)
{
	unsigned int status, mask;
	BE_KERN_DEBUG_MSG("BAYER R2S INTERRUPT OCCURED!!!\n");
	complete(&be_sg_mode_cmd_r2s_obj);
	r2s_interrupt_received = 1;
	status = READ_BE_REG(be_sg_reg_base, D4_BE_SGTOP_STATUS);
	mask = (status |= CMD_R2S);
	WRITE_BE_REG(be_sg_reg_base, D4_BE_SGTOP_STATUS, mask);
}
void __be_sg_isr_prf(void)
{
	unsigned int status, mask;
	BE_KERN_DEBUG_MSG("BAYER PRF INTERRUPT OCCURED!!!\n");
	complete(&be_sg_mode_cmd_prf_obj);
	status = READ_BE_REG (be_sg_reg_base, D4_BE_SGTOP_STATUS);
	mask = (status |= CMD_PRF);
	WRITE_BE_REG(be_sg_reg_base, D4_BE_SGTOP_STATUS, mask);
}


void be_ksg_register_isr_clbk(unsigned int cmd)
{
	switch(cmd)
	{
	case BE_INT_LEVEL_SG_PRF:
	{
		BE_KERN_DEBUG_MSG("SG_PRF TOP ISR REGISTER\n");
		init_completion(&be_sg_mode_cmd_prf_obj);
		be_top_isr_register_clbk(BE_INT_LEVEL_SG_PRF, __be_sg_isr_prf);
	}
	break;
	case BE_INT_LEVEL_SG_R2S:
	{
		BE_KERN_DEBUG_MSG("SG_R2S TOP ISR REGISTER\n");
		init_completion(&be_sg_mode_cmd_r2s_obj);
		init_completion(&be_sg_dma_wdma_ch0_obj);

		be_top_isr_register_clbk(BE_INT_LEVEL_SG_R2S, __be_sg_isr_r2s);
		be_top_isr_register_clbk(BE_INT_LEVEL_DMA, __be_dma_isr_clbk);
		be_dma_register_isr_clbk_obj(1 << (BE_SGR2S_WDMA_CH * 3), __be_sg_isr_wdma_done_ch0);
	}
	break;
	case BE_INT_LEVEL_SG_GME:
	{
		BE_KERN_DEBUG_MSG("SG_GME TOP ISR REGISTER\n");
		init_completion(&be_sg_mode_cmd_gme_obj);
		be_top_isr_register_clbk(BE_INT_LEVEL_SG_GME, __be_sg_isr_gme);
	}
	break;
	case BE_INT_LEVEL_SG_LME:
	{
		BE_KERN_DEBUG_MSG("SG_LME TOP ISR REGISTER\n");
		init_completion(&be_sg_mode_cmd_lme_obj);
		be_top_isr_register_clbk(BE_INT_LEVEL_SG_LME, __be_sg_isr_lme);
	}
	break;
	case BE_INT_LEVEL_SG_LMC:
	{
		BE_KERN_DEBUG_MSG("SG_LMC TOP ISR REGISTER\n");
		init_completion(&be_sg_mode_cmd_lmc_obj);
		init_completion(&be_sg_dma_wdma_ch0_obj);
		be_top_isr_register_clbk(BE_INT_LEVEL_SG_LMC, __be_sg_isr_lmc);
		be_top_isr_register_clbk(BE_INT_LEVEL_DMA, __be_dma_isr_clbk);
		be_dma_register_isr_clbk_obj(1 << (BE_SGLMC_WDMA_CH * 3), __be_sg_isr_wdma_done_ch0);
	}
	break;
	default:
		break;
	}
}

BE_RESULT be_sg_wait_for_kcompletion(unsigned int cmd, int timeout)
{
	BE_RESULT result = D4_BE_SUCCESS;
	switch(cmd){
	case BE_INT_LEVEL_SG_PRF:
		BE_KERN_DEBUG_MSG("WAIT FOR SG::PRF!!!\n");
		/*Wait Until Done*/
		if (wait_for_completion_timeout(&be_sg_mode_cmd_prf_obj,
				msecs_to_jiffies(timeout)) == 0) {
			pr_err("BAYER:  be_sg_mode_cmd_prf timeout while interrupt wait\n");
			result = -D4_BE_ERR_INTR_TIME_OUT;
			goto ERROR;
		}
		result = 0;
		BE_KERN_DEBUG_MSG("be_sg_mode_cmd_prf Done\n");
	break;

	case BE_INT_LEVEL_SG_R2S:
		BE_KERN_DEBUG_MSG("WAIT FOR SG::R2S!!!\n");
		/*Wait Until Done*/
		do {
			if (wait_for_completion_timeout(&be_sg_dma_wdma_ch0_obj,
							msecs_to_jiffies(300)) == 0) {
						pr_err("BAYER:  BE_SG_CMD_R2S_WDMA0 timeout while interrupt wait\n");
						result = -D4_BE_ERR_INTR_TIME_OUT;
						goto ERROR;
					}
			} while (!r2s_interrupt_received);

		r2s_interrupt_received = 0;
		BE_KERN_DEBUG_MSG("SG-R2S Done\n");

	break;


	case BE_INT_LEVEL_SG_GME:
		BE_KERN_DEBUG_MSG("WAIT FOR SG::GME!!!\n");
		/*Wait Until Done*/
		if (wait_for_completion_timeout(&be_sg_mode_cmd_gme_obj,
				msecs_to_jiffies(timeout)) == 0) {
			pr_err("BAYER:  BE_SG_CMD_GME timeout while interrupt wait\n");
			result = -D4_BE_ERR_INTR_TIME_OUT;
			goto ERROR;
		result = 0;
		BE_KERN_DEBUG_MSG("BE_SG_CMD_GME Done!!!\n");

	}
	break;

	case BE_INT_LEVEL_SG_LME:
		BE_KERN_DEBUG_MSG("WAIT FOR SG::LME!!!\n");
		/*Wait Until Done*/
		if (wait_for_completion_timeout(&be_sg_mode_cmd_lme_obj,
				msecs_to_jiffies(timeout)) == 0) {
			pr_err("BAYER:  BE_SG_CMD_LME timeout while interrupt wait\n");
			result = -D4_BE_ERR_INTR_TIME_OUT;
			goto ERROR;
		}
		result = 0;
		BE_KERN_DEBUG_MSG("BE_SG_CMD_LME Done\n");

	break;

	case BE_INT_LEVEL_SG_LMC:
		BE_KERN_DEBUG_MSG("WAIT FOR SG::LMC!!!\n");
		do {
			if (wait_for_completion_timeout(&be_sg_dma_wdma_ch0_obj,
							msecs_to_jiffies(300)) == 0) {
						pr_err("BAYER:  BE_SG_CMD_LMC_WDMA0 timeout while interrupt wait\n");
						result = -D4_BE_ERR_INTR_TIME_OUT;
						goto ERROR;
					}
			} while (!lmc_interrupt_received);

		lmc_interrupt_received = 0;
		BE_KERN_DEBUG_MSG("SG-LMC Done\n");
		break;
	}
ERROR:
	return result;
}

/* ==================================================================================== * */
/*=====================GHOST-CALLBACK/INT REG ========================================== * */
void __be_ghost_isr_gd(void)
{
	complete(&be_ghost_cmd_gd);
	BE_KERN_DEBUG_MSG("CALLBACK INTF BE_GHOST_ISR_GD DONE\n");
}

void __be_ghost_isr_fw_pre(void)
{
	complete(&be_ghost_cmd_fw_pre_compl);
	BE_KERN_DEBUG_MSG("CALLBACK INTF BE_GHOST_ISR_FW_PRE DONE\n");
}

void __be_ghost_isr_fw_weight(void)
{
	complete(&be_ghost_cmd_fw_weight_compl);
	BE_KERN_DEBUG_MSG("CALLBACK INTF BE_GHOST_ISR_FW_WEIGHT DONE\n");
}

void __be_ghost_isr_db(void)
{
	complete(&be_ghost_deghost_blend);
	BE_KERN_DEBUG_MSG("CALLBACK INTF BE_GHOST_ISR_DB DONE\n");
}

void __be_ghost_fchk_isr_pre_gf3(void)
{
	complete(&be_fchk_mode_dth_fwd_obj);
	BE_KERN_DEBUG_MSG("FCHK BE_FCHK_MODE_DTH_FWD_OBJ RECEIVED\n");

}

void __be_ghost_fchk_isr_gf3(void)
{
	complete(&be_fchk_mode_dth_rev_obj);
	BE_KERN_DEBUG_MSG("CALLBACK INTF BE_INT_LEVEL_FC_DT_GF3 DONE\n");

}

void __be_ghost_fchk_isr_os_low_gf(void)
{
	complete(&be_fchk_mode_osmc_lgd_obj);
	BE_KERN_DEBUG_MSG("CALLBACK INT BE_INT_LEVEL_FC_OS_LOW_GF DONE\n");

}

void __be_ghost_fchk_isr_os_count(void)
{
	complete(&be_fchk_mode_osmc_lgc_obj);
	BE_KERN_DEBUG_MSG("CALLBACK INTF BE_INT_LEVEL_FC_OS_COUNT DONE\n");

}

void __be_ghost_fchk_isr_cc_fchk2(void)
{
	complete(&be_fchk_mode_fchk2_obj);
	BE_KERN_DEBUG_MSG("CALLBACK INTF BE_INT_LEVEL_FC_CC_FCHK2 DONE\n");
}

void __be_ghost_fchk_isr_corner_case(void)
{
//	be_fchk_mode_corner_case_flag = 1;
	complete(&be_fchk_mode_corner_case);
	BE_KERN_DEBUG_MSG("CALLBACK INT BE_INT_LEVEL_FCHK_CORNER_CASE DONE\n");

}

void __be_ghost_fchk_isr_overspec(void)
{
//	be_fchk_mode_over_spec_flag = 1;
	complete(&be_fchk_mode_over_spec);
	BE_KERN_DEBUG_MSG("CALLBACK INT BE_INT_LEVEL_FCHK_OVERSPEC DONE\n");

}

void be_kghost_fchk_register_isr_clbk(unsigned int cmd)
{
	switch (cmd) {
	case BE_INT_LEVEL_GD_GD_DIFF_GF:
		BE_KERN_DEBUG_MSG("be_kghost_fchk_register_isr_clbk::BE_INT_LEVEL_GD_GD_DIFF_GF RECEIVED!!!\n");
		init_completion(&be_ghost_cmd_gd);
		be_top_isr_register_clbk(BE_INT_LEVEL_GD_GD_DIFF_GF,
				__be_ghost_isr_gd);
	break;
	case BE_INT_LEVEL_GD_FW_PRE_LVR:
		BE_KERN_DEBUG_MSG("be_kghost_fchk_register_isr_clbk::BE_INT_LEVEL_GD_FW_PRE_LVR RECEIVED!!!\n");
		init_completion(&be_ghost_cmd_fw_pre_compl);
		be_top_isr_register_clbk(BE_INT_LEVEL_GD_FW_PRE_LVR,
				__be_ghost_isr_fw_pre);
	break;
	case BE_INT_LEVEL_GD_FW_WEIGHT:
		BE_KERN_DEBUG_MSG("be_kghost_fchk_register_isr_clbk::BE_INT_LEVEL_GD_FW_WEIGHT RECEIVED!!!\n");
		init_completion(&be_ghost_cmd_fw_weight_compl);
		be_top_isr_register_clbk(BE_INT_LEVEL_GD_FW_WEIGHT,
				__be_ghost_isr_fw_weight);
	break;
	case BE_INT_LEVEL_GD_DB_BLEND:
		BE_KERN_DEBUG_MSG("be_kghost_fchk_register_isr_clbk::BE_INT_LEVEL_GD_DB_BLEND RECEIVED!!!\n");
		init_completion(&be_ghost_deghost_blend);
		be_top_isr_register_clbk(BE_INT_LEVEL_GD_DB_BLEND,
				__be_ghost_isr_db);
	break;
	case BE_INT_LEVEL_FC_DT_PRE_GF3: {
		BE_KERN_DEBUG_MSG("be_kghost_fchk_register_isr_clbk::BE_INT_LEVEL_FC_DT_PRE_GF3 RECEIVED!!!\n");
		init_completion(&be_fchk_mode_dth_fwd_obj);
		be_top_isr_register_clbk(BE_INT_LEVEL_FC_DT_PRE_GF3,
				__be_ghost_fchk_isr_pre_gf3);
	}
	break;
	case BE_INT_LEVEL_FC_DT_GF3: {
		BE_KERN_DEBUG_MSG("be_kghost_fchk_register_isr_clbk::BE_INT_LEVEL_FC_DT_GF3 RECEIVED!!!\n");
		init_completion(&be_fchk_mode_dth_rev_obj);
		be_top_isr_register_clbk(BE_INT_LEVEL_FC_DT_GF3,
				__be_ghost_fchk_isr_gf3);
	}
	break;
	case BE_INT_LEVEL_FC_OS_LOW_GF: {
		BE_KERN_DEBUG_MSG("be_kghost_fchk_register_isr_clbk::BE_INT_LEVEL_FC_OS_LOW_GF RECEIVED!!!\n");

		init_completion(&be_fchk_mode_osmc_lgd_obj);
		init_completion(&be_fchk_mode_corner_case);
		init_completion(&be_fchk_mode_over_spec);
		be_top_isr_register_clbk(BE_INT_LEVEL_FC_OS_LOW_GF,
				__be_ghost_fchk_isr_os_low_gf);
		be_top_isr_register_clbk(BE_INT_LEVEL_FCHK_CORNER_CASE,
				__be_ghost_fchk_isr_corner_case);
		be_top_isr_register_clbk(BE_INT_LEVEL_FCHK_OVERSPEC,
				__be_ghost_fchk_isr_overspec);
		
	}
	break;
	case BE_INT_LEVEL_FC_OS_COUNT: {
		BE_KERN_DEBUG_MSG("be_kghost_fchk_register_isr_clbk::BE_INT_LEVEL_FC_OS_COUNT RECEIVED!!!\n");
		init_completion(&be_fchk_mode_osmc_lgc_obj);
		init_completion(&be_fchk_mode_corner_case);
		init_completion(&be_fchk_mode_over_spec);
		be_top_isr_register_clbk(BE_INT_LEVEL_FC_OS_COUNT,
				__be_ghost_fchk_isr_os_count);
		be_top_isr_register_clbk(BE_INT_LEVEL_FCHK_CORNER_CASE,
				__be_ghost_fchk_isr_corner_case);
		be_top_isr_register_clbk(BE_INT_LEVEL_FCHK_OVERSPEC,
				__be_ghost_fchk_isr_overspec);
	}
	break;
	case BE_INT_LEVEL_FC_CC_FCHK2: {
		BE_KERN_DEBUG_MSG("be_kghost_fchk_register_isr_clbk::BE_INT_LEVEL_FC_CC_FCHK2 RECEIVED!!!\n");
		init_completion(&be_fchk_mode_fchk2_obj);
		
		be_top_isr_register_clbk(BE_INT_LEVEL_FC_CC_FCHK2,
				__be_ghost_fchk_isr_cc_fchk2);
	}
	break;
	default:
	break;
	}
}


BE_RESULT be_kghost_kfchk_wait_for_result(unsigned int cmd, int timeout)
{
	BE_RESULT result = D4_BE_SUCCESS;
	/*Wait Until Done*/
	switch (cmd) {
	case BE_INT_LEVEL_GD_GD_DIFF_GF: {
		/*Wait Until Done*/
		if (wait_for_completion_timeout(&be_ghost_cmd_gd,
				msecs_to_jiffies(timeout)) == 0) {
			pr_err(
					"BAYER:  be_fchk_mode_dth_fwd_obj timeout while interrupt wait\n");
			result = -D4_BE_ERR_INTR_TIME_OUT;
			goto ERROR;
		}
	}
		break;
	case BE_INT_LEVEL_GD_FW_PRE_LVR: {
		/*Wait Until Done*/
		if (wait_for_completion_timeout(&be_ghost_cmd_fw_pre_compl,
				msecs_to_jiffies(timeout)) == 0) {
			pr_err(
					"BAYER:  be_fchk_mode_dth_rev_obj timeout while interrupt wait\n");
			result = -D4_BE_ERR_INTR_TIME_OUT;
			goto ERROR;
		}
	}
		break;
	case BE_INT_LEVEL_GD_FW_WEIGHT: {
		/*Wait Until Done*/
		if (wait_for_completion_timeout(&be_ghost_cmd_fw_weight_compl,
				msecs_to_jiffies(timeout)) == 0) {
			pr_err(
					"BAYER:  be_fchk_mode_osmc_lgd_obj timeout while interrupt wait\n");
			result = -D4_BE_ERR_INTR_TIME_OUT;
			goto ERROR;
		}
	}
		break;
	case BE_INT_LEVEL_GD_DB_BLEND: {
		/*Wait Until Done*/
		if (wait_for_completion_timeout(&be_ghost_deghost_blend,
				msecs_to_jiffies(timeout)) == 0) {
			pr_err(
					"BAYER:  be_fchk_mode_osmc_lgc_obj timeout while interrupt wait\n");
			result = -D4_BE_ERR_INTR_TIME_OUT;
			goto ERROR;
		}
	}
		break;
	case BE_INT_LEVEL_FC_DT_PRE_GF3: {
		/*Wait Until Done*/
		if (wait_for_completion_timeout(&be_fchk_mode_dth_fwd_obj,
				msecs_to_jiffies(timeout)) == 0) {
			pr_err(
					"BAYER:  be_fchk_mode_dth_fwd_obj timeout while interrupt wait\n");
			result = -D4_BE_ERR_INTR_TIME_OUT;
			goto ERROR;
		}
	}
		break;
	case BE_INT_LEVEL_FC_DT_GF3: {
		/*Wait Until Done*/
		if (wait_for_completion_timeout(&be_fchk_mode_dth_rev_obj,
				msecs_to_jiffies(timeout)) == 0) {
			pr_err(
					"BAYER:  be_fchk_mode_dth_rev_obj timeout while interrupt wait\n");
			result = -D4_BE_ERR_INTR_TIME_OUT;
			goto ERROR;
		}
	}
		break;

	/*Fail Check LGD*/	
	case BE_INT_LEVEL_FC_OS_LOW_GF: {
		/*Wait Until Done*/
		if (wait_for_completion_timeout(&be_fchk_mode_osmc_lgd_obj,
				msecs_to_jiffies(timeout)) == 0) {
			pr_err(
					"BAYER:  be_fchk_mode_osmc_lgd_obj timeout while interrupt wait\n");
			result = -D4_BE_ERR_INTR_TIME_OUT;
			goto ERROR;
		}


		if (wait_for_completion_timeout(&be_fchk_mode_corner_case,
				msecs_to_jiffies(timeout)) == 0) {
			pr_err(
					"BAYER:  be_fchk_mode_osmc_lgd_obj timeout while interrupt wait\n");
			result = D4_BE_SUCCESS;
		}
		else {
			result = -D4_BE_AMHDR_FAILCHECK_DETECT;
			goto ERROR;
			}

		if (wait_for_completion_timeout(&be_fchk_mode_over_spec,
				msecs_to_jiffies(timeout)) == 0) {
			pr_err(
					"BAYER:  be_fchk_mode_osmc_lgd_obj timeout while interrupt wait\n");
			result = D4_BE_SUCCESS;
		}
		else {
			result = -D4_BE_AMHDR_FAILCHECK_DETECT;
			goto ERROR;
			}

		/*if((be_fchk_mode_corner_case_flag) ||(be_fchk_mode_over_spec_flag))	{
			result = D4_BE_AMHDR_FAILCHECK_DETECT;
			goto ERROR;
			}
		else {
			complete(&be_fchk_mode_corner_case);
			complete(&be_fchk_mode_over_spec);
			result = 0;
			}*/
	}
		break;
	/*Fail Check LGC*/
	case BE_INT_LEVEL_FC_OS_COUNT: {
		/*Wait Until Done*/
		if (wait_for_completion_timeout(&be_fchk_mode_osmc_lgc_obj,
				msecs_to_jiffies(timeout)) == 0) {
			pr_err(
					"BAYER:  be_fchk_mode_osmc_lgc_obj timeout while interrupt wait\n");
			result = -D4_BE_ERR_INTR_TIME_OUT;
			goto ERROR;
		}

		if (wait_for_completion_timeout(&be_fchk_mode_corner_case,
				msecs_to_jiffies(timeout)) == 0) {
			pr_err(
					"BAYER:  be_fchk_mode_osmc_lgd_obj timeout while interrupt wait\n");
			result = D4_BE_SUCCESS;
		}
		else {
			result = D4_BE_AMHDR_FAILCHECK_DETECT;
			goto ERROR;
			}

		if (wait_for_completion_timeout(&be_fchk_mode_over_spec,
				msecs_to_jiffies(timeout)) == 0) {
			pr_err(
					"BAYER:  be_fchk_mode_osmc_lgd_obj timeout while interrupt wait\n");
		result = D4_BE_SUCCESS;
		}
		else {
			result = -D4_BE_AMHDR_FAILCHECK_DETECT;
			goto ERROR;
			}

		/*
		if((be_fchk_mode_corner_case_flag) ||(be_fchk_mode_over_spec_flag ))	{
			result = D4_BE_AMHDR_FAILCHECK_DETECT;
			goto ERROR;
			}
		else {
			complete(&be_fchk_mode_over_spec);
			complete(&be_fchk_mode_corner_case);
			result = 0;
			}
		*/
		
	}
		break;
	/*Fail Check2*/
	case BE_INT_LEVEL_FC_CC_FCHK2: {
		/*Wait Until Done*/
		if (wait_for_completion_timeout(&be_fchk_mode_fchk2_obj,
				msecs_to_jiffies(timeout)) == 0) {
			pr_err(
					"BAYER:  be_fchk_mode_fchk2_obj timeout while interrupt wait\n");
			result = -D4_BE_ERR_INTR_TIME_OUT;
			goto ERROR;
		}
	}
	break;
	default:
		break;
	}
	result = 0;
ERROR:
	return result;
}
/* ================================================================================= * */
/* 						YCC BLEND INT REGISTRATION/WAITING							* */
/*================================================================================= * */
void __be_blend_isr_lut(void)
{
	unsigned int mask;
	mask = READ_BE_REG (be_blend_reg_base, D4_BE_BLD_STATUS);
	mask |= CMD_LUT;
	WRITE_BE_REG(be_blend_reg_base, D4_BE_BLD_STATUS, mask);
	complete(&be_blend_cmd_lut);
	BE_KERN_DEBUG_MSG("be_blend_isr_lut\n");
}

void __be_blend_isr_max(void)
{
	unsigned int mask;
	mask = READ_BE_REG (be_blend_reg_base, D4_BE_BLD_STATUS);
	mask |= CMD_MAX;
	WRITE_BE_REG(be_blend_reg_base, D4_BE_BLD_STATUS, mask);
	complete(&be_blend_cmd_max);
	BE_KERN_DEBUG_MSG("be_blend_isr_max\n");
}

void __be_blend_isr_blend(void)
{
	unsigned int mask;
	mask = READ_BE_REG (be_blend_reg_base, D4_BE_BLD_STATUS);
	mask |= CMD_BLD;
	WRITE_BE_REG(be_blend_reg_base, D4_BE_BLD_STATUS, mask);
	complete(&be_blend_cmd_bld);
	be_ycc_blend = 1;
	BE_KERN_DEBUG_MSG("be_blend_isr_blend\n");
}


void __be_blend_isr_wdma_done_ch2(void)
{
	complete(&be_blend_wdma_ch2);
	BE_KERN_DEBUG_MSG("be_blend_isr_wdma_done_ch1\n");
}

void be_kblend_register_isr_clbk(unsigned int cmd)
{
	unsigned int mask = READ_BE_REG(be_blend_reg_base, D4_BE_BLD_STATUS);
	mask &= ~cmd;/*ENABLE*/
	WRITE_BE_REG(be_blend_reg_base, D4_BE_BLD_INT_MASK, mask);

	switch (cmd) {
	case BE_INT_LEVEL_BLD_LUT: {
		init_completion(&be_blend_cmd_lut);
		be_top_isr_register_clbk(BE_INT_LEVEL_BLD_LUT, __be_blend_isr_lut);
	}
	break;
	case BE_INT_LEVEL_BLD_MAX: {
		init_completion(&be_blend_cmd_max);
		be_top_isr_register_clbk(BE_INT_LEVEL_BLD_MAX, __be_blend_isr_max);
	}
	break;

	case BE_INT_LEVEL_BLD_BLD: {
		init_completion(&be_blend_cmd_bld);
		init_completion(&be_blend_wdma_ch2);
		be_top_isr_register_clbk(BE_INT_LEVEL_BLD_BLD, __be_blend_isr_blend);
		be_top_isr_register_clbk(BE_INT_LEVEL_DMA, __be_dma_isr_clbk);
		be_dma_register_isr_clbk_obj(BE_DMA_INT_LEVEL_WDMA2_INTBUSEND,
				__be_blend_isr_wdma_done_ch2);
	}
	break;
	}
}

BE_RESULT be_kblend_wait_for_completion(unsigned int cmd, int timeout)
{
	BE_RESULT result = D4_BE_SUCCESS;
	switch (cmd) {
		case BE_INT_LEVEL_BLD_LUT:
		{
		/*Wait	Until	Done*/
		if (wait_for_completion_timeout(&be_blend_cmd_lut,
				msecs_to_jiffies(timeout)) == 0) {
			pr_err("BAYER:	be_blend_cmd_lut	timeout	while	interrupt	wait\n");
			result = -D4_BE_ERR_INTR_TIME_OUT;
			goto ERROR;
		}
		result = 0;
		}
	break;

	case BE_INT_LEVEL_BLD_MAX:
		{
		/*Wait	Until	Done*/
		if (wait_for_completion_timeout(&be_blend_cmd_max,
				msecs_to_jiffies(timeout)) == 0) {
			pr_err("BAYER:	be_blend_cmd_max	timeout	while	interrupt	wait\n");
			result = -D4_BE_ERR_INTR_TIME_OUT;
			goto ERROR;
		}
		result = 0;
		}
	break;

	case BE_INT_LEVEL_BLD_BLD:
		{
		/*Wait	Until	Done*/
		do {
			if (wait_for_completion_timeout(&be_blend_wdma_ch2,
							msecs_to_jiffies(timeout)) == 0) {
						pr_err("BAYER:YCC BLEND	TIMEOUT	WHILE INTERRUPT WAIT!!!\n");
						result = -D4_BE_ERR_INTR_TIME_OUT;
						goto ERROR;
					}
			} while (!be_ycc_blend);

		be_ycc_blend = 0;
		BE_KERN_DEBUG_MSG("YCC BLEND Done\n");
	}
	break;
	}
	ERROR:
	return result;
}


/**
 * @brief this initialize all of be interrupt completion
 * @fn   void be_init_intr_compeletion(void)
 * @param void
 * @return void
 * @author Yunmi Lee <ym4404.lee@samsung.com>
 * @note -
 */
void be_init_intr_compeletion(void)
{
	int i;
	for(i = 0; i < BE_INT_LEVEL_MAX; i++) {
		init_completion(&be_ip_complete[i]);
	}
	for(i = 0; i < BE_DMA_INT_LEVEL_MAX; i++) {
		init_completion(&be_dma_complete[i]);
	}
}

/**
 * @brief this initialize all of ip interrupt completion
 * @fn   void be_init_ip_intr_compeletion(void)
 * @param void
 * @return void
 * @author Yunmi Lee <ym4404.lee@samsung.com>
 * @note -
 */
void be_init_ip_intr_compeletion(void)
{
	int i;
	for(i = 0; i < BE_INT_LEVEL_MAX; i++) {
		init_completion(&be_ip_complete[i]);
	}
}

/**
 * @brief this initialize all of dma interrupt completion
 * @fn   void be_init_dma_intr_compeletion(void)
 * @param void
 * @return void
 * @author Yunmi Lee <ym4404.lee@samsung.com>
 * @note -
 */
void be_init_dma_intr_compeletion(void)
{
	int i;
	for(i = 0; i < BE_DMA_INT_LEVEL_MAX; i++) {
		init_completion(&be_dma_complete[i]);
	}
}

/**
 * @brief this initialize an ip interrupt completion
 * @fn   void be_init_an_ip_intr_compeletion(enum be_interrupt_level intr)
 * @param enum be_interrupt_level intr
 * @return void
 * @author Yunmi Lee <ym4404.lee@samsung.com>
 * @note -
 */
void be_init_an_ip_intr_compeletion(enum be_interrupt_level intr)
{
		init_completion(&be_ip_complete[intr]);
}

/**
 * @brief this initialize an dma interrupt completion
 * @fn   void be_init_an_dma_intr_compeletion(enum be_dma_interrupt_level intr)
 * @param enum be_dma_interrupt_level intr
 * @return void
 * @author Yunmi Lee <ym4404.lee@samsung.com>
 * @note -
 */
void be_init_a_dma_intr_compeletion(enum be_dma_interrupt_level intr)
{
		init_completion(&be_dma_complete[intr]);
}

/**
 * @brief this wait for end of an ip interrupt
 * @fn   void be_wait_core_intr_timeout(enum be_interrupt_level intr, unsigned long timeout)
 * @param enum be_interrupt_level intr
 * @param unsigned long timeout (msec)
 * @return BE_RESULT
 * @author Yunmi Lee <ym4404.lee@samsung.com>
 * @note -
 */
BE_RESULT be_wait_core_intr_timeout(enum be_interrupt_level intr, unsigned long timeout)
{
	unsigned long ret;
	ret = wait_for_completion_timeout(&be_ip_complete[intr],
			msecs_to_jiffies(timeout));
	if (ret == 0) {
		printk("[%s, %d] time out!\n", __func__, __LINE__);
		return D4_BE_ERR_INTR_TIME_OUT;
	}
	return D4_BE_SUCCESS;
}

/**
 * @brief this wait for end of an dma interrupt
 * @fn   void be_wait_dma_intr_timeout(enum be_dma_interrupt_level intr, unsigned long timeout)
 * @param enum be_dma_interrupt_level intr
 * @param unsigned long timeout (msec)
 * @return BE_RESULT
 * @author Yunmi Lee <ym4404.lee@samsung.com>
 * @note -
 */
BE_RESULT be_wait_dma_intr_timeout(enum be_dma_interrupt_level intr, unsigned long timeout)
{
	unsigned long ret;
	ret = wait_for_completion_timeout(&be_dma_complete[intr],
			msecs_to_jiffies(timeout));
	if (ret == 0) {
		printk("[%s, %d] time out!\n", __func__, __LINE__);
		return D4_BE_ERR_INTR_TIME_OUT;
	}
	return D4_BE_SUCCESS;
}


/**
 * @brief internal ip interrupt handler
 * @fn  static void be_internal_interrupt_callback(enum be_interrupt_level intr)
 * @param enum be_interrupt_level intr : ip interrupt level
 * @return void
 * @author Yunmi Lee <ym4404.lee@samsung.com>
 * @note -
 */
static void be_internal_interrupt_callback(enum be_interrupt_level intr)
{
	switch(intr) {
	case BE_INT_LEVEL_FME :
/*	case BE_INT_LEVEL_3DME_EVERY_FIN :*/
	case BE_INT_LEVEL_3DME_ALL_FIN :
		complete(&be_ip_complete[intr]);
		break;
	default :
		/*never reached*/
		break;
	}
}

/**
 * @brief internal interrupt handler
 * @fn   static void be_wakeup_core_intr(enum be_interrupt_level intr)
 * @param enum be_interrupt_level intr
 * @return void
 * @author Yunmi Lee <ym4404.lee@samsung.com>
 * @note -
 */
void be_wakeup_core_intr(enum be_interrupt_level intr)
{
	BE_KERN_DEBUG_MSG("[%s, %d] isr interrupt type %d\n", __func__, __LINE__, intr);
	if (intr != BE_INT_LEVEL_DMA) {
		be_internal_interrupt_callback(intr);
	}
}

/** !oselete
 * @brief internal dma interrupt handler
 * @fn   static void be_wakeup_dma_intr(enum be_dma_interrupt_level intr)
 * @param enum be_dma_interrupt_level intr : dma interrupt level
 * @return void
 * @author Yunmi Lee <ym4404.lee@samsung.com>
 * @note -
 */
#if 0
static void be_wakeup_dma_intr(enum be_dma_interrupt_level intr)
{

	switch (intr) {
	case BE_DMA_INT_LEVEL_WDMA0_INTBUSEND :
	case BE_DMA_INT_LEVEL_WDMA2_INTBUSEND :
	case BE_DMA_INT_LEVEL_WDMA0_INTINPUTEND :
	case BE_DMA_INT_LEVEL_WDMA0_INTERROR :
	case BE_DMA_INT_LEVEL_WDMA1_INTINPUTEND :
	case BE_DMA_INT_LEVEL_WDMA1_INTBUSEND :
	case BE_DMA_INT_LEVEL_WDMA1_INTERROR :
	case BE_DMA_INT_LEVEL_WDMA2_INTINPUTEND :
	case BE_DMA_INT_LEVEL_WDMA2_INTERROR :
		break;
	default :
		/*never reached*/
		break;
	}
}
#endif

static void be_top_rmu(void)
{
	int i;
	__raw_writel(0x00000080, (be_dma_reg_base + 0x544));
	__raw_writel(0x80000000, (be_dma_reg_base + 0x53C));
	__raw_writel(0x00000000, (be_dma_reg_base + 0x554));
	__raw_writel(0x00010000, (be_dma_reg_base + 0x548));

	/**< Wait for Error Interrupt */
	for (i = 0; i < 100; i++) {
		udelay(50);
	}
}

static void be_top_dma_sw_reset(void)
{
	/**< DMA Reset & Interrupt Clear */
	__raw_writel(0xFFFFFFFF, (be_top_reg_base + 0x0C));
	__raw_writel(0x0, (be_top_reg_base + 0x0C));
	__raw_writel(0xFFFFFFFF, (be_top_reg_base + 0x14));
}

/*
static void be_top_reset(void)
{
	__raw_writel(0x11111111, (be_top_reg_base + 0x0C));
}
*/

int be_pmu_requeset(void)
{
	int ret = 0;
#ifdef CONFIG_PMU_SELECT
	/* DMA Start */
	__raw_writel(0x1, (be_dma_reg_base + 0x55C));
	ret = pmu_wait_for_ack(PMU_BAYER);
	if(ret != 0) {
		return ret;
	}
#endif
	return ret;
}

void be_pmu_clear(void)
{
	__raw_writel(0x0, (be_dma_reg_base + 0x55C));
}

/**
 * @brief	BE??PMU(Power Management Unit) ?¤ì •???˜ëŠ” ?¨ìˆ˜
 * @fn      void be_pmu_on_off(enum be_pmu_type pmu_type)
 * @param	pmu_type	[in] On/Off
 * @return	void
 *
 * @author	Yunmi Lee
 * @note	kernel layer
 */
void be_pmu_on_off(enum be_pmu_type pmu_type)
{
#ifdef CONFIG_PMU_SELECT
	int reval;
	struct clk *clock;
	struct d4_rmu_device *rmu;
	if (pmu_type == BE_PMU_ON) {
		reval = d4_pmu_check(PMU_BAYER);
		if (reval != 0) {
			reval = be_pmu_requeset();
			if (reval)
				return;

			rmu = d4_rmu_request();
			if (rmu == NULL)
				return;
			d4_pmu_isoen_set(PMU_BAYER, PMU_CTRL_ON);
			d4_sw_isp_reset(rmu, RST_BE);

			clock = clk_get(be_dev, "be");

			if (clock == -2) {
				if (rmu != NULL)
					kfree(rmu);
				return;
			}

			clk_disable(clock);
			clk_put(clock);

			d4_pmu_scpre_set(PMU_BAYER, PMU_CTRL_ON);
			d4_pmu_scall_set(PMU_BAYER, PMU_CTRL_ON);
			d4_rmu_release(rmu);
		}
	} else {
		rmu = d4_rmu_request();

		if (rmu == NULL)
			return;

		d4_pmu_scpre_set(PMU_BAYER, PMU_CTRL_OFF);
		d4_pmu_scall_set(PMU_BAYER, PMU_CTRL_OFF);
		wait_for_stable(PMU_BAYER);
		clock = clk_get(be_dev, "be");

		if (clock == -2) {
			if (rmu != NULL)
				kfree(rmu);
			return;
		}

		d4_sw_isp_reset(rmu, RST_BE);
		udelay(1);

		clk_enable(clock);
		clk_set_rate(clock, 200000000);
		d4_pmu_isoen_set(PMU_BAYER, PMU_CTRL_OFF);

		udelay(1);
		d4_sw_isp_reset_release(rmu, RST_BE);
		udelay(1);

		/*2~4*/
		d4_pmu_bus_reset(PMU_BAYER);
		be_top_rmu();


		/*rmu reset*/
		d4_sw_isp_reset(rmu, RST_BE);
		udelay(1);
		d4_sw_isp_reset_release(rmu, RST_BE);

		/*2~4*/
		d4_pmu_bus_reset(PMU_BAYER);
		be_top_rmu();

		be_top_dma_sw_reset();

		d4_pmu_bus_reset(PMU_BAYER);
		d4_rmu_release(rmu);
/*
		be_top_reset();
*/
	}
#endif
}

/* temporary test function */
void be_3dme_ktest(void)
{
	int r1, r2, r3, r4;

	/*base reset*/
	__raw_writel(0x0, be_top_reg_base);
	/*clock enable*/
	__raw_writel(0x100000, be_top_reg_base + 0x4);
	__raw_writel(0x100000, be_top_reg_base + 0x8);

	be_enable_irq();

	/*dma & sdram mode*/
	__raw_writel(0x100000, be_top_reg_base + 0x30);
	__raw_writel(0x100000, be_top_reg_base + 0x34);

	/*interrupt enalbe*/
	__raw_writel(0x1000000, be_top_reg_base + 0x10);

	/*sg dma setting*/
	__raw_writel(0x3, be_top_reg_base + 0xa0);
	__raw_writel(0x3, be_top_reg_base + 0xb0);

	/*3dme setting*/
	__raw_writel(0x1515, be_3dme_reg_base);
	__raw_writel(0x280a, be_3dme_reg_base + 0x4);
	__raw_writel(0x4, be_3dme_reg_base + 0x8);

	__raw_writel(0x6400c8, be_3dme_reg_base + 0x400);
	__raw_writel(0xc80064, be_3dme_reg_base + 0x404);
	__raw_writel(0xfa0046, be_3dme_reg_base + 0x408);
	__raw_writel(0x59007D, be_3dme_reg_base + 0x40c);

	/*dma setting - rdma2*/
	//dma reset??
	__raw_writel(0x1, be_dma_reg_base + 0x100);
	__raw_writel(0xc4000000, be_dma_reg_base + 0x10c);
	__raw_writel(0xc40003c0, be_dma_reg_base + 0x110);
	__raw_writel(0x3c0, be_dma_reg_base + 0x114);
	/*ch2, 3 disable*/
	__raw_writel(0x0, be_dma_reg_base + 0x140);

	/*dma setting - rdma3*/
	//dma reset??
	__raw_writel(0x1, be_dma_reg_base + 0x180);
	__raw_writel(0xc4480000, be_dma_reg_base + 0x18c);
	__raw_writel(0xc44803c0, be_dma_reg_base + 0x190);
	__raw_writel(0x3c0, be_dma_reg_base + 0x194);
	/*ch2, 3 disable*/
	__raw_writel(0x0, be_dma_reg_base + 0x1c0);

	/*start*/
	__raw_writel(0x1, be_3dme_reg_base + 0xc);

	r1 = __raw_readl(be_3dme_reg_base + 0x6c00);
	r2 = __raw_readl(be_3dme_reg_base + 0x6c04);
	r3 = __raw_readl(be_3dme_reg_base + 0x6c08);
	r4 = __raw_readl(be_3dme_reg_base + 0x6c0c);

	printk("r1 = %x, r2 = %x, r3 = %x, r4 = %x\n", r1, r2, r3, r4);
}


MODULE_AUTHOR("Niladri Mukherjee <n.mukherjee@samsung.com>");
MODULE_DESCRIPTION("Samsung Drime IV BE Driver");
MODULE_LICENSE("GPL");

