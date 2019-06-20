/*-------------------------------------------------------------
 * Filename: d4dp_gpu_if.h
 *
 * Contents: Implemention of display kernel module bridging to LCD driver
 *
 * Abbreviations:
 *
 * Person Involved: Byungho Ahn
 *
 * Notes: 
 *
 * History
 *  - First created by chun gil lee, 20090205
 *
 * Copyright (c) 2009 SAMSUNG Electronics.
 --------------------------------------------------------------*/

#ifndef __D4DP_LCD_H__
#define __D4DP_LCD_H__

#if 0
int D4dpLcdBridgeInit(struct device *gpu_dev);
void D4dpLcdBridgeExit(struct device *gpu_dev);
#endif

int D4dpLcdBridgeInit(void);
void D4dpLcdBridgeExit(void);

#endif
