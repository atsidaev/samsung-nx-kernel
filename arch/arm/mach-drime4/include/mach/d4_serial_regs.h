/**
 * arch/arm/mach-drime4/include/mach/d4_serial_regs.h
 *
 * Copyright (c) 2011 Samsung Electronics Co., Ltd.
 *              http://www.samsung.com/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __MACH_DRIME4_SERIAL_REGS_H
#define __MACH_DRIME4_SERIAL_REGS_H

#include <mach/map.h>


/*
 * Register offsets
 */
#define PL011_UARTDR      0x0
#define PL011_UARTRSR     0x4
#define PL011_UARTFR      0x18
#define PL011_UARTILPR    0x20
#define PL011_UARTIBRD    0x24
#define PL011_UARTFBRD    0x28
#define PL011_UARTLCR_H   0x2c
#define PL011_UARTTCR     0x30
#define PL011_UARTIFLS    0x34
#define PL011_UARTIMSC    0x38
#define PL011_UARTRIS     0x3c
#define PL011_UARTMIS     0x40
#define PL011_UARTICR     0x44
#define PL011_UARTDMACR   0x48
#define PL011_UARTPeriID0 0xfe0
#define PL011_UARTPeriID1 0xfe4
#define PL011_UARTPeriID2 0xfe8
#define PL011_UARTPeriID3 0xfec
#define PL011_UARTCellID0 0xff0
#define PL011_UARTCellID1 0xff4
#define PL011_UARTCellID2 0xff8
#define PL011_UARTCellID3 0xffc


#define DRIME4_VA_UARTx(uart)   (DRIME4_VA_UART + 0x1000 * (uart))


#endif  // __MACH_DRIME4_SERIAL_REGS_H

