
#ifndef __SHARE_H__
#define __SHARE_H__

extern const char *par[] ;
extern const uint32 baud[];
extern uint32 get_par_sn( char *str);
extern uint8 get_baud_sn( uint32 rate);
extern void binvert( uint8 *str, uint32 len );

#endif