#ifndef PROTOCOL_IOCTL_H_
#define PROTOCOL_IOCTL_H_

#include <linux/types.h>
#include <linux/spi/d4_spi_config.h>

#define POTOCOL_IOC_MAGIC  'p'

/*Collision setting */
#define PROTOCOL_IOCTL_SET							_IOW(POTOCOL_IOC_MAGIC, 1, struct d4_hs_spi_config)
#define PROTOCOL_IOCTL_INT_SREAD				_IOWR(POTOCOL_IOC_MAGIC, 2, struct spi_data_info)
#define PROTOCOL_IOCTL_INT_SWRITE			_IOWR(POTOCOL_IOC_MAGIC, 3, struct spi_data_info)
#define PROTOCOL_IOCTL_INT_DONE				_IO(POTOCOL_IOC_MAGIC, 4)
#define PROTOCOL_IOCTL_RB_CTRL					_IOR(POTOCOL_IOC_MAGIC, 5, unsigned int)
#define PROTOCOL_IOCTL_POWER_OFF				_IO(POTOCOL_IOC_MAGIC, 6)
#define PROTOCOL_IOCTL_INT_FW_READ			_IOWR(POTOCOL_IOC_MAGIC, 7, struct spi_data_info)

#define PROTOCOL_IOCTL_INT_NOTI_WRITE		_IOWR(POTOCOL_IOC_MAGIC, 8, struct spi_data_info)

#endif /*PROTOCOL_IOCTL_H_*/


