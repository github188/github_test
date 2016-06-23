/********************************************************************************

 **** Copyright (C), 2015, BinHong tech Co., Ltd.                ****

 ********************************************************************************
 * File Name     : eth.h
 * Author        : chenxu
 * Date          : 2015-12-23
 * Version       : 1.0
 * Function List :
 * Description   : eth��ض���
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
#define NetDev_xml					"NetDev.xml"			//�����豸�ļ���
#define ETH_THREAD_STACK_SIZE		(100 * 1024)
/*------------------------------------------------------------------------------
Section: Type Definitions
------------------------------------------------------------------------------*/
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
}var_eth_param_t;

//�������ݶ���
typedef struct
{
    float32 lv;						//��һ�βɼ�����
	struct timeval dt;				//����ʱ��
}var_eth_data_t;

typedef struct _var_eth_t_
{
    struct _var_eth_t_ *prev;		//ǰһ���ڵ�
    struct _var_eth_t_ *next;		//��һ���ڵ�
    var_eth_param_t var_p;			//��������
    var_eth_data_t d;				//��һ�βɼ�����
    alarm_stat_e alarm_stat;        //�澯״̬
}var_eth_t;

typedef struct
{
	var_eth_t *head;				/* head in list */
	var_eth_t *tail;				/* tail in list */
	uint32 count;					/* node counter */
}var_eth_list_t;

//����ͨ���豸��ؽṹ�嶨��
typedef struct
{
	uint16 dev_type_id;				//�豸��ʶ��
    uint8 dev_name[DEV_NAME_LEN];	//�豸����
    uint8 dev_id;					//�豸ID
    uint32 dumb_num;				//����n������Ӧ��Ϊ����
    uint8 protocol_type;			//ͨ��Э������
    uint8 addr;						//ͨ�ŵ�ַ
}dev_eth_param_t;

typedef struct _dev_eth_t_
{
    struct _dev_eth_t_ * prev;		//ָ����һ��eth�豸
    struct _dev_eth_t_ * next;		//ָ����һ��eth�豸
    dev_eth_param_t dev_p;			//�豸����
    dev_stat_t dev_stat;			//�豸״̬��Ϣ
	var_eth_list_t varlist;			//�����б�
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
	boolean flagofnetparamchange;	//����������
	link_stat_e link_stat;			//����״̬
	boolean flagofdevparamrequest;	//�����豸/��������������,�Ա�web�ļ�ʱ��Ӧ
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