#ifndef __WK2166_LIB_H__
#define __WK2166_LIB_H__

#include <stdint.h>

#define WK2166_MODULE_ID 1
#define WK2166_MODULE_ERR ((1<<31)|(WK2166_MODULE_ID<<24))
#define WK2166_ERR(errno) (WK2166_MODULE_ERR|errno)

#define E_WK2166_OPEN_HW_RESET_PIN (1)
#define E_WK2166_OPEN_USART2 (1UL<<1)
#define E_WK2166_OPEN_USART3 (1UL<<2)
#define E_WK2166_OPEN_IND_PIN (1UL<<3) 
#define E_WK2166_CONFIG_MAIN_PORT (1UL<<4)
#define E_WK2166_SET_MAIN_PORT_BAUD (1UL<<5)
#define E_WK2166_MATCH_BAUD_RATE (1UL<<6)

#define E_WK2166_SEND_FSR_FIFO_FULL (1UL<<8)
#define E_WK2166_SEND_FSR_BUSY (1UL<<9)
#define E_WK2166_RECV_LSR_PE (1UL<<10)
#define E_WK2166_RECV_LSR_FE (1UL<<11)
#define E_WK2166_RECV_LSR_OE (1UL<<12)
#define E_WK2166_RECV_LSR_BI (1UL<<13)

#define NO_PARITY 0
#define ODD_PARITY 1
#define EVEN_PARUTY 2

#define ONE_STOP_BIT   0
#define TWO_STOP_BITS  1

#define PORT_NUMBER 8
#define SEND_FIFO_MODE
#define DATA_TRANS_IND 
//#define WK2166_INTERRUPT_MODE
#define DEBUG_WK2166
//#define PROFILE_WK2166

/* sub port send & receive */
int wk2xxx_rx_chars(uint8_t port, uint8_t* buf, uint8_t* plen);
int wk2xxx_tx_chars(uint8_t port, uint8_t* buf, uint8_t len);

/* sub port init & config */
void wk2xxx_init_subport(uint8_t ex_port);
void wk2xxx_config_subport(uint8_t ex_port, uint8_t parity, uint8_t stop_bits);
void wk2xxx_set_subport_baud(uint8_t ex_port, uint32_t baud_rate);

/* for debug */
void wk2xxx_read_reg_all(uint8_t ex_port);

/* main port init & config */
int usart_wk2166_init(void);
int wk2xxx_hw_reset(void);
int usart2_wk2xxx_init(void);
int usart3_wk2xxx_init(void);
#ifdef WK2166_INTERRUPT_MODE
void usart2_wk2xxx_irq_bh(void);
void usart3_wk2xxx_irq_bh(void);
#endif

#endif
