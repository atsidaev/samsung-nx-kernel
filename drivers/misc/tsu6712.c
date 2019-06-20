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
#include <linux/tsu6712.h>


#define DRV_VERSION			"0.1"

#define REG_DEVICE_ID			0x01
#define REG_CONTROL			0x02
#define REG_INTERRUPT_1			0x03
#define REG_INTERRUPT_2			0x04
#define REG_INTERRUPT_MASK_1		0x05
#define REG_INTERRUPT_MASK_2		0x06
#define REG_ADC				0x07
#define REG_TIMING_SET_1		0x08
#define REG_TIMING_SET_2		0x09
#define REG_DEVICE_TYPE_1		0x0a
#define REG_DEVICE_TYPE_2		0x0b
#define REG_BUTTON_1			0x0c
#define REG_BUTTON_2			0x0d
#define REG_CARKIT_STATUS		0x0e
#define REG_CARKIT_INT_1		0x0f
#define REG_CARKIT_INT_2		0x10
#define REG_CARKIT_INT_MASK_1		0x11
#define REG_CARKIT_INT_MASK_2		0x12
#define REG_MANUAL_SW_1			0x13
#define REG_MANUAL_SW_2			0x14

#define BIT_SW_OPEN			4
#define BIT_RAW_DATA			3
#define BIT_MANUAL_SW			2
#define BIT_WAIT			1
#define BIT_INT_MASK			0

#define BIT_ATTACH			0
#define BIT_DETACH			1
#define BIT_RESERVED_ATTACH		9

#define BIT_AUDIO_TYPE_1		0
#define BIT_AUDIO_TYPE_2		1
#define BIT_USB				2
#define BIT_UART			3
#define BIT_CARKIT			4
#define BIT_USB_CHARGER			5
#define BIT_DEDICATED			6
#define BIT_USB_OTG			7
#define BIT_JIG_USB_ON			8
#define BIT_JIG_USB_OFF			9
#define BIT_JIG_UART_ON			10
#define BIT_JIG_UART_OFF		11
#define BIT_PPD				12
#define BIT_TTY				13
#define BIT_CHG_AV			14
#define BIT_AUDIO_TYPE_3		15

#define VAL_VBUS_OPEN			0x0000
#define VAL_VBUS2OUT			0x0001
#define VAL_VBUS2MIC			0x0002

#define VAL_DP_OPEN			0x0000
#define VAL_DP2USBDP			0x0004
#define VAL_DP2S_R			0x0008
#define VAL_DP2RX			0x000c
#define VAL_DP2V_R			0x0010

#define VAL_DM_OPEN			0x0000
#define VAL_DM2USBDM			0x0020
#define VAL_DM2S_L			0x0040
#define VAL_DM2TX			0x0060
#define VAL_DM2V_L			0x0080

#define VAL_ID_OPEN			0x0000
#define VAL_ID2VIDEO			0x0100
#define VAL_ID2IDBP			0x0200

#define VAL_JIG_LOW			0x0000
#define VAL_JIG_HIGH			0x0400
#define VAL_BOOT_LOW			0x0000
#define VAL_BOOT_HIGH			0x0800
#define VAL_ISET_Z			0x0000
#define VAL_ISET_GND			0x1000

#define WAKEUP_TIMMING			50


#define DEV_TYPE_AUDIO			((1 << BIT_AUDIO_TYPE_1) \
					| (1 << BIT_AUDIO_TYPE_2) \
					| (1 << BIT_AUDIO_TYPE_3))
#define DEV_TYPE_AV			(1 << BIT_CHG_AV)
#define DEV_TYPE_USB			((1 << BIT_USB) | (1 << BIT_USB_OTG) \
					| (1 << BIT_USB_CHARGER) \
					| (1 << BIT_JIG_USB_ON) \
					| (1 << BIT_JIG_USB_OFF))
#define DEV_TYPE_UART			((1 << BIT_UART) \
					| (1 << BIT_JIG_UART_ON) \
					| (1 << BIT_JIG_UART_OFF))
#define DEV_TYPE_CHARGER		((1 << BIT_USB_CHARGER) \
					| (1 << BIT_DEDICATED))


struct tsu6712_device {
	struct device		*dev;
	struct i2c_client	*i2c_client;
	struct work_struct	work;

	const char		*pinmux_name;
	unsigned		scl_gpio;
	unsigned		sda_gpio;
	unsigned		intb;
	int			irq;

	int			musb_dev_type;
	int			manual_mode;
	int			jig_on;
	int			jig_off;
	int			temp_manual_sw;

	int			(*attach_callback)(int device_type);
	int			(*detach_callback)(int device_type);
};


static int tsu6712_write_register(struct i2c_client *i2c, u8 addr, u8 data)
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


static int tsu6712_read_register(struct i2c_client *i2c, u8 addr, u8 *data)
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


static irqreturn_t tsu6712_isr(int irq, void *handle)
{
	struct tsu6712_device *dev = (struct tsu6712_device *)handle;

	disable_irq_nosync(irq);
	schedule_work(&dev->work);
	enable_irq(irq);

	return IRQ_HANDLED;
}


static int tsu6712_init_muic(struct tsu6712_device *dev)
{
	int ret;
	u16 data;

	data = (1 << BIT_SW_OPEN) | (1 << BIT_RAW_DATA)	| (1 << BIT_WAIT);
	if (!dev->manual_mode)
		data |= (1 << BIT_MANUAL_SW);
	tsu6712_write_register(dev->i2c_client, REG_CONTROL, data);

	data = 0xffff;
	data &= ~((1 << BIT_ATTACH) | (1 << BIT_DETACH)
				| (1 << BIT_RESERVED_ATTACH));

	ret = tsu6712_write_register(dev->i2c_client, REG_INTERRUPT_MASK_1,
							(data & 0x00ff));
	if (ret != 0)
		return ret;

	ret = tsu6712_write_register(dev->i2c_client, REG_INTERRUPT_MASK_2,
							((data & 0xff00) >> 8));
	if (ret != 0)
		return ret;

	return 0;
}


static int tsu6712_sw_reset(struct tsu6712_device *tsu6712)
{
	struct pinctrl *pmux = NULL;
	struct pinctrl_state	*pins_default;

	pinctrl_free_gpio(tsu6712->scl_gpio);
	pinctrl_free_gpio(tsu6712->sda_gpio);

	pinctrl_request_gpio(tsu6712->scl_gpio);
	pinctrl_request_gpio(tsu6712->sda_gpio);

	gpio_direction_output(tsu6712->scl_gpio, 0);
	gpio_direction_output(tsu6712->sda_gpio, 0);

	while (gpio_get_value(tsu6712->intb))
		;

	gpio_set_value(tsu6712->scl_gpio, 1);
	gpio_set_value(tsu6712->sda_gpio, 1);

	pinctrl_free_gpio(tsu6712->scl_gpio);
	pinctrl_free_gpio(tsu6712->sda_gpio);

	pmux = devm_pinctrl_get(tsu6712->dev);

	if (!IS_ERR(pmux)) {
		pins_default = pinctrl_lookup_state(pmux, PINCTRL_STATE_DEFAULT);
		pinctrl_select_state(pmux, pins_default);
		devm_pinctrl_put(pmux);
	} else {
		return -1;
	}

	return 0;
}


static int tsu6712_change_to_automatic_mode(struct tsu6712_device *dev)
{
	int ret;
	u8 data = 0;

	ret = tsu6712_read_register(dev->i2c_client, REG_CONTROL, &data);
	if (ret != 0)
		return ret;

	data |= 1 << BIT_MANUAL_SW;

	ret = tsu6712_write_register(dev->i2c_client, REG_CONTROL, data);
	if (ret != 0)
		return ret;

	return 0;
}


static int tsu6712_change_to_manual_mode(struct tsu6712_device *dev)
{
	int ret;
	u8 data = 0;

	ret = tsu6712_read_register(dev->i2c_client, REG_CONTROL, &data);
	if (ret != 0)
		return ret;

	data &= ~(1 << BIT_MANUAL_SW);

	ret = tsu6712_write_register(dev->i2c_client, REG_CONTROL, data);
	if (ret != 0)
		return ret;

	return 0;
}


static int tsu6712_config_idle(struct tsu6712_device *dev)
{
	int ret;
	u16 data;

	data = VAL_VBUS_OPEN | VAL_DP_OPEN | VAL_DM_OPEN | VAL_ID_OPEN
				| VAL_JIG_LOW | VAL_BOOT_LOW | VAL_ISET_Z;

	ret = tsu6712_write_register(dev->i2c_client, REG_MANUAL_SW_1,
							(data & 0x00ff));
	if (ret != 0)
		return ret;
	ret = tsu6712_write_register(dev->i2c_client, REG_MANUAL_SW_2,
						((data >> 8) & 0x00ff));
	if (ret != 0)
		return ret;

	return 0;
}


static int tsu6712_config_for_USB(struct tsu6712_device *dev)
{
	int ret;
	u16 data;

	data = VAL_VBUS2OUT | VAL_DP2USBDP | VAL_DM2USBDM | VAL_ID2IDBP
							| VAL_ISET_Z;

	if (dev->jig_on)	/* JIG USB ON */
		data |= VAL_JIG_HIGH | VAL_BOOT_HIGH;
	else if (dev->jig_off)	/* JIG USB OFF */
		data |= VAL_JIG_HIGH | VAL_BOOT_LOW;
	else
		data |= VAL_JIG_LOW | VAL_BOOT_LOW;

	ret = tsu6712_write_register(dev->i2c_client, REG_MANUAL_SW_1,
							(data & 0x00ff));
	if (ret != 0)
		return ret;
	ret = tsu6712_write_register(dev->i2c_client, REG_MANUAL_SW_2,
						((data >> 8) & 0x00ff));
	if (ret != 0)
		return ret;

	return 0;
}


static int tsu6712_config_for_UART(struct tsu6712_device *dev)
{
	int ret;
	u16 data;

	data = VAL_DP2RX | VAL_DM2TX | VAL_ID_OPEN | VAL_ISET_Z;

	if (dev->jig_on)	/* JIG UART ON */
		data |= VAL_JIG_HIGH | VAL_BOOT_HIGH | VAL_VBUS2OUT;
	else if (dev->jig_off)	/* JIG UART OFF */
		data |= VAL_JIG_HIGH | VAL_BOOT_LOW | VAL_VBUS2OUT;
	else
		data |= VAL_JIG_LOW | VAL_BOOT_LOW | VAL_VBUS_OPEN;

	ret = tsu6712_write_register(dev->i2c_client, REG_MANUAL_SW_1,
							(data & 0x00ff));
	if (ret != 0)
		return ret;
	ret = tsu6712_write_register(dev->i2c_client, REG_MANUAL_SW_2,
						((data >> 8) & 0x00ff));
	if (ret != 0)
		return ret;

	return 0;
}


static int tsu6712_config_for_AV(struct tsu6712_device *dev)
{
	int ret;
	u16 data;

	data = VAL_VBUS_OPEN | VAL_DP2V_R | VAL_DM2V_L | VAL_ID2VIDEO
				| VAL_JIG_LOW | VAL_BOOT_LOW | VAL_ISET_Z;

	ret = tsu6712_write_register(dev->i2c_client, REG_MANUAL_SW_1,
							(data & 0x00ff));
	if (ret != 0)
		return ret;
	ret = tsu6712_write_register(dev->i2c_client, REG_MANUAL_SW_2,
						((data >> 8) & 0x00ff));
	if (ret != 0)
		return ret;

	return 0;
}


static int tsu6712_config_for_MIC(struct tsu6712_device *dev)
{
	int ret;
	u16 data = 0;
	u8 one_byte = 0;

	ret = tsu6712_read_register(dev->i2c_client, REG_MANUAL_SW_1,
							&one_byte);
	if (ret != 0)
		return ret;

	data = one_byte;

	ret = tsu6712_read_register(dev->i2c_client, REG_MANUAL_SW_2,
							&one_byte);
	if (ret != 0)
		return ret;

	data |= (one_byte << 8) & 0xff00;
	data |= VAL_ID2IDBP;

	ret = tsu6712_write_register(dev->i2c_client, REG_MANUAL_SW_1,
							(data & 0x00ff));
	if (ret != 0)
		return ret;
	ret = tsu6712_write_register(dev->i2c_client, REG_MANUAL_SW_2,
						((data >> 8) & 0x00ff));
	if (ret != 0)
		return ret;

	return 0;
}


static int tsu6712_config_for_charger(struct tsu6712_device *dev)
{
	int ret;
	u16 data;

	data = VAL_VBUS2OUT | VAL_DP_OPEN | VAL_DM_OPEN | VAL_ID_OPEN
				| VAL_JIG_LOW | VAL_BOOT_LOW | VAL_ISET_Z;

	ret = tsu6712_write_register(dev->i2c_client, REG_MANUAL_SW_1,
							(data & 0x00ff));
	if (ret != 0)
		return ret;
	ret = tsu6712_write_register(dev->i2c_client, REG_MANUAL_SW_2,
						((data >> 8) & 0x00ff));
	if (ret != 0)
		return ret;

	return 0;
}


static int tsu6712_config_for_reserved4(struct tsu6712_device *dev)
{
	int ret;
	u16 data;

	data = VAL_VBUS_OPEN | VAL_DP2S_R | VAL_DM2V_L | VAL_ID_OPEN
				| VAL_JIG_LOW | VAL_BOOT_LOW | VAL_ISET_Z;

	ret = tsu6712_write_register(dev->i2c_client, REG_MANUAL_SW_1,
							(data & 0x00ff));
	if (ret != 0)
		return ret;
	ret = tsu6712_write_register(dev->i2c_client, REG_MANUAL_SW_2,
						((data >> 8) & 0x00ff));
	if (ret != 0)
		return ret;

	return 0;
}


static int tsu6712_config_for_reserved5(struct tsu6712_device *dev)
{
	int ret;
	u16 data;

	data = VAL_VBUS2MIC | VAL_DP2S_R | VAL_DM2S_L | VAL_ID2IDBP
				| VAL_JIG_LOW | VAL_BOOT_LOW | VAL_ISET_Z;

	ret = tsu6712_write_register(dev->i2c_client, REG_MANUAL_SW_1,
							(data & 0x00ff));
	if (ret != 0)
		return ret;
	ret = tsu6712_write_register(dev->i2c_client, REG_MANUAL_SW_2,
						((data >> 8) & 0x00ff));
	if (ret != 0)
		return ret;

	return 0;
}


static int tsu6712_process_attachment(struct tsu6712_device *dev,
					int deviceType, int adcValue)
{
	dev->temp_manual_sw = 0;

	if (deviceType & DEV_TYPE_AUDIO)
		dev->musb_dev_type = MUSB_DEV_ETC;
	if (deviceType & DEV_TYPE_USB) {
		dev->musb_dev_type = MUSB_DEV_USB;

		if (deviceType & (1 << BIT_JIG_USB_ON))
			dev->jig_on = 1;
		else
			dev->jig_on = 0;

		if (deviceType & (1 << BIT_JIG_USB_OFF))
			dev->jig_off = 1;
		else
			dev->jig_off = 0;

	}
	if (deviceType & DEV_TYPE_UART) {
		dev->musb_dev_type = MUSB_DEV_UART;

		if (deviceType & (1 << BIT_JIG_UART_ON))
			dev->jig_on = 1;
		else
			dev->jig_on = 0;

		if (deviceType & (1 << BIT_JIG_UART_OFF))
			dev->jig_off = 1;
		else
			dev->jig_off = 0;

	}
	if (deviceType & DEV_TYPE_AV)
		dev->musb_dev_type = MUSB_DEV_AV;
	if (deviceType & DEV_TYPE_CHARGER) {
		if (dev->musb_dev_type == MUSB_DEV_NONE)
			dev->musb_dev_type = MUSB_DEV_ETC;
	}
	if ((adcValue == 0x17) || (adcValue == 0x1b))
		dev->musb_dev_type = MUSB_DEV_ETC;

	if (dev->manual_mode) {
		if (deviceType & DEV_TYPE_USB)
			tsu6712_config_for_USB(dev);
		if (deviceType & DEV_TYPE_UART)
			tsu6712_config_for_UART(dev);
		if (deviceType & DEV_TYPE_AV)
			tsu6712_config_for_AV(dev);
		if (deviceType & (1 << BIT_AUDIO_TYPE_1))
			tsu6712_config_for_MIC(dev);
		if (deviceType & DEV_TYPE_CHARGER)
			tsu6712_config_for_charger(dev);
		if ((adcValue == 0x17) || (adcValue == 0x1b))
			tsu6712_config_for_charger(dev);
	}

	if (dev->attach_callback)
		dev->attach_callback(dev->musb_dev_type);

	return 0;
}


static int tsu6712_process_reserved_attachment(struct tsu6712_device *dev,
						int deviceType, int adcValue)
{
	dev->temp_manual_sw = 1;

	dev->musb_dev_type = MUSB_DEV_ETC;
	tsu6712_change_to_manual_mode(dev);
	if (adcValue == 0x0f) {	/* R=34K, Reserved Accessory #2 */
		if (deviceType & (1 << BIT_JIG_USB_ON))
			dev->jig_on = 1;
		else
			dev->jig_on = 0;

		if (deviceType & (1 << BIT_JIG_USB_OFF))
			dev->jig_off = 1;
		else
			dev->jig_off = 0;

		tsu6712_config_for_USB(dev);
	} else if (adcValue == 0x11) {	/* R=49.9K, Reserved Accessory #4 */
		tsu6712_config_for_reserved4(dev);
	} else if (adcValue == 0x12) {	/* R=64.9K, Reserved Accessory #5 */
		tsu6712_config_for_reserved5(dev);
	}

	if (dev->attach_callback)
		dev->attach_callback(dev->musb_dev_type);

	return 0;
}


static int tsu6712_process_detachment(struct tsu6712_device *dev)
{
	if (dev->manual_mode) {
		tsu6712_config_idle(dev);
	} else {
		/* audio2USB or AV2USB, reserved accessary */
		if (dev->temp_manual_sw) {
			dev->temp_manual_sw = 0;
			tsu6712_config_idle(dev);
			tsu6712_change_to_automatic_mode(dev);
		}
	}

	if (dev->detach_callback)
		dev->detach_callback(dev->musb_dev_type);

	return 0;
}


static int tsu6712_process_interrupt(struct tsu6712_device *dev)
{
	u16 interruptType =0;
	u16 carkitIntType =0;
	u16 deviceType =0;
	u8 adcValue =0;
	u8 data =0;

	tsu6712_read_register(dev->i2c_client, REG_INTERRUPT_1, &data);
	interruptType = data;
	tsu6712_read_register(dev->i2c_client, REG_INTERRUPT_2, &data);
	interruptType |= (data << 8) & 0xff00;

	tsu6712_read_register(dev->i2c_client, REG_CARKIT_INT_1, &data);
	carkitIntType = data;
	tsu6712_read_register(dev->i2c_client, REG_CARKIT_INT_2, &data);
	carkitIntType |= (data << 8) & 0xff00;

	tsu6712_read_register(dev->i2c_client, REG_DEVICE_TYPE_1, &data);
	deviceType = data;
	tsu6712_read_register(dev->i2c_client, REG_DEVICE_TYPE_2, &data);
	deviceType |= (data << 8) & 0xff00;

	tsu6712_read_register(dev->i2c_client, REG_ADC, &adcValue);

	dev_info(dev->dev, "interrupt : %x\n", interruptType);
	dev_info(dev->dev, "carkit interrupt : %x\n", carkitIntType);
	dev_info(dev->dev, "device type : %x\n", deviceType);
	dev_info(dev->dev, "ADC value : %x\n", adcValue);

	/*dev->musb_dev_type = MUSB_DEV_NONE;*/

	if (interruptType & (1 << BIT_ATTACH)) {
		/* illegal state. attach & detach are both setted. */
		if (interruptType & (1 << BIT_DETACH)) {
			dev_err(dev->dev, "illegal state.");
			tsu6712_sw_reset(dev);
			tsu6712_init_muic(dev);

			if (dev->detach_callback)
				dev->detach_callback(dev->musb_dev_type);

			return 0;
		}

		tsu6712_process_attachment(dev, deviceType, adcValue);
	} else if (interruptType & (1 << BIT_RESERVED_ATTACH)) {
		tsu6712_process_reserved_attachment(dev, deviceType, adcValue);
	} else if (interruptType & (1 << BIT_DETACH)) {
		tsu6712_process_detachment(dev);
	}

	dev_info(dev->dev, "device type is %d\n", dev->musb_dev_type);

	return 0;
}


static void tsu6712_work_handler(struct work_struct *work)
{
	struct tsu6712_device *tsu6712 = container_of(work,
				struct tsu6712_device, work);

	mdelay(WAKEUP_TIMMING);
	tsu6712_process_interrupt(tsu6712);
}

static int tsu6712_setup_gpio(struct tsu6712_device *dev)
{
	int ret;

	ret = gpio_request(dev->intb, "TSU6712 Interrupt");
	if (ret != 0) {
		dev_err(dev->dev, "Interrupt GPIO request failed\n");
		goto free_device;
	}
	gpio_direction_input(dev->intb);

	dev->irq = gpio_to_irq(dev->intb);
	ret = request_irq(dev->irq, tsu6712_isr, IRQF_DISABLED,
				"TSU6712 Interrupt", dev);
	if (ret) {
		dev_err(dev->dev, "request IRQ failed\n");
		goto free_intb;
	}

	ret = irq_set_irq_type(dev->irq, IRQ_TYPE_EDGE_FALLING);
	if (ret) {
		dev_err(dev->dev, "set IRQ type failed\n");
		goto free_irq;
	}

	ret = gpio_request(dev->scl_gpio, "TSU6712 SCL Pin");
	if (ret != 0) {
		dev_err(dev->dev, "SCL GPIO request failed\n");
		goto free_irq;
	}
	ret = gpio_request(dev->sda_gpio, "TSU6712 SDA Pin");
	if (ret != 0) {
		dev_err(dev->dev, "SDA GPIO request failed\n");
		goto free_scl_gpio;
	}

	ret = tsu6712_init_muic(dev);
	if (ret != 0) {
		dev_err(dev->dev, "initialize failed\n");
		goto free_sda_gpio;
	}
	ret = tsu6712_process_interrupt(dev);
	if (ret != 0) {
		dev_err(dev->dev, "process state failed\n");
		goto free_sda_gpio;
	}

	return 0;
	
free_sda_gpio:
	gpio_free(dev->sda_gpio);
free_scl_gpio:
	gpio_free(dev->scl_gpio);
free_irq:
	free_irq(dev->irq, dev);
free_intb:
	gpio_free(dev->intb);
free_device:

	return ret;
}

static __devinit int tsu6712_i2c_probe(struct i2c_client *client,
				      const struct i2c_device_id *id)
{
	struct tsu6712_device *tsu6712 = NULL;
	struct tsu6712_platform_data *param = NULL;
	int ret;

	tsu6712 = kzalloc(sizeof(struct tsu6712_device), GFP_KERNEL);
	if (tsu6712 == NULL)
		return -ENOMEM;

	param = (struct tsu6712_platform_data *)client->dev.platform_data;
	if (param == NULL){
		ret = -EINVAL;
		goto faill_free;
	}

	tsu6712->dev = &client->dev;
	tsu6712->i2c_client = client;
	tsu6712->musb_dev_type = MUSB_DEV_NONE;

	tsu6712->pinmux_name = param->pinmux_name;
	tsu6712->scl_gpio = param->scl_gpio;
	tsu6712->sda_gpio = param->sda_gpio;
	if (param->intb == (unsigned)-1)
		tsu6712->intb = GPIO_JACK_INT;
	else
		tsu6712->intb = param->intb;
	tsu6712->manual_mode = param->manual_mode;
	tsu6712->attach_callback = param->attach_callback;
	tsu6712->detach_callback = param->detach_callback;

	INIT_WORK(&tsu6712->work, tsu6712_work_handler);

	i2c_set_clientdata(client, tsu6712);

	ret = tsu6712_setup_gpio(tsu6712);
	if (ret != 0)
		goto faill_free;

	return 0;

faill_free:
	kfree (tsu6712);
	return ret;
}


static __devexit int tsu6712_i2c_remove(struct i2c_client *client)
{
	struct tsu6712_device *tsu6712 = i2c_get_clientdata(client);

	if(tsu6712){
		gpio_free(tsu6712->sda_gpio);
		gpio_free(tsu6712->scl_gpio);
		gpio_free(tsu6712->intb);
		free_irq(tsu6712->irq, tsu6712);
		kfree(tsu6712);
	}
	return 0;
}

static __devinit int tsu6712_i2c_suspend(struct i2c_client *client, pm_message_t state)
{
	struct tsu6712_device *tsu6712 = i2c_get_clientdata(client);

	if(tsu6712){
		gpio_free(tsu6712->sda_gpio);
		gpio_free(tsu6712->scl_gpio);
		gpio_free(tsu6712->intb);
		free_irq(tsu6712->irq, tsu6712);
	}
	return 0;
}

static __devinit int tsu6712_i2c_resume(struct i2c_client *client)
{
	struct tsu6712_device *tsu6712 = i2c_get_clientdata(client);
	int ret;

	if(tsu6712){
		i2c_set_clientdata(client, tsu6712);

		ret = tsu6712_setup_gpio(tsu6712);
		if (ret != 0) {
			return ret;
		}
	}
	return 0;
}


static const struct i2c_device_id tsu6712_i2c_id[] = {
	{ TSU6712_I2C_DEV_NAME, 0 },
	{ }
};


static struct i2c_driver tsu6712_i2c_driver = {
	.driver = {
		.name = TSU6712_I2C_DEV_NAME,
		.owner = THIS_MODULE,
	},
	.probe = tsu6712_i2c_probe,
	.remove = __devexit_p(tsu6712_i2c_remove),
	.suspend = tsu6712_i2c_suspend,
	.resume = tsu6712_i2c_resume,
	.id_table = tsu6712_i2c_id
};


static __init int tsu6712_init(void)
{
	int ret = 0;

	ret = i2c_add_driver(&tsu6712_i2c_driver);
	if (ret) {
		printk(KERN_ERR "Failed to register TSU6712 I2C driver: %d\n",
		       ret);
	}

	return ret;
}


static __exit void tsu6712_exit(void)
{
	i2c_del_driver(&tsu6712_i2c_driver);
}

module_init(tsu6712_init);
module_exit(tsu6712_exit);

MODULE_AUTHOR("Hoin, Choi <goodman@acroem.com>");
MODULE_DESCRIPTION("TSU6712 Micro USB Switch IC driver");
MODULE_LICENSE("GPL");
MODULE_VERSION(DRV_VERSION);
