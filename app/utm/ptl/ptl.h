/********************************************************************************

 **** Copyright (C), 2015, BinHong tech Co., Ltd.                ****

 ********************************************************************************
 * File Name     : ptl.h
 * Author        : chenxu
 * Date          : 2015-11-18
 * Version       : 1.0
 * Function List :
 * Description   : HMAC 上行通信协议API
 * Record        :
 * 1.Date        : 2015-11-18
 *   Author      : chenxu
 *   Modification: Created file

 *******************************************************************************/
#ifndef PTL_H
#define PTL_H
/*------------------------------------------------------------------------------
Section: Includes
------------------------------------------------------------------------------*/
#include "protocol.h"
/*------------------------------------------------------------------------------
Section: Macro Definitions
------------------------------------------------------------------------------*/
/* None */
/*------------------------------------------------------------------------------
Section: Type Definitions
------------------------------------------------------------------------------*/
/* None */
/*------------------------------------------------------------------------------
Section: Globals
------------------------------------------------------------------------------*/
extern const char *value_type[];
extern const char *access_type[];
extern status_t get_item_cfg_info( uint16 dev_type_id, uint16 id, dev_param_t *item );
extern value_type_e get_param_type( uint16 dev_type_id, uint16 pn );
extern status_t get_var_value( uint8* data, var_cfg_t vcfg, float32 *value, uint32 * len);
extern status_t pack_var_value( uint8 *pack, float32 value, var_cfg_t vcfg, uint32 *len );
extern time_t ptl_get_time( uint8 * data);
extern ptl_rtn_e ptl_parse( uint8 *frm, uint32 len, ptl_rst_t *ptl);
extern ptl_rtn_e ptl_pack( ptl_rst_t *ptl, uint8 *pack, int32 *len);
extern value_type_e get_value_type( char *str );
extern uint8 get_access( char *str );
//extern void printbuffer(char *format, uint8 *buffer, uint32 len);
/*------------------------------------------------------------------------------
Section: Function Prototypes
------------------------------------------------------------------------------*/
/* None */

#endif
