/********************************************************************************

 **** Copyright (C), 2015, BinHong tech Co., Ltd.                ****

 ********************************************************************************
 * File Name     : main.c
 * Author        : chenxu
 * Date          : 2015-11-30
 * Version       : 1.0
 * Function List :
 * Description   : Ӧ�ó����ʼ���ļ�
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
	//��־ģ���ʼ��
    log_init();
	
    //ϵͳ������Ϣ��ʼ��
	system_init();	    
	
    //aidҵ���ʼ��
    aid_init();
	
    //web��ʼ��
	web_init();

	//comm��ʼ��
	comm_init();

    //utm��ʼ��
    utm_init();

    //alarm ��ʼ��
    alarm_init();
	
    //�豸��ʼ��
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

