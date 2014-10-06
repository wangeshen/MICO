/** @file update.c
* @brief This file provides functions to update the target flash contents according the 
* update tags written by OTA functions
*
* <!-- Copyright 2012 by MXCHIP Corporation. All rights reserved.        *80*-->
*/


#include "Update_for_OTA.h"
#include "platform.h"
#include "MicoPlatform.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "debug.h"

typedef int Log_Status;					
#define Log_NotExist				1
#define Log_NeedUpdate				2
#define Log_UpdateTagNotExist		3
#define Log_contentTypeNotExist		4
#define Log_dataLengthOverFlow		5
#define Log_StartAddressERROR		6
#define Log_UnkonwnERROR			7

#ifndef MIN
#define MIN(a,b) (((a) < (b))?(a):(b))
#endif

uint32_t destStartAddress, destEndAddress;
mico_flash_t destFlashType;


int updateLogCheck(boot_table_t *updateLog);
extern void Update_error_callback(UPDATE_ERROR_TypeDef error_code);
#define update_log(M, ...) custom_log("UPDATE", M, ##__VA_ARGS__)
#define update_log_trace() custom_log_trace("UPDATE")

void update(void)
{
  boot_table_t updateLog;
  uint32_t i, j, size;
  uint8_t *data = NULL;
  uint8_t *newData = NULL;
  uint8_t *paraSaveInRam;
  uint32_t updateStartAddress;
  uint32_t destStartAddress_tmp;
  uint32_t paraStartAddress;
  
  MicoFlashInitialize( (mico_flash_t)MICO_FLASH_FOR_UPDATE );
  data = malloc(4096);
  memset(data, 0xFF, 4096);
  
  updateStartAddress = UPDATE_START_ADDRESS;
  
  paraStartAddress = PARA_START_ADDRESS;  
  //updateLog = (boot_table_t *)LogAddress;
  MicoFlashRead(MICO_FLASH_FOR_PARA, &paraStartAddress, (uint8_t *)&updateLog, sizeof(boot_table_t));

  /*Not a correct record*/
  if(updateLogCheck(&updateLog) != Log_NeedUpdate){
    size = UPDATE_FLASH_SIZE/4096;
    for(i = 0; i <= size; i++){
      if( i==size )
        MicoFlashRead(MICO_FLASH_FOR_UPDATE, &updateStartAddress, data , UPDATE_FLASH_SIZE%1024);
      else
        MicoFlashRead(MICO_FLASH_FOR_UPDATE, &updateStartAddress, data , 1024);
      
      for(j=0; j<4096; j++){
        if(data[j] != 0xFF){
          update_log("Update data need to be erased");
          MicoFlashInitialize( MICO_FLASH_FOR_UPDATE );
          if(MicoFlashErase( MICO_FLASH_FOR_UPDATE, UPDATE_START_ADDRESS, UPDATE_END_ADDRESS )){
            Update_error_callback( ERASE_UPDATE_DATA_FAILED );
          };
          MicoFlashFinalize( MICO_FLASH_FOR_UPDATE );
          break;
        }
      }
    }
    free(data);
    return;
  }
  
  destStartAddress_tmp = destStartAddress;
  updateStartAddress = UPDATE_START_ADDRESS;
  
  MicoFlashInitialize( destFlashType );
  MicoFlashErase( destFlashType, destStartAddress, destEndAddress );
  size = UPDATE_FLASH_SIZE/4096;
  newData = malloc(4096);
  for(i = 0; i <= size; i++){
    if( i==size ){
      MicoFlashRead(MICO_FLASH_FOR_UPDATE, &updateStartAddress, data , UPDATE_FLASH_SIZE%4096);
      MicoFlashWrite(destFlashType, &destStartAddress_tmp, data, UPDATE_FLASH_SIZE%4096);
      destStartAddress_tmp -= UPDATE_FLASH_SIZE%4096;
      MicoFlashRead(destFlashType, &destStartAddress_tmp, newData , UPDATE_FLASH_SIZE%4096);
      if(memcmp(data, newData, UPDATE_FLASH_SIZE%4096))
        Update_error_callback(PROGRAM_TARGET_FAILED);
    }
    else{
      MicoFlashRead(MICO_FLASH_FOR_UPDATE, &updateStartAddress, data , 4096);
      MicoFlashWrite(destFlashType, &destStartAddress_tmp, data, 4096);
      destStartAddress_tmp -= 4096;
      MicoFlashRead(destFlashType, &destStartAddress_tmp, newData , 4096);
      if(memcmp(data, newData, 4096))
        Update_error_callback(PROGRAM_TARGET_FAILED);      
    }
  }  
  
  updateStartAddress = UPDATE_START_ADDRESS;
  
  paraSaveInRam = (uint8_t *)malloc(PARA_FLASH_SIZE);
  MicoFlashRead(MICO_FLASH_FOR_PARA, &paraStartAddress, paraSaveInRam , PARA_FLASH_SIZE);
  memset(paraSaveInRam, 0xff, sizeof(boot_table_t));
  if(MicoFlashErase(MICO_FLASH_FOR_PARA, PARA_START_ADDRESS, PARA_END_ADDRESS)){
    Update_error_callback(ERASE_UPDATE_TAG_FAILED);
  };
  
  paraStartAddress = PARA_START_ADDRESS;
  if(MicoFlashWrite(MICO_FLASH_FOR_PARA, &paraStartAddress, paraSaveInRam, PARA_FLASH_SIZE)){
    Update_error_callback(ERASE_UPDATE_TAG_FAILED);
  };
  free (paraSaveInRam);
  
  if(MicoFlashErase(MICO_FLASH_FOR_PARA, UPDATE_START_ADDRESS, UPDATE_END_ADDRESS)){
    Update_error_callback(ERASE_UPDATE_DATA_FAILED);
  };
  
  MicoFlashFinalize(MICO_FLASH_FOR_UPDATE);
  MicoFlashFinalize(destFlashType);
}



Log_Status updateLogCheck(boot_table_t *updateLog)
{
  int i;
  
  for(i=0; i<sizeof(boot_table_t); i++){
    if(*((uint8_t *)updateLog + i) != 0xff)
      break;
  }
  if(i == sizeof(boot_table_t))
    return Log_NotExist;
  
  if(updateLog->upgrade_type == 'U'){
    if(updateLog->start_address != UPDATE_START_ADDRESS)
      return Log_StartAddressERROR;
    if(updateLog->type == 'B'){
      destStartAddress = BOOT_START_ADDRESS;
      destEndAddress = BOOT_END_ADDRESS;
      destFlashType = MICO_FLASH_FOR_BOOT;
      if(updateLog->length > BOOT_FLASH_SIZE)
        return Log_dataLengthOverFlow;
    }
    else if(updateLog->type == 'A'){
      destStartAddress = APPLICATION_START_ADDRESS;
      destEndAddress = APPLICATION_END_ADDRESS;
      destFlashType = MICO_FLASH_FOR_APPLICATION;
      if(updateLog->length > USER_FLASH_SIZE)
        return Log_dataLengthOverFlow;
    }
    else if(updateLog->type == 'D'){
      destStartAddress = DRIVER_START_ADDRESS;
      destEndAddress = DRIVER_END_ADDRESS;
      destFlashType = MICO_FLASH_FOR_DRIVER;
      if(updateLog->length > DRIVER_FLASH_SIZE)
        return Log_dataLengthOverFlow;
    }
    else 
      return Log_contentTypeNotExist;
    
    return Log_NeedUpdate;
  }
  else
    return Log_UpdateTagNotExist;
}



