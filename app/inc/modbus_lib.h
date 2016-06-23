#ifndef __MODBUS_LIB_H__
#define __MODBUS_LIB_H__

#include <stdint.h>

#define MODBUS_MODULE_ID 1
#define MODBUS_MODULE_ERR ((1<<31)|(MODBUS_MODULE_ID<<24))
#define MODBUS_ERR(errno) (MODBUS_MODULE_ERR|errno)

#define E_MODBUS_ILLEGAL_FUNCTION (1UL<<0)
#define E_MODBUS_ILLEGAL_DATA_ADDRESS (1UL<<1)
#define E_MODBUS_ILLEGAL_DATA_VALUE (1UL<<2)
#define E_MODBUS_SLAVE_DEVICE_FAILURE (1UL<<3) 
#define E_MODBUS_ACKNOWLEDGE (1UL<<4)
#define E_MODBUS_SLAVE_BUSY (1UL<<5)
#define E_MODBUS_MEMORY_PARITY_ERROR (1UL<<6)
#define E_MODBUS_GATEWAY_PATH_FAILED (1UL<<7)
#define E_MODBUS_GATEWAY_TGT_FAILED (1UL<<8)
#define E_MODBUS_SEND_FSR_BUSY (1UL<<9)

#define E_MODBUS_RECV_CRC (1UL<<16)
#define E_MODBUS_RECV_NODATA (1UL<<17)
#define E_MODBUS_RECV_NOACK (1UL<<18)
#define E_MODBUS_RECV_FRAME_LEN (1UL<<19)

#define MB_SER_PDU_SIZE_MIN     4       /*!< Minimum size of a Modbus RTU frame. */
#define MB_SER_PDU_SIZE_MAX     256     /*!< Maximum size of a Modbus RTU frame. */
#define MB_SER_PDU_SIZE_CRC     2       /*!< Size of CRC field in PDU. */

#ifdef MB_DEBUG
#define DEBUG_MODBUS
#endif

typedef enum 
{
	MB_CH_SERIAL,
	MB_CH_NETWORK
}mb_ch_type_e;

struct mb_write_param
{
	mb_ch_type_e mb_ch_type;
	int port;              
    uint8_t slave_addr;      
    uint16_t start_addr;              
    uint16_t reg_cnt;
	uint8_t *send_buf;
};

struct mb_read_param 
{
	mb_ch_type_e mb_ch_type;
	int port;             
	uint8_t slave_addr;      
	uint16_t start_addr;     
	uint16_t reg_cnt;          
	uint8_t *recv_buf;          
	uint16_t recv_len;
};

int mb_init(void);
int mb_write(struct mb_write_param *pparam);
int mb_read(struct mb_read_param *pparam);
/* only for serial port */
int mb_init_port(uint8_t port);
int mb_config_port(uint8_t port, uint8_t parity, uint8_t stop_bits);
int mb_set_port_baud(uint8_t port, uint32_t baud_rate);

#endif