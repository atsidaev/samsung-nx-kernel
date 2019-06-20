#ifndef __ASM_ARM_SUSPEND_H
#define __ASM_ARM_SUSPEND_H

/*
 * Based on code include/asm-i386/suspend.h
 * Copyright 2001-2002 Pavel Machek <pavel@suse.cz>
 * Copyright 2001 Patrick Mochel <mochel@osdl.org>
 */


static inline int
arch_prepare_suspend(void)
{
	return 0;
}

/* image of the saved processor state */
struct saved_context {
	/*
	 * Structure saved_context would be used to hold processor state
	 * except caller and callee registers, just before suspending.
	 */

	/* coprocessor 15 registers */
//	__u32 ID_code;    /* read only reg */
//	__u32 cache_type; /* read only reg */
//	__u32 TCM_stat;   /* read only reg */
	__u32 CR;
	__u32 TTBR;
	__u32 DACR;
	__u32 D_FSR;
	__u32 I_FSR;
	__u32 FAR;
//	__u32 COR;    /*write only reg */
//	__u32 TLBOR;  /*write only reg */
	__u32 D_CLR;
	__u32 I_CLR;
	__u32 D_TCMRR;
	__u32 I_TCMRR;
	__u32 TLBLR;
	__u32 FCSE;
	__u32 CID;
};

extern void cpu_resume(void);
extern int cpu_suspend(unsigned long, int (*)(unsigned long));

#endif
