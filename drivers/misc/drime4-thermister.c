/* linux/drivers/misc/drime4-thermister.c
*
 * Copyright (c) 2011 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com/
 *
 * DRIME4 - thermister adv voltage reading from adc channel
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
 
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <linux/uaccess.h>
#include <mach/adc.h>
#include <linux/io.h>
#include <mach/map.h>


//defines
//#define 	ADC_CHANNEL_TEST	0
#define DRV_NAME	"Thermister Driver"
#define TEMPERATURE_BUFFER_SIZE	10
#define DRIME4_ADC_INTERRUPT
#ifdef ADC_CHANNEL_TEST
#define DRIME4_CIS_ADC_CHANNEL	2		
#else
#define DRIME4_CIS_ADC_CHANNEL	4  	//Thermister Channel
#define DRIME4_DSP_ADC_CHANNEL	6  	//Thermister Channel

#endif

#define DRIME4_ADC_REF_MVOLT	180
#define DRIME4_ADC_TIME_MARGIN  8191	
#define DRIME4_ADC_DATA_DIFF	55	
#define DRIME4_ADC_READINGS	10
#define ADCVALUE_30DEGREE		684u
#define ADCVALUE_70DEGREE		320u
#define SUCCESS	0
#define FAIL	1
#define READ_FAIL	-1

//thermister data structure
struct thermister_data {
	int adc_value ;
	int temperature;
	int buffer_index;
	unsigned int temperature_buffer[TEMPERATURE_BUFFER_SIZE];
};

//adc client structure
struct drime4_adc_client {
	struct platform_device *pdev;
	struct drime4_adc_device *adc_dev;
	wait_queue_head_t *wait;

	ktime_t start_time;
	unsigned int ref_mvolt;
	unsigned int time_margin;
	unsigned int data_diff;
	unsigned int channel;
	int result;

};
struct drime4_adc_client* adcclient_cis = NULL;
struct drime4_adc_client* adcclient_dsp = NULL;

static struct thermister_data thermister_cis;
static struct thermister_data thermister_dsp;

static struct miscdevice thermister_miscdev ;
int result = -1;


/**
 * fn: drime4_thermister_register_adc	this function call adc register 
 * 		function call to register for thermister channel
 * @param  platform device
 * @param  reference volate for adc
 * @param  time margin value
 * @param  data difference
 * @return  drime4_adc_client structure
*/
static struct drime4_adc_client* drime4_thermister_register_adc (struct platform_device *pdev,
															unsigned int ref_mvolt, 
															unsigned int time_margin,
															unsigned int data_diff,
															unsigned int channel)	
{
    struct drime4_adc_client *adc_c = NULL;
    adc_c = drime4_adc_register(pdev, ref_mvolt, time_margin, data_diff);
    drime4_adc_request_irq(adc_c, NULL, NULL, channel);
    drime4_adc_int_set(adc_c, ADC_INT_OFF, channel);
    drime4_adc_start(adc_c, channel);
    return adc_c;
}

/**
 * fn: drime4_thermister_read_adcvalue : 
 *		Read ADC value	
 * @param  drime4_adc_client structure
 * @param  Channel: Channel 4 for adc voltage value of thermister
 * @return  return bool - SUCCESS or FAIL
*/
static bool drime4_thermister_read_adcvalue(struct drime4_adc_client* client, int channel,  struct thermister_data* t_th_struct)
{
    struct drime4_adc_client *adc_c = client;
    int buf_indx = 0;	
    unsigned int value = 0;
    for (buf_indx = 0; buf_indx<TEMPERATURE_BUFFER_SIZE; buf_indx++)	
    {
		if(adc_c)
		{
        	value = drime4_adc_raw_read_hax(adc_c, channel);
			
		}
		else
		{
			printk("ADC struct Error\n");
		}
#if 0		
        if((value>ADCVALUE_30DEGREE)||(value<ADCVALUE_70DEGREE)){
		value = 0;
        }  
#endif		
		if(t_th_struct)
		{
			t_th_struct->temperature_buffer[buf_indx] = value;
		}
    }

    return SUCCESS;
}


/**
 * fn: drime4_thermister_open : 
 *		Open call for thermister driver	
 * @param  inode structure
 * @param  file pointer
 * @return  return bool - SUCCESS or FAIL
*/
int drime4_thermister_open(struct inode *inode, struct file *fileptr)
{
	thermister_cis.adc_value = 0;
	thermister_cis.temperature = 0;	
	if(!adcclient_cis)
	{
		adcclient_cis = drime4_thermister_register_adc (NULL, DRIME4_ADC_REF_MVOLT,
				DRIME4_ADC_TIME_MARGIN, 
				DRIME4_ADC_DATA_DIFF,
				DRIME4_CIS_ADC_CHANNEL);	
	}
	//printk("Regist Success %x\n",adcclient_cis);
	thermister_dsp.adc_value = 0;
	thermister_dsp.temperature = 0;
	if(!adcclient_dsp)
	{
		adcclient_dsp = drime4_thermister_register_adc (NULL, DRIME4_ADC_REF_MVOLT,
				DRIME4_ADC_TIME_MARGIN, 
				DRIME4_ADC_DATA_DIFF,
				DRIME4_DSP_ADC_CHANNEL);
	}
	//printk("Regist Success %x\n",adcclient_dsp);
	return SUCCESS;
}

/**
 * fn: drime4_thermister_read : read thermister driver	
 * @param  file pointer
 * @param  buffer pointer
 * @param  read count
 * @return  success: return the read count
 * 		   fail: Read fail (negative), 
 * 		   copy to buffer fail -EIO
 *		   
 */
ssize_t drime4_thermister_read(struct file *fileptr, unsigned long *buf, size_t count)
{

	//Purpose is to return array of adc voltage readings may be 10 elements at a time
	//to reduce overhead, temperature calculation from adc voltage value is moved to user space
	//i.e. sensor f/w plugin for thermometer will take care of this calculation
	bool ret_val = FAIL;
	unsigned int buffer[TEMPERATURE_BUFFER_SIZE*2];
	int ret_size = READ_FAIL;
	if(adcclient_cis == NULL){
		printk ("adc registration failed\n");
		return READ_FAIL;
	}
	if(adcclient_dsp == NULL){
		printk ("adc registration failed\n");
		return READ_FAIL;
	}
	if(count < (TEMPERATURE_BUFFER_SIZE * sizeof(unsigned int) * 2))
	{
		printk ("[thermister]Read buffer size is too small\n");
		return READ_FAIL;
	}
	memset(buffer, 0, sizeof(buffer));
	if(!drime4_thermister_read_adcvalue(adcclient_cis, DRIME4_CIS_ADC_CHANNEL, &thermister_cis)){
		memcpy(buffer, thermister_cis.temperature_buffer, sizeof (thermister_cis.temperature_buffer));
	}
	//printk("Thermist Read-2\n");
	if(!drime4_thermister_read_adcvalue(adcclient_dsp, DRIME4_DSP_ADC_CHANNEL, &thermister_dsp)){		
		memcpy(&buffer[TEMPERATURE_BUFFER_SIZE], thermister_dsp.temperature_buffer, sizeof (thermister_dsp.temperature_buffer));		
	}
	
	ret_size = copy_to_user(buf, buffer, sizeof (buffer) ) ;
	if (!ret_size){
		ret_val = count;
		return ret_val;
	}
	return ret_val;		
}

/**
 * fn: drime4_thermister_release : read thermister driver	
 */
int drime4_thermister_release(struct inode *inode, struct file *filp)
{
	if(adcclient_cis)
	{
    	drime4_adc_release(adcclient_cis);
		adcclient_cis = NULL;
	}
	if(adcclient_dsp)
	{
    	drime4_adc_release(adcclient_dsp);
		adcclient_dsp = NULL;
	}
    return 0;
}

//thermister device file operations structure
const struct file_operations thermister_fops = {
	.owner			= THIS_MODULE,
	.open			= drime4_thermister_open,
	.read			= drime4_thermister_read,
	.release			= drime4_thermister_release,
};


//thermister as misc device
static struct miscdevice thermister_miscdev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "drime4-thermister",
	.fops = &thermister_fops,
};

int __init drime4_thermister_init(void)
{
    return (misc_register(&thermister_miscdev));	
}

void __exit drime4_thermister_exit(void)
{
    misc_deregister(&thermister_miscdev);
}


module_init(drime4_thermister_init);
module_exit(drime4_thermister_exit);
MODULE_DESCRIPTION("Thermister Driver");
MODULE_AUTHOR("Prasann Kumar <kmr.prasanna@samsung.com>");
MODULE_LICENSE("GPL");
