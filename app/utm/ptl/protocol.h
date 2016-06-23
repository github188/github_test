/********************************************************************************

 **** Copyright (C), 2015, BinHong tech Co., Ltd.                ****

 ********************************************************************************
 * File Name     : protocol.h
 * Author        : chenxu
 * Date          : 2015-11-18
 * Version       : 1.0
 * Function List :
 * Description   : HMAC ����ͨ��Э����ض���
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
/*Э��汾*/
#define PTL_VERSION			0x01

//֡���Ͷ���
#define	FRMCTG_RSV			0x00		//����
#define	FRMCTG_RPT			0x01		//�ϱ�֡��SU��MC�ϱ��ɼ�����
#define FRMCTG_READ_RES		0x02		//��ȡ֡��Ӧ��Ӧ��֡
#define	FRMCTG_RPT_OFFLINE	0x03		//�ϱ��豸������Ϣ
#define	FRMCTG_READ_SD		0x20		//��ȡ֡��MC��ȡ��ز���
#define	FRMCTG_CTL			0x21		//����֡��MC������ز���
#define	FRMCTG_TIME_SYN		0x22		//��ʱ֡
#define	FRMCTG_RESET		0x23		//�豸����

#define	FRMCTG_CONFIRM		0xFF		//ȷ��֡��֡����Ϊ�գ��ڲ���Ҫ����ʵ������ʱʹ��
#define	FRMCTG_HEATBEAT		0xFE		//����֡��֡����Ϊ�գ�����ά����·
#define	FRMCTG_SIGN			0xFD		//ǩ��֡
#define	FRMCTG_ERR			0xFC		//����֡

//�豸��ʶ�붨��
#define	DEV_TYPE_TEMP		0x0001		//��ʪ�ȴ�����
#define	DEV_TYPE_LEAK		0x0002		//ˮ��x(Э��)
#define	DEV_TYPE_LEAK_STAT	0x0003		//ˮ��k(������)
#define	DEV_TYPE_ACR		0x0004		//����ACR
#define	DEV_TYPE_AMC		0x0005		//����AMC
#define	DEV_TYPE_AIRCOND	0x0006		//���ܿյ�
#define	DEV_TYPE_UPS		0x0007		//UPS
#define	DEV_TYPE_VL_ALARM	0x0008		//���ⱨ��

//���ʶ�д����
#define	ACCESS_R 	0		//ֻ��
#define	ACCESS_W 	1		//ֻд
#define	ACCESS_RW 	2		//�ɶ���д
#define ACCESS_NONE	3		//���ɶ�д

//�����Ӷ���
//#define

/*------------------------------------------------------------------------------
Section: Type Definitions
------------------------------------------------------------------------------*/
/* �������״̬ */
//#pragma pack(push)
/* �趨Ϊ1�ֽڶ��� */
//#pragma pack(1)

//Э�鴦����״̬
typedef enum
{
    PTL_OK = 0,
	PTL_CTG_ERR,	//֡���֧��
	PTL_FMT_ERR,	//���ݸ�ʽ��֧��
	PTL_CS_ERR,		//У�����
	PTL_VER_ERR		//Э��汾����
}ptl_rtn_e;

//���ݸ�ʽ����
typedef struct
{
    uint8 dot : 3;			//С����λ�ã�0-7��0��С����1һλС����2��λС��
    uint8 rsv : 1;			//����λ
    uint8 nbytes : 2;		//00 2���ֽ�,01 1���ֽ�,10 4���ֽ�,11 ����λ
    uint8 value_type : 1;	//��ֵ���� 0��ֵ 1������
	uint8 s : 1;			//0���޷����� 1���з�����

}param_fmt_t;

//�������Ͷ���
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

//�豸������ʶ��ṹ����
typedef struct
{
	uint16	pn;	//������ʶ��
	value_type_e vt;	//��������
	uint8 dot;	//����,0,1,2
	uint8 rw;	//��д����
}dev_param_t;

typedef struct
{
    uint16 pn;
	value_type_e vt;
	float32 dataprecision;
}var_cfg_t;

//�豸��������
typedef struct
{
    uint16 dev_type_id;				//�豸��ʶ��
    dev_param_t * dev_param_cfg;	//�豸��������
    uint16 item_count;				//����������
}dev_conf_t;

//������Ԫ�ṹ����
//typedef struct
//{
//    uint8 pn;			//��������
//    param_fmt_t pfmt;	//���ݸ�ʽ
//}param_t;

//�����ڵ�
typedef struct _param_node_t
{
    uint8 pn;			//������
    value_type_e vt;	//��������
    uint8 dot;			//С�������
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
    uint16 dev_type_id;	//�豸���ͱ�ʶ
    param_node_t * plist;
}param_list_t;

//7�ֽ�ʱ���ʽ
typedef struct
{
    uint8 year_h;        /**< ����ֽ� */
	uint8 year_l;		 /**< ����ֽ� */
    uint8 month;     	 /**< ��      */
    uint8 day;          /**< ��      */
    uint8 hour;         /**< ʱ      */
    uint8 minute;       /**< ��      */
    uint8 second;       /**< ��      */
} ptl_time_t;

//Դ��ַ	5�ֽ�BCD	��ַ��ʽΪ���豸���ͣ�1��-�꣨1��-�£�1��-������ţ�2��
//����01 15 10 0001������Ϊ00Ϊ������ 01Ϊ������������Ϊ15��10�£��������0001
typedef	struct
{
    uint8 dev_type;
	uint8 year;
	uint8 mon;
	uint8 fac_id[2];
}addr_t;

//Э���������ṹ��
typedef struct
{
	uint8 frm_ctg;		//֡���
	time_t stp;			//��Ϣʱ���
	addr_t src_addr;	//Դ��ַ
	addr_t dest_addr;	//Ŀ�ĵ�ַ
	uint16 msg_id;		//��ϢID
	uint16 datalen;		//֡���ݲ��ֳ���
	uint8 *data;		//��Ϣ����
}ptl_rst_t;

/* �ָ�����״̬ */
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
