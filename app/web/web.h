/********************************************************************************

 **** Copyright (C), 2015, BinHong tech Co., Ltd.                ****

 ********************************************************************************
 * File Name     : web.h
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
#ifndef __WEB_H__
#define __WEB_H__
/*------------------------------------------------------------------------------
Section: Includes
------------------------------------------------------------------------------*/
#include "../inc/atypes.h"
#include <pthread.h>
//#include "../devmgr/dev.h"
//#include "opt.h"
/*------------------------------------------------------------------------------
Section: Macro Definitions
------------------------------------------------------------------------------*/
#define WEB_THREAD_NAME			"WEB"

//web消息类型定义
//#define MSG_TYPE_ERROR			-1	//错误
//#define MSG_TYPE_CONFIRM 			0	//空消息
//#define MSG_TYPE_SET_DEV_IO			1	//设置IO设备参数类型消息
//#define MSG_TYPE_SET_DEV_485		2	//设置485设备参数类型消息
//#define MSG_TYPE_SET_DEV_ETH		3	//设置ETH设备参数类型消息

//#define MSG_TYPE_SET_VAR_IO			4	//设置变量参数类型消息
//#define MSG_TYPE_SET_VAR_485		5	//设置变量参数类型消息
//#define MSG_TYPE_SET_VAR_ETH		6	//设置变量参数类型消息

//#define MSG_TYPE_SET_SYSCFG			7	//设置系统参数类型消息

//#define MSG_TYPE_GET_DEV_IO			8	//查询IO设备参数类型消息
//#define MSG_TYPE_GET_DEV_485		9	//查询485设备参数类型消息
//#define MSG_TYPE_GET_DEV_ETH		10	//查询ETH设备参数类型消息

//#define MSG_TYPE_GET_VAR_IO			11	//查询变量参数类型消息
//#define MSG_TYPE_GET_VAR_485		12	//查询变量参数类型消息
//#define MSG_TYPE_GET_VAR_ETH		13	//查询变量参数类型消息

//#define MSG_TYPE_GET_SYSCFG			14	//查询系统参数类型消息

//#define MSG_TYPE_DEL_DEV_IO 		15	//删除IO设备
//#define MSG_TYPE_DEL_DEV_485 		16	//删除485设备
//#define MSG_TYPE_DEL_DEV_ETH 		17	//删除ETH设备

//#define MSG_TYPE_DEL_VAR_IO 		18	//删除IO变量
//#define MSG_TYPE_DEL_VAR_485 		19	//删除485变量
//#define MSG_TYPE_DEL_VAR_ETH 		20	//删除ETH变量

#define WEB_ALARM_FUNC1_REQOFTIME		1
#define WEB_ALARM_FUNC2_REQOFDEVNAME	2

//web 消息队列名
#define WEB_RECV_MSG_NAME		"/webrecvmsg"
#define WEB_SEND_MSG_NAME		"/websendmsg"

#ifndef DEV_NAME_LEN
#define DEV_NAME_LEN		50
#endif

#ifndef VAR_NAME_LEN
#define VAR_NAME_LEN		50
#endif

#define WEB_MSG_DATA_SIZE		512
#define WEB_THREAD_STACK_SIZE		(4 * 1024 *1024)
/*------------------------------------------------------------------------------
Section: Type Definitions
------------------------------------------------------------------------------*/
/* 保存对齐状态 */
//#pragma pack(push)
/* 设定为1字节对齐 */
//#pragma pack(1)

//typedef enum
//{
//    WEB_OPT_DEV_SET = 0,
//	WEB_OPT_DEV_GET,
//	WEB_OPT_DEV_DEL,
//	WEB_OPT_VAR_SET,
//	WEB_OPT_VAR_GET,
//	WEB_OPT_VAR_DEL,
//	WEB_OPT_MAX
//}web_opt_type_e;
/*操作类型*/

typedef enum
{
    OPT_TYPE_SET = 0,
	OPT_TYPE_GET,
	OPT_TYPE_DEL
}opt_type_e;

/*操作对象类型*/
typedef enum
{
    OBJ_TYPE_SYSTEM = 0,
	OBJ_TYPE_SYSTEM_EXP,
	OBJ_TYPE_IO_DEV,
	OBJ_TYPE_IO_VAR,
	OBJ_TYPE_485_SER,
	OBJ_TYPE_485_DEV,
	OBJ_TYPE_485_VAR,	
	OBJ_TYPE_ETH_MNT,
	OBJ_TYPE_ETH_DEV,
	OBJ_TYPE_ETH_VAR,
	OBJ_TYPE_ACCESS,
	OBJ_TYPE_ALARM,
	OBJ_TYPE_TRACE
}obj_type_e;

typedef enum
{
    OPT_OK = 0,
	OPT_ERR_DEV_NEXIST,
	OPT_ERR_VAR_NEXIST,
	OPT_ERR_SER_NEXIST,
	OPT_ERR_NET_NEXIST,
	OPT_ERR_ELSE
}opt_rst_e;

//接收web消息数据结构
typedef struct
{
    opt_type_e optt;				//操作类型
    obj_type_e objt;				//操作对象
    opt_rst_e ret;					//操作结果
    uint8 data[WEB_MSG_DATA_SIZE];				//数据内容
}web_msg_t;

typedef struct
{
    opt_type_e optt;				//操作类型
    obj_type_e objt;				//操作对象
    void *pin;						//输入数据
	void *pout;						//输出数据
}param_opt_t;

//IO设备参数数据结构
typedef struct
{
    uint16 dev_type_id;
	uint8 dev_id;
}webg_dev_info_t;

//变量请求信息
typedef struct
{
    uint16 dev_type_id;				//设备标识码
	uint8 dev_id;					//设备名称
	uint16 pn;						//参数标识码
}webg_var_info_t;

typedef struct
{
    uint16 dev_type_id;				//设备类型
	uint8 dev_id;					//设备编号
	uint16 pn;						//参数编号
	void *data;						//具体信息
}web_opt_t;

typedef struct
{
    uint16 dev_type_id;
	uint8 dev_id;
}dev_header1_t;

typedef struct
{
    uint8 comn;
    uint16 dev_type_id;
	uint8 dev_id;
}dev_header2_t;

typedef struct
{
    char mount_name[DEV_NAME_LEN];
    uint16 dev_type_id;
	uint8 dev_id;
}dev_header3_t;

typedef struct
{
    uint16 dev_type_id;
	uint8 dev_id;
	uint16 pn;
}var_header1_t;

typedef struct
{
    uint8 comn;
	uint16 dev_type_id;
	uint8 dev_id;
	uint16 pn;
}var_header2_t;

typedef struct
{
    char mount_name[DEV_NAME_LEN];
	uint16 dev_type_id;
	uint8 dev_id;
	uint16 pn;
}var_header3_t;



/* 恢复对齐状态 */
//#pragma pack(pop)
/*------------------------------------------------------------------------------
Section: Globals
------------------------------------------------------------------------------*/
extern pthread_mutex_t mq_mutex;
/*------------------------------------------------------------------------------
Section: Function Prototypes
------------------------------------------------------------------------------*/
extern void web_init(void);
extern pthread_t tid_web;


#endif 

