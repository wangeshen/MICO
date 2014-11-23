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
//#include "MICONotificationCenter.h"

#include "MicoVirtualDevice.h"
#include "MVDDeviceInterfaces.h"
#include "MVDCloudInterfaces.h"


#define mvd_log(M, ...) custom_log("MVD", M, ##__VA_ARGS__)
#define mvd_log_trace() custom_log_trace("MVD")


static OSStatus deviceInterfaceInit(mico_Context_t * const inContext);
static OSStatus cloudInterfaceInit(mico_Context_t * const inContext);


/*******************************************************************************
 * virtual device interface init
 ******************************************************************************/

OSStatus MVDInit(void* const context)
{
  OSStatus err = kUnknownErr;
  mico_Context_t* const inContext = (mico_Context_t *)context;
  
  //init MCU connect interface
  err = deviceInterfaceInit(inContext);
  require_noerr_action( err, exit, mvd_log("ERROR: virtual device mcu interface init failed!") );
  
  //init cloud service interface
  err = cloudInterfaceInit(inContext);
  require_noerr_action( err, exit, mvd_log("ERROR: virtual device cloud interface init failed!") );
  
exit:
  return err;
}

//MCU connect
OSStatus deviceInterfaceInit(mico_Context_t * const inContext)
{
  OSStatus err = kUnknownErr;
  
  err = MVDDevInterfaceInit(inContext);
  require_noerr_action( err, exit, mvd_log("ERROR: init device interface failed.") );
  return kNoErr;
  
exit:
  return err;
}

//cloud service connect
OSStatus cloudInterfaceInit(mico_Context_t * const inContext)
{
  OSStatus err = kUnknownErr;

  err = MVDCloudInterfaceInit(inContext);
  require_noerr_action( err, exit, mvd_log("ERROR: init cloud interface failed.") );
  return kNoErr;
  
exit:
  return err;
}

/*******************************************************************************
 * data tranfer protocol
 ******************************************************************************/

// Cloud => MCU
OSStatus MVDCloudMsgProcess(void* context, unsigned char *inBuf, unsigned int inBufLen)
{
  mvd_log_trace();
  OSStatus err = kUnknownErr;
  //mico_Context_t *inContext = (mico_Context_t *)context;

  err = MVDDevInterfaceSend(inBuf, inBufLen);
  require_noerr_action( err, exit, mvd_log("ERROR: send to USART error! err=%d", err) );
  return kNoErr;
  
exit:
  return err;
}

// MCU => Cloud
OSStatus MVDDeviceMsgProcess(void* const context, uint8_t *inBuf, unsigned int inBufLen)
{
  mvd_log_trace();
  OSStatus err = kUnknownErr;
  //mico_Context_t *inContext = (mico_Context_t *)context;

  err = MVDCloudInterfaceSend(inBuf, inBufLen);
  require_noerr_action( err, exit, mvd_log("ERROR: send to cloud error! err=%d", err) );
  return kNoErr;
  
exit:
  return err;
}

//OTA
OSStatus MVDFirmwareUpdate(void* context)
{
  OSStatus err = kUnknownErr;
  mico_Context_t *inContext = (mico_Context_t *)context;
  
  err = MVDCloudInterfaceDevFirmwareUpdate(inContext);
  require_noerr_action( err, exit, mvd_log("ERROR: Firmware Update error! err=%d", err) );
  return kNoErr;
  
exit:
  return err;
}

//activate
OSStatus MVDActivate(void* context)
{
  OSStatus err = kUnknownErr;
  mico_Context_t *inContext = (mico_Context_t *)context;
  
  err = MVDCloudInterfaceDevActivate(inContext);
  require_noerr_action( err, exit, mvd_log("ERROR: device activate failed! err=%d", err) );
  return kNoErr;
  
exit:
  return err;
}
  
//authorize
OSStatus MVDAuthorize(void* context)
{
  OSStatus err = kUnknownErr;
  mico_Context_t *inContext = (mico_Context_t *)context;

  err = MVDCloudInterfaceDevAuthorize(inContext);
  require_noerr_action( err, exit, mvd_log("ERROR: device authorize failed! err=%d", err) );
  return kNoErr;
  
exit:
  return err;
}
