#ifndef __GPIO_COMMON_H__
#define __GPIO_COMMON_H__

#define GPIO_IOC_MAGIC 'G'
#define IOCTL_GPIO_SETOUTPUT _IOW(GPIO_IOC_MAGIC, 0, int)       
#define IOCTL_GPIO_SETINPUT _IOW(GPIO_IOC_MAGIC, 1, int)
#define IOCTL_GPIO_SETVALUE _IOW(GPIO_IOC_MAGIC, 2, int) 
#define IOCTL_GPIO_GETVALUE _IOR(GPIO_IOC_MAGIC, 3, int)

typedef struct 
{
	int pin;
	int data;
	int usepullup;
}at91_gpio_arg;

#endif