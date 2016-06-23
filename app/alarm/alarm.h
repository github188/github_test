/********************************************************************************

 **** Copyright (C), 2016, BinHong tech Co., Ltd.                ****

 ********************************************************************************
 * File Name     : alarm.h
 * Author        : chenxu
 * Date          : 2016-01-18
 * Version       : 1.0
 * Function List :
 * Description   : 告警事件处理
 * Record        :
 * 1.Date        : 2016-01-18
 *   Author      : chenxu
 *   Modification: Created file

 *******************************************************************************/
#ifndef __ALARM_H__
#define __ALARM_H__
/*------------------------------------------------------------------------------
Section: Includes
------------------------------------------------------------------------------*/
#include "atypes.h"
#include <sys/time.h>
#include "web.h"
/*------------------------------------------------------------------------------
Section: Macro Definitions
------------------------------------------------------------------------------*/
#define ALARM_FILE_HEADER		"/usr/devmgr/"

#define DATA_LIST_INIT(list) \
	(list)->head = NULL;\
	(list)->tail = NULL;\
	(list)->count = 0;

#define DATA_LIST_DEL(plist, pnode)\
{\
    if (pnode->prev == NULL)\
		(plist)->head = pnode->next;\
    else\
		pnode->prev->next = pnode->next;\
    if (pnode->next == NULL)\
		(plist)->tail = pnode->prev;\
    else\
		pnode->next->prev = pnode->prev;\
    (plist)->count--;/* update node count */\
}

#define DATA_LIST_INSERT(plist, pnode, temp, member)\
	{\
	    for (temp = (plist)->tail; temp != NULL; temp = temp->prev) {\
	        if(temp->member < pnode->member)\
				break;\
	    }\
		pnode->prev = temp;/* 插在temp的后面 */\ 
		if (temp != NULL) {\
			if (temp->next != NULL) {  /* 不是尾节点 */\
				temp->next->prev = pnode;\
			} else {  /* 是尾节点 */\
				(plist)->tail = pnode;\
			}\
			pnode->next = temp->next;\
			temp->next = pnode;\
		} else {\
			if ((plist)->head != NULL) {  /* 列表不为空 */\
				(plist)->head->prev = pnode;\
			} else {  /* 列表为空 */\
				(plist)->tail = pnode;\
			}\
			pnode->next = (plist)->head;\
			(plist)->head = pnode;\
	    }\
		(plist)->count++;/* 链表中的结点数增1 */\
	}

#define DATA_LIST_FIND(plist, member, value, pnode)\
{\
	pnode = (plist)->head;\
	while(NULL != pnode)\
	    if(pnode->member == value)\
			break;\
}
/*------------------------------------------------------------------------------
Section: Type Definitions
------------------------------------------------------------------------------*/
//事件类型
//typedef enum
//{
//    ALARM_TYPE_STAT = 0,
//	ALARM_TYPE_OLIMIT
//}alarm_type_e;

//事件类型
//typedef enum
//{
//    DATA_TYPE_NONE = 0,
//    DATA_TYPE_STAT,
//	DATA_TYPE_FLOAT32
//}data_type_e;

//告警类型定义
typedef enum
{
    ALARM_TYPE_NONE = 0,
    ALARM_TYPE_ON2OFF,
	ALARM_TYPE_OFF2ON,
	ALARM_TYPE_OLIMIT
}alarm_type_e;

//告警状态定义
typedef enum
{
    ALARM_STAT_NONE = 0,
    ALARM_STAT_ON2OFF,
	ALARM_STAT_OFF2ON,
	ALARM_STAT_OHLIMIT,
	ALARM_STAT_OLLIMIT,
	ALARM_STAT_RESUME,
	ALARM_STAT_MAX
}alarm_stat_e;

//单个变量告警状态数据存储结构
typedef struct
{
    uint16 dev_type_id;
	uint8 dev_id;
	uint16 pn;
	alarm_stat_e alarm_stat;
}alarm_stat_t;

//状态事件
//typedef struct
//{
//    uint16 dev_type_id;		//设备类型
//	uint8 dev_id;			//设备号
//	uint16 pn;				//变量号
//	boolean lstat;			//发生前状态
//	boolean cstat;			//发生后状态
//    struct timeval tv;		//发生时间
//}stat_alarm_t;

//越限事件
//typedef struct
//{
//    uint16 dev_type_id;		//设备类型
//	uint8 dev_id;			//设备号
//	uint16 pn;				//变量号
//	float32 lvalue;			//发生前值
//	float32 cvalue;			//发生后值
//    struct timeval tv;		//发生时间
//}olimit_alarm_t;

//事件存储结构体
//typedef struct
//{
//    alarm_type_e evtp;
//	union
//	{
//	    stat_alarm_t stat_evt;
//		olimit_alarm_t olmt_evt;
//	}evt;
//}alarm_t;

//typedef struct
//{
//    uint16 dev_type_id;
//	uint8 dev_id;
//	uint16 pn;
//	data_type_e dtyp;
//	union
//	{
//	    boolean cstat;
//		float32 cvalue;
//	}d;
//}data_t;

//typedef struct _var_t
//{
//    uint16 pn;
//	struct _var_t * prev;
//	struct _var_t * next;
//	data_type_e dtyp;
//	union
//	{
//	    boolean lstat;
//		float32 lvalue;
//	}ld;
//	uint8 evt_stat;					//事件发生状态
//}var_t;

//typedef struct
//{
//    var_t *head;
//	var_t *tail;
//	uint32 count;
//}var_list_t;

//typedef struct _dev_t
//{
//	uint8 dev_id;
//	struct _dev_t * prev;
//	struct _dev_t * next;
//	var_list_t vlist;
//}adev_t;

//typedef struct
//{
//    adev_t *head;
//	adev_t *tail;
//	uint32 count;
//}dev_list_t;

//typedef struct _dtp_t
//{
//    uint16 dev_type_id;
//	struct _dtp_t * prev;
//	struct _dtp_t * next;
//	dev_list_t dlist;
//}dtp_t;

//typedef struct 
//{
//    dtp_t *head;
//	dtp_t *tail;
//	uint32 count;
//}data_list_t;

typedef struct
{
    time_t starttime;		//筛选的开始时间
	time_t endtime;			//筛选的结束时间
}alarm_requestoftime_t;

/*------------------------------------------------------------------------------
Section: Globals
------------------------------------------------------------------------------*/
/* None */
/*------------------------------------------------------------------------------
Section: Function Prototypes
------------------------------------------------------------------------------*/
extern void alarm_init( void );
extern void alarm_write(char* devname, char *varname, alarm_stat_e type, char *message);
extern opt_rst_e alarm_request( param_opt_t *req );
extern void alarm_hook( boolean flag );

#endif

