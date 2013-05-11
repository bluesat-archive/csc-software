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

#define DTMF_SIZE 64

/**
 * \brief Initialise the queue, interrupts and GPIO for the DTMF driver.
 */
void Comms_DTMF_Init(void);


#endif /* COMMS_DTMF_H_ */
