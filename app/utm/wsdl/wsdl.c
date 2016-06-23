/********************************************************************************

 **** Copyright (C), 2016, BinHong tech Co., Ltd.                ****

 ********************************************************************************
 * File Name     : wsdl.h
 * Author        : chenxu
 * Date          : 2016-01-27
 * Version       : 1.0
 * Function List :
 * Description   : webservice 调用接口
 * Record        :
 * 1.Date        : 2016-01-27
 *   Author      : chenxu
 *   Modification: Created file

 *******************************************************************************/

/*------------------------------------------------------------------------------
Section: Includes
------------------------------------------------------------------------------*/
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "atypes.h"
#include "system.h"
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
static pthread_mutex_t wsdl_mutex;
/*------------------------------------------------------------------------------
Section: Function Prototypes
------------------------------------------------------------------------------*/
/*****************************************************************************
 * Function      : wsdl_reportTHInfo
 * Description   : 以webservice的形式上报温湿度数据
 * Input          : None
 * Output        : None
 * Return        : 
 * Others        : 
 * Record
 * 1.Date        : 20160127
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
void  wsdl_reportTHInfo( uint8 dev_id, uint8 addr, float32 wd, float32 sd )
{
    static boolean is_first_run = true;
    webservice_param_t wsdlp;
	char cmd[100] = {0};
	char preaddr[10] = {0};

    if(is_first_run)
		if(0 != pthread_mutex_init(&wsdl_mutex, NULL))
			assert(0);	

	system_getwsdlparam(&wsdlp);
	memcpy(preaddr, wsdlp.pre_addr, sizeof(wsdlp.pre_addr));
	sprintf(cmd, "/usr/devmgr/reportTHInfotest %s %s%03d%03d %f %f", 
		wsdlp.wsdl_addr, preaddr, addr, dev_id, wd, sd);
	
	pthread_mutex_lock(&wsdl_mutex);
	system(cmd);
	pthread_mutex_unlock(&wsdl_mutex);
}

/*****************************************************************************
 * Function      : wsdl_init
 * Description   : void
 * Input         : void
 * Output        : None
 * Return        : void
 * Others        : 
 * Record
 * 1.Date        : 20160127
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
void wsdl_init( void )
{
	if(0 != pthread_mutex_init(&wsdl_mutex, NULL))
		assert(0);	
}

