#ifndef __RS232_LIB_H__
#define __RS232_LIB_H__

#include <stdint.h>

#define RS232_MODULE_ID 3
#define RS232_MODULE_ERR ((1<<31)|(RS232_MODULE_ID<<24))
#define RS232_ERR(errno) (RS232_MODULE_ERR|errno)

#define E_RS232_OPEN_USART0 (1UL<<0)
#define E_RS232_OPEN_USART1 (1UL<<1)
#define E_RS232_SEND (1UL<<2)
#define E_RS232_RECV (1UL<<3)

#ifdef MB_DEBUG
#define DEBUG_MB_RS232
#endif

int rs232_init_port(uint8_t port);
int rs232_set_port_baud(uint8_t port, uint32_t baud_rate);
int rs232_config_port(uint8_t port, uint8_t parity, uint8_t stop_bits);
int rs232_tx_chars(uint8_t port, const uint8_t* buf, uint8_t len);
int rs232_rx_chars(uint8_t port, uint8_t* buf, uint8_t len);

#endif