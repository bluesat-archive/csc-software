/**
 *  \file Comms_DTMF.h
 *  \brief DTMF driver to be used to interface the UC to the
 *  DTMF Decoder chip.
 *
 *  Comms_DTMF is a driver sitting there waiting for some interrupts.
 *  When an interrupt comes in we read the two DTMF port values (remember
 *  there are two DTMF receives), and then chuck that onto a queue.
 *
 *  $Author: Colin Tan $
 *  \version 1.0
 *  $Date: 2010-05-15 15:55:30 +1000 (Sat, 15 May 2010) $
 *  \warning No Warnings for now
 *  \bug No Bugs for now
 *  \note No Notes for now
 *
 */
#ifndef COMMS_DTMF_H_
#define COMMS_DTMF_H_

#include "FreeRTOS.h"
#include "DTMF_Common.h"


/**
 * \brief Initialise the queue, interrupts and GPIO for the DTMF driver.
 */
void Comms_DTMF_Init(void);

/**
 * \brief Reads a value off the queue. The value will be 8 bits long and
 *        contain a tone [0-3] and a decoder[4-7]
 *
 * \param[in] xBlockTime Time to block. This should be infinite
 * \param[in] elem Pointer to receive buffer
 */
void Comms_DTMF_Read( DtmfTone *DTMF_elem, int *new, portTickType xBlockTime );


#endif /* COMMS_DTMF_H_ */
