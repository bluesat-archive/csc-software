/*
 * \file iap.h
 *
 * \brief In-Application (IAP) programming is performing erase
 *        and write operation on the on-chip Flash memory.
 *
 * \author $Author: James Qin $
 * \version 1.0
 *
 * $Date: 2011-02-26 13:22:30 +1100 (Sat, 26 Feb 2011) $
 *
 */

#ifndef IAP_H_
#define IAP_H_

#include "FreeRTOS.h"
#include "UC_Selection.h"

#define MAX_COMMAND_SIZE	5
#define MAX_RESULT_SIZE		5

#ifdef __LPC24xx_H

   static unsigned portLONG FlashSecAdds[] = {0x00000000,
                                             0x00002000,
                                             0x00004000,
                                             0x00006000,
                                             0x00008000,
                                             0x0000A000,
                                             0x0000C000,
                                             0x0000E000,
                                             0x00010000,
                                             0x00012000,
                                             0x00014000,
                                             0x00016000,
                                             0x00018000,
                                             0x0001A000,
                                             0x0001C000,
                                             0x0001E000};

   typedef enum{
      SECTOR0ADDR     = 0x00000000,
      SECTOR1ADDR     = 0x00002000,
      SECTOR2ADDR     = 0x00004000,
      SECTOR3ADDR     = 0x00006000,
      SECTOR4ADDR     = 0x00008000,
      SECTOR5ADDR     = 0x0000A000,
      SECTOR6ADDR     = 0x0000C000,
      SECTOR7ADDR     = 0x0000E000,
      SECTOR8ADDR     = 0x00010000,
      SECTOR9ADDR     = 0x00012000,
      SECTOR10ADDR    = 0x00014000,
      SECTOR11ADDR    = 0x00016000,
      SECTOR12ADDR    = 0x00018000,
      SECTOR13ADDR    = 0x0001A000,
      SECTOR14ADDR    = 0x0001C000,
      SECTOR15ADDR    = 0x0001E000
   }SECTOR_ADDRESS;

   typedef enum{
      SECTOR0     = 0x00,
      SECTOR1     = 0x01,
      SECTOR2     = 0x02,
      SECTOR3     = 0x03,
      SECTOR4     = 0x04,
      SECTOR5     = 0x05,
      SECTOR6     = 0x06,
      SECTOR7     = 0x07,
      SECTOR8     = 0x08,
      SECTOR9     = 0x09,
      SECTOR10    = 0x0A,
      SECTOR11    = 0x0B,
      SECTOR12    = 0x0C,
      SECTOR13    = 0x0D,
      SECTOR14    = 0x0E,
      SECTOR15    = 0x0F
   }SECTORS;
#define MAX_NUM_SECTS 15 //NOTE: This value refers to the maximum index number in the sectors array
#define UC_FLASH_LIMIT 0x00020000

#elif defined(__LPC21xx_H)

   static unsigned portLONG FlashSecAdds[] = {0x00000000,
                                             0x00002000,
                                             0x00004000,
                                             0x00006000,
                                             0x00008000,
                                             0x0000A000,
                                             0x0000C000,
                                             0x0000E000,
                                             0x00010000,
                                             0x00020000,
                                             0x00030000,
                                             0x00032000,
                                             0x00034000,
                                             0x00036000,
                                             0x00038000,
                                             0x0003A000,
                                             0x0003C000,
                                             0x0003E000};

   typedef enum{
      SECTOR0ADDR     = 0x00000000,
      SECTOR1ADDR     = 0x00002000,
      SECTOR2ADDR     = 0x00004000,
      SECTOR3ADDR     = 0x00006000,
      SECTOR4ADDR     = 0x00008000,
      SECTOR5ADDR     = 0x0000A000,
      SECTOR6ADDR     = 0x0000C000,
      SECTOR7ADDR     = 0x0000E000,
      SECTOR8ADDR     = 0x00010000,
      SECTOR9ADDR     = 0x00020000,
      SECTOR10ADDR    = 0x00030000,
      SECTOR11ADDR    = 0x00032000,
      SECTOR12ADDR    = 0x00034000,
      SECTOR13ADDR    = 0x00036000,
      SECTOR14ADDR    = 0x00038000,
      SECTOR15ADDR    = 0x0003A000,
      SECTOR16ADDR    = 0x0003C000,
      SECTOR17ADDR    = 0x0003E000
   }SECTOR_ADDRESS;

   typedef enum{
      SECTOR0     = 0x00,
      SECTOR1     = 0x01,
      SECTOR2     = 0x02,
      SECTOR3     = 0x03,
      SECTOR4     = 0x04,
      SECTOR5     = 0x05,
      SECTOR6     = 0x06,
      SECTOR7     = 0x07,
      SECTOR8     = 0x08,
      SECTOR9     = 0x09,
      SECTOR10    = 0x0A,
      SECTOR11    = 0x0B,
      SECTOR12    = 0x0C,
      SECTOR13    = 0x0D,
      SECTOR14    = 0x0E,
      SECTOR15    = 0x0F,
      SECTOR16    = 0x10,
      SECTOR17    = 0x11
   }SECTORS;

#define MAX_NUM_SECTS 17 //NOTE: This value refers to the maximum index number in the sectors array
#define UC_FLASH_LIMIT 0x00040000
#endif


typedef enum{
   KB_HALF  = 512,
   KB       = 1024,
   KB_FOUR  = 4096,
   KB_EIGHT = 8192
}BLOCK_SIZE;

typedef enum{

   CMD_SUCCESS          = 0,  //Command is executed successfully.
   INVALID_COMMAND      = 1,  //Invalid command.
   SRC_ADDR_ERROR       = 2,  //Source address is not on a word boundary.
   DST_ADDR_ERROR       = 3,  //Destination address is not on a correct boundary.
   SRC_ADDR_NOT_MAPPED  = 4,  //Source address is not mapped in the memory map.
   DST_ADDR_NOT_MAPPED  = 5,  //Destination address is not mapped in the memory map.
   COUNT_ERROR          = 6,  //Byte count is not multiple of 4 or is not a permitted value.
   INVALID_SECTOR       = 7,  //Sector number is invalid.
   SECTOR_NOT_BLANK     = 8,  //Sector is not blank.
   SECTOR_NOT_PREPARED  = 9,  //Command to prepare sector for write operation was not executed.
   COMPARE_ERROR        = 10, //Source and destination data is not same.
   BUSY                 = 11, //Flash programming hardware interface is busy.
}FLASH_STATUS_CODES;

typedef enum{
   PREP_SECT         	= 50,
   COPY_RAM_TO_FLASH 	= 51,
   ERASE_SECTS       	= 52,
   BLANK_CHECK       	= 53,
   READ_PART_ID      	= 54,
   READ_BOOT_CODE_VER 	= 55,
   COMPARE_ADDRS     	= 56
}IAP_CMD_CODES;

/**
 * \brief Binary Search to determine the flash block to use
 *
 * \param[in] cvpStartLoc  The address to be determined
 * \param[in] ulFloor      The smallest sector number in the range in consideration
 * \param[in] ulPivot      The middle sector number in the range in consideration
 * \param[in] ulCeiling    The largest sector number in the range in consideration
 * \returns The sector number containing the desired address or -1 if the address is
 *          out of the bounds of the system flash memory banks.
 */
portLONG Find_Sector( unsigned portLONG cvpStartLoc,
                      unsigned portLONG ulFloor,
                      unsigned portLONG ulPivot,
                      unsigned portLONG ulCeiling);

/**
 * \brief Prep FLASH for write and erase operations
 *
 * \param[in] enStart Starting Sector nNmber
 * \param[in] enLast  End Sector Number
 * \returns Status code indicating if there was an error and if so
 *          what the error was caused by.
 *
 * \note This command must be executed before executing "Copy RAM to
 *       Flash" or "Erase Sector(s)" command. Successful execution
 *       of the "Copy RAM to Flash" or "Erase Sector(s)" command
 *       causes relevant sectors to be protected again.
 *       The boot sector can not be prepared by this command.
 *       To prepare a single sector use the same "Start" and "End"
 *       sector numbers.
 */
FLASH_STATUS_CODES Prep_Flash_Sect (unsigned portLONG enStart, unsigned portLONG enLast);

/**
 * \brief Copy data from RAM to FLASH
 *
 * \param[in] vpDest Destination Flash address where data bytes are to be written.
 *                   The destination address should be a 512 byte boundary.
 * \param[in] vpSrc  Source RAM address from which data bytes are to be read.
 *                   This address should be on word boundary.
 * \param[in] enSize  Number of bytes to be written,
 *                   Should be 512 | 1024 | 4096 | 8192.
 * \returns Status code indicating if there was an error and if so
 *          what the error was caused by.
 */
FLASH_STATUS_CODES Ram_To_Flash(void * vpDest, void * vpSrc, BLOCK_SIZE enSize);

/**
 * \brief Erase data in sector sector range
 *
 * \param[in] enStart Starting sector to be erased
 * \param[in] enLast  Last Sector to be erased
 * \returns Status code indicating if there was an error and if so
 *          what the error was caused by.
 * \note    To erase a single sector use the same "Start" and "End"
 *          sector numbers.
 * \note    The boot sector can not be erased by this command.
 */
FLASH_STATUS_CODES Erase_Sector (SECTORS enStart, SECTORS enLast);

/**
 * \brief Check if sector / sector range is blank
 *
 * \param[in] enStart Starting sector to be checked
 * \param[in] enLast  Last Sector to be checked
 * \param[out] uipNotBlankOffset
 *                   Offset of the first non blank word location
 *                   if the Status Code is SECTOR_NOT_BLANK.
 * \param[out] uipNonBlankContents
 *                   Contents of non blank word location.
 * \returns Status code indicating if there was an error and if so
 *          what the error was caused by.
 * \note    To blank check a single sector use the same
 *          "Start" and "End" sector numbers.
 */
FLASH_STATUS_CODES Check_Blank (SECTORS enStart, SECTORS enLast, unsigned portLONG * uipNotBlankOffset, unsigned portLONG * uipNonBlankContents);

/**
 * \brief Compare date at two locations
 *
 * \param[in] vpDest_c  Starting Flash or RAM address from where data bytes are
 *                      to be compared. This address should be a word boundary.
 * \param[in] vpSrc_c   Starting Flash or RAM address from where data bytes are
 *                      to be compared. This address should be a word boundary.
 * \param[in] uiSize    Number of bytes to be compared.
 *                      Count should be in multiple of 4.
 * \param[out] uipMismatchOff Offset of the first mismatch if the Status Code
 *                            is COMPARE_ERROR.
 * \returns Status code indicating if there was an error and if so
 *          what the error was caused by.
 */
FLASH_STATUS_CODES Compare_Data (const void * vpDest_c, const void * vpSrc_c, unsigned portLONG uiSize, unsigned portLONG * uipMismatchOff);

#endif /* IAP_H_ */
