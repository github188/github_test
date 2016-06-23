/********************************************************************************

 **** Copyright (C), 2015, BinHong tech Co., Ltd.                ****

 ********************************************************************************
 * File Name     : 485.h
 * Author        : chenxu
 * Date          : 2015-12-23
 * Version       : 1.0
 * Function List :
 * Description   : 485��ض���
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
#define F_DEV_485_CFG				"485.cfg"				//io�豸�����ļ���
#define SerialDev_xml				"SerialDev.xml"			//�����豸�ļ���
#define RS_ADDR_LEN					6	//485��ַ�ֽڳ���	
#define RS485_THREAD_STACK_SIZE		(1 * 1024 * 1024)
/*------------------------------------------------------------------------------
Section: Type Definitions
------------------------------------------------------------------------------*/
/* �������״̬ */
//#pragma pack(push)
/* �趨Ϊ1�ֽڶ��� */
//#pragma pack(1)

//������������
typedef struct
{
    uint16 pn;						//����ID
    uint8 pname[VAR_NAME_LEN];		//������
    value_type_e vt;				//��������
    float32 dataprecision;			//���ݾ���
    uint8 rw;						//��дȨ��
    uint32 period;					//��������
    uint16 reg_addr;				//�Ĵ�����ַ
    uint8 alarm_enable;				//�澯ʹ�� 0-disable ,1-enable
    alarm_type_e wt;				//�澯����
    float32 hvalue;					//����
    float32 lvalue;					//����
}var_485_info_t;

//typedef struct
//{
//    var_header1_t header;
//	var_485_info_t vinfo;
//}web_485_varinfo_t;

//�������ݶ���
typedef struct
{
    float32 lv;						//��һ�βɼ�����
	struct timeval dt;				//����ʱ��
}var_485_data_t;

typedef struct _var_485_t_
{
    struct _var_485_t_ *prev;		//ǰһ���ڵ�
    struct _var_485_t_ *next;		//��һ���ڵ�
    var_485_info_t var_p;			//��������
    var_485_data_t d;				//��һ�βɼ�����
    alarm_stat_e alarm_stat;        //�澯״̬
}var_485_t;

typedef struct
{
	var_485_t *head;				/* head in list */
	var_485_t *tail;				/* tail in list */
	uint32 count;					/* node counter */
}var_485_list_t;

//����ͨ���豸��ؽṹ�嶨��
typedef struct
{
	uint16 dev_type_id;				//�豸��ʶ��
    uint8 dev_name[DEV_NAME_LEN];	//�豸����
    uint8 dev_id;					//�豸ID
    uint32 dumb_num;				//����n������Ӧ��Ϊ����
    uint8 protocol_type;			//ͨ��Э������
    uint8 addr;						//ͨ�ŵ�ַ
}dev_485_info_t;

typedef struct _dev_485_t_
{
    struct _dev_485_t_ * prev;		//ָ����һ��485�豸
    struct _dev_485_t_ * next;		//ָ����һ��485�豸
    dev_485_info_t dev_p;			//�豸����
    dev_stat_t dev_stat;			//�豸״̬��Ϣ
	var_485_list_t varlist;			//�����б�
}dev_485_t;

typedef struct
{
	dev_485_t *head;				/* head in list */
	dev_485_t *tail;				/* tail in list */
	uint32 count;					/* node counter */
}dev_485_list_t;

typedef struct
{
    uint8 comn;						//���ں�
    struct
    {
        uint16 dbit :2;   			/**< 0��3��5-8λ����λ                                      */
        uint16 par  :3;    			/**< 0/1��ż/��У��                                         */ 
        uint16 sbit :2;   			/**< 0/1��1/ 2ֹͣλ                                        */
        uint16 baud :4;   			/**< 0 ~ 13 ������(1200, 1800, 2400, 3600, 4800, 7200, 9600, 
        							 14400, 19200, 28800, 38400, 57600, 76800, 115200)*/
		uint16 rsv  :4;				//����
    }p;
}serial_t;

typedef struct 
{
    serial_t serial;				
	pthread_mutex_t serial_mutex;
	dev_485_list_t dev_485_list;
}mount_serial_t;

/* �ָ�����״̬ */
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