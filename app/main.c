/********************************************************************************

 **** Copyright (C), 2015, BinHong tech Co., Ltd.                ****

 ********************************************************************************
 * File Name     : main.c
 * Author        : chenxu
 * Date          : 2015-11-30
 * Version       : 1.0
 * Function List :
 * Description   : 应用程序初始化文件
 * Record        :
 * 1.Date        : 2015-11-30
 *   Author      : chenxu
 *   Modification: Created file

 *******************************************************************************/

/*------------------------------------------------------------------------------
Section: Includes
------------------------------------------------------------------------------*/
#include <stdio.h> 
#include <stdlib.h>
#include <unistd.h>  
#include <fcntl.h>
#include <time.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include "dev.h"
#include "utm.h"
#include "system.h"
#include "aid.h"
#include "comm.h"
#include "trace.h"
#include "alarm.h"
#include "io.h"
/*------------------------------------------------------------------------------
Section: Macro Definitions
------------------------------------------------------------------------------*/
/*NONE*/
/*------------------------------------------------------------------------------
Section: Type Definitions
------------------------------------------------------------------------------*/
/*NONE*/
/*------------------------------------------------------------------------------
Section: Globals
------------------------------------------------------------------------------*/
/*NONE*/
/*------------------------------------------------------------------------------
Section: Function Prototypes
------------------------------------------------------------------------------*/
int main(int argc, char * argv[])
{   
	//日志模块初始化
    log_init();
	
    //系统配置信息初始化
	system_init();	    
	
    //aid业务初始化
    aid_init();
	
    //web初始化
	web_init();

	//comm初始化
	comm_init();

    //utm初始化
    utm_init();

    //alarm 初始化
    alarm_init();
	
    //设备初始化
    dev_init();

    pthread_join(tid_web, NULL);
    pthread_join(tid_io, NULL);
//      pthread_join(pid_485, NULL);
//      pthread_join(pid_eth, NULL);
    pthread_join(tid_comm, NULL);
    pthread_join(tid_utm, NULL);
    pthread_join(tid_bp, NULL);
    pthread_join(tid_aid, NULL);
	
	return 0;
}

