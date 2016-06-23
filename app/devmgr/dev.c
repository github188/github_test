/********************************************************************************

 **** Copyright (C), 2015, BinHong tech Co., Ltd.                ****

 ********************************************************************************
 * File Name     : dev.c
 * Author        : chenxu
 * Date          : 2015-11-25
 * Version       : 1.0
 * Function List :
 * Description   : 设备处理相关API
 * Record        :
 * 1.Date        : 2015-11-25
 *   Author      : chenxu
 *   Modification: Created file

 *******************************************************************************/

/*------------------------------------------------------------------------------
Section: Includes
------------------------------------------------------------------------------*/
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <string.h>
#include "atypes.h"
#include "dev.h"
#include "./private/hth.h"
#include <fcntl.h>
#include <pthread.h>
#include "maths.h"
#include "gpio.h"
#include "protocol.h"
#include "utm.h"
#include "485.h"
#include "io.h"
#include "modbus_lib.h"
#include "errno.h"
#include "trace.h"
/*------------------------------------------------------------------------------
Section: Macro Definitions
------------------------------------------------------------------------------*/
/*NONE*/
/*------------------------------------------------------------------------------
Section: Globals
------------------------------------------------------------------------------*/
static pthread_mutex_t dev_mutex;

const char * ptl_type_string[] = 
{
	"NULL",
	"IO",
	"Modbus"
};

extern void dev_io_init( void );
extern void dev_485_init(void);
extern opt_rst_e dev_io_mgr(devmgr_t *req);
extern opt_rst_e var_io_mgr( devmgr_t *req );
extern opt_rst_e ser_485_mgr( devmgr_t *req );
extern opt_rst_e dev_485_mgr( devmgr_t *req );
extern opt_rst_e var_485_mgr( devmgr_t *req );
extern opt_rst_e mnt_eth_mgr( devmgr_t *req );
extern opt_rst_e dev_eth_mgr( devmgr_t *req );
extern opt_rst_e var_eth_mgr( devmgr_t *req );
extern utm_rst_e read_io_sd( uint16 dev_type_id, uint8 dev_id, uint8* pack, uint32 *len );
extern utm_rst_e read_485_sd( uint16 dev_type_id, uint8 dev_id, uint8* pack, uint32 *len );
extern utm_rst_e ctrl_485(uint8* data);
extern utm_rst_e ctrl_io(uint8* data);

/*****************************************************************************
 * Function      : get_ptltype_sn
 * Description   : get_ptltype_sn
 * Input         : uint32
 * Output        : None
 * Return        : char *str
 * Others        : 
 * Record
 * 1.Date        : 20151217
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
uint32 get_ptltype_sn( char *str)
{
    uint32 len;
	uint32 idx;

	len = strlen(str);
	for(idx = 0; idx < ARRAY_SIZE(ptl_type_string); idx++)
	{
		if((len == strlen(ptl_type_string[idx]))
			&& (strcmp(str, ptl_type_string[idx]) == 0))
			break;
	}
	if(idx >= ARRAY_SIZE(ptl_type_string))
		idx = PROTOCOL_NONE;

	return idx;
}

/*****************************************************************************
 * Function      : report03_devoffline
 * Description   : 上报设备在线状态信息
 * Input         : uint16 dev_type_id
                uint8 dev_id
                uint8 stat 在线状态
 * Output        : None
 * Return        : void
 * Others        : 
 * Record
 * 1.Date        : 20160304
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
void report03_devoffline( uint16 dev_type_id, uint8 dev_id, uint8 stat )
{
    appbuf_t * appbuf;
	uint32 idx = 0;

	appbuf = (appbuf_t *)malloc(sizeof(appbuf_t));
	assert(appbuf);
	appbuf->buftyp = APPBUF_RPT;
	appbuf->frm_ctg = FRMCTG_RPT_OFFLINE;
	appbuf->data[idx++] = MSB(dev_type_id);
	appbuf->data[idx++] = LSB(dev_type_id);
	appbuf->data[idx++] = dev_id;
	appbuf->data[idx++] = stat;
	appbuf->len = idx;

	char mode[51] = {0};
    system_getcommod(mode);
	if(strcmp(mode,"WebService") != 0)
	    utm_rpt_send(appbuf);
	free(appbuf);
}

/*****************************************************************************
 * Function      : dev_mgr
 * Description   : 设备管理接口
 * Input         : param_opt_t *opt
 * Output        : None
 * Return        : opt_rst_e
 * Others        : 
 * Record
 * 1.Date        : 20151227
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
opt_rst_e dev_mgr( param_opt_t *opt )
{
    opt_rst_e ret = OPT_ERR_ELSE;
	devmgr_t req = {opt->optt, opt->pin, opt->pout};

	switch(opt->objt)
	{
	    case OBJ_TYPE_IO_DEV:
			ret = dev_io_mgr(&req);
			break;
		case OBJ_TYPE_IO_VAR:
			ret = var_io_mgr(&req);
			break;
		case OBJ_TYPE_485_SER:
			ret = ser_485_mgr(&req);
			break;
		case OBJ_TYPE_485_DEV:
			ret = dev_485_mgr(&req);
			break;
		case OBJ_TYPE_485_VAR:
			ret = var_485_mgr(&req);
			break;
		case OBJ_TYPE_ETH_MNT:
			ret = mnt_eth_mgr(&req);
			break;
		case OBJ_TYPE_ETH_DEV:
			ret = dev_eth_mgr(&req);
			break;
		case OBJ_TYPE_ETH_VAR:
			ret = var_eth_mgr(&req);
			break;
		default:
			break;
	}

	return ret;
}

/*****************************************************************************
 * Function      : dev_read_sd
 * Description   : 读取设备参数
 * Input         : uint16 dev_type_id
                uint8 dev_id
                uint8 *pack
                uint32 *len
 * Output        : None
 * Return        : status_t
 * Others        : 
 * Record
 * 1.Date        : 20151220
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
utm_rst_e dev_read_sd( uint16 dev_type_id, uint8 dev_id, uint8 *pack, uint32 *len )
{
    utm_rst_e ret;

    ret = read_io_sd(dev_type_id, dev_id, pack, len);

	if(UTM_DEV_NOTCFG == ret)
		ret = read_485_sd(dev_type_id, dev_id, pack, len);
	if(UTM_DEV_NOTCFG == ret)
		ret = read_eth_sd(dev_type_id, dev_id, pack, len);

    return ret;
}
/*****************************************************************************
 * Function      : dev_ctl
 * Description   : 监控中心下行控制帧处理,设定相关数据
 * Input         : uint8 *data
 * Output        : None
 * Return        : status_t
 * Others        : 
 * Record
 * 1.Date        : 20151220
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
utm_rst_e dev_ctl( uint8 *data )
{
    utm_rst_e ret;

    ret = ctrl_485(data);
    if(UTM_DEV_NOTCFG == ret)
		ret = ctrl_io(data);
	if(UTM_DEV_NOTCFG == ret)
		ret = ctrl_eth(data);
	
    return ret;
}


/*****************************************************************************
 * Function      : dev_init
 * Description   : 设备初始化
 * Input         : void
 * Output        : None
 * Return        : void
 * Others        :
 * Record
 * 1.Date        : 20151126
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
void dev_init( void )
{
    //加载设备配置信息
    dev_io_init();
	dev_485_init();
	dev_eth_init();
} 
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  
