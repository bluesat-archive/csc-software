 /**
 *  \file gsa.c
 *
 *  \brief General Storage Architecture (GSA) provide data storage organisation
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

#define HB_HEADER_SIZE 			6
#define HB_HEADER_EXT_SIZE 		8
#define DB_HEADER_SIZE 			4
#define HEADER_HALFWORD_SIZE 	(HB_HEADER_EXT_SIZE / 2)
#define HEADER_WORD_SIZE 		(HB_HEADER_EXT_SIZE / 4)

typedef union
{
	struct
	{
		/* word 0 */
		unsigned portLONG CS_S1_O:     	 4;	//checksum stage 1 original
		unsigned portLONG CS_S1_I:     	 4;	//checksum stage 1 inverted
		unsigned portLONG CS_S2_O:     	 4;	//checksum stage 2 original
		unsigned portLONG CS_S2_I:	   	 4;	//checksum stage 2 inverted
		unsigned portLONG H:			 1;	//head block bit
		unsigned portLONG NDBI:	  		15;	//Next Data Block Index
	};

	struct
	{
		/* word 0 */
		unsigned portLONG CS_S1_O:		 4;	//checksum stage 1 original
		unsigned portLONG CS_S1_I:		 4; //checksum stage 1 inverted
		unsigned portLONG CS_S2_O: 		 4; //checksum stage 2 original
		unsigned portLONG CS_S2_I:		 4; //checksum stage 2 inverted
		unsigned portLONG H:			 1; //head block bit
		unsigned portLONG PrevHBI:		15;	//previous Head Block Index
		/* word 1 */
		unsigned portLONG AID:			 6; //Application ID
		unsigned portLONG DID:			 6; //Data ID
		unsigned portLONG FDBI_U:		 1;	//First Data Block Index used
		unsigned portLONG Terminal:		 1; //terminal block flag
		unsigned portLONG Upadding:		 2; //usable padding
		unsigned portLONG FDBI:			15;	//First Data Block Index
		unsigned portLONG UUpadding:  	 1;	//unusable padding
	};

	unsigned portSHORT 	usHalfWords	[	HEADER_HALFWORD_SIZE];
	unsigned portLONG 	ulWords		[		HEADER_WORD_SIZE];
} *Header;


