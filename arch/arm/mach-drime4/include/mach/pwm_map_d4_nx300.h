/*
 *
 * We define gpio for only common port for many different kinds of board of NX300
 * 
 */
#ifndef __PWM_MAP_D4_NX300_H__
#define __PWM_MAP_D4_NX300_H__

#include <linux/pwmdev_conf.h>
#include <mach/gpio.h>
#include <mach/version_information.h>

struct platform_pwm_dev_data pwm_pin_map[PWMDEV_MAX] =
{

	{PWMDEV_NULL, (unsigned int)-1, DRIME4_GPIO_END},	 //0
	{PWMDEV_NULL, (unsigned int)-1, DRIME4_GPIO_END},	 //1
	{0			, (unsigned int)-1, GPIO_SHUTTER_MG1_ON},	 //2
	{1			, (unsigned int)-1, GPIO_SHUTTER_MG2_ON},	 //3
	{PWMDEV_NULL, (unsigned int)-1, DRIME4_GPIO_END},	 //4
	{PWMDEV_NULL, (unsigned int)-1, DRIME4_GPIO_END},	 //5
	{PWMDEV_NULL, (unsigned int)-1, DRIME4_GPIO_END},	 //6
	{PWMDEV_NULL, (unsigned int)-1, DRIME4_GPIO_END},	 //7
	{PWMDEV_NULL, (unsigned int)-1, DRIME4_GPIO_END},	 //8
	{PWMDEV_NULL, (unsigned int)-1, DRIME4_GPIO_END},	 //9
	{PWMDEV_NULL, (unsigned int)-1, DRIME4_GPIO_END},	 //10
	{PWMDEV_NULL, (unsigned int)-1, DRIME4_GPIO_END},	 //11
	{PWMDEV_NULL, (unsigned int)-1, DRIME4_GPIO_END},	 //12
	{PWMDEV_NULL, (unsigned int)-1, DRIME4_GPIO_END},	 //13
	{PWMDEV_NULL, (unsigned int)-1, DRIME4_GPIO_END},	 //14
	{PWMDEV_NULL, (unsigned int)-1, DRIME4_GPIO_END},	 //15
	{PWMDEV_NULL, (unsigned int)-1, DRIME4_GPIO_END},	 //16
	{PWMDEV_NULL, (unsigned int)-1, DRIME4_GPIO_END}	 //17
		
};


#endif //__PWM_MAP_D4_NX300_H__

