/********************************************************************************

 **** Copyright (C), 2015, BinHong tech Co., Ltd.                ****

 ********************************************************************************
 * File Name     : comm.c
 * Author        : chenxu
 * Date          : 2015-12-08
 * Version       : 1.0
 * Function List :
 * Description   : 与监控中心维持链接的相关处理
 * Record        :
 * 1.Date        : 2015-12-08
 *   Author      : chenxu
 *   Modification: Created file

 *******************************************************************************/

/*------------------------------------------------------------------------------
Section: Includes
------------------------------------------------------------------------------*/
#include "comm.h"
#include "alarm.h"
#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <fcntl.h>
#include <mqueue.h>
#include <sys/socket.h>
#include <sys/prctl.h>
#include <netinet/in.h>
#include <netdb.h>
#include "trace.h"
#include "gpio.h"
#include "utm.h"
#include "system.h"
#include "trace.h"
#include <linux/if.h> 
#include <linux/ethtool.h> 
#include <linux/sockios.h>
#include <net/route.h>
//#include <netpacket/packet.h>
//#include <net/ethernet.h>
//#include <sys/types.h>
//#include <netinet/if_ether.h>
/*------------------------------------------------------------------------------
Section: Macro Definitions
------------------------------------------------------------------------------*/
#define COMM_LOCK 				pthread_mutex_lock(&comm.comm_mutex)
#define COMM_UNLOCK  			pthread_mutex_unlock(&comm.comm_mutex)
/*心跳3次无确认视为掉线*/
#define MAX_FAIL_HB_COUNT		10
/*------------------------------------------------------------------------------
Section: Type Definitions
------------------------------------------------------------------------------*/
typedef struct
{
	int pin;
    int led_stat;
    pthread_mutex_t led_mutex;
}led_t;
/*------------------------------------------------------------------------------
Section: Globals
------------------------------------------------------------------------------*/
static comm_t comm;
static mqd_t msgq_id_r;
static mqd_t msgq_id_s;
static int sockfd = -1;
static unsigned long ticks_per_sec;
static led_t led[2];
pthread_t tid_comm;
/*------------------------------------------------------------------------------
Section: Function Prototypes
------------------------------------------------------------------------------*/
/*****************************************************************************
 * Function      : led_thread
 * Description   : led闪烁控制线程
 * Input         : void
 * Output        : None
 * Return        : void
 * Others        : 
 * Record
 * 1.Date        : 20160225
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
static void *led_thread( led_t * ledp )
{
    at91_gpio_arg arg = {ledp->pin, 0, 0};
	int fd;
	
	fd = open(DEV_GPIO, O_RDWR);
	assert(fd != -1);

    //翻转
    pthread_mutex_lock(&ledp->led_mutex);
	ledp->led_stat = ledp->led_stat ^ (0x01);
	arg.data = ledp->led_stat;
	ioctl(fd, IOCTL_GPIO_SETVALUE, &arg);
    pthread_mutex_unlock(&ledp->led_mutex);
	
    //延时50ms
    usleep(50 * 1000);
	
    //翻转
    pthread_mutex_lock(&ledp->led_mutex);
	ledp->led_stat = ledp->led_stat ^ (0x01);
	arg.data = ledp->led_stat;
	ioctl(fd, IOCTL_GPIO_SETVALUE, &arg);
    pthread_mutex_unlock(&ledp->led_mutex);

	close(fd);

	return(NULL);
}

/*****************************************************************************
 * Function      : led_comm2mc_hook
 * Description   : 控制器上行灯闪烁
 * Input         : void
 * Output        : None
 * Return        : void
 * Others        : 
 * Record
 * 1.Date        : 20160225
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
static void led_comm2mc_hook( int flag )
{
    int ret;
	int i;
    static boolean is_first_run = true;

	if((flag != 0) && (flag != 1))
		assert(0);

	if(is_first_run)
	{
		int fd;
		at91_gpio_arg arg;

		fd = open(DEV_GPIO, O_RDWR);
		assert(fd != -1);

		arg.pin = LED_UP_STREAM;
		arg.data = 0;
		arg.usepullup = 1;
		ioctl(fd, IOCTL_GPIO_SETOUTPUT, &arg);

		arg.pin = LED_DN_STREAM;
		ioctl(fd, IOCTL_GPIO_SETOUTPUT, &arg);
		
		close(fd);		

		for(i = 0; i < 2; i++)
		{
		    led[i].led_stat = 0;
			if(0 != pthread_mutex_init(&led[i].led_mutex, NULL))
				trace(TR_COMM, "create led_mutex failed\n");
		}

		led[0].pin = LED_UP_STREAM;
		led[1].pin = LED_DN_STREAM;

		is_first_run = false;
		
	}

    pthread_t tid;
	pthread_attr_t attr;

	ret = pthread_attr_init(&attr);
	assert(ret == 0);
	ret = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	assert(ret == 0);
	ret = pthread_attr_setstacksize(&attr, 1024 * 20);
	assert(ret == 0);
	ret = pthread_create(&tid, &attr,(void *) led_thread, (void*)&led[flag]);
	if(ret != 0)
		trace(TR_COMM, "Create pthread error[%d]!\n", ret);
}

/*****************************************************************************
 * Function      : is_netipvalid
 * Description   : IP地址合法性检查
 * Input         : addr_in_t IP
 * Output        : None
 * Return        : staitc boolean
 * Others        : 
 * Record
 * 1.Date        : 20160113
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
static boolean is_netipvalid( in_addr_t IP )
{
	int i;
	struct in_addr addr;
	addr.s_addr = IP;

	i = inet_addr(inet_ntoa(addr));

	if((i == 0)||(i == 0xffffffff))
		return FALSE;
	else
		return TRUE;
}

/*****************************************************************************
 * Function      : set_addr
 * Description   : IP地址配置
 * Input         : addr_in_t addr
                int flag
 * Output        : None
 * Return        : status_t
 * Others        : 
 * Record
 * 1.Date        : 20160112
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
static status_t set_addr( in_addr_t addr, int flag, uint8 ethn)
{
    struct ifreq ifr;
    struct sockaddr_in sin;
	struct in_addr test;
    int sockfd;

	test.s_addr = addr;
	if(!is_netipvalid(addr))
	{
	    trace(TR_COMM, "invalid IP[%s]!!!\n", inet_ntoa(test));
		return ERROR;
	}
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sockfd == -1)
	{
        trace(TR_COMM, "set_addr socket fail!!!\n");
        return ERROR;
    }

	if(0 == ethn)
	    snprintf(ifr.ifr_name, (sizeof(ifr.ifr_name) - 1), "%s", "eth0");
	else if(1 == ethn)
		snprintf(ifr.ifr_name, (sizeof(ifr.ifr_name) - 1), "%s", "eth1");
    else
	{
	    trace(TR_COMM, "ethn [%d] is invalid!!!\n", ethn);
        return ERROR;
	}
	/* Read interface flags */
	if (ioctl(sockfd, SIOCGIFFLAGS, &ifr) < 0)
		trace(TR_COMM,  "ifdown: shutdown \n");

    memset(&sin, 0, sizeof(struct sockaddr));

    sin.sin_family = AF_INET;

    sin.sin_addr.s_addr = addr;

    memcpy(&ifr.ifr_addr, &sin, sizeof(struct sockaddr));

	if(ioctl(sockfd, flag, &ifr) < 0)
	{
		trace(TR_COMM, "fail to set address [%s]. \n", inet_ntoa(test));
        close(sockfd);
		return ERROR;
	}

    close(sockfd);

    return OK;

}

/*****************************************************************************
 * Function      : set_gateway
 * Description   : 网关设置
 * Input          : None
 * Output        : None
 * Return        : uint8 ethn
 * Others        : 
 * Record
 * 1.Date        : 20160112
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
static status_t set_gateway( in_addr_t addr, uint8 ethn )
{
    int sockFd;
    struct sockaddr_in sockaddr;
    struct rtentry rt;
	int ret;

    sockFd = socket(AF_INET, SOCK_DGRAM, 0);

	if (sockFd < 0)
	{
		trace(TR_COMM, "set_gateway Socket create error.\n");
		return ERROR;
	}

    memset(&rt, 0, sizeof(struct rtentry));
#if 0
    memset(&sockaddr, 0, sizeof(struct sockaddr_in));

    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = 0;
	sockaddr.sin_addr.s_addr = addr;

	memcpy ( &rt.rt_gateway, &sockaddr, sizeof(struct sockaddr_in));

	((struct sockaddr_in *)&rt.rt_dst)->sin_family=AF_INET;

	((struct sockaddr_in *)&rt.rt_genmask)->sin_family=AF_INET;

	rt.rt_flags = RTF_GATEWAY;
#else
//    /* Delete existing defalt gateway */    
//    rt.rt_dst.sa_family = AF_INET; 
//    rt.rt_genmask.sa_family = AF_INET; 
//	rt.rt_flags = RTF_UP; 
//	if ((ret = ioctl(sockFd, SIOCDELRT, &rt)) < 0)
//	{
//		 trace(TR_COMM, "ioctl(SIOCDELRT) error[%d]\n", ret);
//		 close(sockFd);
//		 return ERROR;
//	}

	memset(&rt, 0, sizeof(struct rtentry));
    rt.rt_dst.sa_family = AF_INET; 
    ((struct sockaddr_in *)&rt.rt_gateway)->sin_family = AF_INET;    
	((struct sockaddr_in *)&rt.rt_gateway)->sin_addr.s_addr = addr; 

	((struct sockaddr_in *)&rt.rt_dst)->sin_family = AF_INET;
	
	((struct sockaddr_in *)&rt.rt_genmask)->sin_family = AF_INET;
    rt.rt_flags = RTF_GATEWAY;
#endif
	if ((ret = ioctl(sockFd, SIOCADDRT, &rt)) < 0)
	{
		struct in_addr inaddr;

		inaddr.s_addr = addr;
		trace(TR_COMM, "ioctl(SIOCADDRT) [%s] error[%d]\n", inet_ntoa(inaddr), ret);
		close(sockFd);
		return ERROR;
	}

	return OK;
}

/*****************************************************************************
 * Function      : do_comm_chkparamchange
 * Description   : 通信参数变更检查
 * Input         : void
 * Output        : None
 * Return        : void
 * Others        : 
 * Record
 * 1.Date        : 20160111
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
static void do_comm_chkparamchange( void )
{
    COMM_LOCK;
    if(comm.comm_info.flagofparamchange)
	{
	    //更新参数    
	    system_getcommparam(&comm.comm_param);
        trace(TR_COMM, "update comm param mcflag[%d].\n", comm.comm_param.mcflag);
		//配置本地网卡IP、掩码、网关
		set_addr(comm.comm_param.lan1_ip, SIOCSIFADDR, 0);
		set_addr(comm.comm_param.lan1_mask, SIOCSIFNETMASK, 0);
		set_gateway(comm.comm_param.lan1_gateway, 0);

		set_addr(comm.comm_param.lan2_ip, SIOCSIFADDR, 1);
		set_addr(comm.comm_param.lan2_mask, SIOCSIFNETMASK, 1);
		set_gateway(comm.comm_param.lan2_gateway, 1);

        if(LINK_STAT_LOGIN == comm.comm_info.link_stat)
	    	comm.login_callback(false);
	    comm.comm_info.link_stat = LINK_STAT_RESET;
		comm.comm_info.flagofparamchange = false;
	}
	COMM_UNLOCK;
}

/*****************************************************************************
 * Function      : link_init
 * Description   : void
 * Input          : None
 * Output        : None
 * Return        : status_t
 * Others        :
 * Record
 * 1.Date        : 20151209
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
static status_t do_comm_reset( void )
{
    struct sockaddr_in client_addr;
	long flag = 0;

	if(comm.comm_info.link_stat != LINK_STAT_RESET)
		return OK;
    
    if(-1 != sockfd)
	{
		close(sockfd);//删除socket
	    sockfd = -1;
	}

	if((comm.comm_param.mcflag == 0)
		|| (!is_netipvalid(comm.comm_param.mip)))
		return ERROR;

	char mode[100];
	system_getcommod(mode);
	if(strcmp(mode, "WebService") == 0)
		return ERROR;
	
	comm.comm_info.login_stp.tv_sec = 0;
	comm.comm_info.login_stp.tv_usec = 0;
//	comm.comm_info.lst_hbi_tick = times(NULL);
//	comm.comm_info.lst_sign_tick = times(NULL);
    gettimeofday(&comm.comm_info.lst_hbi_tick, NULL);
	gettimeofday(&comm.comm_info.lst_sign_tick, NULL);
	comm.comm_info.noofhblost = 0;
	comm.comm_info.link_stat = LINK_STAT_WAIT_PHY;
	comm.comm_info.isbakserver = false;
	comm.comm_info.slink_time = 0;
	comm.login_callback(false);
	trace(TR_COMM, "do_comm_reset succ\n");

	return OK;
}

/*****************************************************************************
 * Function      : do_comm_chkphy
 * Description   : 检查网线是否链接
 * Input         : void
 * Output        : None
 * Return        : status_t
 * Others        :
 * Record
 * 1.Date        : 20151209
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
static status_t do_comm_chkphy( void )
{
    static status_t phydy = ERROR;
	static time_t ltime = 0;
	time_t ctime = time(NULL);

	if(comm.comm_info.link_stat < LINK_STAT_WAIT_PHY)
		return OK;
	
    if(ctime == ltime)
		return phydy;
	ltime = ctime;

	int skfd; 
	struct ifreq ifr; 
	struct ethtool_value edata; 
	
	edata.cmd = ETHTOOL_GLINK; 
	edata.data = 0; 
	
	memset(&ifr, 0, sizeof(ifr));
	if(comm.comm_param.mcflag == 2)
		strcpy(ifr.ifr_name, "eth1"); 
	else
		strcpy(ifr.ifr_name, "eth0"); 
	ifr.ifr_data = (char *) &edata; 
	
	if(( skfd = socket( AF_INET, SOCK_DGRAM, 0 )) < 0) 
	{
	    phydy = ERROR;
		return phydy;
	}
	
	if(ioctl( skfd, SIOCETHTOOL, &ifr ) == -1) 
		phydy = ERROR;
	else
	{
		phydy = (edata.data > 0) ? OK : ERROR;
	}
	close(skfd); 

    if(phydy != OK)
	{	    
	    if(comm.comm_info.link_stat == LINK_STAT_LOGIN)
			comm.login_callback(false);
		
		if(comm.comm_info.link_stat > LINK_STAT_CSOCKET)
		{
			comm.comm_info.link_stat = LINK_STAT_RESET;
			trace(TR_COMM, "do_comm_chkphy Phy[%d] is not link!!!\n", comm.comm_param.mcflag);	    
		}
	}
	else if(comm.comm_info.link_stat == LINK_STAT_WAIT_PHY)
	{
		trace(TR_COMM, "do_comm_chkphy Phy[%d] ok\n", comm.comm_param.mcflag);
		comm.comm_info.link_stat = LINK_STAT_CSOCKET;
	}
	return phydy;
}

/*****************************************************************************
 * Function      : do_comm_socket
 * Description   : 创建套接字
 * Input         : void
 * Output        : None
 * Return        : static status_t
 * Others        : 
 * Record
 * 1.Date        : 20160108
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
static status_t do_comm_socket( void )
{
    if(comm.comm_info.link_stat != LINK_STAT_CSOCKET)
		return OK;
	
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0)
	{
	    trace(TR_COMM, "open socket err\n");
		return ERROR;
	}

	fcntl(sockfd, F_SETFL, O_NONBLOCK);
    int flags = fcntl(sockfd, F_GETFL, 0);
    fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);

	comm.comm_info.link_stat = LINK_STAT_CONNECT;
	trace(TR_COMM, "do_comm_socket ok\n");

	return OK;
}

/*****************************************************************************
 * Function      : do_comm_connect
 * Description   : 与服务器建立连接
 * Input         : void
 * Output        : None
 * Return        : status_t
 * Others        :
 * Record
 * 1.Date        : 20151209
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
static status_t do_comm_connect( void )
{
    struct sockaddr_in server_addr;
	static time_t ltime = 0;
	time_t ctime = time(NULL);

    if(comm.comm_info.link_stat != LINK_STAT_CONNECT)
		return OK;

    if(ctime == ltime)
		return ERROR;
	ltime = ctime;
    if(!comm.comm_info.slink_time)
		comm.comm_info.slink_time = ctime;
    
    //切换判断
    if(abs(ctime - comm.comm_info.slink_time) > comm.comm_param.lk_time_out)
	{
	    comm.comm_info.isbakserver = comm.comm_info.isbakserver ? false : true;
		comm.comm_info.slink_time = ctime;
	}
	
	bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    if(comm.comm_info.isbakserver)
	{
		server_addr.sin_port = htons(comm.comm_param.bport);
		server_addr.sin_addr.s_addr = comm.comm_param.bip;		
	}
	else
	{
		server_addr.sin_port = htons(comm.comm_param.mport);
		server_addr.sin_addr.s_addr = comm.comm_param.mip;	
	}

    if(connect(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0)
	{
		trace(TR_COMM, "do_comm_connect error! ip[%s], port[%d]\n",
				inet_ntoa(server_addr.sin_addr), ntohs(server_addr.sin_port));
		return ERROR;
	}

    comm.comm_info.slink_time = 0;
	gettimeofday(&comm.comm_info.login_stp, NULL);
	comm.comm_info.link_stat = LINK_STAT_LOGIN;
	comm.login_callback(true);
	trace(TR_COMM, "do_comm_connect ok\n");

    return OK;
}

/*****************************************************************************
 * Function      : do_comm_idlechk
 * Description   : 掉线检查
 * Input         : void
 * Output        : None
 * Return        : status_t
 * Others        :
 * Record
 * 1.Date        : 20151209
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
static status_t do_comm_idlechk( void )
{
    int ret;

    if(comm.comm_info.link_stat != LINK_STAT_LOGIN)
		return ERROR;
	
    if(0 == (ret = recv(sockfd, comm.pbuf, sizeof(comm.pbuf), MSG_PEEK)))
	{
	    comm.comm_info.link_stat = LINK_STAT_RESET;
		comm.login_callback(false);
	    trace(TR_COMM, "socket break\n");
		return ERROR;
	}
		
    //心跳3次无确认,判为掉线
    if(comm.comm_info.noofhblost >= MAX_FAIL_HB_COUNT)
	{
		comm.comm_info.link_stat = LINK_STAT_RESET;
		comm.login_callback(false);
		comm.comm_info.noofhblost = 0;
		return ERROR;
	}

	return OK;
}

/*****************************************************************************
 * Function      : do_comm_heartbeat
 * Description   : 发送心跳
 * Input         : void
 * Output        : None
 * Return        : void
 * Others        :
 * Record
 * 1.Date        : 20151222
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
static void do_comm_heartbeat( void )
{
    //心跳
//	clock_t tick = times(NULL);
//	clock_t overtime = comm.comm_param.secofhbi * ticks_per_sec;

	struct timeval tv;
	uint32 overtimeofms = comm.comm_param.secofhbi * 1000;

	gettimeofday(&tv, NULL);
	if((overtimeofms > 0) &&
		(abs((tv.tv_sec - comm.comm_info.lst_hbi_tick.tv_sec) * 1000 + 
		(tv.tv_usec - comm.comm_info.lst_hbi_tick.tv_usec) / 1000) > overtimeofms))
	{
	    ptl_rst_t ptl;
		uint32 len;
		uint32 idx = 0;
		uint32 sendlen = 0;
		utm_param_t utmp;

        system_getutmparam(&utmp);
        memset((void *)&ptl, 0x00, sizeof(ptl_rst_t));
		ptl.frm_ctg = FRMCTG_HEATBEAT;
		ptl.src_addr = utmp.su_addr;
		ptl.dest_addr = utmp.mc_addr;
		ptl.datalen = 0;
		ptl.data = NULL;
		ptl.msg_id = utm_get_pfc();
		ptl.stp = time(NULL);
		if(PTL_OK == ptl_pack(&ptl, comm.pbuf, &len))
		{
		    trace(TR_COMM, "heart beat pack: \n");
		    trace_buf(TR_COMM_BUF, "", comm.pbuf, len);
		    while(idx < len)
			{
			    sendlen = send(sockfd, &comm.pbuf[idx], (len - idx), MSG_WAITALL);
				if(sendlen > 0)
					idx += sendlen;
			}
//			comm.comm_info.lst_hbi_tick = tick;
            comm.comm_info.lst_hbi_tick = tv;
			comm.comm_info.noofhblost++;
			led_comm2mc_hook(0);
		}
	}
}

/*****************************************************************************
 * Function      : do_comm_sign
 * Description   : 签到
 * Input         : void
 * Output        : None
 * Return        : static void
 * Others        :
 * Record
 * 1.Date        : 20151230
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
static void do_comm_sign( void )
{
    //签到
//	clock_t tick = times(NULL);
//	clock_t overtime = comm.comm_param.secofsign * ticks_per_sec;

//    trace(TR_COMM, "overtime[%ld] tick[%ld] ltick[%ld] tick-ltick[%ld]\n", 
//		overtime, tick, comm.comm_info.lst_sign_tick, (tick - comm.comm_info.lst_sign_tick));
//	if((overtime > 0) &&
//		((tick - comm.comm_info.lst_sign_tick) > overtime))
    struct timeval tv;
	uint32 overtimeofms = comm.comm_param.secofsign * 1000;

	gettimeofday(&tv, NULL);
	if((overtimeofms > 0) &&
		(abs((tv.tv_sec - comm.comm_info.lst_sign_tick.tv_sec) * 1000 + 
		(tv.tv_usec - comm.comm_info.lst_sign_tick.tv_usec) / 1000) > overtimeofms))
	{
	    ptl_rst_t ptl;
		uint32 len;
		uint32 idx = 0;
		uint32 sendlen = 0;
		utm_param_t utmp;

        system_getutmparam(&utmp);
        memset((void *)&ptl, 0x00, sizeof(ptl_rst_t));
		ptl.frm_ctg = FRMCTG_SIGN;
		ptl.src_addr = utmp.su_addr;
		ptl.dest_addr = utmp.mc_addr;
		ptl.datalen = 0;
		ptl.data = NULL;
		ptl.msg_id = utm_get_pfc();
		ptl.stp = time(NULL);
		if(PTL_OK == ptl_pack(&ptl, comm.pbuf, &len))
		{
		    trace(TR_COMM, "sign pack: \n");
		    trace_buf(TR_COMM_BUF, "", comm.pbuf, len);		  
		    while(idx < len)
			{
			    sendlen = send(sockfd, &comm.pbuf[idx], (len - idx), MSG_WAITALL);
				if(sendlen > 0)
					idx += sendlen;
			}
//			comm.comm_info.lst_sign_tick = tick;
            comm.comm_info.lst_sign_tick = tv;
			led_comm2mc_hook(0);
		}
	}
}


/*****************************************************************************
 * Function      : do_comm_trans
 * Description   : 数据收发
 * Input         : void
 * Output        : None
 * Return        : void
 * Others        :
 * Record
 * 1.Date        : 20151209
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
static void do_comm_trans( void )
{
    int rdlen = 0;
	int rcvlen = 0;

    if(0 < (rdlen = recv(sockfd, comm.pbuf, sizeof(comm.pbuf), MSG_DONTWAIT)))
	{
	    appbuf_t * appbuf = NULL;

		appbuf = (appbuf_t *)malloc(sizeof(appbuf_t));
		if(NULL == appbuf)
		{
		    trace(TR_COMM, "malloc appbuf err\n");
			return ;
		}
		appbuf->buftyp = APPBUF_MC;
		appbuf->len = rdlen;
		memcpy(appbuf->data, comm.pbuf, rdlen);
		if(mq_send(msgq_id_s, (char*)appbuf, sizeof(appbuf_t), 1) == -1)
            trace(TR_COMM, "mq_send err\n");
		free(appbuf);
		led_comm2mc_hook(1);
	}

	if((rcvlen = mq_receive(msgq_id_r, (char*)comm.pbuf, 8192, NULL)) > 0)
	{
        uint32 idx = 0, sendlen;

        led_comm2mc_hook(0);
		trace(TR_COMM, "send pack length[%d]\n", rcvlen);
		trace_buf(TR_COMM_BUF, "", comm.pbuf, rcvlen);
		while(idx < rcvlen)
		{
		    sendlen = send(sockfd, &comm.pbuf[idx], (rcvlen - idx), MSG_WAITALL);
			if(sendlen > 0)
				idx += sendlen;
		}
	}

	do_comm_heartbeat();

	do_comm_sign();
}

/*****************************************************************************
 * Function      : cb_recvhbpkg
 * Description   : 接收心跳确认报文回调函数
 * Input         : boolean recv
 * Output        : None
 * Return        : void
 * Others        :
 * Record
 * 1.Date        : 20151222
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
static void cb_recvhbpkg( boolean recv )
{
    COMM_LOCK;
	if (recv)
    {
        comm.comm_info.noofhblost = 0;
        trace(TR_COMM, "Send Heartbeat Response ACK!\r\n");
    }
    else
    {
        comm.comm_info.noofhblost++;
        trace(TR_COMM, "Send Heartbeat Response NAK!\r\n");
    }
	COMM_UNLOCK;
}

/*****************************************************************************
 * Function      : comm_loop
 * Description   : 链接维护线程处理
 * Input          : None
 * Output        : None
 * Return        :
 * Others        :
 * Record
 * 1.Date        : 20151208
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
static void comm_loop( void )
{

    prctl(PR_SET_NAME, COMM_THREAD_NAME);
	
    FOREVER
	{
	     usleep(1000);

         //参数变更检查
         do_comm_chkparamchange();
		 
		 /*复位操作*/
		 if(OK != do_comm_reset())
		 	continue;

		 /*网线的检测，如果检测到插入网线则尝试连接主站*/
		 if(OK != do_comm_chkphy())
			continue;

		 if(OK != do_comm_socket())
		 	continue;

		 /*建立连接*/
		 if(OK != do_comm_connect())
		 	continue;

		 /*掉线检查*/
		if(OK != do_comm_idlechk())
			continue;

		 /*数据收发作业*/
		 do_comm_trans();
	}
}

/*****************************************************************************
 * Function      : comm_send
 * Description   : 数据发送
 * Input         : uint8 *data
				 uint32 len
 * Output        : None
 * Return        : void
 * Others        :
 * Record
 * 1.Date        : 20151208
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
void comm_send( uint8 *data, uint32 len)
{
    if(LINK_STAT_LOGIN == comm.comm_info.link_stat)
	{
	    trace_buf(TR_COMM_BUF, "comm send:", data, len);
	    if(-1 == mq_send(msgq_id_r, data, len, 1))
			trace(TR_COMM, "send msg failed\n");
	}
}

/*****************************************************************************
 * Function      : comm_recv
 * Description   : 接收数据
 * Input         : void *data
 * Output        : None
 * Return        : uint32
 * Others        :
 * Record
 * 1.Date        : 20151208
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
int32 comm_recv( void *data )
{
    int32 len = 0;

	len = (int32) mq_receive(msgq_id_s, data, 8192, NULL);
	if(len > 0)
	    trace(TR_COMM, "comm_recv len[%d]\n", len);

	return len;
}

/*****************************************************************************
 * Function      : comm_notifyofparamchange
 * Description   : void
 * Input          : None
 * Output        : None
 * Return        : void
 * Others        : 
 * Record
 * 1.Date        : 20160111
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
void comm_notifyofparamchange( void )
{
    COMM_LOCK;
	comm.comm_info.flagofparamchange = true;
	COMM_UNLOCK;
}

/*****************************************************************************
 * Function      : comm_init
 * Description   :
 * Input          : None
 * Output        : None
 * Return        : void
 * Others        :
 * Record
 * 1.Date        : 20151208
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
void comm_init( void )
{
    int ret;
	struct mq_attr msgq_attr;

	//创建信号量
	if(0 != pthread_mutex_init(&comm.comm_mutex, NULL))
		trace(TR_COMM, "create dev_mutex failed\n");

	//创建消息队列
	mq_unlink(MSGQ_COMM_SEND_NAME);
	msgq_id_s = mq_open(MSGQ_COMM_SEND_NAME, O_RDWR | O_CREAT, S_IRWXU | S_IRWXG, NULL);
    if(msgq_id_s == (mqd_t)-1)
    {
        trace(TR_COMM, "mq_open err \n");
        return;
    }

	if(mq_getattr(msgq_id_s, &msgq_attr) == -1)
        trace(TR_COMM, "mq_getattr");
	//此处可以修改msgq_attr的相关属性
	msgq_attr.mq_flags = O_NONBLOCK;

    trace(TR_COMM, "Queue \"%s\":\n\t- stores at most %ld messages\n\t- \
        large at most %ld bytes each\n\t- currently holds %ld messages\n\t- mq_flags[%08x]\n",
        MSGQ_COMM_SEND_NAME, msgq_attr.mq_maxmsg, msgq_attr.mq_msgsize, msgq_attr.mq_curmsgs,msgq_attr.mq_flags);

	if(mq_setattr(msgq_id_s, &msgq_attr, NULL) == -1)
        trace(TR_COMM, "mq_setattr");

	mq_unlink(MSGQ_COMM_RECV_NAME);
	msgq_id_r = mq_open(MSGQ_COMM_RECV_NAME, O_RDWR | O_CREAT, S_IRWXU | S_IRWXG, NULL);
    if(msgq_id_r == (mqd_t)-1)
    {
        trace(TR_COMM, "mq_open err \n");
        return;
    }

	if(mq_getattr(msgq_id_r, &msgq_attr) == -1)
        trace(TR_COMM, "mq_getattr");
	//此处可以修改msgq_attr的相关属性
	msgq_attr.mq_flags = O_NONBLOCK;

    trace(TR_COMM, "Queue \"%s\":\n\t- stores at most %ld messages\n\t- \
        large at most %ld bytes each\n\t- currently holds %ld messages\n\t- mq_flags[%08x]\n",
        MSGQ_COMM_RECV_NAME, msgq_attr.mq_maxmsg, msgq_attr.mq_msgsize, msgq_attr.mq_curmsgs,msgq_attr.mq_flags);

	if(mq_setattr(msgq_id_r, &msgq_attr, NULL) == -1)
        trace(TR_COMM, "mq_setattr");

	//创建链接维护线程
	pthread_attr_t attr;

	ret = pthread_attr_init(&attr);
	assert(ret == 0);
	ret = pthread_attr_setstacksize(&attr, COMM_THREAD_STACK_SIZE);
	assert(ret == 0);
	ret = pthread_create(&tid_comm, &attr,(void *) comm_loop,NULL);
	if(ret != 0)
		trace(TR_COMM, "Create pthread error[%d]!\n", ret);

	comm.comm_info.flagofparamchange = true;
	ticks_per_sec = sysconf(_SC_CLK_TCK);

	utm_cb_register(CB_HB_CALL, cb_recvhbpkg);

	comm.login_callback = alarm_hook;
}


