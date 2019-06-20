
#ifndef __LINUX_HSSPI_IO_H
#define __LINUX_HSSPI_IO_H

#include <linux/types.h>
#include <linux/spi/d4_spi_config.h>


#define HS_SPI_IOC_MAGIC  'h'





#define HS_SPI_IOCTL_SET				_IOW(HS_SPI_IOC_MAGIC, 1, struct d4_hs_spi_config)
#define HS_SPI_IOCTL_WRITE				_IOWR(HS_SPI_IOC_MAGIC, 2, struct spi_data_info)
#define LENS_INIT	        			_IO(HS_SPI_IOC_MAGIC, 3)
#define SPI_RB_CHECK                    _IOR(HS_SPI_IOC_MAGIC, 25, unsigned int)
#define HS_SPI_IOCTL_READ				_IOWR(HS_SPI_IOC_MAGIC, 4, struct spi_data_info)
#define SPI_KERNEL_CHECK				_IO(HS_SPI_IOC_MAGIC, 5)
#define SPI_KERNEL_INT_CHECK				_IO(HS_SPI_IOC_MAGIC, 6)
#define LENS_DMA_TEST				_IO(HS_SPI_IOC_MAGIC, 7)

#define RESET_FIX_ENABLE				_IO(HS_SPI_IOC_MAGIC, 8)
#define RESET_FIX_DISABLE				_IO(HS_SPI_IOC_MAGIC, 9)
#define SPI_COL_CHECK				_IO(HS_SPI_IOC_MAGIC, 10)
#define CH3_SPI_TEST				_IO(HS_SPI_IOC_MAGIC, 11)

#define HS_SPI_PHY_INFO				_IOR(HS_SPI_IOC_MAGIC, 12, struct spi_phys_reg_info)
#define HS_SPI_WAIT_TIME			_IOWR(HS_SPI_IOC_MAGIC, 13, unsigned int)
#define HS_SPI_RESET				_IO(HS_SPI_IOC_MAGIC, 14)
#define HS_SPI_BURST				_IOW(HS_SPI_IOC_MAGIC, 15, unsigned int)
#define HS_SPI_PAD_SET			_IOW(HS_SPI_IOC_MAGIC, 18, struct spi_pin_info)
#if 1
#define HS_SPI_IOCTL_INT_SREAD				_IOWR(HS_SPI_IOC_MAGIC, 16, struct spi_data_info)
#define HS_SPI_IOCTL_INT_SWRITE			_IOWR(HS_SPI_IOC_MAGIC, 18, struct spi_data_info)
#define HS_SPI_IOCTL_INT_DONE			_IO(HS_SPI_IOC_MAGIC, 19)
#define PMU_OFF				_IO(HS_SPI_IOC_MAGIC, 17)
#endif

#endif
