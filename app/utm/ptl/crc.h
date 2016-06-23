#ifndef _CRC_H
#define _CRC_H
//#include "stm32f4xx_crc.h"
#include "../inc/atypes.h"
#define CRC32_MPEG2// ���ڲ��
//#define CRC32_TABLE// ���ɱ�
//#define CRC32_STM32// ����stm32

#ifdef CRC32_MPEG2// ���ڲ��
uint32 CRC32_SCal(uint8 *p, uint16 len); 
#endif

#ifdef CRC32_STM32// ����stm32
u32 CRC32_HCal(u8 *p, u16 len);
#endif

//void CrcTest(void);

#endif