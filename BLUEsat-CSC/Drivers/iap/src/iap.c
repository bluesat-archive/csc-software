/*
 * \file iap.c
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

#include "iap.h"
#include "task.h"

static unsigned portLONG command[MAX_COMMAND_SIZE];
static unsigned portLONG result[MAX_RESULT_SIZE];

// Function pointer declaration for in built IAP function
typedef void (*IAP)(unsigned portLONG *,unsigned portLONG *);

static IAP iap_entry = (IAP) IAP_LOCATION;

static void In_App_Prog(unsigned portLONG * ulpCmd, unsigned portLONG * ulpResult)
{
   taskENTER_CRITICAL();
   {
       iap_entry (ulpCmd, ulpResult);
   }
   taskEXIT_CRITICAL();
}

portLONG Find_Sector( unsigned portLONG cvpStartLoc,
                      unsigned portLONG ulFloor,
                      unsigned portLONG ulPivot,
                      unsigned portLONG ulCeiling)
{
   portLONG lRetVal = -1;
   unsigned int lTempPivot;
   unsigned portLONG ulTempLoc = cvpStartLoc;
   // Determine if the address is out of the bounds of the flash banks
   if ((ulTempLoc < FlashSecAdds[ulFloor])||(ulTempLoc >= UC_FLASH_LIMIT))
   {
      return lRetVal;
   }
   // Check for case where the address is in the last sector
   if (ulTempLoc > FlashSecAdds[MAX_NUM_SECTS])
   {
      lRetVal = MAX_NUM_SECTS;
   }
   // Check if the address is the pivot
   else if (ulTempLoc == FlashSecAdds[ulPivot])
   {
      lRetVal = ulPivot;
   }
   // Return the sector number from the final pass
   else if ((ulCeiling == ulPivot)||(ulFloor == ulPivot))//end of search
   {
      lRetVal = (ulTempLoc < FlashSecAdds[ulCeiling])?ulFloor:ulCeiling;
   }
   // Address is smaller than the pivot
   else if (ulTempLoc < FlashSecAdds[ulPivot])
   {
      lTempPivot =  ulFloor+((ulPivot-ulFloor)/2);
      lRetVal = Find_Sector(ulTempLoc, ulFloor,lTempPivot,ulPivot);
   }
   // Address is larger than the pivot
   else if (ulTempLoc > FlashSecAdds[ulPivot])
   {
      lTempPivot =  ulPivot+((ulCeiling-ulPivot)/2);
      lRetVal = Find_Sector(ulTempLoc, ulPivot,lTempPivot,ulCeiling);
   }
   return lRetVal;
}

static portLONG Addr_To_Sect(void * vpAddress)
{
   return Find_Sector( (unsigned portLONG) vpAddress, 0, MAX_NUM_SECTS/2, MAX_NUM_SECTS);
}

FLASH_STATUS_CODES Prep_Flash_Sect (unsigned portLONG enStart, unsigned portLONG enLast)
{
   command[0] = PREP_SECT;
   command[1] = (unsigned portLONG)enStart;
   command[2] = (unsigned portLONG)enLast;
   In_App_Prog((unsigned portLONG *)command,(unsigned portLONG *)result);
   return result[0];
}

static FLASH_STATUS_CODES Prep_Flash_Addr (void * vpAddress, BLOCK_SIZE enSize)
{
   unsigned portLONG ulBase = (unsigned portLONG)vpAddress;
   unsigned portLONG ulEnd  = (unsigned portLONG)vpAddress + enSize - 1;

   portLONG enStart = Addr_To_Sect((void *)ulBase);
   portLONG enLast  = Addr_To_Sect((void *)ulEnd);
   if (enStart == -1)
   {
      return SRC_ADDR_ERROR;
   }
   else if (ulEnd == (unsigned portLONG)(-1))
   {
      return DST_ADDR_ERROR;
   }

   return Prep_Flash_Sect ( enStart, enLast);
}


FLASH_STATUS_CODES Ram_To_Flash(void * vpDest, void * vpSrc, BLOCK_SIZE enSize)
{
   FLASH_STATUS_CODES scResult = Prep_Flash_Addr (vpDest, enSize);
   if (scResult != 0)
   {
      return scResult;
   }

   command[0] = COPY_RAM_TO_FLASH;
   command[1] = (unsigned portLONG)vpDest;
   command[2] = (unsigned portLONG)vpSrc;
   command[3] = enSize;
   command[4] = configCPU_CLOCK_KHZ_RAW;
   In_App_Prog(command,result);
   return result[0];
}

FLASH_STATUS_CODES Erase_Sector (SECTORS enStart, SECTORS enLast)
{
   FLASH_STATUS_CODES scResult = Prep_Flash_Sect ((unsigned portLONG) enStart, (unsigned portLONG) enLast);
   if (scResult != 0)
   {
      return scResult;
   }
   command[0] = ERASE_SECTS;
   command[1] = enStart;
   command[2] = enLast;
   command[3] = configCPU_CLOCK_KHZ_RAW;
   In_App_Prog((unsigned portLONG *)&command,(unsigned portLONG *)&result);
   return result[0];
}

FLASH_STATUS_CODES Check_Blank (SECTORS enStart, SECTORS enLast, unsigned portLONG * uipNotBlankOffset, unsigned portLONG * uipNonBlankContents)
{
   command[0] = BLANK_CHECK;
   command[1] = enStart;
   command[2] = enLast;
   In_App_Prog((unsigned portLONG *)&command,(unsigned portLONG *)&result);
   *uipNotBlankOffset    =  result[1];
   *uipNonBlankContents  =  result[2];
   return result[0];
}

FLASH_STATUS_CODES Compare_Data (const void * vpDest_c, const void * vpSrc_c, unsigned portLONG uiSize, unsigned portLONG * uipMismatchOff)
{
   command[0] = COMPARE_ADDRS;
   command[1] = (unsigned portLONG)vpDest_c;
   command[2] = (unsigned portLONG)vpSrc_c;
   command[3] = uiSize;
   In_App_Prog((unsigned portLONG *)&command,(unsigned portLONG *)&result);
   *uipMismatchOff = result[1];
   return result[0];
}

