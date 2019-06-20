#ifndef _DW_MMC_SYSFS_H_
#define _DW_MMC_SYSFS_H_

extern int g_media_err;
extern int g_support_card;
extern int g_protect_card;
extern u16 g_sample_phase;
extern u16 g_drive_phase;
extern int g_tunable;

extern int dw_mci_create_node(struct device *dev);
extern int dw_mci_remove_node(struct device *dev);

#endif /*_DW_MMC_SYSFS_H_*/
