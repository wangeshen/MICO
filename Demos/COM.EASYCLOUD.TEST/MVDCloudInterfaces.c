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

#include "MVDCloudInterfaces.h"   
#include "EasyCloudService.h"
#include "MicoVirtualDevice.h"


#define cloud_if_log(M, ...) custom_log("MVD_CLOUD_IF", M, ##__VA_ARGS__)
#define cloud_if_log_trace() custom_log_trace("MVD_CLOUD_IF")


static easycloud_service_context_t easyCloudContext;


/*******************************************************************************
 * cloud service callbacks
 ******************************************************************************/

//cloud message recived handler
void cloudMsgArrivedHandler(void* context, 
                            const char* topic, const unsigned int topicLen,
                            unsigned char *msg, unsigned int msgLen)
{
  mico_Context_t *inContext = (mico_Context_t*)context;
  
  //note: get data just for length=len is valid, because Msg is just a buf pionter.
  cloud_if_log("Cloud[%.*s] => MVD: [%d]=%.*s", topicLen, topic, msgLen, msgLen, msg);
  
  MVDCloudMsgProcess(inContext, topic, topicLen, msg, msgLen);
}

//cloud service status changed handler
void cloudServiceStatusChangedHandler(void* context, 
                                      easycloud_service_status_t serviceStateInfo)
{
  mico_Context_t *inContext = (mico_Context_t*)context;

  if (EASYCLOUD_CONNECTED == serviceStateInfo.state){
    cloud_if_log("cloud service connected!");
    inContext->appStatus.virtualDevStatus.isCloudConnected = true;
  }
  else{
    cloud_if_log("cloud service disconnected!");
    inContext->appStatus.virtualDevStatus.isCloudConnected = false;
  }
}

OSStatus MVDCloudInterfacePrintVersion(void)
{
  //OSStatus err = kUnknownErr;
  int cloudServiceLibVersion = 0;
  cloud_if_log("MVDCloudInterfacePrintVersion");
  
  cloudServiceLibVersion = EasyCloudServiceVersion(&easyCloudContext);
  cloud_if_log("EasyCloud library version: v%d.%d.%d", 
               (cloudServiceLibVersion & 0x00FF0000) >> 16,
               (cloudServiceLibVersion & 0x0000FF00) >> 8,
               (cloudServiceLibVersion & 0x000000FF));
  
  return kNoErr;
}

OSStatus MVDCloudInterfaceInit(mico_Context_t* const inContext)
{
  OSStatus err = kUnknownErr;
  cloud_if_log("MVDCloudInterfaceInit");
  
  // set cloud service config
  strncpy(easyCloudContext.service_config_info.bssid, 
          inContext->micoStatus.mac, MAX_SIZE_BSSID);
  strncpy(easyCloudContext.service_config_info.productId, 
          (char*)DEFAULT_PRODUCT_ID, strlen((char*)DEFAULT_PRODUCT_ID));
  strncpy(easyCloudContext.service_config_info.productKey, 
          (char*)DEFAULT_PRODUCT_KEY, strlen((char*)DEFAULT_PRODUCT_KEY));
  easyCloudContext.service_config_info.msgRecvhandler = cloudMsgArrivedHandler;
  easyCloudContext.service_config_info.statusNotify = cloudServiceStatusChangedHandler;
  easyCloudContext.service_config_info.context = (void*)inContext;
  
  // set cloud status
  memset((void*)&(easyCloudContext.service_status), '\0',
         sizeof(easyCloudContext.service_status));
  easyCloudContext.service_status.isActivated = \
         inContext->flashContentInRam.appConfig.virtualDevConfig.isActivated;
  strncpy(easyCloudContext.service_status.deviceId, 
          inContext->flashContentInRam.appConfig.virtualDevConfig.deviceId, 
          MAX_SIZE_DEVICE_ID);
  strncpy(easyCloudContext.service_status.masterDeviceKey, 
          inContext->flashContentInRam.appConfig.virtualDevConfig.masterDeviceKey, 
          MAX_SIZE_DEVICE_KEY);
  
  err = EasyCloudServiceInit(&easyCloudContext);
  require_noerr_action( err, exit, 
                       cloud_if_log("ERROR: EasyCloudServiceInit err = %d.", err) );
  return kNoErr;
  
exit:
  cloud_if_log("ERROR: MVDCloudInterfaceInit err = %d.", err);
  return err;
}

OSStatus MVDCloudInterfaceStart(mico_Context_t* const inContext)
{
  OSStatus err = kUnknownErr;
  
  cloud_if_log("MVDCloudInterfaceStart");
  err = EasyCloudServiceStart(&easyCloudContext);
  require_noerr_action( err, exit, 
                       cloud_if_log("ERROR: EasyCloudServiceStart err=%d.", err));
  return kNoErr;
  
exit:
  return err;
}

easycloud_service_state_t MVDCloudInterfaceGetState(void)
{
  easycloud_service_state_t service_running_state = EASYCLOUD_STOPPED;
  
  cloud_if_log("MVDCloudInterfaceGetState");
  service_running_state = EasyCloudServiceState(&easyCloudContext);
  return service_running_state;
}

OSStatus MVDCloudInterfaceSend(unsigned char *inBuf, unsigned int inBufLen)
{
  cloud_if_log_trace();
  OSStatus err = kUnknownErr;

  //cloud_if_log("MVDCloudInterfaceSend: MVD => Cloud[publish]:[%d]=%.*s", 
  //             inBufLen, inBufLen, inBuf);
  
  err = EasyCloudPublish(&easyCloudContext, inBuf, inBufLen);
  require_noerr_action( err, exit, 
                       cloud_if_log("ERROR: MVDCloudInterfaceSend err=%d", err) );
  return kNoErr;
  
exit:
  return err;
}

OSStatus MVDCloudInterfaceSendto(const char* topic, 
                                 unsigned char *inBuf, unsigned int inBufLen)
{
  cloud_if_log_trace();
  OSStatus err = kUnknownErr;

  //cloud_if_log("MVDCloudInterfaceSendto: MVD => Cloud[%s]:[%d]=%.*s", 
  //             topic, inBufLen, inBufLen, inBuf);
  err = EasyCloudPublishto(&easyCloudContext, topic, inBuf, inBufLen);
  require_noerr_action( err, exit, 
                       cloud_if_log("ERROR: MVDCloudInterfaceSendto err=%d", err) );
  return kNoErr;
  
exit:
  return err;
}

OSStatus MVDCloudInterfaceSendtoChannel(const char* channel, 
                                        unsigned char *inBuf, unsigned int inBufLen)
{
  cloud_if_log_trace();
  OSStatus err = kUnknownErr;

  //cloud_if_log("MVDCloudInterfaceSendtoChannel: MVD => Cloud[%s]:[%d]=%.*s", 
  //             channel, inBufLen, inBufLen, inBuf);
  if(NULL == channel){
    err = EasyCloudPublish(&easyCloudContext, inBuf, inBufLen);
  }
  else{
    err = EasyCloudPublishtoChannel(&easyCloudContext, channel, inBuf, inBufLen);
  }
  require_noerr_action( err, exit, 
                       cloud_if_log("ERROR: MVDCloudInterfaceSendtoChannel err=%d", err) );
  return kNoErr;
  
exit:
  return err;
}

OSStatus MVDCloudInterfaceDevActivate(mico_Context_t* const inContext,
                                      MVDActivateRequestData_t devActivateRequestData)
{
  cloud_if_log_trace();
  OSStatus err = kUnknownErr;
  //easycloud_service_state_t cloudServiceState = EASYCLOUD_STOPPED;
  
  cloud_if_log("MVDCloudInterfaceDevActivate");
   
  // check cloud status
//  cloudServiceState = EasyCloudServiceState(&easyCloudContext);
//  if (EASYCLOUD_STOPPED == cloudServiceState){
//    return kStateErr;
//  }
  
  // set activate params
  strncpy(easyCloudContext.service_config_info.loginId, 
          devActivateRequestData.loginId, MAX_SIZE_LOGIN_ID);
  strncpy(easyCloudContext.service_config_info.devPasswd, 
          devActivateRequestData.devPasswd, MAX_SIZE_DEV_PASSWD);
  strncpy(easyCloudContext.service_config_info.userToken, 
          devActivateRequestData.user_token, MAX_SIZE_USER_TOKEN);
    
  // activate request
  err = EasyCloudActivate(&easyCloudContext);
  require_noerr_action(err, exit, 
                       cloud_if_log("ERROR: EasyCloudActivate, err=%d", err));
  
  // write back device_id/key to MVD
  memset(inContext->flashContentInRam.appConfig.virtualDevConfig.deviceId,
         0, MAX_SIZE_DEVICE_ID);
  memset(inContext->flashContentInRam.appConfig.virtualDevConfig.masterDeviceKey,
         0, MAX_SIZE_DEVICE_KEY);
  strncpy(inContext->flashContentInRam.appConfig.virtualDevConfig.deviceId,
          easyCloudContext.service_status.deviceId, 
          strlen(easyCloudContext.service_status.deviceId));
  strncpy(inContext->flashContentInRam.appConfig.virtualDevConfig.masterDeviceKey,
          easyCloudContext.service_status.masterDeviceKey, 
          strlen(easyCloudContext.service_status.masterDeviceKey));
  inContext->flashContentInRam.appConfig.virtualDevConfig.isActivated = true;
  
  return kNoErr;
  
exit:
  cloud_if_log("ERROR: MVDCloudInterfaceDevActivate, err=%d", err);
  return err;
}

OSStatus MVDCloudInterfaceDevAuthorize(mico_Context_t* const inContext,
                                       MVDAuthorizeRequestData_t devAuthorizeReqData)
{
  cloud_if_log_trace();
  OSStatus err = kUnknownErr;
  //easycloud_service_state_t cloudServiceState = EASYCLOUD_STOPPED;
  
  cloud_if_log("MVDCloudInterfaceDevAuthorize");

//  cloudServiceState = EasyCloudServiceState(&easyCloudContext);
//  if (EASYCLOUD_STOPPED == cloudServiceState){
//    return kStateErr;
//  }
  
  // set authorize params
  strncpy(easyCloudContext.service_config_info.loginId, 
          devAuthorizeReqData.loginId, MAX_SIZE_LOGIN_ID);
  strncpy(easyCloudContext.service_config_info.devPasswd, 
          devAuthorizeReqData.devPasswd, MAX_SIZE_DEV_PASSWD);
  strncpy(easyCloudContext.service_config_info.userToken, 
          devAuthorizeReqData.user_token, MAX_SIZE_USER_TOKEN);
  
  err = EasyCloudAuthorize(&easyCloudContext);
  require_noerr_action( err, exit, 
                       cloud_if_log("ERROR: EasyCloudAuthorize err=%d", err) );
  return kNoErr;
  
exit:
  cloud_if_log("ERROR: MVDCloudInterfaceDevAuthorize err=%d", err);
  return err;
}

OSStatus MVDCloudInterfaceDevFirmwareUpdate(mico_Context_t* const inContext,
                                            MVDOTARequestData_t devOTARequestData)
{
  cloud_if_log_trace();
  OSStatus err = kUnknownErr;

  cloud_if_log("Update firmware...");
  
  //get latest rom version, file_path, md5
  cloud_if_log("get latest rom version...");
  err = EasyCloudGetLatestRomVersion(&easyCloudContext);
  require_noerr_action( err, exit, 
                       cloud_if_log("ERROR: EasyCloudGetLatestRomVersion failed! err=%d", err) );
  
  //FW version compare
  cloud_if_log("currnt_version=%s", 
               inContext->flashContentInRam.appConfig.virtualDevConfig.romVersion);
  cloud_if_log("latestRomVersion=%s", easyCloudContext.service_status.latestRomVersion);
  cloud_if_log("bin_file=%s", easyCloudContext.service_status.bin_file);
  cloud_if_log("bin_md5=%s", easyCloudContext.service_status.bin_md5);
  
#ifdef MVD_FW_UPDAETE_VERSION_CHECK  
  cloud_if_log("fw version check...");
  if(0 == strncmp(inContext->flashContentInRam.appConfig.virtualDevConfig.romVersion,
                  easyCloudContext.service_status.latestRomVersion, 
                  strlen(inContext->flashContentInRam.appConfig.virtualDevConfig.romVersion))){
     cloud_if_log("the current firmware version[%s] is up to date!", 
                  inContext->flashContentInRam.appConfig.virtualDevConfig.romVersion);
     inContext->appStatus.virtualDevStatus.RecvRomFileSize = 0;
     return kNoErr;
  }
#endif
  cloud_if_log("new firmware[%s] found on server, update...",
               easyCloudContext.service_status.latestRomVersion);
  
  //get rom data
  err = EasyCloudGetRomData(&easyCloudContext);
  require_noerr_action( err, exit, 
                       cloud_if_log("ERROR: EasyCloudGetRomData failed! err=%d", err) );
  
  //update rom version in flash
  mico_rtos_lock_mutex(&inContext->flashContentInRam_mutex);
  memset(inContext->flashContentInRam.appConfig.virtualDevConfig.romVersion,
         0, MAX_SIZE_FW_VERSION);
  strncpy(inContext->flashContentInRam.appConfig.virtualDevConfig.romVersion,
          easyCloudContext.service_status.latestRomVersion, 
          strlen(easyCloudContext.service_status.latestRomVersion));
  inContext->appStatus.virtualDevStatus.RecvRomFileSize = \
          easyCloudContext.service_status.bin_file_size;

  // set bootloader to update App firmware
  memset(&inContext->flashContentInRam.bootTable, 0, sizeof(boot_table_t));
  inContext->flashContentInRam.bootTable.length = easyCloudContext.service_status.bin_file_size;;
  inContext->flashContentInRam.bootTable.start_address = UPDATE_START_ADDRESS;
  inContext->flashContentInRam.bootTable.type = 'A';
  inContext->flashContentInRam.bootTable.upgrade_type = 'U';
  if(inContext->flashContentInRam.micoSystemConfig.configured != allConfigured)
    inContext->flashContentInRam.micoSystemConfig.easyLinkByPass = EASYLINK_SOFT_AP_BYPASS;
  MICOUpdateConfiguration(inContext);
  mico_rtos_unlock_mutex(&inContext->flashContentInRam_mutex);
  
  // system reboot
  cloud_if_log("Update done! System will reboot...");
  inContext->micoStatus.sys_state = eState_Software_Reset;
  if(inContext->micoStatus.sys_state_change_sem != NULL )
    mico_rtos_set_semaphore(&inContext->micoStatus.sys_state_change_sem);
  mico_thread_sleep(MICO_WAIT_FOREVER);
  
  return kNoErr;
  
exit:
  return err;
}

OSStatus MVDCloudInterfaceResetCloudDevInfo(mico_Context_t* const inContext,
                                            MVDResetRequestData_t devResetRequestData)
{
  OSStatus err = kUnknownErr;
  
  cloud_if_log("MVDCloudInterfaceResetCloudDevInfo");
  err = EasyCloudDeviceReset(&easyCloudContext);
  require_noerr_action( err, exit, 
                       cloud_if_log("ERROR: EasyCloudDeviceReset err=%d", err) );
  
  //mico_rtos_lock_mutex(&inContext->flashContentInRam_mutex);
  inContext->flashContentInRam.appConfig.virtualDevConfig.isActivated = false;  // need to reActivate
  sprintf(inContext->flashContentInRam.appConfig.virtualDevConfig.deviceId, DEFAULT_DEVICE_ID);
  sprintf(inContext->flashContentInRam.appConfig.virtualDevConfig.masterDeviceKey, DEFAULT_DEVICE_KEY);
  sprintf(inContext->flashContentInRam.appConfig.virtualDevConfig.loginId, DEFAULT_LOGIN_ID);
  sprintf(inContext->flashContentInRam.appConfig.virtualDevConfig.devPasswd, DEFAULT_DEV_PASSWD);
  //sprintf(inContext->flashContentInRam.appConfig.virtualDevConfig.userToken, DEFAULT_USER_TOKEN);
  sprintf(inContext->flashContentInRam.appConfig.virtualDevConfig.userToken, inContext->micoStatus.mac);
  //inContext->appStatus.virtualDevStatus.isCloudConnected = false;
  //MICOUpdateConfiguration(inContext);
  //mico_rtos_unlock_mutex(&inContext->flashContentInRam_mutex);
  
  return kNoErr;
  
exit:
  return err;
}
        
// get a file from server spicified by user, return file size in
//   "inContext->appStatus.virtualDevStatus.RecvRomFileSize" if succeed.
OSStatus MVDCloudInterfaceGetFile(mico_Context_t* const inContext,
                                  MVDDownloadFileRequestData_t devGetFileRequestData)
{
  cloud_if_log_trace();
  OSStatus err = kUnknownErr;

  cloud_if_log("Get file from server...\r\nfile=%s\r\nchecsum=%s\r\nversion=%s\r\n",
               devGetFileRequestData.file_path,
               devGetFileRequestData.file_checksum,
               devGetFileRequestData.file_version);

  // set request params
  memset((void*)easyCloudContext.service_status.bin_file, 0, MAX_SIZE_FILE_PATH);
  memset((void*)easyCloudContext.service_status.bin_md5, 0, MAX_SIZE_FILE_MD5);
  memset((void*)easyCloudContext.service_status.latestRomVersion, 0, MAX_SIZE_FW_VERSION);
  easyCloudContext.service_status.bin_file_size = 0;
  
  strncpy(easyCloudContext.service_status.bin_file, 
          devGetFileRequestData.file_path, MAX_SIZE_FILE_PATH);
  strncpy(easyCloudContext.service_status.bin_md5, 
          devGetFileRequestData.file_checksum, MAX_SIZE_FILE_MD5);
  strncpy(easyCloudContext.service_status.latestRomVersion, 
          devGetFileRequestData.file_version, MAX_SIZE_FW_VERSION);
  
  // get file data
  err = EasyCloudGetRomData(&easyCloudContext);
  require_noerr_action( err, exit, 
                       cloud_if_log("ERROR: EasyCloudGetRomData failed! err=%d", err) );
  
  // notify, record file size for user
   cloud_if_log("Get file from server done! file_size=%lld",
                easyCloudContext.service_status.bin_file_size);
   inContext->appStatus.virtualDevStatus.RecvRomFileSize = \
                easyCloudContext.service_status.bin_file_size;
  
  return kNoErr;
  
exit:
  return err;
}

OSStatus MVDCloudInterfaceStop(mico_Context_t* const inContext)
{  
  cloud_if_log_trace();
  OSStatus err = kUnknownErr;
  
  cloud_if_log("MVDCloudInterfaceStop");
  err = EasyCloudServiceStop(&easyCloudContext);
  require_noerr_action( err, exit, 
                       cloud_if_log("ERROR: EasyCloudServiceStop err=%d.", err) );
  return kNoErr;
  
exit:
  return err;
}

OSStatus MVDCloudInterfaceDeinit(mico_Context_t* const inContext)
{  
  cloud_if_log_trace();
  OSStatus err = kUnknownErr;
  
  cloud_if_log("MVDCloudInterfaceDeinit");
  err = EasyCloudServiceDeInit(&easyCloudContext);
  require_noerr_action( err, exit, 
                       cloud_if_log("ERROR: EasyCloudServiceDeInit err=%d.", err) );
  return kNoErr;
  
exit:
  return err;
}
