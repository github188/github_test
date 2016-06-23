/********************************************************************************

 **** Copyright (C), 2015, BinHong tech Co., Ltd.                ****

 ********************************************************************************
 * File Name     : utm.h
 * Author        : chenxu
 * Date          : 2015-12-07
 * Version       : 1.0
 * Function List :
 * Description   : �����ն��������Ľ�����ش���
 * Record        :
 * 1.Date        : 2015-12-07
 *   Author      : chenxu
 *   Modification: Created file

 *******************************************************************************/
#ifndef __UTM_H__
#define __UTM_H__
/*------------------------------------------------------------------------------
Section: Includes
------------------------------------------------------------------------------*/
#include "../ptl/protocol.h"
#include <pthread.h>
#include "errno.h"
/*------------------------------------------------------------------------------
Section: Macro Definitions
------------------------------------------------------------------------------*/
#define UTM_THREAD_NAME			"UTM"
/*�豸�ϱ���Ϣ��������*/
#define MSGQ_REPORT_NAME		"/rpt"

/*Ӧ�ò㱨������*/
#define APPBUF_MC             	0x01 /*MC���б���*/
#define APPBUF_RPT           	0x02 /*�豸�ϱ���Ϣ*/

/*�ص��������Ͷ���*/
#define CB_HB_CALL		0x01

#define UTM_THREAD_STACK_SIZE	(100 *1024)
/*------------------------------------------------------------------------------
Section: Type Definitions
------------------------------------------------------------------------------*/
/* �������״̬ */
//#pragma pack(push)
/* �趨Ϊ1�ֽڶ��� */
//#pragma pack(1)

typedef struct
{
    uint8 flag;				//֡ͷ	1	68H
	uint16 length;			//����	2	��֡ͷ��ʼ����֡ͷ����У�飨����У�飩���������ֽڸ���
	uint8 ptl_ver;			//Э��汾	1	��һ��Э��Ϊ01H����������
	uint8 rsv1;				//�����ֶ�	1	����
	uint8 frmctg;			//֡���	1	����˵��
	uint8 tbuf[7];			//��Ϣʱ���	7�ֽ�BCD��	20150910105210��Ϊ2015-09-10 10:52:10
	uint8 rsv2[2];			//�����ֶ�	2	����
	addr_t src_addr;		//Դ��ַ	5�ֽ�BCD	
	addr_t dest_addr;		//Ŀ���ַ	5�ֽ�BCD	
	uint16 pfc;				//��ϢID	2	���ڶ���Ϣ���б�ǣ�ÿ�ν����ۼ�
}ptl_header_t;

typedef struct
{
	uint16 dev_type_id;		//�豸��ʶ��
	uint8 dev_id;			//�豸ID
	uint16 datalen;			//��Ϣ�����У������豸���ݳ��Ȳ����豸ID
	uint8 data[128];		//��Ϣ����
}rpt_msg_t;

typedef struct
{
    /**
     * buf����
     * 1: �������ԭʼ����;
     * 2: �豸ʵʱ�����ϱ���Ϣ
     */
    uint8 buftyp;
    uint8 frm_ctg;	//֡����
	uint32 len;
	uint8 data[512];
	
}appbuf_t;	

typedef struct
{
    pthread_mutex_t utm_mutex;

	addr_t suaddr;						//��������ַ��Ϣ

	addr_t mcaddr;						//������ĵ�ַ

	ptl_rst_t lst_frm_info;				//���һ��MC�����ı���ͷ������Ϣ

    appbuf_t appbuf;					//��������Ϣ		

	uint8 pbuf[512];					//�������ݱ��Ļ���
	
	uint16 pfc;							//֡���,֡������

	VOIDFUNCPTRBOOL phb_callback;       //����Ӧ��ص�����  
}utm_t;

/* �ָ�����״̬ */
//#pragma pack(pop)
/*------------------------------------------------------------------------------
Section: Globals
------------------------------------------------------------------------------*/
/* None */
/*------------------------------------------------------------------------------
Section: Function Prototypes
------------------------------------------------------------------------------*/
extern pthread_t tid_utm;
extern void utm_rpt_send(appbuf_t *appbuf);
extern uint16 utm_get_pfc( void );
extern void utm_cb_register( uint8 type, VOIDFUNCPTRBOOL callback );
extern void utm_init(void);

#endif

