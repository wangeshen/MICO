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

#define SizePerRW 4096   /* Bootloader need 2xSizePerRW RAM heap size to operate, 
                            but it can boost the setup. */

uint8_t data[SizePerRW];
uint8_t newData[SizePerRW];
uint8_t paraSaveInRam[PARA_FLASH_SIZE];

uint32_t destStartAddress, destEndAddress;
mico_flash_t destFlashType;


int updateLogCheck(boot_table_t *updateLog);
extern void Update_error_callback(UPDATE_ERROR_TypeDef error_code);
#define update_log(M, ...) custom_log("UPDATE", M, ##__VA_ARGS__)
#define update_log_trace() custom_log_trace("UPDATE")

OSStatus update(void)
{
  boot_table_t updateLog;
  uint32_t i, j, size;
  uint32_t updateStartAddress;
  uint32_t destStartAddress_tmp;
  uint32_t paraStartAddress;
  OSStatus err = kNoErr;
  
  MicoFlashInitialize( (mico_flash_t)MICO_FLASH_FOR_UPDATE );
  memset(data, 0xFF, SizePerRW);
  memset(newData, 0xFF, SizePerRW);
  memset(paraSaveInRam, 0xFF, PARA_FLASH_SIZE);
  
  updateStartAddress = UPDATE_START_ADDRESS;
  
  paraStartAddress = PARA_START_ADDRESS;  
  err = MicoFlashRead(MICO_FLASH_FOR_PARA, &paraStartAddress, (uint8_t *)&updateLog, sizeof(boot_table_t));
  require_noerr(err, exit);

  /*Not a correct record*/
  if(updateLogCheck(&updateLog) != Log_NeedUpdate){
    size = UPDATE_FLASH_SIZE/SizePerRW;
    for(i = 0; i <= size; i++){
      if( i==size ){
        err = MicoFlashRead(MICO_FLASH_FOR_UPDATE, &updateStartAddress, data , UPDATE_FLASH_SIZE%SizePerRW);
        require_noerr(err, exit);
      }
      else{
        err = MicoFlashRead(MICO_FLASH_FOR_UPDATE, &updateStartAddress, data , SizePerRW);
        require_noerr(err, exit);
      }
      
      for(j=0; j<SizePerRW; j++){
        if(data[j] != 0xFF){
          update_log("Update data need to be erased");
          err = MicoFlashInitialize( MICO_FLASH_FOR_UPDATE );
          require_noerr(err, exit);
          err = MicoFlashErase( MICO_FLASH_FOR_UPDATE, UPDATE_START_ADDRESS, UPDATE_END_ADDRESS );
          require_noerr(err, exit);
          err = MicoFlashFinalize( MICO_FLASH_FOR_UPDATE );
          require_noerr(err, exit);
          break;
        }
      }
    }
    goto exit;
  }
  
  update_log("Write OTA data to destination, type:%d, from %d to %d, length 0x%x", destFlashType, destStartAddress, destEndAddress, updateLog.length);
  
  destStartAddress_tmp = destStartAddress;
  updateStartAddress = UPDATE_START_ADDRESS;
  
  err = MicoFlashInitialize( destFlashType );
  require_noerr(err, exit);
  err = MicoFlashErase( destFlashType, destStartAddress, destEndAddress );
  require_noerr(err, exit);
  size = (updateLog.length)/SizePerRW;
  for(i = 0; i <= size; i++){
    if( i==size && (updateLog.length)%SizePerRW){
      err = MicoFlashRead(MICO_FLASH_FOR_UPDATE, &updateStartAddress, data , (updateLog.length)%SizePerRW);
      require_noerr(err, exit);
      err = MicoFlashInitialize( destFlashType );
      require_noerr(err, exit);
      err = MicoFlashWrite(destFlashType, &destStartAddress_tmp, data, (updateLog.length)%SizePerRW);
      require_noerr(err, exit);
      destStartAddress_tmp -= (updateLog.length)%SizePerRW;
      err = MicoFlashRead(destFlashType, &destStartAddress_tmp, newData , (updateLog.length)%SizePerRW);
      require_noerr(err, exit);
      err = memcmp(data, newData, (updateLog.length)%SizePerRW);
      require_noerr_action(err, exit, err = kWriteErr);
    }
    else{
      err = MicoFlashRead(MICO_FLASH_FOR_UPDATE, &updateStartAddress, data , SizePerRW);
      require_noerr(err, exit);
      err = MicoFlashInitialize( destFlashType );
      require_noerr(err, exit);
      err = MicoFlashWrite(destFlashType, &destStartAddress_tmp, data, SizePerRW);
      require_noerr(err, exit);
      destStartAddress_tmp -= SizePerRW;
      err = MicoFlashRead(destFlashType, &destStartAddress_tmp, newData , SizePerRW);
      require_noerr(err, exit);
      err = memcmp(data, newData, SizePerRW);
      require_noerr_action(err, exit, err = kWriteErr); 
    }
  } 
  update_log("Update start to clear data...");
    
  paraStartAddress = PARA_START_ADDRESS;
  err = MicoFlashRead(MICO_FLASH_FOR_PARA, &paraStartAddress, paraSaveInRam, PARA_FLASH_SIZE);
  require_noerr(err, exit);
  memset(paraSaveInRam, 0xff, sizeof(boot_table_t));
  
  err = MicoFlashErase(MICO_FLASH_FOR_PARA, PARA_START_ADDRESS, PARA_END_ADDRESS);
  require_noerr(err, exit);

  paraStartAddress = PARA_START_ADDRESS;
  err = MicoFlashWrite(MICO_FLASH_FOR_PARA, &paraStartAddress, paraSaveInRam, PARA_FLASH_SIZE);
  require_noerr(err, exit);
  
  err = MicoFlashErase(MICO_FLASH_FOR_UPDATE, UPDATE_START_ADDRESS, UPDATE_END_ADDRESS);
  require_noerr(err, exit);
  update_log("Update success");
  
exit:
  if(err != kNoErr) update_log("Update exit with err = %d", err);
  MicoFlashFinalize(MICO_FLASH_FOR_UPDATE);
  MicoFlashFinalize(destFlashType);
  return err;
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
      if(updateLog->length > APPLICATION_FLASH_SIZE)
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



