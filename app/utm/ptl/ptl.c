/********************************************************************************

 **** Copyright (C), 2015, BinHong tech Co., Ltd.                ****

 ********************************************************************************
 * File Name     : ptl.c
 * Author        : chenxu
 * Date          : 2015-11-18
 * Version       : 1.0
 * Function List :
 * Description   : ����ͨ��Э��ʵ�����APIʵ��
 * Record        :
 * 1.Date        : 2015-11-18
 *   Author      : chenxu
 *   Modification: Created file

 *******************************************************************************/

/*------------------------------------------------------------------------------
Section: Includes
------------------------------------------------------------------------------*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../inc/maths.h"
#include "ptl.h"
#include <time.h>
#include "protocol.h"
#include "crc.h"
#include "../log/trace.h"
#include "share.h"
/*------------------------------------------------------------------------------
Section: Macro Definitions
------------------------------------------------------------------------------*/
#define PTLPRINTF(x...) \
    {    \
        (void)printf("[ptl]L[%d]%s()\r\t\t\t\t", __LINE__, __FUNCTION__); \
        (void)printf(x);    \
    }

#define FRAME_HEADER_LEN		27		//����ͷ����(֡������ǰ�������ݳ���)
#define CRC_LEN					4
/*------------------------------------------------------------------------------
Section: Type Definitions
------------------------------------------------------------------------------*/
/* None */
/*------------------------------------------------------------------------------
Section: Globals
------------------------------------------------------------------------------*/
const char *value_type[] = {"None", "Char", "Unsigned Char", "Int", "Unsigned Int", "Long", "Unsigned Long", "Float"};
const char *access_type[] = {"R", "W", "RW", "NONE"};

/*****************************************************************************
 * Function      : get_value_type
 * Description   : �������ʹ����������ַ���ת��������ID
 * Input         : char *str
 * Output        : None
 * Return        : value_type_e
 * Others        : 
 * Record
 * 1.Date        : 20151204
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
value_type_e get_value_type( char *str )
{
    uint32 size = ARRAY_SIZE(value_type);
	uint32 idx;
	value_type_e vt;
	uint32 len = strlen(str);

//    DEVPRINTF("len = %d, %s\n", len, str);
	for(idx = 1; idx < size; idx ++)
	{
//	    DEVPRINTF("%s, len = %d\n", value_type[idx], strlen(value_type[idx]));
	    if((len == strlen(value_type[idx])) && (strcmp(str, value_type[idx]) == 0))
			break;
	}
	if(idx >= size)
		vt = VALUE_TYPE_NONE;
	else
		vt  = (value_type_e)idx;

	return vt;
}

/*****************************************************************************
 * Function      : get_value_type
 * Description   : ��ȡ����Ȩ��
 * Input         : char *str
 * Output        : None
 * Return        : uint8
 * Others        : 
 * Record
 * 1.Date        : 20151204
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
uint8 get_access( char *str )
{
    uint32 size = ARRAY_SIZE(access_type);
	int32 idx;
    uint32 len;

	len = strlen(str);

//    DEVPRINTF("len = %d\n", len);
	for(idx = 0; idx < size; idx ++)
	{
//	    DEVPRINTF("%s len = %d\n",access_type[idx], strlen(access_type[idx]));
	    if((len == strlen(access_type[idx])) 
			&& (strcmp(str, access_type[idx]) == 0))
			break;
	}
	if(idx >= size)
		idx = ACCESS_NONE;

	return idx;
}

/*****************************************************************************
// * Function      : get_item_cfg_info
// * Description   : ��ȡָ��������������Ϣ
// * Input         : uint16 dev_type_id
//                uint16 id
// * Output        : dev_param_t *item
// * Return        : OK ERROR
// * Others        :
// * Record
// * 1.Date        : 20151120
// *   Author      : chenxu
// *   Modification: Created function

//*****************************************************************************/
//status_t get_item_cfg_info( uint16 dev_type_id, uint16 id, dev_param_t *item )
//{
//    int32 left = 0;
//    int32 right = 0;
//    int32 middle;
//	dev_param_t *items;
//	uint16 id_count;

//    if(dev_type_id >= ARRAY_SIZE(devs_conf))
//		return ERROR;

//	items = devs_conf[dev_type_id].dev_param_cfg;
//	id_count = devs_conf[dev_type_id].item_count;
//    right = id_count - 1;

//    while (left <= right)
//    {
//        middle = (left + right) / 2;
//        if (id < items[middle].pn)
//        {
//            right = middle - 1;
//        }
//        else if (id > items[middle].pn)
//        {
//            left = middle + 1;
//        }
//        else
//        {
//            /* ���ǰ���Ƿ�����ͬ��id */
//            while ((middle > 0) && (items[middle].pn == id))
//            {
//                middle--;
//            }
//			memcpy((uint8*)item, &items[middle], sizeof(dev_param_t));
//            return OK;
//        }
//    }

//    return ERROR;
//}

/*****************************************************************************
// * Function      : get_param_type
// * Description   : ��ȡ������������
// * Input         : uint16 dev_type_id
// 				uint16 pn
// * Output        : None
// * Return        : ptl_param_type_t
// * Others        :
// * Record
// * 1.Date        : 20151120
// *   Author      : chenxu
// *   Modification: Created function

//*****************************************************************************/
//value_type_e get_param_type( uint16 dev_type_id, uint16 pn )
//{
//    value_type_e vt = VALUE_TYPE_NONE;
//	dev_param_t item;

//	if(dev_type_id >= ARRAY_SIZE(devs_conf))
//	{
//	    trace(TR_UTM, "dev_type_id [%d]\n", dev_type_id);
//		return vt;
//	}

//	if(OK == get_item_cfg_info(dev_type_id, pn, &item))
//		vt = item.vt;

//	return vt;
//}

/*****************************************************************************
 * Function      : get_var_value
 * Description   : ��ȡ������ֵ
 * Input         : uint8* data
                uint16 dev_type_id �豸���ͺ�
                uint16 pn �������ƺ�
 * Output        : float32 *value
 * Return        : status_t OK or ERROR
 * Others        :
 * Record
 * 1.Date        : 20151120
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
status_t get_var_value( uint8* data, var_cfg_t vcfg, float32 *value, uint32 * len)
{
	float32 weight = 1.0;
	float32 dataprecision = 0;

    *len = 0;
	
	if(VALUE_TYPE_NONE == vcfg.vt)
	{
	    trace(TR_UTM, "vt = VALUE_TYPE_NONE\n");
		return ERROR;
	}

	dataprecision = vcfg.dataprecision;	
	weight = weight * dataprecision;

	switch(vcfg.vt)
	{
		case VALUE_TYPE_UINT8:
			*value = (*(uint8*)data) * weight;
			*len = 1;
			break;
		case VALUE_TYPE_INT8:
			*value = (*(int8*)data) * weight;
			*len = 1;
			break;
		case VALUE_TYPE_INT16:
			binvert(data, 2);
			*value = (*(int16*)data) * weight;
			*len = 2;
			break;
		case VALUE_TYPE_UINT16:
			binvert(data, 2);
			*value = (*(uint16*)data) * weight;
			*len = 2;
			break;
		case VALUE_TYPE_INT32:
			binvert(data, 4);
			*value = (*(int32*)data) * weight;
			*len = 4;
			break;
		case VALUE_TYPE_UINT32:
			binvert(data, 4);
			*value = (*(uint32*)data) * weight;
			*len = 4;
			break;
		case VALUE_TYPE_FLOAT32:
			binvert(data, 4);
			*value = (*(int32*)data) * weight;
			*len = 4;
			break;
		default:
			return ERROR;
	}
	trace_buf(TR_UTM, "data:", data, *len);
	return OK;
}

/*****************************************************************************
 * Function      : pack_var_value
 * Description   : ��������ֵ���
 * Input         : uint8 *pack
                uint16 dev_type_id �豸���ͺ�
                uint16 pn �������ƺ�
 * Output        : uint32 *len �������
 * Return        : status_t OK or ERROR
 * Others        :
 * Record
 * 1.Date        : 20151120
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
status_t pack_var_value( uint8 *pack, float32 value, var_cfg_t vcfg, uint32 *len )
{
    uint32 weight = 1;
	float32 dataprecision = 0;
	uint32 idx = 0;

    if(VALUE_TYPE_NONE == vcfg.vt)
		return ERROR;

	pack[idx++] = vcfg.pn >> 8;
	pack[idx++] = vcfg.pn % 256;

    dataprecision = vcfg.dataprecision;
	weight = weight / dataprecision;
	value *= weight;

	switch(vcfg.vt)
	{
		case VALUE_TYPE_INT8:
			*(int8*)(pack + idx) = (int8)value;
			idx += 1;
			break;
		case VALUE_TYPE_UINT8:
			*(uint8*)(pack + idx) = (uint8)value;
			idx += 1;
			break;
		case VALUE_TYPE_INT16:
			*(int16*)(pack + idx) = (int16)value;
			idx += 2;
			break;
		case VALUE_TYPE_UINT16:
			*(uint16*)(pack + idx) = (uint16)value;
			idx += 2;
			break;
		case VALUE_TYPE_INT32:
			*(int32*)(pack + idx) = (int32)value;
			idx += 4;
			break;
		case VALUE_TYPE_UINT32:
			*(uint32*)(pack + idx) = (uint32)value;
			idx += 4;
			break;
		default:
			return ERROR;
	}

	if(idx > 3)
		binvert(&pack[2], idx - 2);

    *len = idx;
	return OK;
}


/*****************************************************************************
 * Function      : create_param_list
 * Description   : ���ݱ����ṩ�������ɲ�����
 * Input         : uint16 dev_type_id
 				uint8 *data
                uint32 datalen
 * Output        : None
 * Return        : param_list_t
 * Others        :
 * Record
 * 1.Date        : 20151120
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
//param_list_t *create_param_list( uint16 dev_type_id, uint8 *data, uint32 datalen )
//{
//    uint32 idx = 0;
//	param_list_t plist = NULL;
//	param_node_t* pnode = NULL;
//    uint16 pn;
//	uint8 dot = 0;
//	value_type_e vt = VALUE_TYPE_NONE;
//	float32 value = 0.0;
//	dev_param_t item;
//	const dev_conf_t * const dev_conf = &devs_conf[dev_type_id];
//	uint32 type_length = 0;
//	float32  weight = 1.0;

//	while(idx < datalen)
//	{
//	    pn = data[idx] << 8 + data[idx + 1];
//		idx += 2;
//		//�ҵ�item��������Ϣ
//		if(ERROR == get_item_cfg_info(dev_type_id, pn, &item))
//			return plist;
//		vt = item.vt;
//		if(VALUE_TYPE_NONE == vt)
//			return plist;

//		pnode = (param_node_t*)malloc(sizeof(param_node_t));
//		if(NULL == pnode)
//		{
//			trace(TR_UTM, "pnode malloc failed \n");
//			return plist;
//		}
//		//����value��ֵ
//		pnode->dot = item.dot;

//		idx += fmt_length_table[vt];

//		pnode->pn = pn;
//		pnode->vt = vt;
//		pnode->next = NULL;

//		if(NULL == plist)
//		    plist = pnode;
//		else
//			plist->next = pnode;
//	}
//}

/*****************************************************************************
 * Function      : free_param_list
 * Description   : �ͷŲ�����洢�ռ�
 * Input         : param_list_t plist
 * Output        : None
 * Return        : void
 * Others        :
 * Record
 * 1.Date        : 20151120
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
//void free_param_list( param_list_t plist )
//{
//    param_node_t *pnode = plist;
//	param_node_t *pnext = NULL;

//	while(pnode != NULL)
//	{
//	    pnext = pnode->next;
//		free(pnode);
//		pnode = pnext;
//	}
//}

/*****************************************************************************
 * Function      : ptl_get_time
 * Description   : ��7�ֽ�ʱ���ת��Ϊtime_t��ʽ
 * Input          : uint8 * data
 * Output        : None
 * Return        : time_t
 * Others        : 
 * Record
 * 1.Date        : 20151208
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
time_t ptl_get_time( uint8 * data)
{
    ptl_time_t *pt = (ptl_time_t*)data;
    struct tm tm;
	
	if(NULL == pt)
		return 0;

	tm.tm_year = BCD2HEX(pt->year_h) * 100 + BCD2HEX(pt->year_l) - 1900;
	tm.tm_mon = BCD2HEX(pt->month) - 1;
	tm.tm_mday = BCD2HEX(pt->day);
	tm.tm_hour = BCD2HEX(pt->hour);
	tm.tm_min = BCD2HEX(pt->minute);
	tm.tm_sec = BCD2HEX(pt->second);

	return mktime(&tm);
}

/*****************************************************************************
 * Function      : ptl_pack_time
 * Description   : ���ʱ���
 * Input         : time_t tp
 * Output        : uint8 *pack
 * Return        : status_t
 * Others        : 
 * Record
 * 1.Date        : 20151208
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
status_t ptl_pack_time( time_t tp, uint8 *pack )
{
    ptl_time_t *pt = (ptl_time_t*)pack;
    struct tm tm;
	
	if(NULL == pt)
		return ERROR;

	localtime_r(&tp, &tm);
    pt->year_h = HEX2BCD((tm.tm_year + 1900) / 100);
	pt->year_l = HEX2BCD((tm.tm_year + 1900) % 100);
	pt->month = HEX2BCD(tm.tm_mon + 1);
	pt->day = HEX2BCD(tm.tm_mday);
	pt->hour = HEX2BCD(tm.tm_hour);
	pt->minute = HEX2BCD(tm.tm_min);
	pt->second = HEX2BCD(tm.tm_sec);

	return OK;
}

/*****************************************************************************
 * Function      : ptl_parse
 * Description   : ���Ľ���
 * Input         : uint8 *frm
                uint32 len
 * Output        : ptl_rst_t *ptl
 * Return        : ptl_rtn_e
 * Others        :
 * Record
 * 1.Date        : 20151119
 *   Author      : chenxu
 *   Modification: Created function
*****************************************************************************/
ptl_rtn_e ptl_parse( uint8 *frm, uint32 len, ptl_rst_t *ptl)
{
    uint32 pre_char = 0;
	uint16 length = 0;
	uint16 cs = 0;
	uint32 crc32 = 0;
	uint32 calc = 0;
	uint8 ptl_ver = 0;

    do
	{
	    /*��С����*/
	    if((FRAME_HEADER_LEN + CRC_LEN) > (len - pre_char))
        {
            trace(TR_UTM, "packadge len[%d] is less than the mini length[%d]\n", len, FRAME_HEADER_LEN + CRC_LEN);
            return PTL_CS_ERR;
        }
		/*������ʼ��0x68*/
		if (frm[pre_char] != 0x68)
        {
            pre_char ++;
            continue;
        }
        trace(TR_UTM, "len = %d\n", len);
		/*��鳤��*/
		length = frm[pre_char + 1] * 256 + frm[pre_char + 2];
		trace(TR_UTM, "length = %d\n", length);
		if(len < (pre_char + length + 2))
		{
		    trace(TR_UTM, "len[%d], lenght[%d], pre_char[%d]\n", len, length, pre_char);
			return PTL_CS_ERR;
		}

		/*���У���*/
//		cs = frm[pre_char + length] * 256 + frm[pre_char + length + 1];
        crc32 = (frm[pre_char + length] * 0x1000000) + (frm[pre_char + length + 1] * 0x10000) +
                 (frm[pre_char + length + 2] * 0x100) + frm[pre_char + length + 3];
		calc = CRC32_SCal(&frm[pre_char], length);
		if(crc32 != calc)
		{
		    trace(TR_UTM, "pre_char = %d crc32[%d],calc[%d]\n",pre_char, crc32, calc);
			trace_buf(TR_UTM_BUF, "crcbuf:", &frm[pre_char + length], 4);
			return PTL_CS_ERR;
		}
		//Э��汾	1	��һ��Э��Ϊ01H����������
		ptl_ver = frm[pre_char + 3];
		if(PTL_VERSION != ptl_ver)
			return PTL_VER_ERR;
		//�����ֶ�	1	����
		//֡���	1
		ptl->frm_ctg = frm[pre_char + 5];
		//��Ϣʱ���	7�ֽ�BCD��
		ptl->stp = ptl_get_time(&frm[pre_char + 6]);
		//�����ֶ�2�ֽ�
		//Դ��ַ	5�ֽ�BCD
		memcpy(&ptl->src_addr, &frm[pre_char + 15], 5);
		//Ŀ�ĵ�ַ	5�ֽ�BCD
		memcpy(&ptl->dest_addr, &frm[pre_char + 20], 5);
		//��ϢID	2
		ptl->msg_id = (frm[pre_char + 25] << 8) + frm[pre_char + 26];
		//֡����	�ɱ䳤��
		ptl->datalen = length - FRAME_HEADER_LEN;
		ptl->data = &frm[pre_char + FRAME_HEADER_LEN];
		break;
	}while(1);

	return PTL_OK;
}

/*****************************************************************************
 * Function      : ptl_pack
 * Description   : HMAC ����ӿ�
 * Input         : ptl_rst_t ptl
 * Output        : uint8 * pack
 				uint32 *len
 * Return        : ptl_rtn_e
 * Others        :
 * Record
 * 1.Date        : 20151119
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
ptl_rtn_e ptl_pack( ptl_rst_t *ptl, uint8 *pack, int32 *len)
{
    uint32 idx = 0;
    uint32 crc = 0;
	uint32 length = 0;//��֡ͷ��ʼ����֡ͷ����У�飨����У�飩���������ֽڸ���
	//֡ͷ	1
	pack[idx++] = 0x68;
	//����	2�ֽ� = ptl->datalen + 32 ���� 32 + 11
	if(FRMCTG_ERR != ptl->frm_ctg)
		length = ptl->datalen + FRAME_HEADER_LEN;
	else
		length = 11 + FRAME_HEADER_LEN;
	pack[idx++] = length / 256;
	pack[idx++] = length % 256;
	//Э��汾	1
	pack[idx++] = PTL_VERSION;
	//�����ֶ�	1
	pack[idx++] = 0;
	//֡���	1
	pack[idx++] = ptl->frm_ctg;
	//��Ϣʱ���	7�ֽ�BCD��
	ptl_pack_time(ptl->stp, &pack[idx]);
	idx += 7;
	//�����ֶ�	2
	pack[idx++] = 0;
	pack[idx++] = 0;
	//Դ��ַ	5�ֽ�BCD
	memcpy((void*)&pack[idx], (void*)&ptl->src_addr, 5);
	idx += 5;
	//Ŀ�ĵ�ַ	5�ֽ�BCD
	memcpy((void*)&pack[idx], (void*)&ptl->dest_addr, 5);
	idx += 5;
	//��ϢID	2
	pack[idx++] = ptl->msg_id >> 8;
	pack[idx++] = ptl->msg_id % 256;
	//֡����	�ɱ䳤��
	memcpy(&pack[idx], ptl->data, ptl->datalen);
	idx += ptl->datalen;
	//У���	2
	crc = CRC32_SCal(pack, length);
//	trace(TR_UTM, "crc = 0x%08x\n", crc);
	pack[idx++] = (crc >> 24) & 0xFF;
	pack[idx++] = (crc >> 16) & 0xFF;
	pack[idx++] = (crc >>  8) & 0xFF;
	pack[idx++] =         crc & 0xFF;
	
	*len = idx;
	return PTL_OK;
}

//void printbuffer(char *format, uint8 *buffer, uint32 len)
//{
//	uint32 i;

//	printf(format,0,0,0,0,0,0);
//	for(i = 0; i < len; i++){
//        //printf("%02X ", buffer[i],0,0,0,0,0);
//		printf("%02X ", buffer[i],0,0,0,0,0);
//	}
//	printf("\n",0,0,0,0,0,0);
//}

