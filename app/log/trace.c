/********************************************************************************

 **** Copyright (C), 2015, BinHong tech Co., Ltd.                ****

 ********************************************************************************
 * File Name     : trace.c
 * Author        : chenxu
 * Date          : 2015-12-31
 * Version       : 1.0
 * Function List :
 * Description   : 调试跟踪信息
 * Record        :
 * 1.Date        : 2015-12-31
 *   Author      : chenxu
 *   Modification: Created file

 *******************************************************************************/

/*------------------------------------------------------------------------------
Section: Includes
------------------------------------------------------------------------------*/
#include <time.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/prctl.h>
#include <stdarg.h>
#include "trace.h"
#include "atypes.h"
#include <pthread.h>
#include "../web/web.h"
#include "mxml.h"
/*------------------------------------------------------------------------------
Section: Macro Definitions
------------------------------------------------------------------------------*/
#define LOG_FILE_NAME				"/usr/httproot/log/System.log"		/*日志文件名*/
#define LOG_CNT_MAX					1024								/*日志最大容量*/
#define LOG_BUF_LEN_MAX				512									/*日志报文最大长度*/
#define LOG_LOCK					pthread_mutex_lock(&(log.log_mutex))
#define LOG_UNLOCK					pthread_mutex_unlock(&(log.log_mutex))

#define LOG_SAVE
#define TRACE_XML					"/usr/httproot/cfg/Trace.xml"
/*------------------------------------------------------------------------------
Section: Type Definitions
------------------------------------------------------------------------------*/
typedef struct
{
    pthread_mutex_t log_mutex;
    char *log_index_array[LOG_CNT_MAX];
	uint32 log_index;
}log_t;
/*------------------------------------------------------------------------------
Section: Globals
------------------------------------------------------------------------------*/
static uint32 the_trace_mask = 0xFFFFFFFF;
static log_t log;
/*------------------------------------------------------------------------------
Section: Function Prototypes
------------------------------------------------------------------------------*/
/*****************************************************************************
 * Function      : trace_load
 * Description   : 加载trace配置信息
 * Input         : void
 * Output        : None
 * Return        : void
 * Others        : 
 * Record
 * 1.Date        : 20160204
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
static void trace_load( void )
{
    status_t ret = ERROR;
	int fd = -1;
	int whitespace;
	uint32 trace_mask = 0xFFFFFFFF; 
	FILE *fp;
	mxml_node_t	*tree,
		*Node,
    	*node;
	char *attr_value;

	if(NULL == (fp = fopen(TRACE_XML, "r")))
	{
	    trace(TR_SYSTEM,  "fopen %s failed\n", TRACE_XML);
		return;
	}

	tree = mxmlLoadFile(NULL, fp, MXML_NO_CALLBACK);
	fclose(fp);

	Node = mxmlFindElement(tree, tree, (const char *)"Node", NULL, NULL, MXML_DESCEND);
	assert(Node);

	node = mxmlFindElement(Node, Node, (const char *)"UTM", NULL, NULL, MXML_DESCEND);
	assert(node);
	node = mxmlGetLastChild(node);
	assert(node);
	attr_value = mxmlGetText(node, &whitespace);
	assert(attr_value);
	if(attr_value[0] == 'N')
		SETBITS(the_trace_mask, TR_UTM, 0);

	node = mxmlFindElement(Node, Node, (const char *)"UTM_BUF", NULL, NULL, MXML_DESCEND);
	assert(node);
	node = mxmlGetLastChild(node);
	assert(node);
	attr_value = mxmlGetText(node, &whitespace);
	assert(attr_value);
	if(attr_value[0] == 'N')
		SETBITS(the_trace_mask, TR_UTM_BUF, 0);

	node = mxmlFindElement(Node, Node, (const char *)"COM", NULL, NULL, MXML_DESCEND);
	assert(node);
	node = mxmlGetLastChild(node);
	assert(node);
	attr_value = mxmlGetText(node, &whitespace);
	assert(attr_value);
	if(attr_value[0] == 'N')
		SETBITS(the_trace_mask, TR_COMM, 0);

	node = mxmlFindElement(Node, Node, (const char *)"COM_BUF", NULL, NULL, MXML_DESCEND);
	assert(node);
	node = mxmlGetLastChild(node);
	assert(node);
	attr_value = mxmlGetText(node, &whitespace);
	assert(attr_value);
	if(attr_value[0] == 'N')
		SETBITS(the_trace_mask, TR_COMM_BUF, 0);

	node = mxmlFindElement(Node, Node, (const char *)"IO", NULL, NULL, MXML_DESCEND);
	assert(node);
	node = mxmlGetLastChild(node);
	assert(node);
	attr_value = mxmlGetText(node, &whitespace);
	assert(attr_value);
	if(attr_value[0] == 'N')
		SETBITS(the_trace_mask, TR_DEV_IO, 0);

	node = mxmlFindElement(Node, Node, (const char *)"IO_BUF", NULL, NULL, MXML_DESCEND);
	assert(node);
	node = mxmlGetLastChild(node);
	assert(node);
	attr_value = mxmlGetText(node, &whitespace);
	assert(attr_value);
	if(attr_value[0] == 'N')
		SETBITS(the_trace_mask, TR_DEV_IO_BUF, 0);

	node = mxmlFindElement(Node, Node, (const char *)"SER", NULL, NULL, MXML_DESCEND);
	assert(node);
	node = mxmlGetLastChild(node);
	assert(node);
	attr_value = mxmlGetText(node, &whitespace);
	assert(attr_value);
	if(attr_value[0] == 'N')
		SETBITS(the_trace_mask, TR_DEV_SER, 0);

	node = mxmlFindElement(Node, Node, (const char *)"SER_BUF", NULL, NULL, MXML_DESCEND);
	assert(node);
	node = mxmlGetLastChild(node);
	assert(node);
	attr_value = mxmlGetText(node, &whitespace);
	assert(attr_value);
	if(attr_value[0] == 'N')
		SETBITS(the_trace_mask, TR_DEV_SER_BUF, 0);

	node = mxmlFindElement(Node, Node, (const char *)"ETH", NULL, NULL, MXML_DESCEND);
	assert(node);
	node = mxmlGetLastChild(node);
	assert(node);
	attr_value = mxmlGetText(node, &whitespace);
	assert(attr_value);
	if(attr_value[0] == 'N')
		SETBITS(the_trace_mask, TR_DEV_ETH, 0);

	node = mxmlFindElement(Node, Node, (const char *)"ETH_BUF", NULL, NULL, MXML_DESCEND);
	assert(node);
	node = mxmlGetLastChild(node);
	assert(node);
	attr_value = mxmlGetText(node, &whitespace);
	assert(attr_value);
	if(attr_value[0] == 'N')
		SETBITS(the_trace_mask, TR_DEV_ETH, 0);

	node = mxmlFindElement(Node, Node, (const char *)"AID", NULL, NULL, MXML_DESCEND);
	assert(node);
	node = mxmlGetLastChild(node);
	assert(node);
	attr_value = mxmlGetText(node, &whitespace);
	assert(attr_value);
	if(attr_value[0] == 'N')
		SETBITS(the_trace_mask, TR_AID, 0);

	node = mxmlFindElement(Node, Node, (const char *)"WEB", NULL, NULL, MXML_DESCEND);
	assert(node);
	node = mxmlGetLastChild(node);
	assert(node);
	attr_value = mxmlGetText(node, &whitespace);
	assert(attr_value);
	if(attr_value[0] == 'N')
		SETBITS(the_trace_mask, TR_WEB, 0);

	node = mxmlFindElement(Node, Node, (const char *)"SYSTEM", NULL, NULL, MXML_DESCEND);
	assert(node);
	node = mxmlGetLastChild(node);
	assert(node);
	attr_value = mxmlGetText(node, &whitespace);
	assert(attr_value);
	if(attr_value[0] == 'N')
		SETBITS(the_trace_mask, TR_SYSTEM, 0);

	node = mxmlFindElement(Node, Node, (const char *)"ALARM", NULL, NULL, MXML_DESCEND);
	assert(node);
	node = mxmlGetLastChild(node);
	assert(node);
	attr_value = mxmlGetText(node, &whitespace);
	assert(attr_value);
	if(attr_value[0] == 'N')
		SETBITS(the_trace_mask, TR_ALARM, 0);

	mxmlDelete(tree);
}

/*****************************************************************************
 * Function      : trace_save
 * Description   : 保存trace配置
 * Input         : void
 * Output        : None
 * Return        : status_t
 * Others        : 
 * Record
 * 1.Date        : 20160204
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
static status_t trace_save( void )
{
    int ret;
	int          fd = -1;
	FILE	    *fp;
	mxml_node_t	*tree,
		*Node,
		*node;
	char buffer[100] = {0};

	tree = mxmlNewXML("1.0");
	assert(NULL != tree);

	Node = mxmlNewElement(tree, "Node");
	assert(Node);

    node = mxmlNewElement(Node, "UTM");
	assert(node);
	if(BITS(the_trace_mask, TR_UTM))
		buffer[0] = 'Y';
	else
		buffer[0] = 'N';
	node = mxmlNewText(node, 0, buffer);
	assert(node);
	
	node = mxmlNewElement(Node, "UTM_BUF");
	assert(Node);
	if(BITS(the_trace_mask, TR_UTM_BUF))
		buffer[0] = 'Y';
	else
		buffer[0] = 'N';
	node = mxmlNewText(node, 0, buffer);
	assert(node);
	
	node = mxmlNewElement(Node, "COM");
	assert(Node);
	if(BITS(the_trace_mask, TR_COMM))
		buffer[0] = 'Y';
	else
		buffer[0] = 'N';
	node = mxmlNewText(node, 0, buffer);
	assert(node);
	
	node = mxmlNewElement(Node, "COM_BUF");
	assert(Node);
	if(BITS(the_trace_mask, TR_COMM_BUF))
		buffer[0] = 'Y';
	else
		buffer[0] = 'N';
	node = mxmlNewText(node, 0, buffer);
	assert(node);
	
	node = mxmlNewElement(Node, "IO");
	assert(Node);
	if(BITS(the_trace_mask, TR_DEV_IO))
		buffer[0] = 'Y';
	else
		buffer[0] = 'N';
	node = mxmlNewText(node, 0, buffer);
	assert(node);
	
	node = mxmlNewElement(Node, "IO_BUF");
	assert(Node);
	if(BITS(the_trace_mask, TR_DEV_IO_BUF))
		buffer[0] = 'Y';
	else
		buffer[0] = 'N';
	node = mxmlNewText(node, 0, buffer);
	assert(node);
	
	node = mxmlNewElement(Node, "SER");
	assert(Node);
	if(BITS(the_trace_mask, TR_DEV_SER))
		buffer[0] = 'Y';
	else
		buffer[0] = 'N';
	node = mxmlNewText(node, 0, buffer);
	assert(node);
	
	node = mxmlNewElement(Node, "SER_BUF");
	assert(Node);
	if(BITS(the_trace_mask, TR_DEV_SER_BUF))
		buffer[0] = 'Y';
	else
		buffer[0] = 'N';
	node = mxmlNewText(node, 0, buffer);
	assert(node);

	node = mxmlNewElement(Node, "ETH");
	assert(Node);
	if(BITS(the_trace_mask, TR_DEV_ETH))
		buffer[0] = 'Y';
	else
		buffer[0] = 'N';
	node = mxmlNewText(node, 0, buffer);
	assert(node);
	
	node = mxmlNewElement(Node, "ETH_BUF");
	assert(Node);
	if(BITS(the_trace_mask, TR_DEV_ETH_BUF))
		buffer[0] = 'Y';
	else
		buffer[0] = 'N';
	node = mxmlNewText(node, 0, buffer);
	assert(node);

	node = mxmlNewElement(Node, "AID");
	assert(Node);
	if(BITS(the_trace_mask, TR_AID))
		buffer[0] = 'Y';
	else
		buffer[0] = 'N';
	node = mxmlNewText(node, 0, buffer);
	assert(node);

	node = mxmlNewElement(Node, "WEB");
	assert(Node);
	if(BITS(the_trace_mask, TR_WEB))
		buffer[0] = 'Y';
	else
		buffer[0] = 'N';
	node = mxmlNewText(node, 0, buffer);
	assert(node);

    node = mxmlNewElement(Node, "SYSTEM");
	assert(Node);
	if(BITS(the_trace_mask, TR_SYSTEM))
		buffer[0] = 'Y';
	else
		buffer[0] = 'N';
	node = mxmlNewText(node, 0, buffer);
	assert(node);

    node = mxmlNewElement(Node, "ALARM");
	assert(Node);
	if(BITS(the_trace_mask, TR_ALARM))
		buffer[0] = 'Y';
	else
		buffer[0] = 'N';
	node = mxmlNewText(node, 0, buffer);
	assert(node);

	if(NULL == (fp = fopen(TRACE_XML, "w+")))
	{
	    trace(TR_SYSTEM,  "creat file %s failed\n", TRACE_XML);
		mxmlDelete(tree);
		return ERROR;
	}

    ret = mxmlSaveFile(tree, fp, MXML_NO_CALLBACK);
    assert(-1 != ret);
	
    fclose(fp);
    printf("save %s success\n", TRACE_XML);
	mxmlDelete(tree);

	return OK;
}

/*****************************************************************************
 * Function      : log_save
 * Description   : 日志存储
 * Input         : char *buffer
 * Output        : None
 * Return        : void
 * Others        : 
 * Record
 * 1.Date        : 20160113
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
static void log_save( char *buffer )
{
#ifdef LOG_SAVE
    uint32 len = strlen(buffer);
    char *node;
	uint32 idx;
	
	if(len > LOG_BUF_LEN_MAX)
		len = LOG_BUF_LEN_MAX;
	else if(len == 0)
		return;
	
	LOG_LOCK;
//	printf("log_index = %d, len = %d\n", log.log_index, len);
	//日志存内存
	if(NULL == log.log_index_array[log.log_index])
	{
		node = (char*)malloc(len + 1);
		assert(node);
		log.log_index_array[log.log_index] = node;
	}
	else
	{
		node = log.log_index_array[log.log_index];
		if(len > strlen(node))
		{
			node = (char *)realloc(node, (len + 1));			
            assert(node);
			log.log_index_array[log.log_index] = node;
		}
	}
	strncpy(node, buffer, len);
	node[len] = 0;

	FILE * fp;
    char format[5];
	//当写满,清除文件重新写TODO 需要优化为按时间环形存储

	if(0 == log.log_index)
		strcpy(format, "w+");
	else
		strcpy(format, "a");
	
    if(NULL == (fp = fopen(LOG_FILE_NAME, format)))
	{
	    printf("fopen file [%s] err\n", LOG_FILE_NAME);
		LOG_UNLOCK;
		return;
	}
#if 0
	struct timeval tv;
	gettimeofday(&tv, NULL);
    printf("before %1d.%06d\n", tv.tv_sec, tv.tv_usec);
	idx = log.log_index;
	do
	{
		idx++;
		if(idx == LOG_CNT_MAX)
			idx = 0;
		
		if(log.log_index_array[idx])
		    fwrite(log.log_index_array[idx], strlen(log.log_index_array[idx]), 1, fp);
		
	}while(idx != log.log_index);
	gettimeofday(&tv, NULL);
	printf("after  %1d.%06d\n", tv.tv_sec, tv.tv_usec);
#else
    fwrite(log.log_index_array[log.log_index], strlen(log.log_index_array[log.log_index]), 1, fp);
#endif	
	fclose(fp);
	log.log_index++;
	if(log.log_index == LOG_CNT_MAX)
		log.log_index = 0;
	LOG_UNLOCK;
#endif 
}

/*****************************************************************************
 * Function      : trace_head
 * Description   : trace head
 * Input         : char *buffer
 * Output        : None
 * Return        : void
 * Others        : 
 * Record
 * 1.Date        : 20160113
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
static void trace_head( char *buffer )
{
    struct tm daytime;
	char thread_name[20];
    time_t stime = time(NULL);
    (void)localtime_r(&stime, &daytime);
	(void)prctl(PR_GET_NAME, thread_name);
    (void)sprintf(buffer, "[%04d-%02d-%02d %02d:%02d:%02d][%5s]",
            daytime.tm_year + 1900,
            daytime.tm_mon + 1,
            daytime.tm_mday,
            daytime.tm_hour,
            daytime.tm_min,
            daytime.tm_sec,
            thread_name
            );
}

/*****************************************************************************
 * Function      : trace
 * Description   : 日志跟踪接口
 * Input         : uint8 tid
                const char *msg
                ...
 * Output        : None
 * Return        : void
 * Others        : 
 * Record
 * 1.Date        : 20160113
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
void trace( uint8 tid, const char *msg, ... )
{
    va_list ap;
    char buffer[LOG_BUF_LEN_MAX];
	int32 len;

    if (TRCBIT(tid))
    {
        trace_head(buffer);
		len = strlen(buffer);
		if(tid == TR_EVENT) //增加异常标记
		    len += sprintf(&buffer[len], "[EVENT]");
		
        va_start(ap, msg);
        vsnprintf(&buffer[len], LOG_BUF_LEN_MAX - len, msg, ap);
		va_end(ap);
        printf(buffer);
		log_save(buffer);
    }
}

/*****************************************************************************
 * Function      : trace_buf
 * Description   : buf日志跟踪接口
 * Input         : uint8 tid,
				char *format,
	            uint8 *buffer,
	            int32 len
                ...
 * Output        : None
 * Return        : void
 * Others        : 
 * Record
 * 1.Date        : 20160113
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
void trace_buf(uint8 tid,
			char *format,
            uint8 *buffer,
            int32 len)
{
    int32 i;
	int idx = 0;
	char buff[LOG_BUF_LEN_MAX];

    if (TRCBIT(tid))
    {
        trace_head(buff);
		idx = strlen(buff);
	    (void)sprintf(&buff[idx], format);
		idx = strlen(buff);
	    for (i = 0; (i < len) && (idx < LOG_BUF_LEN_MAX - 3); i++, idx += 3)
	    {
	        (void)sprintf(&buff[idx], "%02X ", *(buffer + i));
	    }
	    sprintf(&buff[idx], "\r\n");
		(void)printf(buff);
		log_save(buff);
	}
}

/*****************************************************************************
 * Function      : time_format
 * Description   : 时间格式化
 * Input         : time_t * stp
                char *format
 * Output        : None
 * Return        : void
 * Others        : 
 * Record
 * 1.Date        : 20160128
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
void time2format( time_t * stp, char *format )
{
    time_t tinow;
	struct tm daytime;
	
    if(NULL == stp)
	{
	    tinow = time(NULL);
		stp = &tinow;
	}
	(void)localtime_r(stp, &daytime);
	sprintf(format, "%04d-%02d-%02d %02d:%02d:%02d",
            daytime.tm_year + 1900,
            daytime.tm_mon + 1,
            daytime.tm_mday,
            daytime.tm_hour,
            daytime.tm_min,
            daytime.tm_sec);
}

/*****************************************************************************
 * Function      : printtime
 * Description   : 打印时间
 * Input         : struct timeval *tv
 * Output        : None
 * Return        : void
 * Others        : 
 * Record
 * 1.Date        : 20160129
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
void printtime( struct timeval *tv )
{
    time_t ti;
	struct tm daytime;
	struct timeval tv1;

	if(NULL == tv)
	{
	    gettimeofday(&tv1, NULL);
		tv = &tv1;
	}
	localtime_r(&tv->tv_sec, &daytime);
	printf("%04d-%02d-%02d %02d:%02d:%02d.%d\n",
            daytime.tm_year + 1900,
            daytime.tm_mon + 1,
            daytime.tm_mday,
            daytime.tm_hour,
            daytime.tm_min,
            daytime.tm_sec,
            tv->tv_usec);
}

/*****************************************************************************
 * Function      : trace_mask_update
 * Description   : 更新日志开关信息
 * Input         : uint32 trace_mask
 * Output        : None
 * Return        : void
 * Others        : 
 * Record
 * 1.Date        : 20160113
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
//void trace_mask_update( uint32 trace_mask )
//{
//    if(trace_mask)
//		the_trace_mask = trace_mask;
//}

/*****************************************************************************
 * Function      : trace_mgr
 * Description   : trace 开关控制
 * Input         : param_req_t *req
 * Output        : None
 * Return        : opt_rst_e
 * Others        : 
 * Record
 * 1.Date        : 20160204
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
opt_rst_e trace_mgr( param_opt_t *req )
{
    uint32 * trace_mask;
	opt_rst_e ret = OPT_ERR_ELSE;

	if((req->objt != OBJ_TYPE_TRACE)
		|| (req->pin == NULL))
		return ret;

    trace_mask = req->pin;
	
	LOG_LOCK;
	if(the_trace_mask != *trace_mask)
	{
		the_trace_mask = *trace_mask;
		if(OK == trace_save())
			ret = OPT_OK;
	}
	LOG_UNLOCK;

	return ret;
}

/*****************************************************************************
 * Function      : log_init
 * Description   : void
 * Input         : void
 * Output        : None
 * Return        : void
 * Others        : 
 * Record
 * 1.Date        : 20160113
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
void log_init( void )
{
    uint32 trace_mask;

    memset((void*)&log, 0x00, sizeof(log));
    //创建互斥锁
    if(0 != pthread_mutex_init(&(log.log_mutex), NULL))
	{
		printf("create log_mutex failed\n");
		assert(0);
	}

	trace_load();
}
