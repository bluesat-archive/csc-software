 /**
 *  \file debug.h
 *
 *  \brief Using UART to output debug message to terminal
 *
 *  \author $Author: James Qin $
 *  \version 1.0
 *
 *  $Date: 2010-10-24 23:35:54 +1100 (Sun, 24 Oct 2010) $
 *  \warning No Warnings for now
 *  \bug No Bugs for now
 *  \note No Notes for now
 */

#ifndef DEBUG_H_
#define DEBUG_H_

#define NO_INSERT	0

#include "command.h"
#include "UniversalReturnCode.h"

	#ifndef NO_DEBUG
		/**
		 * \brief Initialise debug service
		 *
		 * \param[in] uxPriority Priority for debug service.
		 */
		void vDebug_Init(unsigned portBASE_TYPE uxPriority);

		/**
		 * \brief Read from UART
		 *
		 * \param[in] pcBuffer Pointer to buffer
		 * \param[in] usMaxSize Max number of bytes to be read
		 *
		 * \returns Number of bytes read
		 */
		unsigned portSHORT	usDebugRead(portCHAR *			pcBuffer,
										unsigned portSHORT 	usMaxSize);

		/**
		 * \brief Print debug messages.
		 * Usage similar to printf except only the following options are available
		 * %% 		= Normal % character
		 * %####s 	= Print a string where #### specify the max possible length of the string
		 * %####x 	= Print a string in hex where #### specify the max possible length of the string
		 * %h		= Print value in hex decimal
		 * %d		= Print value in decimal
		 *
		 * \param[in] taskToken Task token from request task
		 * \param[in] pcFormat Print format.
		 * \param[in] pcInsertion_1 Insertion data 1.
		 * \param[in] pcInsertion_2 Insertion data 2.
		 * \param[in] pcInsertion_3 Insertion data 3.
		 */
		void vDebugPrint(TaskToken 			taskToken,
						portCHAR *			pcFormat,
						unsigned portLONG 	pcInsertion_1,
						unsigned portLONG 	pcInsertion_2,
						unsigned portLONG 	pcInsertion_3);
	#else

		//catch and nullify function calls
		#define vDebug_Init(a)
		#define usDebugRead(a, b)	0
		#define vDebugPrint(a, b, c, d, e)

	#endif /* NO_DEBUG */
#endif /* DEBUG_H_ */
