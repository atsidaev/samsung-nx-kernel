#ifndef _RTC_SYSFS_H_
#define _RTC_SYSFS_H_

int rtc_create_node(struct device *dev);
int rtc_remove_node(struct device *dev);

#endif /*_RTC_SYSFS_H_*/
