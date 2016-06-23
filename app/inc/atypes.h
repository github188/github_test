/**
 ******************************************************************************
 * @file       types.h
 * @brief      系统类型定义
 * @details    本文件定义系统中用到的所有使用的基本类型
 * @copyright  Copyright(C), 2008-2012,Sanxing Electric Co.,Ltd.
 *
 ******************************************************************************
 */

#ifndef _ATYPES_H_
#define _ATYPES_H_



/*------------------------------------------------------------------------------
Section: Includes
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
Section: Macro Definitions
------------------------------------------------------------------------------*/
#ifndef OK
#define OK      0
#endif
#ifndef ERROR
#define ERROR       (-1)
#endif
#ifndef BOOL
#define BOOL  int8
#endif
#ifndef bool
#define bool  uint8
#endif
#ifndef IMPORT
#define IMPORT extern
#endif
#ifndef LOCAL
#define LOCAL static
#endif
#ifndef FAST
#define FAST  register
#endif

#ifndef NULL
#define NULL 0
#endif

/**
 * IO definitions
 *
 * define access restrictions to peripheral registers
 */

#define     __I     volatile const            /*!< defines 'read only' permissions      */
#define     __O     volatile                  /*!< defines 'write only' permissions     */
#define     __IO    volatile                  /*!< defines 'read / write' permissions   */


/*------------------------------------------------------------------------------
Section: Type Definitions
------------------------------------------------------------------------------*/
typedef char                    char_t;
typedef signed char             int8_t;
typedef signed short            int16_t;
typedef signed int              int32_t;
typedef signed long long        int64_t;
typedef unsigned char           uint8_t;
typedef unsigned short          uint16_t;
typedef unsigned int            uint32_t;
typedef unsigned long long      uint64_t;
typedef float                   float32_t;
typedef double                  float64_t;
typedef long double             float128_t;
typedef enum  boolean
{
   FALSE  = 0,
   TRUE   = 1
}bool_e;
typedef int                 status_t;


typedef signed char         int8;
typedef short int           int16;
typedef int                 int32;
typedef long long           int64;

typedef unsigned char       uint8;
typedef unsigned short int  uint16;
typedef unsigned  int       uint32;
typedef unsigned long long  uint64;

typedef unsigned char       byte;
typedef unsigned char       bits;
typedef unsigned char       uchar;
typedef float               float32;


typedef uint32     size_t;

typedef unsigned char tBoolean;

typedef enum  Boolean
{
   false = 0,
   true  = 1
}boolean;
typedef signed long time_t;



#define WAIT_FOREVER  ((uint32)0)

#define FOREVER for (;;)

#define FAR


#define max(x, y)   (((x) < (y)) ? (y) : (x))
#define min(x, y)   (((x) < (y)) ? (x) : (y))
#define isascii(c)  ((unsigned) (c) <= 0177)
#define toascii(c)  ((c) & 0177)
#define BITS(x,y) (((x)>>(y))&0x01u)   /* 判断某位是否为1 */
#define SETBITS(x,y,n) (x) = (n) ? ((x)|(1 << (y))) : ((x) &(~(1 << (y))));


#ifdef __cplusplus
typedef void        (*OSFUNCPTR) (void *);     /* ptr to function returning int */
typedef int         (*FUNCPTR) (...);     /* ptr to function returning int */
typedef void        (*VOIDFUNCPTR) (...); /* ptr to function returning void */
typedef double      (*DBLFUNCPTR) (...);  /* ptr to function returning double*/
typedef float       (*FLTFUNCPTR) (...);  /* ptr to function returning float */
typedef void (*VOIDFUNCPTRBOOL)(boolean);

#else
typedef void        (*OSFUNCPTR) (void *);     /* ptr to function returning int */
typedef int         (*FUNCPTR) ();     /* ptr to function returning int */
typedef void        (*VOIDFUNCPTR) (); /* ptr to function returning void */
typedef double      (*DBLFUNCPTR) ();  /* ptr to function returning double*/
typedef float       (*FLTFUNCPTR) ();  /* ptr to function returning float */
typedef void (*VOIDFUNCPTRBOOL)(boolean b);

#endif          /* _cplusplus */

typedef union {
    unsigned long longValue;
    unsigned char array[4];
    struct{unsigned short high,low;} shortValue;
    struct{unsigned char highest,higher,middle,low;}charValue;
}U_UINT32;



typedef union
{
     unsigned long LongValue;
     unsigned char Array[4];
     struct{unsigned short High,Low;} IntValue;
     struct{unsigned char Highest,Higher,Middle,Low;}CharValue;
}Long_Char;

typedef int     STATUS;

/*------------------------------------------------------------------------------
Section: Globals
------------------------------------------------------------------------------*/
/* None */

/*------------------------------------------------------------------------------
Section: Function Prototypes
------------------------------------------------------------------------------*/

#endif /*_TYPES_H_ */

/*-------------------------------End of types.h-------------------------------*/
