#ifndef LENS_COMM_H_
#define LENS_COMM_H_

#include <linux/spi/spidev.h>

struct lens_spi_ioc_transfer {
	unsigned long		tx_buf;
	unsigned long		rx_buf;
	unsigned long		len;
	unsigned short		delay_usecs;
};

struct lens_spi_ioc_receive {
	unsigned long		tx_buf;
	unsigned long		rx_buf;
	unsigned long		p_len;
	unsigned short		delay_usecs;
};

enum lens_irq_types {
	LENS_IRQ_TYPE_RB,
	LENS_IRQ_TYPE_DET,
};

struct lens_irq_control {
	enum lens_irq_types irqtype;
	int enable;
};

struct lens_gpio_control {
	unsigned short gpio_id;
	unsigned long p_gpio_value;
};

/*Collision setting */
#define LENS_IOC_SPI_WR_FIXED_DELAY		_IOW(SPI_IOC_MAGIC, 5, unsigned long)
#define LENS_IOC_SPI_WR_COL_CHK			_IOW(SPI_IOC_MAGIC, 6, unsigned long)
#define LENS_IOC_SPI_RD_FIXED_DELAY		_IOW(SPI_IOC_MAGIC, 7, unsigned long)
#define LENS_IOC_SPI_DMA_RD			_IOW(SPI_IOC_MAGIC, 8, unsigned long)

#define LENS_IOC_ENABLE_IRQ			_IOW(SPI_IOC_MAGIC, 9, unsigned char)
#define LENS_IOC_REQ_FIN			_IOW(SPI_IOC_MAGIC, 10, unsigned char)

#define LENS_IOC_SPI_DISABLE			_IOW(SPI_IOC_MAGIC, 11, unsigned char)
#define LENS_IOC_SPI_ENABLE			_IOW(SPI_IOC_MAGIC, 12, unsigned char)
#define LENS_IOC_SPI_CLOSE			_IOW(SPI_IOC_MAGIC, 13, unsigned char)

#define LENS_IOC_GPIO_DIR			_IOW(SPI_IOC_MAGIC, 14, unsigned long)
#define LENS_IOC_GPIO_GET			_IOW(SPI_IOC_MAGIC, 15, unsigned long)
#define LENS_IOC_GPIO_SET			_IOW(SPI_IOC_MAGIC, 16, unsigned long)

#define LENS_IOC_MEM_SET			_IOW(SPI_IOC_MAGIC, 17, unsigned long)
#define ERRNO_COLLISION 300
#define ERRNO_FALSE_COLLISION 400

#endif /*LENS_COMM_H_*/

