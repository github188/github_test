/********************************************************************************

 **** Copyright (C), 2015, BinHong tech Co., Ltd.                ****

 ********************************************************************************
 * File Name     : 485.h
 * Author        : chenxu
 * Date          : 2015-12-23
 * Version       : 1.0
 * Function List :
 * Description   : 485相关定义
 * Record        :
 * 1.Date        : 2015-12-23
 *   Author      : chenxu
 *   Modification: Created file

 *******************************************************************************/
#ifndef __485_H__
#define __485_H__
/*------------------------------------------------------------------------------
Section: Includes
------------------------------------------------------------------------------*/
#include "atypes.h"
#include <pthread.h>
#include "dev.h"
#include "alarm.h"
/*------------------------------------------------------------------------------
Section: Macro Definitions
------------------------------------------------------------------------------*/
#define F_DEV_485_CFG				"485.cfg"				//io设备参数文件名
#define SerialDev_xml				"SerialDev.xml"			//串口设备文件名
#define RS_ADDR_LEN					6	//485地址字节长度	
#define RS485_THREAD_STACK_SIZE		(1 * 1024 * 1024)
/*------------------------------------------------------------------------------
Section: Type Definitions
------------------------------------------------------------------------------*/
/* 保存对齐状态 */
//#pragma pack(push)
/* 设定为1字节对齐 */
//#pragma pack(1)

//变量参数定义
typedef struct
{
    uint16 pn;						//变量ID
    uint8 pname[VAR_NAME_LEN];		//变量名
    value_type_e vt;				//数据类型
    float32 dataprecision;			//数据精度
    uint8 rw;						//读写权限
    uint32 period;					//抄读周期
    uint16 reg_addr;				//寄存器地址
    uint8 alarm_enable;				//告警使能 0-disable ,1-enable
    alarm_type_e wt;				//告警类型
    float32 hvalue;					//上限
    float32 lvalue;					//下限
}var_485_info_t;

//typedef struct
//{
//    var_header1_t header;
//	var_485_info_t vinfo;
//}web_485_varinfo_t;

//变量数据定义
typedef struct
{
    float32 lv;						//上一次采集数据
	struct timeval dt;				//数据时标
}var_485_data_t;

typedef struct _var_485_t_
{
    struct _var_485_t_ *prev;		//前一个节点
    struct _var_485_t_ *next;		//下一个节点
    var_485_info_t var_p;			//变量参数
    var_485_data_t d;				//上一次采集数据
    alarm_stat_e alarm_stat;        //告警状态
}var_485_t;

typedef struct
{
	var_485_t *head;				/* head in list */
	var_485_t *tail;				/* tail in list */
	uint32 count;					/* node counter */
}var_485_list_t;

//串口通信设备相关结构体定义
typedef struct
{
	uint16 dev_type_id;				//设备标识码
    uint8 dev_name[DEV_NAME_LEN];	//设备名称
    uint8 dev_id;					//设备ID
    uint32 dumb_num;				//连续n此无响应视为离线
    uint8 protocol_type;			//通信协议类型
    uint8 addr;						//通信地址
}dev_485_info_t;

typedef struct _dev_485_t_
{
    struct _dev_485_t_ * prev;		//指向下一个485设备
    struct _dev_485_t_ * next;		//指向下一个485设备
    dev_485_info_t dev_p;			//设备参数
    dev_stat_t dev_stat;			//设备状态信息
	var_485_list_t varlist;			//变量列表
}dev_485_t;

typedef struct
{
	dev_485_t *head;				/* head in list */
	dev_485_t *tail;				/* tail in list */
	uint32 count;					/* node counter */
}dev_485_list_t;

typedef struct
{
    uint8 comn;						//串口号
    struct
    {
        uint16 dbit :2;   			/**< 0～3：5-8位数据位                                      */
        uint16 par  :3;    			/**< 0/1：偶/奇校验                                         */ 
        uint16 sbit :2;   			/**< 0/1：1/ 2停止位                                        */
        uint16 baud :4;   			/**< 0 ~ 13 波特率(1200, 1800, 2400, 3600, 4800, 7200, 9600, 
        							 14400, 19200, 28800, 38400, 57600, 76800, 115200)*/
		uint16 rsv  :4;				//保留
    }p;
}serial_t;

typedef struct 
{
    serial_t serial;				
	pthread_mutex_t serial_mutex;
	dev_485_list_t dev_485_list;
}mount_serial_t;

/* 恢复对齐状态 */
//#pragma pack(pop)

/*------------------------------------------------------------------------------
Section: Globals
------------------------------------------------------------------------------*/
extern pthread_t pid_485;
//extern const WEBOPTFUNCPTR web_485_opt_fun[];
/*------------------------------------------------------------------------------
Section: Function Prototypes
------------------------------------------------------------------------------*/

#endif