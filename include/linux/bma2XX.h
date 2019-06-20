#ifndef __BMA2XX_H__
#define __BMA2XX_H__

#if defined(CONFIG_BMA2XX)

#include <linux/hs_spi.h>
#include <linux/bma2XX.h>

typedef struct _bma_spi_info {
	unsigned int			channel;
	struct d4_hs_spi_config spi_data;
}bma_spi_info;


bma_spi_info bma_2XX_infos = 
{	
	.channel = 3,
	.spi_data.speed_hz = 125000,
	.spi_data.bpw = 16,
	.spi_data.mode = SH_SPI_MODE_3,
	.spi_data.waittime = 100,
	.spi_data.ss_inven = D4_SPI_TRAN_INVERT_OFF,
	.spi_data.spi_ttype = D4_SPI_TRAN_BURST_OFF,
	.spi_data.setup_select = SH_SPI_SPEED|SH_SPI_BPW|SH_SPI_WAVEMODE |SH_SPI_INVERT|SH_SPI_BURST|SH_SPI_WAITTIME,
	
};


#endif

#endif /* __BMA2XX_H__ */
