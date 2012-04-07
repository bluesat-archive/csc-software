
/**
 *  \file Comms_I2C.h
 *
 *  \brief The I2C driver function is to be used by tasks who wish to communicate to
 * other devices via the I2C port.
 *
 *  \author $Author: Colin Tan $
 *  \version 1.0
 *
 *  $Date: 2010-10-09 10:23:13 +1100 (Sat, 09 Oct 2010) $
 *  \warning No Warnings for now
 *  \bug No Bugs for now
 *  \note  At present the system is designed for only one task to be waiting on an operation.
 * This can be fixed in the future by having the tasks create the semaphores and pass it to
 * the driver to be given after execution.
 *
 *  \note This Driver has been written for a single I2C bus and should be extended to handle
 *  3 busses to act as a common code base.
 *
 *  \note Semaphore is not managed by the driver and should be declared outside of the driver.
 */

#ifdef APPLICATION_H_
	#error "Applications should access drivers via services!"
#endif

#ifndef COMMS_I2C_H_
#define COMMS_I2C_H_


/*
 * Address of Devices
 * */
#define I2C_CRC_ADDR  	(0x04<<1)
#define I2C_CSC_ADDR	(0x02<<1)
#define I2C_FC_ADDR		(0x06<<1) /*FIX ME: I2C address of SA1100 and variable POTS. Last bit of POTS address is R/W*/
#define I2C_VREG1_ADDR 	(0x2C<<1)
#define I2C_VREG2_ADDR 	(0x2D<<1)

#define I2C_NODE portCHAR

/*---------------------------------------------------------------------------------------------*/

/*
 * State Codes
 * */
/*Master Mode*/
#define I2C_M_START_CON       0x08	//Start Condition
#define I2C_M_START_CON_R     0x10	//Repeat Start Condition
#define I2C_M_ARB_LOST        0x38	//Arbitration lost
//Transmit Mode
#define I2C_MTM_INIT_ACKED	    0x18	//Slave ready for data
#define I2C_MTM_INIT_NACKED		0x20
#define I2C_MTM_DATA_ACKED	   	0x28
#define I2C_MTM_DATA_NACKED		0x30
//Receive Mode
#define I2C_MRM_INIT_ACKED	   	0x40
#define I2C_MRM_INIT_NACKED		0x48
#define I2C_MRM_DATA_ACKED	   	0x50
#define I2C_MRM_DATA_NACKED		0x58

/*Slave Mode*/
//Receive Mode
#define I2C_SRM_CALLED_ACKED				0x60	//Address Called and ACK replied
#define I2C_SRM_ARB_LOST			 	    0x68	//Arbitration lost
#define I2C_SRM_GEN_CALL_ACKED				0x70	//General Call received and ACK replied
#define I2C_SRM_ARB_LOST_GEN_CALL_ACKED		0x78	//Arbitration lost & general call received and ACK replied
#define I2C_SRM_DATA_ACKED					0x80	//Data received and ACKED
#define I2C_SRM_DATA_NACKED					0x88	//Data received and NACKED
#define I2C_SRM_GEN_CALL_DATA_ACKED			0x90
#define I2C_SRM_GEN_CALL_DATA_NACKED		0x98
#define I2C_SRM_STOP_RSTART					0xA0	//Stop/Repeat Start received while still addressed
//Transmit Mode
#define I2C_STM_CALLED_ACKED			  	0xA8
#define I2C_STM_ARB_LOST					0xB0
#define I2C_STM_DATA_ACKED					0xB8
#define I2C_STM_DATA_NACKED					0xC0
#define I2C_STM_DATA_END_ACKED				0xC8

/*Misc States*/
#define I2C_NO_STATE	   	0xF8
#define	I2C_BUS_ERR			0x00

/*Control Bits*/
#define I2EN	0x40
#define STA		0x20
#define STO		0x10
#define SI		0x08
#define AA		0x04


#ifndef I2C_BLOCK_TIME
	#define I2C_BLOCK_TIME 5000
#endif



typedef enum{
	I2C_WRITE,
	I2C_READ
}I2C_OPPERATION;

typedef enum{
	SLAVE_TRANSMITTER,
	SLAVE_RECEIVER
}I2C_SLAVE_MODE;

typedef enum{
	PTT_PUSH,
	PTT_RELEASE
}I2C_INSTR;

typedef enum{
	BUFF_EMPTY,
	BUFF_SET,
	BUFF_BUSY,
	BUFF_COMPLETED
}I2C_SLAVE_STATUS;

typedef enum{
	BUS0,
	BUS1,
	BUS2
}I2C_BUS_CHOICE;

typedef enum{
	MASTER,
	SLAVE,
	MASTER_SLAVE
}I2C_M_OR_S;



typedef struct {
	portCHAR 	SLA;
	portLONG	Index_Max;
	portCHAR * 	Buff;
}I2C_Q_Elem;

/**
 * \brief Initialise the pins to I2C port to allow communications.
 */
void Comms_I2C_Init(void);

/**
 * \brief Perform an operation as a master
 *
 * \param[in] dest Destination node address
 * \param[in] op Operation code to be performed by the slave
 * \param[out] result Pointer to flag indicating if I2C commas has been a success
 * \param[in,out] loc Pointer to the buffer which will contain data to be sent and the result
 * \param[out] len Pointer to the buffer which will hold the length of the result that was returned
 * \param[in]  semaphore Semaphore of the calling task to be woken up by the driver upon completion
 * \param[in] bus I2C bus to be used
 * \returns pdTrue for success or pdFalse for failure
 */
signed portBASE_TYPE Comms_I2C_Master(I2C_NODE dest, I2C_OPPERATION op,volatile portCHAR* result, volatile portCHAR* loc, volatile portSHORT *len,  xSemaphoreHandle semaphore,I2C_BUS_CHOICE bus);

/**
 * \brief Perform an operation as a slave
 *
 * \param[in] mode Slave mode to be in
 * \param[out] loc Pointer to receive buffer
 * \param[out] len Pointer to receive length buffer
 * \param[in] semaphore Semphore for calling task, so task can be woken up
 * \param[in] bus I2C bus for the call to go out on
 * \returns pdTrue for success or pdFalse for failure
 */
signed portBASE_TYPE Comms_I2C_Slave(I2C_SLAVE_MODE mode, volatile portCHAR * loc, volatile portSHORT* len, xSemaphoreHandle semaphore, I2C_BUS_CHOICE bus);

/**
 * \brief Clear the Slave to minimise data corruption
 *
 * \param[in] bus Bus to clear.
 * \param[in] mode Clear the receive or transmit buffer depending if a receive
 *            or transmit is about to be performed.
 *
 * \returns pdTrue for success or pdFalse for failure
 */
signed portBASE_TYPE Comms_I2C_Slave_Clear(I2C_BUS_CHOICE bus , I2C_SLAVE_MODE mode);


#endif /* COMMS_I2C_H_ */
