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
static char TX_BUFF_1[BUFFER_SIZE];
static int TX_BUFF_1_SP = 0;
static int TX_BUFF_1_EP = 0;
static int TX_BUFF_1_BC = 0;
static volatile portLONG Modem_1_FREE;
static char TX_BUFF_2[BUFFER_SIZE];
static int TX_BUFF_2_SP = 0;
static int TX_BUFF_2_EP = 0;
static volatile portLONG Modem_2_FREE;
static char buffer = 0;
void Comms_Modem_Timer_Handler(void);
void Comms_Modem_Timer_Wrapper( void ) __attribute__ ((naked));

static xSemaphoreHandle modem_1_MUTEX;
static xSemaphoreHandle modem_2_MUTEX;

static int createModem_Semaphore(void)
{
	vSemaphoreCreateBinary( modem_1_MUTEX );
	if(!modem_1_MUTEX) return pdFAIL;
	vSemaphoreCreateBinary( modem_2_MUTEX );
	if(!modem_2_MUTEX) return pdFAIL;
	return pdTRUE;
}

void modem_takeSemaphore(unsigned char modem)
{
	if (modem == MODEM_1){
		xSemaphoreTake( modem_1_MUTEX, MODEM_NO_BLOCK );
	} else {
		xSemaphoreTake( modem_2_MUTEX, MODEM_NO_BLOCK );
	}
}

void modem_giveSemaphore(unsigned char modem)
{
	if (modem == MODEM_1){
		xSemaphoreGive(modem_1_MUTEX);
	} else {
		xSemaphoreGive(modem_2_MUTEX);
	}
}

/*-------------------------------------------------------------*/
//Initialisation and Interrupt Handler

void Comms_Modem_Timer_Handler(void)
{
	if (((TX_BUFF_1[TX_BUFF_1_SP]&(0x1<<TX_BUFF_1_BC))>>TX_BUFF_1_BC)== 0) {
		buffer = !buffer;
		setGPIO(3,16,buffer);
	} else {
		setGPIO(3,16,buffer);
	}
	if (TX_BUFF_1_BC == 7){
		TX_BUFF_1_BC = 0;
		TX_BUFF_1_SP = (TX_BUFF_1_SP+1)%BUFFER_SIZE;
		Modem_1_FREE = 1;
		if (TX_BUFF_1_SP == TX_BUFF_1_EP){
			disable_VIC_irq(MODEM_INTERRUPTS);

		}
	} else {
		TX_BUFF_1_BC++;
	}
	//setGPIO(2,11,TX_BUFF_1[TX_BUFF_1_SP]);
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
	enable_VIC_irq(MODEM_INTERRUPTS);


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
	setGPIOdir(3,16,1);
	//set M0 amd M1 ports for AFSK2
	setGPIOdir(2,6,1);
	setGPIOdir(2,7,1);
	//set M0 amd M1 ports for AFSK1
	setGPIOdir(2,2,1);
	setGPIOdir(2,3,1);
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

signed portBASE_TYPE Comms_Modem_Write_Char( portCHAR cOutChar, portTickType xBlockTime, portSHORT sel )
{
	(void) xBlockTime;
	//enable_VIC_irq(MODEM_INTERRUPTS);
	//block when it is not free
	if (sel == 1){
		TX_BUFF_1[TX_BUFF_1_EP] = cOutChar;
		TX_BUFF_1_EP = (TX_BUFF_1_EP+1)%BUFFER_SIZE;
		if (TX_BUFF_1_EP == TX_BUFF_1_SP){
			Modem_1_FREE = 0;
		}
	} else {
		TX_BUFF_2[TX_BUFF_2_EP] = cOutChar;
		TX_BUFF_2_EP = (TX_BUFF_2_EP+1)%BUFFER_SIZE;
		if (TX_BUFF_2_EP == TX_BUFF_2_SP){
			Modem_2_FREE = 0;
		}
	}
	return 0;
}

/*

unsigned portSHORT Comms_Modem_Read_Str( portCHAR * pcString, unsigned portSHORT usStringLength, portTickType xBlockTime, portSHORT sel  )
{
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

void Comms_Modem_Write_Str( const portCHAR * const pcString, unsigned portSHORT usStringLength, portSHORT sel )
{
	const signed portCHAR *pxNext;
	unsigned portSHORT usLength = 0;
	setModemTransmit(sel);
	//signed portBASE_TYPE ret;
	/* Send each character in the string, one at a time. */
	pxNext = ( const signed portCHAR * ) pcString;
	{
		while( *pxNext && (usLength < usStringLength))
		{

			Comms_Modem_Write_Char( *pxNext, 1, sel );
			pxNext++;
			usLength++;
		}
	}
	enable_VIC_irq(MODEM_INTERRUPTS);
}

/*-----------------------------------------------------------*/
void Comms_Modem_Write_Hex(const void * const loc, unsigned portSHORT usStringLength,portSHORT sel)
{
	(void)sel;
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
		FIO2SET0 = (0x01 << (2));
		FIO2CLR0 = (0x01 << (3));
	} else {
		FIO2SET0 = (0x01 << (6));
		FIO2CLR0 = (0x01 << (7));
	}
}

void setModemReceive(portSHORT sel)
{
	//Mode select pins for the AFSKK Modems
	//Setting M = 10 sets AFSK to receive
	if (sel == 1){ //modem 1
		FIO2SET0 = (0x01 << (3));
		FIO2CLR0 = (0x01 << (2));
	} else {
		FIO2SET0 = (0x01 << (7));
		FIO2CLR0 = (0x01 << (6));
	}
}
