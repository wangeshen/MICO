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
#define DEFAULT_MVD_CLOUD_CONNECTED_MSG_2MCU       "[MVD]Cloud status: connected\r\n"
#define DEFAULT_MVD_CLOUD_DISCONNECTED_MSG_2MCU    "[MVD]Cloud status: disconnected\r\n"

extern uint64_t cloud_test_data_cnt;

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

  bool connected = false;
  
#ifdef DEVICE_AUTO_ACTIVATE_ENABLE
  MVDActivateRequestData_t devDefaultActivateData;
#endif
  
  mvd_log("MVD main thread start.");
  
  /* Regisist notifications */
  err = MICOAddNotification( mico_notify_WIFI_STATUS_CHANGED, (void *)mvdNotify_WifiStatusHandler );
  require_noerr( err, exit ); 
  
  while(1)
  {
    if(inContext->appStatus.virtualDevStatus.isCloudConnected){
      if (!connected){
        mvd_log("[MVD]Cloud status: connected");
        MVDDevInterfaceSend(DEFAULT_MVD_CLOUD_CONNECTED_MSG_2MCU, 
                                   strlen(DEFAULT_MVD_CLOUD_CONNECTED_MSG_2MCU));
        MVDCloudInterfaceSendtoChannel(PUBLISH_TOPIC_CHANNEL_STATUS,
                                     DEFAULT_MVD_CLOUD_CONNECTED_MSG_2CLOUD, 
                                     strlen(DEFAULT_MVD_CLOUD_CONNECTED_MSG_2CLOUD));
        
        connected = true;
        // exit when cloud connected
        //break;
      }
    }
    else{
      if (connected){
        connected = false; //recovery value;
        mvd_log("[MVD]Cloud status: disconnected");
        MVDDevInterfaceSend(DEFAULT_MVD_CLOUD_DISCONNECTED_MSG_2MCU, 
                            strlen(DEFAULT_MVD_CLOUD_DISCONNECTED_MSG_2MCU));
      }
      
#ifdef DEVICE_AUTO_ACTIVATE_ENABLE
      if((true == _is_wifi_station_on) &&
         (false == inContext->flashContentInRam.appConfig.virtualDevConfig.isActivated)){
        // auto activate, using default login_id/dev_pass/user_token
        mvd_log("[MVD]Device activate by MVD ...");
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
          mvd_log("[MVD]Device activate by MVD [OK]");
        }
        else{
          mvd_log("[MVD]Device activate by MVD [FAILED], will retry in %ds...", 1);
        }
      }
#endif
    }
    
    mico_thread_sleep(1);
  }
  
exit:
  mvd_log("[MVD]EXIT: exit code=%d",err);
  mico_rtos_delete_thread(NULL);
  return;
}


/*******************************************************************************
 * virtual device interfaces init
 ******************************************************************************/

void MVDRestoreDefault(mico_Context_t* const context)
{
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
  //sprintf(context->flashContentInRam.appConfig.virtualDevConfig.userToken, DEFAULT_USER_TOKEN);
  sprintf(context->flashContentInRam.appConfig.virtualDevConfig.userToken, context->micoStatus.mac);
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
 * MVD get state
 ******************************************************************************/
// cloud connect state
bool MVDCloudIsConnect(mico_Context_t* const context)
{
  if(NULL == context){
    return false;
  }
  return context->appStatus.virtualDevStatus.isCloudConnected;
}

// device activate state
bool MVDIsActivated(mico_Context_t* const context)
{
  if(NULL == context){
    return false;
  }
  return context->flashContentInRam.appConfig.virtualDevConfig.isActivated;
}

char* MVDGetDeviceID(mico_Context_t* const context)
{
  if(NULL == context){
    return NULL;
  }
  return (char*)(context->flashContentInRam.appConfig.virtualDevConfig.deviceId);
}

/*******************************************************************************
 * MVD message send interface
 ******************************************************************************/

// MVD => MCU
OSStatus MVDSendMsg2Device(mico_Context_t* const context, 
                           unsigned char *inBuf, unsigned int inBufLen)
{
  mvd_log_trace();
  OSStatus err = kUnknownErr;
  
  err = MVDDevInterfaceSend(inBuf, inBufLen);  // transfer raw data
  require_noerr_action( err, exit, mvd_log("ERROR: send to cloud error! err=%d", err) );
  return kNoErr;
  
exit:
  return err;
}

// MVD => Cloud
OSStatus MVDSendMsg2Cloud(mico_Context_t* const context, const char* topic, 
                       unsigned char *inBuf, unsigned int inBufLen)
{
  mvd_log_trace();
  OSStatus err = kUnknownErr;
  
  err = MVDCloudInterfaceSendtoChannel(topic, inBuf, inBufLen);  // transfer raw data
  require_noerr_action( err, exit, mvd_log("ERROR: send to cloud error! err=%d", err) );
  return kNoErr;
  
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

// get file from server, return file size in
//   "context->appStatus.virtualDevStatus.RecvRomFileSize" if succeed.
OSStatus MVDDownloadFile(mico_Context_t* const context,
                         MVDDownloadFileRequestData_t devGetFileRequestData)
{
  OSStatus err = kUnknownErr;
  mico_Context_t *inContext = context;
  
  err = MVDCloudInterfaceGetFile(inContext, devGetFileRequestData);
  require_noerr_action(err, exit, 
                       mvd_log("ERROR: MVDCloudInterfaceGetFile error! err=%d", err) );
  
exit:
  return err;
}

/*******************************************************************************
 * MVD message exchange protocol
 ******************************************************************************/

// handle message from Cloud
OSStatus MVDCloudMsgProcess(mico_Context_t* context, 
                            const char* topic, const unsigned int topicLen,
                            unsigned char *inBuf, unsigned int inBufLen)
{
  mvd_log_trace();
  OSStatus err = kUnknownErr;
  
  cloud_test_data_cnt += inBufLen;
  mvd_log("[MVD]recv_cnt = [%d/%lld]", inBufLen, cloud_test_data_cnt);
  //err = MVDCloudInterfaceSend(inBuf, inBufLen); // response to cloud
  //err = kNoErr;
  
  err = MVDDevInterfaceSend(inBuf, inBufLen); // transfer raw data to MCU
  require_noerr_action( err, exit, mvd_log("ERROR: send to MCU error! err=%d", err) );
  
exit:
  return err;
}

// handle message from MCU
OSStatus MVDDeviceMsgProcess(mico_Context_t* const context, 
                             uint8_t *inBuf, unsigned int inBufLen)
{
  mvd_log_trace();
  OSStatus err = kUnknownErr;
  MVDOTARequestData_t OTAData;
  MVDDownloadFileRequestData_t devGetFileRequestData;
  json_object *new_obj = NULL;
  uint8_t recv_param_check = 0;  // must recv all three params
  
  memset((void*)(&OTAData), 0, sizeof(OTAData));
  memset((void*)&devGetFileRequestData, 0, sizeof(devGetFileRequestData));
  
  if(0 == strncmp("Get", (const char*)inBuf, strlen("Get"))){
    new_obj = json_tokener_parse((const char*)(inBuf + strlen("Get")));
    require_action(new_obj, exit, err = kUnknownErr);
    
    mvd_log("Recv getfile request object=%s", json_object_to_json_string(new_obj));
    
    json_object_object_foreach(new_obj, key, val) {
      if(!strcmp(key, "bin")){
        strncpy(devGetFileRequestData.file_path, 
                json_object_get_string(val), MAX_SIZE_FILE_PATH);
        recv_param_check += 1;
      }
      else if(!strcmp(key, "bin_md5")){
        strncpy(devGetFileRequestData.file_checksum, 
                json_object_get_string(val), MAX_SIZE_FILE_MD5);
        recv_param_check += 1;
      }
      else if(!strcmp(key, "version")){
        strncpy(devGetFileRequestData.file_version, 
                json_object_get_string(val), MAX_SIZE_FW_VERSION);
        recv_param_check += 1;
      }
      else {
      }
    }
    json_object_put(new_obj);
    require_action(3 == recv_param_check, exit, err = kParamErr);
    
    // get file request
    err = MVDCloudInterfaceGetFile(context, devGetFileRequestData);
    require_noerr_action(err, exit, 
                         mvd_log("ERROR: MVDCloudInterfaceGetFile error! err=%d", err));
    // notify
    err = MVDDevInterfaceSend("Get file OK!\r\n", strlen("Get file OK!\r\n"));
    require_noerr_action( err, exit, mvd_log("ERROR: send to MCU error! err=%d", err) );
  }
  else if(0 == strncmp("Update", (const char*)inBuf, inBufLen)){
    err = MVDFirmwareUpdate(context, OTAData);
    require_noerr_action( err, exit, mvd_log("ERROR: MVDFirmwareUpdate error! err=%d", err) );
    // notify
//    err = MVDDevInterfaceSend("Update OK!", strlen("Update OK!"));
//    require_noerr_action( err, exit, mvd_log("ERROR: send to MCU error! err=%d", err) );
  }
  else{
    err = MVDCloudInterfaceSend(inBuf, inBufLen);  // transfer raw data to Cloud
    require_noerr_action( err, exit, mvd_log("ERROR: send to cloud error! err=%d", err) );
  }
  
exit:
  return err;
}

