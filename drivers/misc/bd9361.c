#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/version.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/poll.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/i2c.h>
#include <linux/gpio.h>
#include <linux/slab.h>
#include <linux/pinctrl/consumer.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>

#include <linux/musb_sw.h>
#include <linux/bd9361.h>


#define DRV_VERSION			"0.1"

#define REG_SOFT_RESET			0x00
#define REG_ENABLE_COUNT		0x01
#define REG_VCNT12				0x02
#define REG_VCNT34				0x03
#define REG_VCNT6_LDO			0x04


#define BIT_RESET				0

#define BIT_EN5				0
#define BIT_EN6				1
#define BIT_EN_LDO			2
#define BIT_CH1PFM			3


struct bd9361_device {
	struct device		*dev;
	struct i2c_client	*i2c_client;
	struct work_struct	work;

	const char		*pinmux_name;
	unsigned		scl_gpio;
	unsigned		sda_gpio;
};


static int bd9361_write_register(struct i2c_client *i2c, u8 addr, u8 data)
{
	int ret;
	u8 buf[2];

	buf[0] = addr;
	buf[1] = data;
	ret = i2c_master_send(i2c, buf, 2);
	if (ret < 0) {
		dev_err(&i2c->dev, "I2C send failed - %d\n", ret);
		return -EIO;
	}

	return 0;
}


static int bd9361_read_register(struct i2c_client *i2c, u8 addr, u8 *data)
{
	int ret;

	ret = i2c_master_send(i2c, &addr, 1);
	if (ret < 0) {
		dev_err(&i2c->dev, "I2C send address failed - %d\n", ret);
		return -EIO;
	}

	ret = i2c_master_recv(i2c, data, 1);
	if (ret < 0) {
		dev_err(&i2c->dev, "I2C recv data failed - %d\n", ret);
		return -EIO;
	}

	return 0;
}


static irqreturn_t bd9361_isr(int irq, void *handle)
{
	struct bd9361_device *dev = (struct bd9361_device *)handle;

	disable_irq_nosync(irq);
	schedule_work(&dev->work);
	enable_irq(irq);

	return IRQ_HANDLED;
}


static int bd9361_set_init(struct bd9361_device *dev)
{
	int ret;
	u16 data;

	printk(KERN_EMERG"bd9361_init\n");
/*set voltage*/
	ret = bd9361_write_register(dev->i2c_client, REG_VCNT12,0x64);
	if (ret != 0)
		return ret;
	ret = bd9361_write_register(dev->i2c_client, REG_VCNT34,0x54);
	if (ret != 0)
		return ret;
	ret = bd9361_write_register(dev->i2c_client, REG_VCNT6_LDO,0x75);
	if (ret != 0)
		return ret;

/*enable*/
	ret = bd9361_write_register(dev->i2c_client, REG_ENABLE_COUNT,0xa);
	if (ret != 0)
		return ret;

	return 0;
}

static int bd9361_setup_gpio(struct bd9361_device *dev)
{
	int ret;

	ret = gpio_request(dev->scl_gpio, "bd9361 SCL Pin");
	if (ret != 0) {
		dev_err(dev->dev, "SCL GPIO request failed\n");
		goto free_scl_gpio;
	}
	ret = gpio_request(dev->sda_gpio, "bd9361 SDA Pin");
	if (ret != 0) {
		dev_err(dev->dev, "SDA GPIO request failed\n");
		goto free_scl_gpio;
	}

	ret = bd9361_set_init(dev);
	if (ret != 0) {
		dev_err(dev->dev, "initialize failed\n");
		goto free_sda_gpio;
	}

	return 0;
	
free_sda_gpio:
	gpio_free(dev->sda_gpio);
free_scl_gpio:
	gpio_free(dev->scl_gpio);
free_device:
	kfree(dev);

	return ret;
}

static __devinit int bd9361_i2c_probe(struct i2c_client *client,
				      const struct i2c_device_id *id)
{
	struct bd9361_device *bd9361 = NULL;
	struct bd9361_platform_data *param = NULL;
	int ret;

	bd9361 = kzalloc(sizeof(struct bd9361_device), GFP_KERNEL);
	if (bd9361 == NULL)
		return -ENOMEM;

	param = (struct bd9361_platform_data *)client->dev.platform_data;
	if (param == NULL){
		kfree (bd9361);
		return -EINVAL;
	}

	bd9361->dev = &client->dev;
	bd9361->i2c_client = client;

	bd9361->pinmux_name = param->pinmux_name;
	bd9361->scl_gpio = param->scl_gpio;
	bd9361->sda_gpio = param->sda_gpio;

	i2c_set_clientdata(client, bd9361);

	ret = bd9361_setup_gpio(bd9361);
	if (ret != 0) {
		return ret;
	}

	return 0;
}


static __devexit int bd9361_i2c_remove(struct i2c_client *client)
{
	struct bd9361_device *bd9361 = i2c_get_clientdata(client);

	if(bd9361){
		gpio_free(bd9361->sda_gpio);
		gpio_free(bd9361->scl_gpio);
		kfree(bd9361);
	}
	return 0;
}

static __devinit int bd9361_i2c_suspend(struct i2c_client *client, pm_message_t state)
{
	struct bd9361_device *bd9361 = i2c_get_clientdata(client);

	if(bd9361){
		gpio_free(bd9361->sda_gpio);
		gpio_free(bd9361->scl_gpio);
	}
	return 0;
}

static __devinit int bd9361_i2c_resume(struct i2c_client *client)
{
	struct bd9361_device *bd9361 = i2c_get_clientdata(client);
	int ret;

	if(bd9361){
		i2c_set_clientdata(client, bd9361);

		ret = bd9361_setup_gpio(bd9361);
		if (ret != 0) {
			return ret;
		}
	}
	return 0;
}


static const struct i2c_device_id bd9361_i2c_id[] = {
	{ BD9361_I2C_DEV_NAME, 0 },
	{ }
};


static struct i2c_driver bd9361_i2c_driver = {
	.driver = {
		.name = BD9361_I2C_DEV_NAME,
		.owner = THIS_MODULE,
	},
	.probe = bd9361_i2c_probe,
	.remove = __devexit_p(bd9361_i2c_remove),
	.suspend = bd9361_i2c_suspend,
	.resume = bd9361_i2c_resume,
	.id_table = bd9361_i2c_id
};


static __init int bd9361_init(void)
{
	int ret = 0;

	ret = i2c_add_driver(&bd9361_i2c_driver);
	if (ret) {
		printk(KERN_ERR "Failed to register bd9361 I2C driver: %d\n",
		       ret);
	}

	return ret;
}


static __exit void bd9361_exit(void)
{
	i2c_del_driver(&bd9361_i2c_driver);
}

module_init(bd9361_init);
module_exit(bd9361_exit);

MODULE_DESCRIPTION("bd9361 PMIC driver");
MODULE_LICENSE("GPL");
MODULE_VERSION(DRV_VERSION);
