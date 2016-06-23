/********************************************************************************

 **** Copyright (C), 2015, BinHong tech Co., Ltd.                ****

 ********************************************************************************
 * File Name     : aid.c
 * Author        : chenxu
 * Date          : 2015-12-14
 * Version       : 1.0
 * Function List :
 * Description   : 辅助业务处理
 * Record        :
 * 1.Date        : 2015-12-14
 *   Author      : chenxu
 *   Modification: Created file

 *******************************************************************************/

/*------------------------------------------------------------------------------
Section: Includes
------------------------------------------------------------------------------*/
#include <pthread.h>
#include <assert.h>
#include <time.h>
#include <sys/times.h>
#include <sys/prctl.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include "aid.h"
#include "../log/trace.h"
#include "../inc/gpio.h"
#include "../inc/jiffies.h"
//#include <linux/Jiffies.h>
/*------------------------------------------------------------------------------
Section: Macro Definitions
------------------------------------------------------------------------------*/
/* None */
/*------------------------------------------------------------------------------
Section: Type Definitions
------------------------------------------------------------------------------*/
#define AIDPRINTF(x...) \
    {    \
        (void)printf("[aid]L[%d]%s()\r\t\t\t\t", __LINE__, __FUNCTION__); \
        (void)printf(x);    \
    }

#define AID_LOCK 		pthread_mutex_lock(&aid.aid_mutex)
#define AID_UNLOCK		pthread_mutex_unlock(&aid.aid_mutex)
/*------------------------------------------------------------------------------
Section: Globals
------------------------------------------------------------------------------*/
pthread_t tid_aid;
pthread_t tid_bp = -1;
static aid_t aid;
static int wdog_stat = 1;
static int hard_reset_flag = 0;
static int wdt_gpio_fd = -1;
static unsigned long ticks_per_sec;
/*------------------------------------------------------------------------------
Section: Function Prototypes
------------------------------------------------------------------------------*/
/*****************************************************************************
 * Function      : wdog_init
 * Description   : 看门狗对应引脚输出配置
 * Input         : void
 * Output        : None
 * Return        : static void
 * Others        : 
 * Record
 * 1.Date        : 20151215
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
static void wdog_init( void )
{
	at91_gpio_arg arg;
	
	wdt_gpio_fd = open(DEV_GPIO, O_RDWR);
	if (wdt_gpio_fd < 0) 
	{
		trace(TR_AID,  "gpio device open fail! %d\n");
		pthread_exit(0);
	}
	
	arg.pin = WDT_GPIO_PIN;
	arg.data = wdog_stat;
	arg.usepullup = 1;
	ioctl(wdt_gpio_fd, IOCTL_GPIO_SETOUTPUT, &arg);
}

/*****************************************************************************
 * Function      : wdog_feed
 * Description   : 硬件喂狗
 * Input         : void
 * Output        : None
 * Return        : void
 * Others        : 
 * Record
 * 1.Date        : 20151215
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
#define WDOG_FEED_MSEC		(400 - 1)
static void wdog_feed( void )
{
	static clock_t ltick = 0;
	clock_t tick;
	at91_gpio_arg arg;
	static struct timeval ltv = {0};
	struct timeval tv;
//	uint32 lst = jiffies;
//    int a;
//    static int b = 0;

    //硬件复位不喂狗
    if(hard_reset_flag)
		return;
//	tick = times(NULL);
//    tick = clock();
//    a = get_rdtsc();
//    if(time_after(tick, (ltick + WDOG_FEED_MSTIME * ticks_per_sec / 1000)))
//    if((tick - ltick) >= (WDOG_FEED_MSTIME * ticks_per_sec / 1000))
//    if((a - b) >= (WDOG_FEED_MSTIME * ticks_per_sec / 1000))
    gettimeofday(&tv, NULL);
    if(abs((tv.tv_sec - ltv.tv_sec) * 1000 + (tv.tv_usec - ltv.tv_usec) / 1000) >= WDOG_FEED_MSEC)
	{
//	    trace(TR_AID, "tick[%d] ltick[%d]\n", tick, ltick);
//        trace(TR_AID, "a[%d] b[%d]\n", a, b);
	    wdog_stat = wdog_stat ^ (0x01);
	    arg.pin = WDT_GPIO_PIN;
		arg.data = wdog_stat;
		arg.usepullup = 1;
		ioctl(wdt_gpio_fd, IOCTL_GPIO_SETVALUE, &arg);
//		ltick = tick;		
        ltv = tv;
	}
}

/*****************************************************************************
 * Function      : do_reset
 * Description   : 复位检查
 * Input         : void
 * Output        : None
 * Return        : void
 * Others        : 
 * Record
 * 1.Date        : 20151215
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
static void do_reset( void )
{
//    clock_t tick;
	static struct timeval ltv;
    struct timeval tv;
	
	if((false == aid.is_hard_reset)
		&& (false == aid.is_soft_reset))
		return;

    //RESET_DELAY_SECONDS 秒后再复位等待控制器返回监控中心一个确认报文
//    tick = times(NULL);
//	if(((tick - aid.rst_cmd_tick) / ticks_per_sec) < RESET_DELAY_SECONDS)
    if(abs(tv.tv_sec - aid.rst_cmd_tick.tv_sec) < RESET_DELAY_SECONDS)
		return;

    //TODO 保存数据
    
    //复位操作
//	trace(TR_AID,  "curr_tick[%d]system reset...\n", tick);
	if(aid.is_hard_reset)
	{
		hard_reset_flag = 1;
		aid.is_hard_reset = false;
	}
	if(aid.is_soft_reset)
	{
		aid.is_soft_reset = false; 
		system("reboot");
	}
		
}

/*****************************************************************************
 * Function      : aid_thread
 * Description   : 辅助业务维护线程
 * Input         : void
 * Output        : None
 * Return        : void
 * Others        : 
 * Record
 * 1.Date        : 20151215
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
static void *aid_thread( void )
{
   prctl(PR_SET_NAME, AID_THREAD_NAME);
   
    FOREVER
	{
	    //延时10ms
	    usleep(1000);

        //硬件喂狗
		wdog_feed();

		//复位检查
		do_reset();
		
		
	}
}

/*****************************************************************************
 * Function      : beep_thread
 * Description   : 启动后响蜂鸣器
 * Input         : void
 * Output        : None
 * Return        : void
 * Others        : 
 * Record
 * 1.Date        : 20151217
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
static void *beep_thread( void )
{
	at91_gpio_arg arg;
	int beep_gpio_fd = -1;
	beep_gpio_fd = open(DEV_GPIO, O_RDWR);
	if (beep_gpio_fd < 0) 
	{
		printf("gpio device open fail! %d\n");
		pthread_exit(0);
	}
	
	/* beep sound for 500ms after power on reset */
	arg.pin = BEEP_GPIO_PIN;
	arg.data = 1;
	arg.usepullup = 1;
	ioctl(beep_gpio_fd, IOCTL_GPIO_SETOUTPUT, &arg);
	ioctl(beep_gpio_fd, IOCTL_GPIO_SETVALUE, &arg);
	usleep(500*1000);
	arg.data = 0;
	ioctl(beep_gpio_fd, IOCTL_GPIO_SETVALUE, &arg);
	
	close(beep_gpio_fd);
}

/*****************************************************************************
 * Function      : aid_set_reset
 * Description   : 复位设置
 * Input         : uint8 reset
 * Output        : None
 * Return        : void
 * Others        : 
 * Record
 * 1.Date        : 20151214
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
void aid_set_reset( uint8 reset)
{
    AID_LOCK;

	if(HARD_RESET == reset)
	{
		aid.is_hard_reset = true;
//		aid.rst_cmd_tick = times(NULL);
        gettimeofday(&aid.rst_cmd_tick, NULL);
	}
	else if(SOFT_RESET == reset)
	{
		aid.is_soft_reset = true;
//		aid.rst_cmd_tick = times(NULL);
        gettimeofday(&aid.rst_cmd_tick, NULL);
	}
//	trace(TR_AID,  "rst_cmd_tick = %d\n", aid.rst_cmd_tick);
	
	AID_UNLOCK;
}

/*****************************************************************************
 * Function      : aid_init
 * Description   : 辅助业务初始化
 * Input         : void
 * Output        : None
 * Return        : void
 * Others        : 
 * Record
 * 1.Date        : 20151214
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
void aid_init( void )
{
    int ret;
	pthread_attr_t attr;
	
    aid.is_hard_reset = false;
	aid.is_soft_reset = false;
	ticks_per_sec = sysconf(_SC_CLK_TCK);

    wdog_init();
	
	if(0 != pthread_mutex_init(&aid.aid_mutex, NULL))
		trace(TR_AID,  "create aid_mutex failed\n");

	ret = pthread_attr_init(&attr);
	assert(ret == 0);
	ret = pthread_attr_setstacksize(&attr, AID_THREAD_STACK_SIZE);
	assert(ret == 0);
	
	//创建aid线程
	ret = pthread_create(&tid_aid, &attr, (void *) aid_thread, NULL);
	if(ret != 0)
		trace(TR_AID,  "Create aid_thread error!\n");

//	//添加蜂鸣器任务
//	ret = pthread_create(&tid_bp, &attr, (void *) beep_thread, NULL);
//	if(ret != 0)
//		trace(TR_AID,  "Create beep_thread pthread error!\n");

}


