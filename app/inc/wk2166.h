#ifndef	__WK2166_H__      
#define __WK2166_H__

#define 	WK2166_GENA     0X00
#define 	WK2166_GRST     0X01
#define		WK2166_GMUT     0X02
#define 	WK2166_GIER     0X10
#define 	WK2166_GIFR     0X11
#define 	WK2166_GPDIR    0X21
#define 	WK2166_GPDAT    0X31
//Global rigister address of port
#define		WK2166_USART2_GPORT    1
#define		WK2166_USART3_GPORT    (WK2166_USART2_GPORT+4)

#define     WK2166_WRITE_REG    0
#define     WK2166_READ_REG     1
#define     WK2166_WRITE_FIFO   2
#define     WK2166_READ_FIFO    3
#define 	WK2166_PAGE1    1
#define 	WK2166_PAGE0    0

//wkxxxx  slave uarts  rigister address defines

#define 	WK2166_SPAGE    0X03
//PAGE0
#define 	WK2166_SCR      0X04
#define 	WK2166_LCR      0X05
#define 	WK2166_FCR      0X06
#define 	WK2166_SIER     0X07
#define 	WK2166_SIFR     0X08
#define 	WK2166_TFCNT    0X09
#define 	WK2166_RFCNT    0X0A
#define 	WK2166_FSR      0X0B
#define 	WK2166_LSR      0X0C
#define 	WK2166_FDAT     0X0D
#define 	WK2166_FWCR     0X0E
#define 	WK2166_RS485    0X0F
//PAGE1
#define 	WK2166_BAUD1    0X04
#define 	WK2166_BAUD0    0X05
#define 	WK2166_PRES     0X06
#define 	WK2166_RFTL     0X07
#define 	WK2166_TFTL     0X08
#define 	WK2166_FWTH     0X09
#define 	WK2166_FWTL     0X0A
#define 	WK2166_XON1     0X0B
#define 	WK2166_XOFF1    0X0C
#define 	WK2166_SADR     0X0D
#define 	WK2166_SAEN     0X0E
#define 	WK2166_RTSDLY   0X0F

//wkxxx register bit defines
// GENA
#define 	WK2166_UT4EN	0x08
#define 	WK2166_UT3EN	0x04
#define 	WK2166_UT2EN	0x02
#define 	WK2166_UT1EN	0x01
//GRST
#define 	WK2166_UT4SLEEP	0x80
#define 	WK2166_UT3SLEEP	0x40
#define 	WK2166_UT2SLEEP	0x20
#define 	WK2166_UT1SLEEP	0x10
#define 	WK2166_UT4RST	0x08
#define 	WK2166_UT3RST	0x04
#define 	WK2166_UT2RST	0x02
#define 	WK2166_UT1RST	0x01
//GIER
#define 	WK2166_UT4IE	0x08
#define 	WK2166_UT3IE	0x04
#define 	WK2166_UT2IE	0x02
#define 	WK2166_UT1IE	0x01
//GIFR
#define 	WK2166_UT4INT	0x08
#define 	WK2166_UT3INT	0x04
#define 	WK2166_UT2INT	0x02
#define 	WK2166_UT1INT	0x01
//SPAGE
#define 	WK2166_SPAGE0	0x00
#define 	WK2166_SPAGE1   0x01
//SCR
#define 	WK2166_SLEEPEN	0x04
#define 	WK2166_TXEN     0x02
#define 	WK2166_RXEN     0x01
//LCR
#define 	WK2166_BREAK	0x20
#define 	WK2166_IREN     0x10
#define 	WK2166_PAEN     0x08
#define 	WK2166_PAM1     0x04
#define 	WK2166_PAM0     0x02
#define 	WK2166_STPL     0x01
//FCR
#define 	WK2166_TFEN   0x08
#define 	WK2166_RFEN   0x04
#define 	WK2166_TFRST  0x02
#define 	WK2166_RFRST  0x01
//SIER
#define 	WK2166_FERR_IEN      0x80
#define 	WK2166_CTS_IEN       0x40
#define 	WK2166_RTS_IEN       0x20
#define 	WK2166_XOFF_IEN      0x10
#define 	WK2166_TFEMPTY_IEN   0x08
#define 	WK2166_TFTRIG_IEN    0x04
#define 	WK2166_RXOUT_IEN     0x02
#define 	WK2166_RFTRIG_IEN    0x01
//SIFR
#define 	WK2166_FERR_INT      0x80
#define 	WK2166_CTS_INT       0x40
#define 	WK2166_RTS_INT       0x20
#define 	WK2166_XOFF_INT      0x10
#define 	WK2166_TFEMPTY_INT   0x08
#define 	WK2166_TFTRIG_INT    0x04
#define 	WK2166_RXOVT_INT     0x02
#define 	WK2166_RFTRIG_INT    0x01

//TFCNT
//RFCNT
//FSR
#define 	WK2166_RFOE     0x80
#define 	WK2166_RFBI     0x40
#define 	WK2166_RFFE     0x20
#define 	WK2166_RFPE     0x10
#define 	WK2166_RDAT     0x08
#define 	WK2166_TDAT     0x04
#define 	WK2166_TFULL    0x02
#define 	WK2166_TBUSY    0x01
//LSR
#define 	WK2166_OE       0x08
#define 	WK2166_BI       0x04
#define 	WK2166_FE       0x02
#define 	WK2166_PE       0x01
//FWCR
//RS485
#define     WK2166_RS485_MODE  0x40
#define     WK2166_RTS_ENABLE  0x02
#define     WK2166_RTS_LOW   0x01

#define 	NR_PORTS 	4
#define		PORT_WK2166 1

#endif

