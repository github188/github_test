/********************************************************************************

 **** Copyright (C), 2016, BinHong tech Co., Ltd.                ****

 ********************************************************************************
 * File Name     : trace.h
 * Author        : chenxu
 * Date          : 2016-01-06
 * Version       : 1.0
 * Function List :
 * Description   : log相关
 * Record        :
 * 1.Date        : 2016-01-06
 *   Author      : chenxu
 *   Modification: Created file

 *******************************************************************************/

/*------------------------------------------------------------------------------
Section: Includes
------------------------------------------------------------------------------*/
#include "../inc/atypes.h"
#include "../web/web.h"
/*------------------------------------------------------------------------------
Section: Macro Definitions
------------------------------------------------------------------------------*/
#define TR_UTM				0
#define TR_UTM_BUF			1

#define TR_COMM				2
#define TR_COMM_BUF			3

#define TR_DEV_IO			4
#define TR_DEV_IO_BUF		5

#define TR_DEV_SER			6
#define TR_DEV_SER_BUF		7

#define TR_DEV_ETH			8
#define TR_DEV_ETH_BUF		9

#define TR_AID				10

#define TR_WEB				11

#define TR_SYSTEM			12

#define TR_ALARM			13

#define TR_EVENT			14		//此类日志不做开关控制，直接存储


#define TRCBIT(bitnb)   ( the_trace_mask & (((unsigned long)0x1) << (bitnb)) )

/*------------------------------------------------------------------------------
Section: Type Definitions
------------------------------------------------------------------------------*/
/* None */
/*------------------------------------------------------------------------------
Section: Globals
------------------------------------------------------------------------------*/
/* None */
/*------------------------------------------------------------------------------
Section: Function Prototypes
------------------------------------------------------------------------------*/
extern void 
trace(uint8 tid, 
		const char *msg, ...);

extern void
trace_buf(uint8 tid,
			char *format,
            uint8 *buffer,
            int32 len);
extern void 
time2format( time_t * stp, 
			char *format );

//extern void 
//trace_mask_update(uint32 trace_mask);

extern void 
printtime( struct timeval *tv );

extern opt_rst_e 
trace_mgr( param_opt_t *req );

extern void 
log_init(void);
