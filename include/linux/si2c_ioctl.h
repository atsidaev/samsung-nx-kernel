
#ifndef __LINUX_SI2CDEV_H
#define __LINUX_SI2CDEV_H


#include <linux/types.h>
#include <linux/d4_si2c_config.h>

#define SI2C_IOC_MAGIC  's'




#define SI2C_SIOCTL_SET							_IOW(SI2C_IOC_MAGIC, 1, unsigned int)
#define SI2C_ACK_WAIT								_IOW(SI2C_IOC_MAGIC, 2, int)
#define SI2C_RECIVE_ADD_CHECK				_IOR(SI2C_IOC_MAGIC, 3, unsigned int)


#define SI2C_BYTE_READ								_IOR(SI2C_IOC_MAGIC, 4, unsigned int)
#define SI2C_RECIVE_STOP						_IO(SI2C_IOC_MAGIC, 5)



#define SI2C_TRANSMIT_ADD_CHECK		_IOR(SI2C_IOC_MAGIC, 6, unsigned int)


#define SI2C_BYTE_WRITE							_IOR(SI2C_IOC_MAGIC, 7, unsigned int)
#define SI2C_LAST_WRITE							_IOR(SI2C_IOC_MAGIC, 8, unsigned int)
#define SI2C_TRANSMIT_STOP					_IO(SI2C_IOC_MAGIC, 9)

#define SI2C_RECIVE_SLAVE						_IOWR(SI2C_IOC_MAGIC, 10, struct d4_si2c_config)
#define SI2C_TRANSMIT_SLAVE					_IOWR(SI2C_IOC_MAGIC, 11, struct d4_si2c_config)
#endif
