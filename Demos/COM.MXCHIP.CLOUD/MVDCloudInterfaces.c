/**
******************************************************************************
* @file    MVDCloudInterfaces.c 
* @author  Eshen Wang
* @version V0.2.0
* @date    21-Nov-2014
* @brief   This file contains the implementations of cloud service interfaces 
*          for MICO virtual device.
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

#include "MICODefine.h"

#include "MicoVirtualDevice.h"
#include "MVDCloudInterfaces.h"   
#include "EasyCloudService.h"


#define cloud_if_log(M, ...) custom_log("MVD_CLOUD_IF", M, ##__VA_ARGS__)
#define cloud_if_log_trace() custom_log_trace("MVD_CLOUD_IF")


easycloud_service_context_t *pEasyCloudContext = NULL;


/*******************************************************************************
 * cloud service callbacks
 ******************************************************************************/

//cloud message recived handler
void cloudMsgArrivedHandler(void* context, unsigned char *msg, unsigned int msgLen)
{
  //note: get data just for length=len is valid, because Msg is just a buf pionter.
  cloud_if_log("Cloud => DEVICE: [%d]=%.*s", msgLen, msgLen, msg);
  
  MVDCloudMsgProcess(context, msg, msgLen);
}

//cloud service status changed handler
void cloudServiceStatusChangedHandler(void* context, easycloud_service_status_t serviceStateInfo)
{
  easycloud_service_status_t cloudStatusInfo = serviceStateInfo;
  mico_Context_t *inContext = (mico_Context_t*)context;
  
  if (CLOUD_SERVICE_STATUS_CONNECTED == serviceStateInfo.state){
    cloud_if_log("cloud service connected!");
    inContext->appStatus.virtualDevStatus.isCloudConnected = true;
    mico_rtos_lock_mutex(&inContext->flashContentInRam_mutex);
    inContext->flashContentInRam.appConfig.virtualDevConfig.isActivated = cloudStatusInfo.isActivated;
    sprintf(inContext->flashContentInRam.appConfig.virtualDevConfig.deviceId, 
            cloudStatusInfo.deviceId, MAX_SIZE_DEVICE_ID);
    sprintf(inContext->flashContentInRam.appConfig.virtualDevConfig.masterDeviceKey, 
            cloudStatusInfo.masterDeviceKey, MAX_SIZE_DEVICE_KEY);
    MICOUpdateConfiguration(context);
    mico_rtos_unlock_mutex(&inContext->flashContentInRam_mutex);
  }
  else{
    cloud_if_log("cloud service disconnected!");
    inContext->appStatus.virtualDevStatus.isCloudConnected = false;
  }
}

OSStatus MVDCloudInterfaceInit(mico_Context_t* const inContext)
{
  OSStatus err = kUnknownErr;
  int cloudServiceLibVersion = 0;
  
  inContext->appStatus.virtualDevStatus.isCloudConnected = false;
  
  //cloud service config info
  easycloud_service_config_t cloud_service_config = {
    inContext->flashContentInRam.appConfig.virtualDevConfig.cloudServerDomain,
    inContext->flashContentInRam.appConfig.virtualDevConfig.cloudServerPort,
    inContext->micoStatus.mac,
    inContext->flashContentInRam.appConfig.virtualDevConfig.productId,
    inContext->flashContentInRam.appConfig.virtualDevConfig.productKey,
    inContext->flashContentInRam.appConfig.virtualDevConfig.loginId,
    inContext->flashContentInRam.appConfig.virtualDevConfig.devPasswd,
    inContext->flashContentInRam.appConfig.virtualDevConfig.userToken,
    inContext->flashContentInRam.appConfig.virtualDevConfig.mqttServerDomain,
    inContext->flashContentInRam.appConfig.virtualDevConfig.mqttServerPort,
    inContext->flashContentInRam.appConfig.virtualDevConfig.mqttkeepAliveInterval,
    cloudMsgArrivedHandler,
    cloudServiceStatusChangedHandler,
    (void*)inContext
  };
  
  err = EasyCloudServiceVersion(&cloudServiceLibVersion);
  require_noerr_action( err, exit, cloud_if_log("ERROR: Get EasyCloud service version failed.") );
  cloud_if_log("EasyCloud Service library version: %d.%d.%d", 
         (cloudServiceLibVersion >> 16) & 0xFF, 
         (cloudServiceLibVersion >>8) & 0xFF, 
         cloudServiceLibVersion & 0xFF );
  
  //start cloud service
  err = EasyCloudServiceInit(pEasyCloudContext, cloud_service_config);
  require_noerr_action( err, exit, cloud_if_log("ERROR: EasyCloud service init failed.") );
  err = EasyCloudServiceStart(pEasyCloudContext);
  require_noerr_action( err, exit, cloud_if_log("ERROR: EasyCloud service start failed.") );
  return kNoErr;
  
exit:
  if(pEasyCloudContext){
    free(pEasyCloudContext);
    pEasyCloudContext = NULL;
  }
  return err;
}

OSStatus MVDCloudInterfaceSend(unsigned char *inBuf, unsigned int inBufLen)
{
  cloud_if_log_trace();
  OSStatus err = kUnknownErr;

  cloud_if_log("DEVICE => Cloud:[%d]=%.*s", inBufLen, inBufLen, inBuf);
  err = EasyCloudUpload(pEasyCloudContext, inBuf, inBufLen);
  require_noerr_action( err, exit, cloud_if_log("ERROR: EasyCloud upload failed! err=%d", err) );
  return kNoErr;
  
exit:
  return err;
}

OSStatus MVDCloudInterfaceDevActivate(mico_Context_t* const inContext)
{
  cloud_if_log_trace();
  OSStatus err = kUnknownErr;

  cloud_if_log("Device activate...");
  err = EasyCloudActivate(pEasyCloudContext);
  require_noerr_action( err, exit, cloud_if_log("ERROR: EasyCloud activate failed! err=%d", err) );
  return kNoErr;
  
exit:
  return err;
}

OSStatus MVDCloudInterfaceDevAuthorize(mico_Context_t* const inContext)
{
  cloud_if_log_trace();
  OSStatus err = kUnknownErr;

  cloud_if_log("Device authorize...");
  err = EasyCloudAuthorize(pEasyCloudContext);
  require_noerr_action( err, exit, cloud_if_log("ERROR: EasyCloud authorize failed! err=%d", err) );
  return kNoErr;
  
exit:
  return err;
}

OSStatus MVDCloudInterfaceDevFirmwareUpdate(mico_Context_t* const inContext)
{
  cloud_if_log_trace();
  OSStatus err = kUnknownErr;

  cloud_if_log("Update firmware...");
  err = EasyCloudFirmwareUpdate(pEasyCloudContext);
  require_noerr_action( err, exit, cloud_if_log("ERROR: Firmware update failed! err=%d", err) );
  return kNoErr;
  
exit:
  return err;
}
