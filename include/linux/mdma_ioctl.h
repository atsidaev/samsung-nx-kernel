
#ifndef __LINUX_MDMA_IO_H
#define __LINUX_MDMA_IO_H

#include <linux/types.h>
#include <linux/mdma_type.h>

#define MDMA_IOC_MAGIC  'd'

#define MDMA_IOCTL_COPY		_IOW(MDMA_IOC_MAGIC, 1, struct mdma_set_data)

#endif
