/********************************************************************************

 **** Copyright (C), 2015, BinHong tech Co., Ltd.                ****

 ********************************************************************************
 * File Name     : utm.h
 * Author        : chenxu
 * Date          : 2015-12-07
 * Version       : 1.0
 * Function List :
 * Description   : 控制终端与监控中心交互相关处理
 * Record        :
 * 1.Date        : 2015-12-07
 *   Author      : chenxu
 *   Modification: Created file

 *******************************************************************************/
#ifndef __UTM_H__
#define __UTM_H__
/*------------------------------------------------------------------------------
Section: Includes
------------------------------------------------------------------------------*/
#include "../ptl/protocol.h"
#include <pthread.h>
#include "errno.h"
/*------------------------------------------------------------------------------
Section: Macro Definitions
------------------------------------------------------------------------------*/
#define UTM_THREAD_NAME			"UTM"
/*设备上报消息队列名称*/
#define MSGQ_REPORT_NAME		"/rpt"

/*应用层报文类型*/
#define APPBUF_MC             	0x01 /*MC下行报文*/
#define APPBUF_RPT           	0x02 /*设备上报信息*/

/*回调函数类型定义*/
#define CB_HB_CALL		0x01

#define UTM_THREAD_STACK_SIZE	(100 *1024)
/*------------------------------------------------------------------------------
Section: Type Definitions
------------------------------------------------------------------------------*/
/* 保存对齐状态 */
//#pragma pack(push)
/* 设定为1字节对齐 */
//#pragma pack(1)

typedef struct
{
    uint8 flag;				//帧头	1	68H
	uint16 length;			//长度	2	从帧头开始（含帧头）到校验（不含校验）结束所有字节个数
	uint8 ptl_ver;			//协议版本	1	第一版协议为01H，依次类推
	uint8 rsv1;				//保留字段	1	备用
	uint8 frmctg;			//帧类别	1	见后说明
	uint8 tbuf[7];			//消息时间戳	7字节BCD码	20150910105210，为2015-09-10 10:52:10
	uint8 rsv2[2];			//保留字段	2	备用
	addr_t src_addr;		//源地址	5字节BCD	
	addr_t dest_addr;		//目标地址	5字节BCD	
	uint16 pfc;				//消息ID	2	用于对消息进行标记，每次进行累加
}ptl_header_t;

typedef struct
{
	uint16 dev_type_id;		//设备标识码
	uint8 dev_id;			//设备ID
	uint16 datalen;			//消息正文中，单个设备数据长度不含设备ID
	uint8 data[128];		//消息正文
}rpt_msg_t;

typedef struct
{
    /**
     * buf类型
     * 1: 监控中心原始报文;
     * 2: 设备实时数据上报信息
     */
    uint8 buftyp;
    uint8 frm_ctg;	//帧类型
	uint32 len;
	uint8 data[512];
	
}appbuf_t;	

typedef struct
{
    pthread_mutex_t utm_mutex;

	addr_t suaddr;						//控制器地址信息

	addr_t mcaddr;						//监控中心地址

	ptl_rst_t lst_frm_info;				//最近一次MC下来的报文头解析信息

    appbuf_t appbuf;					//待处理信息		

	uint8 pbuf[512];					//发送数据报文缓存
	
	uint16 pfc;							//帧序号,帧计数器

	VOIDFUNCPTRBOOL phb_callback;       //心跳应答回调函数  
}utm_t;

/* 恢复对齐状态 */
//#pragma pack(pop)
/*------------------------------------------------------------------------------
Section: Globals
------------------------------------------------------------------------------*/
/* None */
/*------------------------------------------------------------------------------
Section: Function Prototypes
------------------------------------------------------------------------------*/
extern pthread_t tid_utm;
extern void utm_rpt_send(appbuf_t *appbuf);
extern uint16 utm_get_pfc( void );
extern void utm_cb_register( uint8 type, VOIDFUNCPTRBOOL callback );
extern void utm_init(void);

#endif

