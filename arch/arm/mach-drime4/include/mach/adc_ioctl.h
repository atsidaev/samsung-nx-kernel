
#ifndef __LINUX_ADCDEV_H
#define __LINUX_ADCDEV_H


#include <linux/types.h>

#define ADC_IOC_MAGIC  'c'

struct adc_value {
	unsigned int channel;
	unsigned int read_value;
};

#define ADC_IOCTL_GET_VALUE				_IOWR(ADC_IOC_MAGIC, 1, struct adc_value)

#endif
