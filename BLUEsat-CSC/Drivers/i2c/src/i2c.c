/**
 *  \file Comms_I2C.c
 *
 *  \brief The I2C driver function is to be used by tasks who wish to communicate to
 * other devices via the I2C port.
 *
 *  \author $Author: Colin Tan $
 *  \version 1.0
 *
 *  $Date: 2010-10-16 23:26:58 +1100 (Sat, 16 Oct 2010) $
 *  \warning No Warnings for now
 *  \bug No Bugs for now
 *  \note At present the system is designed for only one task to be waiting on an operation.
 * This can be fixed in the future by having the tasks create the semaphores and pass it to
 * the driver to be given after execution.
 */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "irq.h"
#include "i2c.h"
#include "vic.h"



//TODO: remove the test
#ifndef I2CTEST
	#define I2CTEST 0
#endif

#ifndef CHIP
	#define CHIP 2468
#endif

/*-----------------------------------------------------------*/
/*I2C Configurations*/

#define I2C_CYCLE_HIGH	90
#define I2C_CYCLE_LOW	90

#define I2C_BUFF_LEN 	10
#define I2C_QUEUE_LEN 	10

/*Timing For Blocks*/
#define I2C_NO_BLOCK 	(( portTickType ) 0)

#define I2C_MAX_BUS		3
#define I2C0_IRQ_SRC	9
#define I2C1_IRQ_SRC	19
#define I2C2_IRQ_SRC	30

#define I2C_MAX_QUEUE_LENGTH 30 //Max is 40 as 40 telem dev on each bus
#define I2C_YES		1
#define I2C_NO		0

/*Bus Free Flag Register Mask and Macros*/
#define BUS_MASK ( (unsigned portCHAR) 0x1)
#define BUS_FREE(a) I2C_Bus_Free|(BUS_MASK<<a)
#define BUS_BUSY(a) I2C_Bus_Free&(~(BUS_MASK<<a))
#define BUS_IS_FREE(bus) I2C_Bus_Free&(BUS_MASK<<bus)

#define SLAVE_WAITING (bus) ((Bus_S_Ctrl[bus].Status_TX==BUFF_SET)||(Bus_S_Ctrl[bus].Status_RX==BUFF_SET))

#define I2C_B01_PORT		PINSEL0
#define I2C_B02_PORT		PINSEL1
#define I2C_B01_EN			( ( unsigned portLONG ) 0x00A0000F)
#define I2C_B02_EN			( ( unsigned portLONG ) 0x01400000)

#define I2C_BUS0			( ( unsigned portLONG ) 0x00000200)
#define I2C_BUS1			( ( unsigned portLONG ) 0x00080000)
#define I2C_BUS2			( ( unsigned portLONG ) 0x40000000)

/*-----------------------------------------
 * Bus Access and State Control Structures
 * ----------------------------------------*/

/*
 * Active Bus Control Structure
 */
typedef struct {
	portCHAR 	SLA;		//Address of present slave and Operation Bit
	portSHORT	Index;		//Present position in the buffer
	xSemaphoreHandle Sem;	//Semaphore to be given after operations are completed
	portSHORT *	Index_Max;	//Limit of buffer size or data stored in buffer
	portCHAR *	Tx_Rx_Buff;	//Present buffer bus is using
	portCHAR *	Fail;		//Result of the operation either a I2C_YES or I2C_NO
}Bus_Master_Ctrl;

/*
 * Slave Control Structure
 * ~ Gives driver quick access to slave buffer locations
 **/
typedef struct{
	I2C_SLAVE_STATUS Status_TX;	//Determine the state of the TX Buffer to determine if access is sane
	I2C_SLAVE_STATUS Status_RX;
	portCHAR * Buf_TX;			//Buffer set up and maintained by the calling task
	portCHAR * Buf_RX;
	portSHORT * Max_TX;			//Pointer to the maximum size of the buffer also sued to return the length of the buffer actually used in a read
	portSHORT * Max_RX;
	xSemaphoreHandle Sem_TX;	// Semaphores to allow calling tasks to block on the call
	xSemaphoreHandle Sem_RX;
}Bus_Slave_Ctrl;

/*
 * Register Control Structure
 * ~ Allows easy switching between bus registers by the ISR
 */
typedef struct{
	unsigned int * Conset;	//Set Flags
	unsigned int * Stat;	//Status of Bus
	unsigned int * Dat;		//Data Register
	unsigned int * Adr;		//Address of Unit
	unsigned int * Sclh;	//Length of High part of period
	unsigned int * Scll;	//Length of Low part of period
	unsigned int * ConClr;	//Clear Flags
}I2C_Registers;

/*-----------------------------------------------------------*/

/*Static Variables Used throughout*/
static volatile I2C_Registers 	Bus_Regs[I2C_MAX_BUS];
static volatile Bus_Master_Ctrl Bus_M_Ctrl[I2C_MAX_BUS];
static volatile Bus_Slave_Ctrl 	Bus_S_Ctrl[I2C_MAX_BUS];
static xQueueHandle 			I2C_Bus[I2C_MAX_BUS];
static portCHAR 				I2C_Bus_Free;

/*-----------------------------------------------------------*/
/*Internal functions*/
void Comms_I2C_Handler(void);
void Comms_I2C_Wrapper( void ) __attribute__ ((naked));

static inline void Comms_I2C_Clr_Req	(volatile Bus_Master_Ctrl * master,volatile Bus_Slave_Ctrl * slave);
static inline void Comms_I2C_Clr_Slave_TX (volatile Bus_Slave_Ctrl * slave);
static inline void Comms_I2C_Clr_Slave_RX (volatile Bus_Slave_Ctrl * slave);
static inline void Comms_I2C_Reg_Init	(I2C_BUS_CHOICE bus);
static inline void Comms_I2C_Reg_Setup	(I2C_BUS_CHOICE bus);

static signed portBASE_TYPE Comms_I2C_Ld_Ctrl_Info	(I2C_BUS_CHOICE bus, portBASE_TYPE * xHigherPriorityTaskWokenByPost);
static signed portBASE_TYPE Comms_I2C_Ld_Slave		(I2C_BUS_CHOICE bus, I2C_SLAVE_MODE mode);
static inline void Comms_I2C_Slave_Start(I2C_BUS_CHOICE bus, I2C_SLAVE_MODE mode);
static inline void Comms_I2C_Nxt_Chk(I2C_BUS_CHOICE bus,I2C_M_OR_S pres);

static signed portBASE_TYPE Comms_I2C_Tx_Nxt_Elem	(I2C_BUS_CHOICE bus);
static signed portBASE_TYPE Comms_I2C_Rx_Nxt_Elem	(I2C_BUS_CHOICE bus);

static inline void Comms_I2C_Slave_Response	(I2C_BUS_CHOICE bus,I2C_SLAVE_MODE mode);
static inline void Comms_I2C_Rx_Response	(I2C_BUS_CHOICE bus);

static inline void Comms_I2C_Bus_Queue_Empty	(I2C_BUS_CHOICE bus,I2C_M_OR_S present);
static inline void Comms_I2C_Bus_Queue_Not_Empty(I2C_BUS_CHOICE bus,I2C_M_OR_S present);
static inline void Comms_I2C_Call_Slave			(I2C_BUS_CHOICE bus);
static inline void Comms_I2C_Slave_Completed	(I2C_BUS_CHOICE bus, I2C_SLAVE_MODE mode, portBASE_TYPE * xHigherPriorityTaskWokenByPost);
static inline void Comms_I2C_End(I2C_BUS_CHOICE bus, signed portBASE_TYPE result, portBASE_TYPE * xHigherPriorityTaskWokenByPost);
static inline void Comms_I2C_Err(I2C_BUS_CHOICE bus, signed portBASE_TYPE bus_error, portBASE_TYPE * xHigherPriorityTaskWokenByPost);

/*-----------------------------------------------------------*/

/*
 * 	I2C Init
 *
 * 	This Function Initialises the I2C Driver.
 * 	~ High and low time calculation: (400000/(14.75*4))/2=73.750
 *	~ To handle the migration to the LPC2468, alternate inatilizations
 *	for the VIC and PCB have been incleded and can be activated by setting the CHIP to LPC2468.
 *
 * */

void Comms_I2C_Init(void)
{
	portSHORT count;
	taskENTER_CRITICAL();
	{
		I2C_Bus_Free=0;

		// Step 1: Power
		PCONP = (PCONP & (~(0x1 << 7))) | (0x1 << 7); // power for I2C0

		// Step 2: Clock
		PCLKSEL0 = (PCLKSEL0 & (~(0x3 <<14))) | (0x1 << 14); // set up the peripheral clock for I2C0

		// Step 3: initialise PINSEL registers
		I2C_B01_PORT	|= I2C_B01_EN; // fine - this is to initialise the GPIO port for SDA1 and SCL1
		I2C_B02_PORT	|= I2C_B02_EN; // set up SDA0 and SCL0

		// Step 4: interrupt
		install_irq( I2C0_IRQ_SRC, Comms_I2C_Wrapper,HIGHEST_PRIORITY );
		enable_VIC_irq(I2C0_IRQ_SRC);
		install_irq( I2C1_IRQ_SRC, Comms_I2C_Wrapper,HIGHEST_PRIORITY );
		enable_VIC_irq(I2C1_IRQ_SRC);
		install_irq( I2C2_IRQ_SRC, Comms_I2C_Wrapper,HIGHEST_PRIORITY );
		enable_VIC_irq(I2C2_IRQ_SRC);

		// Step 5: Initialisation
		for (count=0; count<I2C_MAX_BUS;count++){// Set up bus control data and register structure as well as queues
			Comms_I2C_Clr_Req(&Bus_M_Ctrl[count],&Bus_S_Ctrl[count]);
			Comms_I2C_Reg_Init(count);
			Comms_I2C_Reg_Setup(count);
			I2C_Bus[count]= xQueueCreate( I2C_MAX_QUEUE_LENGTH, ( unsigned portBASE_TYPE ) sizeof( Bus_Master_Ctrl) );
			I2C_Bus_Free|=(BUS_MASK<<count);
		}
	}
	taskEXIT_CRITICAL();
}


/*-----------------------------------------------------------*/

/*
 * 	Register Init
 *
 * 	This Function Initialises the Array of registers which will
 * 	be used to communicate with the bus.
 *	- Due to the approach used to set up the registers, order
 *	of initialisation has to be preserved
 *
 * */

static inline void Comms_I2C_Reg_Init(I2C_BUS_CHOICE bus)
{
	unsigned int * base;
	switch (bus){
		case BUS0:
			base=(unsigned int *)I2C0_BASE_ADDR;
			break;
		case BUS1:
			base=(unsigned int *)I2C1_BASE_ADDR;
			break;
		case BUS2:
			base=(unsigned int *)I2C2_BASE_ADDR;
			break;
	}
	//Order of the registers is important for right offset.
	Bus_Regs[bus].Conset	=base++;
	Bus_Regs[bus].Stat		=base++;
	Bus_Regs[bus].Dat		=base++;
	Bus_Regs[bus].Adr		=base++;
	Bus_Regs[bus].Sclh		=base++;
	Bus_Regs[bus].Scll		=base++;
	Bus_Regs[bus].ConClr	=base++;
}



/*-----------------------------------------------------------*/

/*
 * 	I2C Reg Setup
 *
 * 	This Function Initialises the I2C Bus.
 *
 * 	the following calculation is applied in the old board.
 * 	//~ High and low time calculation: ((14.75MHz*4)/400000)/2=73.750
 *
 * 	with LPC2468
 * 	(72MHz / 0.4MHz) / 2 = 90
 *
 *
 * 	~ In the event that different opperating modes or speeds are
 * 	required, this function has to be modified
 *
 * */

static inline void Comms_I2C_Reg_Setup(I2C_BUS_CHOICE bus)
{
#if I2CTEST
	*(Bus_Regs[bus].Adr)	=I2C_FC_ADDR ;			//Set UC Slave address
#else
	*(Bus_Regs[bus].Adr)	=I2C_CSC_ADDR ;
#endif
	*(Bus_Regs[bus].ConClr)	=(I2EN|STA|SI|STO|AA);
	*(Bus_Regs[bus].Scll)	=I2C_CYCLE_LOW;			//Set High time and Low time
	*(Bus_Regs[bus].Sclh) 	=I2C_CYCLE_HIGH;
	*(Bus_Regs[bus].Conset)	=(I2EN|AA);				//Slave mode only set when buffers loaded
}



/*-----------------------------------------------------------*/

/*
 * 	Comms I2C Wrapper Function
 * */
void Comms_I2C_Wrapper( void )
{
	portSAVE_CONTEXT();		// Save the context
	Comms_I2C_Handler();
	portRESTORE_CONTEXT(); 	// Restore the context
}

/*-----------------------------------------------------------*/

/*	Comms I2C Handler
 * 	This functions decides on what to do next based on the present
 * 	state of the driver.
 *  Assumptions made:
 *  ~ Buffers that have been placed in the Master queues and
 *  the slave control structures have been verified and will be
 *  stable throughout the process of the stste system and will only
 *  be accessed / modified (by another task other than the ISR)
 *  after the ISR has completed midifying it.
 *
 *
 *  NOTE:
 *  ~ In the event of a failure be it a nack or a bad state,
 * 	the driver will drop the comms link. This means that
 * 	The requesting function will receive a fail from the
 * 	semaphone and will have to do a retransmission if
 * 	necessary. This choice was made to improve reliability
 * 	of the driver and improve response time.
 *
 * 	~ Data received is 0xff or there is only one slot left/no
 * 	more data left to be sent by the slave.
 *
 *	~ This driver only allows one task to be a slave receiver
 *	or transmitter. If multi task slave support is required
 *	the driver has to be modified to include locking to allow
 * 	sane sharing of the buffer pointers
 *
 * UNUSED STATES:
 * General Call Slave States - Unused
 * case I2C_SRM_GEN_CALL_ACKED:
 * case I2C_SRM_GEN_CALL_DATA_ACKED:
 * case I2C_SRM_GEN_CALL_DATA_NACKED:
 * case I2C_SRM_ARB_LOST_GEN_CALL_ACKED:
 */

void Comms_I2C_Handler(void)
{
	portBASE_TYPE xHigherPriorityTaskWokenByPost=pdFAIL;
	portLONG Active=0;
	I2C_BUS_CHOICE bus=0;
	Active=VICIRQStatus&(I2C_BUS0|I2C_BUS1|I2C_BUS2);
	if (Active){							// Determine which bus caused the interrupt
		switch (Active){
			case I2C_BUS0:
							bus=BUS0;
							break;
			case I2C_BUS1:
							bus=BUS1;
							break;
			case I2C_BUS2:
							bus=BUS2;
							break;
		}


		switch (*Bus_Regs[bus].Stat){
			/*Error States*/
			case I2C_NO_STATE:	//Wait for another state change
								break;
			case I2C_BUS_ERR:	//Bus Error has occurred. Stop is issued to rest.
								Comms_I2C_Err(bus, pdTRUE, &xHigherPriorityTaskWokenByPost);
								break;
			case I2C_M_ARB_LOST://Error packet is dropped from the queue src will do a retrans
								Comms_I2C_Err(bus, pdFALSE, &xHigherPriorityTaskWokenByPost);
								break;
			/*Master States*/
			case I2C_M_START_CON://State System started, send SLA
								if (!Comms_I2C_Ld_Ctrl_Info(bus, &xHigherPriorityTaskWokenByPost)){
									Comms_I2C_Bus_Queue_Empty(bus, MASTER);
									break;
								}
			case I2C_M_START_CON_R:
								Comms_I2C_Call_Slave(bus);
								break;


			case I2C_MTM_DATA_ACKED://Data acked
			case I2C_MTM_INIT_ACKED://Slave has acked address
								if (!Comms_I2C_Tx_Nxt_Elem(bus)){	// Load bytes of data it into I2DAT
									Comms_I2C_End(bus, pdPASS, &xHigherPriorityTaskWokenByPost);
								}else{
									*Bus_Regs[bus].ConClr=(STA|SI);
								}
								break;

			case I2C_MTM_DATA_NACKED:// Data has nacked
			case I2C_MTM_INIT_NACKED:// Slave has nacked address [TX]
			case I2C_MRM_INIT_NACKED:// Slave has nacked address [RX]
								Comms_I2C_End(bus, pdFAIL, &xHigherPriorityTaskWokenByPost);
								break;


			case I2C_MRM_INIT_ACKED:// Slave has acked address get ready to rx data
								Comms_I2C_Rx_Response(bus);
								break;

			case I2C_MRM_DATA_ACKED:
								Comms_I2C_Rx_Nxt_Elem(bus);
								Comms_I2C_Rx_Response(bus);
								break;

			case I2C_MRM_DATA_NACKED:// Buffer full
								Comms_I2C_Rx_Nxt_Elem(bus);
								Comms_I2C_End(bus, pdPASS, &xHigherPriorityTaskWokenByPost);
								break;

			case I2C_SRM_CALLED_ACKED:	/*Set up Control structure to start receiving data*/
								Comms_I2C_Slave_Start(bus, SLAVE_RECEIVER);

								break;
			case I2C_SRM_DATA_ACKED:	/*Start receiving data*/
								Comms_I2C_Rx_Nxt_Elem(bus);
								Comms_I2C_Slave_Response(bus, SLAVE_RECEIVER);
								break;

			case I2C_SRM_DATA_NACKED:		//Handle the Data_SRM_DATA_NACKED:	//Buffer out of space
			case I2C_SRM_STOP_RSTART:
								Comms_I2C_Slave_Completed(bus, SLAVE_RECEIVER, &xHigherPriorityTaskWokenByPost);
								break;
			case I2C_STM_ARB_LOST:
			case I2C_SRM_ARB_LOST:		//Master will restart Comms
								*Bus_Regs[bus].ConClr=(SI|AA);
								break;



			case I2C_STM_CALLED_ACKED:	/*Start sending data*/
								Comms_I2C_Slave_Start(bus, SLAVE_TRANSMITTER);

								break;
			case I2C_STM_DATA_ACKED:
								Comms_I2C_Tx_Nxt_Elem(bus);
								Comms_I2C_Slave_Response(bus, SLAVE_TRANSMITTER);
								break;

			case I2C_STM_DATA_END_ACKED:
			case I2C_STM_DATA_NACKED:
								Comms_I2C_Slave_Completed(bus, SLAVE_TRANSMITTER, &xHigherPriorityTaskWokenByPost);
								break;

			default:			break;
		}
	}

	if( xHigherPriorityTaskWokenByPost ){
		portYIELD_FROM_ISR();//Debug provision not really necessary
	}
	/* Clear the ISR in the VIC. */
	VICVectAddr = 0x00;
}

/*-----------------------------------------------------------*/

/*
 * 	Comms I2C Master
 * 	This function is used by an external task to perform a
 * 	Master read or write to a slave device.
 *
 *  Buffer and length locations MUST NOT change while waiting
 *  for the operation to complete.
 *
 *	Fail Cases:
 *	- NULL Buffer
 *	- 0 Length
 *	- No Space on queue
 *
 * */

signed portBASE_TYPE Comms_I2C_Master(I2C_NODE dest, I2C_OPPERATION op,volatile portCHAR* result, volatile portCHAR* loc, volatile portSHORT *len,  xSemaphoreHandle semaphore,I2C_BUS_CHOICE bus)
{
	Bus_Master_Ctrl temp;
	signed portBASE_TYPE outcome=pdFAIL;
	if ((*len>0)&&(loc!=NULL)){		//Buffer size has to be greater than 0
		portENTER_CRITICAL();
		{
			if (semaphore !=NULL){
				xSemaphoreTake( semaphore, I2C_NO_BLOCK );
			}
			if ( !xQueueIsQueueFullFromISR( I2C_Bus[bus] )){//OK to use since in critical section
				*result=I2C_YES;
				temp.SLA		= ((dest << 1)|op);
				temp.Index		= 0;
				temp.Fail		= (portCHAR*)result;
				temp.Sem		= semaphore;
				temp.Index_Max	= (portSHORT*)len;
				temp.Tx_Rx_Buff	= (portCHAR*)loc;
				xQueueSendToBack( I2C_Bus[bus], (void *) &temp, I2C_NO_BLOCK);// If we reach here there is space for the item on the queue
				if (BUS_IS_FREE(bus)){	//Check if bus is free.
					*(Bus_Regs[bus].Conset)=(STA);				//Start Condition
				}
				outcome=pdPASS;
			}
		}
		portEXIT_CRITICAL();
	}
	return outcome;
}

/*-----------------------------------------------------------*/

/*
 *	Comms I2C Slave
 *
 *	This function is used by the external task to perform slave buffer
 *	allocation for slave operations.
 *
 *	The driver has been setup as a one shot and is designed to only
 *	allow one task to be using it at a time in the event that a
 *	multi task slave set is required than the slave system employed
 *	in this driver will have to be modified to use local buffers and
 *	a handler task.
 *
 *	Fail Cases:
 *	- NULL Buffer
 *	- 0 Length
 *
 * */

signed portBASE_TYPE Comms_I2C_Slave(I2C_SLAVE_MODE mode, volatile portCHAR * loc, volatile portSHORT* len, xSemaphoreHandle semaphore, I2C_BUS_CHOICE bus)
{
	signed portBASE_TYPE outcome=pdFAIL;
	if ((*len>0)&&(loc!=NULL)){
		portENTER_CRITICAL();
		{
			if (semaphore !=NULL){
				xSemaphoreTake( semaphore, I2C_NO_BLOCK );
			}

			if (mode==SLAVE_RECEIVER){
				if (Bus_S_Ctrl[bus].Status_RX!=BUFF_BUSY){
					Bus_S_Ctrl[bus].Sem_RX	= semaphore;
					Bus_S_Ctrl[bus].Buf_RX	= (portCHAR*)loc;
					Bus_S_Ctrl[bus].Max_RX	= (portSHORT*)len;
					Bus_S_Ctrl[bus].Status_RX=BUFF_SET;
					outcome=pdPASS;
				}
			}else{
				if (Bus_S_Ctrl[bus].Status_TX!=BUFF_BUSY){
					Bus_S_Ctrl[bus].Sem_TX	= semaphore;
					Bus_S_Ctrl[bus].Buf_TX	= (portCHAR*)loc;
					Bus_S_Ctrl[bus].Max_TX	= (portSHORT*)len;
					Bus_S_Ctrl[bus].Status_TX=BUFF_SET;
					outcome=pdPASS;
				}

			}
			if (BUS_IS_FREE(bus)){	//Check if bus is free.
				*(Bus_Regs[bus].Conset)	=(AA);//Ensure that the slave can respond to calls to its SLA
			}
		}
		portEXIT_CRITICAL();
	}
	return outcome;
}

/*-----------------------------------------------------------*/

/*
 * 	Clear Slave Buffer
 *
 * 	Clear function to be used by an external task to either
 * 	prep the slave buffer or after a I2C communications session
 * 	to minimize data corruption possibility.
 *
 * */

signed portBASE_TYPE Comms_I2C_Slave_Clear(I2C_BUS_CHOICE bus , I2C_SLAVE_MODE mode)
{
	signed portBASE_TYPE outcome=pdFALSE;
	portENTER_CRITICAL();
	{
		if (BUS_IS_FREE(bus)){
			switch(mode){
				case SLAVE_TRANSMITTER:
								Comms_I2C_Clr_Slave_TX (&Bus_S_Ctrl[bus]);
								outcome=pdTRUE;
								break;
				case SLAVE_RECEIVER:
								Comms_I2C_Clr_Slave_RX (&Bus_S_Ctrl[bus]);
								outcome=pdTRUE;
								break;
			}

		}
	}
	portEXIT_CRITICAL();
	return outcome;
}


/*--------------------------------------------------------*
 * The Following functions are called from within the ISR *
 *--------------------------------------------------------*/

/*
 * 	Load Master Control Information
 *
 * 	The next set of information is popped from the master mode queue into the
 * 	active bus control structure.
 *
 * */
static signed portBASE_TYPE Comms_I2C_Ld_Ctrl_Info(I2C_BUS_CHOICE bus, portBASE_TYPE * xHigherPriorityTaskWokenByPost)
{
	return xQueueReceiveFromISR(I2C_Bus[bus], (void *)&Bus_M_Ctrl[bus],xHigherPriorityTaskWokenByPost);
}

/*-----------------------------------------------------------*/

/*
 * 	Signal Slave Address
 *
 * 	A master state system has been started. The target slave address is loaded
 * 	into the data register for the bus and the bus is flagged as busy.
 *
 * */

static inline void Comms_I2C_Call_Slave(I2C_BUS_CHOICE bus)
{
	*Bus_Regs[bus].Dat=Bus_M_Ctrl[bus].SLA;
	*Bus_Regs[bus].ConClr=(SI);
	I2C_Bus_Free=BUS_BUSY(bus);
}

/*-----------------------------------------------------------*/

/*
 * 	Slave Transmit and Receive Start Function
 *
 * 	This function is called when a slave mode is initialized.
 * 	~ Checks Slave buffers to determine if they are in a sane mode for comms
 * 	~ If buffers are sane, the active control structure is set up
 * 	~ If buffers are not sane, state machine is told to NACK the next packet
 * 	to signal slave failure and the control structure is cleared to prevent
 * 	data corruption of previous buffers.
 *
 * */
static inline void Comms_I2C_Slave_Start(I2C_BUS_CHOICE bus, I2C_SLAVE_MODE mode){
	if (Comms_I2C_Ld_Slave(bus, mode)){// If the buffer Ready
		if (mode==SLAVE_TRANSMITTER){
			Comms_I2C_Tx_Nxt_Elem(bus);
		}
		*Bus_Regs[bus].Conset=(AA);
		*Bus_Regs[bus].ConClr=(SI);
	}else{
		Comms_I2C_Clr_Req(&Bus_M_Ctrl[bus],NULL);
		*Bus_Regs[bus].ConClr=(SI|AA);
	}
	I2C_Bus_Free=BUS_BUSY(bus);
}

/*-----------------------------------------------------------*/

/*
 * 	Load Slave Control Information
 *
 * 	A slave state system has been initiated. Therefore the relevant buffer
 * 	information is copied into the bus control structure.
 *
 * 	In the event that the relevant buffer is not ready for a transfer, the function
 * 	will fail, otherwise, the slave buffer will enter a busy state until the completion
 * 	of the transfer.
 *
 * */
static signed portBASE_TYPE Comms_I2C_Ld_Slave(I2C_BUS_CHOICE bus, I2C_SLAVE_MODE mode)
{
	signed portBASE_TYPE outcome = pdFAIL;
	if (mode == SLAVE_RECEIVER){
		if (Bus_S_Ctrl[bus].Status_RX!=BUFF_SET){
			return outcome;							//Exit if the buffer is not ready.
		}
		Bus_S_Ctrl[bus].Status_RX=BUFF_BUSY;
	}else{
		if ((Bus_S_Ctrl[bus].Status_TX==BUFF_EMPTY)||(Bus_S_Ctrl[bus].Status_TX==BUFF_BUSY)){
			return outcome;
		}
		Bus_S_Ctrl[bus].Status_TX=BUFF_BUSY;
	}
	Bus_M_Ctrl[bus].Fail		=NULL;
	Bus_M_Ctrl[bus].SLA			=0;
	Bus_M_Ctrl[bus].Index		=0;
	Bus_M_Ctrl[bus].Sem			=(mode==SLAVE_RECEIVER)?Bus_S_Ctrl[bus].Sem_RX:Bus_S_Ctrl[bus].Sem_TX;
	Bus_M_Ctrl[bus].Tx_Rx_Buff	=(mode==SLAVE_RECEIVER)?Bus_S_Ctrl[bus].Buf_RX:Bus_S_Ctrl[bus].Buf_TX;
	Bus_M_Ctrl[bus].Index_Max	=(mode==SLAVE_RECEIVER)?Bus_S_Ctrl[bus].Max_RX:Bus_S_Ctrl[bus].Max_TX;
	outcome = pdPASS;
	return outcome;
}


/*-----------------------------------------------------------*/

/*
 * 	I2C Tx Next Elem
 * 	This function extracts the next byte of data from the
 * 	TX buffer for transfer. If no more elements are left,
 * 	it returns false.
 * */
static signed portBASE_TYPE Comms_I2C_Tx_Nxt_Elem(I2C_BUS_CHOICE bus)
{
	signed portBASE_TYPE result=pdFAIL;
	if (*Bus_M_Ctrl[bus].Index_Max>Bus_M_Ctrl[bus].Index){
		*Bus_Regs[bus].Dat=Bus_M_Ctrl[bus].Tx_Rx_Buff[Bus_M_Ctrl[bus].Index++];
		result=pdPASS;
	}
	return result;
}

/*-----------------------------------------------------------*/

/*
 * 	Receive Next Element
 *
 * 	Used in the receive functions, as long as the max index
 * 	value has not been over run, data is copied from the data
 * 	register into the buffer.
 *
 * 	Upon over running the max index, a false is returned.
 *
 * */
static signed portBASE_TYPE Comms_I2C_Rx_Nxt_Elem(I2C_BUS_CHOICE bus)
{
	signed portBASE_TYPE result=pdFAIL;
	if (*Bus_M_Ctrl[bus].Index_Max>Bus_M_Ctrl[bus].Index){
		Bus_M_Ctrl[bus].Tx_Rx_Buff[Bus_M_Ctrl[bus].Index++]=*Bus_Regs[bus].Dat;
		result=pdPASS;
	}
	return result;
}

/*-----------------------------------------------------------*/

/*
 * 	Master Data Response
 *
 * 	During the normal states of receiving data, the next packet is
 * 	set to ACK if there is still space on the buffer else it is
 * 	set to NACK the next packet. In this case, if there is less
 * 	than 1 byte on the buffer, the next bye should be NACKed to
 * 	give the sender warning that the buffer is full.
 *
 * 	Used in Master mode states.
 *
 * */
static inline void Comms_I2C_Rx_Response(I2C_BUS_CHOICE bus)
{
	portSHORT max=*Bus_M_Ctrl[bus].Index_Max;
	portSHORT pres=Bus_M_Ctrl[bus].Index;
	if (max>(++pres)){// True if there is more than one slot left
		*Bus_Regs[bus].Conset=(AA);
	}
	else{
		*Bus_Regs[bus].ConClr=(AA);
	}
	*Bus_Regs[bus].ConClr=(STA|SI);
}

/*-----------------------------------------------------------*/

/*
 *	Slave Data Response
 *
 *	Used in receiving data to determine the next state that the
 *	slave will be in. In the event that there is less than 1 slot
 *	left, the next packet will be NACKed to signal to the sender
 *	that the buffer is out of space.
 * */

static inline void Comms_I2C_Slave_Response(I2C_BUS_CHOICE bus,I2C_SLAVE_MODE mode )
{
	portSHORT max=*Bus_M_Ctrl[bus].Index_Max;
	portSHORT pres=Bus_M_Ctrl[bus].Index;
	if(mode==SLAVE_RECEIVER){					//State system Nacks the last packet in the buffer
		pres++;
	}
	if (max>pres){
		*Bus_Regs[bus].Conset=(AA);
		*Bus_Regs[bus].ConClr=(SI);
	}else{
		*Bus_Regs[bus].ConClr=(SI|AA);
	}
}

/*-----------------------------------------------------------*/

/*
 * 	Slave Operations Completed
 *
 * 	After all the data has been sent or received, the
 * 	semaphore is given to wake up the calling task and the
 * 	respective buffer is set to the BUFF_COMPLETED state.
 *
 * 	Depending if there are pending messages, the state machine
 * 	might be set to transmit the pending messages or to stop
 * 	transmissions altogether.
 * */

static inline void Comms_I2C_Slave_Completed(I2C_BUS_CHOICE bus, I2C_SLAVE_MODE mode, portBASE_TYPE * xHigherPriorityTaskWokenByPost)
{
	if (Bus_M_Ctrl[bus].Sem!=NULL){
		xSemaphoreGiveFromISR(Bus_M_Ctrl[bus].Sem, xHigherPriorityTaskWokenByPost );
	}

	if (mode==SLAVE_RECEIVER){
		Bus_S_Ctrl[bus].Status_RX=BUFF_COMPLETED;
		if (Bus_M_Ctrl[bus].Index_Max!=NULL){
			*(Bus_M_Ctrl[bus].Index_Max)= Bus_M_Ctrl[bus].Index;
		}

	}else{
		Bus_S_Ctrl[bus].Status_TX=BUFF_COMPLETED;
	}
	Comms_I2C_Clr_Req(&Bus_M_Ctrl[bus],NULL);
	Comms_I2C_Nxt_Chk(bus, SLAVE);
}

/*-----------------------------------------------------------*/

/*
 * 	End Master Mode Operation
 *
 * 	Sets the result, gives the semaphore to wake up a blocking
 * 	task and depending if there are any pending requests on the
 * 	queue loads them up for transmission.
 * */

static inline void Comms_I2C_End(I2C_BUS_CHOICE bus, signed portBASE_TYPE result, portBASE_TYPE * xHigherPriorityTaskWokenByPost)
{
	*Bus_M_Ctrl[bus].Fail=(result)?I2C_NO:I2C_YES;
	if (Bus_M_Ctrl[bus].Sem!=NULL){
		xSemaphoreGiveFromISR(Bus_M_Ctrl[bus].Sem, xHigherPriorityTaskWokenByPost );
	}
	Comms_I2C_Nxt_Chk(bus, MASTER);
}

/*-----------------------------------------------------------*/

/*
 *	Error Handler
 *
 *	Bus Errors are handled by flushing the queue and reinitialising
 *	the bus, while state errors are handled by dropping the present
 *	communications and handling the next one on the queue.
 * */

static inline void Comms_I2C_Err(I2C_BUS_CHOICE bus, signed portBASE_TYPE bus_error, portBASE_TYPE * xHigherPriorityTaskWokenByPost)
{
	Bus_Master_Ctrl temp;
	*Bus_M_Ctrl[bus].Fail=I2C_YES;
	if (bus_error){												//Bus Error
		*Bus_Regs[bus].Conset=(STO);
		*Bus_Regs[bus].ConClr=(STA|SI);
		while (uxQueueMessagesWaitingFromISR( I2C_Bus[bus] )){	//Clear Queue
			xQueueReceiveFromISR(I2C_Bus[bus], (void *)&temp,xHigherPriorityTaskWokenByPost);
			*temp.Fail=I2C_YES;
		}
	}else{														//State Error
		if (Bus_M_Ctrl[bus].Sem!=NULL){
			xSemaphoreGiveFromISR(Bus_M_Ctrl[bus].Sem, xHigherPriorityTaskWokenByPost );
		}
		if(xQueueIsQueueEmptyFromISR( I2C_Bus[bus])){			//Queue Empty
			*Bus_Regs[bus].ConClr=(STA|SI);
			I2C_Bus_Free=BUS_FREE(bus);
		}else{													//Queue Not Empty
			*Bus_Regs[bus].Conset=(STA);
			*Bus_Regs[bus].ConClr=(SI);
			I2C_Bus_Free=BUS_BUSY(bus);
		}
	}
}


/*-----------------------------------------------------------*/

/*
 * 	Bus Response Helper
 *
 * 	Determines if there are requests in the queue that need to be
 * 	handled and sets the flags to either start a new communications
 * 	session or to end communications for now.
 *
 * */
static inline void Comms_I2C_Nxt_Chk(I2C_BUS_CHOICE bus,I2C_M_OR_S pres)
{
	if(xQueueIsQueueEmptyFromISR( I2C_Bus[bus])){	//Queue Empty
		Comms_I2C_Bus_Queue_Empty(bus,pres);
	}else{											//Queue Not Empty
		Comms_I2C_Bus_Queue_Not_Empty(bus, pres);
	}
}


/*-----------------------------------------------------------*/

/*
 * 	Queue Empty Response
 *
 * 	Since the queue is empty, signal the bus to stop communications,
 * 	set the bus flag to free.
 *
 * 	In the case of a Slave mode, just set the bus to respond to the
 * 	slave address if called.
 * */

static inline void Comms_I2C_Bus_Queue_Empty(I2C_BUS_CHOICE bus,I2C_M_OR_S present )
{
	if (present==MASTER){
		*Bus_Regs[bus].Conset=(STO);
		*Bus_Regs[bus].ConClr=(STA|SI);
	}else{
		*Bus_Regs[bus].Conset=(AA);
		*Bus_Regs[bus].ConClr=(STA|SI);
	}
	I2C_Bus_Free=BUS_FREE(bus);
}


/*-----------------------------------------------------------*/

/*
 * 	Queue Not Empty Response
 *
 * 	Since the queue is not empty, signal the bus to start communications,
 * 	set the bus flag to  busy.
 *
 * 	In the case of a Slave mode, set the bus to start communications
 * 	to handle the master mode requests.
 * */

static inline void Comms_I2C_Bus_Queue_Not_Empty(I2C_BUS_CHOICE bus, I2C_M_OR_S present)
{
	if (present==MASTER){
		*Bus_Regs[bus].Conset=(STA|STO);
		*Bus_Regs[bus].ConClr=(SI);
	}else{
		*Bus_Regs[bus].Conset=(STA);
		*Bus_Regs[bus].ConClr=(SI|AA);
	}
	I2C_Bus_Free=BUS_BUSY(bus);
}

/*-----------------------------------------------------------*/

/*
 * 	I2C Clear Requests
 *
 * 	To ensure that previous buffers used are not corrupted
 *  by new transfers, Control Structures should be cleared
 *  as soon as possible.
 * */

static inline void Comms_I2C_Clr_Req(volatile Bus_Master_Ctrl * master,volatile Bus_Slave_Ctrl * slave)
{
	if(master !=NULL){
		master->Index		= 0;
		master->SLA			= 0;
		master->Sem			= NULL;
		master->Fail		= NULL;
		master->Tx_Rx_Buff	= NULL;
		master->Index_Max	= NULL;
	}
	if (slave!=NULL){
		Comms_I2C_Clr_Slave_TX (slave);
		Comms_I2C_Clr_Slave_RX (slave);
	}
}

/*-----------------------------------------------------------*/

/*
 * 	Slave Control Structure Clear
 *
 * 	The TX and RX Clear functions are used to flush the slave
 * 	control structure when the buffers are no longer required
 * 	by the driver. This helps to minimise corruption of the buffers
 * 	by incoming states.
 *
 * 	These functions are intended for internal use for tasks, use the
 * 	Comms_I2C_Slave_Clear function.
 * */

static inline void Comms_I2C_Clr_Slave_RX (volatile Bus_Slave_Ctrl * slave)
{
	if (slave!=NULL){
		slave->Status_RX	= BUFF_EMPTY;
		slave->Sem_RX		= NULL;
		slave->Buf_RX		= NULL;
		slave->Max_RX		= NULL;
	}
}

static inline void Comms_I2C_Clr_Slave_TX (volatile Bus_Slave_Ctrl * slave)
{
	if (slave!=NULL){
		slave->Status_TX	= BUFF_EMPTY;
		slave->Sem_TX		= NULL;
		slave->Buf_TX		= NULL;
		slave->Max_TX		= NULL;
	}
}




