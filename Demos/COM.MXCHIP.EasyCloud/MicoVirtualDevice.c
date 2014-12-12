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

#include "MicoVirtualDevice.h"
#include "MVDDeviceInterfaces.h"
#include "MVDCloudInterfaces.h"

#include "MVDMsgProtocol.h"


#define mvd_log(M, ...) custom_log("MVD", M, ##__VA_ARGS__)
#define mvd_log_trace() custom_log_trace("MVD")


// default device info
#define DEFAULT_DEVICE_ID                "none"
#define DEFAULT_DEVICE_KEY               "none"

#define DEFAULT_MVD_CLOUD_CONNECTED      "{\"MVDCloud\":\"connected\"}"

void MVDMainThread(void *arg)
{
  mico_Context_t *inContext = (mico_Context_t *)arg;
  micoMemInfo_t *memInfo = NULL;
  bool connected = false;
#ifdef DEVICE_AUTO_ACTIVATE_ENABLE
  OSStatus err = kUnknownErr;
  int wait_time = DEVICE_AUTO_ACTIVATE_TIME/1;  //auto activate after 5s
  MVDActivateRequestData_t devDefaultActivateData;
#endif
  
  mvd_log("MVD main thread start.");
      
  memInfo = mico_memory_info();
  mvd_log("[MVD]system free mem=%d", memInfo->free_memory);
  
  while(1)
  {
    if(inContext->appStatus.virtualDevStatus.isCloudConnected){
      if (!connected){
        mvd_log("[MVD]cloud service connected!");
        MVDCloudInterfaceSend(DEFAULT_MVD_CLOUD_CONNECTED, 
                              strlen(DEFAULT_MVD_CLOUD_CONNECTED));
        
        connected = true;
      }
    }
    else{
      if (connected){
        connected = false; //recovery value;
#ifdef DEVICE_AUTO_ACTIVATE_ENABLE
        wait_time = DEVICE_AUTO_ACTIVATE_TIME/1;  //recovery value;
#endif
        mvd_log("[MVD]cloud service disconnected!");
        //MVDCloudInterfaceSend("device disconnected!", strlen("device disconnected!"));
      }
      
#ifdef DEVICE_AUTO_ACTIVATE_ENABLE
      if(false == inContext->flashContentInRam.appConfig.virtualDevConfig.isActivated){
        if (wait_time > 0){
          mvd_log("cloud service disconnected, will activate device after %ds...", 
                  wait_time*1);
          wait_time--;
        }
        else if(0 == wait_time){
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
            wait_time = -1;  //activate ok, never do activate again.
          }
          else{
            wait_time = 1;  //reactivate after 1 seconds
            mvd_log("device activate failed, will retry in %ds...", wait_time*1);
          }
        }
        else{
        }
      }
#endif
    }
    
    mico_thread_sleep(1);
  }
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
                            unsigned char *inBuf, unsigned int inBufLen)
{
  mvd_log_trace();
  OSStatus err = kUnknownErr;
  //mico_Context_t *inContext = context;
  unsigned char* usartCmd = NULL;
  unsigned int usartCmdLen = 0;
  
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
OSStatus MVDFirmwareUpdate(mico_Context_t* const context)
{
  OSStatus err = kUnknownErr;
  mico_Context_t *inContext = context;
  
  err = MVDCloudInterfaceDevFirmwareUpdate(inContext);
  require_noerr_action(err, exit, 
                       mvd_log("ERROR: Firmware Update error! err=%d", err) );
  return kNoErr;
  
exit:
  return err;
}

//reset device info on cloud
OSStatus MVDResetCloudDevInfo(mico_Context_t* const context)
{
  OSStatus err = kUnknownErr;
  mico_Context_t *inContext = context;
  
  err = MVDCloudInterfaceResetCloudDevInfo(inContext);
  require_noerr_action(err, exit, 
                       mvd_log("ERROR: reset device info on cloud error! err=%d", err) );
  return kNoErr;
  
exit:
  return err;
}

