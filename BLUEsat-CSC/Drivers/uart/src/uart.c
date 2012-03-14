/**
 *  \file uart.c
 *
 *  \brief UART Driver to allow the writing and reading
 *         of characters out the UART port
 *
 *  \author $Author: Colin Tan $
 *  \version 1.0
 *
 *  $Date: 2010-10-24 23:35:54 +1100 (Sun, 24 Oct 2010) $
 *  \warning No Warnings for now
 *  \bug No Bugs for now
 *  \note No Notes for now
 */

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "irq.h"
#include "semphr.h"
#include "uart.h"

#define UART_PWR_CTRL		PCONP
#define UART_PWR_EN			( ( unsigned portLONG ) 1)
#define UART0_PWR_OFFSET	( ( unsigned portLONG ) 3)
#define UART01_CLK_SEL		PCLKSEL0
#define UART0_CLK_OFFSET	( ( unsigned portLONG ) 6)

/*Clock rate multipliers*/
#define UART_CLK_QUARTER	( ( unsigned portLONG ) 0)
#define UART_CLK_FULL		( ( unsigned portLONG ) 1)
#define UART_CLK_HALF		( ( unsigned portLONG ) 2)
#define UART_CLK_EIGHTH		( ( unsigned portLONG ) 3)

#define UART_DIV_LATCH_EN	( ( unsigned portLONG ) 0x00000080) /*DLAB =1 enables DLL and DLM Registers, DLAB =0 enables interrupts*/
#define UART0_DLL			   U0DLL /*Lower 8 divisor bits*/
#define UART0_DLM			   U0DLM /*Upper 8 divisor bits*/
#define UART0_FDIV			U0FDR /*Fractional divisor bits*/
#define UART_WRD_LEN_8CHR	( ( unsigned portLONG ) 0x00000003)
#define UART_STOP_BIT_1		( ( unsigned portLONG ) 0x00000000)
#define UART_RBR_INT_EN		( ( unsigned portLONG ) 0x00000001)
#define UART_THRE_INT_EN	( ( unsigned portLONG ) 0x00000002)
#define UART_RX_STAT_INT_EN ( ( unsigned portLONG ) 0x00000004)

#define ser_BAUD_RATE					( ( unsigned portLONG ) 115200 )
#define serCLEAR_VIC_INTERRUPT		( ( unsigned portLONG ) 0 )
#define serBUFF_LENGTH					( ( unsigned portLONG ) 100)

/* Constants to determine the ISR source. */
#define serSOURCE_THRE					( ( unsigned portCHAR ) 0x02 )
#define serSOURCE_RX_TIMEOUT			( ( unsigned portCHAR ) 0x0c )
#define serSOURCE_ERROR					( ( unsigned portCHAR ) 0x06 )
#define serSOURCE_RX					   ( ( unsigned portCHAR ) 0x04 )
#define serINTERRUPT_SOURCE_MASK		( ( unsigned portCHAR ) 0x0f )

/* Constants to setup and access the UART. */
#define serFIFO_ON						( ( unsigned portCHAR ) 0x01 )
#define serCLEAR_FIFO					( ( unsigned portCHAR ) 0x06 )
#define serWANTED_CLOCK_SCALING			( ( unsigned portLONG ) 16 )

#define serNO_BLOCK						( ( portTickType ) 0 )


/*-----------------------------------------------------------*/
/* Queues used to hold received characters, and characters waiting to be
transmitted. */
static xQueueHandle RX_BUFF;
static xQueueHandle TX_BUFF;
static volatile portLONG UART_FREE;
static xSemaphoreHandle TX_MUTEX;

void Comms_UART_Handler(void);
void Comms_UART_Wrapper( void ) __attribute__ ((naked));

/*-------------------------------------------------------------*/
//Initialisation and Interrupt Handler

void Comms_UART_Handler(void)
{
	signed portCHAR cChar;
	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

	/* What caused the interrupt? */
	switch( U0IIR & serINTERRUPT_SOURCE_MASK )
	{
		case serSOURCE_ERROR :	/* Not handling this, but clear the interrupt. */
								cChar = U0LSR;
								break;

		case serSOURCE_THRE	 :	/* The THRE is empty.  If there is another
								character in the Tx queue, send it now. */

			if( xQueueReceiveFromISR( TX_BUFF, &cChar, &xHigherPriorityTaskWoken ) == pdTRUE )
								{
									U0THR = cChar;
								}
								else
								{
									UART_FREE= pdTRUE; // No More Characters to tx
								}
								break;

		case serSOURCE_RX_TIMEOUT :
		case serSOURCE_RX	:	/* A character was received.  Place it in
								the queue of received characters. */
								cChar = U0RBR;
								xQueueSendFromISR( RX_BUFF, &cChar, &xHigherPriorityTaskWoken );
								break;

		default				:	/* There is nothing to do, leave the ISR. */
								break;
	}

	if( xHigherPriorityTaskWoken )
	{
		portYIELD_FROM_ISR();//Debug provision not really necessary
	}

	/* Clear the ISR in the VIC. */
	VICVectAddr = serCLEAR_VIC_INTERRUPT;
}

void Comms_UART_Wrapper( void )
{
	portSAVE_CONTEXT();		// Save the context
	Comms_UART_Handler();
	portRESTORE_CONTEXT(); 	// Restore the context
}

unsigned portLONG Comms_UART_Init(void)
{
	unsigned portLONG ulDivisor, ulWantedClock;

	UART_PWR_CTRL |= UART_PWR_EN << UART0_PWR_OFFSET;

	/* Set up the peripheral clock for UART0 */
	UART01_CLK_SEL |= UART_CLK_FULL << UART0_CLK_OFFSET;

	// Setup UART mutex
	vSemaphoreCreateBinary( TX_MUTEX );
	if(!TX_MUTEX) return pdFAIL;

	/* Set up queues and Empty Flag */
	RX_BUFF 	= xQueueCreate( serBUFF_LENGTH, ( unsigned portBASE_TYPE ) sizeof( signed portCHAR ) );
	TX_BUFF 	= xQueueCreate( serBUFF_LENGTH + 1, ( unsigned portBASE_TYPE ) sizeof( signed portCHAR ) );
	UART_FREE 	= ( portLONG ) pdTRUE;
	if (!RX_BUFF||!TX_BUFF){
		// Error Has Happened In the creation of the buffers.
		return pdFAIL;
	}
	
	portENTER_CRITICAL();
	{
		PINSEL0|=( ( unsigned portLONG ) 0x00000050);

		/* Setup the baud rate:  Calculate the divisor value. */
		ulWantedClock = ser_BAUD_RATE * serWANTED_CLOCK_SCALING;
		ulDivisor = configCPU_CLOCK_HZ / ulWantedClock;

		/* Set the DLAB bit so we can access the divisor. */
		U0LCR = UART_DIV_LATCH_EN;
		/* Setup the divisor. */
		UART0_DLL = ( unsigned portCHAR ) ( ulDivisor & ( unsigned portLONG ) 0xff );
		ulDivisor >>= 8;
		UART0_DLM = ( unsigned portCHAR ) ( ulDivisor & ( unsigned portLONG ) 0xff );

		/* Turn on the FIFO's and clear the buffers. */
		U0FCR = ( serFIFO_ON | serCLEAR_FIFO );

		/* Setup transmission format. */
		U0LCR = (UART_WRD_LEN_8CHR|UART_STOP_BIT_1);
		U0IER = (UART_RBR_INT_EN|UART_THRE_INT_EN|UART_RX_STAT_INT_EN);
		install_irq( 6, Comms_UART_Wrapper, HIGHEST_PRIORITY );
		enable_VIC_irq(6);
	}
	portEXIT_CRITICAL();
		
	return pdTRUE;
}

/*---------------------------------------------------------------------------------*/
// UART access functions

signed portBASE_TYPE Comms_UART_Read_Char( signed portCHAR *pcRxedChar, portTickType xBlockTime )
{
	if( xQueueReceive( RX_BUFF, pcRxedChar, xBlockTime ) )
	{
		return pdTRUE;
	}
	else
	{
		return pdFALSE;
	}
}

/*-----------------------------------------------------------*/

signed portBASE_TYPE Comms_UART_Write_Char( signed portCHAR cOutChar, portTickType xBlockTime )
{
	signed portBASE_TYPE xReturn;

	xSemaphoreTake( TX_MUTEX, portMAX_DELAY );
	{
		/* Is there space to write directly to the UART? */
		if( UART_FREE == ( portLONG ) pdTRUE )
		{
			/* We wrote the character directly to the UART, so was
			successful. */

			UART_FREE = pdFALSE;
			U0THR = cOutChar;
			xReturn = pdPASS;
		}
		else
		{
			/* We cannot write directly to the UART, so queue the character.
			Block for a maximum of xBlockTime if there is no space in the
			queue. */

			xReturn = xQueueSend( TX_BUFF, &cOutChar, xBlockTime );

			/* Depending on queue sizing and task prioritisation:  While we
			were blocked waiting to post interrupts were not disabled.  It is
			possible that the serial ISR has emptied the Tx queue, in which
			case we need to start the Tx off again. */
			if( ( UART_FREE == ( portLONG ) pdTRUE ) && ( xReturn == pdPASS ) )
			{
				xQueueReceive( TX_BUFF, &cOutChar, serNO_BLOCK );
				UART_FREE = pdFALSE;
				U0THR = cOutChar;
			}
		}
	}
	
	xSemaphoreGive( TX_MUTEX );
	
	return xReturn;
}





