/********************************************************************************

 **** Copyright (C), 2015, BinHong tech Co., Ltd.                ****

 ********************************************************************************
 * File Name     : dev.h
 * Author        : chenxu
 * Date          : 2015-11-25
 * Version       : 1.0
 * Function List :
 * Description   : �豸������ݽṹ����
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

//IO�豸��������

//�豸��Ϣ�ļ�����



//ͨ��Э�����Ͷ���
#define PROTOCOL_NONE				0	//δ֪Э��
#define PROTOCOL_IO					1	//IOЭ��
#define PROTOCOL_MODBUS				2	//ModbusЭ��

#define DEV_NAME_LEN		50
#define VAR_NAME_LEN		50
#define MOUNT_NAME_LEN		50

#define MAGIC_VALUE			7777.7777

#define PATH				"/usr/devmgr"			//�豸�����ļ����Ŀ¼
#define CFG_PATH			"/usr/httproot/cfg/"			


#define DEV_OFFLINE			0
#define DEV_ONLINE			1

/*------------------------------------------------------------------------------
Section: Type Definitions
------------------------------------------------------------------------------*/
/* �������״̬ */
//#pragma pack(push)
/* �趨Ϊ1�ֽڶ��� */
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

//�豸״̬����
typedef struct
{
    uint8 is_online;				//�豸�Ƿ�����
	uint32 dumb_num;				//�豸��������Ӧ����
	struct timeval lst_rd_time;		//�豸���һ����Ӧʱ��
}dev_stat_t;

/* �ָ�����״̬ */
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
