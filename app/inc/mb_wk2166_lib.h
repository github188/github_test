#ifndef __WK2166_LIB_H__
#define __WK2166_LIB_H__

#include <stdint.h>

#define WK2166_MODULE_ID 2
#define WK2166_MODULE_ERR ((1<<31)|(WK2166_MODULE_ID<<24))
#define WK2166_ERR(errno) (WK2166_MODULE_ERR|errno)

#define E_WK2166_OPEN_HW_RESET_PIN (1UL<<0)
#define E_WK2166_OPEN_USART2 (1UL<<1)
#define E_WK2166_OPEN_USART3 (1UL<<2) 
#define E_WK2166_MATCH_BAUD_RATE (1UL<<3)

#define E_WK2166_SEND_FSR_FIFO_FULL (1UL<<8)
#define E_WK2166_SEND_FSR_BUSY (1UL<<9)
#define E_WK2166_RECV_LSR_PE (1UL<<10)
#define E_WK2166_RECV_LSR_FE (1UL<<11)
#define E_WK2166_RECV_LSR_OE (1UL<<12)
#define E_WK2166_RECV_LSR_BI (1UL<<13)

#define SEND_FIFO_MODE
#define DATA_TRANS_IND 
//#define WK2166_INTERRUPT_MODE

#ifdef MB_DEBUG
#define DEBUG_MB_WK2166
#endif

/* main port init & config */
int usart_wk2166_init(void);
#ifdef WK2166_INTERRUPT_MODE
void usart2_wk2166_irq_bh(void);
void usart3_wk2166_irq_bh(void);
#endif

/* sub port init & config */
void wk2166_init_subport(uint8_t ex_port);
void wk2166_config_subport(uint8_t ex_port, uint8_t parity, uint8_t stop_bits);
void wk2166_set_subport_baud(uint8_t ex_port, uint32_t baud_rate);

/* sub port send & receive */
int wk2166_tx_chars(uint8_t port, const uint8_t* buf, uint8_t len);
int wk2166_rx_chars(uint8_t port, uint8_t* buf, uint8_t len);

/* for debug */
void wk2166_read_reg_all(uint8_t ex_port);

#endif
