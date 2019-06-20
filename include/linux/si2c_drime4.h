/**
 * @file si2c_drime4.h
 * @brief DRIMe4 Slave I2C Interface
 * @author kyuchun han <kyuchun.han@samsung.com>
 * 2010 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifndef _D4_SI2C_H
#define _D4_SI2C_H


struct drime4_si2c;


struct drime4_si2c *drime4_si2c_request(int si2c_id);

int drime4_si2c_config(struct drime4_si2c *si2c,
		enum si2c_tran_mode value, unsigned char saddr);

unsigned int drime4_si2c_transmit_addr_check(struct drime4_si2c *si2c);
void drime4_si2c_byte_write(struct drime4_si2c *si2c, unsigned char val);
void drime4_si2c_last_byte_write(struct drime4_si2c *si2c, unsigned char val);
void drime4_si2c_transmit_stop(struct drime4_si2c *si2c);


unsigned int drime4_si2c_recive_addr_check(struct drime4_si2c *si2c);
unsigned char drime4_si2c_byte_read(struct drime4_si2c *si2c);
void drime4_si2c_recive_stop(struct drime4_si2c *si2c);

int drime4_si2c_ack_waiting(struct drime4_si2c *si2c, int time_out);

int drime4_status_check(struct drime4_si2c *si2c);
void drime4_si2c_err_stop(struct drime4_si2c *si2c);

/*for test*/
void drime4_si2c_test(void);

#endif /* _D4_HS_SPI_H */

