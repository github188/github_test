/********************************************************************************

 **** Copyright (C), 2015, BinHong tech Co., Ltd.                ****

 ********************************************************************************
 * File Name     : dev.h
 * Author        : chenxu
 * Date          : 2015-11-25
 * Version       : 1.0
 * Function List :
 * Description   : 设备相关数据结构定义
 * Record        :
 * 1.Date        : 2015-11-25
 *   Author      : chenxu
 *   Modification: Created file

 *******************************************************************************/
#ifndef _DEV_H_
#define _DEV_H_
/*------------------------------------------------------------------------------
Section: Includes
------------------------------------------------------------------------------*/
#include "atypes.h"
#include "ptl.h"
#include <arpa/inet.h>
#include <sys/time.h>
#include "web.h"
//#include "io.h"
//#include "485.h"
#include "errno.h"
/*------------------------------------------------------------------------------
Section: Macro Definitions
------------------------------------------------------------------------------*/

//IO设备索引定义

//设备信息文件定义



//通信协议类型定义
#define PROTOCOL_NONE				0	//未知协议
#define PROTOCOL_IO					1	//IO协议
#define PROTOCOL_MODBUS				2	//Modbus协议

#define DEV_NAME_LEN		50
#define VAR_NAME_LEN		50
#define MOUNT_NAME_LEN		50

#define MAGIC_VALUE			7777.7777

#define PATH				"/usr/devmgr"			//设备参数文件存放目录
#define CFG_PATH			"/usr/httproot/cfg/"			


#define DEV_OFFLINE			0
#define DEV_ONLINE			1

/*------------------------------------------------------------------------------
Section: Type Definitions
------------------------------------------------------------------------------*/
/* 保存对齐状态 */
//#pragma pack(push)
/* 设定为1字节对齐 */
//#pragma pack(1)

typedef enum
{
    MOUNT_TYPE_NONE = 0,
    MOUNT_TYPE_IO,					//IO?????
	MOUNT_TYPE_485,					//485?????
	MOUNT_TYPE_ETH					//Ethernet?????
}mount_type_e;

typedef struct
{
    opt_type_e optt;
	void * pin;
	void * pout;
}devmgr_t;

//设备状态定义
typedef struct
{
    uint8 is_online;				//设备是否在线
	uint32 dumb_num;				//设备连续无响应次数
	struct timeval lst_rd_time;		//设备最后一次响应时间
}dev_stat_t;

/* 恢复对齐状态 */
//#pragma pack(pop)
/*------------------------------------------------------------------------------
Section: Globals
------------------------------------------------------------------------------*/
extern const char * ptl_type_string[];

extern opt_rst_e dev_mgr( param_opt_t *opt );
extern utm_rst_e dev_read_sd( uint16 dev_type_id, uint8 dev_id, uint8 *pack, uint32 *len );
extern utm_rst_e dev_ctl( uint8 *data );
extern void report03_devoffline( uint16 dev_type_id, uint8 dev_id, uint8 stat );
extern uint32 get_ptltype_sn(char *);
extern void dev_init(void);
/*------------------------------------------------------------------------------
Section: Function Prototypes
------------------------------------------------------------------------------*/
/* None */

#endif
