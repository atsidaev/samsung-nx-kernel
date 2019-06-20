#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/signal.h>
#include <linux/sched.h>
#include <linux/io.h>
#include <linux/uaccess.h>
#include <linux/pinctrl/consumer.h>

#include <linux/str_ctrl.h>
#include <linux/str_interface.h>
//#include <linux/spi/spidev.h>
#include <linux/hs_spi.h>
#include <linux/platform_device.h>
#include <linux/pinctrl/pinmux.h>
#include <linux/device.h>

#include <mach/gpio.h>


#define DEBUG (0)

#define Sdebug1(a) if(DEBUG) printk(a)
#define Sdebug2(a,b) if(DEBUG) printk(a,b)
#define Sdebug3(a,b,c) if(DEBUG) printk(a,b,c)


/* test strobe fire test */
#undef TEST_STROBE_FIRE

struct str_spi {
	u8 tx_buffer[255];
	u8 rx_buffer[255];
	
	struct hs_spi_data *spi;
};

static struct str_spi str_spi_data;





struct str_data {
	dev_t devt;
	struct device *dev;

	struct mutex buf_lock;
	struct hs_spi_data *spi;

	unsigned int int_ext_str_det;
	unsigned int out_str_trig;
	unsigned int out_str_vcc_on;





	spinlock_t lock;
	bool initialized;

	unsigned long exposure_time;
	unsigned long adjustment_time;

	struct hrtimer htimer;
	struct work_struct work;

	struct pinmux *pmx;
};


static int strobe_spi_normal_transfer(struct strobe_spi_ioc_transfer *ioc,
				      struct str_spi *str_spi_data)
{
	char * a;
	int i=0;
	struct hs_spi_data      *spi = str_spi_data->spi;
	struct d4_hs_spi_config spi_data;
	struct spi_data_info    t = {
		.data_len       = ioc->len,
		.wbuffer        = str_spi_data->tx_buffer,
		.rbuffer        = str_spi_data->rx_buffer,
	};

	Sdebug2("pr:%d\n",ioc->pre_delay);
	Sdebug2("ho:%d\n",ioc->hold_delay);
	Sdebug2("po:%d\n",ioc->post_delay);
	Sdebug2("len:%d\n",ioc->len);
	
	spi_data.delayconf.pre_delay = ioc->pre_delay;
	spi_data.delayconf.hold_delay = ioc->hold_delay;
	spi_data.delayconf.post_delay= ioc->post_delay;

	spi_data.spi_ttype = D4_SPI_TRAN_BURST_OFF;
	spi_data.setup_select = SH_SPI_BURST|HS_SPI_DELAY;
	
	hs_spi_config(spi, &spi_data);
	hs_spi_polling_rw(spi, &t);

	a  = (char*)t.rbuffer;
	Sdebug1("ioctl rx: ");
	for(i=0; i<ioc->len;i++)
		Sdebug2("%x ",*(a+i));
	Sdebug1("\n");

	return 0;
}

static int strobe_spi_burst_transfer(struct strobe_spi_ioc_transfer *ioc,
				      struct str_spi *str_spi_data)
{
	char * a;
	int i=0;
	struct hs_spi_data      *spi = str_spi_data->spi;
	struct d4_hs_spi_config spi_data;
	struct spi_data_info    t = {
		.data_len       = ioc->len,
		.wbuffer        = str_spi_data->tx_buffer,
		.rbuffer        = str_spi_data->rx_buffer,
	};

	Sdebug2("pr:%d\n",ioc->pre_delay);
	Sdebug2("ho:%d\n",ioc->hold_delay);
	Sdebug2("po:%d\n",ioc->post_delay);
	Sdebug2("len:%d\n",ioc->len);
	
	spi_data.delayconf.pre_delay = ioc->pre_delay;
	spi_data.delayconf.hold_delay = ioc->hold_delay;
	spi_data.delayconf.post_delay= ioc->post_delay;

	spi_data.spi_ttype = D4_SPI_TRAN_BURST_ON;
	spi_data.setup_select = SH_SPI_BURST|HS_SPI_DELAY;
	
	hs_spi_config(spi, &spi_data);
	hs_spi_polling_rw(spi, &t);

	a  = (char*)t.rbuffer;
	Sdebug1("ioctl rx: ");
	for(i=0; i<ioc->len;i++)
		Sdebug2("%x ",*(a+i));
	Sdebug1("\n");

	return 0;
}

static int str_gpio_get_value(unsigned int gpio, unsigned int * value)
{
	*value = gpio_get_value(gpio);
	Sdebug2("%d\n",*value);
	return 0;
}

static int str_gpio_set_value(unsigned int gpio, unsigned int value)
{
	gpio_set_value(gpio, (int)value);
	Sdebug2("%d\n",value);
	return 0;
}


static long str_ioctl(struct file *filp, unsigned int cmd,
				    unsigned long arg)
{
	int i;
	int ret = 0;
	int size = 0;
	unsigned int value = 0;
	
	struct miscdevice *miscdev = filp->private_data;
	struct str_data *data = miscdev->this_device->platform_data;

	struct strobe_spi_ioc_transfer strobe_transfer;

	if (_IOC_TYPE(cmd) != STR_IOC_MAGIC)
		return -1;


	size = _IOC_SIZE(cmd);

	mutex_lock(&(data->buf_lock));

	switch (cmd) {
	case EX_STR_DET_GET :
		if(copy_from_user((void *) &value, (const void *) arg, size))
			goto fail;
		
		Sdebug1("\nEX_STR_DET_GET :");
		str_gpio_get_value(data->int_ext_str_det,&value);

		if(copy_to_user(	(const void *)arg,&value, size))
			goto fail;	
		break;

	case EX_STR_VCCON_SET :
		if(copy_from_user((void *) &value, (const void *) arg, size))
			ret = -EFAULT;

		Sdebug2("\nEX_STR_VCCON_SET :%d\n", value);
		str_gpio_set_value(data->out_str_vcc_on,value);
		break;

	case EX_STR_TRIG_SET:
		if(copy_from_user((void *) &value, (const void *) arg, size))
			ret = -EFAULT;

		Sdebug2("\nEX_STR_TRIG_SET :%d\n", value);
		str_gpio_set_value(data->out_str_trig,value);
		break;
		
	case HOTSHOE_BURST_TRANSFER:
		if (copy_from_user(&strobe_transfer, (const void *)arg,
				     sizeof(struct strobe_spi_ioc_transfer))) {
			ret = -EFAULT;
			break;
		}
		Sdebug1("\n HOTSHOE_BURST_TRANSFER\n");

		if (strobe_transfer.tx_buf == 0 ||
		    strobe_transfer.rx_buf == 0) {
			ret = -EFAULT;
			break;
		}

		if (copy_from_user(str_spi_data.tx_buffer,
				     (const void *)strobe_transfer.tx_buf,
				     strobe_transfer.len)) {
			ret = -EFAULT;
			break;
		}

		Sdebug1("ioctl tx :");
		for(i=0; i<strobe_transfer.len; i++){
			Sdebug2("%x ",str_spi_data.tx_buffer[i]);
			str_spi_data.rx_buffer[i]=3;
		}
		Sdebug1("\n");

		strobe_spi_burst_transfer(&strobe_transfer, &str_spi_data);

		if (strobe_transfer.rx_buf) {
			ret = copy_to_user(
				(const void *)strobe_transfer.rx_buf,
				str_spi_data.rx_buffer, strobe_transfer.len);
		} else
			ret = 0;


		if (ret > 0) {
			ret = -EFAULT;
			break;
		}

		break;

	case HOTSHOE_NORMAL_TRANSFER:
		if (copy_from_user(&strobe_transfer, (const void *)arg,
				     sizeof(struct strobe_spi_ioc_transfer))) {
			ret = -EFAULT;
			break;
		}
		Sdebug1("\n HOTSHOE_NORMAL_TRANSFER\n");

		if (strobe_transfer.tx_buf == 0 ||
		    strobe_transfer.rx_buf == 0) {
			ret = -EFAULT;
			break;
		}

		if (copy_from_user(str_spi_data.tx_buffer,
				     (const void *)strobe_transfer.tx_buf,
				     strobe_transfer.len)) {
			ret = -EFAULT;
			break;
		}

		Sdebug1("ioctl tx :");
		for(i=0; i<strobe_transfer.len; i++){
			Sdebug2("%x ",str_spi_data.tx_buffer[i]);
			str_spi_data.rx_buffer[i]=3;
		}
		Sdebug1("\n");

		strobe_spi_normal_transfer(&strobe_transfer, &str_spi_data);

		if (strobe_transfer.rx_buf) {
			ret = copy_to_user(
				(const void *)strobe_transfer.rx_buf,
				str_spi_data.rx_buffer, strobe_transfer.len);
		} else
			ret = 0;


		if (ret > 0) {
			ret = -EFAULT;
			break;
		}

		break;


	default:
		ret = -EINVAL;
		break;
	}

fail:

	mutex_unlock(&(data->buf_lock));

	return ret;
}

static int str_open(struct inode *inode, struct file *filp)
{
	return 0;
}

static int str_read(struct file *filep, char __user * buf, 
	size_t count, loff_t *f_pos)
{
	return 0;
}


static int str_release(struct inode *inode, struct file *filp)
{
	return 0;
}


const struct file_operations str_ctrl_fops = {
	.owner = THIS_MODULE,
	.open = str_open,
	.unlocked_ioctl = str_ioctl,
	.read = str_read,
	.release = str_release,
};

static struct miscdevice str_ctrl_miscdev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "str",
	.fops = &str_ctrl_fops,
};

static int __init str_ctrl_probe(struct platform_device *pdev)
{
	int ret, i;
	struct str_data *data;
	struct platform_str_ctrl_data *pdata = pdev->dev.platform_data;
	struct d4_hs_spi_config spi_data;

	data = kzalloc(sizeof(struct str_data), GFP_KERNEL);
	if (!data) {
		ret  = -ENOMEM;
		goto err_alloc;
	}

	data->dev = &pdev->dev;

	str_spi_data.spi = hs_spi_request(pdata->spio_ch);

	if (str_spi_data.spi == NULL) {
		ret = -ENOMEM;
		goto err_free_gpio;
	}

	spi_data.speed_hz = 210900;
	spi_data.bpw = 8;
	spi_data.mode = SH_SPI_MODE_3;
	spi_data.waittime = 200;
	spi_data.ss_inven = D4_SPI_TRAN_INVERT_OFF;
	spi_data.setup_select = SH_SPI_SPEED|SH_SPI_BPW|SH_SPI_WAVEMODE |
				SH_SPI_INVERT|SH_SPI_BURST|SH_SPI_WAITTIME|SH_SPI_BURST|HS_SPI_DELAY;



	spi_data.delayconf.pre_delay = 120;
	spi_data.delayconf.hold_delay = 30;
	spi_data.delayconf.post_delay= 0;

	spi_data.spi_ttype = D4_SPI_TRAN_BURST_ON;
//	spi_data.setup_select = SH_SPI_BURST|HS_SPI_DELAY;

	hs_spi_config(str_spi_data.spi, &spi_data);

	for (i = 0; i < pdata->ngpio; i++) {
		struct str_gpio *gpio_port = &pdata->gpios[i];
		if (gpio_port->port == (unsigned)-1)
		{
			if (strcmp(gpio_port->name, "det") == 0)
				gpio_port->port = GPIO_EXT_STR_DET;
			else if (strcmp(gpio_port->name, "vccon") == 0)
				gpio_port->port = GPIO_STR_VCC_ON;			
		}
		pinctrl_request_gpio(gpio_port->port);

		ret = gpio_request(gpio_port->port,  gpio_port->name);
		if (ret) {
			dev_err(&pdev->dev, "unable to claim gpio %u, err=%d\n",
				gpio_port->port, ret);
			goto err_free_gpio;
		}

		if( gpio_port->dir == GPIO_DIR_IN)
			ret = gpio_direction_input(gpio_port->port);
		else
			ret = gpio_direction_output(gpio_port->port, gpio_port->value);
		if (ret) {
			dev_err(&pdev->dev,
				"unable to set direction on gpio %u, err=%d\n",
				gpio_port->port, ret);
			goto err_free_gpio;
		}
		if(strcmp(gpio_port->name,"det")==0)
			data->int_ext_str_det = gpio_port->port;
		else if(strcmp(gpio_port->name,"trig")==0)
			data->out_str_trig = gpio_port->port;
		else if(strcmp(gpio_port->name,"vccon")==0)
			data->out_str_vcc_on = gpio_port->port;
		else
			dev_err(&pdev->dev, "Failed to match gpio name.\n");
	}

	ret = misc_register(&str_ctrl_miscdev);
	if (ret < 0) {
		dev_err(&pdev->dev, "Failed to register misc driver.\n");
		goto err_free_gpio;
	}
	str_ctrl_miscdev.this_device->platform_data = data;

	mutex_init(&(data->buf_lock));

	platform_set_drvdata(pdev, data);

	return ret;


//	spin_lock_init(&data->lock);


//err_req:
//	for (i = 0; i < pdata->n_pwm_infos; i++) {
//		if (pwm->pwm[i])
//			d4_ht_pwm_free(pwm->pwm[i]);
//	}
err_free_gpio:
	kfree(data);
err_alloc:
	return ret;
}

static int __devexit str_ctrl_remove(struct platform_device* pdev)
{
	//int i;
	struct str_data *pwm = platform_get_drvdata(pdev);
	if(pwm)
		kfree(pwm);
	return 0;
}

static struct platform_driver str_ctrl_driver = {
	.probe = str_ctrl_probe,
	.remove = __devexit_p(str_ctrl_remove),
	.driver = {
		.name = "str-ctrl",
		.owner = THIS_MODULE,
	},
};

static int __init str_ctrl_init(void)
{
	return platform_driver_register(&str_ctrl_driver);
}

static void __exit str_ctrl_exit(void)
{
	platform_driver_unregister(&str_ctrl_driver);
}

#ifndef CONFIG_SCORE_FAST_RESUME
module_init(str_ctrl_init);
#else
fast_dev_initcall(str_ctrl_init);
#endif
module_exit(str_ctrl_exit);

MODULE_AUTHOR("Kim Tae Kyung");
MODULE_DESCRIPTION("str ctrl driver");
MODULE_LICENSE("GPL");
