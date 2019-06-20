
#ifndef __LINUX_HSSPI_IO_H
#define __LINUX_HSSPI_IO_H

#include <linux/types.h>
#include <mach/ptc_type.h>

#define PTC_IOC_MAGIC  't'





#define PTC_CONFIG_SET					_IOW(PTC_IOC_MAGIC, 1, struct ptc_config)
#define PTC_START								_IOW(PTC_IOC_MAGIC, 2, unsigned int)
#define PTC_STOP								_IOW(PTC_IOC_MAGIC, 3, unsigned int)
#define PTC_CNT_CLEAR					_IO(PTC_IOC_MAGIC, 4)
#endif
