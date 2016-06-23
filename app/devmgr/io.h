/********************************************************************************

 **** Copyright (C), 2015, BinHong tech Co., Ltd.                ****

 ********************************************************************************
 * File Name     : io.h
 * Author        : chenxu
 * Date          : 2015-12-22
 * Version       : 1.0
 * Function List :
 * Description   : IO设备相关定义
 * Record        :
 * 1.Date        : 2015-12-22
 *   Author      : chenxu
 *   Modification: Created file

 *******************************************************************************/
#ifndef __IO_H__
#define __IO_H__
/*------------------------------------------------------------------------------
Section: Includes
------------------------------------------------------------------------------*/
#include <pthread.h>
#include "dev.h"
#include "atypes.h"
#include "alarm.h"
/*------------------------------------------------------------------------------
Section: Macro Definitions
------------------------------------------------------------------------------*/
#define IO_THREAD_NAME		"IO"
//输入引脚定义
#define DIN1				(128 + 14)	
#define DIN2				(128 + 16)
#define DIN3				(128 + 12)
#define DIN4				(128 + 20)
#define DIN5				(128 + 15)
#define DIN6				(128 + 13)
#define DIN7				(128 + 17)
#define DIN8				(128 + 11)
//输出引脚定义
#define DOUT0				(64 + 20)
#define DOUT1				(64 + 19)
#define DOUT2				(64 + 12)
#define DOUT3				(64 + 11)

//#define F_DEV_IO_CFG		"IoDev.xml"				//io设备参数文件名
#define F_DEV_IO_CFG		"io.cfg"				//io设备参数文件名
#define IoDev_XML			"IoDev.xml"				//io设备参数文件名
#define IO_THREAD_STACK_SIZE		(100 * 1024)	

/*------------------------------------------------------------------------------
Section: Type Definitions
------------------------------------------------------------------------------*/
/* 保存对齐状态 */
//#pragma pack(push)
/* 设定为1字节对齐 */
//#pragma pack(1)

//IO变量告警类型定义
typedef enum
{
    WARN_TYPE_NONE = 0,
    WARN_TYPE_ON ,
	WARN_TYPE_OFF,
	WARN_TYPE_ON2OFF,
	WARN_TYPE_OFF2ON
}io_warn_e;

//变量参数定义
typedef struct
{
    uint16 pn;						//
    uint8 pname[VAR_NAME_LEN];		//
    value_type_e vt;				//
    uint8 dot;						//
    uint8 rw;						//
    uint32 period;					//
    uint8 alarm_enable;				//告警使能
    uint8 warn_string[20];			//告警字符串
    alarm_type_e wt;					//告警类型
}var_io_info_t;

//变量数据定义
typedef struct
{
    boolean stat;					//上一次IO状态
	struct timeval dt;				//数据时标
}var_io_data_t;

typedef struct _var_io_t_
{
    struct _var_io_t_ *prev;		//上一个
    struct _var_io_t_ *next;		//下一个
    var_io_info_t var_p;			//变量参数
    var_io_data_t d;				//IO上一次采集数据
}var_io_t;

typedef struct
{
	var_io_t *head;					/* head in list */
	var_io_t *tail;					/* tail in list */
	uint32 count;					/* node counter */
}var_io_list_t;

//IO设备配置信息
typedef struct 
{
    uint16 dev_type_id;				//设备标识码
    uint8 dev_id;					//设备ID
    uint8 dev_name[DEV_NAME_LEN];	//设备名称
    uint8 mount_name[MOUNT_NAME_LEN];//挂载点名称
    uint32 pin_index;				//IO的引脚索引
}dev_io_info_t;

typedef struct _dev_io_t_
{
    struct _dev_io_t_ * prev;		//指向下一个IO设备
    struct _dev_io_t_ * next;		//指向下一个IO设备
    dev_io_info_t dev_p;			//设备参数
    dev_stat_t dev_stat;			//设备状态信息
    var_io_list_t varlist;			//变量列表
}dev_io_t;

typedef struct
{
	dev_io_t *head;					/* head in list */
	dev_io_t *tail;					/* tail in list */
	uint32 count;					/* node counter */
}dev_io_list_t;

/* 恢复对齐状态 */
//#pragma pack(pop)
/*------------------------------------------------------------------------------
Section: Globals
------------------------------------------------------------------------------*/
//extern const WEBOPTFUNCPTR web_io_opt_fun[];
extern pthread_t tid_io;
/*------------------------------------------------------------------------------
Section: Function Prototypes
------------------------------------------------------------------------------*/

#endif
