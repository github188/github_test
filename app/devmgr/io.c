/********************************************************************************

 **** Copyright (C), 2015, BinHong tech Co., Ltd.                ****

 ********************************************************************************
 * File Name     : io.c
 * Author        : chenxu
 * Date          : 2015-12-22
 * Version       : 1.0
 * Function List :
 * Description   : IO设备处理相关API
 * Record        :
 * 1.Date        : 2015-12-22
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
#include "maths.h"
#include "gpio.h"
#include "protocol.h"
#include "utm.h"
#include "modbus_lib.h"
#include "web.h"
#include "mxml.h"
#include "alarm.h"
#include "io.h"

/*------------------------------------------------------------------------------
Section: Macro Definitions
------------------------------------------------------------------------------*/
#define DEV_IO_LOCK 	pthread_mutex_lock(&dev_io_mutex)
#define DEV_IO_UNLOCK  	pthread_mutex_unlock(&dev_io_mutex)

//存储IO状态文件
#define F_DEV_IO_STAT	"/usr/devmgr/iostat"

/*------------------------------------------------------------------------------
Section: Type Definitions
------------------------------------------------------------------------------*/
typedef struct
{
    uint16 dev_type_id;
	uint8 dev_id;
	uint16 pn;
	boolean stat;
}io_stat_t;
/*------------------------------------------------------------------------------
Section: Globals
------------------------------------------------------------------------------*/
pthread_t tid_io;
dev_io_list_t dev_io_list;
static pthread_mutex_t dev_io_mutex;
static int fd_io = -1;
const uint32 pin_in_index[] = {0 , DIN1, DIN2, DIN3, DIN4, DIN5, DIN6, DIN7, DIN8};
const uint32 pin_out_index[] = {DOUT0, DOUT1, DOUT2, DOUT3};
const char* AlarmCondition[] = {"NONE", "ON告警", "OFF告警", "ON至OFF告警", "OFF至ON告警"};
/*------------------------------------------------------------------------------
Section: Function Prototypes
------------------------------------------------------------------------------*/
/*****************************************************************************
 * Function      : AlarmCondition
 * Description   : AlarmCondition
 * Input         : char *str
 * Output        : None
 * Return        : io_warn_e
 * Others        : 
 * Record
 * 1.Date        : 20160104
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
io_warn_e get_warn_type( char * str)
{
    io_warn_e wt = WARN_TYPE_NONE;
    uint32 len = strlen(str);
    uint32 size = ARRAY_SIZE(AlarmCondition);
    uint32 i;
	
	for(i = 1; i < size; i++)
	{
	    if((strlen(AlarmCondition[i]) <= len)
			&& (strstr(str, AlarmCondition[i]) != NULL))
		{
    	    wt = (io_warn_e)i;
			break;
    	}
	}

	return wt;
}

/*****************************************************************************
 * Function      : get_pin_sn
 * Description   : 获取IO挂在序号
 * Input          : None
 * Output        : None
 * Return        : 
 * Others        : 
 * Record
 * 1.Date        : 20160104
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
int  get_pin_sn( uint32 pin_index )
{
    uint32 size = ARRAY_SIZE(pin_in_index);
	uint32 i;

	for(i = 1; i < size; i++)
	{
	    if(pin_in_index[i] == pin_index)
			return i;
	}
	size  = ARRAY_SIZE(pin_out_index);
	for(i = 0; i < size; i++)
	{
	    if(pin_out_index[i] == pin_index)
			return (i + 9);
	}
	
	return -1;
}

/*****************************************************************************
 * Function      : dev_io_list_find
 * Description   : 查找指定IO设备的节点信息
 * Input         : dev_io_list_t *plist
                uint16 dev_type_id
                uint8 dev_id
 * Output        : None
 * Return        : dev_io_t *
 * Others        :
 * Record
 * 1.Date        : 20151126
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
static dev_io_t * dev_io_list_find(uint16 dev_type_id, uint8 dev_id )
{
    dev_io_t *ptmp = dev_io_list.head;

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
 * Function      : var_io_list_find
 * Description   : 查找指定IO设备变量的信息
 * Input         : uint16 pn, 
 				var_io_list_t *vlist
 * Output        : None
 * Return        : var_io_t *
 * Others        :
 * Record
 * 1.Date        : 20151126
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
static var_io_t * var_io_list_find( uint16 pn, var_io_list_t *vlist )
{
    var_io_t * vnode = NULL;

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
 * Function      : io_stat_save
 * Description   : 保存当前IO设备状态
 * Input         : void
 * Output        : None
 * Return        : void
 * Others        : 
 * Record
 * 1.Date        : 20160215
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
static void io_stat_save( void )
{
    dev_io_t *pnode;
	var_io_t *vnode;
    FILE *fp;
	io_stat_t iostat;

    if(NULL == (fp = fopen(F_DEV_IO_STAT, "wb")))
	{
	    trace(TR_DEV_IO, "fopen file [%s] err\n", F_DEV_IO_STAT);
		return;
	}

	pnode = dev_io_list.head;
	while(NULL != pnode)
	{
	    vnode = pnode->varlist.head;
		while(NULL != vnode)
		{
		    iostat.dev_type_id = pnode->dev_p.dev_type_id;
			iostat.dev_id = pnode->dev_p.dev_id;
			iostat.pn = vnode->var_p.pn;
            iostat.stat = vnode->d.stat;
			fwrite((char*)&iostat, sizeof(iostat), 1, fp);
			vnode = vnode->next;
		}
		pnode = pnode->next;
	}
	fclose(fp);
	
}

/*****************************************************************************
 * Function      : io_stat_load
 * Description   : 加载io状态信息
 * Input         : void
 * Output        : None
 * Return        : static void
 * Others        : 
 * Record
 * 1.Date        : 20160215
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
static void io_stat_load( void)
{
    dev_io_t *pnode;
	var_io_t *vnode;
    FILE *fp;
	io_stat_t iostat;

    if(NULL == (fp = fopen(F_DEV_IO_STAT, "r")))
	{
	    trace(TR_DEV_IO, "fopen file [%s] err\n", F_DEV_IO_STAT);
		return;
	}

	while(0 != fread((char*)&iostat, sizeof(iostat), 1, fp))
	{
	    pnode = dev_io_list_find(iostat.dev_type_id, iostat.dev_id);
		if(NULL != pnode)
		{
		    vnode = var_io_list_find(iostat.pn, &pnode->varlist);
			if(NULL != vnode)
				vnode->d.stat = iostat.stat;
		}
	}
	fclose(fp);

}

/*****************************************************************************
 * Function      : dev_io_read
 * Description   : IO设备数据采集
 * Input          : void
 * Output        : None
 * Return        : void
 * Others        : 
 * Record
 * 1.Date        : 20151201
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
static void *dev_io_read( void )
{
    dev_io_t *pnode = NULL;
	dev_io_list_t * const plist = &dev_io_list;
	var_io_list_t *vlist;
	var_io_t * vnode;
	struct timeval tv;

    prctl(PR_SET_NAME, IO_THREAD_NAME);

    FOREVER
	{
	    usleep(1000);//延时1毫秒
	    DEV_IO_LOCK;
		
	    pnode = plist->head;
	    while(NULL != pnode)
    	{
    	    vlist = &pnode->varlist;
			do
			{
			    if(NULL == vlist)
					break;
				vnode = vlist->head;
				while(NULL != vnode)
				{
				    do
			    	{
			    	    //采集周期为0的不采集
			    	    if(0 == vnode->var_p.period)
							break;
						//没有读权限的不采集
						if((vnode->var_p.rw != ACCESS_RW) && (vnode->var_p.rw != ACCESS_R))
							break;
						
						if(-1 == gettimeofday(&tv, NULL))
						{
						    trace(TR_DEV_IO, "gettimeofday err\n");
							break;
						}
						if(abs((tv.tv_sec - vnode->d.dt.tv_sec) * 1000 + 
							(tv.tv_usec - vnode->d.dt.tv_usec) / 1000) < vnode->var_p.period)
							break;
						//查询该变量
						at91_gpio_arg arg;
						arg.pin = pnode->dev_p.pin_index;		
						arg.data = 555;		
						ioctl(fd_io, IOCTL_GPIO_GETVALUE, &arg);
						trace(TR_DEV_IO, "dev_type[%d]dev_id[%d]dev_name[%s]pn[%d]pname[%s]data[%d]\n", 
							pnode->dev_p.dev_type_id, 
							pnode->dev_p.dev_id,
							pnode->dev_p.dev_name, 
							vnode->var_p.pn, vnode->var_p.pname, arg.data);

						//告警判断
						boolean cstat = (boolean)arg.data;
						if(cstat != vnode->d.stat)
						{
						    char message[100];
							snprintf(message, 100, "%s %s %s",
								pnode->dev_p.mount_name,
								pnode->dev_p.dev_name, 
								(cstat == 1) ? "OFF to ON": "ON to OFF");
							
							trace(TR_DEV_IO, "%s\n", message);
							
							if((vnode->var_p.alarm_enable)
								&& (((vnode->var_p.wt == ALARM_TYPE_OFF2ON) && (cstat == true)) ||
								((vnode->var_p.wt == ALARM_TYPE_ON2OFF) && (cstat == false))))
								alarm_write(pnode->dev_p.dev_name, 
								            vnode->var_p.pname, 
								            cstat ? ALARM_STAT_OFF2ON : ALARM_STAT_ON2OFF,
								            message);

							//保存当前IO状态
							io_stat_save();
						}
						//数据上送
                        appbuf_t *appbuf = (appbuf_t *)malloc(sizeof(appbuf_t));
						uint16 idx = 0;

						assert((NULL != appbuf));
						appbuf->buftyp = APPBUF_RPT;
						appbuf->frm_ctg = FRMCTG_RPT;
						//设备类型
						appbuf->data[idx++] = pnode->dev_p.dev_type_id / 256;
						appbuf->data[idx++] = pnode->dev_p.dev_type_id % 256;
						//长度后面再写
						appbuf->data[idx++] = 3;
						appbuf->data[idx++] = 0;						
						//设备ID
						appbuf->data[idx++] = pnode->dev_p.dev_id;

						//变量标识码
						appbuf->data[idx++] = vnode->var_p.pn % 256;
						appbuf->data[idx++] = vnode->var_p.pn / 256;
						//数据内容
						appbuf->data[idx++] = (uint8)arg.data;
						appbuf->len = idx;
						//发送数据
						char mode[51] = {0};
	                    system_getcommod(mode);
						if(strcmp(mode,"WebService") != 0)
							utm_rpt_send(appbuf);
						free(appbuf);
						//更新数据
						vnode->d.stat = (boolean)arg.data;
						vnode->d.dt = tv;
						//更新设备在线状态
//						if((pnode->dev_stat.is_online == true)
//							&& (pnode->dev_stat.dumb_num >= pnode->dev_p.dumb_num))
//							pnode->dev_stat.is_online = false;
			    	}while(0);
					
					vnode = vnode->next;
				}
			}while(0);
			
			pnode = pnode->next;
    	}
		DEV_IO_UNLOCK;
	}
}

static const char *				/* O - Whitespace string or NULL */
whitespace_cb(mxml_node_t *node,	/* I - Element node */
              int         where)	/* I - Open or close tag? */
{
  mxml_node_t	*parent;		/* Parent node */
  int		level;			/* Indentation level */
  const char	*name;			/* Name of element */
  static const char *tabs = "\t\t\t\t\t\t\t\t";
					/* Tabs for indentation */


 /*
  * We can conditionally break to a new line before or after any element.
  * These are just common HTML elements...
  */

  name = node->value.element.name;

  if (!strcmp(name, "html") || !strcmp(name, "head") || !strcmp(name, "body") ||
      !strcmp(name, "pre") || !strcmp(name, "p") ||
      !strcmp(name, "h1") || !strcmp(name, "h2") || !strcmp(name, "h3") ||
      !strcmp(name, "h4") || !strcmp(name, "h5"))
//      !strcmp(name, "Dev") || !strcmp(name, "Param") || !strcmp(name, "ParamName") ||
//      !strcmp(name, "MountPoint") || !strcmp(name, "IoDev") || !strcmp(name, "ParamType") ||
//      !strcmp(name, "AlarmEnable") || !strcmp(name, "AlarmCondition") || !strcmp(name, "SampleFrequency"))
  {
   /*
    * Newlines before open and after close...
    */

    if (where == MXML_WS_BEFORE_OPEN || where == MXML_WS_AFTER_CLOSE)
      return ("\n");
  }
  else if (!strcmp(name, "dl") || !strcmp(name, "ol") || !strcmp(name, "ul"))
  {
   /*
    * Put a newline before and after list elements...
    */

    return ("\n");
  }
  else if (!strcmp(name, "dd") || !strcmp(name, "dt") || !strcmp(name, "li") 
//  	|| !strcmp(name, "Dev") || !strcmp(name, "Param") || !strcmp(name, "ParamName") ||
//      !strcmp(name, "MountPoint") || !strcmp(name, "IoDev") || !strcmp(name, "ParamType") ||
//      !strcmp(name, "AlarmEnable") || !strcmp(name, "AlarmCondition") || !strcmp(name, "SampleFrequency")
      )
  {
   /*
    * Put a tab before <li>'s, <dd>'s, and <dt>'s, and a newline after them...
    */

    if (where == MXML_WS_BEFORE_OPEN)
      return ("\t");
    else if (where == MXML_WS_AFTER_CLOSE)
      return ("\n");
  }
  else if (!strncmp(name, "?xml", 4))
  {
    if (where == MXML_WS_AFTER_OPEN)
      return ("\n");
    else
      return (NULL);
  }
  else if (where == MXML_WS_BEFORE_OPEN ||
           ((!strcmp(name, "MountPoint") || !strcmp(name, "Dev") 
           || !strcmp(name, "Param")
           ) &&
	    where == MXML_WS_BEFORE_CLOSE))
  {
    for (level = -1, parent = node->parent;
         parent;
	 level ++, parent = parent->parent);

    if (level > 8)
      level = 8;
    else if (level < 0)
      level = 0;

    return (tabs + 8 - level);
  }
  else if (where == MXML_WS_AFTER_CLOSE ||
           ((!strcmp(name, "IoDev") || !strcmp(name, "MountPoint") || !strcmp(name, "Dev") 
           || !strcmp(name, "Param")
           ) &&
            where == MXML_WS_AFTER_OPEN))
    return ("\n");
  else if (where == MXML_WS_AFTER_OPEN && !node->child)
    return ("\n");

 /*
  * Return NULL for no added whitespace...
  */

  return (NULL);
}

static mxml_type_t				/* O - Data type */
type_cb(mxml_node_t *node)		/* I - Element node */
{
  const char	*type;			/* Type string */


 /*
  * You can lookup attributes and/or use the element name, hierarchy, etc...
  */

  if ((type = mxmlElementGetAttr(node, "type")) == NULL)
    type = node->value.element.name;

  if (!strcmp(type, "integer"))
    return (MXML_INTEGER);
  else if (!strcmp(type, "opaque") || !strcmp(type, "pre"))
    return (MXML_OPAQUE);
  else if (!strcmp(type, "real"))
    return (MXML_REAL);
  else
    return (MXML_TEXT);
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
static void dev_io_load( void )
{
	int	i;
	FILE *fp;
	char filename[100];	
	value_type_e vt;
	io_warn_e wt;
	uint8 rw;
    char *attr_value;
	int  whitespace;
	char buf[100];
	mxml_node_t	*tree,
			*IoDev,
			*MountPoint,
			*Dev,
			*Param,
			*node;

    LIST_INIT(((dev_io_list_t *)&dev_io_list));
	sprintf(filename, "%s%s", CFG_PATH, IoDev_XML);
	if((fp = fopen(filename, "r")) == NULL)
	{
		trace(TR_DEV_IO,  "open file %s failed\n", filename);
		return;
	}	
	
	tree = mxmlLoadFile(NULL, fp, MXML_NO_CALLBACK);
//    tree = mxmlLoadFile(NULL, fp, type_cb);	
	fclose(fp);

	//find IoDev
	IoDev = mxmlFindElement(tree, tree, "IoDev", NULL, NULL, MXML_DESCEND);
    assert(IoDev != NULL);

	MountPoint = mxmlFindElement(IoDev, tree, "MountPoint", NULL, NULL, MXML_DESCEND);
	while(NULL != MountPoint)
	{
	    dev_io_t *nodeio, *dtemp;
		var_io_t *vnode, *vtemp;

		nodeio = (dev_io_t *)calloc(1, sizeof(dev_io_t));
		assert(NULL != nodeio);
		
        //挂在信息
		attr_value = mxmlElementGetAttr(MountPoint, "Name");
		assert(NULL != attr_value);	
		strcpy(nodeio->dev_p.mount_name, attr_value);
		if(attr_value[1] == 'I')
		{
			rw = ACCESS_R;
		    nodeio->dev_p.pin_index = pin_in_index[atoi(&attr_value[2])];	
		}
		else
		{
			rw = ACCESS_W;
		    nodeio->dev_p.pin_index = pin_out_index[atoi(&attr_value[2]) - 9];	
		}
		trace(TR_DEV_IO,  "Name=%s, sn[%d], pin_index = %d,rw = %d\n", 
			attr_value, atoi(&attr_value[2]), nodeio->dev_p.pin_index, rw);
		//设备信息
		Dev = mxmlFindElement(MountPoint, MountPoint, "Dev", NULL, NULL, MXML_DESCEND);
		assert(Dev != NULL);

		attr_value = mxmlElementGetAttr(Dev, "DevName");
		assert(NULL != attr_value);
		strcpy(nodeio->dev_p.dev_name, attr_value);
		trace(TR_DEV_IO,  "DevName=%s\n", nodeio->dev_p.dev_name);

		attr_value = mxmlElementGetAttr(Dev, "DevType");
		assert(NULL != attr_value);
		nodeio->dev_p.dev_type_id = atoi(attr_value);
		trace(TR_DEV_IO,  "DevType=%d\n", nodeio->dev_p.dev_type_id);
		
		attr_value = mxmlElementGetAttr(Dev, "DevID");
		assert(NULL != attr_value);
		nodeio->dev_p.dev_id = atoi(attr_value);
		trace(TR_DEV_IO,  "DevID=%d\n", nodeio->dev_p.dev_id);
		//param
		Param = mxmlFindElement(Dev, Dev, "Param", NULL, NULL, MXML_DESCEND);
		while(NULL != Param)
		{
			vnode = (var_io_t *)calloc(1, sizeof(var_io_t));
			assert(NULL != vnode);
			
	        //ParamName
			node = mxmlFindElement(Param, Param, "ParamName", NULL, NULL, MXML_DESCEND);
			assert(node != NULL);
			node = mxmlGetLastChild(node);
			attr_value = mxmlGetText(node, &whitespace);
			assert(attr_value != NULL);		
	        strcpy(vnode->var_p.pname, attr_value);
			trace(TR_DEV_IO,  "ParamName=%s\n", vnode->var_p.pname);

			node = mxmlFindElement(Param, Param, "ParamType", NULL, NULL, MXML_DESCEND);
			assert(node != NULL);
			node = mxmlGetLastChild(node);
			attr_value = mxmlGetText(node, &whitespace);
	        vnode->var_p.pn = atoi(attr_value);
			trace(TR_DEV_IO,  "ParamType=%d\n", vnode->var_p.pn);

			node = mxmlFindElement(Param, Param, "AlarmEnable", NULL, NULL, MXML_DESCEND);
			assert(node != NULL);
			node = mxmlGetLastChild(node);
			attr_value = mxmlGetText(node, &whitespace);
			assert(attr_value != NULL);
			trace(TR_DEV_IO,  "AlarmEnable=%s\n", attr_value);
	        //告警使能标志
	        if(strstr(attr_value, "True") != NULL)
				vnode->var_p.alarm_enable = 1;
			else
				vnode->var_p.alarm_enable = 0;

			node = mxmlFindElement(Param, Param, "AlarmCondition", NULL, NULL, MXML_DESCEND);
			assert(node != NULL);
			node = mxmlGetLastChild(node);
			attr_value = mxmlGetText(node, &whitespace);
			assert(attr_value != NULL);
			char *p1, *p2;
			p1 = strstr(attr_value, "ON");
			p2 = strstr(attr_value, "OFF");
//	        if(p1 && !p2)
//				vnode->var_p.wt = WARN_TYPE_ON;
//			else if(!p1 && p2)
//				vnode->var_p.wt = WARN_TYPE_OFF;
			if(p1 && p2 && (p1 < p2))
				vnode->var_p.wt = ALARM_TYPE_ON2OFF;
			else if(p1 && p2 && (p1 > p2))
				vnode->var_p.wt = ALARM_TYPE_OFF2ON;
			else 
				vnode->var_p.wt = ALARM_TYPE_NONE;
			strcpy(vnode->var_p.warn_string, attr_value);
			trace(TR_DEV_IO,  "wt:%d warn_string:%s\n", vnode->var_p.wt, vnode->var_p.warn_string);
	        

			node = mxmlFindElement(Param, Param, "SampleFrequency", NULL, NULL, MXML_DESCEND);
			assert(node != NULL);
			node = mxmlGetLastChild(node);
			assert(node);
			attr_value = mxmlGetText(node, &whitespace);
	        vnode->var_p.period =atoi(attr_value);
			trace(TR_DEV_IO,  "SampleFrequency=%d\n", vnode->var_p.period);

			var_io_list_t *vlist = &nodeio->varlist;
	        LIST_INIT(vlist);
	        VAR_LIST_INSERT(vlist, vnode, vtemp, pn);

			Param = mxmlGetNextSibling(Param);
			if(Param)
			{
			    trace(TR_DEV_IO,  "Param->type = %d\n", Param->type);
				Param = mxmlGetNextSibling(Param);
			}
		}
        DEV_LIST_INSERT((&dev_io_list), nodeio, dtemp);

		MountPoint = mxmlGetNextSibling(MountPoint);
	}

	mxmlDelete(tree);
    return;
}

/*****************************************************************************
 * Function      : dev_io_save
 * Description   : 将io参数存入文件
 * Input         : void
 * Output        : None
 * Return        : status_t
 * Others        : 
 * Record
 * 1.Date        : 20151229
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
static status_t dev_io_save( void )
{
    status_t ret = ERROR;
	char filename[100];
	char filename1[100];
	int			i;
	int          fd = -1;
	FILE	    *fp;
	mxml_node_t	*tree;
	mxml_node_t	*MountPoint;
	mxml_node_t	*IoDev;
    mxml_node_t	*Dev;
	mxml_node_t	*Param;
    mxml_node_t	*node;
	char buffer[100];

	tree = mxmlNewXML("1.0");
	assert(NULL != tree);	
	
	IoDev = mxmlNewElement(tree, "IoDev");
	assert(NULL != IoDev);
	
	dev_io_t *pnode = dev_io_list.head;
	var_io_t *vnode;
	var_io_list_t * vlist;
	
	while(NULL != pnode)
	{
	    vlist = &pnode->varlist;
	    vnode = vlist->head;
		
	    MountPoint = mxmlNewElement(IoDev, "MountPoint");
		assert(NULL != MountPoint);
		trace(TR_DEV_IO,  "mount_name = %s\n", pnode->dev_p.mount_name);
		mxmlElementSetAttr(MountPoint, "Name", pnode->dev_p.mount_name);
		mxmlElementSetAttr(MountPoint, "Direction", 
			((pnode->dev_p.mount_name[1] == 'I') ? "In" : "Out"));

		Dev = mxmlNewElement(MountPoint, "Dev");
		assert(NULL != Dev);
		
        mxmlElementSetAttr(Dev, "DevName", pnode->dev_p.dev_name);
		sprintf(buffer, "%d", pnode->dev_p.dev_type_id);
		mxmlElementSetAttr(Dev, "DevType", buffer);
		mxmlElementSetAttr(Dev, "Protocal", "IO");
        sprintf(buffer, "%d", pnode->dev_p.dev_id);
		mxmlElementSetAttr(Dev, "DevID", buffer);	
		
		while(NULL != vnode)
		{
		    trace(TR_DEV_IO,  "pn [%d]\n", vnode->var_p.pn);
			
            Param = mxmlNewElement(Dev, "Param");	
			assert(NULL != Param);
			
			node = mxmlNewElement(Param ,"ParamName");
			assert(NULL != node);
			node = mxmlNewText(node, 0, vnode->var_p.pname);
			assert(NULL != node);
			
			node = mxmlNewElement(Param, "ParamType");
			assert(NULL != node);
			node = mxmlNewInteger(node, vnode->var_p.pn);
			assert(NULL != node);
			
			node = mxmlNewElement(Param, "AlarmEnable");
			assert(NULL != node);
			node = mxmlNewText(node, 0, (vnode->var_p.alarm_enable == 1) ? "True": "False");
			assert(NULL != node);
			
			node = mxmlNewElement(Param, "AlarmCondition");
			assert(NULL != node);
			trace(TR_DEV_IO,  "wt:%d warn_string:%s\n", vnode->var_p.wt, vnode->var_p.warn_string);
			node = mxmlNewText(node, 0, vnode->var_p.warn_string);
            assert(NULL != node);
			
			node = mxmlNewElement(Param, "SampleFrequency");
			assert(NULL != node);
			node = mxmlNewInteger(node, vnode->var_p.period);
            assert(NULL != node);

			vnode =  vnode->next;
		}
        pnode = pnode->next;
	}
	
	sprintf(filename, "%s%s", CFG_PATH, IoDev_XML);

	if(NULL == (fp = fopen(filename, "w+")))
	{
	    trace(TR_DEV_IO,  "creat file %s failed\n", filename);
		mxmlDelete(tree);
		return ret;
	}

    ret = mxmlSaveFile(tree, fp, MXML_NO_CALLBACK);
//    ret = mxmlSaveFile(tree, fp, whitespace_cb);
    assert(-1 != ret);
	
    fclose(fp);
    trace(TR_DEV_IO,  "save success\n");
	mxmlDelete(tree);

	ret = OK;
	return ret;
}

/*****************************************************************************
 * Function      : dev_io_mgr
 * Description   : IO设备管理
 * Input         : devmgr_t *req
 * Output        : None
 * Return        : opt_rst_e
 * Others        : 
 * Record
 * 1.Date        : 20151227
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
opt_rst_e dev_io_mgr( devmgr_t *req )
{
    dev_header1_t *header = (dev_header1_t *)req->pin;
	dev_io_t * pnode = NULL;
	dev_io_t * temp = NULL;
	opt_rst_e ret = OPT_OK;
	boolean is_changed = false;

    trace(TR_DEV_IO,  "dev_type_id[%d]dev_id[%d]\n", header->dev_type_id, header->dev_id);
    DEV_IO_LOCK;
	pnode = dev_io_list_find(header->dev_type_id, header->dev_id);
	
	if(OPT_TYPE_SET == req->optt)
	{
	    dev_io_info_t *info = (dev_io_info_t *)(req->pin + sizeof(dev_header1_t));

		trace(TR_DEV_IO,  "%p %pdev_type_id[%d]dev_id[%d]-%s-%d\n", 
			req->pin, info, info->dev_type_id, info->dev_id, info->dev_name, info->pin_index);
	    if((NULL != pnode) && 
			(memcmp((void*)info, (void*)&pnode->dev_p, sizeof(dev_io_info_t)) != 0))
    	{
	    	memcpy((void*)&pnode->dev_p, (void*)info, sizeof(dev_io_info_t));
			is_changed = true;
    	}
		else if(NULL == pnode)
		{
		    pnode = (dev_io_t*)calloc(1, sizeof(dev_io_t));
			assert(pnode != NULL);
			memcpy((void*)&pnode->dev_p, (void*)info, sizeof(dev_io_info_t));
			LIST_INIT(&pnode->varlist);
			DEV_LIST_INSERT((&dev_io_list), pnode, temp);
			is_changed = true;
		}
	}
	else if(OPT_TYPE_GET == req->optt)
	{
	    if(NULL != pnode)
			memcpy(req->pout, (void*)&pnode->dev_p, sizeof(dev_io_info_t));
		else
			ret = OPT_ERR_DEV_NEXIST;
	}
	if(OPT_TYPE_DEL == req->optt)
	{
	    if(NULL != pnode)
    	{
    	    var_io_list_t *vlist = &pnode->varlist;
			var_io_t *vnode = NULL;
    	    //删除变量列表
			vnode = vlist->tail;
			while(NULL != vnode)
			{
			    LIST_DEL(vlist, vnode);
				free(vnode);
				vnode = vlist->tail;
			}
			//删除设备
			LIST_DEL((&dev_io_list), pnode);
			free(pnode);
			is_changed = true;
			dev_io_save();
    	}
		else
			ret = OPT_ERR_DEV_NEXIST;
	}

	if(true == is_changed)
		dev_io_save();

	DEV_IO_UNLOCK;

	return ret;
}

/*****************************************************************************
 * Function      : var_io_mgr
 * Description   : IO变量管理
 * Input         : devmgr_t *req
 * Output        : None
 * Return        : opt_rst_e
 * Others        : 
 * Record
 * 1.Date        : 20151227
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
opt_rst_e var_io_mgr( devmgr_t *req )
{
    opt_rst_e ret = OPT_OK;
    var_header1_t *header = (var_header1_t *)req->pin;
    dev_io_t *pnode = NULL;
	boolean is_changed = false;

    DEV_IO_LOCK;
	pnode = dev_io_list_find(header->dev_type_id, header->dev_id);
    if(NULL == pnode)
	{
	    DEV_IO_UNLOCK;
		return OPT_ERR_DEV_NEXIST;
	}

	var_io_t *vnode = NULL;
	var_io_t *temp = NULL;
	var_io_list_t *vlist = &pnode->varlist;
	vnode = var_io_list_find(header->pn, vlist);
	if(OPT_TYPE_SET == req->optt)
	{
	    var_io_info_t *vinfo = (var_io_info_t *)(req->pin + sizeof(var_header1_t));

		if((NULL != vnode) &&
			(memcmp((void*)&vnode->var_p, (void*)vinfo, sizeof(var_io_info_t)) != 0))
    	{
    	    trace(TR_DEV_IO,  "modify pn[%d] wt[%d]\n", vinfo->pn, vinfo->wt);
			memcpy((void*)&vnode->var_p, (void*)vinfo, sizeof(var_io_info_t));
			trace(TR_DEV_IO,  "modify pn[%d] wt[%d]\n", vnode->var_p.pn, vnode->var_p.wt);
			is_changed = true;
    	}
		else if(NULL == vnode)
		{
		    trace(TR_DEV_IO,  "add pn[%d] wt[%d]\n", vinfo->pn, vinfo->wt);
			vnode = (var_io_t *)calloc(1, sizeof(var_io_t));
			assert(vnode != NULL);		
			memcpy((void*)&vnode->var_p, (void*)vinfo, sizeof(var_io_info_t));
			trace(TR_DEV_IO,  "add pn[%d] wt[%d]\n", vnode->var_p.pn, vnode->var_p.wt);
			VAR_LIST_INSERT(vlist, vnode, temp, pn);
			is_changed = true;
		}	
	}
	else if(OPT_TYPE_GET == req->optt)
	{
	    if(NULL != vnode)
			memcpy(req->pout, (void*)&vnode->var_p, sizeof(var_io_info_t));
		else
		    ret = OPT_ERR_VAR_NEXIST;
	}
	else if(OPT_TYPE_DEL == req->optt)
	{
	    if(NULL != vnode)
    	{
    	    LIST_DEL(vlist, vnode);
			free(vnode);
			is_changed = true;
    	}
		else
		    ret = OPT_ERR_VAR_NEXIST;
	}

    if(true == is_changed)
		dev_io_save();
	
    DEV_IO_UNLOCK;
	return ret;
}

/*****************************************************************************
 * Function      : io_read_sd
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
utm_rst_e read_io_sd( uint16 dev_type_id, uint8 dev_id, uint8* pack, uint32 *len )
{
    dev_io_t *node;
	utm_rst_e ret;
	uint32 idx = 0;
	var_io_t *vnode;

	node = dev_io_list_find(dev_type_id, dev_id);
	if(NULL != node)
	{
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
			var_cfg_t vcfg = {vnode->var_p.pn, vnode->var_p.vt, vnode->var_p.dot};
			
			if(OK == pack_var_value(&pack[idx], (float32)vnode->d.stat, vcfg, &plen))
			    idx += plen;
			else
				trace(TR_DEV_IO,  "value[%f]-pn[%d]-vt[%d]-dataprecision[%f] pack err\n", 
					(float32)vnode->d.stat, vcfg.pn, vcfg.vt, vcfg.dataprecision);
			vnode = vnode->next;
		}
		//消息正文中，单个设备数据长度不含设备ID
		pack[2] = (idx - 5) >> 8;
		pack[3] = (idx - 5) % 256;
		*len = idx;
		ret = UTM_OK;
	}
	else
		ret = UTM_DEV_NOTCFG;

	return ret;
}

/*****************************************************************************
 * Function      : ctrl_io
 * Description   : io设备控制命令
 * Input         : uint8 *data
 * Output        : None
 * Return        : utm_rst_e
 * Others        : 
 * Record
 * 1.Date        : 20151230
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
utm_rst_e ctrl_io( uint8 *data )
{
	uint16 dev_type_id = (data[0] << 8) + data[1];
	uint16 len =  (data[2] << 8 ) + data[3];
	uint8 dev_id = data[4];
	dev_io_t *node = NULL;
	var_io_list_t *vlist = NULL;
	utm_rst_e ret = UTM_ERR_ELSE;
	
	node = dev_io_list_find(dev_type_id, dev_id);
    trace_buf(TR_DEV_IO_BUF, "io write data:", data, 8);
	if(NULL != node)
	{
	    uint16 pn;
		uint32 idx = 5;
	    var_io_t *vnode = NULL;

		vlist = &node->varlist;
		while(idx < (len + 5))
		{
		    pn = (data[idx] << 8) + data[idx + 1];
			trace(TR_DEV_IO, "ioctl pn = %d \n", pn);
			idx += 2;
			vnode = var_io_list_find(pn, vlist);
			if(NULL == vnode)
			{
			    ret = UTM_VAR_NOTCFG;
				break;
			}
			//判断变量是否可写
			if((vnode->var_p.rw != ACCESS_W) && (vnode->var_p.rw != ACCESS_RW))
				break;

			uint32 plen = 0;
			float32 value;
			var_cfg_t vcfg = {pn, vnode->var_p.vt, vnode->var_p.dot};
			
			if(OK == get_var_value(&data[idx], vcfg, &value, &plen))
			{
                int stat = 0;
                at91_gpio_arg arg;
				
				if(value > 0.0001)
					stat = 1;

                trace(TR_DEV_IO,  "ioctl pin_index = %d stat = %d\n", node->dev_p.pin_index, stat);
				arg.pin = node->dev_p.pin_index;
				arg.data = stat;
				arg.usepullup = 0;
				ioctl(fd_io, IOCTL_GPIO_SETVALUE, &arg);
				
				idx += plen;
				vnode->d.stat = stat;
				gettimeofday(&vnode->d.dt, NULL); 
			}
			else
			{
				trace(TR_DEV_IO,  "get_var_value err\n");
				break;
			}    
		}
		if(idx != (len + 5))
			ret = UTM_PACK_FMT_ERR;
		else
			ret = UTM_OK;
	}
	else
		ret = UTM_DEV_NOTCFG;

	return ret;
}

/*****************************************************************************
 * Function      : dev_io_init
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
void dev_io_init( void )
{
    at91_gpio_arg arg;
	uint32 idx;
	int ret;

	fd_io = open(DEV_GPIO, O_RDWR);
	assert(fd_io != -1);

	for(idx = 1; idx < ARRAY_SIZE(pin_in_index); idx++)
	{
		arg.pin = pin_in_index[idx];
		arg.data = 0;
		arg.usepullup = 0;
		ioctl(fd_io, IOCTL_GPIO_SETINPUT, &arg);    
	}

	for(idx = 0; idx < ARRAY_SIZE(pin_out_index); idx++)
	{
		arg.pin = pin_out_index[idx];
		arg.data = 0;
		arg.usepullup = 0;
		ioctl(fd_io, IOCTL_GPIO_SETOUTPUT, &arg);    
	}

    //加载配置文件
    dev_io_load();

	//加载复位前IO状态
	io_stat_load();

	//创建互斥锁
    if(0 != pthread_mutex_init(&dev_io_mutex, NULL))
		trace(TR_DEV_IO,  "create dev_mutex failed\n");
	
    //创建IO设备轮询线程
    //创建上行业务处理线程
	pthread_attr_t attr;
	
	ret = pthread_attr_init(&attr);
	assert(ret == 0);
	ret = pthread_attr_setstacksize(&attr, IO_THREAD_STACK_SIZE);
	assert(ret == 0);
	ret = pthread_create(&tid_io, &attr, (void *) dev_io_read, NULL);
	if(ret != 0)
		trace(TR_DEV_IO,  "Create dev_io_read pthread error[%d]!\n", ret);
}

