/******************************************************************************
 *   irq.h:  Interrupt related Header file for NXP LPC230x Family
 *   Microprocessors
 *
 *   Copyright(C) 2006, NXP Semiconductor
 *   All rights reserved.
 *
 *   History
 *   2006.09.01  ver 1.00    Prelimnary version, first Release
 *
******************************************************************************/
#ifndef __IRQ_H
#define __IRQ_H
#ifndef INC_FREERTOS_H
   #include "FreeRTOS.h"
#endif
#define I_Bit			0x80
#define F_Bit			0x40

#define SYS32Mode		0x1F
#define IRQ32Mode		0x12
#define FIQ32Mode		0x11

#define HIGHEST_PRIORITY	0x01
#define NORMAL_PRIORITY		0x05
#define LOWEST_PRIORITY		0x0F



/* Be aware that, from compiler to compiler, nested interrupt will have to
be handled differently. More details can be found in Philips LPC2000
family app-note AN10381 */

/* unlike Keil CARM Compiler, in ARM's RealView compiler, don't save and
restore registers into the stack in RVD as the compiler does that for you.
See RVD ARM compiler Inline and embedded assemblers, "Rules for
using __asm and asm keywords. */

#define IENABLE __asm { MRS sysreg, SPSR; MSR CPSR_c, #SYS32Mode }
#define IDISABLE __asm { MSR CPSR_c, #(IRQ32Mode|I_Bit); MSR SPSR_cxsf, sysreg }

extern void init_VIC( void );
extern portLONG install_irq( portLONG IntNumber, void *HandlerAddr, portLONG Priority );
extern void enable_VIC_irq(portLONG IntNumber);
extern void disable_VIC_irq(portLONG IntNumber);

#endif /* end __IRQ_H */

/******************************************************************************
**                            End Of File
******************************************************************************/
