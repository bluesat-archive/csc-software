/**
 *  \file emc.c
 *
 *  \brief External Memory Controller for access
 *  		static memory onboard.
 *
 *  \author $Author: James Qin $
 *  \version 1.0
 *
 *  $Date: 2012-03-18 17:16:42 +1000 (Sun, 18 Mar 2012) $
 *  \warning No Warnings for now
 *  \bug No Bugs for now
 *  \note No Notes for now
 */

#include "emc.h"

//EMC power bit
#define PCEMC_SET 			0x800

/********* Static Memory **********/
	/* EMC Control register */
//EMC enable
#define EMC_EN 				0x00000001
//Address mirror
#define ADDR_MIRROR 		0x00000002
//Low power mode
#define LOW_POW				0x00000004

	/* EMC Status register */
//Bit position
#define BUSY				0
#define WRITE_BUFF_STATUS	1
#define SELF_REFRESH_ACK	2

	/* EMC Config register */
//Endianness
#define LITTLE_ENDIAN		0x00000000
#define BIG_ENDIAN			0x00000001

	/* EMC Config0-3 */
//Memory Width
#define MEM_WIDTH_8			0x00000000
#define MEM_WIDTH_16		0x00000001
#define MEM_WIDTH_32		0x00000002
//Page mode
#define ASY_PAGE_MODE		0x00000008
//Chip select polarity
#define CS_POL_LOW			0x00000000
#define CS_POL_HIGH			0x00000040
//Byte lane state
#define R_HIGH_W_LOW		0x00000000
#define R_LOW_W_LOW			0x00000080
//Extended wait
#define EXTD_WAIT_EN		0x00000100
//Buffer enable
#define BUFF_EN				0x00080000
//Write protect
#define WRTIE_PROTECT		0x00100000

/**********************************/

/**
 * \brief Initialise static memory banks.
 */
void vInitStaticMemoryBanks(void);

void EMC_Init(void)
{
	// Pin connect block setup

	//D[0..15], mode = 10
	PINSEL6  =  0x55555555;
	PINMODE6 =  0xAAAAAAAA;

	//A[0..15], mode = 10
	PINSEL8  =  0x55555555;
	PINMODE8 =  0xAAAAAAAA;

	//A[16..23], !OE, !WE, !CS0, !CS1, BLS0-BLS3, mode = 10
	PINSEL9  =  0x50555155;
	PINMODE9 =  0xA0AAA2AA;

	// enable EMC power
	PCONP 	|= 	PCEMC_SET;

	// enable EMC
	EMC_CTRL = 	EMC_EN;

	//initialise static memories
	vInitStaticMemoryBanks();
}

//static banks initialise control
#define STATIC_BANK_0_ENABLED	TRUE
#define STATIC_BANK_1_ENABLED	FALSE
#define STATIC_BANK_2_ENABLED	FALSE
#define STATIC_BANK_3_ENABLED	FALSE
//(FRAM - FM22L16)
#define FM22L16_WWEN			0xf
#define FM22L16_WOEN			0x4
#define FM22L16_WRD				0x1f
#define FM22L16_WPAGE			0x7
#define FM22L16_WWR				0x1f
#define FM22L16_WTURN			0xf
//(SRAM - CY62167DV30)
#define CY62167DV30_WWEN		0x3;
#define CY62167DV30_WOEN		0x1;
#define CY62167DV30_WRD			0x4;
#define CY62167DV30_WPAGE		0x0;
#define CY62167DV30_WWR			0x4;
#define CY62167DV30_WTURN		0x1;

void vInitStaticMemoryBanks(void)
{
#if STATIC_BANK_0_ENABLED == TRUE
	//static memory bank general configuration
	EMC_STA_CFG0 		= MEM_WIDTH_8 | R_LOW_W_LOW;
	//static memory bank timing configuration
	EMC_STA_WAITWEN0	= FM22L16_WWEN;
	EMC_STA_WAITOEN0	= FM22L16_WOEN;
	EMC_STA_WAITRD0		= FM22L16_WRD;
	EMC_STA_WAITPAGE0	= FM22L16_WPAGE;
	EMC_STA_WAITWR0		= FM22L16_WWR;
	EMC_STA_WAITTURN0	= FM22L16_WTURN;

#endif
#if STATIC_BANK_1_ENABLED == TRUE
	//static memory bank general configuration
	EMC_STA_CFG1 		= MEM_WIDTH_16 | R_LOW_W_LOW;
	//static memory bank timing configuration
	EMC_STA_WAITWEN1	= CY62167DV30_WWEN;
	EMC_STA_WAITOEN1	= CY62167DV30_WOEN;
	EMC_STA_WAITRD1		= CY62167DV30_WRD;
	EMC_STA_WAITPAGE1	= CY62167DV30_WPAGE;
	EMC_STA_WAITWR1		= CY62167DV30_WWR;
	EMC_STA_WAITTURN1	= CY62167DV30_WTURN;

#endif
#if STATIC_BANK_2_ENABLED == TRUE
	//static memory bank general configuration
	EMC_STA_CFG2 		= MEM_WIDTH_16 | R_LOW_W_LOW;
	//static memory bank timing configuration
	EMC_STA_WAITWEN2	= CY62167DV30_WWEN;
	EMC_STA_WAITOEN2	= CY62167DV30_WOEN;
	EMC_STA_WAITRD2		= CY62167DV30_WRD;
	EMC_STA_WAITPAGE2	= CY62167DV30_WPAGE;
	EMC_STA_WAITWR2		= CY62167DV30_WWR;
	EMC_STA_WAITTURN2	= CY62167DV30_WTURN;

#endif
#if STATIC_BANK_3_ENABLED == TRUE
	//static memory bank general configuration
	EMC_STA_CFG3 		= MEM_WIDTH_16 | R_LOW_W_LOW;
	//static memory bank timing configuration
	EMC_STA_WAITWEN3	= CY62167DV30_WWEN;
	EMC_STA_WAITOEN3	= CY62167DV30_WOEN;
	EMC_STA_WAITRD3		= CY62167DV30_WRD;
	EMC_STA_WAITPAGE3	= CY62167DV30_WPAGE;
	EMC_STA_WAITWR3		= CY62167DV30_WWR;
	EMC_STA_WAITTURN3	= CY62167DV30_WTURN;

#endif
}

