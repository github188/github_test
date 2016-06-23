/********************************************************************************

 **** Copyright (C), 2015, BinHong tech Co., Ltd.                ****

 ********************************************************************************
 * File Name     : 485.c
 * Author        : chenxu
 * Date          : 2015-12-23
 * Version       : 1.0
 * Function List :
 * Description   : 485通道相关处理
 * Record        :
 * 1.Date        : 2015-12-23
 *   Author      : chenxu
 *   Modification: Created file

 *******************************************************************************/

/*------------------------------------------------------------------------------
Section: Includes
------------------------------------------------------------------------------*/
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/prctl.h>
#include <string.h>
#include "list.h"
#include "trace.h"
#include "atypes.h"
#include "dev.h"
#include "./private/hth.h"
#include <fcntl.h>
#include <pthread.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>  
#include <termios.h>
#include <errno.h> 
#include "maths.h"
#include "gpio.h"
#include "protocol.h"
#include "utm.h"
//#include "modbus_lib.h"
#include "serial_lib.h"
#include "mb_wk2166_lib.h"
#include "modbus_lib.h"
//#include "gpio_common.h"
#include "web.h"
#include "mxml.h"
#include "wsdl.h"
#include "system.h"
#include "alarm.h"
#include "share.h"
#include "ptl.h"
#include "485.h"
/*------------------------------------------------------------------------------
Section: Macro Definitions
------------------------------------------------------------------------------*/
#define DEV_485_LOCK(comn)		pthread_mutex_lock(&(dev_485[comn - 1].serial_mutex))
#define DEV_485_UNLOCK(comn)	pthread_mutex_unlock(&(dev_485[comn - 1].serial_mutex))
#define IS_WK2166_COMN(comn)	(comn >= 1 && comn <= 8) ? true : false 
#define E_MODBUS_SER_ERR ((~(E_MODBUS_ILLEGAL_DATA_ADDRESS | \
							   E_MODBUS_ILLEGAL_DATA_VALUE | \
									     E_MODBUS_RECV_CRC | \
								       E_MODBUS_RECV_NOACK | \
								   E_MODBUS_RECV_FRAME_LEN)) & 0xFFFFFF)
#define WSDL_PROTOCOL
#define SERIAL_ALARM
#define SERIAL_COUNT			10

/*------------------------------------------------------------------------------
Section: Type Definitions
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
Section: Globals
------------------------------------------------------------------------------*/
pthread_t tid_485;
mount_serial_t dev_485[SERIAL_COUNT];
static int comn_array[SERIAL_COUNT] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
static int serial_modify_array[SERIAL_COUNT + 1] = {0};//子串口参数修改请求标志
//static int speed_arr[] = {  B115200, B57600, B38400, B19200, B14400, B9600, B4800, B2400, B1200};
//static int name_arr[] = {115200, 57600, 38400,  19200,  14400, 9600,  4800,  2400, 1200};
static int usart_fd[2];
pthread_t tid_485_array[SERIAL_COUNT];

/*------------------------------------------------------------------------------
Section: Function Prototypes
------------------------------------------------------------------------------*/
static void set_speed(int fd, int speed)
{
    int   i;
    int   status;
    struct termios   Opt;
    tcgetattr(fd, &Opt);

    tcflush(fd, TCIOFLUSH);
    cfsetispeed(&Opt, speed);
    cfsetospeed(&Opt, speed);
    status = tcsetattr(fd, TCSANOW, &Opt);
    if (status != 0)
	    trace(TR_DEV_SER, "function[%s] line[%d] failed\n", __FUNCTION__, __LINE__);
}

static int set_Parity(int fd, int databits, int stopbits, int parity)
{
    struct termios options;
    if  ( tcgetattr( fd,&options)  !=  0)
    {
  	perror("SetupSerial 1");
  	return ERROR;
    }
    options.c_cflag &= ~CSIZE;
    switch (databits)
    {
    case 7:
	options.c_cflag |= CS7;
	break;
    case 8:
	options.c_cflag |= CS8;
	break;
    default:
	fprintf(stderr,"Unsupported data size\n");
	return ERROR;
    }
    switch (parity)
    {
    case 'n':
    case 'N':
	options.c_cflag &= ~PARENB;   
	options.c_iflag &= ~INPCK;   
	break;
    case 'o':
    case 'O':
	options.c_cflag |= (PARODD | PARENB); 
	options.c_iflag |= INPCK;           
	break;
    case 'e':
    case 'E':
	options.c_cflag |= PARENB;     
	options.c_cflag &= ~PARODD;
	options.c_iflag |= INPCK;     
	break;
    case 'S':
    case 's':  
	options.c_cflag &= ~PARENB;
	options.c_cflag &= ~CSTOPB;
	break;
    default:
	fprintf(stderr,"Unsupported parity\n");
	return ERROR;
    }
    switch (stopbits)
    {
    case 1:
	options.c_cflag &= ~CSTOPB;
	break;
    case 2:
	options.c_cflag |= CSTOPB;
	break;
    default:
	fprintf(stderr,"Unsupported stop bits\n");
	return ERROR;
    }


	options.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL|IXON);
	options.c_oflag &= ~OPOST;
	options.c_lflag &= ~(ECHO|ECHONL|ICANON|ISIG|IEXTEN);

    /* Set input parity option */
    
    if (parity != 'n')
	options.c_iflag |= INPCK;
    options.c_cc[VTIME] = 150; // 15 seconds
    options.c_cc[VMIN] = 0;
    
    
		tcflush(fd,TCIFLUSH); /* Update the options and do it NOW */
    if (tcsetattr(fd,TCSANOW,&options) != 0)
    {
	perror("SetupSerial 3");
	return ERROR;
    }
    return OK;
}

/*****************************************************************************
 * Function      : usarts_open
 * Description   : 打开本地CH9 CH10
 * Input         : void
 * Output        : None
 * Return        : void
 * Others        : 
 * Record
 * 1.Date        : 20160224
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
static void usarts_open( void )
{
	usart_fd[0] = open("/dev/ttyAT1", O_RDWR);
	if(usart_fd[0] < 0)
	{
	    trace(TR_DEV_SER, "CH9 open failed\n");
		assert(0);
	}

	usart_fd[1] = open("/dev/ttyAT2", O_RDWR);
	if(usart_fd[1] < 0)
	{
	    trace(TR_DEV_SER, "CH10 open failed\n");
		assert(0);
	}
}

/*****************************************************************************
 * Function      : serial_init
 * Description   : 串口初始化
 * Input         : serial_t
 * Output        : None
 * Return        : status_t
 * Others        : 
 * Record
 * 1.Date        : 20160224
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
static status_t serial_init(const serial_t *ser)
{
    int ret;
    if((ser->comn > SERIAL_COUNT) || (ser->comn < 1))
		return ERROR;

    trace(TR_DEV_SER, "comn[%d] par[%d] sbit[%d] baud[%d]\n", 
		ser->comn, ser->p.par, ser->p.sbit, baud[ser->p.baud]);
    ret = mb_init_port(ser->comn);
	assert(ret == 0);
	ret = mb_config_port(ser->comn, ser->p.par, ser->p.sbit);
	assert(ret == 0);
	ret = mb_set_port_baud(ser->comn, baud[ser->p.baud]);
	assert(ret == 0);

	wk2166_read_reg_all(ser->comn);
	return OK;
//	if(IS_WK2166_COMN(ser->comn))
//	{
//	    wk2xxx_init_subport(ser->comn);
//		wk2xxx_config_subport(ser->comn, ser->p.par, ser->p.sbit - 1);	
//		wk2xxx_set_subport_baud(ser->comn, baud[ser->p.baud]);	
//	}
//	else
//	{
//	    int fd = (ser->comn == 9) ? usart_fd[0] : usart_fd[1];
//		char parity = (ser->p.par == 1) ? 'O' : ((ser->p.par == 2) ? 'E' : 'N');
//		
//	    set_speed(fd, baud[ser->p.baud]); //设置串口波特率		
//	    set_Parity(fd, ser->p.dbit + 5, ser->p.sbit, parity); //设置8位数据位，1位停止位，无校验等其他设置。
//	}
}

/*****************************************************************************
 * Function      : serial_request_start
 * Description   : 外部模块提出串口访问请求
 * Input         : const uint8 comn
 * Output        : None
 * Return        : void
 * Others        : 
 * Record
 * 1.Date        : 20160203
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
static void serial_request_start( const uint8 comn )
{
    serial_modify_array[comn] = 1;
	DEV_485_LOCK(comn);
}

/*****************************************************************************
 * Function      : serial_request_end
 * Description   : 串口访问结束
 * Input         : const uint8 comn
 * Output        : None
 * Return        : void
 * Others        : 
 * Record
 * 1.Date        : 20160203
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
static void serial_request_end( const uint8 comn )
{
    DEV_485_UNLOCK(comn);
    serial_modify_array[comn] = 0;
}

/*****************************************************************************
 * Function      : dev_485_list_find
 * Description   : void
 * Input         : dev_485_list_t *plist
                uint16 dev_type_id
                uint8 dev_id
 * Output        : None
 * Return        : dev_485_t *
 * Others        : 
 * Record
 * 1.Date        : 20151128
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
static dev_485_t * dev_485_list_find( uint8 comn, uint16 dev_type_id, uint8 dev_id )
{
    dev_485_t *ptmp = dev_485[comn - 1].dev_485_list.head;

	while(ptmp != NULL)
	{
	    if((ptmp->dev_p.dev_type_id == dev_type_id)
			&& (ptmp->dev_p.dev_id == dev_id))
			break;
		ptmp = ptmp->next;
	}
	return ptmp;
}

/*****************************************************************************
 * Function      : var_485_list_find
 * Description   : 485设备变量查找
 * Input          : uint16 pn 
 				var_485_list_t *vlist
 * Output        : None
 * Return        : var_485_t *
 * Others        : 
 * Record
 * 1.Date        : 20151227
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
static var_485_t * var_485_list_find( uint16 pn, var_485_list_t *vlist )
{
    var_485_t * vnode = NULL;

    if(NULL == vlist)
		return vnode;
	
	vnode = vlist->head;
    while(NULL != vnode)
	{
	    if(pn == vnode->var_p.pn)
			break;
		vnode = vnode->next;
	}

	return vnode;
}

/*****************************************************************************
 * Function      : serial_alarm_save
 * Description   : 存储端口下变量告警状态
 * Input         : uint8 comn
 * Output        : None
 * Return        : void
 * Others        : 
 * Record
 * 1.Date        : 20160127
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
static void serial_alarm_save( uint8 comn )
{
    FILE *fp;
    char filename[100] = {0};
    dev_485_t *pnode;
	var_485_t *vnode;
	alarm_stat_t valarm;

    if((comn > SERIAL_COUNT) || (comn < 1) || (dev_485[comn - 1].serial.comn == 0))
		return;

	//TODO 待优化成单个变量写
	sprintf(filename, "%salarm%x", ALARM_FILE_HEADER, comn);
    if(NULL == (fp = fopen(filename, "wb")))
	{
	    trace(TR_DEV_SER, "fopen file [%s] err\n", filename);
		return;
	}
	pnode = dev_485[comn - 1].dev_485_list.head;
	while(NULL != pnode)
	{
	    vnode = pnode->varlist.head;
		while(NULL != vnode)
		{
		    valarm.dev_type_id = pnode->dev_p.dev_type_id;
			valarm.dev_id = pnode->dev_p.dev_id;
			valarm.pn = vnode->var_p.pn;
            valarm.alarm_stat = vnode->alarm_stat;
			fwrite((char*)&valarm, sizeof(valarm), 1, fp);
			vnode = vnode->next;
		}
		pnode = pnode->next;
	}
	fclose(fp);

}

/*****************************************************************************
 * Function      : serial_alarm_load
 * Description   : 加载通道告警状态信息
 * Input         : uint8 comn
 * Output        : None
 * Return        : static void
 * Others        : 
 * Record
 * 1.Date        : 20160128
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
static void serial_alarm_load( uint8 comn )
{
    FILE *fp;
    char filename[100] = {0};
	dev_485_t *pnode;
	var_485_t *vnode;
	alarm_stat_t valarm;

    printf("serial_alarm_load comn = %d\n", comn);
	if((comn > 8) || (comn < 1) || (dev_485[comn - 1].serial.comn == 0))
		return;

	sprintf(filename, "%salarm%x", ALARM_FILE_HEADER, comn);
    if(NULL == (fp = fopen(filename, "r")))
	{
	    trace(TR_DEV_SER, "fopen file [%s] err\n", filename);
		return;
	}
	while(0 != fread((char*)&valarm, sizeof(valarm), 1, fp))
	{
	    pnode = dev_485_list_find(comn, valarm.dev_type_id, valarm.dev_id);
		if(NULL != pnode)
		{
		    vnode = var_485_list_find(valarm.pn, &pnode->varlist);
			if(NULL != vnode)
				vnode->alarm_stat = valarm.alarm_stat;
		}
	}
	fclose(fp);
}

/*****************************************************************************
 * Function      : serial_alarm
 * Description   : 串口设备越限告警
 * Input         : dev_485_t *pnode
                var_485_t *vnode
                float32 cvalue
 * Output        : None
 * Return        : void
 * Others        : 
 * Record
 * 1.Date        : 20160127
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
static void serial_alarm( uint8 comn, dev_485_t *pnode, var_485_t *vnode, float32 cvalue )
{
#ifdef SERIAL_ALARM
	//告警处理
    if((vnode->var_p.alarm_enable) && (vnode->var_p.wt == ALARM_TYPE_OLIMIT)
		&& (vnode->var_p.hvalue > vnode->var_p.lvalue))
	{
	    boolean flag = false;
		char message[100] = {0};
	    //越上限
	    if((vnode->alarm_stat != ALARM_STAT_OHLIMIT)
			&& (cvalue > vnode->var_p.hvalue))
    	{
    	    snprintf(message, 100, "before %.3f, after %.3f",vnode->d.lv, cvalue);
			vnode->alarm_stat = ALARM_STAT_OHLIMIT;
			flag = true;
    	}
	    //越下限
	    else if((vnode->alarm_stat != ALARM_STAT_OLLIMIT)
			&& (cvalue < vnode->var_p.lvalue))
    	{
    	    snprintf(message, 100, "before %.3f, after %.3f",vnode->d.lv, cvalue);
			vnode->alarm_stat = ALARM_STAT_OLLIMIT;
			flag = true;
    	}
	    //恢复
	    else if((vnode->alarm_stat != ALARM_STAT_RESUME)
			&& (cvalue > vnode->var_p.lvalue) && (cvalue < vnode->var_p.hvalue))
    	{
    	    snprintf(message, 100, "before %.3f, after %.3f", vnode->d.lv, cvalue);
			flag = true;
			vnode->alarm_stat = ALARM_STAT_RESUME;
    	}

		if(flag)
		{
			alarm_write(pnode->dev_p.dev_name,
				        vnode->var_p.pname,
				        vnode->alarm_stat,
				        message);
			//存变量告警状态
			serial_alarm_save(comn);
		}
	}
#endif
}

/*****************************************************************************
 * Function      : load_dev
 * Description   : IO设备初始化
 * Input         : mount_type_e mt
 * Output        : None
 * Return        : void
 * Others        :
 * Record
 * 1.Date        : 20151126
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
static void dev_485_load(void)
{
    uint32 idx;
    status_t ret = ERROR;
	char filename[100];
	int			i;
	int          fd = -1;
	FILE	    *fp;
	mxml_node_t	*tree,
		*SerialDev,
		*MountPoint,
    	*Dev,
		*Param,
    	*node;
	char *attr_value;
	int  whitespace;
	dev_485_t *pnode, *temp;
	var_485_t *vnode, *vtemp;
	
	for(idx = 0; idx < SERIAL_COUNT; idx++)
	{
	    LIST_INIT(&dev_485[idx].dev_485_list);
		dev_485[idx].serial.comn = 0;
	}	


    sprintf(filename, "%s%s", CFG_PATH, SerialDev_xml);
	if(NULL == (fp = fopen(filename, "r")))
	{
	    trace(TR_DEV_SER,  "fopen %s failed\n", filename);
		return;
	}

	tree = mxmlLoadFile(NULL, fp, MXML_NO_CALLBACK);
	fclose(fp);

	SerialDev = mxmlFindElement(tree, tree, "SerialDev", NULL, NULL, MXML_DESCEND);
	assert(SerialDev != NULL);

	MountPoint = mxmlFindElement(SerialDev, SerialDev, "MountPoint", NULL, NULL, MXML_DESCEND);
	while(NULL != MountPoint)
	{
	    uint8 comn = 0;
	    attr_value = mxmlElementGetAttr(MountPoint, "Name");
		assert(attr_value);
		comn = atoi(&attr_value[2]);
		trace(TR_DEV_SER,  "comn = %d\n", comn);

		if((comn < 1) || (comn > SERIAL_COUNT))
			goto NextMountPoint;
		dev_485[comn - 1].serial.comn = comn;

		attr_value = mxmlElementGetAttr(MountPoint, "Baudrate");
		assert(NULL != attr_value);
		dev_485[comn - 1].serial.p.baud = get_baud_sn(atoi(attr_value));
		trace(TR_DEV_SER,  "baud = %d\n", dev_485[comn - 1].serial.p.baud);
		
        attr_value = mxmlElementGetAttr(MountPoint, "DataBits");
		assert(NULL != attr_value);
		dev_485[comn - 1].serial.p.dbit = atoi(attr_value) - 5;
		trace(TR_DEV_SER,  "dbit = %d\n", dev_485[comn - 1].serial.p.dbit);
		
		attr_value = mxmlElementGetAttr(MountPoint, "CheckBits");
		assert(NULL != attr_value);
		dev_485[comn - 1].serial.p.par = get_par_sn(attr_value);
		trace(TR_DEV_SER,  "par = %d\n", dev_485[comn - 1].serial.p.par);

		attr_value = mxmlElementGetAttr(MountPoint, "StopBits");
		assert(NULL != attr_value);
		dev_485[comn - 1].serial.p.sbit = atoi(attr_value);
		trace(TR_DEV_SER,  "sbit = %d\n", dev_485[comn - 1].serial.p.sbit);

		//初始化子串口
		serial_init((const serial_t *)&dev_485[comn - 1].serial);

		Dev = mxmlFindElement(MountPoint, MountPoint, "Dev", NULL, NULL, MXML_DESCEND);
		while(NULL != Dev)
		{
		    pnode = (dev_485_t *)calloc(1, sizeof(dev_485_t));
			assert(NULL != pnode);
			LIST_INIT(&pnode->varlist);

			attr_value = mxmlElementGetAttr(Dev, "DevName");
			assert(NULL != attr_value);
			strcpy(pnode->dev_p.dev_name, attr_value);
			trace(TR_DEV_SER,  "dev_name= %s\n", pnode->dev_p.dev_name);

			attr_value = mxmlElementGetAttr(Dev, "DevType");
			assert(NULL != attr_value);
			pnode->dev_p.dev_type_id = atoi(attr_value);
			trace(TR_DEV_SER,  "dev_type_id = %d\n", pnode->dev_p.dev_type_id);

			attr_value = mxmlElementGetAttr(Dev, "Protocal");
			assert(attr_value);
			pnode->dev_p.protocol_type = get_ptltype_sn(attr_value);
			trace(TR_DEV_SER,  "protocol_type = %s[%d]\n", attr_value, pnode->dev_p.protocol_type);

			attr_value = mxmlElementGetAttr(Dev, "DevID");
			assert(NULL != attr_value);
			pnode->dev_p.dev_id = atoi(attr_value);
			trace(TR_DEV_SER,  "dev_id = %d\n", pnode->dev_p.dev_id);

			attr_value = mxmlElementGetAttr(Dev, "DevAddr");
			assert(NULL != attr_value);
			pnode->dev_p.addr = atoi(attr_value);
			trace(TR_DEV_SER,  "addr = %d\n", pnode->dev_p.addr);

			attr_value = mxmlElementGetAttr(Dev, "NoAck");
			assert(NULL != attr_value);
			pnode->dev_p.dumb_num = atoi(attr_value);
			trace(TR_DEV_SER,  "dumb_num = %d\n", pnode->dev_p.dumb_num);

			//param
			Param = mxmlFindElement(Dev, Dev, "Param", NULL, NULL, MXML_DESCEND);
			while(NULL != Param)
			{
			    vnode = (var_485_t*)calloc(1, sizeof(var_485_t));
				assert(NULL != vnode);

				node = mxmlFindElement(Param, Param, "Name", NULL, NULL, MXML_DESCEND);
				assert(NULL != node);
				node = mxmlGetLastChild(node);
				assert(NULL != node);
				attr_value = mxmlGetText(node, &whitespace);
				assert(attr_value);
		        strcpy(vnode->var_p.pname, attr_value);
				trace(TR_DEV_SER,  "pname = %s\n", vnode->var_p.pname);

				node = mxmlFindElement(Param, Param, "ID", NULL, NULL, MXML_DESCEND);
				assert(NULL != node);
				node = mxmlGetLastChild(node);
				assert(NULL != node);
				attr_value = mxmlGetText(node, &whitespace);
				assert(attr_value);
		        vnode->var_p.pn = atoi(attr_value);
				trace(TR_DEV_SER,  "pn = %d\n", vnode->var_p.pn);

				node = mxmlFindElement(Param, Param, "DataType", NULL, NULL, MXML_DESCEND);
				assert(NULL != node);
				node = mxmlGetLastChild(node);
				assert(NULL != node);
				attr_value = mxmlGetText(node, &whitespace);
				assert(attr_value);
		        vnode->var_p.vt = get_value_type(attr_value);
				trace(TR_DEV_SER,  "vt = %d\n", vnode->var_p.vt);

				node = mxmlFindElement(Param, Param, "RWType", NULL, NULL, MXML_DESCEND);
				assert(NULL != node);
				node = mxmlGetLastChild(node);
				assert(NULL != node);
				attr_value = mxmlGetText(node, &whitespace);
				assert(attr_value);
		        vnode->var_p.rw = get_access(attr_value);
				trace(TR_DEV_SER,  "rw = %d\n", vnode->var_p.rw);

				node = mxmlFindElement(Param, Param, "RegAddr", NULL, NULL, MXML_DESCEND);
				assert(NULL != node);
				node = mxmlGetLastChild(node);
				assert(NULL != node);
				attr_value = mxmlGetText(node, &whitespace);
				assert(attr_value);
		        vnode->var_p.reg_addr = atoi(attr_value);
				trace(TR_DEV_SER,  "reg_addr = %d\n", vnode->var_p.reg_addr);

				node = mxmlFindElement(Param, Param, "DataPrecision", NULL, NULL, MXML_DESCEND);
				assert(NULL != node);
				node = mxmlGetLastChild(node);
				assert(NULL != node);
				attr_value = mxmlGetText(node, &whitespace);
				assert(attr_value);
		        vnode->var_p.dataprecision = atof(attr_value);
				trace(TR_DEV_SER,  "dataprecision = %f\n", vnode->var_p.dataprecision);

				node = mxmlFindElement(Param, Param, "AlarmEnable", NULL, NULL, MXML_DESCEND);
				assert(NULL != node);
				node = mxmlGetLastChild(node);
				assert(NULL != node);
				attr_value = mxmlGetText(node, &whitespace);
				assert(attr_value);
		        vnode->var_p.alarm_enable = (strstr(attr_value, "True") != NULL) ? 1 : 0;
				trace(TR_DEV_SER,  "alarm_enable = %d\n", vnode->var_p.alarm_enable);

				vnode->var_p.wt = ALARM_TYPE_OLIMIT;

				node = mxmlFindElement(Param, Param, "AlarmUpper", NULL, NULL, MXML_DESCEND);
				assert(NULL != node);
				node = mxmlGetLastChild(node);
				assert(NULL != node);
				attr_value = mxmlGetText(node, &whitespace);
				assert(attr_value);
		        vnode->var_p.hvalue = atof(attr_value);
				trace(TR_DEV_SER,  "hvalue = %f\n", vnode->var_p.hvalue);

				node = mxmlFindElement(Param, Param, "AlarmFloor", NULL, NULL, MXML_DESCEND);
				assert(NULL != node);
				node = mxmlGetLastChild(node);
				assert(NULL != node);
				attr_value = mxmlGetText(node, &whitespace);
				assert(attr_value);
		        vnode->var_p.lvalue = atof(attr_value);
				trace(TR_DEV_SER,  "lvalue = %f\n", vnode->var_p.lvalue);

				node = mxmlFindElement(Param, Param, "SampleFrequency", NULL, NULL, MXML_DESCEND);
				assert(NULL != node);
				node = mxmlGetLastChild(node);
				assert(NULL != node);
				attr_value = mxmlGetText(node, &whitespace);
				assert(attr_value);
		        vnode->var_p.period = atoi(attr_value);
				vnode->d.lv = MAGIC_VALUE;
				trace(TR_DEV_SER,  "period = %d\n", vnode->var_p.period);

				VAR_LIST_INSERT((&pnode->varlist), vnode, vtemp, reg_addr);

				Param = mxmlGetNextSibling(Param);	
			}

			DEV_LIST_INSERT((&dev_485[comn - 1].dev_485_list), pnode, temp);
			
			Dev = mxmlGetNextSibling(Dev);
		}
        
NextMountPoint:
		MountPoint = mxmlGetNextSibling(MountPoint);	
	}

	mxmlDelete(tree);
    return;
}

/*****************************************************************************
 * Function      : dev_485_save
 * Description   : 485设备参数存储
 * Input         : void
 * Output        : None
 * Return        : status_t
 * Others        : 
 * Record
 * 1.Date        : 20160105
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
static status_t dev_485_save( void )
{
    status_t ret = ERROR;
	char filename[100];
	char filename1[100];
	int			i;
	int          fd = -1;
	FILE	    *fp;
	mxml_node_t	*tree,
		*SerialDev,
		*MountPoint,
    	*Dev,
		*Param,
    	*node;
	char buffer[100];

	tree = mxmlNewXML("1.0");
	assert(NULL != tree);

	SerialDev = mxmlNewElement(tree, "SerialDev");
	assert(SerialDev);

    uint8 idx;
	serial_t *ser;
	for(idx = 0; idx < SERIAL_COUNT; idx++)
	{
	    ser = &dev_485[idx].serial;
	    if(ser->comn == 0)
			continue;
//		trace(TR_DEV_SER,  "idx = %d, comn = %d\n", idx, ser->comn);
		MountPoint = mxmlNewElement(SerialDev, "MountPoint");
		assert(NULL != MountPoint);
		sprintf(buffer, "CH%d", ser->comn);
		mxmlElementSetAttr(MountPoint, "Name", buffer);
		sprintf(buffer, "0x%02x", 0);
		mxmlElementSetAttr(MountPoint, "BusID", buffer);
		sprintf(buffer, "%d", baud[ser->p.baud]);
		mxmlElementSetAttr(MountPoint, "Baudrate", buffer);
		sprintf(buffer, "%d", ser->p.dbit + 5);
		mxmlElementSetAttr(MountPoint, "DataBits", buffer);
		sprintf(buffer, "%s", par[ser->p.par]);
		mxmlElementSetAttr(MountPoint, "CheckBits", buffer);
		sprintf(buffer, "%d", ser->p.sbit);
		mxmlElementSetAttr(MountPoint, "StopBits", buffer);
		
		dev_485_t *pnode = dev_485[idx].dev_485_list.head;
		while(NULL != pnode)
		{
		    Dev = mxmlNewElement(MountPoint, "Dev");
			assert(Dev);
			mxmlElementSetAttr(Dev, "DevName", pnode->dev_p.dev_name);
			sprintf(buffer, "%d", pnode->dev_p.dev_type_id);
			mxmlElementSetAttr(Dev, "DevType", buffer);
			mxmlElementSetAttr(Dev, "Protocal", "Modbus");//TODO协议类型暂时写死
			sprintf(buffer, "%d", pnode->dev_p.dev_id);
			mxmlElementSetAttr(Dev, "DevID", buffer);
			sprintf(buffer, "%d", pnode->dev_p.addr);
			mxmlElementSetAttr(Dev, "DevAddr", buffer);
			sprintf(buffer, "%d", pnode->dev_p.dumb_num);
			mxmlElementSetAttr(Dev, "NoAck", buffer);

			var_485_t *vnode = pnode->varlist.head;
			while(NULL != vnode)
			{
			    Param = mxmlNewElement(Dev, "Param");
				assert(Param);

				node = mxmlNewElement(Param, "Name");
				assert(node);
				node = mxmlNewText(node, 0, vnode->var_p.pname);

				node = mxmlNewElement(Param, "ID");
				assert(node);
				node = mxmlNewInteger(node, vnode->var_p.pn);

				node = mxmlNewElement(Param, "DataType");
				assert(node);
				node = mxmlNewText(node, 0, value_type[vnode->var_p.vt]);

				node = mxmlNewElement(Param, "RWType");
				assert(node);
				node = mxmlNewText(node, 0, access_type[vnode->var_p.rw]);

                node = mxmlNewElement(Param, "RegAddr");
				assert(node);
				node = mxmlNewInteger(node, vnode->var_p.reg_addr);

				node = mxmlNewElement(Param, "DataPrecision");
				assert(node);
				node = mxmlNewReal(node, vnode->var_p.dataprecision);

				node = mxmlNewElement(Param, "AlarmEnable");
				assert(node);
				node = mxmlNewText(node, 0, (vnode->var_p.alarm_enable) ? "True":"False");

                node = mxmlNewElement(Param, "AlarmUpper");
				assert(node);
				node = mxmlNewReal(node, vnode->var_p.hvalue);

				node = mxmlNewElement(Param, "AlarmFloor");
				assert(node);
				node = mxmlNewReal(node, vnode->var_p.lvalue);

				node = mxmlNewElement(Param, "SampleFrequency");
				assert(node);
				node = mxmlNewInteger(node, vnode->var_p.period);

				vnode = vnode->next;
			}
			
			pnode = pnode->next;
		}	
	}

	sprintf(filename, "%s%s", CFG_PATH, SerialDev_xml);
	if(NULL == (fp = fopen(filename, "w+")))
	{
	    trace(TR_DEV_SER,  "creat file %s failed\n", filename);
		mxmlDelete(tree);
		return ERROR;
	}

    ret = mxmlSaveFile(tree, fp, MXML_NO_CALLBACK);
    assert(-1 != ret);
	
    fclose(fp);
    trace(TR_DEV_SER,  "save %s success\n", filename);
	mxmlDelete(tree);
	
	return OK;
}


/*****************************************************************************
 * Function      : mb_single_var_read
 * Description   : 单个寄存器读
 * Input         : dev_485_t *
                var_t *vnode
 * Output        : None
 * Return        : void
 * Others        : 
 * Record
 * 1.Date        : 20151217
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
static void mb_single_var_read(const uint8 comn, dev_485_t * pnode, var_485_t *vnode )
{
    uint8 buffer[MB_SER_PDU_SIZE_MAX];
    struct mb_read_param mbp = {MB_CH_SERIAL, comn, 
				(uint8_t)pnode->dev_p.addr, vnode->var_p.reg_addr, 1, buffer, 0};
	struct timeval tv;
	int ret;

    if(NULL == vnode)
		return;

    gettimeofday(&tv, NULL);
	trace(TR_DEV_SER, "single_read comn[%d] DevType[%d] DevId[%d] addr[%d] pn[%d] regaddr[%d]\n", 
		comn, pnode->dev_p.dev_type_id, pnode->dev_p.dev_id, pnode->dev_p.addr, vnode->var_p.pn, 
		vnode->var_p.reg_addr);
	if(0 == (ret = mb_read(&mbp)))
	{
	    trace_buf(TR_DEV_SER_BUF, "single_recv :", mbp.recv_buf, mbp.recv_len);
		if(mbp.recv_len == 5)
		{
		    appbuf_t *appbuf = (appbuf_t *)malloc(sizeof(appbuf_t));
			uint16 idx = 0;

			assert((NULL != appbuf));
			appbuf->buftyp = APPBUF_RPT;
			appbuf->frm_ctg = FRMCTG_RPT;
			//设备类型
			appbuf->data[idx++] = pnode->dev_p.dev_type_id / 256;
			appbuf->data[idx++] = pnode->dev_p.dev_type_id % 256;
			//先不写长度
			idx += 2;
			//设备号
			appbuf->data[idx++] = pnode->dev_p.dev_id;

            //获取寄存器真值
            modbus_chan_t modchan = {mbp.mb_ch_type, mbp.port, mbp.slave_addr};
            float32 value = hth_reg_to_value(pnode->dev_p.dev_type_id, 
            	vnode->var_p.pn, &mbp.recv_buf[3], vnode->var_p.dataprecision, &modchan);
			var_cfg_t vcfg = {vnode->var_p.pn, vnode->var_p.vt, vnode->var_p.dataprecision};
			uint32 len;

			if(OK == pack_var_value(&appbuf->data[idx], value, vcfg, &len))
			{
				idx += len;
				appbuf->len = idx;
				appbuf->data[2] = (idx - 5) >> 8;
				appbuf->data[3] = (idx - 5) % 256;

				char mode[51] = {0};
                system_getcommod(mode);
				if(strcmp(mode,"WebService") != 0)
				    utm_rpt_send(appbuf);
			}
			free(appbuf);

            //告警处理
            serial_alarm(comn, pnode, vnode, value);
			//更新数据
			vnode->d.lv = value;
			//更新抄读状态
			pnode->dev_stat.lst_rd_time = tv;
			if(pnode->dev_stat.is_online == false)
			{
			   pnode->dev_stat.is_online = true;
			   pnode->dev_stat.dumb_num = 0;
			   report03_devoffline(pnode->dev_p.dev_type_id, pnode->dev_p.dev_id, DEV_ONLINE);
			}
		}
	}
	else
	{
	    trace(TR_DEV_SER, "read 485addr[%d] reg_addr[%d] failed[ret = 0x%08X]\n", 
	    	pnode->dev_p.addr, vnode->var_p.reg_addr, ret);
	    pnode->dev_stat.dumb_num ++;
		if(pnode->dev_stat.dumb_num >= pnode->dev_p.dumb_num)
		{
//				    pnode->dev_stat.dumb_num = 0;
            pnode->dev_stat.is_online = false;
		}
		vnode->d.lv = MAGIC_VALUE;
		//串口异常重新初始化子串口
		if(ret & E_MODBUS_SER_ERR)
		{
		    const serial_t *ser = &dev_485[comn - 1].serial;
			trace(TR_DEV_SER, "subport[%d] err, re init ...\n");
			serial_init(ser);
		}
	}
	//记录抄读时间
	vnode->d.dt = tv;
}

/*****************************************************************************
 * Function      : mb_multi_var_read
 * Description   : modbus设备多寄存器读
 * Input         : dev_485_t *pnode
 * Output        : None
 * Return        : void
 * Others        : 
 * Record
 * 1.Date        : 20151217
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
static void mb_multi_var_read(const uint8 comn, dev_485_t *pnode )
{
	#define INVALID_ADDR	0xFFFF
	uint16 s_addr = INVALID_ADDR;
	uint16 l_addr = INVALID_ADDR;
	uint16 count = 0;
	uint32 period = 0;
	var_485_t * vnode = NULL;
	var_485_t *vsnode = NULL;
	struct timeval tv;
	struct timeval early_dt = {0,0};
	
    if(pnode == NULL)
		return;
	vnode = pnode->varlist.head;

    gettimeofday(&tv, NULL);
	while(NULL != vnode)
	{
	    while(1)
    	{
    	    if(0 == vnode->var_p.period)
				break;

            if((vnode->var_p.rw != ACCESS_R) && (vnode->var_p.rw != ACCESS_RW))
				break;
			//第一个
			if(0 == count)
			{
			    s_addr = vnode->var_p.reg_addr;
				l_addr = vnode->var_p.reg_addr;
				vsnode = vnode;
				early_dt = vnode->d.dt;
				count++;
//				trace(TR_DEV_SER,  "1 pn[%d] count[%d]\n", vnode->var_p.pn, count);
			}
			//找到连续
			else if((l_addr + 1 == vnode->var_p.reg_addr)
				&& (vsnode->var_p.period == vnode->var_p.period))
			{
			    l_addr = vnode->var_p.reg_addr;
			    count++;
				//记录最早抄读时间
				if((vnode->d.dt.tv_sec < early_dt.tv_sec) ||
					((vnode->d.dt.tv_sec == early_dt.tv_sec) && (vnode->d.dt.tv_usec < early_dt.tv_usec))
				)
					early_dt = vnode->d.dt;
//				trace(TR_DEV_SER,  "2 pn[%d] count[%d]\n", vnode->var_p.pn, count);
			}
			//前后不连续,且已找到连续多个
            else if(1 < count)
        	{
//        	    trace(TR_DEV_SER,  "3 pn[%d] count[%d]\n", vnode->var_p.pn, count);
        	    l_addr = INVALID_ADDR;
				vnode = vnode->prev;
        	}
			//前后不连续,且[未]找到连续多个
			else
			{
//			    trace(TR_DEV_SER,  "4 pn[%d] count[%d]\n", vnode->var_p.pn, count);
                //在这里把单个寄存器数据读了
                if(abs((tv.tv_sec - vsnode->d.dt.tv_sec) * 1000 + 
					(tv.tv_usec - vsnode->d.dt.tv_usec) / 1000) >= vsnode->var_p.period)
	                mb_single_var_read(comn, pnode, vsnode);
				
			    s_addr = vnode->var_p.reg_addr;
				l_addr = vnode->var_p.reg_addr;
				vsnode = vnode;
				early_dt = vnode->d.dt;
			}
			break;
    	}

		if((1 < count) && 
			(abs((tv.tv_sec - early_dt.tv_sec) * 1000 + 
				(tv.tv_usec - early_dt.tv_usec) / 1000) >= vsnode->var_p.period) && 
			((INVALID_ADDR == l_addr) || (NULL == vnode->next)))
		{
		    uint8 buffer[MB_SER_PDU_SIZE_MAX];
		    struct mb_read_param mbp = {MB_CH_SERIAL, comn, 
				(uint8_t)pnode->dev_p.addr, s_addr, count, buffer, 0};
			uint32 slen, rlen;
            int ret;

            trace(TR_DEV_SER, "multi_read comn[%d] DevTypeId[%d]DevId[%d]s_addr[%d]start_reg[%d]count[%d]\n",
            comn, pnode->dev_p.dev_type_id, pnode->dev_p.dev_id, pnode->dev_p.addr, s_addr, count);
			if(0 == (ret = mb_read(&mbp)))
			{
				trace_buf(TR_DEV_SER_BUF, "multi_recv :", mbp.recv_buf, mbp.recv_len);
				if(mbp.recv_len == 3 + 2 * count)
				{
				    appbuf_t *appbuf = (appbuf_t *)malloc(sizeof(appbuf_t));
					uint16 idx = 0;
#ifdef WSDL_PROTOCOL
					float32 wd, sd;
					boolean wdflag = false, sdflag = false;
#endif
					assert(NULL != appbuf);
					appbuf->buftyp = APPBUF_RPT;
					appbuf->frm_ctg = FRMCTG_RPT;
					//设备类型
					appbuf->data[idx++] = pnode->dev_p.dev_type_id / 256;
					appbuf->data[idx++] = pnode->dev_p.dev_type_id % 256;
					//长度后面在写
					idx +=2;					
					//设备ID
					appbuf->data[idx++] = pnode->dev_p.dev_id;
					while(count && (NULL != vsnode))
					{
					    uint32 len;
						float32 value = 0.0;
						var_cfg_t vcfg = {vsnode->var_p.pn, vsnode->var_p.vt, vsnode->var_p.dataprecision};
						
					    //获取真值
					    modbus_chan_t modchan = {mbp.mb_ch_type, mbp.port, mbp.slave_addr};
						value = hth_reg_to_value(pnode->dev_p.dev_type_id, vsnode->var_p.pn, 
							&mbp.recv_buf[mbp.recv_len - 2 * count], vsnode->var_p.dataprecision, &modchan);
#ifdef WSDL_PROTOCOL
						//温湿度传感器
						if(pnode->dev_p.dev_type_id == 1)
						{
						    if(vsnode->var_p.pn == 1)
					    	{
					    	    wdflag = true;
								wd = value;
					    	}
							else if(vsnode->var_p.pn == 2)
							{
							    sdflag = true;
								sd = value;
							}
						}
#endif
						if(OK == pack_var_value(&appbuf->data[idx], value, vcfg, &len))
						    idx += len;
						else
							trace(TR_DEV_SER, "value[%f]-pn[%d]-vt[%d]-dataprecision[%f] pack err\n", 
								value, vcfg.pn, vcfg.vt, vcfg.dataprecision);

                        //告警处理
                        serial_alarm(comn, pnode, vsnode, value);
                    	
						//保存当前数据
						vsnode->d.dt = tv;
						vsnode->d.lv = value;
						
						vsnode = vsnode->next;
						count--;
					}
					//记录设备最近一次成功应答记录
					pnode->dev_stat.lst_rd_time = tv;
					if(pnode->dev_stat.is_online == false)
					{
					   pnode->dev_stat.is_online = true;
					   pnode->dev_stat.dumb_num = 0;
					   report03_devoffline(pnode->dev_p.dev_type_id, pnode->dev_p.dev_id, DEV_ONLINE);
					}
					
					appbuf->len = idx;
					appbuf->data[2] = (idx - 5) >> 8;
					appbuf->data[3] = (idx - 5) % 256;
#ifdef WSDL_PROTOCOL
                    char mode[51] = {0};
                    system_getcommod(mode);
					if(strcmp(mode,"WebService") == 0)
					{
					    if(wdflag && sdflag)
							wsdl_reportTHInfo(pnode->dev_p.dev_id, pnode->dev_p.addr, wd, sd);
						else
							trace(TR_DEV_SER, "not thinfo dev \n");
					}
					else
#endif
						utm_rpt_send(appbuf);
					free(appbuf);
				}    
			}
			else
			{
			    trace(TR_DEV_SER, "read 485addr[%d] s_addr[%d] count[%d] failed[ret = 0x%08X]\n", 
					pnode->dev_p.addr, s_addr, count, ret);
				while(count-- && (NULL != vsnode))
				{
					//记录抄读时间
					vsnode->d.dt = tv;
//					vsnode->d.lv = MAGIC_VALUE;
					vsnode = vsnode->next;
				}
			    //TODO设备无响应次数
			    pnode->dev_stat.dumb_num ++;
				if((pnode->dev_stat.dumb_num >= pnode->dev_p.dumb_num)
					&& (pnode->dev_stat.is_online == true))
				{
                    pnode->dev_stat.is_online = false;
					report03_devoffline(pnode->dev_p.dev_type_id, pnode->dev_p.dev_id, DEV_OFFLINE);
				}
				//串口异常重新初始化子串口
				if(ret & E_MODBUS_SER_ERR)
				{
				    const serial_t *ser = &dev_485[comn - 1].serial;
					trace(TR_DEV_SER, "subport[%d] err, re init ...\n", comn);
				    serial_init(ser);
				}
			}

			count = 0;
			s_addr = INVALID_ADDR;
			vsnode = NULL;
			gettimeofday(&tv, NULL);
		}
		else if((abs((tv.tv_sec - vnode->d.dt.tv_sec) * 1000 
			+ (tv.tv_usec - vnode->d.dt.tv_usec) / 1000) >= vnode->var_p.period) 
			&& (NULL == vnode->next))
		{
		    //不连续,最后一个单个采集
		    mb_single_var_read(comn, pnode, vnode);
		}
		else if(l_addr == INVALID_ADDR)
		{
		    count = 0;
			s_addr = INVALID_ADDR;
			vsnode = NULL;
		}
		vnode = vnode->next;
	}
}

/*****************************************************************************
 * Function      : dev_485_read
 * Description   : dev_485_read
 * Input         : void
 * Output        : None
 * Return        : void
 * Others        : 
 * Record
 * 1.Date        : 20151201
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
static void *dev_485_read( void * arg)
{
    dev_485_t *pnode = NULL;
	var_485_list_t *vlist;
	var_485_t * vnode;
	struct timeval tv;
	time_t ltime;
	float32 value = 0.0;
	const int comn = *((int*)arg);
	dev_485_list_t * const plist = &dev_485[comn - 1].dev_485_list;
	char buffer[20];

	sprintf(buffer, "SER%d", comn);
	prctl(PR_SET_NAME, buffer);
	
	trace(TR_DEV_SER, "comn = %d\n", comn);
    FOREVER
	{
		usleep(10000);				    

        DEV_485_LOCK(comn);
        ltime = time(NULL);
	    pnode = plist->head;
	    while(NULL != pnode)
    	{
    	    vlist = &pnode->varlist;
			do
			{
			    if(NULL == vlist)
					break;
				
				//modbus协议采集
				if(PROTOCOL_MODBUS == pnode->dev_p.protocol_type)
					mb_multi_var_read((uint8)comn, pnode);	
				if(serial_modify_array[comn])
					goto have_a_rest;
//				//再尝试单寄存器读取
//				vnode = vlist->head;
//				while(NULL != vnode)
//				{
//				    do
//			    	{
//			    	    //周期为零不读
//			    	    if(0 == vnode->var_p.period)
//							break;
//						//变量类型不可读,不采集
//						if((vnode->var_p.rw != ACCESS_RW) && (vnode->var_p.rw != ACCESS_R))
//							break;
//						
//						if(-1 == gettimeofday(&tv, NULL))
//						{
//						    trace(TR_DEV_SER,  "gettimeofday err\n");
//							break;
//						}
//						if(abs((tv.tv_sec - vnode->d.dt.tv_sec) * 1000 + 
//							(tv.tv_usec - vnode->d.dt.tv_usec) / 1000) < vnode->var_p.period)
//							break;

//						mb_single_var_read(comn, pnode, vnode);
//						if(serial_modify_array[comn])
//							goto have_a_rest;

//			    	}while(0);
//					
//					vnode = vnode->next;
//				}
			}while(0);
			
			pnode = pnode->next;
			//里面时间长了影响web的及时响应,打断一下			
		
    	}
have_a_rest:
		DEV_485_UNLOCK(comn);

	}
}

/*****************************************************************************
 * Function      : ser_485_mgr
 * Description   : 485串口参数管理
 * Input         : devmgr_t *req
 * Output        : None
 * Return        : opt_rst_e
 * Others        : 
 * Record
 * 1.Date        : 20151227
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
opt_rst_e ser_485_mgr( devmgr_t *req )
{
    opt_rst_e ret = OPT_OK;
	uint8 comn = *((uint8*)req->pin);
	boolean is_changed = false;

	trace(TR_DEV_SER,  "comn = %d\n", comn);
	
    if((comn < 1) || (comn > SERIAL_COUNT))
		return OPT_ERR_ELSE;

	//申请访问串口下数据
	serial_request_start(comn);
	
	if(OPT_TYPE_SET == req->optt)
	{
	    serial_t *ser = (serial_t *)(req->pin + sizeof(uint8));

        
	    trace(TR_DEV_SER,  "ser->comn = %d par[%d] sbit[%d] baud[%d]\n", 
	    ser->comn, ser->p.par, ser->p.sbit, ser->p.baud);
		if(memcmp((void*)ser, (void*)&dev_485[ser->comn - 1].serial, sizeof(serial_t)) != 0)
		{
			memcpy((void*)&dev_485[ser->comn - 1].serial, (void*)ser, sizeof(serial_t));

			//子串口初始化
			serial_init((const serial_t *)ser);
			is_changed = true;
		}
	}
	else if(OPT_TYPE_GET == req->optt)
	{
	    if(comn != dev_485[comn - 1].serial.comn)
			ret = OPT_ERR_SER_NEXIST;
		else
			memcpy(req->pout, (void*)&dev_485[comn - 1].serial, sizeof(serial_t));
	}
	else if(OPT_TYPE_DEL == req->optt)
	{
	    dev_485_t *pnode = NULL;
		var_485_t *vnode = NULL;
		dev_485_list_t *plist = &dev_485[comn - 1].dev_485_list;

		pnode = plist->head;
		while(NULL != pnode)
		{
		    //删除变量列表
		    vnode = pnode->varlist.head;
			while(NULL != vnode)
			{
			    LIST_DEL((&pnode->varlist), vnode);
				free(vnode);
				vnode = pnode->varlist.head;
			}
			LIST_DEL(plist, pnode);
			free(pnode);
			pnode = plist->head;
		}
		is_changed = true;
		dev_485[comn - 1].serial.comn = 0;
	}

	if(true == is_changed)
		dev_485_save();
	//结束访问
	serial_request_end(comn);

	return ret;
}

/*****************************************************************************
 * Function      : dev_485_mgr
 * Description   : 485设备管理
 * Input         : devmgr_t *req
 * Output        : None
 * Return        : opt_rst_e
 * Others        : 
 * Record
 * 1.Date        : 20151227
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
opt_rst_e dev_485_mgr( devmgr_t *req )
{
    dev_header2_t *header = (dev_header2_t*)req->pin;
    opt_rst_e ret = OPT_OK;
	boolean is_changed = false;
	
    if((header->comn < 1) || (header->comn > SERIAL_COUNT))
		return OPT_ERR_ELSE;

	dev_485_t *pnode, *temp;

    //申请访问串口下数据
    serial_request_start(header->comn);
	pnode = dev_485_list_find(header->comn, header->dev_type_id, header->dev_id);

	if(OPT_TYPE_SET == req->optt)
	{
	    dev_485_info_t *info = (dev_485_info_t*)(((uint8*)req->pin) + sizeof(dev_header2_t));
        		
	    if((NULL != pnode) &&
			(memcmp((void*)info, (void*)&pnode->dev_p, sizeof(dev_485_info_t)) != 0))
    	{
			memcpy((void*)&pnode->dev_p, (void*)info, sizeof(dev_485_info_t));
			is_changed = true;
    	}
		else if(NULL == pnode)
		{
		    pnode = (dev_485_t *)calloc(1, sizeof(dev_485_t));
			assert(NULL != pnode);
			memcpy((void*)&pnode->dev_p, (void*)info, sizeof(dev_485_info_t));
			LIST_INIT((&pnode->varlist));
			DEV_LIST_INSERT((&dev_485[header->comn - 1].dev_485_list), pnode, temp);
			is_changed = true;
		}
	}
	else if(OPT_TYPE_GET == req->optt)
	{	
	    if(NULL != pnode)
			memcpy(req->pout, (void*)&pnode->dev_p, sizeof(dev_485_info_t));
		else
			ret = OPT_ERR_DEV_NEXIST;
	}
	else if(OPT_TYPE_DEL == req->optt)
	{		
	    if(NULL != pnode)
    	{
    	    var_485_t *vnode = NULL;
			dev_485_list_t *plist = &dev_485[header->comn - 1].dev_485_list;
			var_485_list_t *vlist = &pnode->varlist;
			
    	    vnode = vlist->head;
			while(NULL != vnode)
			{
			    LIST_DEL(vlist, vnode);
				free(vnode);
				vnode = vlist->head;
			}
			LIST_DEL(plist, pnode);
			free(pnode);
			is_changed = true;
    	}
		else
			ret = OPT_ERR_DEV_NEXIST;
	}

	if(true == is_changed)
		dev_485_save();

	serial_request_end(header->comn);
	return ret;
}

/*****************************************************************************
 * Function      : var_485_mgr
 * Description   : 485设备变量管理
 * Input         : devmgr_t *req
 * Output        : None
 * Return        : opt_rst_e
 * Others        : 
 * Record
 * 1.Date        : 20151227
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
opt_rst_e var_485_mgr( devmgr_t *req )
{
    var_header2_t *header = (var_header2_t*)req->pin;
    boolean is_changed = false;
	
	if((header->comn < 1) || (header->comn > SERIAL_COUNT))
		return OPT_ERR_ELSE;

	//申请访问串口下数据
    serial_request_start(header->comn);

	dev_485_t *pnode = NULL;
	pnode = dev_485_list_find(header->comn, header->dev_type_id, header->dev_id);
	if(NULL == pnode)
	{
	    DEV_485_UNLOCK(header->comn);
		return OPT_ERR_DEV_NEXIST;
	}

	opt_rst_e ret = OPT_OK;
	var_485_t *vnode = NULL;

	vnode = var_485_list_find(header->pn, &pnode->varlist);

	if(OPT_TYPE_SET == req->optt)
	{
	    var_485_info_t *vinfo = (var_485_info_t *)(((uint8*)req->pin) + sizeof(var_header2_t));
		var_485_t *temp = NULL;
		
	    if((NULL != vnode) &&
			(memcmp((void*)vinfo, (void*)&vnode->var_p, sizeof(var_485_info_t)) != 0))
    	{
			memcpy((void*)&vnode->var_p, (void*)vinfo, sizeof(var_485_info_t));
			is_changed = true;
			//重新排序
			var_485_list_t vlist;

			LIST_INIT(&vlist);
			vnode = pnode->varlist.head;
			while(NULL != vnode)
			{
			    LIST_DEL((&pnode->varlist), vnode);
			    VAR_LIST_INSERT((&vlist), vnode, temp, reg_addr);
				vnode = pnode->varlist.head;
			}
			pnode->varlist = vlist;
    	}
		else if(NULL == vnode)
		{
		    vnode = (var_485_t*)calloc(1, sizeof(var_485_t));
			assert(vnode != NULL);
			memcpy((void*)&vnode->var_p, (void*)vinfo, sizeof(var_485_info_t));
			vnode->d.lv = MAGIC_VALUE;
			VAR_LIST_INSERT((&pnode->varlist), vnode, temp, reg_addr);
			is_changed = true;
		}
	}
	else if(OPT_TYPE_GET == req->optt)
	{
	    if(NULL != vnode)
			memcpy(req->pout, (void*)&vnode->var_p, sizeof(var_485_info_t));
		else
		    ret = OPT_ERR_VAR_NEXIST;
	}
	if(OPT_TYPE_DEL == req->optt)
	{
	    if(NULL != vnode)
    	{
    	    LIST_DEL((&pnode->varlist), vnode);
			free(vnode);
			is_changed = true;
    	}
		else
		    ret = OPT_ERR_VAR_NEXIST;
	}

    if(true == is_changed)
		dev_485_save();
	
	//结束访问
    serial_request_end(header->comn);
	return ret;
}

/*****************************************************************************
 * Function      : read_485_sd
 * Description   : 读取设备数据
 * Input         : uint16 dev_type_id
                uint8 dev_id
                uint8* pack
                uint32 *len
 * Output        : None
 * Return        : status_t
 * Others        : 
 * Record
 * 1.Date        : 20151230
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
utm_rst_e read_485_sd( uint16 dev_type_id, uint8 dev_id, uint8* pack, uint32 *len )
{
    dev_485_t *node;
	utm_rst_e ret = UTM_DEV_NOTCFG;;
	uint32 idx = 0;
	uint8 comn;
	var_485_t *vnode;

	for(comn = 1; comn <= SERIAL_COUNT; comn++)
	{
	    //申请访问串口下数据
	    serial_request_start(comn);
		
		node = dev_485_list_find(comn, dev_type_id, dev_id);	    
		if(NULL == node)
			goto request_end;

        pack[idx++] = dev_type_id >> 8;
		pack[idx++] = dev_type_id % 256;
		//长度先填0
		pack[idx++] = 0;
		pack[idx++] = 0;
		pack[idx++] = dev_id;
				
		vnode = node->varlist.head;
		while(NULL != vnode)
	    {
		    uint32 plen;
			var_cfg_t vcfg = {vnode->var_p.pn, vnode->var_p.vt, vnode->var_p.dataprecision};
			
			if(OK == pack_var_value(&pack[idx], vnode->d.lv, vcfg, &plen))
			    idx += plen;
			else
				trace(TR_DEV_SER,  "value[%f]-pn[%d]-vt[%d]-dataprecision[%f] pack err\n", 
					vnode->d.lv, vcfg.pn, vcfg.vt, vcfg.dataprecision);
			vnode = vnode->next;
		}
		//消息正文中，单个设备数据长度不含设备ID
		pack[2] = (idx - 5) >> 8;
		pack[3] = (idx - 5) % 256;
		*len = idx;
		ret = UTM_OK;
		serial_request_end(comn);
		break;
		
request_end:
		serial_request_end(comn);
	}

	return ret;
}

/*****************************************************************************
 * Function      : ctrl_485
 * Description   : 485的控制帧
 * Input         : uint8 *data
 * Output        : None
 * Return        : utm_rst_e
 * Others        : 
 * Record
 * 1.Date        : 20151230
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
utm_rst_e ctrl_485( uint8 *data )
{
	uint8 comn;
    uint16 pn;
	uint32 idx = 5;
	var_485_t *vnode = NULL;
	uint16 dev_type_id = (data[0] << 8) + data[1];
	uint16 len =  (data[2] << 8 ) + data[3];
	uint8 dev_id = data[4];
	dev_485_t *node485 = NULL;
	var_485_list_t *vlist = NULL;
	utm_rst_e ret = UTM_DEV_NOTCFG;
	
	for(comn = 1; comn <= SERIAL_COUNT; comn++)
	{
	    serial_request_start(comn);
		
		node485 = dev_485_list_find(comn, dev_type_id, dev_id);
		if(NULL == node485)
			goto request_continue;

		vlist = &node485->varlist;
		while(idx < (len + 5))
		{
		    pn = (data[idx] << 8) + data[idx + 1];
			idx += 2;
			trace(TR_DEV_SER,  "pn = %d\n", pn);
			vnode = var_485_list_find(pn, vlist);
			if(NULL == vnode)
			{
			    trace(TR_DEV_SER,  "UTM_VAR_NOTCFG\n");
			    ret = UTM_VAR_NOTCFG;
				goto request_end;
			}
			//判断变量是否可写
			if((vnode->var_p.rw != ACCESS_W) && (vnode->var_p.rw != ACCESS_RW))
				break;
           
            int mbret;
			uint8 send_buf[2] = {0};
			uint32 plen = 0;
			float32 value;
			var_cfg_t vcfg = {pn, vnode->var_p.vt, vnode->var_p.dataprecision};
			struct mb_write_param param = {comn, node485->dev_p.addr, vnode->var_p.reg_addr, 1, send_buf};
			
			if(OK == get_var_value(&data[idx], vcfg, &value, &plen))
			{
			    trace(TR_DEV_SER,  "dev_type_id[%d]dev_id[%d]pn[%d] reg_addr[%d] value[%f]mb write \n", 
						dev_type_id, dev_id, pn, vnode->var_p.reg_addr, value);
			    hth_value_to_reg(value, vnode->var_p.dataprecision, send_buf);
				trace_buf(TR_DEV_SER_BUF, "modbus send:", send_buf, 2);
				if(0 != (mbret = mb_write(&param)))
				{
					trace(TR_DEV_SER,  "dev_type_id[%d]dev_id[%d]pn[%d] mb write err[mbret = 0x%08X]\n", 
						dev_type_id, dev_id, pn, mbret);
					if(mbret == MODBUS_ERR(E_MODBUS_RECV_NOACK))
						ret = UTM_MODBUS_NOACK;
					else if(mbret == MODBUS_ERR(E_MODBUS_RECV_FRAME_LEN))
						ret = UTM_MODBUS_RECVLEN;
					else if(mbret == MODBUS_ERR(E_MODBUS_RECV_CRC))
						ret = UTM_MODBUS_CRC;
					else 
						ret = UTM_MODBUS_ELSE;
					break;
				}
				idx += plen;
				vnode->d.lv = value;
				gettimeofday(&vnode->d.dt, NULL); 
			}
			else
			{
				trace(TR_DEV_SER,  "get_var_value err\n");
				break;
			}    
		}
		if(idx != (len + 5))
			ret = UTM_PACK_FMT_ERR;
		else
			ret = UTM_OK;
		
request_end:
		serial_request_end(comn);
		break;
		
request_continue:
		serial_request_end(comn);
	}

	return ret;
}

/*****************************************************************************
 * Function      : dev_485_init
 * Description   : IO设备对应GPIO引脚初始化
 * Input         : void
 * Output        : None
 * Return        : void
 * Others        : 
 * Record
 * 1.Date        : 20151222
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
void dev_485_init( void )
{
	uint8 idx;
	int ret0, ret;

#if 1
    ret0 = mb_init();
    	//加载配置文件
    dev_485_load();
	
    for(idx = 0; idx < SERIAL_COUNT; idx ++)
	{
	    serial_t ser;

		memcpy((void*)&ser, (void*)&dev_485[idx].serial, sizeof(serial_t));
		//子串口初始化
		if(ser.comn == 0)
			continue;

		//初始化端口变量告警状态
		serial_alarm_load(ser.comn);
	}
	
	//创建上行业务处理线程
	pthread_attr_t attr;
	
	ret = pthread_attr_init(&attr);
	assert(ret == 0);
	ret = pthread_attr_setstacksize(&attr, RS485_THREAD_STACK_SIZE);
	assert(ret == 0);
		
    
	if((ret0 & E_WK2166_OPEN_HW_RESET_PIN) == 0)
	{
		if((ret0 & E_WK2166_OPEN_USART2) == 0)
		{
            for(idx = 0; idx < 4; idx++)
			{
			    //创建互斥锁
			    if(0 != pthread_mutex_init(&(dev_485[idx].serial_mutex), NULL))
		    	{
					trace(TR_DEV_SER,  "create dev_485_mutex [%d] failed\n", idx);
					assert(0);
		    	}
				
			    ret = pthread_create(&tid_485_array[idx], &attr, dev_485_read, (void*)&comn_array[idx]);
				if(ret != 0)
					trace(TR_DEV_SER,  "Create dev_io_read pthread error[%d]!\n", ret);
			}
		}
		else
			trace(TR_EVENT, "open usart2 err\n");
		if((ret0 & E_WK2166_OPEN_USART3) == 0)
		{
			for(idx = 4; idx < 8; idx++)
			{
			    //创建互斥锁
			    if(0 != pthread_mutex_init(&(dev_485[idx].serial_mutex), NULL))
		    	{
					trace(TR_DEV_SER,  "create dev_485_mutex [%d] failed\n", idx);
					assert(0);
		    	}
				
			    ret = pthread_create(&tid_485_array[idx], &attr, dev_485_read, (void*)&comn_array[idx]);
				if(ret != 0)
					trace(TR_DEV_SER,  "Create dev_io_read pthread error[%d]!\n", ret);
			}
		}
		else
			trace(TR_EVENT, "open usart3 err\n");
	}

	for(idx = 8; idx < 10; idx++)
	{
	    //创建互斥锁
	    if(0 != pthread_mutex_init(&(dev_485[idx].serial_mutex), NULL))
    	{
			trace(TR_DEV_SER,  "create dev_485_mutex [%d] failed\n", idx);
			assert(0);
    	}
		
	    ret = pthread_create(&tid_485_array[idx], &attr, dev_485_read, (void*)&comn_array[idx]);
		if(ret != 0)
			trace(TR_DEV_SER,  "Create dev_io_read pthread error[%d]!\n", ret);
	}
#else
    //主串口初始化
    ret = usart_wk2166_init();
	if(0 != ret)
	{
	    trace(TR_SYSTEM, "main port init err[0x%08X]\n", ret);
	    assert(0);
	}
	//CH9和CH10打开串口
	usarts_open();

	//加载配置文件
    dev_485_load();
	
    for(idx = 0; idx < SERIAL_COUNT; idx ++)
	{
	    serial_t ser;

		memcpy((void*)&ser, (void*)&dev_485[idx].serial, sizeof(serial_t));
		//子串口初始化
		if(ser.comn == 0)
			continue;

		//初始化端口变量告警状态
		serial_alarm_load(ser.comn);
	}

	trace(TR_DEV_SER,  "usart2_wk2xxx_init\n");

    //创建上行业务处理线程
	pthread_attr_t attr;
	
	ret = pthread_attr_init(&attr);
	assert(ret == 0);
	ret = pthread_attr_setstacksize(&attr, RS485_THREAD_STACK_SIZE);
	assert(ret == 0);
    //创建IO设备轮询线程
	for(idx = 0; idx < SERIAL_COUNT; idx++)
	{
	    //创建互斥锁
	    if(0 != pthread_mutex_init(&(dev_485[idx].serial_mutex), NULL))
    	{
			trace(TR_DEV_SER,  "create dev_485_mutex [%d] failed\n", idx);
			assert(0);
    	}
		
	    ret = pthread_create(&tid_485_array[idx], &attr, dev_485_read, (void*)&comn_array[idx]);
		if(ret != 0)
			trace(TR_DEV_SER,  "Create dev_io_read pthread error[%d]!\n", ret);
	}
#endif
}

