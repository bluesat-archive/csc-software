 /**
 *  \file gsa.c
 *
 *  \brief General Storage Architecture (GSA) provide memory management
 *
 *  \author $Author: James Qin $
 *  \version 1.0
 *
 *  $Date: 2010-10-24 23:35:54 +1100 (Sun, 24 Oct 2010) $
 *  \warning No Warnings for now
 *  \bug No Bugs for now
 *  \note No Notes for now
 */

#include "gsa.h"

typedef union
{
	struct
	{
		/* word 0 */
		unsigned portLONG CS_S1_O:     	 4;   //checksum stage 1 original
		unsigned portLONG CS_S1_I:     	 4;   //checksum stage 1 inverted
		unsigned portLONG CS_S2_O:     	 4;   //checksum stage 2 original
		unsigned portLONG CS_S2_I:	   	 4;   //checksum stage 2 inverted
		unsigned portLONG H:			 1;   //head block bit
		unsigned portLONG NTDBI:	  	15;	  //next trailing data block index
	};

	struct
	{
		/* word 0 */
		unsigned portLONG CS_S1_O:		 4; //checksum stage 1 original
		unsigned portLONG CS_S1_I:		 4; //checksum stage 1 inverted
		unsigned portLONG CS_S2_O: 		 4; //checksum stage 2 original
		unsigned portLONG CS_S2_I:		 4; //checksum stage 2 inverted
		unsigned portLONG H:			 1; //head block bit
		unsigned portLONG PrevHBI:		15;	//previous head block index
		/* word 1 */
		unsigned portLONG AID:			 6; //application ID
		unsigned portLONG DID:			 6; //data ID
		unsigned portLONG FTDBI_U:		 1;	//first trailing data block index used
		unsigned portLONG Terminal:		 1; //terminal block flag
		unsigned portLONG Upadding:		 2; //usable padding
		unsigned portLONG FTDBI:		15;	//first trailing date block index
		unsigned portLONG UUpadding:  	 1;	//unusable padding
	};

	unsigned portSHORT usHalfWords[HEADER_HALFWORD_SIZE];
	unsigned portLONG ulWords[HEADER_WORD_SIZE];
} Header;
