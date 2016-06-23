/********************************************************************************

 **** Copyright (C), 2015, BinHong tech Co., Ltd.                ****

 ********************************************************************************
 * File Name     : hth.h
 * Author        : chenxu
 * Date          : 2015-12-19
 * Version       : 1.0
 * Function List :
 * Description   : hth寄存器数值解析接口
 * Record        :
 * 1.Date        : 2015-12-19
 *   Author      : chenxu
 *   Modification: Created file

 *******************************************************************************/
#ifndef __HTH_H__
#define __HTH_H__
/*------------------------------------------------------------------------------
Section: Includes
------------------------------------------------------------------------------*/
#include "atypes.h"
#include "modbus_lib.h"
/*------------------------------------------------------------------------------
Section: Macro Definitions
------------------------------------------------------------------------------*/
/* None */
/*------------------------------------------------------------------------------
Section: Type Definitions
------------------------------------------------------------------------------*/
typedef float32 (*MBREG2VALUEFUNCPTR) (uint16, uint8 *, float32, void *);

typedef struct
{
    mb_ch_type_e chantyp;		//通道类型
    int port;					//通道号
    uint8 devaddr;				//modbus设备地址
}modbus_chan_t;
/*------------------------------------------------------------------------------
Section: Globals
------------------------------------------------------------------------------*/
/* None */
/*------------------------------------------------------------------------------
Section: Function Prototypes
------------------------------------------------------------------------------*/
extern void hth_value_to_reg( float32 value, float32 dataprecision, uint8 *buf );
extern float32 hth_reg_to_value( uint16 dev_type_id, uint16 pn, uint8 *buf, float32 dataprecision, modbus_chan_t *modchan);

#endif
