/********************************************************************************

 **** Copyright (C), 2015, BinHong tech Co., Ltd.                ****

 ********************************************************************************
 * File Name     : eth.h
 * Author        : chenxu
 * Date          : 2015-12-23
 * Version       : 1.0
 * Function List :
 * Description   : eth相关定义
 * Record        :
 * 1.Date        : 2015-12-23
 *   Author      : chenxu
 *   Modification: Created file

 *******************************************************************************/
#ifndef __ETH_H__
#define __ETH_H__
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
#define NetDev_xml					"NetDev.xml"			//网络设备文件名
#define ETH_THREAD_STACK_SIZE		(100 * 1024)
/*------------------------------------------------------------------------------
Section: Type Definitions
------------------------------------------------------------------------------*/
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
}var_eth_param_t;

//变量数据定义
typedef struct
{
    float32 lv;						//上一次采集数据
	struct timeval dt;				//数据时标
}var_eth_data_t;

typedef struct _var_eth_t_
{
    struct _var_eth_t_ *prev;		//前一个节点
    struct _var_eth_t_ *next;		//下一个节点
    var_eth_param_t var_p;			//变量参数
    var_eth_data_t d;				//上一次采集数据
    alarm_stat_e alarm_stat;        //告警状态
}var_eth_t;

typedef struct
{
	var_eth_t *head;				/* head in list */
	var_eth_t *tail;				/* tail in list */
	uint32 count;					/* node counter */
}var_eth_list_t;

//串口通信设备相关结构体定义
typedef struct
{
	uint16 dev_type_id;				//设备标识码
    uint8 dev_name[DEV_NAME_LEN];	//设备名称
    uint8 dev_id;					//设备ID
    uint32 dumb_num;				//连续n此无响应视为离线
    uint8 protocol_type;			//通信协议类型
    uint8 addr;						//通信地址
}dev_eth_param_t;

typedef struct _dev_eth_t_
{
    struct _dev_eth_t_ * prev;		//指向下一个eth设备
    struct _dev_eth_t_ * next;		//指向下一个eth设备
    dev_eth_param_t dev_p;			//设备参数
    dev_stat_t dev_stat;			//设备状态信息
	var_eth_list_t varlist;			//变量列表
}dev_eth_t;

typedef struct
{
	dev_eth_t *head;				/* head in list */
	dev_eth_t *tail;				/* tail in list */
	uint32 count;					/* node counter */
}dev_eth_list_t;

typedef struct
{
    char name[DEV_NAME_LEN];
	char ptl_type[DEV_NAME_LEN];
    struct in_addr ip;
	unsigned short port;
}net_t;

typedef enum
{
    LINK_STAT_RESET = 0,
	LINK_STAT_WAIT_PHY,
	LINK_STAT_CSOCKET,
	LINK_STAT_CONNECT,
	LINK_STAT_LOGIN
}link_stat_e;

typedef struct _mount_net_t_ 
{
    struct _mount_net_t_ *prev;
	struct _mount_net_t_ *next;			
	pthread_mutex_t net_mutex;
	pthread_t tid;
    net_t net;
	int sockfd;
	boolean flagofnetparamchange;	//参数变更标记
	link_stat_e link_stat;			//链接状态
	boolean flagofdevparamrequest;	//增加设备/变量参数请求标记,以便web的及时响应
	dev_eth_list_t dev_eth_list;
}mount_net_t;

typedef struct
{
    mount_net_t *head;
	mount_net_t *tail;
	uint32 count;
}mount_list_t;

/*------------------------------------------------------------------------------
Section: Globals
------------------------------------------------------------------------------*/
extern void dev_eth_init( void );
/*------------------------------------------------------------------------------
Section: Function Prototypes
------------------------------------------------------------------------------*/

#endif