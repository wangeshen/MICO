/**
******************************************************************************
* @file    user_params_storage.c 
* @author  Eshen Wang
* @version V1.0.0
* @date    21-Apr-2015
* @brief   This file provide functions to read and write user settings on 
*          nonvolatile memory.
******************************************************************************
*
*  The MIT License
*  Copyright (c) 2014 MXCHIP Inc.
*
*  Permission is hereby granted, free of charge, to any person obtaining a copy 
*  of this software and associated documentation files (the "Software"), to deal
*  in the Software without restriction, including without limitation the rights 
*  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*  copies of the Software, and to permit persons to whom the Software is furnished
*  to do so, subject to the following conditions:
*
*  The above copyright notice and this permission notice shall be included in
*  all copies or substantial portions of the Software.
*
*  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
*  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
*  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
*  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
*  WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR 
*  IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
******************************************************************************
*/

#include "MICODefine.h"
#include "MICO.h"
#include "MicoPlatform.h"
#include "user_params_storage.h"

#define user_params_storage_log(M, ...) custom_log("USER_PARAMS_STORAGE", M, ##__VA_ARGS__)
#define user_params_storage_log_trace() custom_log_trace("USER_PARAMS_STORAGE")

OSStatus userParams_RestoreDefault(mico_Context_t *mico_context, user_context_t *user_context)
{ 
  OSStatus err = kNoErr;
  uint32_t paraStartAddress, paraEndAddress;
 
  paraStartAddress = EX_PARA_START_ADDRESS;
  paraEndAddress = EX_PARA_END_ADDRESS;
  
  if( (NULL == mico_context) || (NULL == user_context) ){
    return kParamErr;
  }
  
  if(NULL == user_context->config_mutex){
    mico_rtos_init_mutex(&user_context->config_mutex);
  }
  mico_rtos_lock_mutex(&user_context->config_mutex);
  
  user_params_storage_log("userParams_RestoreDefault.");
  
  /* resotre user params */
  // user config
  memset(&(user_context->config), 0x0, sizeof(user_context->config));
  
  sprintf(user_context->config.dev_name, DEFAULT_DEVICE_NAME);
  user_context->config.dev_name_len = strlen(DEFAULT_DEVICE_NAME);
  sprintf(user_context->config.dev_manufacturer, DEFAULT_MANUFACTURER);
  user_context->config.dev_manufacturer_len = strlen(DEFAULT_MANUFACTURER);
  
  user_context->config.rgb_led_sw = false;
  user_context->config.rgb_led_hues = 0;
  user_context->config.rgb_led_saturation = 0;
  user_context->config.rgb_led_brightness = 0;
  
  user_context->config.dc_motor_switch = 0;
  
//  user_context->config.light_sensor_event = true;
//  user_context->config.infrared_reflective_event = true;
//  user_context->config.uart_rx_event = true;
  
  // user status
  user_context->status.user_config_need_update = false;
  user_context->status.light_sensor_data = 0;
  user_context->status.infrared_reflective_data = 0;
  user_context->status.temperature = 0;
  user_context->status.humidity = 0;
  user_context->status.uart_rx_data_len = 0;
  
  user_params_storage_log("user_context->config.dev_name = [%s]", user_context->config.dev_name);
  
  /* write flash */
  mico_rtos_lock_mutex(&mico_context->flashContentInRam_mutex);
  err = MicoFlashInitialize(MICO_FLASH_FOR_EX_PARA);
  require_noerr(err, exit);
  err = MicoFlashErase(MICO_FLASH_FOR_EX_PARA, paraStartAddress, paraEndAddress);
  require_noerr(err, exit);
  err = MicoFlashWrite(MICO_FLASH_FOR_EX_PARA, &paraStartAddress, (uint8_t *)&(user_context->config), sizeof(user_config_t));
  require_noerr(err, exit);
  err = MicoFlashFinalize(MICO_FLASH_FOR_EX_PARA);
  require_noerr(err, exit);
  mico_rtos_unlock_mutex(&mico_context->flashContentInRam_mutex);
  
  mico_rtos_unlock_mutex(&user_context->config_mutex);

exit:
  return err;
}

OSStatus userParams_Read(mico_Context_t *mico_context, user_context_t *user_context)
{
  uint32_t configInFlash;
  OSStatus err = kNoErr;
  
  configInFlash = EX_PARA_START_ADDRESS;
  if( (NULL == mico_context) || (NULL == user_context) ){
    return kParamErr;
  }
  
  if(NULL == user_context->config_mutex){
    mico_rtos_init_mutex(&user_context->config_mutex);
  }
  mico_rtos_lock_mutex(&user_context->config_mutex);
  
  /* read flash */
  mico_rtos_lock_mutex(&mico_context->flashContentInRam_mutex);
  err = MicoFlashInitialize(MICO_FLASH_FOR_EX_PARA);
  require_noerr(err, exit);
  err = MicoFlashRead(MICO_FLASH_FOR_EX_PARA, &configInFlash, (uint8_t *)&(user_context->config), sizeof(user_config_t));
  mico_rtos_unlock_mutex(&mico_context->flashContentInRam_mutex);
  
  mico_rtos_unlock_mutex(&user_context->config_mutex);
  
  user_params_storage_log("user_context->config.dev_name = [%s]", user_context->config.dev_name);
  
exit: 
  return err;
}

OSStatus userParams_Update(mico_Context_t *mico_context, user_context_t *user_context)
{
  OSStatus err = kNoErr;
  uint32_t paraStartAddress, paraEndAddress;
 
  paraStartAddress = EX_PARA_START_ADDRESS;
  paraEndAddress = EX_PARA_END_ADDRESS;
  
  if( (NULL == mico_context) || (NULL == user_context) ){
    return kParamErr;
  }
  
  if(NULL == user_context->config_mutex){
    mico_rtos_init_mutex(&user_context->config_mutex);
  }
  mico_rtos_lock_mutex(&user_context->config_mutex);
  
  /* write flash */
  mico_rtos_lock_mutex(&mico_context->flashContentInRam_mutex);
  err = MicoFlashInitialize(MICO_FLASH_FOR_EX_PARA);
  require_noerr(err, exit);
  err = MicoFlashErase(MICO_FLASH_FOR_EX_PARA, paraStartAddress, paraEndAddress);
  require_noerr(err, exit);
  err = MicoFlashWrite(MICO_FLASH_FOR_EX_PARA, &paraStartAddress, (uint8_t *)&(user_context->config), sizeof(user_config_t));
  require_noerr(err, exit);
  err = MicoFlashFinalize(MICO_FLASH_FOR_EX_PARA);
  require_noerr(err, exit);
  mico_rtos_unlock_mutex(&mico_context->flashContentInRam_mutex);
  
  mico_rtos_unlock_mutex(&user_context->config_mutex);

exit:
  return err;
}
