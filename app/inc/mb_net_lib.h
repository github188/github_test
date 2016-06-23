#ifndef __MB_NET_LIB_H__
#define __MB_NET_LIB_H__

#include <stdint.h>

#define MB_NET_MODULE_ID 5
#define MB_NET_MODULE_ERR ((1<<31)|(MB_NET_MODULE_ID<<24))
#define MB_NET_ERR(errno) (MB_NET_MODULE_ERR|errno)

#define E_MB_NET_SEND (1UL<<0)
#define E_MB_NET_RECV (1UL<<1)

#ifdef MB_DEBUG
#define DEBUG_MB_NET
#endif

int mb_net_tx_chars(int fd, const uint8_t* buf, uint8_t len);
int mb_net_rx_chars(int fd, uint8_t* buf, uint8_t len);

#endif