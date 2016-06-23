/********************************************************************************

 **** Copyright (C), 2015, BinHong tech Co., Ltd.                ****

 ********************************************************************************
 * File Name     : system.h
 * Author        : chenxu
 * Date          : 2015-12-13
 * Version       : 1.0
 * Function List :
 * Description   : 系统配置定义
 * Record        :
 * 1.Date        : 2015-12-13
 *   Author      : chenxu
 *   Modification: Created file

 *******************************************************************************/
#ifndef __SYSTEM_H__
#define __SYSTEM_H__
/*------------------------------------------------------------------------------
Section: Includes
------------------------------------------------------------------------------*/
#include <arpa/inet.h>
#include "../inc/atypes.h"
#include "../ptl/protocol.h"
#include "../web/web.h"
#include <pthread.h>
/*------------------------------------------------------------------------------
Section: Macro Definitions
------------------------------------------------------------------------------*/
#define DNS_CFG

#define F_SYS_CFG					"/usr/devmgr/sys.cfg"
#define USER_LEN_MAX				30
#define PASSWD_LEN_MAX				30
/*------------------------------------------------------------------------------
Section: Type Definitions
------------------------------------------------------------------------------*/
typedef struct
{  
    in_addr_t mip;			//主服务器
//    in_addr_t mmask;
//	in_addr_t mgateway;
    uint16 mport;			//主端口
    
    in_addr_t bip;			//备用服务器
//    in_addr_t bmask;
//	in_addr_t bgateway;
    uint16 bport;			//备用端口

    in_addr_t lan1_ip;   		//本地IP
    in_addr_t lan1_mask;   		//子网掩码
    in_addr_t lan1_gateway;   	//网关

	in_addr_t lan2_ip;   		//本地IP
    in_addr_t lan2_mask;   		//子网掩码
    in_addr_t lan2_gateway;   	//网关

	int mcflag;					//监控中心端口

    uint32 secofhbi;			//心跳帧发送周期
    uint32 secofsign;			//签到帧发送周期
	uint32 lk_time_out;			//链接超时时间,单位秒
}comm_param_t;

typedef struct
{
    addr_t mc_addr;					//监控中心地址
	addr_t su_addr;					//控制器地址
}utm_param_t;

typedef struct
{
    char wsdl_addr[100];			//契约地址字符串
	char pre_addr[4];				//设备ID4字节前导字符
}webservice_param_t;

typedef struct
{
    char comm_mod[50];				//上行通信方式
	webservice_param_t wsdlp;		//wsdl服务参数
//	uint32 the_trace_mask;			//日志开关
}exp_param_t;

typedef struct 
{    
	comm_param_t comp;				//通信参数
	
	utm_param_t utmp;				//协议模块相关参数
#ifdef DNS_CFG
	in_addr_t dns1;					//主服务器
#endif	
}sys_param_t;

/*------------------------------------------------------------------------------
Section: Globals
------------------------------------------------------------------------------*/
/* None */
/*------------------------------------------------------------------------------
Section: Function Prototypes
------------------------------------------------------------------------------*/
extern void system_init( void );
//extern void sys_get_param ( sys_param_t * syscfg );
extern void system_getcommparam( comm_param_t *comp );
extern void system_getutmparam( utm_param_t *utmp );
//extern void system_gettracemask( uint32 *trace_mask );
extern void system_getwsdlparam( webservice_param_t *wsdlp );
extern void system_getcommod(char * mode);
extern opt_rst_e system_mgr( param_opt_t *opt);
extern opt_rst_e usr_mgr( param_opt_t *opt );

#endif

