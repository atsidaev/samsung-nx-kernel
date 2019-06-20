#ifndef _D4_ADC_IOCTL_H_
#define _D4_ADC_IOCTL_H_

#define D4ADC_IOCTL_DEV_NAME "d4_adc_ioctl"

#define D4_ADC_IOCTL_MAGIC 't'

#define D4_BAT_INIT 			_IO(D4_ADC_IOCTL_MAGIC, 0)
#define D4_BAT_GET_VALUE 	_IOR(D4_ADC_IOCTL_MAGIC, 1, int )

#define D4_ADC_IOCTL_MAX 2

#endif

