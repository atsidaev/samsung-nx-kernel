/**
 * @file d4_rmu.h
 * @brief DRIMe4 Pules Trigger Counter Interface
 * @author kyuchun han <kyuchun.han@samsung.com>
 * 2010 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifndef _D4_RMU_H
#define _D4_RMU_H

struct d4_rmu_device;
struct rmu_device;


enum sw_reset_ips {
    RST_MM      = 1,
    RST_DDR     = 2,
    RST_GPU     = 4,
    RST_PP      = 5,
    RST_IPCM    = 6,
    RST_IPCS    = 7,
    RST_EP      = 8,
    RST_BE      = 9,
    RST_DP      = 10,
    RST_JPEG    = 11,
    RST_CODEC   = 12,
    RST_MIPI    = 14,
    RST_OTF     = 15,
    RST_HDMIPHY = 16,
    RST_SLVDS   = 17,
    RST_A9CPU   = 24
};


enum sw_reset_pll {
    RST_PLL_AUD,
    RST_PLL_ARM,
    RST_PLL_LCD,
    RST_PLL_SYS1,
    RST_PLL_SYS2
};

enum hw_status_reg {
    NAND_DONE       = 0,
    RSTn_SPLL2      = 1,
    RSTn_SPLL1      = 2,
    RSTn_APLL       = 3,
    RSTn_DDR        = 4,
    RSTn_CA9        = 5,
    RSTn_MSYS       = 6,
    RSTn_PERI       = 7,
    RSTn_PWON       = 8,
    PLL_READY       = 9,
    DFTMON_SPLL2    = 10,
    DFTMON_SPLL1    = 11,
    DFTMON_LPLL     = 12,
    DFTMON_CA9      = 13,
    SYS_PCURM       = 15,
    SYS_EXMST       = 16,
    SYS_DDRSEL      = 17,
    SYS_A9XTAL      = 18,
    SYS_REMAP       = 19,
    SYS_MODE        = 21,
    PCURSTn         = 22,
    SWRSTn          = 23,
    SYS_CFG         = 24,
    WDTRSTn         = 31
};

enum pll_monitor_reg {
    DMON_ARMPLL,
    DMON_SYSPLL2,
    DMON_SYSPLL1,
    DMON_LCDPLL,
    DMON_AUDPLL
};


enum pcu_resource {
    PCU_POWERON_0,
    PCU_POWERON_1,
    PCU_POWERON_2,
    PCU_POWERON_3,
    PCU_POWERON_4
};


struct d4_rmu_device *d4_rmu_request(void);
void d4_rmu_release(struct d4_rmu_device *rmu);

int d4_pcu_resource_get(struct d4_rmu_device *rmu, enum pcu_resource power_type);
void d4_pcu_hold_set(struct d4_rmu_device *rmu);
void d4_pcu_off_set(struct d4_rmu_device *rmu);
void d4_pcu_ddroff_set(struct d4_rmu_device *rmu);
void d4_pcu_intr_mask_set(struct d4_rmu_device *rmu);

void d4_sw_pll_reset(struct d4_rmu_device *rmu, enum sw_reset_pll pll_type);
void d4_sw_isp_reset(struct d4_rmu_device *rmu, enum sw_reset_ips ip_type);
void d4_sw_isp_reset_release(struct d4_rmu_device *rmu, enum sw_reset_ips ip_type);

unsigned int d4_hw_status_read(struct d4_rmu_device *rmu, enum hw_status_reg hw_status);
unsigned int d4_pll_monitor_read(struct d4_rmu_device *rmu, enum pll_monitor_reg pll_mon);

#endif /* _D4_RMU_H */


