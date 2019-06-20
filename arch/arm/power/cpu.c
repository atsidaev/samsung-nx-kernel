/*
 * cpu.c  - Suspend support specific for ARM.
 *             based on arch/i386/power/cpu.c
 *
 * Distribute under GPLv2
 *
 * Copyright (c) 2002 Pavel Machek <pavel@suse.cz>
 * Copyright (c) 2001 Patrick Mochel <mochel@osdl.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/spinlock.h>
#include <linux/poll.h>
#include <linux/delay.h>
#include <linux/sysrq.h>
#include <linux/proc_fs.h>
#include <linux/pm.h>
#include <linux/device.h>
#include <linux/suspend.h>
#include <asm/irq.h>
#include <asm/uaccess.h>
#include <asm/tlbflush.h>

#include <asm/suspend.h>

/* this file must move to "arch/arm/mach-s5pc110/..." */
/*extern int s5pc11x_pm_save_reg(void);*/
/*extern int s5pc11x_pm_restore_reg(void);*/
/*extern int in_suspend;*/
static struct saved_context saved_context;

void __save_processor_state(struct saved_context *ctxt)
{
	printk(KERN_NOTICE"start %s\n", __func__);
	/* save preempt state and disable it */
	preempt_disable();

	/* save coprocessor 15 registers */
	asm volatile ("mrc p15, 0, %0, c1, c0, 0" : "=r" (ctxt->CR));
	asm volatile ("mrc p15, 0, %0, c3, c0, 0" : "=r" (ctxt->DACR));
	asm volatile ("mrc p15, 0, %0, c5, c0, 0" : "=r" (ctxt->D_FSR));
	asm volatile ("mrc p15, 0, %0, c5, c0, 1" : "=r" (ctxt->I_FSR));
	asm volatile ("mrc p15, 0, %0, c6, c0, 0" : "=r" (ctxt->FAR));
	asm volatile ("mrc p15, 0, %0, c9, c0, 0" : "=r" (ctxt->D_CLR));
	asm volatile ("mrc p15, 0, %0, c9, c0, 1" : "=r" (ctxt->I_CLR));
	asm volatile ("mrc p15, 0, %0, c9, c1, 0" : "=r" (ctxt->D_TCMRR));
	asm volatile ("mrc p15, 0, %0, c9, c1, 1" : "=r" (ctxt->I_TCMRR));
	asm volatile ("mrc p15, 0, %0, c10, c0, 0" : "=r" (ctxt->TLBLR));
	asm volatile ("mrc p15, 0, %0, c13, c0, 0" : "=r" (ctxt->FCSE));
	asm volatile ("mrc p15, 0, %0, c13, c0, 1" : "=r" (ctxt->CID));
	asm volatile ("mrc p15, 0, %0, c2, c0, 0" : "=r" (ctxt->TTBR));


	printk(KERN_NOTICE"end %s\n", __func__);
}


void save_processor_state(void)
{
	printk(KERN_NOTICE"start %s\n", __func__);

/*
	if (in_suspend){
	}
*/
	__save_processor_state(&saved_context);

	printk(KERN_NOTICE"end %s\n", __func__);
}

void __restore_processor_state(struct saved_context *ctxt)
{
	printk(KERN_NOTICE"start %s\n", __func__);

	/* restore coprocessor 15 registers */
	asm volatile ("mcr p15, 0, %0, c2, c0, 0" : : "r" (ctxt->TTBR));
	asm volatile ("mcr p15, 0, %0, c13, c0, 1" : : "r" (ctxt->CID));
	asm volatile ("mcr p15, 0, %0, c13, c0, 0" : : "r" (ctxt->FCSE));
	asm volatile ("mcr p15, 0, %0, c10, c0, 0" : : "r" (ctxt->TLBLR));
	asm volatile ("mcr p15, 0, %0, c9, c1, 1" : : "r" (ctxt->I_TCMRR));
	asm volatile ("mcr p15, 0, %0, c9, c1, 0" : : "r" (ctxt->D_TCMRR));
	asm volatile ("mcr p15, 0, %0, c9, c0, 1" : : "r" (ctxt->I_CLR));
	asm volatile ("mcr p15, 0, %0, c9, c0, 0" : : "r" (ctxt->D_CLR));
	asm volatile ("mcr p15, 0, %0, c6, c0, 0" : : "r" (ctxt->FAR));
	asm volatile ("mcr p15, 0, %0, c5, c0, 1" : : "r" (ctxt->I_FSR));
	asm volatile ("mcr p15, 0, %0, c5, c0, 0" : : "r" (ctxt->D_FSR));
	asm volatile ("mcr p15, 0, %0, c3, c0, 0" : : "r" (ctxt->DACR));
	asm volatile ("mcr p15, 0, %0, c1, c0, 0" : : "r" (ctxt->CR));

	/* restore preempt state */
	preempt_enable();
	printk(KERN_NOTICE"end %s\n", __func__);
}

void restore_processor_state(void)
{
	printk(KERN_NOTICE"start %s\n", __func__);
	__restore_processor_state(&saved_context);
	printk(KERN_NOTICE"end %s\n", __func__);
}

EXPORT_SYMBOL(save_processor_state);
EXPORT_SYMBOL(restore_processor_state);

