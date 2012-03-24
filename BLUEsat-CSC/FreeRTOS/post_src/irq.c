/*****************************************************************************
 *   irq.c: Interrupt handler C file for NXP LPC230x Family Microprocessors
 *
 *   Copyright(C) 2006, NXP Semiconductor
 *   All rights reserved.
 *
 *   History
 *   2006.07.13  ver 1.00    Prelimnary version, first Release
 *
******************************************************************************/
#include "lpc24xx.h"			/* LPC23XX Peripheral Registers	*/
#include "irq.h"

/* Initialize the interrupt controller */
/******************************************************************************
** Function name:		init_VIC
**
** Descriptions:		Initialize VIC interrupt controller.
** parameters:			None
** Returned value:		None
**
******************************************************************************/
#define VIC_SIZE 32
#define FALSE 0
#define TRUE 1
#define VECT_ADDR_INDEX 0x100
#define VECT_CNTL_INDEX 0x200
#define OFFSET 4
void init_VIC(void)
{
    portLONG i = 0;
    portLONG *vect_addr, *vect_cntl;

    /* initialize VIC*/
    VICIntEnClr = 0xffffffff;
    VICVectAddr = 0;
    VICIntSelect = 0;

    /* set all the vector and vector control register to 0 */
    for ( i = 0; i < VIC_SIZE; i++ )
    {
		vect_addr = (portLONG *)(VIC_BASE_ADDR + VECT_ADDR_INDEX + i*OFFSET);
		vect_cntl = (portLONG *)(VIC_BASE_ADDR + VECT_CNTL_INDEX + i*OFFSET);
		*vect_addr = 0x0;
		*vect_cntl = 0xF;
    }
    return;
}

/******************************************************************************
** Function name:		install_irq
**
** Descriptions:		Install interrupt handler
** parameters:			Interrupt number, interrupt handler address,
**						interrupt priority
** Returned value:		true or false, return false if IntNum is out of range
**
******************************************************************************/
portLONG install_irq( portLONG IntNumber, void *HandlerAddr, portLONG Priority )
{
    portLONG *vect_addr;
    portLONG *vect_cntl;

    VICIntEnClr = 1 << IntNumber;	/* Disable Interrupt */
    if ( IntNumber >= VIC_SIZE )
    {
		return ( FALSE );
    }
    else
    {
		/* find first un-assigned VIC address for the handler */
		vect_addr = (portLONG *)(VIC_BASE_ADDR + VECT_ADDR_INDEX + IntNumber*OFFSET);
		vect_cntl = (portLONG *)(VIC_BASE_ADDR + VECT_CNTL_INDEX + IntNumber*OFFSET);
		*vect_addr = (portLONG)HandlerAddr;	/* set interrupt vector */
		*vect_cntl = Priority;
		return( TRUE );
    }
}
/******************************************************************************
** Function name:		enable_VIC_irq
**
** Descriptions:		enable interrupt handler
** parameters:			Interrupt number
** Returned value:		none
**
******************************************************************************/

void enable_VIC_irq(portLONG IntNumber)
{
	VICIntEnable = 0x01 << IntNumber;	/* Enable Interrupt */
}

/******************************************************************************
** Function name:		disable_VIC_irq
**
** Descriptions:		disable interrupt handler
** parameters:			Interrupt number
** Returned value:		none
**
******************************************************************************/

void disable_VIC_irq(portLONG IntNumber)
{
	VICIntEnClr = 0x01 << IntNumber;	/* disable Interrupt */
}
/******************************************************************************
**                            End Of File
******************************************************************************/
