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

#include "MVDMsgProtocol.h"


#define mvd_log(M, ...) custom_log("MVD", M, ##__VA_ARGS__)
#define mvd_log_trace() custom_log_trace("MVD")


// default device info
#define DEFAULT_DEVICE_ID                "none"
#define DEFAULT_DEVICE_KEY               "none"

#define DEFAULT_MVD_CLOUD_CONNECTED      "{\"MVDCloud\":\"connected\"}"

static bool _wifi_on = false;

void mvdNotify_WifiStatusHandler(WiFiEvent event, mico_Context_t * const inContext)
{
  mvd_log_trace();
  (void)inContext;
  switch (event) {
  case NOTIFY_STATION_UP:
    mvd_log("Station up");
    _wifi_on = true;
    break;
  case NOTIFY_STATION_DOWN:
    mvd_log("Station down");
    _wifi_on = false;
    break;
  default:
    break;
  }
  return;
}

void MVDMainThread(void *arg)
{
  mico_Context_t *inContext = (mico_Context_t *)arg;
  bool connected = false;
  
#ifdef DEVICE_AUTO_ACTIVATE_ENABLE
  OSStatus err = kUnknownErr;
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
        mvd_log("[MVD]cloud service connected!");
        MVDCloudInterfaceSend(DEFAULT_MVD_CLOUD_CONNECTED, 
                              strlen(DEFAULT_MVD_CLOUD_CONNECTED));
        
        connected = true;
        // set LED to blue means cloud connected, data: on/off,H,S,B
        LedControlMsgHandler("1,240,100,100", strlen("1,240,100,100"));
      }
    }
    else{
      if (connected){
        connected = false;
        // white means cloud disconnect.
        LedControlMsgHandler("1,0,0,100", strlen("1,0,0,100"));
        mvd_log("[MVD]cloud service disconnected!");
      }
      
#ifdef DEVICE_AUTO_ACTIVATE_ENABLE
      if((true == _wifi_on) &&
         (false == inContext->flashContentInRam.appConfig.virtualDevConfig.isActivated)){
          // auto activate, using default login_id/dev_pass/user_token
          mvd_log("auto activate device by MVD...");
          memset((void*)&devDefaultActivateData, 0, sizeof(devDefaultActivateData));
          strncpy(devDefaultActivateData.loginId,
                  inContext->flashContentInRam.appConfig.virtualDevConfig.loginId,
                  MAX_SIZE_LOGIN_ID);
          strncpy(devDefaultActivateData.devPasswd,
                  inContext->flashContentInRam.appConfig.virtualDevConfig.devPasswd,
                  MAX_SIZE_DEV_PASSWD);
          strncpy(devDefaultActivateData.user_token,
                  inContext->flashContentInRam.appConfig.virtualDevConfig.userToken,
                  MAX_SIZE_USER_TOKEN);
          err = MVDCloudInterfaceDevActivate(inContext, devDefaultActivateData);
          if(kNoErr == err){
            mvd_log("device activate success!");
          }
          else{
            mvd_log("device activate failed, will retry in 1s...");
          }
        }
      }
#endif
    
    mico_thread_sleep(1);
  }
  
exit:
  mvd_log("[MVD] exit with err=%d", err);
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
  sprintf(context->flashContentInRam.appConfig.virtualDevConfig.userToken, DEFAULT_USER_TOKEN);
}

OSStatus MVDInit(mico_Context_t* const inContext)
{
  OSStatus err = kUnknownErr;
  
  //init MVD status
  inContext->appStatus.virtualDevStatus.isCloudConnected = false;
  inContext->appStatus.virtualDevStatus.RecvRomFileSize = 0;
  
  MVDRestoreDefault(inContext);
  
  //init MCU connect interface
//  err = MVDDevInterfaceInit(inContext);
//  require_noerr_action(err, exit, 
//                       mvd_log("ERROR: virtual device mcu interface init failed!") );
  
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
  unsigned char* usartCmd = NULL;
  unsigned int usartCmdLen = 0;
  char* responseTopic = NULL;
  
  // translate cloud json message to usart protocol format
  err = MVDMsgTransformCloud2Device(inBuf, inBufLen, &usartCmd, &usartCmdLen);
  require_noerr_action(err, exit, 
                       mvd_log("ERROR: message translate error! err=%d", err));
  
  // add response to cloud, replace topic 'device_id/in/xxx' to 'device_id/out/xxx'
  responseTopic = str_replace(responseTopic, topic, topicLen, "/in", "/out");
  
  if((NULL != usartCmd) && (usartCmdLen > 0) && (NULL != responseTopic)){
    err = MVDCloudInterfaceSendto(responseTopic, usartCmd, usartCmdLen);
  }
  if(NULL != usartCmd){
    free(usartCmd);
    usartCmd = NULL;
    usartCmdLen = 0;
  }
  if(NULL != responseTopic){
    free(responseTopic);
    responseTopic = NULL;
  }
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
  //mico_Context_t *inContext = context;
  unsigned char* cloudMsg = NULL;
  unsigned int cloudMsgLen = 0;
  
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

