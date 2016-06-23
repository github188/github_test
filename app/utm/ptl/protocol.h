/********************************************************************************

 **** Copyright (C), 2015, BinHong tech Co., Ltd.                ****

 ********************************************************************************
 * File Name     : protocol.h
 * Author        : chenxu
 * Date          : 2015-11-18
 * Version       : 1.0
 * Function List :
 * Description   : HMAC 上行通信协议相关定义
 * Record        :
 * 1.Date        : 2015-11-18
 *   Author      : chenxu
 *   Modification: Created file

 *******************************************************************************/
#ifndef _PROTOCOL_H_
#define _PROTOCOL_H_

/*------------------------------------------------------------------------------
Section: Includes
------------------------------------------------------------------------------*/
#include "../inc/atypes.h"
/*------------------------------------------------------------------------------
Section: Macro Definitions
------------------------------------------------------------------------------*/
/*协议版本*/
#define PTL_VERSION			0x01

//帧类型定义
#define	FRMCTG_RSV			0x00		//保留
#define	FRMCTG_RPT			0x01		//上报帧，SU向MC上报采集数据
#define FRMCTG_READ_RES		0x02		//读取帧对应的应答帧
#define	FRMCTG_RPT_OFFLINE	0x03		//上报设备离线信息
#define	FRMCTG_READ_SD		0x20		//读取帧，MC读取相关参数
#define	FRMCTG_CTL			0x21		//控制帧，MC设置相关参数
#define	FRMCTG_TIME_SYN		0x22		//对时帧
#define	FRMCTG_RESET		0x23		//设备重启

#define	FRMCTG_CONFIRM		0xFF		//确认帧，帧内容为空，在不需要返回实际内容时使用
#define	FRMCTG_HEATBEAT		0xFE		//心跳帧，帧内容为空，用于维持链路
#define	FRMCTG_SIGN			0xFD		//签到帧
#define	FRMCTG_ERR			0xFC		//错误帧

//设备标识码定义
#define	DEV_TYPE_TEMP		0x0001		//温湿度传感器
#define	DEV_TYPE_LEAK		0x0002		//水浸x(协议)
#define	DEV_TYPE_LEAK_STAT	0x0003		//水浸k(开关量)
#define	DEV_TYPE_ACR		0x0004		//电量ACR
#define	DEV_TYPE_AMC		0x0005		//电量AMC
#define	DEV_TYPE_AIRCOND	0x0006		//精密空调
#define	DEV_TYPE_UPS		0x0007		//UPS
#define	DEV_TYPE_VL_ALARM	0x0008		//声光报警

//访问读写定义
#define	ACCESS_R 	0		//只读
#define	ACCESS_W 	1		//只写
#define	ACCESS_RW 	2		//可读可写
#define ACCESS_NONE	3		//不可读写

//错误子定义
//#define

/*------------------------------------------------------------------------------
Section: Type Definitions
------------------------------------------------------------------------------*/
/* 保存对齐状态 */
//#pragma pack(push)
/* 设定为1字节对齐 */
//#pragma pack(1)

//协议处理返回状态
typedef enum
{
    PTL_OK = 0,
	PTL_CTG_ERR,	//帧类别不支持
	PTL_FMT_ERR,	//数据格式不支持
	PTL_CS_ERR,		//校验错误
	PTL_VER_ERR		//协议版本错误
}ptl_rtn_e;

//数据格式定义
typedef struct
{
    uint8 dot : 3;			//小数点位置：0-7，0无小数，1一位小数，2两位小数
    uint8 rsv : 1;			//保留位
    uint8 nbytes : 2;		//00 2个字节,01 1个字节,10 4个字节,11 保留位
    uint8 value_type : 1;	//数值类型 0数值 1开关量
	uint8 s : 1;			//0：无符号数 1：有符号数

}param_fmt_t;

//数据类型定义
typedef enum
{
    VALUE_TYPE_NONE = 0,
	VALUE_TYPE_INT8,			//
	VALUE_TYPE_UINT8,
	VALUE_TYPE_INT16,
	VALUE_TYPE_UINT16,
	VALUE_TYPE_INT32,
	VALUE_TYPE_UINT32,
	VALUE_TYPE_FLOAT32,
	VALUE_TYPE_MAX
}value_type_e;

//设备参数标识表结构定义
typedef struct
{
	uint16	pn;	//参数标识码
	value_type_e vt;	//参数类型
	uint8 dot;	//精度,0,1,2
	uint8 rw;	//读写类型
}dev_param_t;

typedef struct
{
    uint16 pn;
	value_type_e vt;
	float32 dataprecision;
}var_cfg_t;

//设备参数配置
typedef struct
{
    uint16 dev_type_id;				//设备标识码
    dev_param_t * dev_param_cfg;	//设备参数配置
    uint16 item_count;				//配置项数量
}dev_conf_t;

//参数单元结构定义
//typedef struct
//{
//    uint8 pn;			//参数名称
//    param_fmt_t pfmt;	//数据格式
//}param_t;

//参数节点
typedef struct _param_node_t
{
    uint8 pn;			//参数名
    value_type_e vt;	//数据类型
    uint8 dot;			//小数点个数
    union
	{
	    int8 value_i8;
		uint8 value_ui8;
		int16 value_i16;
		uint16 value_ui16;
		int32 value_i32;
		uint32 value_ui32;
		float32 value_f32;
		float32 value;
	}u;
	struct _param_node_t *next;
}param_node_t;

typedef struct
{
    uint16 dev_type_id;	//设备类型标识
    param_node_t * plist;
}param_list_t;

//7字节时间格式
typedef struct
{
    uint8 year_h;        /**< 年高字节 */
	uint8 year_l;		 /**< 年低字节 */
    uint8 month;     	 /**< 月      */
    uint8 day;          /**< 日      */
    uint8 hour;         /**< 时      */
    uint8 minute;       /**< 分      */
    uint8 second;       /**< 秒      */
} ptl_time_t;

//源地址	5字节BCD	地址格式为：设备类型（1）-年（1）-月（1）-生产编号（2）
//例：01 15 10 0001，类型为00为服务器 01为控制器，年月为15年10月，生产编号0001
typedef	struct
{
    uint8 dev_type;
	uint8 year;
	uint8 mon;
	uint8 fac_id[2];
}addr_t;

//协议解析输出结构体
typedef struct
{
	uint8 frm_ctg;		//帧类别
	time_t stp;			//消息时间戳
	addr_t src_addr;	//源地址
	addr_t dest_addr;	//目的地址
	uint16 msg_id;		//消息ID
	uint16 datalen;		//帧内容部分长度
	uint8 *data;		//消息正文
}ptl_rst_t;

/* 恢复对齐状态 */
//#pragma pack(pop)

/*------------------------------------------------------------------------------
Section: Globals
------------------------------------------------------------------------------*/
/* None */
/*------------------------------------------------------------------------------
Section: Function Prototypes
------------------------------------------------------------------------------*/
/* None */


#endif
