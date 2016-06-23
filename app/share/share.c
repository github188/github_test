#include "atypes.h"
#include "maths.h"
#include "gpio.h"

const char *par[] = {"None", "Odd", "Even", "Mark", "Space"};
const uint32 baud[] = {1200, 1800, 2400, 3600, 4800, 7200, 
	9600, 14400, 19200, 28800, 38400, 57600, 76800, 115200};
/*****************************************************************************
 * Function      : get_par_sn
 * Description   : get_par_sn
 * Input         : uint32
 * Output        : None
 * Return        : char *str
 * Others        : 
 * Record
 * 1.Date        : 20151217
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
uint32 get_par_sn( char *str)
{
    uint32 len;
	uint32 idx;

	len = strlen(str);
	for(idx = 0; idx < ARRAY_SIZE(par); idx++)
	{
		if((len == strlen(par[idx]))
			&& (strcmp(str, par[idx]) == 0))
			break;
	}
	if(idx >= ARRAY_SIZE(par))
		idx = 0;

	return idx;
}

/*****************************************************************************
 * Function      : get_baud_sn
 * Description   : get_baud_sn
 * Input         : uint32 rate
 * Output        : None
 * Return        : uint8
 * Others        : 
 * Record
 * 1.Date        : 20151218
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
uint8 get_baud_sn( uint32 rate)
{
    uint32 size = ARRAY_SIZE(baud);
	uint32 idx;

	for(idx = 0; idx < size; idx++)
	{
	    if(baud[idx] == rate)
			break;
	}
	if(idx >= size)
		idx = 6;
	
	return idx;
}

/*****************************************************************************
 * Function      : binvert
 * Description   : ÄÚ´æµ¹ÖÃ
 * Input         : uint8 *str
                uint32 len
 * Output        : None
 * Return        : void
 * Others        : 
 * Record
 * 1.Date        : 20151222
 *   Author      : chenxu
 *   Modification: Created function

*****************************************************************************/
void binvert( uint8 *str, uint32 len )
{
    uint8 temp;
    uint32 left = 0;
	uint32 right = len - 1;

	if(len < 1)
		return;
	while(left < right)
	{
	    temp = str[left];
		str[left] = str[right];
		str[right] = temp;
		left++;
		right--;
	}
}

