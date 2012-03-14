/**
 *  \file uart.h
 *
 *  \brief UART Driver to allow the writing and reading
 *         of characters out the UART port
 *
 *  \author $Author: Colin Tan $
 *  \version 1.0
 *
 *  $Date: 2010-05-15 17:16:42 +1000 (Sat, 15 May 2010) $
 *  \warning No Warnings for now
 *  \bug No Bugs for now
 *  \note No Notes for now
 */

#ifdef APPLICATION_H_
	#error "Applications should access drivers via services"
#endif

#ifndef UART_H_
#define UART_H_

enum UART_CHANNEL_ID
{
	READ0,
	WRITE0
};

/**
 * \brief Initialise the pins to UART port to allow serial
 *        communications.
 */
unsigned portLONG Comms_UART_Init(void);

/**
 * \brief Reads a single character off the port
 *
 * \param[in] xBlockTime Time to block, in ticks,  while waiting for the character.
 * \param[in] pcRxedChar Pointer to receive buffer for a single character.
 *
 * \returns pdTrue for success or pdFalse for failure
 */
signed portBASE_TYPE Comms_UART_Read_Char( signed portCHAR *pcRxedChar, portTickType xBlockTime );

/**
 * \brief Writes a single character onto the port
 *
 * \param[in] xBlockTime Time to block, in ticks,  while waiting for the character.
 * \param[out] cOutChar Output character to be written.
 *
 * \returns pdTrue for success or pdFalse for failure
 */
signed portBASE_TYPE Comms_UART_Write_Char( signed portCHAR cOutChar, portTickType xBlockTime );

/**
 * \brief Acquire UART channel
 *
 * \param[in] enChannelID Channel ID
 * \param[in] xBlockTime Time to block, in ticks
 *
 * \returns void
 */
void vAcquireUARTChannel(enum UART_CHANNEL_ID enChannelID, portTickType xBlockTime);

/**
 * \brief Release UART channel
 *
 * \param[in] enChannelID Channel ID
 *
 * \returns void
 */
void vReleaseUARTChannel(enum UART_CHANNEL_ID enChannelID);

#endif /* UART_H_ */
