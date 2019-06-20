#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/hs_spi.h>
#include <linux/miscdevice.h>
#include <asm/uaccess.h>

#include <linux/bma2XX.h>
#include <mach/version_information.h>

#define DRV_VERSION			"0.2"

#define CMD_WRITE			0x00
#define CMD_READ			0x80

#define BMA222_CHIP_ID			0x03 //bma2XX: 0x03, bma2XXE: 0xf8
#define BMA254_CHIP_ID			0xfa //bma254: 0xfa

#define BMA2XX_REG_CHIP_ID		0x00
#define BMA2XX_REG_G_RANGE		0x0F
#define BMA2XX_REG_BANDWIDTH	0x10
#define BMA2XX_REG_DATA_FORMAT	0x13
#define BMA222_REG_ACC_X		0x03
#define BMA222_REG_ACC_Y		0x05
#define BMA222_REG_ACC_Z		0x07
#define BMA254_REG_ACC_X_LSB	0x02
#define BMA254_REG_ACC_X_MSB	0x03
#define BMA254_REG_ACC_Y_LSB	0x04
#define BMA254_REG_ACC_Y_MSB	0x05
#define BMA254_REG_ACC_Z_LSB	0x06
#define BMA254_REG_ACC_Z_MSB	0x07


#define BMA2x2_ACC_12_LSB__POS           4
#define BMA2x2_ACC_12_LSB__LEN           4
#define BMA2x2_ACC_12_LSB__MSK           0xF0
#define BMA2x2_ACC_MSB__POS          	 0
#define BMA2x2_ACC_MSB__LEN          	 8
#define BMA2x2_ACC_MSB__MSK          	 0xFF


/* EasyCASE ) */
#define BMA2x2_GET_BITSLICE(regvar, bitname)\
                        (regvar & bitname##__MSK) >> bitname##__POS



#define AXIS_BUFFER			(3)

struct bma2XX_device {
	struct device		*dev;
	struct hs_spi_data	*spi;
	struct work_struct	work;
};

static struct bma2XX_device bma2XX;

static int bma2XX_write_register(struct hs_spi_data *spi, u8 addr, u8 data)
{
	struct spi_data_info spi_wdata;
	u8 write_buf[2];

	write_buf[1] = CMD_WRITE | addr;
	write_buf[0] = data;

	spi_wdata.data_len = 1;
	spi_wdata.wbuffer = write_buf;
	spi_wdata.rbuffer = NULL;
	hs_spi_config(spi, &bma_2XX_infos.spi_data);
	return hs_spi_polling_write(spi, &spi_wdata);
}


static int bma2XX_read_register_byte(struct hs_spi_data *spi, u8 addr, u8 *data)
{
	int ret;
	struct spi_data_info spi_rwdata;
	u8 write_buf[2] = { 0, };
	u8 read_buf[2] = { 0, };

	write_buf[1] = CMD_READ | addr;
	write_buf[0] = 0x0;

	spi_rwdata.data_len = 1;
	spi_rwdata.wbuffer = write_buf;
	spi_rwdata.rbuffer = read_buf;
	hs_spi_config(spi, &bma_2XX_infos.spi_data);
	ret = hs_spi_polling_rw(spi, &spi_rwdata);
	*data = read_buf[0];

	return ret;
}

static int bma2XX_read_register_bytes(struct hs_spi_data *spi, u8 addr, u8 *data, u8 data_len)
{
	int ret = 0;
	int i = 0;
	u8 write_buf[32] = { 0, };
	u8 read_buf[32] = { 0, };
	static struct d4_hs_spi_config spi_info_multi = 
	{
		.speed_hz = 125000,
		.bpw = 8,
		.mode = SH_SPI_MODE_3,
		.waittime = 100,
		.ss_inven = D4_SPI_TRAN_INVERT_OFF,
		.spi_ttype = D4_SPI_TRAN_BURST_ON,
		.setup_select = SH_SPI_SPEED|SH_SPI_BPW|SH_SPI_WAVEMODE |SH_SPI_INVERT|SH_SPI_BURST|SH_SPI_WAITTIME,		
	};
	struct spi_data_info spi_rwdata;
	
	spi_rwdata.data_len = data_len + 1; //+1 means address
	spi_rwdata.wbuffer = write_buf;
	spi_rwdata.rbuffer = read_buf;
	
	write_buf[0] = CMD_READ | addr;

	hs_spi_config(spi, &spi_info_multi);
	ret = hs_spi_polling_rw(spi, &spi_rwdata);
	memcpy(data, &read_buf[1], data_len);	
	return ret;
}


static int bma2XX_initialize(void)
{
	int ret = 0;
	u8 data = 0;

	ret = bma2XX_read_register_byte(bma2XX.spi, BMA2XX_REG_CHIP_ID, &data);
	if (ret) 
	{
		dev_err(bma2XX.dev, "read Chip ID failed\n");
		return -EIO;
	}

	if (data != BMA254_CHIP_ID) 
	{
		dev_err(bma2XX.dev, "invalid chip ID : %x\n", data);
		return -EIO;
	}		
	ret = bma2XX_write_register(bma2XX.spi, BMA2XX_REG_BANDWIDTH, 0x09); //bandwidth 15.63Hz
	if (ret)
	{
		dev_err(bma2XX.dev, "write bandwidth failed\n");
		return -EIO;
	}
	
	ret = bma2XX_write_register(bma2XX.spi, BMA2XX_REG_G_RANGE, 0x0C); //+- 2g range
	if (ret)
	{
		dev_err(bma2XX.dev, "write range failed\n");
		return -EIO;
	}
	ret = bma2XX_write_register(bma2XX.spi, BMA2XX_REG_DATA_FORMAT, 0x00); //Filter Data 
	if (ret)
	{
		dev_err(bma2XX.dev, "write filter failed\n");
		return -EIO;
	}
	return 0;
}


static int bma2XX_get_acc_axis(s16 *x, s16 *y, s16 *z)
{
	int ret = 0;
	s16 s_temp_val = 0;
	u8 u_dataes[6] = {0, };
	ret = bma2XX_read_register_bytes(bma2XX.spi, BMA254_REG_ACC_X_LSB, u_dataes, 6);
	if (ret)
	{
		dev_err(bma2XX.dev, "read ACC XYZ failed\n");
		return ret;
	}
	s_temp_val = BMA2x2_GET_BITSLICE(u_dataes[0],BMA2x2_ACC_12_LSB)| (BMA2x2_GET_BITSLICE(u_dataes[1],BMA2x2_ACC_MSB)<<(BMA2x2_ACC_12_LSB__LEN));
    s_temp_val = s_temp_val << (sizeof(short)*8-(BMA2x2_ACC_12_LSB__LEN + BMA2x2_ACC_MSB__LEN));
    s_temp_val = s_temp_val >> (sizeof(short)*8-(BMA2x2_ACC_12_LSB__LEN + BMA2x2_ACC_MSB__LEN));
	*x = s_temp_val;
	s_temp_val = 0;
	s_temp_val = BMA2x2_GET_BITSLICE(u_dataes[2],BMA2x2_ACC_12_LSB)| (BMA2x2_GET_BITSLICE(u_dataes[3],BMA2x2_ACC_MSB)<<(BMA2x2_ACC_12_LSB__LEN));
    s_temp_val = s_temp_val << (sizeof(short)*8-(BMA2x2_ACC_12_LSB__LEN + BMA2x2_ACC_MSB__LEN));
    s_temp_val = s_temp_val >> (sizeof(short)*8-(BMA2x2_ACC_12_LSB__LEN + BMA2x2_ACC_MSB__LEN));
	*y = s_temp_val;		
	s_temp_val = 0;
	s_temp_val = BMA2x2_GET_BITSLICE(u_dataes[4],BMA2x2_ACC_12_LSB)| (BMA2x2_GET_BITSLICE(u_dataes[5],BMA2x2_ACC_MSB)<<(BMA2x2_ACC_12_LSB__LEN));
    s_temp_val = s_temp_val << (sizeof(short)*8-(BMA2x2_ACC_12_LSB__LEN + BMA2x2_ACC_MSB__LEN));
    s_temp_val = s_temp_val >> (sizeof(short)*8-(BMA2x2_ACC_12_LSB__LEN + BMA2x2_ACC_MSB__LEN));
	*z = s_temp_val;

	return 0;
}

static int bma2XX_open(struct inode *inode, struct file *filp)
{
	int ret;
	ret = bma2XX_initialize();	
	if (ret) {
		dev_err(bma2XX.dev, "initialize device failed\n");
		return -EIO;
	}

	return 0;
}

static int bma2XX_read(struct file *filep, char __user * buf, size_t count,
								loff_t *f_pos)
{
	s16 acc_x = 0, acc_y = 0, acc_z = 0;
	s16 ret_x = 0, ret_y = 0, ret_z = 0;
	int i_data_size = count / 3;
	if(bma2XX_get_acc_axis(&acc_z, &acc_y, &acc_x) < 0)
	{
		return -EINVAL;
	}
#ifdef	CONFIG_MACH_D4_NX2000
	ret_x = acc_z * -1;
	ret_y = acc_y * -1;
	ret_z = acc_x * -1;
#else
	ret_x = acc_y * -1;
	ret_y = acc_z * -1;
	ret_z = acc_x;
#endif	
	if(count < 0)
	{
		return -EINVAL;
	}
	if (copy_to_user(buf, &ret_x, i_data_size) != 0) {
		dev_err(bma2XX.dev, "copy orientation info\n");
		return -EIO;
	}
	if (copy_to_user(buf + i_data_size, &ret_y, i_data_size) != 0) {
		dev_err(bma2XX.dev, "copy orientation info\n");
		return -EIO;
	}
	if (copy_to_user(buf + (i_data_size * 2), &ret_z, i_data_size) != 0) {
		dev_err(bma2XX.dev, "copy orientation info\n");
		return -EIO;
	}

	return 1;
}
static int bma2XX_write(struct file *filp, const char __user *buf,
		size_t count, loff_t *f_pos)
{
	bma2XX_initialize();
	return 1;
}

static int bma222_release(struct inode *inode, struct file *filp)
{
	return 0;
}

const struct file_operations bma2XX_fops = 
{
	.owner			= THIS_MODULE,
	.open			= bma2XX_open,
	.read			= bma2XX_read,
	.write			= bma2XX_write,
	.release		= bma222_release,
};


static struct miscdevice bma2XX_miscdev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "accel",
	.fops = &bma2XX_fops,
};


static __init int bma2XX_init(void)
{
	int ret = 0;
	bma2XX.spi = hs_spi_request(bma_2XX_infos.channel);

	if (bma2XX.spi == NULL)
		return -1;

	hs_spi_config(bma2XX.spi, &bma_2XX_infos.spi_data);

	ret = misc_register(&bma2XX_miscdev);
	
	if (ret != 0)
		return -1;
	
	bma2XX.dev = bma2XX_miscdev.this_device;
	return 0;
}


static __exit void bma2XX_exit(void)
{
	misc_deregister(&bma2XX_miscdev);
}

#ifndef CONFIG_SCORE_FAST_RESUME
module_init(bma2XX_init);
#else
fast_dev_initcall(bma2XX_init);
#endif
module_exit(bma2XX_exit);

MODULE_AUTHOR("Hoin, Choi <goodman@acroem.com>");
MODULE_DESCRIPTION("BMA222 triaxial digital accelerometer driver");
MODULE_LICENSE("GPL");
MODULE_VERSION(DRV_VERSION);
