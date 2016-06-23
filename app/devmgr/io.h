/********************************************************************************

 **** Copyright (C), 2015, BinHong tech Co., Ltd.                ****

 ********************************************************************************
 * File Name     : io.h
 * Author        : chenxu
 * Date          : 2015-12-22
 * Version       : 1.0
 * Function List :
 * Description   : IO�豸��ض���
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
//�������Ŷ���
#define DIN1				(128 + 14)	
#define DIN2				(128 + 16)
#define DIN3				(128 + 12)
#define DIN4				(128 + 20)
#define DIN5				(128 + 15)
#define DIN6				(128 + 13)
#define DIN7				(128 + 17)
#define DIN8				(128 + 11)
//������Ŷ���
#define DOUT0				(64 + 20)
#define DOUT1				(64 + 19)
#define DOUT2				(64 + 12)
#define DOUT3				(64 + 11)

//#define F_DEV_IO_CFG		"IoDev.xml"				//io�豸�����ļ���
#define F_DEV_IO_CFG		"io.cfg"				//io�豸�����ļ���
#define IoDev_XML			"IoDev.xml"				//io�豸�����ļ���
#define IO_THREAD_STACK_SIZE		(100 * 1024)	

/*------------------------------------------------------------------------------
Section: Type Definitions
------------------------------------------------------------------------------*/
/* �������״̬ */
//#pragma pack(push)
/* �趨Ϊ1�ֽڶ��� */
//#pragma pack(1)

//IO�����澯���Ͷ���
typedef enum
{
    WARN_TYPE_NONE = 0,
    WARN_TYPE_ON ,
	WARN_TYPE_OFF,
	WARN_TYPE_ON2OFF,
	WARN_TYPE_OFF2ON
}io_warn_e;

//������������
typedef struct
{
    uint16 pn;						//
    uint8 pname[VAR_NAME_LEN];		//
    value_type_e vt;				//
    uint8 dot;						//
    uint8 rw;						//
    uint32 period;					//
    uint8 alarm_enable;				//�澯ʹ��
    uint8 warn_string[20];			//�澯�ַ���
    alarm_type_e wt;					//�澯����
}var_io_info_t;

//�������ݶ���
typedef struct
{
    boolean stat;					//��һ��IO״̬
	struct timeval dt;				//����ʱ��
}var_io_data_t;

typedef struct _var_io_t_
{
    struct _var_io_t_ *prev;		//��һ��
    struct _var_io_t_ *next;		//��һ��
    var_io_info_t var_p;			//��������
    var_io_data_t d;				//IO��һ�βɼ�����
}var_io_t;

typedef struct
{
	var_io_t *head;					/* head in list */
	var_io_t *tail;					/* tail in list */
	uint32 count;					/* node counter */
}var_io_list_t;

//IO�豸������Ϣ
typedef struct 
{
    uint16 dev_type_id;				//�豸��ʶ��
    uint8 dev_id;					//�豸ID
    uint8 dev_name[DEV_NAME_LEN];	//�豸����
    uint8 mount_name[MOUNT_NAME_LEN];//���ص�����
    uint32 pin_index;				//IO����������
}dev_io_info_t;

typedef struct _dev_io_t_
{
    struct _dev_io_t_ * prev;		//ָ����һ��IO�豸
    struct _dev_io_t_ * next;		//ָ����һ��IO�豸
    dev_io_info_t dev_p;			//�豸����
    dev_stat_t dev_stat;			//�豸״̬��Ϣ
    var_io_list_t varlist;			//�����б�
}dev_io_t;

typedef struct
{
	dev_io_t *head;					/* head in list */
	dev_io_t *tail;					/* tail in list */
	uint32 count;					/* node counter */
}dev_io_list_t;

/* �ָ�����״̬ */
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
