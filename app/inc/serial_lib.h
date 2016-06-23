#ifndef __SERAIL_LIB_H__
#define __SERIAL_LIB_H__

#include <stdint.h>

#define SERIAL_MODULE_ID 4
#define SERIAL_MODULE_ERR ((1<<31)|(SERIAL_MODULE_ID<<24))
#define SERIAL_ERR(errno) (SERIAL_MODULE_ERR|errno)

#define E_SERIAL_CONFIG_PORT (1UL<<0)
#define E_SERIAL_SET_PORT_BAUD (1UL<<1)
#define E_SERIAL_OPEN_IND_PIN (1UL<<2)

#define SERIAL_PORT_NUMBER 10

#define NO_PARITY 0
#define ODD_PARITY 1
#define EVEN_PARITY 2

#define ONE_STOP_BIT   1
#define TWO_STOP_BITS  2

#define MIN_BAUD_RATE  1200
#define MAX_BAUD_RATE  115200

#ifdef MB_DEBUG
#define DEBUG_SERIAL
#endif

extern uint8_t port_array[SERIAL_PORT_NUMBER];
extern uint8_t ex_port_array[SERIAL_PORT_NUMBER];

int serial_init_data_transfer_indication(void);
void serial_begin_data_transfer(uint8_t port);
void serial_finish_data_transfer(uint8_t port);
int serial_set_port_baud(int usart_fd, int speed);
int serial_config_port(int usart_fd, int databits, int stopbits, int parity);

#endif