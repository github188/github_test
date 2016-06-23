/********************************************************************************

 **** Copyright (C), 2015, BinHong tech Co., Ltd.                ****

 ********************************************************************************
 * File Name     : comm.h
 * Author        : chenxu
 * Date          : 2015-12-08
 * Version       : 1.0
 * Function List :
 * Description   : ʵ����������ά�����ӵ���ش���
 * Record        :
 * 1.Date        : 2015-12-08
 *   Author      : chenxu
 *   Modification: Created file

 *******************************************************************************/
#ifndef __COMM_H__
#define __COMM_H__
/*------------------------------------------------------------------------------
Section: Includes
------------------------------------------------------------------------------*/
#include <time.h>
#include "../inc/atypes.h"
#include <arpa/inet.h>
#include "../system/system.h"
/*------------------------------------------------------------------------------
Section: Macro Definitions
------------------------------------------------------------------------------*/
#define COMM_THREAD_NAME		"COM"
#define MSGQ_COMM_SEND_NAME		"/comm-send"
#define MSGQ_COMM_RECV_NAME		"/comm-recv"

#define COMM_THREAD_STACK_SIZE		(100 * 1024)

/*------------------------------------------------------------------------------
Section: Type Definitions
------------------------------------------------------------------------------*/
/* �������״̬ */
//#pragma pack(push)
/* �趨Ϊ1�ֽڶ��� */
//#pragma pack(1)

typedef enum
{
    LINK_STAT_RESET = 0,
	LINK_STAT_WAIT_PHY,
	LINK_STAT_CSOCKET,
	LINK_STAT_CONNECT,
	LINK_STAT_LOGIN
}link_stat_e;

typedef struct
{
    link_stat_e link_stat;		//����״̬
    boolean isbakserver;		//true-ʹ�ñ��÷�����,false-ʹ����������
    time_t slink_time;			//��ʼ����ʱ��
    struct timeval login_stp;	//��¼ʱ��
//    clock_t lst_hbi_tick;		//��һ�η�������ʱ��
//    clock_t lst_sign_tick;		//��һ�η�������ʱ��
    struct timeval lst_hbi_tick;		//��һ�η�������ʱ��
    struct timeval lst_sign_tick;		//��һ�η�������ʱ��    
    uint32 noofhblost;			//������ȷ�ϴ���
    boolean flagofparamchange;	//����������
}comm_info_t;

typedef struct
{
    pthread_mutex_t comm_mutex;
    comm_param_t comm_param;
	comm_info_t comm_info;
	VOIDFUNCPTRBOOL login_callback;
	uint8 pbuf[512];
}comm_t;


/* �ָ�����״̬ */
//#pragma pack(pop)
/*------------------------------------------------------------------------------
Section: Globals
------------------------------------------------------------------------------*/
/* None */
/*------------------------------------------------------------------------------
Section: Function Prototypes
------------------------------------------------------------------------------*/
extern pthread_t tid_comm;
extern void comm_send( uint8 *data, uint32 len);
extern int32 comm_recv( void *data );
extern void comm_init( void );
extern void comm_notifyofparamchange( void );
#endif
