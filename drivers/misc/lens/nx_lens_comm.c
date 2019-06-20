/**
 * @file nx_lens_comm.c
 * @brief Experimental NX series lens communication manager driver
 * @author Hong yeol choi <chykrkr@acroem.com>
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/ioctl.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/miscdevice.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/wait.h>
#include <linux/sched.h>

#include <linux/fb.h>
#include <linux/hs_spi.h>

#include <linux/spi/lens_comm.h>
#include <linux/interrupt.h>
#include <linux/irqreturn.h>
#include <linux/poll.h>

#include <linux/io.h>
#include <linux/uaccess.h>

#include <mach/gpio.h>
#include <mach/map.h>
#include <linux/spi/d4_spi_config.h>


//#define USE_WORK_QUEUE 1
/**
 * It seems that there are cases
 * physical signal and fifo queue data isn't match when using DMA read.
 * (Especially at low clock ...)
 * If you need DMA feature for mode 0 write, undefine it and use DMA feature
 * at your own risk.
 */
#undef DMA_CHIP_BUG_NOT_RESOLVED

#undef USE_HEX_DUMP

/**
 *  TP for Lens debug.
 *  Please do not use it as is, ask your H/W team to know GPIO pin number
 *  that can be used as TP safely.
*/
#undef LENS_TP /*DRIME4_GPIO15(0)*/

#define INSURE_SPI_PINMUX_SEQ

/**
 * Circular buffer size that is used not to abandon rb interrupts.
 */
#define CIRC_RB_BUF_SIZE 512

#define DRV_VERSION     "0.8"

/**
 * Maximum clock hz that can be used stable.
 */
#define MAX_STABLE_SPI_CLOCK    2531300

/**
 * SPI message buffer size.
 */
static unsigned bufsiz = 10*1024; //lens read buffer size check!!!! (JSH_2012.0618) //12-24 colorshading value is over 2238
module_param(bufsiz, uint, S_IRUGO);
MODULE_PARM_DESC(bufsiz, "data bytes in biggest supported SPI message");

/**
 * Structure for Inter bytes delay times in milli second
 * that is used for each SPI frequency.
 * It's value is used to insure at least 4 milli seconds inter bytes delay.
 */
struct freq_to_delay_elem {
	__u32 clock;    /*SPI clock frequency */
	__u32 mode0_w;  /*Inter bytes delay for transfer mode 0 write*/
	__u32 mode1_w;  /*Inter bytes delay for transfer mode 1 write*/
	__u32 mode1_r;  /*Inter bytes delay for transfer mode 1 read*/
};

DECLARE_WAIT_QUEUE_HEAD(int_poll);

#ifdef CIRC_RB_BUF_SIZE
struct rb_circ_buf {
	unsigned char *buf;
	int head;
	int tail;
	int size;

	/*lock for producer*/
	spinlock_t p_lock;

	/*lock for consumer*/
	spinlock_t c_lock;
};

struct rb_circ_buf cbuf;
#endif

struct spidev_data {
	dev_t			devt;
	spinlock_t		spi_lock;
	spinlock_t              int_lock;

	struct hs_spi_data        *spi;
	struct d4_hs_spi_config   spi_data;
	struct freq_to_delay_elem *delay_times;

	wait_queue_head_t       wq_int_poll;

	struct mutex		buf_lock;
	unsigned		users;

	/* buffer is NULL unless this device is open (users > 0) */
	u8			*buffer;

	struct work_struct rb_work;
	struct work_struct det_work;

	u8 int_status;

	u8 condition;
};


static int prev_int_values[] = {0xFF, 0xFF};
static u8 prev_int_data = 0xFF;

#ifdef INSURE_SPI_PINMUX_SEQ
enum eLens_spi_status {
	LENS_SPI_STATUS_NONE,
	LENS_SPI_STATUS_DISABLE,
	LENS_SPI_STATUS_ENABLE,
	LENS_SPI_STATUS_CLOSE,
};

static enum eLens_spi_status next_spi_status = LENS_SPI_STATUS_DISABLE;
#endif

static struct spidev_data *gSpidev;

/**
 * Mapping table to get irq number from gpio number.
 * It's elements must be GPIO id.
 */
static unsigned int irq_map[] = {
	(unsigned int)-1, // GPIO_LENS_RB,
	(unsigned int)-1, // GPIO_LENS_DET,
};

/**
 * IRQ status for lens interrupts.
 * This status indicates that interrupt is enabled or disabled.
 * IRQ Disabled  0
 * IRQ Enalbed   1
 */
static unsigned int irq_status[] = {
	0, /* irq status for RB */
	0, /* irq status for DET */
};

static struct freq_to_delay_elem min_delay = {
	/*Clock value is not important*/
	.clock   = 0,
	.mode0_w = 0,
	.mode1_w = 0,
	.mode1_r = 0,
};

static struct freq_to_delay_elem max_delay =  {
	/*Clock value is not important*/
	.clock   = 0,
	.mode0_w = 4,
	.mode1_w = 4,
	.mode1_r = 4,
};

/**
 * Delay time table to insure 4 us inter byte delay.
 * I measured 316400 2531300 case only.
 */
static struct freq_to_delay_elem map_freq_to_delay[] = {
	/*
	  clock		mode0_w		mode1_w		mode1_r
	*/
	{13200,		0,		0,		0},
	{26400,		0,		0,		0},
	{39600,		0,		0,		0},
	{48800,		0,		0,		0},
	{52700,		0,		0,		0},
	{79100,		0,		0,		0},
	{97700,		0,		0,		0},
	{105500,	0,		0,		0},
	{158200,	0,		0,		0},
	{195300,	0,		0,		0},

	{210900,	0,		0,		0},

	{316400,	0,		0,		0},

	{390600,	0,		0,		0},
	{421900,	0,		0,		0},
	{632800,	0,		0,		0},
	{781300,	0,		0,		0},
	{843800,	0,		0,		0},
	{1265600,	0,		0,		0},
	{1562500,	0,		0,		0},
	{1687500,	3,		3,		3},

	{2531300,	4,		2,		2},

	/*
	 * Delay times must be higher than 2531300 case.
	 */
	{3125000,	4,		2,		2},
	{3375000,	4,		2,		2},
	{5062500,	4,		2,		2},
	{6250000,	4,		2,		2},
	{6750000,	4,		2,		2},
	{10125000,	4,		2,		2},
	{12500000,	4,		2,		2},
	{13500000,	4,		2,		2},
	{20250000,	4,		2,		2},

	{25000000,	4,		2,		2},
	{40500000,	4,		2,		2},
	{50000000,	4,		2,		2},
};

#ifdef CIRC_RB_BUF_SIZE
/**
 * @brief Push data to given circular buffer
 * @param buffer Circular buffer that is used as fifo queue
 * @param data Data to be pushed to circular buffer
 */
static inline void push_to_circ_buf(struct rb_circ_buf *buffer,
					 unsigned char data)
{
	unsigned long head;
	unsigned long tail;

	spin_lock(&buffer->p_lock);

	head = buffer->head;
	tail = ACCESS_ONCE(buffer->tail);

	if (CIRC_SPACE(head, tail, CIRC_RB_BUF_SIZE) >= 1) {
		/* insert one item into the buffer */
		*(buffer->buf + head) = data;

		smp_wmb(); /* commit the item before incrementing the head */

		buffer->head = (head + 1) & (buffer->size - 1);

		/* wake_up() will make sure that the head is committed before
		 * waking anyone up */
		/*wake_up(consumer);*/
	} else {
		printk(KERN_ERR "queue full\n");
		while (1)
			;
	}

	/*
	   Else case shall be unavoidable rb interrrupt loss
	   due to buffer size
	*/

	spin_unlock(&buffer->p_lock);
}

/**
 * @brief Pop up item from circular buffer.
 * @param buffer Circular buffer that is used as fifo queue
 * @param data Pointer to data that is user to fetch an item from queue
 * @return If this function fails, it return negative value.
 */
static inline int pop_from_circ_buf(struct rb_circ_buf *buffer,
					  unsigned char *data)
{
	int result = -1;
	unsigned long head;
	unsigned long tail;

	spin_lock(&buffer->c_lock);

	head = ACCESS_ONCE(buffer->head);
	tail = buffer->tail;

	if (CIRC_CNT(head, tail, buffer->size) >= 1) {
		/* read index before reading contents at that index */
		smp_read_barrier_depends();

		/* extract one item from the buffer */
		*data = *(buffer->buf + tail);

		/* finish reading descriptor before incrementing tail */
		smp_mb();

		buffer->tail = (tail + 1) & (buffer->size - 1);

		result = 0;
	}

	spin_unlock(&buffer->c_lock);

	return result;
}

/**
 * @brief Get item count for given buffer
 * @param buffer Circular buffer that's item count to be calculated.
 */
static inline int get_rb_circ_buf_count(struct rb_circ_buf *buffer)
{
	int result;
	unsigned long head;
	unsigned long tail;

	head = ACCESS_ONCE(buffer->head);
	tail = buffer->tail;

	result = CIRC_CNT(head, tail, buffer->size);

	return result;
}

/**
 * @brief Get item exist 
 * @param buffer Circular buffer that's item count to be calculated.
 */
static inline int get_exist_event(struct rb_circ_buf *buffer)
{
	int result;
	unsigned long head;
	unsigned long tail;

	head = ACCESS_ONCE(buffer->head);
	tail = buffer->tail;

	result = CIRC_CNT(head, tail, buffer->size);

	if(gSpidev->int_status & 0x02)
		result = result + 1;

	return result;
}


#endif

/**
 * Save interrupt status. If interrupt is RB then it status is queued.
 * @param spidev
 * @param int_no	0 : RB
 *			1 : DET
 * @param value It's value must be 0 or 1
 */
static inline void set_int_status(struct spidev_data *spidev, int int_no,
				  int value)
{

	spin_lock(&spidev->int_lock);

	if (value != prev_int_values[int_no]) {
		spidev->int_status |= 1 << int_no;

		if (value)
			spidev->int_status |= (1 << int_no) << 4;
		else
			spidev->int_status &= ~((1 << int_no) << 4);

		prev_int_values[int_no] = value;

#ifdef CIRC_RB_BUF_SIZE
		if (int_no == 0)
			push_to_circ_buf(&cbuf, value);
#endif
	}

	spin_unlock(&spidev->int_lock);
}

/**
 * @brief
 * @return latest interrupt status for RB and DET.
 *         But if CIRC_RB_BUF_SIZE define statement is effective
 *         then interrupt status for RB indicates latest interrupt
 *         status that is not handled yet.
 */
static inline u8 get_and_clear_int_status(struct spidev_data *spidev)
{
	u8 ret;
#ifdef CIRC_RB_BUF_SIZE
	unsigned char temp;
#endif


	spin_lock(&spidev->int_lock);
	ret = spidev->int_status;

#ifdef CIRC_RB_BUF_SIZE
	/* pop success */
	if (pop_from_circ_buf(&cbuf, &temp) >= 0) {
		ret |= 1; /* set rb int flag */

		if (temp)
			ret |= 1 << 4;
		else
			ret &= ~(1 << 4);
	}
#endif

	spidev->int_status &= 0xF0;


	spin_unlock(&spidev->int_lock);

	return ret;
}

/**
 * Update spi inter byte delay times according to SPI current SPI clock.
 * This function must be called after you changed SPI clock setting.
 */
static inline void update_delay_time(struct spidev_data *spidev)
{
	unsigned int speed_hz;
	int count_clocks;
	int i;
	struct freq_to_delay_elem *elem = NULL;
	struct freq_to_delay_elem *next_elem = NULL;

	count_clocks = sizeof(map_freq_to_delay) / sizeof(map_freq_to_delay[0]);

	speed_hz = spidev->spi_data.speed_hz;

	if (speed_hz < 13200) {
		spidev->delay_times = &min_delay;
		spidev->delay_times->clock = speed_hz;
		return;
	}

	if (speed_hz > 50000000) {
		spidev->delay_times = &max_delay;
		spidev->delay_times->clock = speed_hz;
		return;
	}

	spidev->delay_times = NULL;

	for (i = 0 ; i < count_clocks ; i++) {
		elem = &(map_freq_to_delay[i]);

		if (i == count_clocks - 1)
			next_elem = NULL;
		else
			next_elem = &(map_freq_to_delay[i + 1]);

		if (speed_hz == elem->clock || next_elem == NULL ||
		    speed_hz < next_elem->clock) {
			spidev->delay_times = elem;
			break;
		}
	}

#if 0
	printk(KERN_ERR "clock = %u, mode0_w = %u, mode1_w = %u, mode1_r = %u\n",
	       spidev->delay_times->clock,
	       spidev->delay_times->mode0_w,
	       spidev->delay_times->mode1_w,
	       spidev->delay_times->mode1_r
		);
#endif
}

/**
 * Initialization code for lens related gpio pins.
 */
static void init_gpios(void)
{
	int error;

#ifdef LENS_TP
	error = gpio_request(LENS_TP, "lens_tp");
	error = gpio_direction_output(LENS_TP, 0);
	error = gpio_export(LENS_TP, true);
#endif

	error = gpio_request(GPIO_LENS_3_3V_ON, "");
	error = gpio_direction_output(GPIO_LENS_3_3V_ON, 0);
	error = gpio_export(GPIO_LENS_3_3V_ON, true);

	error = gpio_request(GPIO_LENS_5_0V_ON, "");
	error = gpio_direction_output(GPIO_LENS_5_0V_ON, 0);
	error = gpio_export(GPIO_LENS_5_0V_ON, true);

	error = gpio_request(GPIO_LENS_RB, "");
	error = gpio_direction_input(GPIO_LENS_RB);
	error = gpio_export(GPIO_LENS_RB, true);

	error = gpio_request(GPIO_LENS_DET, "");
	error = gpio_direction_input(GPIO_LENS_DET);
	error = gpio_export(GPIO_LENS_DET, true);

	error = gpio_request(GPIO_LENS_SYNC_EN1, "");
	error = gpio_direction_output(GPIO_LENS_SYNC_EN1, 0);
	error = gpio_export(GPIO_LENS_SYNC_EN1, true);

	error = gpio_request(GPIO_LENS_SYNC_EN2, "");
	error = gpio_direction_output(GPIO_LENS_SYNC_EN2, 0);
	error = gpio_export(GPIO_LENS_SYNC_EN2, true);

	error = gpio_request(GPIO_LENS_SCK, "");
	error = gpio_direction_output(GPIO_LENS_SCK, 0);
	error = gpio_export(GPIO_LENS_SCK, true);

	error = gpio_request(GPIO_LENS_DIN, "");
	error = gpio_direction_output(GPIO_LENS_DIN, 0);
	error = gpio_export(GPIO_LENS_DIN, true);

	error = gpio_request(GPIO_LENS_DOUT, "");
	error = gpio_direction_output(GPIO_LENS_DOUT, 0);
	error = gpio_export(GPIO_LENS_DOUT, true);
}

#ifdef USE_WORK_QUEUE
/**
 * Work queue handler for RB interrupts.
 * It does just to wake up poll request.
 */
static void rb_work(struct work_struct *work)
{
	struct spidev_data *spidev;
	/* Wake up process that waits polling return */
	/*printk(KERN_ERR "%s : I'm working\n", __func__);*/

	spidev = container_of(work, struct spidev_data, rb_work);

	wake_up_interruptible(&spidev->wq_int_poll);

	/*enable_irq(gpio_to_irq(GPIO_LENS_RB));*/
}

/**
 * Work queue handler for DET interrupts.
 * It does just to wake up poll request.
 */
static void det_work(struct work_struct *work)
{
	struct spidev_data *spidev;
	/* Wake up process that waits polling return */
	/*printk(KERN_ERR "%s : I'm working\n", __func__);*/
	spidev = container_of(work, struct spidev_data, det_work);

	wake_up_interruptible(&spidev->wq_int_poll);

	/*enable_irq(gpio_to_irq(GPIO_LENS_DET));*/
}
#endif


/* Interrupt handler related - start -*/
/**
 * Kernel level IRQ handler for RB interrupt.
 * Beware that after physical signal's level is changed it takes 8~10 us
 * to reach IRQ handler routine.
 * So if RB interrupt signal's level is changed more than one time in 8~10 us
 * then interrupt abandon is unavoidable as long as you use normal kernel API.
 */
static irqreturn_t rb_int_isr(int irq, void *data)
{
	struct spidev_data *spidev = data;

	int gpio_status;
	gpio_status = gpio_get_value(GPIO_LENS_RB);

	set_int_status(spidev, 0, gpio_status);

	/* We should not abandon even a single RB interrupt! */
#ifdef USE_WORK_QUEUE
	schedule_work(&spidev->rb_work);
#else
	//printk("rb_int_isr %d\n",gpio_status);
	wake_up_interruptible(&spidev->wq_int_poll);
#endif

	return IRQ_HANDLED;
}

/**
 * @brief Kernel level IRQ handler for DET interrupt.
 */
static irqreturn_t det_int_isr(int irq, void *data)
{
	struct spidev_data *spidev = data;
	int gpio_status;
	gpio_status = gpio_get_value(GPIO_LENS_DET);

	set_int_status(spidev, 1, gpio_status);

#ifdef USE_WORK_QUEUE
	if (!work_pending(&spidev->det_work)) {
		/*disable_irq_nosync(irq);*/
		schedule_work(&spidev->det_work);
	}
#else
	//printk("det_int_isr %d\n",gpio_status);
	wake_up_interruptible(&spidev->wq_int_poll);
#endif
	return IRQ_HANDLED;
}

/**
 * @brief Helper function to request lens related IRQ
 */
static int request_lens_int(irq_handler_t int_handler, int gpio,
			    int irqflags, void *data)
{
	int error, irq;

	irq = gpio_to_irq(gpio);

	if (irq < 0) {
		error = irq;
		printk(KERN_ERR "Unable to get irq number\n");

		goto fail;
	}

	error = request_irq(irq, int_handler, irqflags, "", data);

	if (error) {
		printk(KERN_ERR "Unable to request irq\n");
		goto fail;
	}

	return 0;

fail:
	return error;
}
/* Interrupt handler related - end -*/

#ifdef USE_HEX_DUMP
/**
 * @brief helper function to dump data in hex
 */
inline void print_hex(void *buf, size_t count)
{
	int i;
	unsigned char *buffer = buf;

	for (i = 0 ; i < count ; i++)
		printk(KERN_ERR "%02X", *(buffer + i));

	printk(KERN_ERR "\n");
}
#else
#define print_hex(BUF, count)
#endif


/**
 * Mode 0 SPI write.
 * It transfers SPI data by bytes with inter bytes delay of 4 us.
 * @param spidev
 * @param lens SPI message lengths in bytes
 * @param usecs It's not used anymore refactoring required.
 */
ssize_t spi_write_fixed_delay(struct spidev_data *spidev,
				     size_t len, u16 usecs)
{
	int i;
	unsigned long inter_byte_delay = spidev->delay_times->mode0_w;
	struct hs_spi_data	*spi = spidev->spi;
	struct spi_data_info	t = {
		.data_len	= 1,
		.wbuffer	= spidev->buffer,
		.rbuffer	= NULL
	};

	if (inter_byte_delay > 0)
		for (i = 0 ; i < len ; i++) {
			hs_spi_polling_write(spi, &t);
			t.wbuffer++;

			udelay(inter_byte_delay);
		}
	else {
		t.data_len = len;
		hs_spi_polling_write(spi, &t);
	}

	return 0;
}

/**
 * Mode 1 SPI write
 * It transfers SPI data by bytes with inter bytes delay of 4 us.
 * And checks collision status.
 *
 * @param spidev
 * @param lens SPI message lengths in bytes
 * @param usecs It's not used anymore refactoring required.
 * @param col_ret If there are collision this data must be non-zero value.
 * @return If there are collision it returns -ERRNO_COLLISION (negative)
 */
ssize_t spi_write_chk_col(struct spidev_data *spidev,
				 size_t len, u16 usecs, u8 *col_ret)
{
	u8 rx_data = 0;
	unsigned long inter_byte_delay = spidev->delay_times->mode1_w;
	struct hs_spi_data	*spi = spidev->spi;
	struct spi_data_info	t = {
		.data_len	= 1,
		.wbuffer	= spidev->buffer,
		.rbuffer	= &rx_data,
	};
	int i;

	if (gpio_get_value(GPIO_LENS_RB) == 0)
		return -ERRNO_FALSE_COLLISION;

	/*Ugly but efficient ...*/
	if (inter_byte_delay > 0)
		for (i = 0 ; i < len ; i++) {
			hs_spi_polling_rw(spi, &t);

			if (gpio_get_value(GPIO_LENS_RB) == 0 &&
			    rx_data != 0x0) {

				if (col_ret)
					*col_ret = rx_data;
#ifdef LENS_TP
				do {
					static int togg_val = 1;
					gpio_set_value(LENS_TP, togg_val);
					togg_val = !togg_val;
				} while (0);
#endif

				return -ERRNO_COLLISION;
			}
			t.wbuffer++;

			udelay(inter_byte_delay);
		}
	else
		for (i = 0 ; i < len ; i++) {
			hs_spi_polling_rw(spi, &t);

			if (gpio_get_value(GPIO_LENS_RB) == 0 &&
			    rx_data != 0x0) {

				if (col_ret)
					*col_ret = rx_data;

				return -ERRNO_COLLISION;
			}
			t.wbuffer++;
		}

	return 0;
}

/**
 * Mode 1 SPI read
 * It receives SPI data by bytes with inter bytes delay of 4 us.
 *
 * @param spidev
 * @param lens SPI message lengths in bytes
 * @param usecs It's not used anymore refactoring required.
 */
ssize_t spi_read_fixed_delay(struct spidev_data *spidev,
				    size_t *len, u16 usecs)
{
	unsigned long inter_byte_delay = spidev->delay_times->mode1_r;
	struct hs_spi_data	*spi = spidev->spi;
	struct spi_data_info	t = {
		.data_len	= 1,
		.wbuffer	= NULL,
		.rbuffer	= spidev->buffer
	};

	int i;

	/*Ugly but efficient ...*/
	if (inter_byte_delay > 0)
		for (i = 0 ; i < *len ; i++) {

			if (gpio_get_value(GPIO_LENS_RB) == 1)
			{
				*len = i; 
				break;
			}
			
			hs_spi_polling_read(spi, &t);
			t.rbuffer++;

			udelay(inter_byte_delay);
		}
	else {
		t.data_len = *len;
		hs_spi_polling_read(spi, &t);
	}

	return 0;
}

/**
 * Mode 0 SPI read
 * It receives SPI data by bytes with no inter bytes delay
 * It recommend to use DMA transfer but when using DMA transfer there are cases
 * that physical signal is not match to received data.
 * You can turn or DMA feature by undefine DMA_CHIP_BUG_NOT_RESOLVED.
 * Try it yourself and use dma feature at your own risk.
 *
 * @param spidev
 * @param lens SPI message lengths in bytes
 */
ssize_t spi_dma_read(struct spidev_data *spidev, size_t len)
{
#ifndef DMA_CHIP_BUG_NOT_RESOLVED
	struct hs_spi_data	*spi = spidev->spi;
	struct spi_data_info	t = {
		.data_len	= len,
		.wbuffer	= NULL,
		.rbuffer	= spidev->buffer
	};

	hs_spi_dma_transfer(spi, &t);

	/*
	  Chip bug confirm print ...
	  Signal wave and buffer data is not equal somtimes.
	  0x03AA55
	  0x000320
	*/
#if 0
	printk(KERN_ERR "*BUF [%d]: ", len);
	print_hex(spidev->buffer,
		  (len <= 3) ? len : 3);
#endif

	return 0;
#else
	struct hs_spi_data	*spi = spidev->spi;
	struct spi_data_info	t = {
		.data_len	= len,
		.wbuffer	= NULL,
		.rbuffer	= spidev->buffer
	};

	hs_spi_polling_read(spi, &t);
	return 0;
#endif
}

/* Read-only message with current device setup */

/**
 * Kernel level read function.
 * read must be called after poll or select in user level application.
 */
ssize_t
lensdev_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
	struct spidev_data	*spidev;
	ssize_t			status = 0;
	int missing;

	u8 int_data;


	if (count > bufsiz) {
		status = -EMSGSIZE;
		goto fail;
	}

	spidev = filp->private_data;

	int_data = get_and_clear_int_status(spidev);

	if ((int_data == prev_int_data)||((int_data&0xf)==0))
		status = 0;
	else
		status = 1+get_rb_circ_buf_count(&cbuf);

	prev_int_data = int_data;

	missing = copy_to_user(buf, &int_data, 1);

	if (missing > 0) {
		status = -EFAULT;
		goto fail;
	}

fail:
	return status;
}


static int lens_comm_open(struct inode *inode, struct file *filp)
{
	int			status = -ENXIO;

	filp->private_data = gSpidev;

	if (!gSpidev->buffer) {
		gSpidev->buffer = kmalloc(bufsiz, GFP_KERNEL);
		if (!gSpidev->buffer) {
			/*dev_dbg(&gSpidev->spi->dev, "open/ENOMEM\n");*/
			status = -ENOMEM;
		}
	}

	nonseekable_open(inode, filp);
	status = 0;

	return status;
}

static int lens_comm_close(struct inode *inode, struct file *filp)
{
	struct spidev_data	*spidev;

	spidev = filp->private_data;
	filp->private_data = NULL;

	wake_up_interruptible(&spidev->wq_int_poll);

	kfree(spidev->buffer);

	return 0;
}

static void lens_mem_ctrl(struct spidev_data *spidev, unsigned int value)
{
	if (value) {
		if (!spidev->buffer)
			spidev->buffer = kmalloc(bufsiz, GFP_KERNEL);
	} else {
		if (spidev->buffer)
			kfree(spidev->buffer);
	}
}



/**
 * ioctl handling function.
 */
long lens_comm_ioctl(struct file *filp, unsigned int cmd,
			    unsigned long arg)
{
	int			retval = 0;
	struct spidev_data	*spidev;
	struct d4_hs_spi_config *spi_data;
	u32			tmp;
	struct lens_spi_ioc_transfer lens_transfer;
	struct lens_spi_ioc_receive lens_receive;
	
	struct lens_irq_control irq_control;
	struct lens_gpio_control gpio_control;
	unsigned int irq;
	unsigned int value;
	u8 col_ret = 0x0;
	int retcol;
	u16 gpio_id;
	u8  gpio_value;
	u8  gpio_dir;

	size_t local_len; 

	/* Check type and command number */
	if (_IOC_TYPE(cmd) != SPI_IOC_MAGIC)
		return -ENOTTY;

	spidev = filp->private_data;
	spin_lock(&spidev->spi_lock);
	spi_data = &(spidev->spi_data);
	spin_unlock(&spidev->spi_lock);

	if (spi_data == NULL)
		return -ESHUTDOWN;

	/* use the buffer lock here for triple duty:
	 *  - prevent I/O (from us) so calling spi_setup() is safe;
	 *  - prevent concurrent SPI_IOC_WR_* from morphing
	 *    data fields while SPI_IOC_RD_* reads them;
	 *  - SPI_IOC_MESSAGE needs the buffer locked "normally".
	 */
	mutex_lock(&spidev->buf_lock);

	switch (cmd) {
	case SPI_IOC_WR_MAX_SPEED_HZ:
		/*Changing clock is buggy.*/
		if (__copy_from_user(&tmp, (void __user *)arg, sizeof(u32))) {
			retval = -EFAULT;
			break;
		}

		if (tmp > MAX_STABLE_SPI_CLOCK) {
			retval = -EFAULT;
			break;
		}

		spi_data->speed_hz = tmp;
		spi_data->setup_select = SH_SPI_SPEED;

		hs_spi_config(spidev->spi, spi_data);

		spidev->spi_data.setup_select = SH_SPI_SPEED|
			SH_SPI_BPW|SH_SPI_WAVEMODE|
			SH_SPI_WAITTIME|SH_SPI_INVERT|SH_SPI_BURST;

		retval = 0;

		update_delay_time(spidev);
		break;

	case LENS_IOC_SPI_WR_FIXED_DELAY:
		if (__copy_from_user(&lens_transfer, (void __user *)arg,
				     sizeof(struct lens_spi_ioc_transfer))) {
			retval = -EFAULT;
			break;
		}

		if (__copy_from_user(spidev->buffer,
				     (void __user *)lens_transfer.tx_buf,
				     lens_transfer.len)) {
			retval = -EFAULT;
			break;
		}
		
		if (spi_write_fixed_delay(spidev, lens_transfer.len,
					  lens_transfer.delay_usecs)) {
			retval = -EFAULT;
			break;
		}

		break;

	case LENS_IOC_SPI_WR_COL_CHK:
		/*Make sure default value to 0x0 (No collision)*/
		col_ret = 0x0;
		if (__copy_from_user(&lens_transfer, (void __user *)arg,
				     sizeof(struct lens_spi_ioc_transfer))) {
			retval = -EFAULT;
			break;
		}

		if (__copy_from_user(spidev->buffer,
				     (void __user *)lens_transfer.tx_buf,
				     lens_transfer.len)) {
			retval = -EFAULT;
			break;
		}

		retcol = spi_write_chk_col(spidev, lens_transfer.len,
					   lens_transfer.delay_usecs,
					   &col_ret);

		if (retcol == -ERRNO_FALSE_COLLISION) {
			retval = retcol;
			break;
		}

		if (lens_transfer.rx_buf) {
			retval = copy_to_user(
				(void __user *)lens_transfer.rx_buf,
				&col_ret, sizeof(u8));
		} else
			retval = 0;

		if (retval > 0) {
			retval = -EFAULT;
			break;
		}

		retval = retcol;
		break;

	case LENS_IOC_SPI_RD_FIXED_DELAY:
		if (__copy_from_user(&lens_receive, (void __user *)arg,
				     sizeof(struct lens_spi_ioc_receive))) {
			retval = -EFAULT;
			break;
		}

		if (__copy_from_user(&local_len,
			     (void __user *)lens_receive.p_len,
			     sizeof(size_t))) {
			retval = -EFAULT;
			break;
		}
		
		if (spi_read_fixed_delay(spidev, &local_len,
					 lens_receive.delay_usecs)) {

			retval = -EFAULT;
			break;
		}

		retval = copy_to_user((void __user *)lens_receive.rx_buf,
				      spidev->buffer,
					  local_len);
		retval = copy_to_user((void __user *)lens_receive.p_len,
				      &local_len,
					  sizeof(size_t));

		if (retval > 0) {
			retval = -EFAULT;
			break;
		}
		break;

	case LENS_IOC_SPI_DMA_RD:
		if (__copy_from_user(&lens_receive, (void __user *)arg,
				     sizeof(struct lens_spi_ioc_receive))) {

			printk(KERN_ERR "SPI DMA FAILED\n");
			retval = -EFAULT;
			break;
		}		

		if (__copy_from_user(&local_len,
			     (void __user *)lens_receive.p_len,
			     sizeof(size_t))) {
			retval = -EFAULT;
			break;
		}
		
		if (spi_dma_read(spidev, local_len)) {
			retval = -EFAULT;
			break;
		}

		retval = copy_to_user(
			(void __user *)lens_receive.rx_buf,
			spidev->buffer,
			local_len);

		if (retval > 0) {
			retval = -EFAULT;
			break;
		}

		break;

	case LENS_IOC_ENABLE_IRQ:
		if (__copy_from_user(&irq_control, (void __user *)arg,
				     sizeof(struct lens_irq_control))) {

			printk(KERN_ERR "IRQ control failed\n");
			retval = -EFAULT;
			break;
		}

		if (irq_control.irqtype != LENS_IRQ_TYPE_RB &&
		    irq_control.irqtype != LENS_IRQ_TYPE_DET) {
			retval = -EINVAL;
			break;
		}

		irq = gpio_to_irq(irq_map[irq_control.irqtype]);

		/*Make sure 0 or 1*/
		irq_control.enable = !!irq_control.enable;

		/* Double disable or enable prohibited */
		if (irq_control.enable == irq_status[irq_control.irqtype])
			break;

		cbuf.head = cbuf.tail = 0; 
		
		if (irq_control.enable)
			enable_irq(irq);
		else
			disable_irq(irq);

		irq_status[irq_control.irqtype] = irq_control.enable;
		break;

	case LENS_IOC_REQ_FIN:
		wake_up_interruptible(&spidev->wq_int_poll);
		break;

	case LENS_IOC_SPI_DISABLE:
#ifdef INSURE_SPI_PINMUX_SEQ
		if (next_spi_status != LENS_SPI_STATUS_DISABLE)
			break;
#endif

		hs_spi_fix(spidev->spi, SH_SPI_FIX_DIS);

#ifdef INSURE_SPI_PINMUX_SEQ
		next_spi_status = LENS_SPI_STATUS_ENABLE;
#endif
		break;

	case LENS_IOC_SPI_ENABLE:
#ifdef INSURE_SPI_PINMUX_SEQ
		if (next_spi_status != LENS_SPI_STATUS_ENABLE)
			break;
#endif

		hs_spi_fix(spidev->spi, SH_SPI_FIX_EN);
		hs_spi_config(spidev->spi, &spidev->spi_data);

#ifdef INSURE_SPI_PINMUX_SEQ
		next_spi_status = LENS_SPI_STATUS_CLOSE;
#endif
		break;

	case LENS_IOC_SPI_CLOSE:
#ifdef INSURE_SPI_PINMUX_SEQ
		if (next_spi_status != LENS_SPI_STATUS_CLOSE)
			break;
#endif

		hs_spi_config_clear(spidev->spi);
		hs_spi_close(spidev->spi);

#ifdef INSURE_SPI_PINMUX_SEQ
		next_spi_status = LENS_SPI_STATUS_DISABLE;
#endif
		break;

	case LENS_IOC_GPIO_DIR:
		gpio_id = arg >> 8;
		gpio_value = arg & 0xF;
		gpio_dir = (arg >> 4) & 0xF;

		if (gpio_dir)
			gpio_direction_output(gpio_id, gpio_value);
		else
			gpio_direction_input(gpio_id);

		break;

	case LENS_IOC_GPIO_SET:
		gpio_id = arg >> 8;
		gpio_value = arg & 0xF;

		gpio_set_value(gpio_id, gpio_value);
		break;

	case LENS_IOC_GPIO_GET:
		if (__copy_from_user(&gpio_control, (void __user *)arg,
				     sizeof(struct lens_gpio_control))) {

			printk(KERN_ERR "GPIO GET FAILED\n");
			retval = -EFAULT;
			break;
		}

		gpio_value = (u16) gpio_get_value(gpio_control.gpio_id);

		if (gpio_control.p_gpio_value) {
			retval = copy_to_user(
				(void __user *)gpio_control.p_gpio_value,
				&gpio_value, sizeof(u8));
		} else
			retval = -EINVAL;

	case LENS_IOC_MEM_SET:
		retval = __copy_from_user((void *) &value, (const void *)arg, sizeof(u32));
		if (retval < 0) {
			printk("ioctl fail: [%d]", cmd);
			retval = -EFAULT;
			break;
		}
		lens_mem_ctrl(spidev, value);
		break;
	}

	mutex_unlock(&spidev->buf_lock);

	return retval;
}

/**
 * poll implementation for kernel level
 */
static unsigned int lens_comm_poll(struct file *filp,
				   struct poll_table_struct *wait)
{
	unsigned int mask = 0;
	struct spidev_data	*spidev;

	//printk("lens_comm_poll\n");
	spidev = filp->private_data;

#if 0
	/*Unexpectly poll_wait returns immediatly.*/
	poll_wait(filp, &spidev->wq_int_poll, wait);
#else

#ifdef CIRC_RB_BUF_SIZE

#if 0
	/*Deadlock 현상이 발생하여 event wait로 변경. Sanghyuk.cha 2012.12.19*/
	if (get_rb_circ_buf_count(&cbuf) <= 0){
		printk("Waiting interrupt\n");
		interruptible_sleep_on(&spidev->wq_int_poll);
	}
#else
	//printk("Waiting interrupt\n");
	wait_event_interruptible(spidev->wq_int_poll,(get_exist_event(&cbuf) > 0));
#endif
	
#else
	interruptible_sleep_on(&spidev->wq_int_poll);
#endif

#endif
	//printk("Received interrupt\n");

	mask |= POLLIN | POLLRDNORM;

	return mask;
}

static const struct file_operations lens_comm_fops = {
	.owner   = THIS_MODULE,
	.read    = lensdev_read,
	.open    = lens_comm_open,
	.release = lens_comm_close,
	/* ioctl field is removed since 2.6.36 kernel.
	   But style checker complains about it
	 */
#if 0 /*LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 36)*/
	.ioctl          = lens_comm_ioctl,
#else
	.unlocked_ioctl = lens_comm_ioctl,
#endif
	.poll    = lens_comm_poll,
};

static struct miscdevice lens_comm_dev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name  = "lens_comm",
	.fops  = &lens_comm_fops
};


/**
 * @brief Module initialization code.
 */
static int lens_probe(struct platform_device *pdev)
{
	int result = 0;
	int irqflag;

	irq_map[0] = GPIO_LENS_RB;
	irq_map[1] = GPIO_LENS_DET;

	/* Allocate driver data */
	gSpidev = kzalloc(sizeof(*gSpidev), GFP_KERNEL);
	if (!gSpidev)
		return -ENOMEM;

#ifdef CIRC_RB_BUF_SIZE
	cbuf.buf = kmalloc(CIRC_RB_BUF_SIZE, GFP_KERNEL);
	cbuf.size = CIRC_RB_BUF_SIZE;

	if (!cbuf.buf) {
		result = -ENOMEM;
		goto err_malloc;
	}

	spin_lock_init(&cbuf.p_lock);
	spin_lock_init(&cbuf.c_lock);
#endif

	/* Initialize the driver data */

	gSpidev->spi = hs_spi_request(2);


	gSpidev->spi_data.speed_hz  = 316400 /*250000*/ /*2531300*/;
	gSpidev->spi_data.bpw       = 8;
	gSpidev->spi_data.mode      = SH_SPI_MODE_3;
	gSpidev->spi_data.waittime  = 100;
	gSpidev->spi_data.ss_inven  = D4_SPI_TRAN_INVERT_OFF;
	gSpidev->spi_data.spi_ttype = D4_SPI_TRAN_BURST_OFF;
	gSpidev->spi_data.setup_select =
		SH_SPI_SPEED | SH_SPI_BPW | SH_SPI_WAVEMODE |
		SH_SPI_WAITTIME | SH_SPI_INVERT | SH_SPI_BURST;

	update_delay_time(gSpidev);

	spin_lock_init(&gSpidev->spi_lock);
	spin_lock_init(&gSpidev->int_lock);
	mutex_init(&gSpidev->buf_lock);

	init_waitqueue_head(&gSpidev->wq_int_poll);

#ifdef USE_WORK_QUEUE
	INIT_WORK(&gSpidev->rb_work, rb_work);
	INIT_WORK(&gSpidev->det_work, det_work);
#endif

	irqflag = IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING;
	result = request_lens_int(det_int_isr, GPIO_LENS_DET, irqflag, gSpidev);
	disable_irq_nosync(gpio_to_irq(GPIO_LENS_DET));

	if (result < 0) {
		printk(KERN_ERR "%s: SPI det setup has been failed", __func__);
		goto err_buf;

	}

/*
  When setting trigger level low and high,
  only high level trigger works ...
*/
	irqflag = IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING;
	result = request_lens_int(rb_int_isr, GPIO_LENS_RB, irqflag, gSpidev);
	disable_irq_nosync(gpio_to_irq(GPIO_LENS_RB));

	if (result < 0) {
		printk(KERN_ERR "%s: SPI int setup has been failed", __func__);
		goto err_buf;
	}

	init_gpios();
	result = misc_register(&lens_comm_dev);
	if (result < 0) {
		printk("failed to register lens misc driver\n");
		goto err_buf;
	}

	return 0;

err_buf:
	kfree(cbuf.buf);
err_malloc:
	kfree(gSpidev);
	return result;
}

static void nx_lens_comm_exit(void)
{
	/* make sure ops on existing fds can abort cleanly */
	spin_lock(&gSpidev->spi_lock);
	gSpidev->spi = NULL;
	spin_unlock(&gSpidev->spi_lock);

	misc_deregister(&lens_comm_dev);
}

static int lens_remove(struct platform_device *pdev)
{
	return 0;
}

static int lens_suspend(struct platform_device *pdev,
		pm_message_t state)
{

#ifdef CIRC_RB_BUF_SIZE
	kfree(cbuf.buf);
#endif

	return 0;
}

static int lens_resume(struct platform_device *pdev)
{
	prev_int_data = 0xFF;
	prev_int_values[0] = 0xFF;
	prev_int_values[1] = 0xFF;

#ifdef CIRC_RB_BUF_SIZE
	cbuf.buf = kmalloc(CIRC_RB_BUF_SIZE, GFP_KERNEL);
	cbuf.size = CIRC_RB_BUF_SIZE;

	if (!cbuf.buf)
		return -ENOMEM;
#endif
	return 0;
}


static struct platform_driver lens_driver = { .driver = {
	.name = "lens_comm",
	.owner = THIS_MODULE,
}, .probe = lens_probe, .remove = lens_remove,
		.suspend = lens_suspend, .resume = lens_resume, };


static int __init lens_init(void)
{
	return platform_driver_register(&lens_driver);
}


static void __exit lens_comm_exit(void)
{
	nx_lens_comm_exit();
	platform_driver_unregister(&lens_driver);
}



#ifndef CONFIG_SCORE_FAST_RESUME
module_init(lens_init);
#else
fast_dev_initcall(lens_init);
#endif
module_exit(lens_comm_exit);

MODULE_AUTHOR("Hong yeol, Choi <chykrkr@acroem.com>");
MODULE_DESCRIPTION("lens communication manager driver");
MODULE_LICENSE("GPL");
MODULE_VERSION(DRV_VERSION);
