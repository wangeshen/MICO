/**
******************************************************************************
* @file    MicoVirtualDevice.c
* @author  Eshen Wang
* @version V0.2.0
* @date    21-Nov-2014
* @brief   This file contains the implementations
*          of MICO virtual device. 
  operation
******************************************************************************
* @attention
*
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
* TIME. AS A RESULT, MXCHIP Inc. SHALL NOT BE HELD LIABLE FOR ANY
* DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
* FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
* CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*
* <h2><center>&copy; COPYRIGHT 2014 MXCHIP Inc.</center></h2>
******************************************************************************
*/ 

#include <stdio.h>

#include "MICODefine.h"
#include "MICONotificationCenter.h"

#include "MicoVirtualDevice.h"
#include "MVDDeviceInterfaces.h"
#include "MVDCloudInterfaces.h"
#include "EasyCloudUtils.h"


#define mvd_log(M, ...) custom_log("MVD", M, ##__VA_ARGS__)
#define mvd_log_trace() custom_log_trace("MVD")


#define DEFAULT_MVD_CLOUD_CONNECTED_MSG_2CLOUD     "{\"MVDCloud\":\"connected\"}"
#define DEFAULT_MVD_CLOUD_CONNECTED_MSG_2MCU       "[MVD]Cloud connected!\r\n"
#define DEFAULT_MVD_CLOUD_DISCONNECTED_MSG_2MCU    "[MVD]Cloud disconnected!\r\n"

static bool _is_wifi_station_on = false;

void mvdNotify_WifiStatusHandler(WiFiEvent event, mico_Context_t * const inContext)
{
  mvd_log_trace();
  (void)inContext;
  switch (event) {
  case NOTIFY_STATION_UP:
    _is_wifi_station_on = true;
    break;
  case NOTIFY_STATION_DOWN:
    _is_wifi_station_on = false;
    break;
  case NOTIFY_AP_UP:
    break;
  case NOTIFY_AP_DOWN:
    break;
  default:
    break;
  }
  return;
}


void MVDMainThread(void *arg)
{
  OSStatus err = kUnknownErr;
  mico_Context_t *inContext = (mico_Context_t *)arg;
  //micoMemInfo_t *memInfo = NULL;
  bool connected = false;
  
#ifdef DEVICE_AUTO_ACTIVATE_ENABLE
  MVDActivateRequestData_t devDefaultActivateData;
#endif
  
  mvd_log("MVD main thread start.");
  
  /* Regisist notifications */
  err = MICOAddNotification( mico_notify_WIFI_STATUS_CHANGED, (void *)mvdNotify_WifiStatusHandler );
  require_noerr( err, exit ); 
      
//  memInfo = mico_memory_info();
//  mvd_log("[MVD]system free mem=%d", memInfo->free_memory);
  
  while(1)
  {
    //memInfo = mico_memory_info();
    //mvd_log("[MVD]system free mem=%d", memInfo->free_memory);
    
    if(inContext->appStatus.virtualDevStatus.isCloudConnected){
      if (!connected){
        mvd_log("[MVD]cloud connected!");
        MVDDevInterfaceSend(DEFAULT_MVD_CLOUD_CONNECTED_MSG_2MCU, 
                                   strlen(DEFAULT_MVD_CLOUD_CONNECTED_MSG_2MCU));
        MVDCloudInterfaceSend(DEFAULT_MVD_CLOUD_CONNECTED_MSG_2CLOUD, 
                              strlen(DEFAULT_MVD_CLOUD_CONNECTED_MSG_2CLOUD));
        
        connected = true;
      }
    }
    else{
      if (connected){
        connected = false;
        mvd_log("[MVD]cloud disconnected!");
        MVDDevInterfaceSend(DEFAULT_MVD_CLOUD_DISCONNECTED_MSG_2MCU, 
                            strlen(DEFAULT_MVD_CLOUD_DISCONNECTED_MSG_2MCU));
      }
      
#ifdef DEVICE_AUTO_ACTIVATE_ENABLE
      if((true == _is_wifi_station_on) &&
         (false == inContext->flashContentInRam.appConfig.virtualDevConfig.isActivated)){
        // auto activate, using default login_id/dev_pass/user_token(MAC)
        mvd_log("auto activate device by MVD...");
        memset((void*)&devDefaultActivateData, 0, sizeof(devDefaultActivateData));
        strncpy(devDefaultActivateData.loginId,
                inContext->flashContentInRam.appConfig.virtualDevConfig.loginId,
                MAX_SIZE_LOGIN_ID);
        strncpy(devDefaultActivateData.devPasswd,
                inContext->flashContentInRam.appConfig.virtualDevConfig.devPasswd,
                MAX_SIZE_DEV_PASSWD);
        strncpy(devDefaultActivateData.user_token,
                inContext->micoStatus.mac,
                MAX_SIZE_USER_TOKEN);
        err = MVDCloudInterfaceDevActivate(inContext, devDefaultActivateData);
        if(kNoErr == err){
          mvd_log("device activate success!");
        }
        else{
          mvd_log("device activate failed, will retry in %ds...", 1);
        }
      }
#endif
    }
    
    mico_thread_sleep(1);
  }
  
exit:
  mvd_log("[MVD]ERROR: exit with err=%d",err);
  return;
}


/*******************************************************************************
 * virtual device interfaces init
 ******************************************************************************/

void MVDRestoreDefault(mico_Context_t* const context)
{
  //mvd_log("[MVD] appRestoreDefault_callback!");
  // clean all config buffer
  memset((void*)&(context->flashContentInRam.appConfig.virtualDevConfig), 
         0, sizeof(virtual_device_config_t));
  
  context->flashContentInRam.appConfig.virtualDevConfig.USART_BaudRate = 115200;
  
  context->flashContentInRam.appConfig.virtualDevConfig.isActivated = false;
  sprintf(context->flashContentInRam.appConfig.virtualDevConfig.deviceId, DEFAULT_DEVICE_ID);
  sprintf(context->flashContentInRam.appConfig.virtualDevConfig.masterDeviceKey, DEFAULT_DEVICE_KEY);
  sprintf(context->flashContentInRam.appConfig.virtualDevConfig.romVersion, DEFAULT_ROM_VERSION);
  
  sprintf(context->flashContentInRam.appConfig.virtualDevConfig.loginId, DEFAULT_LOGIN_ID);
  sprintf(context->flashContentInRam.appConfig.virtualDevConfig.devPasswd, DEFAULT_DEV_PASSWD);
  sprintf(context->flashContentInRam.appConfig.virtualDevConfig.userToken, DEFAULT_USER_TOKEN);
}

OSStatus MVDInit(mico_Context_t* const inContext)
{
  OSStatus err = kUnknownErr;
  
  //init MVD status
  inContext->appStatus.virtualDevStatus.isCloudConnected = false;
  inContext->appStatus.virtualDevStatus.RecvRomFileSize = 0;
  
  //init MCU connect interface
  err = MVDDevInterfaceInit(inContext);
  require_noerr_action(err, exit, 
                       mvd_log("ERROR: virtual device mcu interface init failed!") );
  
  //init cloud service interface
  err = MVDCloudInterfaceInit(inContext);
  require_noerr_action(err, exit, 
                       mvd_log("ERROR: virtual device cloud interface init failed!") );
  
  // start MVD monitor thread
  err = mico_rtos_create_thread(NULL, MICO_APPLICATION_PRIORITY, "MVD main", 
                                MVDMainThread, STACK_SIZE_MVD_MAIN_THREAD, 
                                inContext );
  
exit:
  return err;
}

/*******************************************************************************
 * MVD message exchange protocol
 ******************************************************************************/

// Cloud => MCU
OSStatus MVDCloudMsgProcess(mico_Context_t* context, 
                            const char* topic,
                            const unsigned int topicLen,
                            unsigned char *inBuf, unsigned int inBufLen)
{
  mvd_log_trace();
  OSStatus err = kUnknownErr;
  //mico_Context_t *inContext = context;
  //unsigned char* usartCmd = NULL;
  //unsigned int usartCmdLen = 0;
  
  err = MVDDevInterfaceSend(inBuf, inBufLen); // transfer raw data
/*
  // translate cloud json message to usart protocol format
  err = MVDMsgTransformCloud2Device(inBuf, inBufLen, &usartCmd, &usartCmdLen);
  require_noerr_action(err, exit, 
                       mvd_log("ERROR: message translate error! err=%d", err));

  // send data
  err = MVDDevInterfaceSend(usartCmd, usartCmdLen);
  if(NULL != usartCmd){
    free(usartCmd);
    usartCmd = NULL;
    usartCmdLen = 0;
  }
*/
  require_noerr_action( err, exit, mvd_log("ERROR: send to MCU error! err=%d", err) );
  return kNoErr;
  
exit:
  return err;
}

// MCU => Cloud
OSStatus MVDDeviceMsgProcess(mico_Context_t* const context, 
                             uint8_t *inBuf, unsigned int inBufLen)
{
  mvd_log_trace();
  OSStatus err = kUnknownErr;
  mico_Context_t *inContext = context;
  //unsigned char* cloudMsg = NULL;
  //unsigned int cloudMsgLen = 0;
  MVDGetFileRequestData_t devGetFileRequestData;
  json_object *new_obj = NULL;
  
  memset((void*)&devGetFileRequestData, 0, sizeof(devGetFileRequestData));
  
  if(0 == strncmp("Update", (const char*)inBuf, strlen("Update"))){
    new_obj = json_tokener_parse((const char*)(inBuf + strlen("Update")));
    require_action(new_obj, exit, err = kUnknownErr);
    
    mvd_log("Recv getfile request object=%s", json_object_to_json_string(new_obj));
    
    json_object_object_foreach(new_obj, key, val) {
      if(!strcmp(key, "bin")){
        strncpy(devGetFileRequestData.file_path, 
                json_object_get_string(val), MAX_SIZE_FILE_PATH);
      }
      else if(!strcmp(key, "bin_md5")){
        strncpy(devGetFileRequestData.file_checksum, 
                json_object_get_string(val), MAX_SIZE_FILE_MD5);
      }
      else if(!strcmp(key, "version")){
        strncpy(devGetFileRequestData.file_version, 
                json_object_get_string(val), MAX_SIZE_FW_VERSION);
      }
      else {
      }
    }
    json_object_put(new_obj);
    
    err = MVDCloudInterfaceGetFile(inContext, devGetFileRequestData);
    require_noerr_action(err, exit, 
                         mvd_log("ERROR: MVDCloudInterfaceGetFile error! err=%d", err));
  }
  else{
  
  err = MVDCloudInterfaceSend(inBuf, inBufLen);  // transfer raw data
/*
  // translate mcu usart message to json for cloud
  err = MVDMsgTransformDevice2Cloud(inBuf, inBufLen, &cloudMsg, &cloudMsgLen);
  require_noerr_action(err, exit, 
                       mvd_log("ERROR: message translate error! err=%d", err));
  //mvd_log("send[%d]=%.*s", cloudMsgLen, cloudMsgLen, cloudMsg);
  
  // send data
  err = MVDCloudInterfaceSend(cloudMsg, cloudMsgLen);
  
  if(NULL != cloudMsg){
    free(cloudMsg);
    cloudMsg = NULL;
    cloudMsgLen = 0;
  }
*/
  require_noerr_action( err, exit, mvd_log("ERROR: send to cloud error! err=%d", err) );
  return kNoErr;
  }
  
exit:
  return err;
}


/*******************************************************************************
 * MVD cloud control interfaces
 ******************************************************************************/

//activate
OSStatus MVDActivate(mico_Context_t* const context, 
                     MVDActivateRequestData_t activateData)
{
  OSStatus err = kUnknownErr;
  
  err = MVDCloudInterfaceDevActivate(context, activateData);
  require_noerr_action(err, exit, 
                       mvd_log("ERROR: device activate failed! err=%d", err) );
  return kNoErr;
  
exit:
  return err;
}
  
//authorize
OSStatus MVDAuthorize(mico_Context_t* const context,
                      MVDAuthorizeRequestData_t authorizeData)
{
  OSStatus err = kUnknownErr;
  mico_Context_t *inContext = context;

  err = MVDCloudInterfaceDevAuthorize(inContext, authorizeData);
  require_noerr_action(err, exit, 
                       mvd_log("ERROR: device authorize failed! err=%d", err) );
  return kNoErr;
  
exit:
  return err;
}

//OTA
OSStatus MVDFirmwareUpdate(mico_Context_t* const context,
                           MVDOTARequestData_t OTAData)
{
  OSStatus err = kUnknownErr;
  mico_Context_t *inContext = context;
  
  err = MVDCloudInterfaceDevFirmwareUpdate(inContext, OTAData);
  require_noerr_action(err, exit, 
                       mvd_log("ERROR: Firmware Update error! err=%d", err) );
  return kNoErr;
  
exit:
  return err;
}

//reset device info on cloud
OSStatus MVDResetCloudDevInfo(mico_Context_t* const context,
                              MVDResetRequestData_t devResetData)
{
  OSStatus err = kUnknownErr;
  mico_Context_t *inContext = context;
  
  err = MVDCloudInterfaceResetCloudDevInfo(inContext, devResetData);
  require_noerr_action(err, exit, 
                       mvd_log("ERROR: reset device info on cloud error! err=%d", err) );
  return kNoErr;
  
exit:
  return err;
}

//get state of the MVD( e.g. isActivate/isConnected)
OSStatus MVDGetState(mico_Context_t* const context,
                     MVDGetStateRequestData_t getStateRequestData,
                     void* outDevState)
{
  //OSStatus err = kUnknownErr;
  mico_Context_t *inContext = context;
  json_object* report = (json_object*)outDevState;
  
  if((NULL == context) || (NULL == outDevState)){
    return kParamErr;
  }
  
  // login_id/dev_passwd ok ?
  if((0 != strncmp(context->flashContentInRam.appConfig.virtualDevConfig.loginId, 
                   getStateRequestData.loginId, 
                   strlen(context->flashContentInRam.appConfig.virtualDevConfig.loginId))) ||
     (0 != strncmp(context->flashContentInRam.appConfig.virtualDevConfig.devPasswd, 
                   getStateRequestData.devPasswd, 
                   strlen(context->flashContentInRam.appConfig.virtualDevConfig.devPasswd))))
  {
    mvd_log("ERROR: MVDGetState: loginId/devPasswd mismatch!");
    return kMismatchErr;
  }
  
  json_object_object_add(report, "isActivated",
                         json_object_new_boolean(inContext->flashContentInRam.appConfig.virtualDevConfig.isActivated)); 
  json_object_object_add(report, "isConnected",
                         json_object_new_boolean(inContext->appStatus.virtualDevStatus.isCloudConnected));
  json_object_object_add(report, "version",
                         json_object_new_string(inContext->flashContentInRam.appConfig.virtualDevConfig.romVersion));
  
  return kNoErr;
}

