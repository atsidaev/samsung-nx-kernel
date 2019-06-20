
#ifndef __LINUX_GPIO_IO_H
#define __LINUX_GPIO_IO_H

#include <linux/types.h>



#define GPIO_IOC_MAGIC  'g'


struct gpio_get_reg_info {
	unsigned int phys_addr;
	unsigned int phys_size;
};


struct gpio_int_info {
	unsigned int group;
	unsigned int pinnum;
	unsigned int enable;
};

#define GPIO_PHY_INFO			_IOR(GPIO_IOC_MAGIC, 1, struct gpio_get_reg_info)
#define GPIO_INT_SET				_IOW(GPIO_IOC_MAGIC, 2, struct gpio_int_info)
#define GPIO_INT_SELECT		_IOW(GPIO_IOC_MAGIC, 3, struct gpio_int_info)
#define GPIO_INT_TYPE			_IOW(GPIO_IOC_MAGIC, 4, struct gpio_int_info)
#define GPIO_PAD_SET				_IOW(GPIO_IOC_MAGIC, 5, unsigned int)
#define GPIO_PAD_FREE			_IOW(GPIO_IOC_MAGIC, 6, unsigned int)


#endif
