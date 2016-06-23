/********************************************************************************

 **** Copyright (C), 2016, BinHong tech Co., Ltd.                ****

 ********************************************************************************
 * File Name     : event.c
 * Author        : chenxu
 * Date          : 2016-01-18
 * Version       : 1.0
 * Function List :
 * Description   : 告警事件API
 * Record        :
 * 1.Date        : 2016-01-18
 *   Author      : chenxu
 *   Modification: Created file

 *******************************************************************************/

/*------------------------------------------------------------------------------
Section: Includes
------------------------------------------------------------------------------*/
#include "alarm.h"
#include <time.h>
#include <sys/time.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>
#include "trace.h"
#include "sqlite3.h"
#include "web.h"
#include "maths.h"
#include "gpio.h"
#include <string.h>
/*------------------------------------------------------------------------------
Section: Macro Definitions
------------------------------------------------------------------------------*/
#define ALARM_ACTION
#ifdef ALARM_ACTION
#define SQLITE3
#endif
#define ALARM_LOCK 	    		pthread_mutex_lock(&alarm_mutex)
#define ALARM_UNLOCK  			pthread_mutex_unlock(&alarm_mutex)
#define ALARM_FILE_NAME			"/usr/httproot/alarm/System.alarm"
#define BUFFER_SIZE				1024
#define ERR_OK			 0
#define ERR_DTP_NEXIST	-1
#define ERR_DEV_NEXIST	-2
#define ERR_VAR_NEXIST	-3


/*------------------------------------------------------------------------------
Section: Type Definitions
------------------------------------------------------------------------------*/
#define F_ALARM_FILE_NAME			"/usr/httproot/alarm/System.alarm"
#define MAX_ALARM_COUNT				1024
#define MAX_ALARM_FREMLEN			128
/*------------------------------------------------------------------------------
Section: Globals
------------------------------------------------------------------------------*/;
static pthread_mutex_t alarm_mutex;
static const char *alarm_type_string[ALARM_STAT_MAX] = 
	{"NULL",
	 "ON   TO   OFF",
	 "OFF  TO   ON",
	 "OVER HIGH LIMIT",
	 "OVER LOW  LIMIT",
	 "RESUME"
	};

/*****************************************************************************
 * Function      : alarm_hook
 * Description   : 告警钩子函数，控制告警灯的亮和灭
 * Input         : boolean flag
 * Output        : None
 * Return        : void
 * Others        : 
 * Record
 * 1.Date        : 20160229
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
void alarm_hook( boolean flag )
{
    static boolean is_first_run = true;

    ALARM_LOCK;
	
	if(is_first_run)
	{
		int fd;
		at91_gpio_arg arg;

		fd = open(DEV_GPIO, O_RDWR);
		assert(fd != -1);

		arg.pin = LED_ALARM;
		arg.data = 0;
		arg.usepullup = 1;
		ioctl(fd, IOCTL_GPIO_SETOUTPUT, &arg);
		
		close(fd);		

		is_first_run = false;	
	}

    at91_gpio_arg arg = {LED_ALARM, 0, 1};
	int fd;
	
	fd = open(DEV_GPIO, O_RDWR);
	if(-1 == fd)
	{
	    //测试时出现打开失败的情况，这里在日志里面记录下
	    trace(TR_EVENT, "function[%s] open gpio failed!!!\n", __FUNCTION__);
		assert(0);
	}
		
	arg.data = flag ? 0 : 1;
	ioctl(fd, IOCTL_GPIO_SETVALUE, &arg);

	close(fd);

	ALARM_UNLOCK;
	
}

/*****************************************************************************
 * Function      : alarm_open
 * Description   : 打开数据库
 * Input         : void
 * Output        : None
 * Return        : void
 * Others        : 
 * Record
 * 1.Date        : 20160202
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
static sqlite3 * alarm_open( void )
{
    uint32 index;
	sqlite3 *db;
	struct tm daytime;
	time_t sec, lsec = 0;
	uint32 ms, lms = 0;
	char buf[30];
	int result;
	char *errmsg = NULL;
	char *sql = "create table if not exists alarmtable( \
		DevName, \
		VarName,\
		OccurTime,\
		WarnType,\
		Message\
		)";

    //打开数据库mdatabase.db，如果不存在就创建它
    result = sqlite3_open("/usr/devmgr/mdatabase.db", &db);  
	if(result != SQLITE_OK)
	{
		trace(TR_ALARM, "Can't open datebase\n%s\n", sqlite3_errmsg(db));
		return NULL;
	}
	result = sqlite3_exec(db, sql,0,0, &errmsg);
	//创建一个测试表，表名alarmtable
	if(result != SQLITE_OK)
		trace(TR_ALARM, "warning:Create table fail! May table \
		alarmtable already result[%d] errmsg[%s].\n", result,  errmsg);

	return db;
}

/*****************************************************************************
 * Function      : alarm_close
 * Description   : 关闭数据库
 * Input         : sqlite3 * db
 * Output        : None
 * Return        : void
 * Others        : 
 * Record
 * 1.Date        : 20160202
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
static void alarm_close( sqlite3 * db )
{
    sqlite3_close(db);
}

/*****************************************************************************
 * Function      : alarm_requestfortime
 * Description   : 按时间筛选告警
 * Input         : void *pin
 * Output        : None
 * Return        : int
 * Others        : 
 * Record
 * 1.Date        : 20160128
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
static int alarm_requestfortime( void *pin )
{
    sqlite3 *db;
    alarm_requestoftime_t *reqtime;
	char sql[200];
	char tbuf1[30] = {0}, tbuf2[30] = {0};
	int result;
	char *errmsg = NULL;
	char filename[100] = {0};
	int nrow = 0, ncol = 0, i,j;
	char **table;
	char buffer[BUFFER_SIZE];
	int len;
	int idx;
	FILE *fp;
	int ret = ERROR;
#ifdef SQLITE3	
	if(NULL == pin)
		return ERROR;
	
	reqtime = (alarm_requestoftime_t *)pin;
	time2format(&reqtime->starttime, tbuf1);
	time2format(&reqtime->endtime, tbuf2);
	trace(TR_ALARM, "start time[%s] end time[%s]\n", tbuf1, tbuf2);
	
	if(reqtime->endtime < reqtime->starttime)
		return ret;
	trace(TR_ALARM, "alarm_requestfortime\n");
	ALARM_LOCK;
    //查询数据库
	if(NULL == (db = alarm_open()))
	{
	    trace(TR_ALARM, "alarm_open failed\n");
		return ret;
	}
	sprintf(sql, "select * from alarmtable where OccurTime between '%s' and '%s'", tbuf1, tbuf2);
	result = sqlite3_exec(db,sql,0,0,&errmsg);
	sqlite3_get_table(db,sql,&table,&nrow,&ncol,&errmsg);

	if(NULL == (fp = fopen(ALARM_FILE_NAME, "w+")))
	{
	    trace(TR_ALARM, "fopen %s failed\n", ALARM_FILE_NAME);
		goto Request_end;
	}
	printf("nrow = %d, ncol = %d\n", nrow, ncol);
	sprintf(buffer, "%s -> %s, number of records = %d\n\n", tbuf1, tbuf2, nrow);
	fwrite(buffer, strlen(buffer), 1, fp);
	for(i = 0; i < nrow+1; i++) 
	{
		for(j = 0; j < ncol; j++) 
		{
			len = MAX(24, strlen(table[i*ncol+j]));
			if((idx + len) > BUFFER_SIZE - 1)
			{
			    fwrite(buffer, idx, 1, fp);
				idx = 0;
			}
		    len = sprintf(&buffer[idx], "% -23s\t", table[i*ncol+j]);
			idx += len;
		}
		len = sprintf(&buffer[idx], "\n");
		idx += len;
	}
	if(idx > 0)
	    fwrite(buffer, idx, 1, fp);
	fclose(fp);
	ret = OK;
Request_end:
	sqlite3_free_table(table);
	alarm_close(db);
	ALARM_UNLOCK;
#endif
	return ret;
}

/*****************************************************************************
 * Function      : alarm_requestfordev
 * Description   : 按时间筛选告警
 * Input         : void *pin
 * Output        : None
 * Return        : int
 * Others        : 
 * Record
 * 1.Date        : 20160128
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
static int alarm_requestfordev( void *pin )
{
    sqlite3 *db;
    char *devname;
	char sql[200];
	char *errmsg = NULL;
	int result;
	int ret = ERROR;
	int len,idx = 0;
    int nrow = 0, ncol = 0, i,j;
	char **table;
	char buffer[BUFFER_SIZE];
	char buf[50];
	FILE *fp;

#ifdef SQLITE3
	if(NULL == pin)
		return ret;
	devname = (char *)pin;

    trace(TR_ALARM, "devname[%s]\n", devname);
	ALARM_LOCK;
	if(NULL == (db = alarm_open()))
	{
	    trace(TR_ALARM, "alarm_open failed\n");
		return ret;
	}
	sprintf(sql, "select * from alarmtable where DevName='%s'", devname);
	
//	printtime(NULL);
	result = sqlite3_exec(db,sql,0,0,&errmsg);
//	printtime(NULL);
	sqlite3_get_table(db,sql,&table,&nrow,&ncol,&errmsg);
//    printtime(NULL);
	
    if(NULL == (fp = fopen(ALARM_FILE_NAME, "w+")))
	{
	    trace(TR_ALARM, "fopen %s failed\n", ALARM_FILE_NAME);
		goto Request_end;
	}
	trace(TR_ALARM, "nrow = %d, ncol = %d\n", nrow, ncol);
	sprintf(buffer, "DevName = %s, number of records = %d\n\n", devname, nrow);
	fwrite(buffer, strlen(buffer), 1, fp);
	for(i = 0; i < nrow+1; i++) 
	{
		for(j = 0; j < ncol; j++) 
		{
			len = MAX(23, strlen(table[i*ncol+j]));
			if((idx + len) > BUFFER_SIZE - 1)
			{
			    fwrite(buffer, idx, 1, fp);
				idx = 0;
			}
		    len = sprintf(&buffer[idx], "% -23s\t", table[i*ncol+j]);
			idx += len;
		}
		len = sprintf(&buffer[idx], "\n");
		idx += len;
	}
	if(idx > 0)
	    fwrite(buffer, idx, 1, fp);
	fclose(fp);
	ret = OK;
Request_end: 
	sqlite3_free_table(table);
	alarm_close(db);
	ALARM_UNLOCK;
#endif
	return ret;
}

static const FUNCPTR alarm_request_fun[] = 
{
    NULL,
    (FUNCPTR)alarm_requestfortime,
	(FUNCPTR)alarm_requestfordev
};
/*****************************************************************************
 * Function      : alarm_request
 * Description   : 告警日志请求
 * Input         : param_req_t *req
 * Output        : None
 * Return        : opt_rst_e
 * Others        : 
 * Record
 * 1.Date        : 20160128
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
opt_rst_e alarm_request( param_opt_t *req )
{
    opt_rst_e ret = OPT_ERR_ELSE;
	
    if(OBJ_TYPE_ALARM != req->objt)
		return ret;

	if(req->optt > ARRAY_SIZE(alarm_request_fun))
		return ret;

    if(NULL == alarm_request_fun[req->optt])
		return ret;
	
	if(OK == alarm_request_fun[req->optt](req->pin))
		ret = OPT_OK;

	return ret;
}

/*****************************************************************************
 * Function      : alarm_write
 * Description   : 记录事件
 * Input         : uint16 dev_type_id
                uint8 dev_id
                uint16 pn
                char *message
 * Output        : None
 * Return        : void
 * Others        :
 * Record
 * 1.Date        : 20160124
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
void alarm_write(char* devname, char *varname, alarm_stat_e type, char *message)
{
#ifdef SQLITE3
#define MAX_MESSAGE_LEN		20
    sqlite3 *db;
	int result;
	char *errmsg = NULL;
	char sql[200];
	char tbuf[30] ={0};
	time_t tinow = time(NULL);
	struct tm daytime;
	struct timeval tv;

	ALARM_LOCK;
	trace(TR_ALARM, "devname[%s] varname[%s] alarm_type[%s] message[%s]\n", 
		devname, varname, alarm_type_string[type], message);
    //插入一条记录
    time2format(NULL, tbuf);
	memset(sql, 0x00, sizeof(sql));
	sprintf(sql, "insert into alarmtable values('%s','%s','%s','%s','%s')", 
		devname, varname, tbuf, alarm_type_string[type], (NULL != message) ? message:"null");
//	printtime(NULL);

    db = alarm_open();
	assert(db);
	result = sqlite3_exec(db,sql,0,0,&errmsg);
	if(result != SQLITE_OK) 
	{
		trace(TR_ALARM, "Can't insert into datebase result[%d] errmsg[%s]\n", result,errmsg);
		trace(TR_ALARM, "%s\n", sql);
		goto Write_Err;
	}
//    printtime(NULL);
Write_Err:
    alarm_close(db);
	ALARM_UNLOCK;
#endif
}

/*****************************************************************************
 * Function      : alarm_init
 * Description   : 事件模块初始化
 * Input         : void
 * Output        : None
 * Return        : void
 * Others        :
 * Record
 * 1.Date        : 20160118
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
void alarm_init( void )
{
#ifdef ALARM_ACTION

	trace(TR_ALARM, "alarm_init\n");
    //创建互斥锁
    if(0 != pthread_mutex_init(&alarm_mutex, NULL))
	{
		trace(TR_ALARM,  "create alarm_mutex failed\n");
		assert(0);
	}
#endif
}

