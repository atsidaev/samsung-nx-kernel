#ifndef _D4KEYS_SYSFS_H_
#define _D4KEYS_SYSFS_H_

extern int g_boot_mode;
extern int g_boot_st1;
extern int g_boot_st2;
extern int d4keys_create_node(struct device *dev);
extern int d4keys_remove_node(struct device *dev);

#endif /*_D4KEYS_SYSFS_H_*/
