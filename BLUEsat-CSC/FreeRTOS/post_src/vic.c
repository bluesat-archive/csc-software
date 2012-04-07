/*
 * VIC_Helper.c
 *
 *  Created on: 8/06/2009
 *      Author: colin
 *
 *   This is a set of helper functions for the LPC2468.
 */

#include "FreeRTOS.h"
#include "vic.h"
#include "lpc24xx.h"


void VIC_Init (void){
	unsigned portLONG i = 0;
	unsigned portLONG *vect_addr, *vect_prio;
	VICIntSelect	=0x0;
	VICIntEnClr		=0xFFFFEFF0;
	VICProtection	=0x0;
	/* set all the vector and vector control register to 0 */
	for ( i = 0; i < VIC_SIZE; i++ )
	{
		vect_addr = (unsigned portLONG *)(VIC_BASE_ADDR + VECT_ADDR_INDEX + i*4);
		vect_prio = (unsigned portLONG *)(VIC_BASE_ADDR + VECT_PRIO_INDEX + i*4);
		*vect_addr = 0x0;
		*vect_prio = 0xF;
	}
}

signed portBASE_TYPE  VIC_Install_Irq( unsigned portLONG IntNumber, void *HandlerAddr,unsigned portLONG Priority )
{
	unsigned portLONG *vect_addr;
	unsigned portLONG *vect_prio;

    VICIntEnClr = 1 << IntNumber;	/* Disable Interrupt */
    if ( IntNumber >= VIC_SIZE )
    {
		return ( pdFALSE );
    }
    else
    {
		/* find first un-assigned VIC address for the handler */
		vect_addr = (unsigned portLONG *)(VIC_BASE_ADDR + VECT_ADDR_INDEX + IntNumber*4);
		vect_prio = (unsigned portLONG *)(VIC_BASE_ADDR + VECT_PRIO_INDEX + IntNumber*4);
		*vect_addr = (unsigned portLONG)HandlerAddr;	/* set interrupt vector */
		*vect_prio = Priority;
		VICIntEnable = 1 << IntNumber;	/* Enable Interrupt */
		return( pdTRUE );
    }
}
