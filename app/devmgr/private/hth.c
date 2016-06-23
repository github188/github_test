/********************************************************************************

 **** Copyright (C), 2015, BinHong tech Co., Ltd.                ****

 ********************************************************************************
 * File Name     : hth.c
 * Author        : chenxu
 * Date          : 2015-12-19
 * Version       : 1.0
 * Function List :
 * Description   : ��ͨ��Modbus˽��Э�����
 * Record        :
 * 1.Date        : 2015-12-19
 *   Author      : chenxu
 *   Modification: Created file

 *******************************************************************************/

/*------------------------------------------------------------------------------
Section: Includes
------------------------------------------------------------------------------*/
#include "../../inc/atypes.h"
#include "../../inc/maths.h"
#include <stdio.h>
#include <math.h>
#include <assert.h>
#include "hth.h"
#include "share.h"
#include "dev.h"
#include "trace.h"
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
/*------------------------------------------------------------------------------
Section: Function Prototypes
------------------------------------------------------------------------------*/
/*****************************************************************************
 * Function      : reg_to_value
 * Description   : �ӼĴ�����ȡ��ֵͨ�ýӿ�
 * Input          : None
 * Output        : None
 * Return        : 
 * Others        : 
 * Record
 * 1.Date        : 20160301
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
static float32  reg_to_value(uint16 pn, uint8 *buf, float32 dataprecision, modbus_chan_t *modchan)
{
    uint16 reg = (buf[0] << 8) + buf[1];;

	return (reg * dataprecision);
}

/*****************************************************************************
 * Function      : typ1_reg_to_value
 * Description   : ���ݼĴ������ݻ�ȡ��ֵ
 * Input         : uint8 buf[2]
 * Output        : None
 * Return        : float32
 * Others        : 
 * Record
 * 1.Date        : 20151219
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
static float32 typ1_reg_to_value(uint16 pn, uint8 *buf, float32 dataprecision, modbus_chan_t *modchan)
{
    uint16 reg = 0;
    float32 value = 1.0;
	
	value = value * dataprecision;

	//��ʪ�ȴ�����������״̬��Ϣ
	if(pn == 4)
		reg = 0x01 & buf[0];
	else
		reg = (buf[0] << 8) + buf[1];

	return (dataprecision * reg);
}

/*****************************************************************************
 * Function      : typ2_reg_to_value
 * Description   : ���ݼĴ������ݻ�ȡ��ֵ
 * Input         : uint8 buf[2]
 * Output        : None
 * Return        : float32
 * Others        : 
 * Record
 * 1.Date        : 20151219
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
static float32 typ2_reg_to_value(uint16 pn, uint8 *buf, float32 dataprecision, modbus_chan_t *modchan)
{
    uint16 reg = 0;
    float32 value = 1.0;

    reg = (buf[0] << 8) + buf[1];

	/*
	 *й¶λ������ֵ˵���������յ���й¶λ������/100�����õ���ǰй¶λ�õ�ʵ��������
	 *������յ���й¶λ������ֵΪ01 18 ��ʮ������ת��Ϊʮ����Ϊ280����
	 *��ʵ��й¶λ������Ϊ280/100 = 2.8�ס�
	 */
	if(pn == 1)
		value = (reg * dataprecision) / 100;
	else
		value = reg * dataprecision;

	return value;
}

/*****************************************************************************
 * Function      : typ2_reg_to_value
 * Description   : ���ݼĴ������ݻ�ȡ��ֵ,������ACRϵ��
 * Input         : uint8 buf[2]
 * Output        : None
 * Return        : float32
 * Others        : 
 * Record
 * 1.Date        : 20151219
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
static float32 typ4_reg_to_value(uint16 pn, uint8 *buf, float32 dataprecision, modbus_chan_t *modchan)
{
    uint16 reg = 0;
    float32 value = -1.0;
	uint8 dct = 0;//���豸�л�ȡdct��ֵ,��0023H�ĵ��ֽڶ���
	uint8 dpt = 0;//���豸�л�ȡdpt��ֵ,��0023H�ĸ��ֽڶ���
	uint8 dpq = 0;//���豸�л�ȡdpt��ֵ,��0024H�ĸ��ֽڶ���
	float32 s = 1.0; //TODO ����λ
	uint8 buffer[MB_SER_PDU_SIZE_MAX];
			struct mb_read_param pparam = {modchan->chantyp, modchan->port, 
				modchan->devaddr, 0, 1, buffer, 0};

    reg = (buf[0] << 8) + buf[1];
    switch(pn)
	{
	    case 1 ... 3: //����
	        pparam.start_addr = 0x0023;
			value = (float32)reg;
			if(0 == mb_read(&pparam))
			{
			    dct = buffer[4];
			    value = value * pow(10, (dct - 4));
				printf("pow(10, (dct - 4)) = %f\n", pow(10, (dct - 4)));
			}
			else
			{
			    trace(TR_EVENT, "function[%s] mb_read dct fail,chantyp[%d],port[%d],devaddr[%d]\n",
					__FUNCTION__, modchan->chantyp, modchan->port, modchan->devaddr);
				value = MAGIC_VALUE;
			}
			break;
		case 4 ... 9://�ࡢ�ߵ�ѹ
		    pparam.start_addr = 0x0023;
			value = (float32)reg;
			if(0 == mb_read(&pparam))
			{
			    dpt = buffer[3];
		        value = value * pow(10, (dpt - 4));
				printf("pow(10, (dpt - 4)) = %f\n", pow(10, (dpt - 4)));
			}
			else
			{
			    trace(TR_EVENT, "function[%s] mb_read dpt fail,chantyp[%d],port[%d],devaddr[%d]\n",
					__FUNCTION__, modchan->chantyp, modchan->port, modchan->devaddr);
				value = MAGIC_VALUE;
			}
			break;
		case 0x0a ... 0x11://��/�޹�����
		    pparam.start_addr = 0x0024;
			value = (float32)reg;
			if(0 == mb_read(&pparam))
			{
			    dpq = buffer[3];
		        value = value * pow(10, (dpq - 4));
				printf("pow(10, (dpq - 4)) = %f\n", pow(10, (dpq - 4)));
			}
			else
			{
			    trace(TR_EVENT, "function[%s] mb_read dpq fail,chantyp[%d],port[%d],devaddr[%d]\n",
					__FUNCTION__, modchan->chantyp, modchan->port, modchan->devaddr);
				value = MAGIC_VALUE;
			}	        
			break;
		case 0x16 ... 0x19://��������
		    value = ((float32)reg) / 1000;
			break;
		case 0x1A://Ƶ��
		    value = ((float32)reg) / 100;
			break;
	}

	return value;
}

/*****************************************************************************
 * Function      : hth_value_to_reg
 * Description   : ����ֵת��Ϊ�Ĵ�������
 * Input         : float32 *value
                uint8 dot
 * Output        : uint8 *buf
 * Return        : void
 * Others        : 
 * Record
 * 1.Date        : 20151220
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
void hth_value_to_reg( float32 value, float32 dataprecision, uint8 *buf )
{
    uint16 reg;

    value = value / dataprecision;
	
	reg = (uint16)value;
	buf[0] = reg >> 8;
	buf[1] = reg % 256;
}

static MBREG2VALUEFUNCPTR reg2value_funs[] = 
{
    NULL,
	typ1_reg_to_value,
	typ2_reg_to_value,
	reg_to_value,
	typ4_reg_to_value,
	reg_to_value,
	reg_to_value
};

/*****************************************************************************
 * Function      : hth_reg_to_value
 * Description   : ��ͨ��modbus�Ĵ�����ֵ��ȡ����
 * Input         : uint16 dev_type_id �豸����
                uint16 pn �������
                uint8 *buf 2�ֽڼĴ�������
                float32 dataprecision ���ݾ���
                modbus_chan_t modchan
 * Output        : None
 * Return        : float32
 * Others        : 
 * Record
 * 1.Date        : 20160301
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
float32 hth_reg_to_value( uint16 dev_type_id, uint16 pn, uint8 *buf, float32 dataprecision, modbus_chan_t *modchan)
{
    uint32 size = ARRAY_SIZE(reg2value_funs);

	assert(dev_type_id < size);

    if(reg2value_funs[dev_type_id])
		return reg2value_funs[dev_type_id](pn, buf, dataprecision, modchan);
	else
		return MAGIC_VALUE;
}
