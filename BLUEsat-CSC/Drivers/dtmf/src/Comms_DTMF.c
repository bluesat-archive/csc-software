/**
 *  \file Comms_DTMF.c
 *
 *  \brief DTMF driver to be used to interface the UC to the
 *  DTMF Decoder chip.
 *
 *  \author $Author: Colin Tan $
 *  \version 1.0
 *
 *  $Date: 2010-09-11 18:21:52 +1000 (Sat, 11 Sep 2010) $
 *  \warning No Warnings for now
 *  \bug No Bugs for now
 *  \note No Notes for now
 */

/*Ported to lpc2468 by Felix Kam
 *Notes: the DTMF_BUFF will be oversee and handled by Dtmf.c, which worked on the old board
 *For a brief description of the DTMF->command task scheme, see Dtmf.c
 *At 16 Jan 2012, interrupt is working on lpc2468 and raw digits can be printed out
 *At 22 Jan, the scheme is functional
 *At 2 June, the driver is being ported to the CSCR2 new code base
 * */

#include "Comms_DTMF.h"
#include "queue.h"
#include "task.h"
#include "irq.h"
#include "gpio.h"


/* DTMF  Settings*/
#define DTMF_EDGE_SEN            ( ( unsigned portLONG ) 0x08 )
#define DTMF_RISING_EDGE         ( ( unsigned portLONG ) 0x08 )
#define DTMF_CLEAR_INT           ( ( unsigned portLONG ) 0x08 )

#define DTMF_INT_1					 ((unsigned portLONG) 0x1<<5)
#define DTMF_INT_2					 ((unsigned portLONG) 0x1)

#define DTMF_INT_PINS_SIN        ((1<<5)&(1))
#define DTMF_BUFF_LENGTH         ( ( unsigned portLONG ) 20 )//50 Tone long buffer
#define DTMF_BUFF_Empty          ( ( unsigned portLONG ) 0 )

#define DTMF_1_OFFSET         ( ( unsigned portLONG ) 24 )
#define DTMF_2_OFFSET      ( ( unsigned portLONG ) 28 )

#define DTMF_DECODER1            ( ( unsigned portLONG ) 0 )
#define DTMF_DECODER2            ( ( unsigned portLONG ) 1 )

#define IO2IntEnR (*(volatile unsigned long *)(0xE00280B0))//somehow these are not included in lpc24xx.h
#define IO2IntEnF (*(volatile unsigned long *)(0xE00280B4))
#define IO2IntClr (*(volatile unsigned long *)(0xE00280AC))

//xQueueHandle DTMF_BUFF;

void Comms_DTMF_Wrapper(void) __attribute__ ((naked));
void Comms_DTMF_Handler (void);

//unsigned int decoder1value(void);
//unsigned int decoder2value(void);

/**
	eBoolean is the enum for the ::Boolean type.

	\warning Do not use this. Use ::Boolean instead.

	\see ::Boolean
 */
enum eBoolean {
	false = 0,
	true = 1
};

/**
	typedefs the eBoolean enum to a type. This hides the
	internal implementation from the user.

	\see #eBoolean
*/
typedef enum eBoolean Boolean;

DtmfTone tone;
int newData;

void Comms_DTMF_Init(void)
{
	/*Set up Queue and content counter*/
	//DTMF_BUFF = xQueueCreate( DTMF_BUFF_LENGTH, ( unsigned portBASE_TYPE ) sizeof( DtmfTone ) );

	//disable EXT3 interrupt first, noting all P[2] GPIO pins share EXT3 Interrupt
	VICIntEnable &= ~(0x1<<17);

	//Set edge sensitive
	EXTMODE |=  DTMF_EDGE_SEN;
	//Set polarity to rising edge sensitive
	EXTPOLAR  |= DTMF_RISING_EDGE ;

	//set pin 10,15(P2[5,0]) to be GPIO
	set_Gpio_func(2,5,0);
	set_Gpio_func(2,0,0);
	//set (P2[1-4;6-9]) to be GPIO
	set_Gpio_func(2,6,0);
	set_Gpio_func(2,7,0);
	set_Gpio_func(2,8,0);
	set_Gpio_func(2,9,0);

	set_Gpio_func(2,1,0);
	set_Gpio_func(2,2,0);
	set_Gpio_func(2,3,0);
	set_Gpio_func(2,4,0);

	//set all to be input pins
	setGPIOdir(2,5,INPUT);
	setGPIOdir(2,0,INPUT);

	setGPIOdir(2,6,INPUT);
	setGPIOdir(2,7,INPUT);
	setGPIOdir(2,8,INPUT);
	setGPIOdir(2,9,INPUT);

	setGPIOdir(2,1,INPUT);
	setGPIOdir(2,2,INPUT);
	setGPIOdir(2,3,INPUT);
	setGPIOdir(2,4,INPUT);
	//enable rising edge interrupt
	IO2IntEnR |= (DTMF_INT_PINS_SIN);
	IO2IntEnF &=~ DTMF_INT_PINS_SIN;

	EXTINT|= 0x1<<3;//clear the intterupt
	VICVectAddr =0;
	IO2IntClr = DTMF_INT_PINS_SIN;//clear the GPIO interrupts
	VICIntEnable |= 0x1<<17;//enable VIC interrupt for EXT3
	install_irq(17, Comms_DTMF_Wrapper, HIGHEST_PRIORITY );
	enable_VIC_irq(17);
}


/*
unsigned int decoder1value(){
	if(!(FIO2PIN&DTMF_INT_1)) return 0x28;// it's not ready
	return (FIO1PIN&(0xF<<24))>>24;
}

unsigned int decoder2value(){
	//no interrupt on decoder 1, it's not ready, return 16 to indicate that
	if(!(FIO2PIN&(DTMF_INT_2))) return 0x28;// it's not ready
	return (FIO1PIN&(0xF<<28))>>28;
}
*/



//Spurious Interrupts not handled as not expected since the interrupt is edge sensitive

/*
 * /brief Interrupt service handler function. Determines which decoder has found something
 *        and places it onto the queue. Tone information pushed onto the queue is of:
 *                                  [data 0-3][decoder [4-7]]
 *        The code checks the first decoder again after checking the second decoder if the
 *        first decoder was not ready the first time. This is to cater for the case where by
 *        the first decoder becomes ready while the second decoder is being processed.
 *
 * /note  Queues don't block if there is no room. Also they don't directly wake a task that
 *        was blocked on a queue read, instead they return a flag to say whether a context
 *        switch is required or not (i.e. has a task with a higher priority than us been
 *        woken by this post).
 */

void Comms_DTMF_Handler (void)
{
	//ported to lpc2468, this ought to work
	//Interrupts have been disabled should check for decoders before and after pulling data
   portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
//   char s[3];
   Boolean bPriDecodeFailed = true;
   if (FIO2PIN&DTMF_INT_PINS_SIN)//Will only be necessary/have effect if we have other GPIO Interrupt sources
   {//One decoder has decoded something
      if (FIO2PIN&(DTMF_INT_1))
      {//Check if the First decoder is ready
         tone.decoder = DTMF_DECODER1;
         tone.tone = (getGPIO(2,9))+(getGPIO(2,8)<<1)+(getGPIO(2,7)<<2)+(getGPIO(2,6)<<3);
         newData = 1;
         //xQueueSendFromISR(DTMF_BUFF,&tone,&xHigherPriorityTaskWoken);
         bPriDecodeFailed = false;
         //Comms_UART_Write_Str("a",1 );k

       }
      if (FIO2PIN&(DTMF_INT_2))
      {//Check if the second decoder is ready
         tone.decoder = DTMF_DECODER2;
         tone.tone = (getGPIO(2,4))+(getGPIO(2,3)<<1)+(getGPIO(2,2)<<2)+(getGPIO(2,1)<<3);
         //xQueueSendFromISR(DTMF_BUFF,&tone,&xHigherPriorityTaskWoken);
     	//Comms_UART_Write_Str("b",1 );

      }
      if ((FIO2PIN&(DTMF_INT_1)) && bPriDecodeFailed)
      {// If the first decoder was not ready try it again
         tone.decoder = DTMF_DECODER1;
         tone.tone = (getGPIO(2,9))+(getGPIO(2,8)<<1)+(getGPIO(2,7)<<2)+(getGPIO(2,6)<<3);
         //xQueueSendFromISR(DTMF_BUFF,&tone,&xHigherPriorityTaskWoken);
     	//Comms_UART_Write_Str("c",1 );

      }
  }

   if( xHigherPriorityTaskWoken )
   {
      portYIELD_FROM_ISR();
   }
	IO2IntClr = DTMF_INT_PINS_SIN;//clear the GPIO interrupts
	EXTINT|= 0x1<<3;//clear the intterupt
	VICVectAddr =0;

}

void Comms_DTMF_Wrapper(void)
{
   portSAVE_CONTEXT();     // Save the context
   Comms_DTMF_Handler();
   portRESTORE_CONTEXT();  // Restore the context
}

/*signed portBASE_TYPE*/void Comms_DTMF_Read( DtmfTone *DTMF_elem, int *new, portTickType xBlockTime )
{
	(void) xBlockTime;
	*new = newData;
	*DTMF_elem = tone;
	newData = 0;
//   return xQueueReceive(DTMF_BUFF,DTMF_elem,xBlockTime);
}

