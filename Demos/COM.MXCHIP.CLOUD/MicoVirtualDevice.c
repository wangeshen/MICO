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

#include "MicoVirtualDevice.h"
#include "MVDDeviceInterfaces.h"
#include "MVDCloudInterfaces.h"

#include "MICODefine.h"
//#include "MICONotificationCenter.h"


#define mvd_log(M, ...) custom_log("MVD", M, ##__VA_ARGS__)
#define mvd_log_trace() custom_log_trace("MVD")


/*******************************************************************************
 * virtual device interfaces init
 ******************************************************************************/

void MVDRestoreDefault(mico_Context_t* const context)
{
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
  
  //init status
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

  err = MVDDevInterfaceSend(inBuf, inBufLen);
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

  err = MVDCloudInterfaceSend(inBuf, inBufLen);
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

