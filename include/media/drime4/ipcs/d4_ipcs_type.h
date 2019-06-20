/**
 * @file	d4_ipcs_type.h
 * @brief	IPCS driver header for Samsung DRIMe4 Camera Interface driver
 *
 * @author	Donjin Jung <djin81.jung@samsung.com>,
 * Copyright (c) 2012 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __D4_K_IPCS_TYPE_H__
#define __D4_K_IPCS_TYPE_H__

#define K_IPCS_MAX_BUFFER 16

/* Enumeration *******************************************************/
/**
 * @enum	ipcs_k_on_off
 * @brief	K_IPCS Device Driver의 On/Off member
 */
enum ipcs_k_on_off {
	IPCS_K_OFF = 0,/**< Off */
	IPCS_K_ON/**< On */
};

/**
 * @enum	ipcs_k_interrupt
 * @brief	VDMA Interrupt select
 * @note	WDMA0, WDMA1, WDMA2, WDMA3 의 BusEnd/Error interrupt 만 check 한다.
 */
enum ipcs_k_interrupt {
	IPCS_K_INTR_BUSEND_WDMA0 = 0,/**< WMDA0의 Bus End Interrupt(Main Out)*/
	IPCS_K_INTR_ERROR_WDMA0 = 2,/**< WMDA0의 Interrupt Error(Main Out)*/
	IPCS_K_INTR_BUSEND_WDMA1 = 3,/**< WMDA1의 Bus End Interrupt(Main Resize Out)*/
	IPCS_K_INTR_ERROR_WDMA1 = 5,/**< WMDA1의 Interrupt Error(Main Resize Out)*/
	IPCS_K_INTR_BUSEND_WDMA2 = 6,/**< WMDA2의 Bus End Interrupt(Sub Resize1 Out)*/
	IPCS_K_INTR_ERROR_WDMA2 = 8,/**< WMDA2의 Interrupt Error(Sub Resize1 Out)*/
	IPCS_K_INTR_BUSEND_WDMA3 = 9,/**< WMDA3의 Bus End Interrupt(Sub Resize2 Out)*/
	IPCS_K_INTR_ERROR_WDMA3 = 11,/**< WMDA3의 Interrupt Error(Sub Resize2 Out)*/
	IPCS_K_INTR_MAX = 16
};

/* structure *******************************************************/
/**
 * @struct	ipcs_k_physical_reg_info
 * @brief	Physical register information
 */
struct ipcs_k_physical_reg_info {
	unsigned int reg_start_addr;
	unsigned int reg_size;
};

/**
 * @struct	ipcs_k_reg_ctrl_base_info
 * @brief	ipcs를 control 함에 있어 기본이 되는 device 정보
 */
struct ipcs_k_reg_ctrl_base_info {
	struct device *dev_info;
	unsigned int reg_base;
	int irq_num;

	struct ipcs_k_physical_reg_info phys_reg_info;
};

#endif   /* __D4_K_IPCS_TYPE_H__ */
