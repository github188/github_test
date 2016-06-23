/********************************************************************************

 **** Copyright (C), 2015, BinHong tech Co., Ltd.                ****

 ********************************************************************************
 * File Name     : system.h
 * Author        : chenxu
 * Date          : 2015-12-13
 * Version       : 1.0
 * Function List :
 * Description   : ϵͳ���ö���
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
    in_addr_t mip;			//��������
//    in_addr_t mmask;
//	in_addr_t mgateway;
    uint16 mport;			//���˿�
    
    in_addr_t bip;			//���÷�����
//    in_addr_t bmask;
//	in_addr_t bgateway;
    uint16 bport;			//���ö˿�

    in_addr_t lan1_ip;   		//����IP
    in_addr_t lan1_mask;   		//��������
    in_addr_t lan1_gateway;   	//����

	in_addr_t lan2_ip;   		//����IP
    in_addr_t lan2_mask;   		//��������
    in_addr_t lan2_gateway;   	//����

	int mcflag;					//������Ķ˿�

    uint32 secofhbi;			//����֡��������
    uint32 secofsign;			//ǩ��֡��������
	uint32 lk_time_out;			//���ӳ�ʱʱ��,��λ��
}comm_param_t;

typedef struct
{
    addr_t mc_addr;					//������ĵ�ַ
	addr_t su_addr;					//��������ַ
}utm_param_t;

typedef struct
{
    char wsdl_addr[100];			//��Լ��ַ�ַ���
	char pre_addr[4];				//�豸ID4�ֽ�ǰ���ַ�
}webservice_param_t;

typedef struct
{
    char comm_mod[50];				//����ͨ�ŷ�ʽ
	webservice_param_t wsdlp;		//wsdl�������
//	uint32 the_trace_mask;			//��־����
}exp_param_t;

typedef struct 
{    
	comm_param_t comp;				//ͨ�Ų���
	
	utm_param_t utmp;				//Э��ģ����ز���
#ifdef DNS_CFG
	in_addr_t dns1;					//��������
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

