/********************************************************************************

 **** Copyright (C), 2015, BinHong tech Co., Ltd.                ****

 ********************************************************************************
 * File Name     : aid.h
 * Author        : chenxu
 * Date          : 2015-12-14
 * Version       : 1.0
 * Function List :
 * Description   : ����ҵ������ض���
 * Record        :
 * 1.Date        : 2015-12-14
 *   Author      : chenxu
 *   Modification: Created file

 *******************************************************************************/

/*------------------------------------------------------------------------------
Section: Includes
------------------------------------------------------------------------------*/
#include "../inc/atypes.h"
#include <time.h>
/*------------------------------------------------------------------------------
Section: Macro Definitions
------------------------------------------------------------------------------*/
#define AID_THREAD_NAME				"AID"
#define HARD_RESET					1
#define SOFT_RESET					2
#define RESET_DELAY_SECONDS			5		//���յ���������ȴ�5������� 
#define AID_THREAD_STACK_SIZE		(100 *1024)
/*------------------------------------------------------------------------------
Section: Type Definitions
------------------------------------------------------------------------------*/
typedef struct
{
    pthread_mutex_t aid_mutex;			//�����ź���
    boolean is_soft_reset;				//�Ƿ���Ҫ�����λ
    boolean is_hard_reset;				//�Ƿ���ҪӲ����λ
//    clock_t  rst_cmd_tick;				//���ո�λ����ʱ��
    struct timeval  rst_cmd_tick;				//���ո�λ����ʱ��
}aid_t;
/*------------------------------------------------------------------------------
Section: Globals
------------------------------------------------------------------------------*/
/* None */
/*------------------------------------------------------------------------------
Section: Function Prototypes
------------------------------------------------------------------------------*/
extern pthread_t tid_aid;
extern pthread_t tid_bp;
extern void aid_set_reset( uint8 reset);
extern void aid_init( void );

