/********************************************************************************

 **** Copyright (C), 2015, BinHong tech Co., Ltd.                ****

 ********************************************************************************
 * File Name     : web.c
 * Author        : chenxu
 * Date          : 2015-11-30
 * Version       : 1.0
 * Function List :
 * Description   : 与web交互的相关接口实现
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
#include <assert.h>
#include <sys/prctl.h>
#include <string.h>
#include "web.h"
#include "../devmgr/dev.h"
#include <pthread.h>
#include <mqueue.h>
#include "../log/trace.h"
//#include "alarm.h"
/*------------------------------------------------------------------------------
Section: Macro Definitions
------------------------------------------------------------------------------*/
#define WEBPRINTF(x...) \
    {    \
        (void)printf("[web]L[%d]%s()\r\t\t\t\t", __LINE__, __FUNCTION__); \
        (void)printf(x);    \
    }
/*------------------------------------------------------------------------------
Section: Type Definitions
------------------------------------------------------------------------------*/
/* None */
/*------------------------------------------------------------------------------
Section: Globals
------------------------------------------------------------------------------*/
pthread_t tid_web = -1;
/*------------------------------------------------------------------------------
Section: Function Prototypes
------------------------------------------------------------------------------*/
/*****************************************************************************
 * Function      : web_proc
 * Description   : 接收从web发送过来的参数修改信息
 * Input         : void
 * Output        : None
 * Return        : void
 * Others        : 
 * Record
 * 1.Date        : 20151126
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
static void *web_proc( void )
{
    mqd_t rmsgq_id;
	mqd_t smsgq_id;
    web_msg_t msg;
	int recvlen;

    unsigned int sender;
    struct mq_attr msgq_attr;
	webg_var_info_t *g_vinfo;
	webg_dev_info_t *g_dinfo;
	boolean is_need_answer = false;
	opt_rst_e ret;
	uint8 outbuf[WEB_MSG_DATA_SIZE];

	//设置线程名字
	prctl(PR_SET_NAME, WEB_THREAD_NAME);

	//创建一个接收消息队列
	mq_unlink(WEB_RECV_MSG_NAME);
	rmsgq_id = mq_open(WEB_RECV_MSG_NAME, O_RDONLY | O_CREAT, S_IRWXU | S_IRWXG, NULL);
    if(rmsgq_id == (mqd_t)-1)
    {
        trace(TR_WEB,  "mq_open err \n");
        return;
    }

	if(mq_getattr(rmsgq_id, &msgq_attr) == -1)
        trace(TR_WEB,  "mq_getattr");
	//此处可以修改msgq_attr的相关属性   
	msgq_attr.mq_msgsize = sizeof(web_msg_t);
	
    trace(TR_WEB,  "Queue \"%s\":\n\t- stores at most %ld messages\n\t- \
        large at most %ld bytes each\n\t- currently holds %ld messages\n\t- mq_flags[%08x]\n", 
        WEB_RECV_MSG_NAME, msgq_attr.mq_maxmsg, msgq_attr.mq_msgsize, msgq_attr.mq_curmsgs,msgq_attr.mq_flags);	
	
	if(mq_setattr(rmsgq_id, &msgq_attr, NULL) == -1)
        trace(TR_WEB,  "mq_setattr");

	if(mq_getattr(rmsgq_id, &msgq_attr) == -1)
        trace(TR_WEB,  "mq_getattr");

		trace(TR_WEB,  "Queue \"%s\":\n\t- stores at most %ld messages\n\t- \
        large at most %ld bytes each\n\t- currently holds %ld messages\n\t- mq_flags[%08x]\n", 
        WEB_RECV_MSG_NAME, msgq_attr.mq_maxmsg, msgq_attr.mq_msgsize, msgq_attr.mq_curmsgs,msgq_attr.mq_flags);	

	//创建一个发送消息队列
	mq_unlink(WEB_SEND_MSG_NAME);	
	smsgq_id = mq_open(WEB_SEND_MSG_NAME, O_WRONLY | O_CREAT, S_IRWXU | S_IRWXG, NULL);
    if(smsgq_id == (mqd_t)-1)
    {
        trace(TR_WEB,  "mq_open err \n");
        return;
	}
	
	if(mq_getattr(smsgq_id, &msgq_attr) == -1)
        trace(TR_WEB,  "mq_getattr");
    //此处可以修改msgq_attr的相关属性
	msgq_attr.mq_msgsize = sizeof(web_msg_t);
	
    trace(TR_WEB,  "Queue \"%s\":\n\t- stores at most %ld messages\n\t- \
        large at most %ld bytes each\n\t- currently holds %ld messages\n\t- mq_flags[%08x]\n", 
        WEB_SEND_MSG_NAME, msgq_attr.mq_maxmsg, msgq_attr.mq_msgsize, msgq_attr.mq_curmsgs,msgq_attr.mq_flags);
	
	if(mq_setattr(smsgq_id, &msgq_attr, NULL) == -1)
        trace(TR_WEB,  "mq_setattr");

	
    while(1)
	{
	    if ((recvlen = mq_receive(rmsgq_id, (char*)&msg, 8192, NULL)) == -1) 
        {
            trace(TR_WEB,  "mq_receive err \n");
	        break;
        }
		assert(recvlen == sizeof(web_msg_t));
		
		trace(TR_WEB,  "msg objt[%d] optt[%d] \n", msg.objt, msg.optt);
		param_opt_t opt = {msg.optt, msg.objt, (void*)msg.data, (void*)outbuf};

		switch(msg.objt)
		{
		    case OBJ_TYPE_SYSTEM:
			case OBJ_TYPE_SYSTEM_EXP:
				ret = system_mgr(&opt);
				break;
			case OBJ_TYPE_ACCESS:
				ret = usr_mgr(&opt);
				break;
		    case OBJ_TYPE_IO_DEV:
			case OBJ_TYPE_IO_VAR:
			case OBJ_TYPE_485_SER:
			case OBJ_TYPE_485_DEV:
			case OBJ_TYPE_485_VAR:
			case OBJ_TYPE_ETH_MNT:
			case OBJ_TYPE_ETH_DEV:
			case OBJ_TYPE_ETH_VAR:
				ret = dev_mgr(&opt);
				break;
			case OBJ_TYPE_ALARM:
				ret = alarm_request(&opt);
				break;
			case OBJ_TYPE_TRACE:
				ret = trace_mgr(&opt);
				break;
			default:
				ret = ERROR;
		}
		
		trace(TR_WEB,  "ret = %d\n", ret);
        //返回信息
        msg.ret = ret;
		if(mq_send(smsgq_id, (char*)&msg, sizeof(msg), 1) == -1)
            trace(TR_WEB,  "mq_send err\n");
	}
	mq_close(rmsgq_id);
	mq_unlink(WEB_RECV_MSG_NAME);
	mq_close(smsgq_id);
	mq_unlink(WEB_SEND_MSG_NAME);
}

/*****************************************************************************
 * Function      : web_init
 * Description   : 与web交互相关处理初始化
 * Input         : void
 * Output        : None
 * Return        : void
 * Others        : 
 * Record
 * 1.Date        : 20151130
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
void web_init( void )
{
	int i,ret;
	pthread_attr_t attr;
	
	ret = pthread_attr_init(&attr);
	assert(ret == 0);
	ret = pthread_attr_setstacksize(&attr, WEB_THREAD_STACK_SIZE);
	assert(ret == 0);
	
//	//创建相应web配置线程
	ret = pthread_create(&tid_web, &attr, (void *) web_proc, NULL);
	if(ret != 0)
		trace(TR_WEB,  "Create pthread error[%d]!\n", ret);
}


