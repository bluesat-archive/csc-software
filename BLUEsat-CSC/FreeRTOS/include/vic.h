/*
 * VIC_Helper.h
 *
 *  Created on: 8/06/2009
 *      Author: colin
 *
 *   This is a set of helper functions for the LPC2486.
 */

#ifndef VIC_HELPER_H_
#define VIC_HELPER_H_

#define FIQ_CLEAR		0
#define FIQ_SET			1

#define HIGHEST_PRIORITY	0x01
#define MID_PRIORITY		0x05
#define LOWEST_PRIORITY		0x0F

#define	WDT_INT			0
#define SWI_INT			1
#define ARM_CORE0_INT	2
#define	ARM_CORE1_INT	3
#define	TIMER0_INT		4
#define TIMER1_INT		5
#define UART0_INT       6
#define	UART1_INT		7
#define	PWM0_1_INT		8
#define I2C0_INT	    9
#define SPI0_INT		10			/* SPI and SSP0 share VIC slot */
#define SSP0_INT		10
#define	SSP1_INT		11
#define	PLL_INT			12
#define RTC_INT			13
#define EINT0_INT       14
#define EINT1_INT       15
#define EINT2_INT       16
#define EINT3_INT       17
#define	ADC0_INT        18
#define I2C1_INT        19
#define BOD_INT			20
#define EMAC_INT		21
#define USB_INT			22
#define CAN_INT			23
#define MCI_INT			24
#define GPDMA_INT		25
#define TIMER2_INT		26
#define TIMER3_INT		27
#define UART2_INT		28
#define UART3_INT		29
#define I2C2_INT		30
#define I2S_INT			31

#define VIC_SIZE		32

#define VECT_ADDR_INDEX	0x100
#define VECT_PRIO_INDEX 0x200


void VIC_Init (void);
signed portBASE_TYPE  VIC_Install_Irq( unsigned portLONG IntNumber, void *HandlerAddr,unsigned portLONG Priority );

#endif /* VIC_HELPER_H_ */
