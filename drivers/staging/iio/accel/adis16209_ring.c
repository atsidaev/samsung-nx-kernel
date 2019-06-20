#include <linux/export.h>
#include <linux/interrupt.h>
#include <linux/mutex.h>
#include <linux/kernel.h>
#include <linux/spi/spi.h>
#include <linux/slab.h>

#include <linux/iio/iio.h>
#include "../ring_sw.h"
#include <linux/iio/trigger_consumer.h>
#include "adis16209.h"

/**
 * adis16209_read_ring_data() read data registers which will be placed into ring
 * @indio_dev: the IIO device
 * @rx: somewhere to pass back the value read
 **/
static int adis16209_read_ring_data(struct iio_dev *indio_dev, u8 *rx)
{
	struct spi_message msg;
	struct adis16209_state *st = iio_priv(indio_dev);
	struct spi_transfer xfers[ADIS16209_OUTPUTS + 1];
	int ret;
	int i;

	mutex_lock(&st->buf_lock);

	spi_message_init(&msg);

	memset(xfers, 0, sizeof(xfers));
	for (i = 0; i <= ADIS16209_OUTPUTS; i++) {
		xfers[i].bits_per_word = 8;
		xfers[i].cs_change = 1;
		xfers[i].len = 2;
		xfers[i].delay_usecs = 30;
		xfers[i].tx_buf = st->tx + 2 * i;
		st->tx[2 * i]
			= ADIS16209_READ_REG(ADIS16209_SUPPLY_OUT + 2 * i);
		st->tx[2 * i + 1] = 0;
		if (i >= 1)
			xfers[i].rx_buf = rx + 2 * (i - 1);
		spi_message_add_tail(&xfers[i], &msg);
	}

	ret = spi_sync(st->us, &msg);
	if (ret)
		dev_err(&st->us->dev, "problem when burst reading");

	mutex_unlock(&st->buf_lock);

	return ret;
}

/* Whilst this makes a lot of calls to iio_sw_ring functions - it is to device
 * specific to be rolled into the core.
 */
static irqreturn_t adis16209_trigger_handler(int irq, void *p)
{
	struct iio_poll_func *pf = p;
	struct iio_dev *indio_dev = pf->indio_dev;
	struct adis16209_state *st = iio_priv(indio_dev);
	struct iio_buffer *ring = indio_dev->buffer;
	int i = 0;
	s16 *data;

	data = kmalloc(indio_dev->scan_bytes, GFP_KERNEL);
	if (data == NULL) {
		dev_err(&st->us->dev, "memory alloc failed in ring bh");
		return -ENOMEM;
	}

	if (!bitmap_empty(indio_dev->active_scan_mask, indio_dev->masklength) &&
	    adis16209_read_ring_data(indio_dev, st->rx) >= 0)
		for (; i < bitmap_weight(indio_dev->active_scan_mask,
					 indio_dev->masklength); i++)
			data[i] = be16_to_cpup((__be16 *)&(st->rx[i*2]));

	/* Guaranteed to be aligned with 8 byte boundary */
	if (indio_dev->scan_timestamp)
		*((s64 *)(data + ((i + 3)/4)*4)) = pf->timestamp;

	ring->access->store_to(ring, (u8 *)data, pf->timestamp);

	iio_trigger_notify_done(indio_dev->trig);
	kfree(data);

	return IRQ_HANDLED;
}

void adis16209_unconfigure_ring(struct iio_dev *indio_dev)
{
	iio_dealloc_pollfunc(indio_dev->pollfunc);
	iio_sw_rb_free(indio_dev->buffer);
}

static const struct iio_buffer_setup_ops adis16209_ring_setup_ops = {
	.preenable = &iio_sw_buffer_preenable,
	.postenable = &iio_triggered_buffer_postenable,
	.predisable = &iio_triggered_buffer_predisable,
};

int adis16209_configure_ring(struct iio_dev *indio_dev)
{
	int ret = 0;
	struct iio_buffer *ring;

	ring = iio_sw_rb_allocate(indio_dev);
	if (!ring) {
		ret = -ENOMEM;
		return ret;
	}
	indio_dev->buffer = ring;
	ring->scan_timestamp = true;
	indio_dev->setup_ops = &adis16209_ring_setup_ops;

	indio_dev->pollfunc = iio_alloc_pollfunc(&iio_pollfunc_store_time,
						 &adis16209_trigger_handler,
						 IRQF_ONESHOT,
						 indio_dev,
						 "%s_consumer%d",
						 indio_dev->name,
						 indio_dev->id);
	if (indio_dev->pollfunc == NULL) {
		ret = -ENOMEM;
		goto error_iio_sw_rb_free;
	}

	indio_dev->modes |= INDIO_BUFFER_TRIGGERED;
	return 0;

error_iio_sw_rb_free:
	iio_sw_rb_free(indio_dev->buffer);
	return ret;
}
