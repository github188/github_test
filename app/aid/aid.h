/********************************************************************************

 **** Copyright (C), 2015, BinHong tech Co., Ltd.                ****

 ********************************************************************************
 * File Name     : aid.h
 * Author        : chenxu
 * Date          : 2015-12-14
 * Version       : 1.0
 * Function List :
 * Description   : 辅助业务处理相关定义
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
#define RESET_DELAY_SECONDS			5		//接收到重启命令等待5秒后重启 
#define AID_THREAD_STACK_SIZE		(100 *1024)
/*------------------------------------------------------------------------------
Section: Type Definitions
------------------------------------------------------------------------------*/
typedef struct
{
    pthread_mutex_t aid_mutex;			//互斥信号量
    boolean is_soft_reset;				//是否需要软件复位
    boolean is_hard_reset;				//是否需要硬件复位
//    clock_t  rst_cmd_tick;				//接收复位命令时间
    struct timeval  rst_cmd_tick;				//接收复位命令时间
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

