/********************************************************************************

 **** Copyright (C), 2015, BinHong tech Co., Ltd.                ****

 ********************************************************************************
 * File Name     : system.c
 * Author        : chenxu
 * Date          : 2015-12-13
 * Version       : 1.0
 * Function List :
 * Description   : 系统配置处理
 * Record        :
 * 1.Date        : 2015-12-13
 *   Author      : chenxu
 *   Modification: Created file

 *******************************************************************************/

/*------------------------------------------------------------------------------
Section: Includes
------------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "atypes.h"
#include "maths.h"
#include <fcntl.h>
#include <pthread.h>
#include "system.h"
#include "mxml.h"
#include "trace.h"
/*------------------------------------------------------------------------------
Section: Macro Definitions
------------------------------------------------------------------------------*/
#define SYSPRINTF(x...) \
    {    \
        (void)printf("[sys]L[%d]%s()\r\t\t\t\t", __LINE__, __FUNCTION__); \
        (void)printf(x);    \
    }
#define SYSTEM_XML
#define SYSTEM_XML_FILE				"/usr/httproot/cfg/System.xml"
#define SYSTEM_LOCK 		pthread_mutex_lock(&system_mutex)
#define SYSTEM_UNLOCK		pthread_mutex_unlock(&system_mutex)
/*------------------------------------------------------------------------------
Section: Type Definitions
------------------------------------------------------------------------------*/
/* None */
/*------------------------------------------------------------------------------
Section: Globals
------------------------------------------------------------------------------*/
static sys_param_t system_param;
static exp_param_t sysexp;
static pthread_mutex_t system_mutex;			//互斥信号量
static char user[USER_LEN_MAX];		//用户名
static char passwd[PASSWD_LEN_MAX];	//密码
/*------------------------------------------------------------------------------
Section: Function Prototypes
------------------------------------------------------------------------------*/
/*****************************************************************************
 * Function      : system_save
 * Description   : save system config file
 * Input         : void
 * Output        : None
 * Return        : status_t
 * Others        : 
 * Record
 * 1.Date        : 20160110
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
static status_t system_save( void )
{
#ifdef SYSTEM_XML
    status_t ret = ERROR;
	char filename[100];
	char filename1[100];
	char buffer[100];
	int			i;
	int          fd = -1;
	FILE	    *fp;
	mxml_node_t	*tree, 
		*Node,
		*User,
		*Lan1,
		*Lan2,
		*DNS1,
		*Exp,
		*MainServer,
		*BackupServer,
		*ReplaceTime,
		*SignInCycle,
		*HeartBeatCycle,
		*node;

	tree = mxmlNewXML("1.0");
	assert(tree);
	Node = mxmlNewElement(tree, "Node");
	assert(Node);
	
	User = mxmlNewElement(Node, "User");
	assert(User);

	node = mxmlNewElement(User, "UserName");
	assert(node);
	node = mxmlNewText(node, 0, user);
	assert(node);

	node = mxmlNewElement(User, "UserPws");
	assert(node);
	node = mxmlNewText(node, 0, passwd);
	assert(node);

    //Lan1
	Lan1 = mxmlNewElement(Node, "Lan1");
	assert(Lan1);
	
	node = mxmlNewElement(Lan1, "IP");
	assert(node);
	inet_ntop(AF_INET, (void *)&system_param.comp.lan1_ip, buffer, 16);
	buffer[16] = 0;
	node = mxmlNewText(node, 0, buffer);
	assert(node);

	node = mxmlNewElement(Lan1, "Mask");
	assert(node);
	inet_ntop(AF_INET, (void *)&system_param.comp.lan1_mask, buffer, 16);
	buffer[16] = 0;
	node = mxmlNewText(node, 0, buffer);
	assert(node);

	node = mxmlNewElement(Lan1, "GateWay");
	assert(node);
	inet_ntop(AF_INET, (void *)&system_param.comp.lan1_gateway, buffer, 16);
	buffer[16] = 0;
	node = mxmlNewText(node, 0, buffer);
	assert(node);

	node = mxmlNewElement(Lan1, "MCFlag");
	assert(node);
	node = mxmlNewText(node, 0, (system_param.comp.mcflag == 1) ? "True" : "False");
	assert(node);

	//Lan2
	Lan2 = mxmlNewElement(Node, "Lan2");
	assert(Lan2);
	
	node = mxmlNewElement(Lan2, "IP");
	assert(node);
	inet_ntop(AF_INET, (void *)&system_param.comp.lan2_ip, buffer, 16);
	buffer[16] = 0;
	node = mxmlNewText(node, 0, buffer);
	assert(node);

	node = mxmlNewElement(Lan2, "Mask");
	assert(node);
	inet_ntop(AF_INET, (void *)&system_param.comp.lan2_mask, buffer, 16);
	buffer[16] = 0;
	node = mxmlNewText(node, 0, buffer);
	assert(node);

	node = mxmlNewElement(Lan2, "GateWay");
	assert(node);
	inet_ntop(AF_INET, (void *)&system_param.comp.lan2_gateway, buffer, 16);
	buffer[16] = 0;
	node = mxmlNewText(node, 0, buffer);
	assert(node);

	node = mxmlNewElement(Lan2, "MCFlag");
	assert(node);
	node = mxmlNewText(node, 0, (system_param.comp.mcflag == 2) ? "True" : "False");
	assert(node);
	
#ifdef DNS_CFG
	DNS1 = mxmlNewElement(Node, "DNS1");
	assert(DNS1);
	inet_ntop(AF_INET, (void *)&system_param.dns1, buffer, 16);
	buffer[16] = 0;
	node = mxmlNewText(DNS1, 0, buffer);
	assert(node);
#endif

	//MainServer
	MainServer = mxmlNewElement(Node, "MainServer");
	assert(MainServer);
    
	node = mxmlNewElement(MainServer, "IP");
	assert(node);
	inet_ntop(AF_INET, (void *)&system_param.comp.mip, buffer, 16);
	buffer[16] = 0;
	node = mxmlNewText(node, 0, buffer);
	assert(node);

//	node = mxmlNewElement(MainServer, "Mask");
//	assert(node);
//	inet_ntop(AF_INET, (void *)&system_param.comp.mmask, buffer, 16);
//	buffer[16] = 0;
//	node = mxmlNewText(node, 0, buffer);
//	assert(node);

//    node = mxmlNewElement(MainServer, "GateWay");
//	assert(node);
//	inet_ntop(AF_INET, (void *)&system_param.comp.mgateway, buffer, 16);
//	buffer[16] = 0;
//	node = mxmlNewText(node, 0, buffer);
//	assert(node);

	node = mxmlNewElement(MainServer, "Port");
	assert(node);
	node = mxmlNewInteger(node, system_param.comp.mport);
	assert(node);

	//BackupServer
	MainServer = mxmlNewElement(Node, "BackupServer");
	assert(MainServer);
    
	node = mxmlNewElement(MainServer, "IP");
	assert(node);
	inet_ntop(AF_INET, (void *)&system_param.comp.bip, buffer, 16);
	buffer[16] = 0;
	node = mxmlNewText(node, 0, buffer);
	assert(node);

//	node = mxmlNewElement(MainServer, "Mask");
//	assert(node);
//	inet_ntop(AF_INET, (void *)&system_param.comp.bmask, buffer, 16);
//	buffer[16] = 0;
//	node = mxmlNewText(node, 0, buffer);
//	assert(node);

//    node = mxmlNewElement(MainServer, "GateWay");
//	assert(node);
//	inet_ntop(AF_INET, (void *)&system_param.comp.bgateway, buffer, 16);
//	buffer[16] = 0;
//	node = mxmlNewText(node, 0, buffer);
//	assert(node);

	node = mxmlNewElement(MainServer, "Port");
	assert(node);
	node = mxmlNewInteger(node, system_param.comp.bport);
	assert(node);

	//服务器切换时间
	node = mxmlNewElement(Node, "ReplaceTime");
	assert(node);
	node = mxmlNewInteger(node, system_param.comp.lk_time_out);
	assert(node);

	//签到周期
    node = mxmlNewElement(Node, "SignInCycle");
	assert(node);
	node = mxmlNewInteger(node, system_param.comp.secofsign);
	assert(node);

	//监控中心地址
	node = mxmlNewElement(Node, "McAddr");
	assert(node);
	sprintf(buffer, "%02d%02d%02d%04d", BCD2HEX(system_param.utmp.mc_addr.dev_type),
		BCD2HEX(system_param.utmp.mc_addr.year),
		BCD2HEX(system_param.utmp.mc_addr.mon),
		BCD2HEX(system_param.utmp.mc_addr.fac_id[0]) * 100 +
		BCD2HEX(system_param.utmp.mc_addr.fac_id[1]));
		node = mxmlNewText(node, 0, buffer);
	assert(node);

    //监控中心地址
	node = mxmlNewElement(Node, "SuAddr");
	assert(node);
	sprintf(buffer, "%02d%02d%02d%04d", BCD2HEX(system_param.utmp.su_addr.dev_type),
		BCD2HEX(system_param.utmp.su_addr.year),
		BCD2HEX(system_param.utmp.su_addr.mon),
		BCD2HEX(system_param.utmp.su_addr.fac_id[0]) * 100 +
		BCD2HEX(system_param.utmp.su_addr.fac_id[1]));
		node = mxmlNewText(node, 0, buffer);
	assert(node);

    
	//心跳周期
	node = mxmlNewElement(Node, "HeartBeatCycle");
	assert(node);
	node = mxmlNewInteger(node, system_param.comp.secofhbi);
	assert(node);

	//扩展部分
	Exp = mxmlNewElement(Node, "Exp");
	assert(Exp);

	node = mxmlNewElement(Exp, "CommMod");
	assert(node);
    if('\0' != sysexp.comm_mod[sizeof(sysexp.comm_mod) - 1])
		sysexp.comm_mod[sizeof(sysexp.comm_mod) - 1] = 0;
	node = mxmlNewText(node, 0, sysexp.comm_mod);
	assert(node);

    node = mxmlNewElement(Exp, "WsdlAddr");
	assert(node);
    if('\0' != sysexp.wsdlp.wsdl_addr[sizeof(sysexp.wsdlp.wsdl_addr) - 1])
		sysexp.wsdlp.wsdl_addr[sizeof(sysexp.wsdlp.wsdl_addr) - 1] = 0;
	node = mxmlNewText(node, 0, sysexp.wsdlp.wsdl_addr);
	assert(node);

	node = mxmlNewElement(Exp, "PreAddr");
	assert(node);
	memset(buffer, 0x00, sizeof(buffer));
	memcpy(buffer, sysexp.wsdlp.pre_addr, sizeof(sysexp.wsdlp.pre_addr));
	node = mxmlNewText(node, 0, buffer);
	assert(node);

	if(NULL == (fp = fopen(SYSTEM_XML_FILE, "w+")))
		goto Error;

    ret = mxmlSaveFile(tree, fp, MXML_NO_CALLBACK);
    assert(-1 != ret);
	fclose(fp);
	ret =  OK;

Error:
    mxmlDelete(tree);
	return ret;
#endif
}

/*****************************************************************************
 * Function      : load_sys_cfg
 * Description   : 从配置文件加载系统配置信息
 * Input         : void
 * Output        : None
 * Return        : static void 
 * Others        : 
 * Record
 * 1.Date        : 20151213
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
static void system_load( void )
{
#ifdef SYSTEM_XML
    status_t ret = ERROR;
	int whitespace;
	char buffer[20];
	const char * attr_value;
	FILE	    *fp;
	mxml_node_t	*tree, 
		*Node,
		*User,
		*UserName,
		*UserPws,
		*Lan1,
		*Lan2,
		*DNS1,
		*Exp,
		*MainServer,
		*BackupServer,
		*ReplaceTime,
		*SignInCycle,
		*HeartBeatCycle,
		*McAddr,
		*SuAddr,
		*CommMod,
		*WsdlAddr,
		*PreAddr,
		*node;
	if(NULL == (fp = fopen(SYSTEM_XML_FILE, "r")))
	{
	    trace(TR_SYSTEM, "fopen %s failed\n", SYSTEM_XML_FILE);
		return;
	}

	tree = mxmlLoadFile(NULL, fp, MXML_NO_CALLBACK);
	assert(tree);
	fclose(fp);
    
	Node = mxmlFindElement(tree, tree, "Node", NULL, NULL, MXML_DESCEND);
	assert(Node);

	User = mxmlFindElement(Node, Node, "User", NULL, NULL, MXML_DESCEND);
	assert(UserName);

	UserName = mxmlFindElement(User, User, "UserName", NULL, NULL, MXML_DESCEND);
	assert(UserName);
	node = mxmlGetLastChild(UserName);
	assert(node);
	attr_value = mxmlGetText(node, &whitespace);
	assert(attr_value);
    strncpy(user, attr_value, USER_LEN_MAX - 1);
	trace(TR_SYSTEM, "UserName = %s\n", user);

	UserPws = mxmlFindElement(User, User, "UserPws", NULL, NULL, MXML_DESCEND);
	assert(UserPws);
	node = mxmlGetLastChild(UserPws);
	assert(node);
	attr_value = mxmlGetText(node, &whitespace);
	assert(attr_value);
    strncpy(passwd, attr_value, PASSWD_LEN_MAX - 1);
	trace(TR_SYSTEM, "UserPws = %s\n", passwd);

    //Lan1
	Lan1 = mxmlFindElement(Node, Node, "Lan1", NULL, NULL, MXML_DESCEND);
	assert(Lan1);
	
	node = mxmlFindElement(Lan1, Lan1, "IP", NULL, NULL, MXML_DESCEND);
	assert(node);
	node = mxmlGetLastChild(node);
	assert(node);
	attr_value = mxmlGetText(node, &whitespace);
	assert(attr_value);
	inet_pton(AF_INET, attr_value, (void*)&system_param.comp.lan1_ip);
	trace(TR_SYSTEM, "lan1_ip = 0x%X\n", system_param.comp.lan1_ip);

    node = mxmlFindElement(Lan1, Lan1, "Mask", NULL, NULL, MXML_DESCEND);
	assert(node);
	node = mxmlGetLastChild(node);
	assert(node);
	attr_value = mxmlGetText(node, &whitespace);
	assert(attr_value);
	inet_pton(AF_INET, attr_value, (void*)&system_param.comp.lan1_mask);
	trace(TR_SYSTEM, "lan1_mask = 0x%X\n", system_param.comp.lan1_mask);
	
    node = mxmlFindElement(Lan1, Lan1, "GateWay", NULL, NULL, MXML_DESCEND);
	assert(node);
	node = mxmlGetLastChild(node);
	assert(node);
	attr_value = mxmlGetText(node, &whitespace);
	assert(attr_value);
	inet_pton(AF_INET, attr_value, (void*)&system_param.comp.lan1_gateway);
	trace(TR_SYSTEM, "lan1_gateway = 0x%X\n", system_param.comp.lan1_gateway);

	node = mxmlFindElement(Lan1, Lan1, "MCFlag", NULL, NULL, MXML_DESCEND);
	assert(node);
	node = mxmlGetLastChild(node);
	assert(node);
	attr_value = mxmlGetText(node, &whitespace);
	assert(attr_value);
	trace(TR_SYSTEM, "MCFlag = %s\n", attr_value);
	if(strncmp(attr_value, "True", 4) == 0)
		system_param.comp.mcflag = 1;

	//Lan2
	Lan2 = mxmlFindElement(Node, Node, "Lan2", NULL, NULL, MXML_DESCEND);
	assert(Lan2);
	
	node = mxmlFindElement(Lan2, Lan2, "IP", NULL, NULL, MXML_DESCEND);
	assert(node);
	node = mxmlGetLastChild(node);
	assert(node);
	attr_value = mxmlGetText(node, &whitespace);
	assert(attr_value);
	inet_pton(AF_INET, attr_value, (void*)&system_param.comp.lan2_ip);
	trace(TR_SYSTEM, "lan2_ip = 0x%X\n", system_param.comp.lan2_ip);

    node = mxmlFindElement(Lan2, Lan2, "Mask", NULL, NULL, MXML_DESCEND);
	assert(node);
	node = mxmlGetLastChild(node);
	assert(node);
	attr_value = mxmlGetText(node, &whitespace);
	assert(attr_value);
	inet_pton(AF_INET, attr_value, (void*)&system_param.comp.lan2_mask);
	trace(TR_SYSTEM, "lan2_mask = 0x%X\n", system_param.comp.lan2_mask);
	
    node = mxmlFindElement(Lan2, Lan2, "GateWay", NULL, NULL, MXML_DESCEND);
	assert(node);
	node = mxmlGetLastChild(node);
	assert(node);
	attr_value = mxmlGetText(node, &whitespace);
	assert(attr_value);
	inet_pton(AF_INET, attr_value, (void*)&system_param.comp.lan2_gateway);
	trace(TR_SYSTEM, "lan2_gateway = 0x%X\n", system_param.comp.lan2_gateway);

	node = mxmlFindElement(Lan2, Lan2, "MCFlag", NULL, NULL, MXML_DESCEND);
	assert(node);
	node = mxmlGetLastChild(node);
	assert(node);
	attr_value = mxmlGetText(node, &whitespace);
	assert(attr_value);
	trace(TR_SYSTEM, "MCFlag = %s\n", attr_value);
	if(strncmp(attr_value, "True", 4) == 0)
		system_param.comp.mcflag = 2;

    //DNS1
#ifdef DNS_CFG
	DNS1 = mxmlFindElement(Node, Node, "DNS1", NULL, NULL, MXML_DESCEND);
	assert(DNS1);
	node = mxmlGetLastChild(DNS1);
	assert(node);
	attr_value = mxmlGetText(node, &whitespace);	
	assert(attr_value);
	inet_pton(AF_INET, attr_value, (void*)&system_param.dns1);
	trace(TR_SYSTEM, "DNS1 = %s\n", attr_value);
#endif

	//MainServer
	MainServer = mxmlFindElement(Node, Node, "MainServer", NULL, NULL, MXML_DESCEND);
	assert(MainServer);
	
	node = mxmlFindElement(MainServer, MainServer, "IP", NULL, NULL, MXML_DESCEND);
	assert(node);
	node = mxmlGetLastChild(node);
	assert(node);
	attr_value = mxmlGetText(node, &whitespace);
	assert(attr_value);
	inet_pton(AF_INET, attr_value, (void*)&system_param.comp.mip);
	trace(TR_SYSTEM, "mip = 0x%X\n", system_param.comp.mip);

//    node = mxmlFindElement(MainServer, MainServer, "Mask", NULL, NULL, MXML_DESCEND);
//	assert(node);
//	node = mxmlGetLastChild(node);
//	assert(node);
//	attr_value = mxmlGetText(node, &whitespace);
//	assert(attr_value);
//	inet_pton(AF_INET, attr_value, (void*)&system_param.comp.mmask);
//	trace(TR_SYSTEM, "mmask = 0x%X\n", system_param.comp.mmask);
//	
//    node = mxmlFindElement(MainServer, MainServer, "GateWay", NULL, NULL, MXML_DESCEND);
//	assert(node);
//	node = mxmlGetLastChild(node);
//	assert(node);
//	attr_value = mxmlGetText(node, &whitespace);
//	assert(attr_value);
//	inet_pton(AF_INET, attr_value, (void*)&system_param.comp.mgateway);
//	trace(TR_SYSTEM, "mgateway = 0x%X\n", system_param.comp.mgateway);

	node = mxmlFindElement(MainServer, MainServer, "Port", NULL, NULL, MXML_DESCEND);
	assert(node);
	node = mxmlGetLastChild(node);
	assert(node);
	attr_value = mxmlGetText(node, &whitespace);
	assert(attr_value);
	system_param.comp.mport = atoi(attr_value);
	trace(TR_SYSTEM, "mport = %d\n", system_param.comp.mport);

	//BackupServer
	BackupServer = mxmlFindElement(Node, Node, "BackupServer", NULL, NULL, MXML_DESCEND);
	assert(BackupServer);
	
	node = mxmlFindElement(BackupServer, BackupServer, "IP", NULL, NULL, MXML_DESCEND);
	assert(node);
	node = mxmlGetLastChild(node);
	assert(node);
	attr_value = mxmlGetText(node, &whitespace);
	assert(attr_value);
	inet_pton(AF_INET, attr_value, (void*)&system_param.comp.bip);
	trace(TR_SYSTEM, "bip = 0x%X\n", system_param.comp.bip);

//    node = mxmlFindElement(BackupServer, BackupServer, "Mask", NULL, NULL, MXML_DESCEND);
//	assert(node);
//	node = mxmlGetLastChild(node);
//	assert(node);
//	attr_value = mxmlGetText(node, &whitespace);
//	assert(attr_value);
//	inet_pton(AF_INET, attr_value, (void*)&system_param.comp.bmask);
//	trace(TR_SYSTEM, "bmask = 0x%X\n", system_param.comp.bmask);
//	
//    node = mxmlFindElement(BackupServer, BackupServer, "GateWay", NULL, NULL, MXML_DESCEND);
//	assert(node);
//	node = mxmlGetLastChild(node);
//	assert(node);
//	attr_value = mxmlGetText(node, &whitespace);
//	assert(attr_value);
//	inet_pton(AF_INET, attr_value, (void*)&system_param.comp.bgateway);
//	trace(TR_SYSTEM, "bgateway = 0x%X\n", system_param.comp.bgateway);

	node = mxmlFindElement(BackupServer, BackupServer, "Port", NULL, NULL, MXML_DESCEND);
	assert(node);
	node = mxmlGetLastChild(node);
	assert(node);
	attr_value = mxmlGetText(node, &whitespace);
	assert(attr_value);
	system_param.comp.bport = atoi(attr_value);
	trace(TR_SYSTEM, "bport = %d\n", system_param.comp.bport);

    //ReplaceTime
	ReplaceTime = mxmlFindElement(Node, Node, "ReplaceTime", NULL, NULL, MXML_DESCEND);
	assert(ReplaceTime);
	node = mxmlGetLastChild(ReplaceTime);
	assert(node);
	attr_value = mxmlGetText(node, &whitespace);
	assert(attr_value);
	system_param.comp.lk_time_out = atoi(attr_value);
	trace(TR_SYSTEM, "ReplaceTime = %d\n", system_param.comp.lk_time_out);

	//SignInCycle
	SignInCycle = mxmlFindElement(Node, Node, "SignInCycle", NULL, NULL, MXML_DESCEND);
	assert(SignInCycle);
	node = mxmlGetLastChild(SignInCycle);
	assert(node);
	attr_value = mxmlGetText(node, &whitespace);
	assert(attr_value);
	system_param.comp.secofsign = atoi(attr_value);
	trace(TR_SYSTEM, "SignInCycle = %d\n", system_param.comp.secofsign);
	
    //HeartBeatCycle
	HeartBeatCycle = mxmlFindElement(Node, Node, "HeartBeatCycle", NULL, NULL, MXML_DESCEND);
	assert(HeartBeatCycle);
	node = mxmlGetLastChild(HeartBeatCycle);
	assert(node);
	attr_value = mxmlGetText(node, &whitespace);
	assert(attr_value);
	system_param.comp.secofhbi = atoi(attr_value);
	trace(TR_SYSTEM, "HeartBeatCycle = %d\n", system_param.comp.secofhbi);

	//监控中心地址
	McAddr = mxmlFindElement(Node, Node, "McAddr", NULL, NULL, MXML_DESCEND);
	assert(McAddr);
	node = mxmlGetLastChild(McAddr);
	assert(node);
	attr_value = mxmlGetText(node, &whitespace);
	assert(attr_value);
	system_param.utmp.mc_addr.dev_type = HEX2BCD((attr_value[0] - 48) * 10 + (attr_value[1] - 48));
	system_param.utmp.mc_addr.year = HEX2BCD((attr_value[2] - 48) * 10 + (attr_value[3] - 48));
	system_param.utmp.mc_addr.mon = HEX2BCD((attr_value[4] - 48) * 10 + (attr_value[5] - 48));
	system_param.utmp.mc_addr.fac_id[0] = HEX2BCD(atoi(&attr_value[6]) / 100);
	system_param.utmp.mc_addr.fac_id[1] = HEX2BCD(atoi(&attr_value[6]) % 100);
	trace_buf(TR_SYSTEM, "McAddr = ", &system_param.utmp.mc_addr, sizeof(system_param.utmp.mc_addr));

	//控制器地址
	SuAddr = mxmlFindElement(Node, Node, "SuAddr", NULL, NULL, MXML_DESCEND);
	assert(SuAddr);
	node = mxmlGetLastChild(SuAddr);
	assert(node);
	attr_value = mxmlGetText(node, &whitespace);
	assert(attr_value);
	system_param.utmp.su_addr.dev_type = HEX2BCD((attr_value[0] - 48) * 10 + (attr_value[1] - 48));
	system_param.utmp.su_addr.year = HEX2BCD((attr_value[2] - 48) * 10 + (attr_value[3] - 48));
	system_param.utmp.su_addr.mon = HEX2BCD((attr_value[4] - 48) * 10 + (attr_value[5] - 48));
	system_param.utmp.su_addr.fac_id[0] = HEX2BCD(atoi(&attr_value[6]) / 100);
	system_param.utmp.su_addr.fac_id[1] = HEX2BCD(atoi(&attr_value[6]) % 100);		
    trace_buf(TR_SYSTEM, "SuAddr = ", &system_param.utmp.su_addr, sizeof(system_param.utmp.su_addr));
	
	//Exp
	Exp = mxmlFindElement(Node, Node, "Exp", NULL, NULL, MXML_DESCEND);
	assert(Exp);

	CommMod = mxmlFindElement(Exp, Exp, "CommMod", NULL, NULL, MXML_DESCEND);
	node = mxmlGetLastChild(CommMod);
	assert(node);
	attr_value = mxmlGetText(node, &whitespace);
	assert(attr_value);
	strncpy(sysexp.comm_mod, attr_value, sizeof(sysexp.comm_mod) - 1);
	trace(TR_SYSTEM, "CommMod = %s\n", sysexp.comm_mod);

    WsdlAddr = mxmlFindElement(Exp, Exp, "WsdlAddr", NULL, NULL, MXML_DESCEND);
	node = mxmlGetLastChild(WsdlAddr);
	assert(node);
	attr_value = mxmlGetText(node, &whitespace);
	assert(attr_value);
	strncpy(sysexp.wsdlp.wsdl_addr, attr_value, sizeof(sysexp.wsdlp.wsdl_addr) - 1);
	trace(TR_SYSTEM, "WsdlAddr = %s\n", sysexp.wsdlp.wsdl_addr);

    PreAddr = mxmlFindElement(Exp, Exp, "PreAddr", NULL, NULL, MXML_DESCEND);
	node = mxmlGetLastChild(PreAddr);
	assert(node);
	attr_value = mxmlGetText(node, &whitespace);
	assert(attr_value);
	strncpy(sysexp.wsdlp.pre_addr, attr_value, sizeof(sysexp.wsdlp.pre_addr));
	trace(TR_SYSTEM, "PreAddr = %s\n", sysexp.wsdlp.pre_addr);
    
	mxmlDelete(tree);
#endif
}

/*****************************************************************************
 * Function      : system_mgr
 * Description   : 系统参数管理接口
 * Input         : param_opt_t *opt
 * Output        : None
 * Return        : opt_rst_e
 * Others        : 
 * Record
 * 1.Date        : 20160110
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
opt_rst_e system_mgr( param_opt_t *opt)
{
    opt_rst_e ret = OPT_OK;
	boolean is_changed = false;
	
    if(opt->pin == NULL)
		return OPT_ERR_ELSE;  

    SYSTEM_LOCK;
	if(opt->optt == OPT_TYPE_SET)
	{
	    if(OBJ_TYPE_SYSTEM == opt->objt)
    	{
    	    sys_param_t *temp = (sys_param_t *)opt->pin;
			
			//通信参数变更
			if(memcmp((void*)&system_param.comp, (void*)&temp->comp, sizeof(comm_param_t)) != 0)
			{
			    trace(TR_SYSTEM, "communication param modify\n");
			    memcpy((void*)&system_param.comp, (void*)&temp->comp, sizeof(comm_param_t));
			    //通知comm模块,更新通信参数
			    comm_notifyofparamchange();
				is_changed = true;
			}

		    //监控中心地址和设备地址变更
		    if(memcmp((void*)&system_param.utmp, (void*)&temp->utmp, sizeof(utm_param_t)) != 0)
			{
			    trace(TR_SYSTEM, "utm param modify\n");
			    memcpy((void*)&system_param.utmp, (void*)&temp->utmp, sizeof(utm_param_t));
				is_changed = true;
			}

			//域名解析服务器变更
#ifdef DNS_CFG
			if(memcmp(&system_param.dns1, &temp->dns1, sizeof(temp->dns1)) != 0)
			{
			    char cmd[100] = {0};
				char dnsbuf[20] = {0};
			    //删除原来的DNS
			    inet_ntop(AF_INET, (void *)&system_param.dns1, dnsbuf, 16);
			    sprintf(cmd, "sed \'/%s/d\' /etc/resolv.conf >> /etc/test.conf", dnsbuf);
			    system(cmd);
				//添加新的DNS
				inet_ntop(AF_INET, (void *)&temp->dns1, dnsbuf, 16);
			    sprintf(cmd, "echo \"nameserver %s\"  >> /etc/test.conf", dnsbuf);
				system(cmd);
				//更新
				sprintf(cmd, "mv /etc/resolv.conf /etc/resolv.conf.bak");
				system(cmd);
				sprintf(cmd, "mv /etc/test.conf /etc/resolv.conf");
				system(cmd);

				system_param.dns1 = temp->dns1;
				is_changed = true;
			}
#endif
    	}
		else if(OBJ_TYPE_SYSTEM_EXP == opt->objt)
		{
		    exp_param_t *exp = (exp_param_t *)opt->pin;
//			trace(TR_SYSTEM, "exp  comm_mod %s \n pre_addr %s\n wsdl_addr %s\n", exp->comm_mod, exp->wsdlp.pre_addr, exp->wsdlp.wsdl_addr);
//			trace(TR_SYSTEM, "exp  comm_mod %s \n pre_addr %s\n wsdl_addr %s\n", sysexp.comm_mod, sysexp.wsdlp.pre_addr, sysexp.wsdlp.wsdl_addr);
		    if(strncmp((void*)sysexp.comm_mod, (void*)exp->comm_mod, sizeof(exp->comm_mod)) != 0)
			{
			    trace(TR_SYSTEM, "exp comm_mod modify\n");
			    memcpy((void*)sysexp.comm_mod, (void*)exp->comm_mod, sizeof(exp->comm_mod));
				
				//通知comm模块,更新通信参数
			    comm_notifyofparamchange();
				
				is_changed = true;
			}
			if(strncmp((void*)&sysexp.wsdlp, (void*)&exp->wsdlp, sizeof(exp->wsdlp)) != 0)
			{
			    trace(TR_SYSTEM, "exp wsdl param modify\n");
			    memcpy((void*)&sysexp.wsdlp, (void*)&exp->wsdlp, sizeof(exp->wsdlp));
				is_changed = true;
			}
		}
	}
	else
	{
	    trace(TR_SYSTEM, "sys opt err[%d]\n", opt->optt);
		ret = OPT_ERR_ELSE;
	}

	if(true == is_changed)
		system_save();
	
    SYSTEM_UNLOCK;
	
	return ret;
}

/*****************************************************************************
 * Function      : usr_mgr
 * Description   : 用户管理
 * Input          : None
 * Output        : None
 * Return        : opt_rst_e
 * Others        : 
 * Record
 * 1.Date        : 20160115
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
opt_rst_e usr_mgr( param_opt_t *opt )
{
    opt_rst_e ret = OPT_OK;;
	char *username;
	char *key;

	if((OBJ_TYPE_ACCESS == opt->objt)
		&& (OPT_TYPE_SET == opt->optt)
		&& (opt->pin))
	{
	    username = opt->pin;
		key = (char *)(opt->pin + USER_LEN_MAX);

		SYSTEM_LOCK;
		
	    if((strncmp(user, username, USER_LEN_MAX) != 0)
			|| (strncmp(passwd, key, PASSWD_LEN_MAX) != 0))
    	{
            strncpy(user, username, USER_LEN_MAX);
			strncpy(passwd, key, PASSWD_LEN_MAX);
			
    	    system_save();
    	}	

		SYSTEM_UNLOCK;
	}

	return ret;
}


/*****************************************************************************
 * Function      : sys_get_param
 * Description   : 获取系统配置信息
 * Input          : sys_param_t * syscfg
 * Output        : None
 * Return        : void
 * Others        : 
 * Record
 * 1.Date        : 20151213
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
//void sys_get_param ( sys_param_t * syscfg )
//{
//    memcpy((void*)syscfg, (void*)&system_param, sizeof(sys_param_t));
//}

/*****************************************************************************
 * Function      : system_getcommparam
 * Description   : 获取通信参数
 * Input         : comm_param_t *comp
 * Output        : None
 * Return        : void
 * Others        : 
 * Record
 * 1.Date        : 20160111
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
void system_getcommparam( comm_param_t *comp )
{
    SYSTEM_LOCK;
    memcpy((void*)comp, (void*)&system_param.comp, sizeof(comm_param_t));
	SYSTEM_UNLOCK;
}

/*****************************************************************************
 * Function      : system_getutmparam
 * Description   : 获取utm模块相关参数
 * Input         : utm_param_t *utmp
 * Output        : None
 * Return        : void
 * Others        : 
 * Record
 * 1.Date        : 20160111
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
void system_getutmparam( utm_param_t *utmp )
{
    SYSTEM_LOCK;
    memcpy((void*)utmp, (void*)&system_param.utmp, sizeof(utm_param_t));
	SYSTEM_UNLOCK;
}

/*****************************************************************************
 * Function      : system_gettracemask
 * Description   : 获取日志开关信息
 * Input         : uint32 *trace_mask
 * Output        : None
 * Return        : void 
 * Others        : 
 * Record
 * 1.Date        : 20160113
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
//void  system_gettracemask( uint32 *trace_mask )
//{
//    SYSTEM_LOCK;
//    *trace_mask = sysexp.the_trace_mask;
//	SYSTEM_UNLOCK;
//}

/*****************************************************************************
 * Function      : system_getwsdlparam
 * Description   : void
 * Input         : webservice_param_t *wsdlp
 * Output        : None
 * Return        : void
 * Others        : 
 * Record
 * 1.Date        : 20160126
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
void system_getwsdlparam( webservice_param_t *wsdlp )
{
    SYSTEM_LOCK;
	*wsdlp = sysexp.wsdlp;
	SYSTEM_UNLOCK;
}

/*****************************************************************************
 * Function      : system_getcommod
 * Description   : 获取上行通信方式
 * Input         : char *commmod
 * Output        : None
 * Return        : void
 * Others        : 
 * Record
 * 1.Date        : 20160126
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
void system_getcommod( char *mode )
{
    SYSTEM_LOCK;
	strncpy(mode, sysexp.comm_mod, sizeof(sysexp.comm_mod));
	SYSTEM_UNLOCK;
}

/*****************************************************************************
 * Function      : system_init
 * Description   : 系统配置初始化
 * Input         : void
 * Output        : None
 * Return        : void
 * Others        : 
 * Record
 * 1.Date        : 20151213
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
void system_init( void )
{
    if(0 != pthread_mutex_init(&system_mutex, NULL))
		trace(TR_SYSTEM, "create aid_mutex failed\n");
	
    //加载系统配置
    system_load();

//	//更新trace_mask
//	trace_mask_update(sysexp.the_trace_mask);
}


