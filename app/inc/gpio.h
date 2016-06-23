/********************************************************************************

 **** Copyright (C), 2015, BinHong tech Co., Ltd.                ****

 ********************************************************************************
 * File Name     : gpio.h
 * Author        : chenxu
 * Date          : 2015-12-15
 * Version       : 1.0
 * Function List :
 * Description   : gpio相关定义
 * Record        :
 * 1.Date        : 2015-12-15
 *   Author      : chenxu
 *   Modification: Created file

 *******************************************************************************/
#ifndef __GPIO_H__
#define __GPIO_H__

/*------------------------------------------------------------------------------
Section: Includes
------------------------------------------------------------------------------*/
#include <sys/ioctl.h>
/*------------------------------------------------------------------------------
Section: Macro Definitions
------------------------------------------------------------------------------*/
#define DEV_GPIO "/dev/atmel_gpio"
#define	AT91_PIN_PC18	(0x40 + 18)
#define AT91_PIN_PC30	(0x40 + 30)
#define WDT_GPIO_PIN    AT91_PIN_PC18
#define BEEP_GPIO_PIN   AT91_PIN_PC30
#define GPIO_IOC_MAGIC  'G'
#define IOCTL_GPIO_SETOUTPUT              _IOW(GPIO_IOC_MAGIC, 0, int)       
#define IOCTL_GPIO_SETINPUT               _IOW(GPIO_IOC_MAGIC, 1, int)
#define IOCTL_GPIO_SETVALUE               _IOW(GPIO_IOC_MAGIC, 2, int) 
#define IOCTL_GPIO_GETVALUE    		  	  _IOR(GPIO_IOC_MAGIC, 3, int)

#define AT91_PIN_PD0		(0x60)
#define AT91_PIN_PD12		(AT91_PIN_PD0 + 12)
#define AT91_PIN_PD15		(AT91_PIN_PD0 + 15)
#define AT91_PIN_PD21		(AT91_PIN_PD0 + 21)
#define LED_ALARM			AT91_PIN_PD12
#define LED_UP_STREAM		AT91_PIN_PD15
#define LED_DN_STREAM		AT91_PIN_PD21

/*------------------------------------------------------------------------------
Section: Type Definitions
------------------------------------------------------------------------------*/
typedef struct 
{
	int pin;
	int data;
	int usepullup;
}at91_gpio_arg;
/*------------------------------------------------------------------------------
Section: Globals
------------------------------------------------------------------------------*/
/* None */
/*------------------------------------------------------------------------------
Section: Function Prototypes
------------------------------------------------------------------------------*/
/* None */

#endif
