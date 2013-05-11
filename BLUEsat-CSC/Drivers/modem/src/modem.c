/**
 *  \file modem.c
 *
 *  \brief The modem driver that handles read and write using gpio
 *
 *  \author $Author: Sam Jiang$
 *  \version 1.01
 *
 *  $Date: 2012-05-12 16:58:58 +1100 (Sat, 12 May 2012) $
 *  \warning No Warnings for now
 *  \bug No Bugs for now
 *  \note Only have write function. Uses timer interrupt for 1200 bps
 */


#include "FreeRTOS.h"
#include "lpc24xx.h"
#include "modem.h"
#include "irq.h"
#include "gpio.h"
#include "semphr.h"
/*-----------------------------------------------------------*/
/* Cyclic buffer to store characters waiting to be
transmitted. */
static char TX_BUFF[BUFFER_SIZE];
static int TX_BUFF_SP = 0; //Start position
static int TX_BUFF_EP = 0; //End position
static int TX_BUFF_BC = 0; //bit count
static char buffer = 0; //current output

void Comms_Modem_Timer_Handler(void);
void Comms_Modem_Timer_Wrapper( void ) __attribute__ ((naked));

static xSemaphoreHandle modem_MUTEX;

static int createModem_Semaphore(void)
{
	vSemaphoreCreateBinary( modem_MUTEX );
	if(!modem_MUTEX) return pdFAIL;
	xSemaphoreTake( modem_MUTEX, 1 );
	return pdTRUE;
}

void modem_takeSemaphore(void)
{
	xSemaphoreTake( modem_MUTEX, 2000 );
}

void modem_giveSemaphore(void)
{
	signed portBASE_TYPE i;
	xSemaphoreGiveFromISR(modem_MUTEX,&i);
}

/*-------------------------------------------------------------*/
//Initialisation and Interrupt Handler

void Comms_Modem_Timer_Handler(void)
{
	if (((TX_BUFF[TX_BUFF_SP]&(0x1<<TX_BUFF_BC))>>TX_BUFF_BC)== 0) {
		buffer = !buffer;
		setGPIO(0,15,buffer);
		setGPIO(4,22,buffer);
	} else {
		setGPIO(0,15,buffer);
		setGPIO(4,22,buffer);
	}
	if (TX_BUFF_BC == 7){
		TX_BUFF_BC = 0;
		TX_BUFF_SP = (TX_BUFF_SP+1)%BUFFER_SIZE;
		if (TX_BUFF_SP == TX_BUFF_EP){
			modem_giveSemaphore();
			disable_VIC_irq(MODEM_INTERRUPTS);
		}
	} else {
		TX_BUFF_BC++;
	}

	T1IR = 0xFF;
	/* Clear the ISR in the VIC. */
	VICVectAddr = CLEAR_VIC_INTERRUPT;
}


void Comms_Modem_Timer_Wrapper( void )
{
	portSAVE_CONTEXT();		// Save the context
	Comms_Modem_Timer_Handler();
	portRESTORE_CONTEXT(); 	// Restore the context
}

void Comms_Modem_Timer_Init(void)
{
	// step 1: turn on the power for timer 1
	PCONP = PCONP | (0x1 << 2);

	// step 2: set the peripheral clock to CCLK / 4
	PCLKSEL1 = PCLKSEL1 & (~(0x3 << 2));

	// step 3: not set as we are not going to use external output for timer

	// step 4: interrupts
	// set the condition for interrupt
	T1MR0 = 1;
	// enable interrupt on MR0I, disable other features
	T1MCR = 3;
	// enable interrupt
	install_irq(MODEM_INTERRUPTS, Comms_Modem_Timer_Wrapper,HIGHEST_PRIORITY );
	//enable_VIC_irq(MODEM_INTERRUPTS);


	// set the timer 1 to timer mode
	T1CTCR = T1CTCR & (~(0x3));
	//T1PR will overflow every 1/1200 seconds;
	T1PR = (9000000)/1200-1;
	// reset timer and prescaler counter
	T1PC = 0;
	//clear the interrupt
	T1IR = 0;
	// enable the timer
	T1TCR = T1TCR | (0x3);//turn on tc and pc, reset both
	T1TCR = T1TCR & (~(0x2));//turn off reseting
	//Set GIOP directions
	//set AFSK 1 TX port
	setGPIOdir(4,22,1);
	setGPIOdir(0,15,1);
	//set M0 amd M1 ports for AFSK2
	setGPIOdir(0,18,1);
	setGPIOdir(0,19,1);
	//set M0 amd M1 ports for AFSK1
	setGPIOdir(0,21,1);
	setGPIOdir(0,22,1);
	createModem_Semaphore();
}

/*
signed portBASE_TYPE Comms_Modem_Read_Char( signed portCHAR *pcRxedChar, portTickType xBlockTime, portSHORT sel)
{
	if (sel == 1) {
		if( xQueueReceive( RX_BUFF_1, pcRxedChar, xBlockTime ) )
		{
			return pdTRUE;
		}
		else
		{
			return pdFALSE;
		}
	} else {
		if( xQueueReceive( RX_BUFF_2, pcRxedChar, xBlockTime ) )
		{
			return pdTRUE;
		}
		else
		{
			return pdFALSE;
		}
	}
}
*/

signed portBASE_TYPE Comms_Modem_Write_Char( portCHAR cOutChar, portTickType xBlockTime)
{
	(void) xBlockTime;
	TX_BUFF[TX_BUFF_EP] = cOutChar;
	TX_BUFF_EP = (TX_BUFF_EP+1)%BUFFER_SIZE;
	return 0;
}

/*

unsigned portSHORT Comms_Modem_Read_Str( portCHAR * pcString, unsigned portSHORT usStringLength, portTickType xBlockTime, portSHORT sel  )
{pcInputBuf
	portCHAR * tmp;
	unsigned portSHORT count;
	( void ) usStringLength;
	for (tmp=pcString, count=0;count<usStringLength;++count,++tmp ){
		Comms_Modem_Read_Char( (signed portCHAR*)tmp,  xBlockTime, sel );
		if (*tmp=='\r'){
			break;
		}
	}
	*tmp='\0'; //String termination
	return count;
}

*/

void Comms_Modem_Write_Str( const portCHAR * const pcString, unsigned portSHORT usStringLength)
{
	const signed portCHAR *pxNext;
	unsigned portSHORT usLength = 0;
	//signed portBASE_TYPE ret;
	/* Send each character in the string, one at a time. */
	pxNext = ( const signed portCHAR * ) pcString;
	{
		while(usLength < usStringLength)
		{

			Comms_Modem_Write_Char( *pxNext, 1);
			pxNext++;
			usLength++;
		}
	}
	enable_VIC_irq(MODEM_INTERRUPTS);
}

/*-----------------------------------------------------------*/
void Comms_Modem_Write_Hex(const void * const loc, unsigned portSHORT usStringLength)
{
	const portCHAR *temp;
	portSHORT index;
	{
		for (index = usStringLength-1, temp = (const portCHAR *)loc; index >= 0; index--){
			//Comms_Modem_Write_Char( HEX_Convert(MSN(temp[index])), serBLOCK, sel );
			//Comms_Modem_Write_Char( HEX_Convert(LSN(temp[index])), serBLOCK, sel );
		}
	}
}

void setModemTransmit(portSHORT sel)
{
	//Mode select pins for the AFSKK Modems
	//Setting M = 01 sets AFSK to transmit
	if (sel == 1){ //modem 1
		setGPIO(0, 21, 1);
		setGPIO(0, 22, 0);
	} else {
		setGPIO(0, 18, 1);
		setGPIO(0, 19, 0);
	}
}

void setModemReceive(portSHORT sel)
{
	//Mode select pins for the AFSKK Modems
	//Setting M = 10 sets AFSK to receive
	if (sel == 1){ //modem 1
		setGPIO(0, 21, 0);
		setGPIO(0, 22, 1);
	} else {
		setGPIO(0, 18, 0);
		setGPIO(0, 19, 1);
	}
}
