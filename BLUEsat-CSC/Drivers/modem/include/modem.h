/**
 *  \file modem.h
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


#ifdef APPLICATION_H_
	#error "Applications should access drivers via services!"
#endif

#ifndef MODEM_H_
#define MODEM_H_

#define MODEM_INTERRUPTS			( ( unsigned portCHAR ) 0x05 )
#define CLEAR_VIC_INTERRUPT		( ( unsigned portLONG ) 0 )

#define MAX_INFO_SIZE						256
#define BUFFER_SIZE						(20+MAX_INFO_SIZE)

#define MODEM_1 1
#define MODEM_2 2

#define MODEM_NO_BLOCK 0

void Comms_Modem_Timer_Init(void);

signed portBASE_TYPE Comms_Modem_Write_Char( portCHAR cOutChar, portTickType xBlockTime);
void Comms_Modem_Write_Str( const  portCHAR * const pcString, unsigned portSHORT usStringLength);
void Comms_Modem_Write_Hex(const void * const loc, unsigned portSHORT usStringLength);
void setModemTransmit(portSHORT sel);
void setModemReceive(portSHORT sel);
void modem_takeSemaphore(void);
void modem_giveSemaphore(void);
#endif /* MODEM_H_ */
