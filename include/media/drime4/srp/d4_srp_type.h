/**
 * @file d4_srp_type.h
 * @brief DRIMe4 SRP Structure & Enumeration Define
 * @author Geunjae Yu <geunjae.yu@samsung.com>
 * 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/
#ifndef __D4_SRP_TYPE_H__
#define __D4_SRP_TYPE_H__


/******************************************************************************/
#define SRP_SUCCESS		0
#define SRP_LOAD_OK		1
#define SRP_FAIL		(-1)
#define SRP_IRQ_FAIL		(-2)
#define SRP_TIMEOUT		(-3)
#define SRP_CLK_SET_FAIL	(-4)
#define SRP_MEM_ALLOC_FAIL	(-10)
#define SRP_MEM_FREE_FAIL	(-11)
#define SRP_MEM_NULL_POINTER	(-12)

#define	MAX_RP_GPR		15
#define	SRP_RESULT 		int



/**
 * @enum srp_reg_selection
 * @brief Register type 선택을 위한 Enumeration
 *        Register physical information 정보를 얻어 오기 위한 register 선택 시 사용
 *
 * @author
 */
enum srp_reg_selection {
	SRP_REG,
	DAP_REG,
	COMMBOX_REG,
	SSRAM_REG
};

/**
 * @enum D4_SRP_COMBOX_REG
 * @brief GPR Commbox register define
 *
 * @author Geunjae Yu
 * @note 	NONE
 */
enum D4_SRP_COMBOX_REG {
	D4_SRP_RSTN		= 0x0, 	/** 0x000 */
	D4_SRP_HALT		= 0x1,	/** 0x004 */
	D4_SRP_WAKEUP_WFE	= 0x2,	/** 0x008 */
	D4_SRP_INST_OFFSET	= 0x3,	/** 0x00C */
	D4_SRP_IRQ		= 0x4,	/** 0x010 */
	D4_SRP_IRQ0		= 0x5,	/** 0x014 */
	D4_SRP_IRQ1		= 0x6,	/** 0x018 */
	D4_SRP_IRQ2		= 0x7,	/** 0x01C */
	D4_SRP_IRQ3		= 0x8,	/** 0x020 */
	D4_SRP_STATUS		= 0x18,	/** 0x060 */
	D4_SRP_HOST_IRQ		= 0x1C,	/** 0x070 */
	D4_SRP_GPI		= 0x20,	/** 0x080 */
	D4_SRP_GPO		= 0x24,	/** 0x090 */

	D4_SRP_GPR0		= 0x40,	/** 0x100 */
	D4_SRP_GPR1		,	/** 0x104 */
	D4_SRP_GPR2		,	/** 0x108 */
	D4_SRP_GPR3		,	/** 0x10C */
	D4_SRP_GPR4		,	/** 0x110 */
	D4_SRP_GPR5		,	/** 0x114 */
	D4_SRP_GPR6		,	/** 0x118 */
	D4_SRP_GPR7		,	/** 0x11C */

	D4_SRP_GPR8		,	/** 0x120 */
	D4_SRP_GPR9		,	/** 0x124 */
	D4_SRP_GPR10 		,	/** 0x128 */
	D4_SRP_GPR11 		,	/** 0x12C */
	D4_SRP_GPR12 		,	/** 0x130 */
	D4_SRP_GPR13 		,	/** 0x134 */
	D4_SRP_GPR14 		,	/** 0x138 */
	D4_SRP_GPR15 		,	/** 0x13C */
};

/**
 * @struct srp_phys_reg_info
 * @brief Physical register information 사용을 위한 structure
 *
 * @author
 */
struct srp_phys_reg_info {
	unsigned int reg_start_addr; 	/* Register physical start address */
	unsigned int reg_size;		 /* Register size */
};



#endif   /* __D4_SRP_TYPE_H__ */
