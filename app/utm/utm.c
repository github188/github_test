/********************************************************************************

 **** Copyright (C), 2015, BinHong tech Co., Ltd.                ****

 ********************************************************************************
 * File Name     : utm.c
 * Author        : chenxu
 * Date          : 2015-12-07
 * Version       : 1.0
 * Function List :
 * Description   : 于监控中心交互相关处理
 * Record        :
 * 1.Date        : 2015-12-07
 *   Author      : chenxu
 *   Modification: Created file

 *******************************************************************************/

/*------------------------------------------------------------------------------
Section: Includes
------------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <sys/prctl.h>
#include "utm.h"
#include "trace.h"
#include "../ptl/ptl.h"
#include "../ptl/protocol.h"
#include "../comm/comm.h"
#include "../aid/aid.h"
#include "../system/system.h"
#include <pthread.h>
#include <mqueue.h>
#include "../log/trace.h"
/*------------------------------------------------------------------------------
Section: Macro Definitions
------------------------------------------------------------------------------*/
#define UTM_LOCK 		pthread_mutex_lock(&utm.utm_mutex)
#define UTM_UNLOCK  	pthread_mutex_unlock(&utm.utm_mutex)
/*------------------------------------------------------------------------------
Section: Type Definitions
------------------------------------------------------------------------------*/
/* None */
/*------------------------------------------------------------------------------
Section: Globals
------------------------------------------------------------------------------*/
static utm_t utm;
static mqd_t msgq_id_rpt;
pthread_t tid_utm;
/*------------------------------------------------------------------------------
Section: Function Prototypes
------------------------------------------------------------------------------*/
/*****************************************************************************
 * Function      : read_sd
 * Description   : 读取设备参数
 * Input         : uint8 *data
                uint8 *pack
 * Output        : None
 * Return        : utm_rst_t
 * Others        : 
 * Record
 * 1.Date        : 20151220
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
static utm_rst_e cmd_read_sd( uint8 *data, uint8 *pack , uint32 *len)
{
    utm_rst_e ret;
	uint16 dev_type_id = (data[0] << 8) + data[1];
	uint8 dev_id = data[2];

    if(NULL != utm.phb_callback)
	    utm.phb_callback(true);

	trace(TR_UTM, "dev_type_id[%d] dev_id[%d]\n", dev_type_id, dev_id);
	
    *len = 0;
	ret = dev_read_sd(dev_type_id, dev_id, pack, len);
	
	return ret;
}

/*****************************************************************************
 * Function      : cmd_ctl
 * Description   : 控制帧,设置相关参数
 * Input         : uint8 *data
 * Output        : None
 * Return        : utm_rst_e
 * Others        : 
 * Record
 * 1.Date        : 20151220
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
static utm_rst_e cmd_ctl( uint8 *data )
{
    utm_rst_e ret;

    if(NULL != utm.phb_callback)
	    utm.phb_callback(true);
	
    ret = dev_ctl(data);

	return ret;
}

/*****************************************************************************
 * Function      : time_sync
 * Description   : 对时
 * Input         : uint8 *data
 * Output        : None
 * Return        : static utm_rst_e
 * Others        : 
 * Record
 * 1.Date        : 20151213
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
static utm_rst_e cmd_time_sync( uint8 *data )
{
    struct timeval tv;
	time_t t;

    if(NULL != utm.phb_callback)
	    utm.phb_callback(true);
	trace_buf(TR_UTM_BUF, "time buf: ", data, 7);
	t = ptl_get_time(data);
	if(0 == t)
		return UTM_TIME_ERR;
	tv.tv_sec = t;
	tv.tv_usec = 0;

	settimeofday(&tv, NULL);
	
	return UTM_OK;
}

/*****************************************************************************
 * Function      : system_reset
 * Description   : 设备重启
 * Input          : void
 * Output        : None
 * Return        : void
 * Others        : 
 * Record
 * 1.Date        : 20151215
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
static utm_rst_e cmd_system_reset( void )
{
    aid_set_reset(SOFT_RESET);

	return UTM_OK;
}

/*****************************************************************************
 * Function      : cmd_confirm
 * Description   : 监控中心下行确认
 * Input         : uint8 pfc
 * Output        : None
 * Return        : void
 * Others        : 
 * Record
 * 1.Date        : 20151222
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
static void cmd_confirm( uint8 pfc )
{
    //目前只处理监控中心心跳确认
    if(NULL != utm.phb_callback)
	    utm.phb_callback(true);
}

/*****************************************************************************
 * Function      : utm_confirm
 * Description   : 控制器向监控中心发送确认报文
 * Input         : uint16 pfc 要确认的帧序号
 * Output        : None
 * Return        : void 
 * Others        : 
 * Record
 * 1.Date        : 20151213
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
static void utm_confirm(boolean flag, uint16 pfc , utm_rst_e errno)
{
    int32 len;
	uint32 idx;
    ptl_rst_t ptl;
	ptl_rtn_e ret_ptl;
	appbuf_t *appbuf = &utm.appbuf;
	utm_rst_e ret;
	uint8 buffer[100];

    if(true == flag)
	{
		ptl.frm_ctg = FRMCTG_CONFIRM;
		ptl.datalen = 0;
	}
	else
	{
	    ptl.frm_ctg = FRMCTG_ERR;
		ptl.data = buffer;
		buffer[0] = utm.lst_frm_info.frm_ctg;
		ptl_pack_time(utm.lst_frm_info.stp, &buffer[1]);
		buffer[8] = utm.lst_frm_info.msg_id /256;
		buffer[9] = utm.lst_frm_info.msg_id %256;
		buffer[10] = errno;
		ptl.datalen = 11;
	}
	ptl.stp = time(NULL);
	memcpy((void*)&ptl.src_addr, (void*)&utm.lst_frm_info.src_addr, sizeof(addr_t));
	memcpy((void*)&ptl.dest_addr, (void*)&utm.suaddr, sizeof(addr_t));
	ptl.msg_id = pfc;

	if(PTL_OK == ptl_pack(&ptl, utm.pbuf, &len))
	{
	    trace_buf(TR_UTM, "send confirm pack:", utm.pbuf, len);	 
        comm_send((uint8*)utm.pbuf, len);
	}
}

/*****************************************************************************
 * Function      : utm_loop
 * Description   : 接收监控中心下行业务报文,发送实时上报数据
 * Input         : void
 * Output        : None
 * Return        : void
 * Others        : 
 * Record
 * 1.Date        : 20151208
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
static void utm_loop( void )
{	
    int32 len;
	uint32 idx;
    ptl_rst_t ptl;
	ptl_rtn_e ret_ptl;
	appbuf_t *appbuf = &utm.appbuf;
	uint8 buf[512];
	utm_rst_e ret;

	prctl(PR_SET_NAME, UTM_THREAD_NAME);
	
    FOREVER
	{
	    //分别获取监控中心的下行数据和设备的上行数据
	    usleep(1000);
	    appbuf->buftyp = 0;
		len = -2;
		if((comm_recv((void*)appbuf) > 0)
			|| ((len = (int32)mq_receive(msgq_id_rpt, (char*)appbuf, 8192, NULL)) > 0))
		{
			//如果是上报数据直接打包发送
			if(APPBUF_RPT == appbuf->buftyp)
	    	{	
	    	    utm_param_t utmp;

				system_getutmparam(&utmp);
				ptl.frm_ctg = appbuf->frm_ctg;
				ptl.stp = time(NULL);
				memcpy((void *)&ptl.src_addr, (void *)&utmp.su_addr, sizeof(addr_t));
				memcpy((void *)&ptl.dest_addr, (void *)&utmp.mc_addr, sizeof(addr_t));
				ptl.msg_id = utm_get_pfc();
				ptl.datalen = appbuf->len;
				ptl.data = appbuf->data;
	    	    if(PTL_OK == (ret_ptl = ptl_pack(&ptl, utm.pbuf, &len)))
    	    	{
		            comm_send((uint8*)utm.pbuf, len);
    	    	}
				else
					trace(TR_UTM, "ptl_pack failed, err[%d]\n", ret_ptl);
				
				continue;
	    	}

			if(appbuf->buftyp != APPBUF_MC)
				continue;
            trace_buf(TR_UTM_BUF, "recv from mc:", appbuf->data, appbuf->len);			
			/*对监控中心下行数据进行校验*/
			if(PTL_OK != (ret_ptl = ptl_parse(appbuf->data, appbuf->len, &ptl)))
			{
			    trace(TR_UTM, "recieve a err[ret = %d] frame\n", ret_ptl);
				continue;
			}

			//校验目的地址
//			if(memcmp((void*)&utm.suaddr, (void*)&ptl.dest_addr, sizeof(addr_t)) != 0)
//			{
//			    trace(TR_UTM, "recieve dest addr not match\n");
//				continue;
//			}

			//保存上一次正常通信信息
			memcpy((void *)&utm.lst_frm_info, (void*)&ptl, sizeof(ptl_rst_t));
			//业务处理
			switch(ptl.frm_ctg)
			{
			    case FRMCTG_READ_SD:
					ret = cmd_read_sd(ptl.data, buf, &len);
					trace_buf(TR_UTM_BUF, "ptl.data:", ptl.data, ptl.datalen);
					if(UTM_OK == ret)
					{
					    utm_param_t utmp;

						system_getutmparam(&utmp);
					    ptl.frm_ctg = FRMCTG_READ_RES;
						ptl.stp = time(NULL);
						memcpy((void *)&ptl.src_addr, (void *)&utmp.su_addr, sizeof(addr_t));
						memcpy((void *)&ptl.dest_addr, (void *)&utm.lst_frm_info.src_addr, sizeof(addr_t));
						ptl.msg_id = utm.lst_frm_info.msg_id;
						ptl.datalen = len;
						ptl.data = buf;
						if(PTL_OK == (ret_ptl = ptl_pack(&ptl, utm.pbuf, &len)))
		    	    	{
		    	    	    trace_buf(TR_UTM_BUF, "comm send:", utm.pbuf, len);
				            comm_send((uint8*)utm.pbuf, len);
		    	    	}
						else
							trace(TR_UTM, "ptl_pack failed, err[%d]\n", ret_ptl);
						
					}
					else
						trace(TR_UTM, "utm get sd err:dev_type_id[%d]-dev_id[%d]ret[%d]\n", 
							((ptl.data[0] << 8) + ptl.data[1]), ptl.data[2], ret);
					break;
				case FRMCTG_CTL :
					ret = cmd_ctl(ptl.data);
					if(UTM_OK == ret)
						utm_confirm(true, utm.lst_frm_info.msg_id, UTM_OK);
					else
						utm_confirm(false, utm.lst_frm_info.msg_id, ret);
					break;
				case FRMCTG_TIME_SYN:
					ret = cmd_time_sync(ptl.data);
					if(UTM_OK == ret)
						utm_confirm(true, utm.lst_frm_info.msg_id, UTM_OK);
					break;
				case FRMCTG_RESET:
					ret = cmd_system_reset();
					if(UTM_OK == ret)
						utm_confirm(true, utm.lst_frm_info.msg_id, UTM_OK);
					break;
				case FRMCTG_CONFIRM:
					cmd_confirm(utm.lst_frm_info.msg_id);
					break;
				default:
					break;
			}

			//返回数据
			
		}
//		trace(TR_UTM,"222222222222len [%d]\n", len);
	}
}

/*****************************************************************************
 * Function      : utm_get_pfc
 * Description   : 获取帧序号
 * Input         : void
 * Output        : None
 * Return        : uint16
 * Others        : 
 * Record
 * 1.Date        : 20151208
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
uint16 utm_get_pfc( void )
{
    uint16 pfc;

    UTM_LOCK;
	pfc = utm.pfc++;
	UTM_UNLOCK;
	
    return pfc;
}

/*****************************************************************************
 * Function      : utm_rpt_send
 * Description   : 发送一条上报信息到队列
 * Input         : rpt_info_t
 * Output        : None
 * Return        : int
 * Others        : 
 * Record
 * 1.Date        : 20151208
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
void utm_rpt_send( appbuf_t *appbuf)
{
	while(-1 == mq_send(msgq_id_rpt, (char*)appbuf, sizeof(appbuf_t), 1))
	    usleep(1000);	
}

/*****************************************************************************
// * Function      : utm_get_mcaddr
// * Description   : 获取监控中心地址
// * Input         : addr_t *mcaddr
// * Output        : None
// * Return        : void
// * Others        : 
// * Record
// * 1.Date        : 20151222
// *   Author      : chenxu
// *   Modification: Created function

//*****************************************************************************/
//void utm_get_mcaddr( addr_t *mcaddr )
//{
//    UTM_LOCK;
//	memcpy((char*)mcaddr, (char*)&utm.mcaddr, sizeof(addr_t));
//	UTM_UNLOCK;
//}

/*****************************************************************************
// * Function      : utm_get_suaddr
// * Description   : 获取控制器地址
// * Input         : addr_t *suaddr
// * Output        : None
// * Return        : void
// * Others        : 
// * Record
// * 1.Date        : 20151222
// *   Author      : chenxu
// *   Modification: Created function

//*****************************************************************************/
//void utm_get_suaddr( addr_t *suaddr )
//{
//    UTM_LOCK;
//	memcpy((char*)suaddr, (char*)&utm.suaddr, sizeof(addr_t));
//	UTM_UNLOCK;
//}

/*****************************************************************************
 * Function      : utm_cb_register
 * Description   : utm回调函数注册
 * Input         : uint8 type
                VOIDFUNCPTRBOOL callback
 * Output        : None
 * Return        : void
 * Others        : 
 * Record
 * 1.Date        : 20151222
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
void utm_cb_register( uint8 type, VOIDFUNCPTRBOOL callback )
{
    if(CB_HB_CALL == type)
		utm.phb_callback = callback;
}

/*****************************************************************************
 * Function      : utm_init
 * Description   : utm模块初始化
 * Input         : void
 * Output        : None
 * Return        : void
 * Others        : 
 * Record
 * 1.Date        : 20151207
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
void utm_init( void )
{    
	int i, ret;    
    struct mq_attr msgq_attr;
	sys_param_t sys_param;
	
	//创建数据采集数据上报消息队列
	msgq_attr.mq_maxmsg = 10;
	msgq_attr.mq_msgsize = 1024;
	msgq_attr.mq_curmsgs = 0;
	msgq_attr.mq_flags = 0;//O_NONBLOCK;
	
	mq_unlink(MSGQ_REPORT_NAME);
	msgq_id_rpt = mq_open(MSGQ_REPORT_NAME, O_RDWR | O_CREAT, S_IRWXU | S_IRWXG, NULL);
    if(msgq_id_rpt == (mqd_t)-1)
    {
        trace(TR_UTM, "mq_open err \n");
        return;
    }

	trace(TR_UTM, "msgq_id_rpt [%d] \n", msgq_id_rpt);

	if(mq_getattr(msgq_id_rpt, &msgq_attr) == -1)
        trace(TR_UTM, "mq_getattr err\n");

    msgq_attr.mq_flags = O_NONBLOCK;
	if(mq_setattr(msgq_id_rpt, &msgq_attr, NULL) == -1)
        trace(TR_UTM, "mq_setattr err\n");
	
	trace(TR_UTM, "Queue \"%s\":\n\t- stores at most %ld messages\n\t- \
        large at most %ld bytes each\n\t- currently holds %ld messages\n\t- mq_flags[%08x]\n", 
        MSGQ_REPORT_NAME, msgq_attr.mq_maxmsg, msgq_attr.mq_msgsize, msgq_attr.mq_curmsgs,msgq_attr.mq_flags);	
	

	//创建utm互斥信号量
	if(0 != pthread_mutex_init(&utm.utm_mutex, NULL))
		trace(TR_UTM, "create utm_mutex failed\n");

	//创建上行业务处理线程
	pthread_attr_t attr;
	
	ret = pthread_attr_init(&attr);
	assert(ret == 0);
	ret = pthread_attr_setstacksize(&attr, UTM_THREAD_STACK_SIZE);
	assert(ret == 0);
	ret = pthread_create(&tid_utm, &attr, (void *) utm_loop,NULL);
	if(ret != 0)
		trace(TR_UTM, "Create pthread error[%d]!\n", ret);

//    int child;
//	if(0 == fork())
//	{
//	    printf("proc21111111111111111111111111111111111\n");
//		execlp("/usr/devmgr/test1",NULL);
//		printf("utm exit\n");
//		exit(0);
//	}

}
/* None */

                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        
