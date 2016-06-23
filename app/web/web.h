/********************************************************************************

 **** Copyright (C), 2015, BinHong tech Co., Ltd.                ****

 ********************************************************************************
 * File Name     : web.h
 * Author        : chenxu
 * Date          : 2015-11-30
 * Version       : 1.0
 * Function List :
 * Description   : ��web��������ؽӿ�ʵ��
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

//web��Ϣ���Ͷ���
//#define MSG_TYPE_ERROR			-1	//����
//#define MSG_TYPE_CONFIRM 			0	//����Ϣ
//#define MSG_TYPE_SET_DEV_IO			1	//����IO�豸����������Ϣ
//#define MSG_TYPE_SET_DEV_485		2	//����485�豸����������Ϣ
//#define MSG_TYPE_SET_DEV_ETH		3	//����ETH�豸����������Ϣ

//#define MSG_TYPE_SET_VAR_IO			4	//���ñ�������������Ϣ
//#define MSG_TYPE_SET_VAR_485		5	//���ñ�������������Ϣ
//#define MSG_TYPE_SET_VAR_ETH		6	//���ñ�������������Ϣ

//#define MSG_TYPE_SET_SYSCFG			7	//����ϵͳ����������Ϣ

//#define MSG_TYPE_GET_DEV_IO			8	//��ѯIO�豸����������Ϣ
//#define MSG_TYPE_GET_DEV_485		9	//��ѯ485�豸����������Ϣ
//#define MSG_TYPE_GET_DEV_ETH		10	//��ѯETH�豸����������Ϣ

//#define MSG_TYPE_GET_VAR_IO			11	//��ѯ��������������Ϣ
//#define MSG_TYPE_GET_VAR_485		12	//��ѯ��������������Ϣ
//#define MSG_TYPE_GET_VAR_ETH		13	//��ѯ��������������Ϣ

//#define MSG_TYPE_GET_SYSCFG			14	//��ѯϵͳ����������Ϣ

//#define MSG_TYPE_DEL_DEV_IO 		15	//ɾ��IO�豸
//#define MSG_TYPE_DEL_DEV_485 		16	//ɾ��485�豸
//#define MSG_TYPE_DEL_DEV_ETH 		17	//ɾ��ETH�豸

//#define MSG_TYPE_DEL_VAR_IO 		18	//ɾ��IO����
//#define MSG_TYPE_DEL_VAR_485 		19	//ɾ��485����
//#define MSG_TYPE_DEL_VAR_ETH 		20	//ɾ��ETH����

#define WEB_ALARM_FUNC1_REQOFTIME		1
#define WEB_ALARM_FUNC2_REQOFDEVNAME	2

//web ��Ϣ������
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
/* �������״̬ */
//#pragma pack(push)
/* �趨Ϊ1�ֽڶ��� */
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
/*��������*/

typedef enum
{
    OPT_TYPE_SET = 0,
	OPT_TYPE_GET,
	OPT_TYPE_DEL
}opt_type_e;

/*������������*/
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

//����web��Ϣ���ݽṹ
typedef struct
{
    opt_type_e optt;				//��������
    obj_type_e objt;				//��������
    opt_rst_e ret;					//�������
    uint8 data[WEB_MSG_DATA_SIZE];				//��������
}web_msg_t;

typedef struct
{
    opt_type_e optt;				//��������
    obj_type_e objt;				//��������
    void *pin;						//��������
	void *pout;						//�������
}param_opt_t;

//IO�豸�������ݽṹ
typedef struct
{
    uint16 dev_type_id;
	uint8 dev_id;
}webg_dev_info_t;

//����������Ϣ
typedef struct
{
    uint16 dev_type_id;				//�豸��ʶ��
	uint8 dev_id;					//�豸����
	uint16 pn;						//������ʶ��
}webg_var_info_t;

typedef struct
{
    uint16 dev_type_id;				//�豸����
	uint8 dev_id;					//�豸���
	uint16 pn;						//�������
	void *data;						//������Ϣ
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



/* �ָ�����״̬ */
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

