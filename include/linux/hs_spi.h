/**
 * @file d4_ht_pwm.h
 * @brief DRIMe4 Hardware Trigger PWM Interface
 * @author kyuchun han <kyuchun.han@samsung.com>
 * 2010 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifndef _D4_HS_SPI_H
#define _D4_HS_SPI_H

#include <linux/spi/d4_spi_config.h>

struct hs_spi_data;


struct hs_spi_data *hs_spi_request(int hs_spi_id);


void hs_spi_get_phy_info(struct hs_spi_data *spi, struct spi_phys_reg_info *info);

void hs_spi_config(struct hs_spi_data *spi, struct d4_hs_spi_config *spi_info);
void hs_spi_config_clear(struct hs_spi_data *spi);
void hs_spi_close(struct hs_spi_data *spi);

void hs_spi_config_clear(struct hs_spi_data *spi);

int hs_spi_interrupt_write(struct hs_spi_data *spi, struct spi_data_info *spi_data);
int hs_spi_interrupt_read(struct hs_spi_data *spi, struct spi_data_info *spi_data);
int hs_spi_interrupt_rw(struct hs_spi_data *spi, struct spi_data_info *spi_data);

void hs_spi_pad_reset(struct hs_spi_data *spi);
void hs_spi_fix(struct hs_spi_data *spi, enum hs_spi_fix val);
void hs_spi_burst_set(struct hs_spi_data *spi, unsigned int val);

int hs_spi_polling_write(struct hs_spi_data *spi, struct spi_data_info *spi_data);
int hs_spi_polling_read(struct hs_spi_data *spi, struct spi_data_info *spi_data);
int hs_spi_polling_rw(struct hs_spi_data *spi, struct spi_data_info *spi_data);

int hs_spi_dma_transfer(struct hs_spi_data *spi, struct spi_data_info *spi_data);
void hs_spi_slave_set(struct hs_spi_data *spi);


/**/
int hs_spi_slave_interrupt_read(struct hs_spi_data *spi,
		struct spi_data_info *spi_data, int ReadyBusy, int notification, int rb_val, int noti_val);
int hs_spi_slave_interrupt_write(struct hs_spi_data *spi,
		struct spi_data_info *spi_data, int ReadyBusy, int notification, int rb_val, int noti_val);
int hs_spi_slave_interrupt_burst_read(struct hs_spi_data *spi,
		struct spi_data_info *spi_data,  int ReadyBusy, int notification, int rb_val, int noti_val);
int hs_spi_slave_interrupt_burst_dma_read(struct hs_spi_data *spi,
		struct spi_data_info *spi_data);

void hs_spi_pin_set(struct hs_spi_data *spi, struct spi_pin_info *pad_info);
void hs_spi_int_done(struct hs_spi_data *spi);

#endif /* _D4_HS_SPI_H */

