#ifndef __WK2166_IRQ_H__
#define __WK2166_IRQ_H__

#define WK2166_INT_MAGIC 'G'
#define WK2166_INFO_USER_PID _IOW(WK2166_INT_MAGIC, 4, int) 
#define WK2166_POLL_INT      _IOW(WK2166_INT_MAGIC, 5, int)

#endif