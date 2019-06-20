/*
 * d4_mem.c
 *
 *  Created on: 2011. 8. 29.
 *      Author: Junkwon Choi
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/version.h>
#include <linux/clk.h>
#include <linux/mutex.h>
#include <linux/poll.h>
#include <linux/wait.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/memory.h>
#include <linux/ctype.h>
#include <linux/slab.h>
#include <linux/dma-mapping.h>
#include <linux/device.h>
#include <linux/sched.h>

#include <asm/uaccess.h>
#include <asm/bug.h>
#include <asm/current.h>

#include <mach/d4_mem.h>

unsigned int d4_uservirt_to_phys(unsigned int virt_addr)
{
	struct mm_struct *mm = current->mm;
	unsigned long offset, phy_addr = 0, curr_pfn;
	struct vm_area_struct *vma = NULL;
	unsigned int ret = 1;

	down_read(&mm->mmap_sem);

	vma = find_vma(mm, virt_addr);
	if (!vma) {
		printk(KERN_DEBUG "%s: invalid userspace address.\n", __func__);
		up_read(&mm->mmap_sem);
		return 0;
	}

	/* for kernel direct-mapped memory. */
	if (virt_addr >= PAGE_OFFSET)
		phy_addr = virt_to_phys((void *)virt_addr);
	/* for memory mapped to dma coherent region. */
	else {
		offset = virt_addr & ~PAGE_MASK;

		/* get current page frame number to virt_addr. */
		ret = follow_pfn(vma, virt_addr, &curr_pfn);
		if (ret) {
			printk(KERN_DEBUG "%s: invalid userspace address.\n", __func__);
			up_read(&mm->mmap_sem);
			return 0;
		}

		phy_addr = (curr_pfn << PAGE_SHIFT) + offset;
	}

	up_read(&mm->mmap_sem);

	return phy_addr;
}
