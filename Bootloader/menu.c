/**
  ******************************************************************************
  * @file    STM32F2xx_IAP/src/menu.c 
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    02-May-2011
  * @brief   This file provides the software which contains the main menu routine.
  *          The main menu gives the options of:
  *             - downloading a new binary file, 
  *             - uploading internal flash memory,
  *             - executing the binary file already loaded 
  *             - disabling the write protection of the Flash sectors where the 
  *               user loads his binary file.
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */ 

/** @addtogroup STM32F2xx_IAP
  * @{
  */

/* Includes ------------------------------------------------------------------*/
#include "bootloader.h"
#include "common.h"
#include "menu.h"
#include "ymodem.h"
#include "stdio.h"
#include "string.h"
#include "platform_common_config.h"
#include "StringUtils.h"
#include "MicoPlatform.h"
#include <ctype.h>                    /* character functions                 */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
uint8_t tab_1024[1024] =
  {
    0
  };
uint8_t FileName[FILE_NAME_LENGTH];
char ERROR_STR [] = "\n\r*** ERROR: %s\n\r";    /* ERROR message string in code   */

extern const char menu[];
extern void getline (char *line, int n);          /* input line               */
extern void startApplication(void);

/* Private function prototypes -----------------------------------------------*/
void SerialDownload(mico_flash_t flash, uint32_t flashdestination, int32_t maxRecvSize);
void SerialUpload(void);

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  Analyse a command parameter
  * @param  commandBody: command string address
  * @param  para: The para we are looking for
  * @param  paraBody: A pointer to the buffer to receive the para body.
  * @param  paraBodyLength: The length, in bytes, of the buffer pointed to by the paraBody parameter.
  * @retval the actual length of the paraBody received, -1 means failed to find this paras 
  */
int findCommandPara(char *commandBody, char para, char *paraBody, int paraBodyLength)
{
  int i = 0;
  int k, j;
  int retval = -1;
  para = toupper(para);
  while(commandBody[i] != 0) {
    if(commandBody[i] == '-' && commandBody[i+1] == para){   /* para found!             */
      retval = 0;
      for (k = i+2; commandBody[k] == ' '; k++);      /* skip blanks                 */
      for(j = 0; commandBody[k] != ' ' && commandBody[k] != 0 && commandBody[k] != '-'; j++, k++){   /* para body found!             */
          paraBody[j] = commandBody[k];
          retval ++;
          if( retval == paraBodyLength)
            return retval;
        }
    }
    i++;
  }
  return retval;
}


/**
  * @brief  Download a file via serial port
  * @param  None
  * @retval None
  */
void SerialDownload(mico_flash_t flash,  uint32_t flashdestination, int32_t maxRecvSize)
{
  uint8_t Number[10] = "          ";
  int32_t Size = 0;

  SerialPutString("Waiting for the file to be sent ... (press 'a' to abort)\n\r");
  Size = Ymodem_Receive(&tab_1024[0], flash, flashdestination, maxRecvSize);
  if (Size > 0)
  {
    SerialPutString("\n\n\r Programming Completed Successfully!\n\r--------------------------------\r\n Name: ");
    SerialPutString(FileName);
    
    Int2Str(Number, Size);
    SerialPutString("\n\r Size: ");
    SerialPutString(Number);
    SerialPutString(" Bytes\r\n");
    SerialPutString("-------------------\n");
  }
  else if (Size == -1)
  {
    SerialPutString("\n\n\rThe image size is higher than the allowed space memory!\n\r");
  }
  else if (Size == -2)
  {
    SerialPutString("\n\n\rVerification failed!\n\r");
  }
  else if (Size == -3)
  {
    SerialPutString("\r\n\nAborted by user.\n\r");
  }
  else
  {
    SerialPutString("\n\rFailed to receive the file!\n\r");
  }
}

/**
  * @brief  Upload a file via serial port.
  * @param  None
  * @retval None
  */
// void SerialUpload(void)
// {
//   uint8_t status = 0 ; 

//   SerialPutString("\n\n\rSelect Receive File\n\r");

//   if (GetKey() == CRC16)
//   {
//     /* Transmit the flash image through ymodem protocol */
//     status = Ymodem_Transmit((uint8_t*)APPLICATION_START_ADDRESS, (const uint8_t*)"UploadedFlashImage.bin", USER_FLASH_SIZE);

//     if (status != 0) 
//     {
//       SerialPutString("\n\rError Occurred while Transmitting File\n\r");
//     }
//     else
//     {
//       SerialPutString("\n\rFile uploaded successfully \n\r");
//     }
//   }
// }

/**
  * @brief  Display the Main Menu on HyperTerminal
  * @param  None
  * @retval None
  */
void Main_Menu(void)
{
  char cmdbuf [200] = {0}, cmdname[15] = {0};                            /* command input buffer        */
  int i, j;                                       /* index for command buffer    */

  while (1)  {                                 /* loop forever                */
    printf ("\n\rMXCHIP> ");
    getline (&cmdbuf[0], sizeof (cmdbuf));     /* input command line          */

    for (i = 0; cmdbuf[i] == ' '; i++);        /* skip blanks on head         */
    for (; cmdbuf[i] != 0; i++)  {             /* convert to upper characters */
		  cmdbuf[i] = toupper(cmdbuf[i]);
    }

    for (i = 0; cmdbuf[i] == ' '; i++);        /* skip blanks on head         */
    for(j=0; cmdbuf[i] != ' '&&cmdbuf[i] != 0; i++,j++)  {         /* find command name       */
		  cmdname[j] = cmdbuf[i];
    }
    cmdname[j] = '\0';

    /***************** Command: Update the application  *************************/
    if(strcmp(cmdname, "FWUPDATE") == 0 || strcmp(cmdname, "1") == 0)	{
      if (findCommandPara(cmdbuf, 'a', NULL, 200) != -1){
       	printf ("\n\rErasing......\n\r");
        MicoFlashInitialize(MICO_FLASH_FOR_APPLICATION);
        MicoFlashErase(MICO_FLASH_FOR_APPLICATION, APPLICATION_START_ADDRESS, APPLICATION_END_ADDRESS);
        MicoFlashFinalize(MICO_FLASH_FOR_APPLICATION);
      }

      printf ("\n\rUpdating......\n\r");
      SerialDownload(MICO_FLASH_FOR_APPLICATION, APPLICATION_START_ADDRESS, USER_FLASH_SIZE);
      break;													   	
    }
    /***************** Command: Update the bootloader  *************************/
    else if(strcmp(cmdname, "BOOTUPDATE") == 0) {
      printf ("\n\rCaution: Prepare to update the bootloader, press ENTER to proceed...\n\r");
      if( GetKey () == 0x0D){
        printf ("\n\rUpdating......\n\r");
        //SerialDownload(BOOT_START_ADDRESS, BOOT_FLASH_SIZE);
        printf ("\n\rUpdate success!\n\r");
        MicoSystemReboot();
      }
      else
        printf ("\nUpdate cancelled.....\n");
      break;                             
    }
    /***************** Command: Update the RF driver  *************************/
    else if(strcmp(cmdname, "DRIVERUPDATE") == 0) {
      printf ("\n\rCaution: Prepare to update the 8782 driver, press ENTER to proceed...\n\r");
      if( GetKey () == 0x0D){
        printf ("\n\rUpdating......\n\r");
        SerialDownload(MICO_FLASH_FOR_DRIVER, DRIVER_START_ADDRESS, DRIVER_FLASH_SIZE);
        printf ("\n\rUpdate success! \n\r");
      }
      else
        printf ("\n\rUpdate cancelled.....\n\r");
      break;                             
    }
	/***************** Command: Update the Full flash  *************************/
    else if(strcmp(cmdname, "FLASHUPDATE") == 0) {
      printf ("\nCaution: Prepare to update the flash, press ENTER to proceed...\n");
      if( GetKey () == 0x0D){
        printf ("\n\rUpdating......\n\r");
        SerialDownload( MICO_INTERNAL_FLASH, INTERNAL_FLASH_START_ADDRESS, INTERNAL_FLASH_SIZE );
        printf ("\n\rUpdate success! \n\r");
      }
      else
        printf ("\n\rUpdate cancelled.....\n\r");
      break;                             
    }
    /***************** Command: Excute the application *************************/
    else if(strcmp(cmdname, "BOOT") == 0 || strcmp(cmdname, "3") == 0)	{
      printf ("\n\rBooting.......\n\r");
      startApplication();
    }

   /***************** Command: Reboot *************************/
    else if(strcmp(cmdname, "REBOOT") == 0 || strcmp(cmdname, "4") == 0)  {
      printf ("\n\rReBooting.......\n\r");
      MicoSystemReboot();
    break;                              
  }

	else if(strcmp(cmdname, "HELP") == 0 || strcmp(cmdname, "?") == 0)	{
    printf ("%s", menu);                         /* display command menu        */
		break;
	}

	else if(strcmp(cmdname, "") == 0 )	{                         
		break;
	}
	else{
	    printf (ERROR_STR, "UNKNOWN COMMAND");
		break;
	}
  }
}

/**
  * @}
  */

/*******************(C)COPYRIGHT 2011 STMicroelectronics *****END OF FILE******/
