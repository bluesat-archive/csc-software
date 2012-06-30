 /**
 *  \file storageDemo.h
 *
 *  \brief An application demonstrating how to use CSC storage
 *
 *  \author $Author: James Qin $
 *  \version 1.0
 *
 *  $Date: 2010-10-24 23:35:54 +1100 (Sun, 24 Oct 2010) $
 *  \warning No Warnings for now
 *  \bug No Bugs for now
 *  \note No Notes for now
 */

#include "application.h"
#include "storageDemo.h"
#include "storage.h"
#include "memory.h"
#include "debug.h"
#include "lib_string.h"

#define STORAGE_DEMO_Q_SIZE		1
#define MAX_READ_BUF_SIZE		50

//task token for accessing services and other applications
static TaskToken StorageDemo_TaskToken;

static portTASK_FUNCTION(vStorageDemoTask, pvParameters);
void vPrintMenu(void);

void vStorageDemo_Init(unsigned portBASE_TYPE uxPriority)
{
	StorageDemo_TaskToken = ActivateTask(TASK_STORAGE_DEMO,
									"StorageDemo",
									APP_TASK_TYPE,
									uxPriority,
									APP_STACK_SIZE,
									vStorageDemoTask);

	vActivateQueue(StorageDemo_TaskToken, STORAGE_DEMO_Q_SIZE);
}

#define TEST_BUF_SIZE	2048
#define TEST_VALUE		0x42657374
static portTASK_FUNCTION(vStorageDemoTask, pvParameters)
{
	(void) pvParameters;
	UnivRetCode 		enResult;
	portCHAR			pcInputBuf[MAX_READ_BUF_SIZE+1];
	unsigned portSHORT	usReadLen;
	unsigned portLONG 	ulRetSize;
	void *pvTestBufferW = NULL;
	void *pvTestBufferR = NULL;

	vPrintMenu();

	for ( ; ; )
	{
		usReadLen = usDebugRead(pcInputBuf, MAX_READ_BUF_SIZE);

		if ((pcInputBuf[0] == 'S' && pcInputBuf[1] == 'D') && usReadLen > 4)
		{
			enResult = enDataStore(StorageDemo_TaskToken,
									ulDeciStringToVal(&pcInputBuf[2], 2),
									usReadLen-4,
									&pcInputBuf[4]);
		}
		else if ((pcInputBuf[0] == 'A' && pcInputBuf[1] == 'D') && usReadLen > 4)
		{
			enResult = enDataAppend(StorageDemo_TaskToken,
									ulDeciStringToVal(&pcInputBuf[2], 2),
									usReadLen-4,
									&pcInputBuf[4]);
		}
		else if ((pcInputBuf[0] == 'R' && pcInputBuf[1] == 'D') && usReadLen == 12)
		{
			enResult = enDataRead(StorageDemo_TaskToken,
								ulDeciStringToVal(&pcInputBuf[2], 2),
								ulDeciStringToVal(&pcInputBuf[8], 4),
								ulDeciStringToVal(&pcInputBuf[4], 4),
								pcInputBuf,
								&ulRetSize);

			pcInputBuf[ulRetSize] = 0;

			vDebugPrint(StorageDemo_TaskToken,
						"Data: %50s\n\r",
						(unsigned portLONG)pcInputBuf,
						NO_INSERT,
						NO_INSERT);
		}
		else if ((pcInputBuf[0] == 'C' && pcInputBuf[1] == 'S') && usReadLen == 4)
		{
			enResult = enDataSize(StorageDemo_TaskToken,
								ulDeciStringToVal(&pcInputBuf[2], 2),
								&ulRetSize);

			vDebugPrint(StorageDemo_TaskToken,
						"Request entry size: %d\n\r",
						ulRetSize,
						NO_INSERT,
						NO_INSERT);
		}
		else if ((pcInputBuf[0] == 'D' && pcInputBuf[1] == 'D') && usReadLen == 4)
		{
			enResult = enDataDelete(StorageDemo_TaskToken,
									ulDeciStringToVal(&pcInputBuf[2], 2));
		}
		else if ((pcInputBuf[0] == 'F' && pcInputBuf[1] == 'F') && usReadLen == 2)
		{
			enResult = enMgmtSysCmd(StorageDemo_TaskToken, 5);
		}
		else if ((pcInputBuf[0] == 'T' && pcInputBuf[1] == 'S') && usReadLen == 4)
		{
			if (pvTestBufferW == NULL) pvTestBufferW = pvJMalloc(TEST_BUF_SIZE);

			vDebugPrint(StorageDemo_TaskToken,
						"pvTestBufferW = %p\n\r",
						(unsigned portLONG)pvTestBufferW,
						NO_INSERT,
						NO_INSERT);

			if (pvTestBufferW != NULL)
			{
				for (usReadLen = 0; usReadLen < TEST_BUF_SIZE / sizeof(portLONG); ++usReadLen)
				{
					((unsigned portLONG *)pvTestBufferW)[usReadLen] = TEST_VALUE;
				}
				enResult = enDataStore(StorageDemo_TaskToken,
										ulDeciStringToVal(&pcInputBuf[2], 2),
										TEST_BUF_SIZE / 4,
										(portCHAR *)pvTestBufferW);
				enResult = enDataAppend(StorageDemo_TaskToken,
										ulDeciStringToVal(&pcInputBuf[2], 2),
										TEST_BUF_SIZE / 4,
										(portCHAR *)pvTestBufferW);
				enResult = enDataAppend(StorageDemo_TaskToken,
										ulDeciStringToVal(&pcInputBuf[2], 2),
										TEST_BUF_SIZE / 4,
										(portCHAR *)pvTestBufferW);
				enResult = enDataAppend(StorageDemo_TaskToken,
										ulDeciStringToVal(&pcInputBuf[2], 2),
										TEST_BUF_SIZE / 4,
										(portCHAR *)pvTestBufferW);
			}
			enResult = 1;
		}
		else if ((pcInputBuf[0] == 'T' && pcInputBuf[1] == 'T') && usReadLen == 8)
		{
			if (pvTestBufferR == NULL) pvTestBufferR = pvJMalloc(TEST_BUF_SIZE);

			vDebugPrint(StorageDemo_TaskToken,
						"pvTestBufferR = %p\n\r",
						(unsigned portLONG)pvTestBufferW,
						NO_INSERT,
						NO_INSERT);

			if (pvTestBufferR != NULL)
			{
				enResult = enDataRead(StorageDemo_TaskToken,
										ulDeciStringToVal(&pcInputBuf[2], 2),
										ulDeciStringToVal(&pcInputBuf[4], 4),
										TEST_BUF_SIZE - ulDeciStringToVal(&pcInputBuf[4], 4),
										(portCHAR *)pvTestBufferR,
										&ulRetSize);
				vDebugPrint(StorageDemo_TaskToken,
							"Test Read: %d bytes\n\r",
							ulRetSize,
							NO_INSERT,
							NO_INSERT);
				for (usReadLen = 0; usReadLen < TEST_BUF_SIZE - ulDeciStringToVal(&pcInputBuf[4], 4); ++usReadLen)
				{
					if (((unsigned portCHAR *)pvTestBufferR)[usReadLen]
					    != ((unsigned portCHAR *)pvTestBufferW)[usReadLen + ulDeciStringToVal(&pcInputBuf[4], 4)])
					{
					vDebugPrint(StorageDemo_TaskToken,
								"Invalid data: Position %d - CV:%p IV:%p\n\r",
								usReadLen,
								((unsigned portCHAR *)pvTestBufferW)[usReadLen + ulDeciStringToVal(&pcInputBuf[4], 4)],
								((unsigned portCHAR *)pvTestBufferR)[usReadLen]);
					}
				}
				vDebugPrint(StorageDemo_TaskToken,
							"Compare complete!\n\r",
							NO_INSERT,
							NO_INSERT,
							NO_INSERT);
			}
			enResult = 1;
		}
		else if ((pcInputBuf[0] == 'P' && pcInputBuf[1] == 'T') && usReadLen == 2)
		{
			vDebugPrint(StorageDemo_TaskToken,
						"PT not implemented!\n\r",
						NO_INSERT,
						NO_INSERT,
						NO_INSERT);
			continue;
		}
		else if ((pcInputBuf[0] == 'M' && pcInputBuf[1] == 'U') && usReadLen == 2)
		{
			vPrintMenu();
			continue;
		}
		else
		{
			vDebugPrint(StorageDemo_TaskToken,
						"Invalid command!\n\r",
						NO_INSERT,
						NO_INSERT,
						NO_INSERT);
			vPrintMenu();
			continue;
		}

		vDebugPrint(StorageDemo_TaskToken,
					"Result = %d\n\r",
					enResult,
					NO_INSERT,
					NO_INSERT);
	}
}

#define MENU_L00 " ----------------------------------- Menu -----------------------------------\n\r
#define MENU_L01 SD@@(Data)\t- Store Data @@(DID)\n\r
#define MENU_L02 AD@@(Data)\t- Append Data @@(DID)\n\r
#define MENU_L03 RD@@$$$$%%%%\t- Read Data @@(DID), $$$$(Size), %%%%(offset)\n\r
#define MENU_L04 CS@@\t\t- Check Size @@(DID)\n\r
#define MENU_L05 DD@@\t\t- Delete slot @@(DID)\n\r
#define MENU_L06 TS@@\t\t- Test Set @@(DID)\n\r
#define MENU_L07 TT@@%%%%\t- Test Test @@(DID), %%%%(offset)\n\r
#define MENU_L08 FF\t\t- Format\n\r
#define MENU_L09 PT\t\t- Print FMT Table\n\r
#define MENU_L10 MU\t\t- Display this menu\n\r
#define MENU_L11 Note: All value in decimal\n\r
#define MENU_L12 ----------------------------------------------------------------------------\n\r"
#define MENU MENU_L00 MENU_L01 MENU_L02 MENU_L03 MENU_L04 MENU_L05 MENU_L06 MENU_L07 MENU_L08 MENU_L09 MENU_L10 MENU_L11 MENU_L12

void vPrintMenu(void)
{
	vDebugPrint(StorageDemo_TaskToken,
				"\n\r%1000s\n\r",
				(unsigned portLONG)MENU,
				NO_INSERT,
				NO_INSERT);
}

