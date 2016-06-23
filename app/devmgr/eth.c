/********************************************************************************

 **** Copyright (C), 2015, BinHong tech Co., Ltd.                ****

 ********************************************************************************
 * File Name     : eth.c
 * Author        : chenxu
 * Date          : 2015-12-23
 * Version       : 1.0
 * Function List :
 * Description   : eth通道相关处理
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
#include "maths.h"
#include "gpio.h"
#include "protocol.h"
#include "utm.h"
#include "modbus_lib.h"
#include "wk2166_lib.h"
#include "web.h"
#include "mxml.h"
#include "wsdl.h"
#include "system.h"
#include "alarm.h"
#include "share.h"
#include "ptl.h"
#include "eth.h"
#include <linux/if.h> 
#include <linux/ethtool.h> 
#include <linux/sockios.h>
/*------------------------------------------------------------------------------
Section: Macro Definitions
------------------------------------------------------------------------------*/
#ifdef SERIAL_ALARM
#undef SERIAL_ALARM
#endif
#define SERIAL_ALARM 


/*------------------------------------------------------------------------------
Section: Type Definitions
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
Section: Globals
------------------------------------------------------------------------------*/
static mount_list_t net_mount_list;
static boolean is_phy_ready = false;
/*------------------------------------------------------------------------------
Section: Function Prototypes
------------------------------------------------------------------------------*/
static void *dev_eth_read(mount_net_t * const mount_net);

/*****************************************************************************
 * Function      : do_linkstat_check
 * Description   : 链接状态检查
 * Input         : mount_net_t *mount_net
 * Output        : None
 * Return        : status_t
 * Others        : 
 * Record
 * 1.Date        : 20160302
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
static status_t do_linkstat_check( mount_net_t *mount_net )
{
    status_t ret = ERROR;
	struct sockaddr_in server_addr;
	
    //参数变更检查
    if(mount_net->flagofnetparamchange)
	{
		mount_net->link_stat = LINK_STAT_RESET;
		mount_net->flagofnetparamchange = false;
	}

	switch(mount_net->link_stat)
	{
	    case LINK_STAT_RESET:
			mount_net->link_stat = LINK_STAT_WAIT_PHY;
			if(mount_net->sockfd > 0)
			{
				close(mount_net->sockfd);
				mount_net->sockfd = -1;
			}
			trace(TR_DEV_ETH, "link reset ok!\n");
			break;
		case LINK_STAT_WAIT_PHY:
			if(is_phy_ready)
			{
				mount_net->link_stat = LINK_STAT_CSOCKET;
				trace(TR_DEV_ETH, "link phy is ready!\n");
			}
			break;
		case LINK_STAT_CSOCKET:
			//创建挂载点的socket
		    mount_net->sockfd = socket(AF_INET, SOCK_STREAM, 0);
			if(mount_net->sockfd < 0)
			    trace(TR_DEV_ETH, "open socket err\n");
            else
        	{
        	    //非阻塞模式
			    fcntl(mount_net->sockfd, F_SETFL, O_NONBLOCK);
			    int flags = fcntl(mount_net->sockfd, F_GETFL, 0);
			    fcntl(mount_net->sockfd, F_SETFL, flags | O_NONBLOCK);
				mount_net->link_stat = LINK_STAT_CONNECT;
				trace(TR_DEV_ETH, "link creat socket succ!\n");
        	}
			break;
		case LINK_STAT_CONNECT:
			bzero(&server_addr, sizeof(server_addr));
		    server_addr.sin_family = AF_INET;

			server_addr.sin_port = htons(mount_net->net.port);
			server_addr.sin_addr = mount_net->net.ip;	

		    if(ret = connect(mount_net->sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) == 0)
//				trace(TR_DEV_ETH, "do_comm_connect error[%d]! ip[%s], port[%d]\n",ret, 
//						inet_ntoa(server_addr.sin_addr), ntohs(server_addr.sin_port));
//			else
			{
				mount_net->link_stat = LINK_STAT_LOGIN;
				trace(TR_DEV_ETH, "link connect succ!\n");
			}
			break;
		case LINK_STAT_LOGIN:
			if(is_phy_ready)
			{
			    //检查sock服务端是否断开链接
	            char ch;
				if(0 == (ret = recv(mount_net->sockfd, &ch, sizeof(ch), MSG_DONTWAIT|MSG_PEEK)))
				{
				    mount_net->link_stat = LINK_STAT_RESET;
				    trace(TR_DEV_ETH, "socket break\n");
				}
				else
					ret = OK;
			}
			else
				mount_net->link_stat = LINK_STAT_RESET;
			break;
		default:
			break;
	}

	return ret;
}

/*****************************************************************************
 * Function      : net_phy_check
 * Description   : 网络设备网线接入检查
 * Input         : void
 * Output        : None
 * Return        : void
 * Others        : 
 * Record
 * 1.Date        : 20160302
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
static void *net_phy_check( void )
{
    int skfd; 
	struct ifreq ifr; 
	struct ethtool_value edata;
	comm_param_t commp;
	boolean phy_ready;

	prctl(PR_SET_NAME, "PHY");
	
	FOREVER
	{

        sleep(1);
	
		edata.cmd = ETHTOOL_GLINK; 
		edata.data = 0; 
		
		memset(&ifr, 0, sizeof(ifr));
		system_getcommparam(&commp);
		if(commp.mcflag == 2)
			strcpy(ifr.ifr_name, "eth0"); 
		else
			strcpy(ifr.ifr_name, "eth1"); 
		ifr.ifr_data = (char *) &edata; 
		
		if(( skfd = socket( AF_INET, SOCK_DGRAM, 0 )) < 0) 
		{
		    continue;
		}
		
		if(ioctl( skfd, SIOCETHTOOL, &ifr ) != -1) 
		{
			phy_ready = (edata.data > 0) ? true : false;
			if(is_phy_ready != phy_ready)
			{
			    is_phy_ready = phy_ready;
				trace(TR_DEV_ETH, "NetWork[%s] %s\n", ifr.ifr_name, phy_ready ? "Connect":"DisConnect");
			}
		}
		close(skfd); 
	}
}


/*****************************************************************************
 * Function      : net_mount_insert
 * Description   : 插入网络挂载点
 * Input         : mount_net_t *mount_net
 * Output        : None
 * Return        : void
 * Others        : 
 * Record
 * 1.Date        : 20160215
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
static void net_mount_insert( mount_net_t *mount_net )
{
    if(NULL != net_mount_list.tail)
	{
		net_mount_list.tail->next = mount_net;
		mount_net->prev = net_mount_list.tail;
	}
	else
	{
	    net_mount_list.head = mount_net;
	}

    net_mount_list.tail = mount_net;
	net_mount_list.count++;
}

/*****************************************************************************
 * Function      : net_mount_find
 * Description   : 
 * Input          : char *name
 * Output        : None
 * Return        : 
 * Others        : 
 * Record
 * 1.Date        : 20160215
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
static mount_net_t * net_mount_find( char *name)
{
    mount_net_t *mnode = net_mount_list.head;
    uint32 len = strlen(name);
	
	while(mnode != NULL)
	{
	    if((strlen(mnode->net.name) == len)
			&& (strcmp(mnode->net.name, name) == 0))
			break;
		mnode = mnode->next;
	}

	return mnode;
}

/*****************************************************************************
 * Function      : net_mount_init
 * Description   : 网络挂载点初始化
 * Input         : mount_net_t *mount_net
 * Output        : None
 * Return        : status_t
 * Others        : 
 * Record
 * 1.Date        : 20160229
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
static status_t net_mount_init( mount_net_t *mount_net )
{
    int ret;
	pthread_attr_t attr;
	struct sockaddr_in server_addr;

	//创建互斥量
    ret = pthread_mutex_init(&mount_net->net_mutex, NULL);
	assert(ret == 0);

	mount_net->flagofnetparamchange = true;
//	//创建挂载点的socket
//    mount_net->sockfd = socket(AF_INET, SOCK_STREAM, 0);
//	if(mount_net->sockfd < 0)
//	{
//	    trace(TR_DEV_ETH, "open socket err\n");
//		return ERROR;
//	}

//	//非阻塞模式
//    fcntl(mount_net->sockfd, F_SETFL, O_NONBLOCK);
//    int flags = fcntl(mount_net->sockfd, F_GETFL, 0);
//    fcntl(mount_net->sockfd, F_SETFL, flags | O_NONBLOCK);
//	
//	bzero(&server_addr, sizeof(server_addr));
//    server_addr.sin_family = AF_INET;

//	server_addr.sin_port = htons(mount_net->net.port);
//	server_addr.sin_addr = mount_net->net.ip;	

//    if(connect(mount_net->sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0)
//	{

//		trace(TR_DEV_ETH, "do_comm_connect error! ip[%s], port[%d]\n",
//				inet_ntoa(server_addr.sin_addr), ntohs(server_addr.sin_port));
//		return ERROR;
//	}
	//创建采集线程
	ret = pthread_attr_init(&attr);
	assert(ret == 0);
	ret = pthread_attr_setstacksize(&attr, ETH_THREAD_STACK_SIZE);
	assert(ret == 0);
    ret = pthread_create(&mount_net->tid, &attr, dev_eth_read, (void*)mount_net);
	assert(ret == 0);

	return OK;
}

/*****************************************************************************
 * Function      : net_mount_exit
 * Description   : 退出网络挂载点的所有服务
 * Input         : mount_net_t * mount_net
 * Output        : None
 * Return        : status_t
 * Others        : 
 * Record
 * 1.Date        : 20160229
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
static status_t net_mount_exit( mount_net_t * mount_net )
{
    int ret;
	void *status;
    //采集线程退出
	
    ret = pthread_cancel(mount_net->tid);
	if(0 == ret)
	{
		ret = pthread_join(mount_net->tid, &status);
		assert(ret == 0);
		trace(TR_DEV_ETH, "thread resource recycle succ,ret[%d] status[%s]\n", ret, status);
	}
	else
		trace(TR_DEV_ETH, "%s tid[%d] failed\n", __FUNCTION__);
	
	//关闭socket
	if(-1 != mount_net->sockfd)
	{
		close(mount_net->sockfd);
	    mount_net->sockfd = -1;
	}

	return OK;
}

/*****************************************************************************
// * Function      : net_read_thread_creat
// * Description   : net_read_thread_creat
// * Input         : mount_net_t *mount_net
// * Output        : None
// * Return        : void
// * Others        : 
// * Record
// * 1.Date        : 20160215
// *   Author      : chenxu
// *   Modification: Created function

//*****************************************************************************/
//static void net_read_thread_creat( mount_net_t *mount_net )
//{
//    int ret;
//	pthread_attr_t attr;

//	ret = pthread_mutex_init(&mount_net->net_mutex, NULL);
//	assert(ret == 0);
//	ret = pthread_attr_init(&attr);
//	assert(ret == 0);
//	ret = pthread_attr_setstacksize(&attr, ETH_THREAD_STACK_SIZE);
//	assert(ret == 0);
//    ret = pthread_create(&mount_net->tid, &attr, dev_eth_read, (void*)mount_net);
//	assert(ret == 0);
//}

/*****************************************************************************
// * Function      : net_read_thread_cancel
// * Description   : net_read_thread_cancel
// * Input         : pthread_t tid
// * Output        : None
// * Return        : void
// * Others        : 
// * Record
// * 1.Date        : 20160215
// *   Author      : chenxu
// *   Modification: Created function

//*****************************************************************************/
//static void net_read_thread_cancel( pthread_t tid)
//{
//    int ret;
//	void *status;
//	
//    ret = pthread_cancel(tid);
//	if(0 == ret)
//	{
//		ret = pthread_join(tid, &status);
//		assert(ret == 0);
//		trace(TR_DEV_ETH, "thread resource recycle succ\n");
//	}
//	else
//		trace(TR_DEV_ETH, "%s tid[%d] failed\n", __FUNCTION__);
//}

/*****************************************************************************
 * Function      : dev_eth_list_find
 * Description   : void
 * Input         : dev_eth_list_t *plist
                uint16 dev_type_id
                uint8 dev_id
 * Output        : None
 * Return        : dev_eth_t *
 * Others        : 
 * Record
 * 1.Date        : 20151128
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
static dev_eth_t * dev_eth_list_find( dev_eth_list_t *plist, uint16 dev_type_id, uint8 dev_id )
{
    dev_eth_t *ptmp = plist->head;

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
 * Function      : var_eth_list_find
 * Description   : eth设备变量查找
 * Input          : uint16 pn 
 				var_eth_list_t *vlist
 * Output        : None
 * Return        : var_eth_t *
 * Others        : 
 * Record
 * 1.Date        : 20151227
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
static var_eth_t * var_eth_list_find( uint16 pn, var_eth_list_t *vlist )
{
    var_eth_t * vnode = NULL;

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
 * Function      : net_alarm_save
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
static void net_alarm_save( mount_net_t *mount_net )
{
    FILE *fp;
    char filename[100] = {0};
    dev_eth_t *pnode;
	var_eth_t *vnode;
	alarm_stat_t valarm;

	//TODO 待优化成单个变量写
	sprintf(filename, "%salarm%d", ALARM_FILE_HEADER, mount_net->net.port);
    if(NULL == (fp = fopen(filename, "wb")))
	{
	    trace(TR_DEV_ETH, "fopen file [%s] err\n", filename);
		return;
	}
	pnode = mount_net->dev_eth_list.head;
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
 * Function      : net_alarm_load
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
static void net_alarm_load(mount_net_t *mount_net )
{
    FILE *fp;
    char filename[100] = {0};
	dev_eth_t *pnode;
	var_eth_t *vnode;
	alarm_stat_t valarm;

    printf("serial_alarm_load port = %d\n", mount_net->net.port);
	sprintf(filename, "%salarm%d", ALARM_FILE_HEADER, mount_net->net.port);
    if(NULL == (fp = fopen(filename, "r")))
	{
	    trace(TR_DEV_ETH, "fopen file [%s] err\n", filename);
		return;
	}
	while(0 != fread((char*)&valarm, sizeof(valarm), 1, fp))
	{
	    pnode = dev_eth_list_find(&mount_net->dev_eth_list, valarm.dev_type_id, valarm.dev_id);
		if(NULL != pnode)
		{
		    vnode = var_eth_list_find(valarm.pn, &pnode->varlist);
			if(NULL != vnode)
				vnode->alarm_stat = valarm.alarm_stat;
		}
	}
	fclose(fp);
}

/*****************************************************************************
 * Function      : serial_alarm
 * Description   : 串口设备越限告警
 * Input         : dev_eth_t *pnode
                var_eth_t *vnode
                float32 cvalue
 * Output        : None
 * Return        : void
 * Others        : 
 * Record
 * 1.Date        : 20160127
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
static void net_alarm( mount_net_t *mount_net, dev_eth_t *pnode, var_eth_t *vnode, float32 cvalue )
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
			net_alarm_save(mount_net);
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
static void dev_eth_load(void)
{
    uint32 idx;
    status_t ret = ERROR;
	char filename[100];
	int			i;
	int          fd = -1;
	FILE	    *fp;
	mxml_node_t	*tree,
		*NetDev,
		*MountPoint,
    	*Dev,
		*Param,
    	*node;
	char *attr_value;
	int  whitespace;
	mount_net_t *mount_net;
	dev_eth_t *pnode, *temp;
	var_eth_t *vnode, *vtemp;
	
	LIST_INIT(&net_mount_list);

    sprintf(filename, "%s%s", CFG_PATH, NetDev_xml);
	if(NULL == (fp = fopen(filename, "r")))
	{
	    trace(TR_DEV_ETH,  "fopen %s failed\n", filename);
		return;
	}

	tree = mxmlLoadFile(NULL, fp, MXML_NO_CALLBACK);
	fclose(fp);

	NetDev = mxmlFindElement(tree, tree, "NetDev", NULL, NULL, MXML_DESCEND);
	assert(NetDev != NULL);

	MountPoint = mxmlFindElement(NetDev, NetDev, "MountPoint", NULL, NULL, MXML_DESCEND);
	while(NULL != MountPoint)
	{
	    mount_net = (mount_net_t *) calloc(1, sizeof(mount_net_t));
		assert(mount_net);
		LIST_INIT(&mount_net->dev_eth_list);
		
	    attr_value = mxmlElementGetAttr(MountPoint, "Name");
		assert(attr_value);
		strncpy(mount_net->net.name, attr_value, sizeof(mount_net->net.name));
		trace(TR_DEV_ETH, "net.name = %s\n", mount_net->net.name);

		attr_value = mxmlElementGetAttr(MountPoint, "IP");
		assert(attr_value);
		inet_pton(AF_INET, attr_value, (void*)&mount_net->net.ip);
		trace(TR_DEV_ETH, "net.ip = 0x%X\n", mount_net->net.ip);

		attr_value = mxmlElementGetAttr(MountPoint, "Port");
		assert(attr_value);
		mount_net->net.port = atoi(attr_value);
		trace(TR_DEV_ETH, "net.port = %d\n", mount_net->net.port);

		attr_value = mxmlElementGetAttr(MountPoint, "Protocal");
		assert(attr_value);
		strncpy(mount_net->net.ptl_type, attr_value, sizeof(mount_net->net.ptl_type));
		trace(TR_DEV_ETH, "net.ptl_type = %s\n", mount_net->net.ptl_type);

        //将挂载点插入net_mount_list
        net_mount_insert(mount_net);
        
		//找设备
		Dev = mxmlFindElement(MountPoint, MountPoint, "Dev", NULL, NULL, MXML_DESCEND);
		while(NULL != Dev)
		{
		    pnode = (dev_eth_t *)calloc(1, sizeof(dev_eth_t));
			assert(pnode);
			LIST_INIT(&pnode->varlist);

			attr_value = mxmlElementGetAttr(Dev, "DevName");
			assert(attr_value);
			strncpy(pnode->dev_p.dev_name, attr_value, sizeof(pnode->dev_p.dev_name));
			trace(TR_DEV_ETH,  "dev_name= %s\n", pnode->dev_p.dev_name);

			attr_value = mxmlElementGetAttr(Dev, "DevType");
			assert(attr_value);
			pnode->dev_p.dev_type_id = atoi(attr_value);
			trace(TR_DEV_ETH,  "dev_type_id = %d\n", pnode->dev_p.dev_type_id);

            attr_value = mxmlElementGetAttr(Dev, "Protocal");
			assert(attr_value);
			pnode->dev_p.protocol_type = get_ptltype_sn(attr_value);
			trace(TR_DEV_ETH,  "protocol_type = %s[%d]\n", attr_value, pnode->dev_p.protocol_type);
			
			attr_value = mxmlElementGetAttr(Dev, "DevID");
			assert(NULL != attr_value);
			pnode->dev_p.dev_id = atoi(attr_value);
			trace(TR_DEV_ETH, "dev_id = %d\n", pnode->dev_p.dev_id);

			attr_value = mxmlElementGetAttr(Dev, "DevAddr");
			assert(NULL != attr_value);
			pnode->dev_p.addr = atoi(attr_value);
			trace(TR_DEV_ETH,  "addr = %d\n", pnode->dev_p.addr);

			attr_value = mxmlElementGetAttr(Dev, "NoAck");
			assert(NULL != attr_value);
			pnode->dev_p.dumb_num = atoi(attr_value);
			trace(TR_DEV_ETH,  "dumb_num = %d\n", pnode->dev_p.dumb_num);

			//param
			Param = mxmlFindElement(Dev, Dev, "Param", NULL, NULL, MXML_DESCEND);
			while(NULL != Param)
			{
			    vnode = (var_eth_t*)calloc(1, sizeof(var_eth_t));
				assert(NULL != vnode);

				node = mxmlFindElement(Param, Param, "Name", NULL, NULL, MXML_DESCEND);
				assert(NULL != node);
				node = mxmlGetLastChild(node);
				assert(NULL != node);
				attr_value = mxmlGetText(node, &whitespace);
				assert(attr_value);
		        strcpy(vnode->var_p.pname, attr_value);
				trace(TR_DEV_ETH,  "pname = %s\n", vnode->var_p.pname);

				node = mxmlFindElement(Param, Param, "ID", NULL, NULL, MXML_DESCEND);
				assert(NULL != node);
				node = mxmlGetLastChild(node);
				assert(NULL != node);
				attr_value = mxmlGetText(node, &whitespace);
				assert(attr_value);
		        vnode->var_p.pn = atoi(attr_value);
				trace(TR_DEV_ETH,  "pn = %d\n", vnode->var_p.pn);

				node = mxmlFindElement(Param, Param, "DataType", NULL, NULL, MXML_DESCEND);
				assert(NULL != node);
				node = mxmlGetLastChild(node);
				assert(NULL != node);
				attr_value = mxmlGetText(node, &whitespace);
				assert(attr_value);
		        vnode->var_p.vt = get_value_type(attr_value);
				trace(TR_DEV_ETH,  "vt = %d\n", vnode->var_p.vt);

				node = mxmlFindElement(Param, Param, "RWType", NULL, NULL, MXML_DESCEND);
				assert(NULL != node);
				node = mxmlGetLastChild(node);
				assert(NULL != node);
				attr_value = mxmlGetText(node, &whitespace);
				assert(attr_value);
		        vnode->var_p.rw = get_access(attr_value);
				trace(TR_DEV_ETH,  "rw = %d\n", vnode->var_p.rw);

				node = mxmlFindElement(Param, Param, "RegAddr", NULL, NULL, MXML_DESCEND);
				assert(NULL != node);
				node = mxmlGetLastChild(node);
				assert(NULL != node);
				attr_value = mxmlGetText(node, &whitespace);
				assert(attr_value);
		        vnode->var_p.reg_addr = atoi(attr_value);
				trace(TR_DEV_ETH,  "reg_addr = %d\n", vnode->var_p.reg_addr);

				node = mxmlFindElement(Param, Param, "DataPrecision", NULL, NULL, MXML_DESCEND);
				assert(NULL != node);
				node = mxmlGetLastChild(node);
				assert(NULL != node);
				attr_value = mxmlGetText(node, &whitespace);
				assert(attr_value);
		        vnode->var_p.dataprecision = atof(attr_value);
				trace(TR_DEV_ETH,  "dataprecision = %f\n", vnode->var_p.dataprecision);

				node = mxmlFindElement(Param, Param, "AlarmEnable", NULL, NULL, MXML_DESCEND);
				assert(NULL != node);
				node = mxmlGetLastChild(node);
				assert(NULL != node);
				attr_value = mxmlGetText(node, &whitespace);
				assert(attr_value);
		        vnode->var_p.alarm_enable = (strstr(attr_value, "True") != NULL) ? 1 : 0;
				trace(TR_DEV_ETH,  "alarm_enable = %d\n", vnode->var_p.alarm_enable);

				vnode->var_p.wt = ALARM_TYPE_OLIMIT;

				node = mxmlFindElement(Param, Param, "AlarmUpper", NULL, NULL, MXML_DESCEND);
				assert(NULL != node);
				node = mxmlGetLastChild(node);
				assert(NULL != node);
				attr_value = mxmlGetText(node, &whitespace);
				assert(attr_value);
		        vnode->var_p.hvalue = atof(attr_value);
				trace(TR_DEV_ETH,  "hvalue = %f\n", vnode->var_p.hvalue);

				node = mxmlFindElement(Param, Param, "AlarmFloor", NULL, NULL, MXML_DESCEND);
				assert(NULL != node);
				node = mxmlGetLastChild(node);
				assert(NULL != node);
				attr_value = mxmlGetText(node, &whitespace);
				assert(attr_value);
		        vnode->var_p.lvalue = atof(attr_value);
				trace(TR_DEV_ETH,  "lvalue = %f\n", vnode->var_p.lvalue);

				node = mxmlFindElement(Param, Param, "SampleFrequency", NULL, NULL, MXML_DESCEND);
				assert(NULL != node);
				node = mxmlGetLastChild(node);
				assert(NULL != node);
				attr_value = mxmlGetText(node, &whitespace);
				assert(attr_value);
		        vnode->var_p.period = atoi(attr_value);
				vnode->d.lv = MAGIC_VALUE;
				trace(TR_DEV_ETH,  "period = %d\n", vnode->var_p.period);

				VAR_LIST_INSERT((&pnode->varlist), vnode, vtemp, reg_addr);

				Param = mxmlGetNextSibling(Param);	
			}

			DEV_LIST_INSERT((&mount_net->dev_eth_list), pnode, temp);
			
			Dev = mxmlGetNextSibling(Dev);
		}

		//创建挂载点数据采集线程
		net_mount_init(mount_net);
        
NextMountPoint:
		MountPoint = mxmlGetNextSibling(MountPoint);	
	}

	mxmlDelete(tree);
    return;
}

/*****************************************************************************
 * Function      : dev_eth_save
 * Description   : eth设备参数存储
 * Input         : void
 * Output        : None
 * Return        : status_t
 * Others        : 
 * Record
 * 1.Date        : 20160105
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
static status_t dev_eth_save( void )
{
    status_t ret = ERROR;
	char filename[100];
	char filename1[100];
	int			i;
	int          fd = -1;
	FILE	    *fp;
	mxml_node_t	*tree,
		*NetDev,
		*MountPoint,
    	*Dev,
		*Param,
    	*node;
	char buffer[100];
	mount_net_t *mount_net;

	tree = mxmlNewXML("1.0");
	assert(NULL != tree);

	NetDev = mxmlNewElement(tree, "NetDev");
	assert(NetDev);

    mount_net = net_mount_list.head;
	while(NULL != mount_net)
	{
		MountPoint = mxmlNewElement(NetDev, "MountPoint");
		assert(MountPoint);
		mxmlElementSetAttr(MountPoint, "Name", mount_net->net.name);
		mxmlElementSetAttr(MountPoint, "Protocal", mount_net->net.ptl_type);
		memset(buffer, 0x00, sizeof(buffer));
		inet_ntop(AF_INET, (void *)&mount_net->net.ip, buffer, 16);
		mxmlElementSetAttr(MountPoint, "IP", buffer);		
		sprintf(buffer, "%d", mount_net->net.port);
		mxmlElementSetAttr(MountPoint, "Port", buffer);
		
		dev_eth_t *pnode = mount_net->dev_eth_list.head;
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

			var_eth_t *vnode = pnode->varlist.head;
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
		mount_net = mount_net->next;
	}

	sprintf(filename, "%s%s", CFG_PATH, NetDev_xml);
	if(NULL == (fp = fopen(filename, "w+")))
	{
	    trace(TR_DEV_ETH,  "creat file %s failed\n", filename);
		mxmlDelete(tree);
		return ERROR;
	}

    ret = mxmlSaveFile(tree, fp, MXML_NO_CALLBACK);
    assert(-1 != ret);
	
    fclose(fp);
    trace(TR_DEV_ETH,  "save %s success\n", filename);
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
static void mb_single_var_read(mount_net_t *mount_net, dev_eth_t * pnode, var_eth_t *vnode )
{
    uint8 buffer[MB_SER_PDU_SIZE_MAX];
    struct mb_read_param mbp = {MB_CH_NETWORK, mount_net->sockfd, 
				(uint8_t)pnode->dev_p.addr, vnode->var_p.reg_addr, 1, buffer, 0};
	struct timeval tv;
	int ret;

    if(NULL == vnode)
		return;

    gettimeofday(&tv, NULL);
	trace(TR_DEV_ETH, "single_read sockfd[%d] DevType[%d] DevId[%d] addr[%d] pn[%d] regaddr[%d]\n", 
		mount_net->sockfd, pnode->dev_p.dev_type_id, pnode->dev_p.dev_id, pnode->dev_p.addr, vnode->var_p.pn, 
		vnode->var_p.reg_addr);
	if(0 == (ret = mb_read(&mbp)))
	{
	    trace_buf(TR_DEV_ETH_BUF, "single_recv :", mbp.recv_buf, mbp.recv_len);
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
            net_alarm(mount_net, pnode, vnode, value);
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
	    trace(TR_DEV_ETH, "read 485addr[%d] reg_addr[%d] failed[ret = 0x%08X]\n", 
	    	pnode->dev_p.addr, vnode->var_p.reg_addr, ret);
	    pnode->dev_stat.dumb_num ++;
		if(pnode->dev_stat.dumb_num >= pnode->dev_p.dumb_num)
		{
//				    pnode->dev_stat.dumb_num = 0;
            pnode->dev_stat.is_online = false;
		}
		vnode->d.lv = MAGIC_VALUE;
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
static void mb_multi_var_read(mount_net_t *mount_net, dev_eth_t *pnode )
{
	#define INVALID_ADDR	0xFFFF
	uint16 s_addr = INVALID_ADDR;
	uint16 l_addr = INVALID_ADDR;
	uint16 count = 0;
	uint32 period = 0;
	var_eth_t * vnode = NULL;
	var_eth_t *vsnode = NULL;
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
	                mb_single_var_read(mount_net, pnode, vsnode);
				
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
		    struct mb_read_param mbp = {MB_CH_NETWORK, mount_net->sockfd, 
				(uint8_t)pnode->dev_p.addr, s_addr, count, buffer, 0};
			uint32 slen, rlen;
            int ret;

            trace(TR_DEV_ETH, "multi_read sockfd[%d] DevTypeId[%d]DevId[%d]s_addr[%d]start_reg[%d]count[%d]\n",
            mount_net->sockfd, pnode->dev_p.dev_type_id, pnode->dev_p.dev_id, pnode->dev_p.addr, s_addr, count);
			if(0 == (ret = mb_read(&mbp)))
			{
				trace_buf(TR_DEV_ETH_BUF, "multi_recv :", mbp.recv_buf, mbp.recv_len);
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
							trace(TR_DEV_ETH, "value[%f]-pn[%d]-vt[%d]-dataprecision[%f] pack err\n", 
								value, vcfg.pn, vcfg.vt, vcfg.dataprecision);

                        //告警处理
                        net_alarm(mount_net, pnode, vsnode, value);
                    	
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
							trace(TR_DEV_ETH, "not thinfo dev \n");
					}
					else
#endif
						utm_rpt_send(appbuf);
					free(appbuf);
				}    
			}
			else
			{
			    trace(TR_DEV_ETH, "read 485addr[%d] s_addr[%d] count[%d] failed[ret = 0x%08X]\n", 
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
		    mb_single_var_read(mount_net, pnode, vnode);
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
 * Function      : dev_eth_read
 * Description   : dev_eth_read
 * Input         : void
 * Output        : None
 * Return        : void
 * Others        : 
 * Record
 * 1.Date        : 20151201
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
static void *dev_eth_read(mount_net_t * const mount_net)
{
    dev_eth_t *pnode = NULL;
	var_eth_list_t *vlist;
	var_eth_t * vnode;
	float32 value = 0.0;
	dev_eth_list_t * const plist = &mount_net->dev_eth_list;
	char buffer[20];

	prctl(PR_SET_NAME, mount_net->net.name);
	net_alarm_load(mount_net);
	trace(TR_DEV_ETH, "net thread %s start!\n", mount_net->net.name);
    FOREVER
	{
		usleep(10000);				    

        pthread_mutex_lock(&mount_net->net_mutex);
		
		if(OK != do_linkstat_check(mount_net))
		{
		    pthread_mutex_unlock(&mount_net->net_mutex);
			continue;
		}
		
	    pnode = plist->head;
	    while(NULL != pnode)
    	{
    	    vlist = &pnode->varlist;
			do
			{
			    if(NULL == vlist)
					break;
				
				//modbus协议
				if(PROTOCOL_MODBUS == pnode->dev_p.protocol_type)
					mb_multi_var_read(mount_net, pnode);
				
				//检查web请求标记，如果有请求主动让出资源
				if(true == mount_net->flagofdevparamrequest)
				{ 
					goto have_a_rest;
				}
			}while(0);
			
			pnode = pnode->next;
			//里面时间长了影响web的及时响应,打断一下			
		
    	}
have_a_rest:
		pthread_mutex_unlock(&mount_net->net_mutex);

	}
}

/*****************************************************************************
 * Function      : mnt_eth_mgr
 * Description   : eth串口参数管理
 * Input         : devmgr_t *req
 * Output        : None
 * Return        : opt_rst_e
 * Others        : 
 * Record
 * 1.Date        : 20151227
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
opt_rst_e mnt_eth_mgr( devmgr_t *req )
{
    opt_rst_e ret = OPT_OK;
	boolean is_changed = false;
	mount_net_t *mount_net;
	char *name = req->pin;
	
	if(OPT_TYPE_SET == req->optt)
	{
	    net_t *net = (net_t *)(req->pin + sizeof(net->name));

		mount_net = net_mount_find(net->name);
        //修改
		if(NULL != mount_net)
		{
		    if(memcmp((char*)net, (char*)&mount_net->net, sizeof(net_t)) != 0)
	    	{
	    	    pthread_mutex_lock(&mount_net->net_mutex);
	    	    memcpy((char*)&mount_net->net, (char*)net, sizeof(net_t));
				// 参数变化的对应处理
				mount_net->flagofnetparamchange = true;
				pthread_mutex_unlock(&mount_net->net_mutex);
				is_changed = true;
	    	}
		}
		//添加
		else
		{
		    mount_net = (mount_net_t *) calloc(1, sizeof(mount_net_t));
			LIST_INIT(&mount_net->dev_eth_list);
			memcpy((char*)&mount_net->net, (char*)net, sizeof(net_t));
			net_mount_insert(mount_net);
			//添加挂载点对应处理
			net_mount_init(mount_net);
			is_changed = true;
		}
	}
	else if(OPT_TYPE_GET == req->optt)
	{
		ret = OPT_ERR_ELSE;
	}
	else if(OPT_TYPE_DEL == req->optt)
	{
	    dev_eth_t *pnode = NULL;
		var_eth_t *vnode = NULL;
		dev_eth_list_t *plist;

        printf("del %s\n", name);
        if(NULL != (mount_net = net_mount_find(name)))
    	{
    	    pthread_mutex_lock(&mount_net->net_mutex);
    	    plist = &mount_net->dev_eth_list;
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
			net_mount_exit(mount_net);
			LIST_DEL(&net_mount_list, mount_net);
			free(mount_net);
			is_changed = true;
    	}
		else
			ret = OPT_ERR_NET_NEXIST;
	}

	if(true == is_changed)
		dev_eth_save();

	return ret;
}

/*****************************************************************************
 * Function      : dev_eth_mgr
 * Description   : eth设备管理
 * Input         : devmgr_t *req
 * Output        : None
 * Return        : opt_rst_e
 * Others        : 
 * Record
 * 1.Date        : 20151227
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
opt_rst_e dev_eth_mgr( devmgr_t *req )
{
    dev_header3_t *header = (dev_header3_t*)req->pin;
    opt_rst_e ret = OPT_OK;
	boolean is_changed = false;
	dev_eth_t *pnode = NULL, *temp;
	mount_net_t *mount_net;

	mount_net = net_mount_find(header->mount_name);
	if(NULL == mount_net)
		return OPT_ERR_NET_NEXIST;
	mount_net->flagofdevparamrequest = true;
    pthread_mutex_lock(&mount_net->net_mutex);
	pnode = dev_eth_list_find(&mount_net->dev_eth_list, header->dev_type_id, header->dev_id);
	
	if(OPT_TYPE_SET == req->optt)
	{
	    dev_eth_param_t *info = (dev_eth_param_t*)(((uint8*)req->pin) + sizeof(dev_header3_t));
        		
	    if((NULL != pnode) &&
			(memcmp((void*)info, (void*)&pnode->dev_p, sizeof(dev_eth_param_t)) != 0))
    	{
			memcpy((void*)&pnode->dev_p, (void*)info, sizeof(dev_eth_param_t));
			is_changed = true;
    	}
		else if(NULL == pnode)
		{
		    if(mount_net)
	    	{
			    pnode = (dev_eth_t *)calloc(1, sizeof(dev_eth_t));
				assert(NULL != pnode);
				memcpy((void*)&pnode->dev_p, (void*)info, sizeof(dev_eth_param_t));
				LIST_INIT((&pnode->varlist));
				DEV_LIST_INSERT((&mount_net->dev_eth_list), pnode, temp);
				is_changed = true;
	    	}
			else
				ret = OPT_ERR_NET_NEXIST;
		}
	}
	else if(OPT_TYPE_GET == req->optt)
	{	
		ret = OPT_ERR_ELSE;
	}
	else if(OPT_TYPE_DEL == req->optt)
	{		
	    if(NULL != pnode)
    	{
    	    var_eth_t *vnode = NULL;
			dev_eth_list_t *plist = &mount_net->dev_eth_list;
			var_eth_list_t *vlist = &pnode->varlist;
			
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
		dev_eth_save();

    pthread_mutex_unlock(&mount_net->net_mutex);
	mount_net->flagofdevparamrequest = false;

	return ret;
}

/*****************************************************************************
 * Function      : var_eth_mgr
 * Description   : eth设备变量管理
 * Input         : devmgr_t *req
 * Output        : None
 * Return        : opt_rst_e
 * Others        : 
 * Record
 * 1.Date        : 20151227
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
opt_rst_e var_eth_mgr( devmgr_t *req )
{
    var_header3_t *header = (var_header3_t*)req->pin;
    boolean is_changed = false;
	mount_net_t *mount_net;
	dev_eth_t *pnode = NULL;

	mount_net = net_mount_find(header->mount_name);
	if(NULL == mount_net)
		return OPT_ERR_NET_NEXIST;
	mount_net->flagofdevparamrequest = true;
    pthread_mutex_lock(&mount_net->net_mutex);
	pnode = dev_eth_list_find(&mount_net->dev_eth_list, header->dev_type_id, header->dev_id);
	if(NULL == pnode)
	{
	    pthread_mutex_unlock(&mount_net->net_mutex);
		return OPT_ERR_DEV_NEXIST;
	}

	opt_rst_e ret = OPT_OK;
	var_eth_t *vnode = NULL;

	vnode = var_eth_list_find(header->pn, &pnode->varlist);

	if(OPT_TYPE_SET == req->optt)
	{
	    var_eth_param_t *vinfo = (var_eth_param_t *)(((uint8*)req->pin) + sizeof(var_header3_t));
		var_eth_t *temp = NULL;
		
	    if((NULL != vnode) &&
			(memcmp((void*)vinfo, (void*)&vnode->var_p, sizeof(var_eth_param_t)) != 0))
    	{
			memcpy((void*)&vnode->var_p, (void*)vinfo, sizeof(var_eth_param_t));
			is_changed = true;
			//重新排序
			var_eth_list_t vlist;

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
		    vnode = (var_eth_t*)calloc(1, sizeof(var_eth_t));
			assert(vnode != NULL);
			memcpy((void*)&vnode->var_p, (void*)vinfo, sizeof(var_eth_param_t));
			vnode->d.lv = MAGIC_VALUE;
			VAR_LIST_INSERT((&pnode->varlist), vnode, temp, reg_addr);
			is_changed = true;
		}
	}
	else if(OPT_TYPE_GET == req->optt)
	{
	    if(NULL != vnode)
			memcpy(req->pout, (void*)&vnode->var_p, sizeof(var_eth_param_t));
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
		dev_eth_save();

	pthread_mutex_unlock(&mount_net->net_mutex);
	mount_net->flagofdevparamrequest = false;
	
	return ret;
}

/*****************************************************************************
 * Function      : read_eth_sd
 * Description   : 读取网络设备数据
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
utm_rst_e read_eth_sd( uint16 dev_type_id, uint8 dev_id, uint8* pack, uint32 *len )
{
    dev_eth_t *node;
	utm_rst_e ret = UTM_DEV_NOTCFG;;
	uint32 idx = 0;
	uint8 comn;
	var_eth_t *vnode;

    //设备查找
    mount_net_t *mount_net = net_mount_list.head;
    while(NULL != mount_net)
	{
	    mount_net->flagofdevparamrequest = true;
		pthread_mutex_lock(&mount_net->net_mutex);
	    node = mount_net->dev_eth_list.head;
		while(NULL != node)
		{
		    if((node->dev_p.dev_type_id == dev_type_id)
				&& (node->dev_p.dev_id == dev_id))
		    	break;

			node = node->next;
		}
		if(NULL == node)
		{
			pthread_mutex_unlock(&mount_net->net_mutex);
		    mount_net->flagofdevparamrequest = false;
			mount_net = mount_net->next;
		}
		else
			break;
	}
	
    if(node)
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
			var_cfg_t vcfg = {vnode->var_p.pn, vnode->var_p.vt, vnode->var_p.dataprecision};
			
			if(OK == pack_var_value(&pack[idx], vnode->d.lv, vcfg, &plen))
			    idx += plen;
			else
				trace(TR_DEV_ETH,  "value[%f]-pn[%d]-vt[%d]-dataprecision[%f] pack err\n", 
					vnode->d.lv, vcfg.pn, vcfg.vt, vcfg.dataprecision);
			vnode = vnode->next;
		}
		//消息正文中，单个设备数据长度不含设备ID
		pack[2] = (idx - 5) >> 8;
		pack[3] = (idx - 5) % 256;
		*len = idx;
		ret = UTM_OK;

		pthread_mutex_unlock(&mount_net->net_mutex);
	    mount_net->flagofdevparamrequest = false;
	}

	return ret;
}

/*****************************************************************************
 * Function      : ctrl_eth
 * Description   : eth的控制帧
 * Input         : uint8 *data
 * Output        : None
 * Return        : utm_rst_e
 * Others        : 
 * Record
 * 1.Date        : 20151230
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
utm_rst_e ctrl_eth( uint8 *data )
{
	uint8 comn;
    uint16 pn;
	uint32 idx = 5;
	var_eth_t *vnode = NULL;
	uint16 dev_type_id = (data[0] << 8) + data[1];
	uint16 len =  (data[2] << 8 ) + data[3];
	uint8 dev_id = data[4];
	dev_eth_t *node = NULL;
	var_eth_list_t *vlist = NULL;
	utm_rst_e ret = UTM_DEV_NOTCFG;

	//设备查找
    mount_net_t *mount_net = net_mount_list.head;
    while(NULL != mount_net)
	{
	    mount_net->flagofdevparamrequest = true;
		pthread_mutex_lock(&mount_net->net_mutex);
	    node = mount_net->dev_eth_list.head;
		while(NULL != node)
		{
		    if((node->dev_p.dev_type_id == dev_type_id)
				&& (node->dev_p.dev_id == dev_id))
		    	break;

			node = node->next;
		}
		if(NULL == node)
		{
			pthread_mutex_unlock(&mount_net->net_mutex);
		    mount_net->flagofdevparamrequest = false;
			mount_net = mount_net->next;
		}
		else
			break;
	}

	if(node)
	{
		vlist = &node->varlist;
		while(idx < (len + 5))
		{
		    pn = (data[idx] << 8) + data[idx + 1];
			idx += 2;
			trace(TR_DEV_ETH,  "pn = %d\n", pn);
			vnode = var_eth_list_find(pn, vlist);
			if(NULL == vnode)
			{
			    trace(TR_DEV_ETH,  "UTM_VAR_NOTCFG\n");
			    ret = UTM_VAR_NOTCFG;
				goto request_end;
			}
			//判断变量是否可写
			if((vnode->var_p.rw != ACCESS_W) && (vnode->var_p.rw != ACCESS_RW))
			{
			    trace(TR_DEV_ETH, "[%s]->[%s] can not write!!!!\n", 
					node->dev_p.dev_name, vnode->var_p.pname);
				continue;
			}
           
            int mbret;
			uint8 send_buf[2] = {0};
			uint32 plen = 0;
			float32 value;
			var_cfg_t vcfg = {pn, vnode->var_p.vt, vnode->var_p.dataprecision};
			struct mb_write_param param = {comn, node->dev_p.addr, vnode->var_p.reg_addr, 1, send_buf};
			
			if(OK == get_var_value(&data[idx], vcfg, &value, &plen))
			{
			    trace(TR_DEV_ETH,  "dev_type_id[%d]dev_id[%d]pn[%d] reg_addr[%d] value[%f]mb write \n", 
						dev_type_id, dev_id, pn, vnode->var_p.reg_addr, value);
			    hth_value_to_reg(value, vnode->var_p.dataprecision, send_buf);
				trace_buf(TR_DEV_ETH_BUF, "modbus send:", send_buf, 2);
				if(0 != (mbret = mb_write(&param)))
				{
					trace(TR_DEV_ETH,  "dev_type_id[%d]dev_id[%d]pn[%d] mb write err[mbret = 0x%08X]\n", 
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
				trace(TR_DEV_ETH,  "get_var_value err\n");
				break;
			}    
		}
		if(idx != (len + 5))
			ret = UTM_PACK_FMT_ERR;
		else
			ret = UTM_OK;
		
	request_end:
		pthread_mutex_unlock(&mount_net->net_mutex);
		    mount_net->flagofdevparamrequest = false;
	}

	return ret;
}

/*****************************************************************************
 * Function      : dev_eth_init
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
void dev_eth_init( void )
{
    int ret;
	pthread_attr_t attr;
	pthread_t tid;
	
    //加载配置文件
    dev_eth_load();

	ret = pthread_attr_init(&attr);
	assert(ret == 0);
	ret = pthread_attr_setstacksize(&attr, ETH_THREAD_STACK_SIZE);
	assert(ret == 0);
    ret = pthread_create(&tid, &attr, net_phy_check, NULL);
	assert(ret == 0);
	
}

