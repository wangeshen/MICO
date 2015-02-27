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
void cloudServiceStatusChangedHandler(void* context, easycloud_service_status_t serviceStateInfo)
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

OSStatus MVDCloudInterfaceInit(mico_Context_t* const inContext)
{
  OSStatus err = kUnknownErr;
  int cloudServiceLibVersion = 0;
  
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
  memset((void*)&(easyCloudContext.service_status), '\0', sizeof(easyCloudContext.service_status));
  easyCloudContext.service_status.isActivated = inContext->flashContentInRam.appConfig.virtualDevConfig.isActivated;
  strncpy(easyCloudContext.service_status.deviceId, 
          inContext->flashContentInRam.appConfig.virtualDevConfig.deviceId, MAX_SIZE_DEVICE_ID);
  strncpy(easyCloudContext.service_status.masterDeviceKey, 
          inContext->flashContentInRam.appConfig.virtualDevConfig.masterDeviceKey, MAX_SIZE_DEVICE_KEY);
  
  cloudServiceLibVersion = EasyCloudServiceVersion(&easyCloudContext);
  cloud_if_log("EasyCloud library version: %d.%d.%d", 
               (cloudServiceLibVersion & 0x00FF0000) >> 16,
               (cloudServiceLibVersion & 0x0000FF00) >> 8,
               (cloudServiceLibVersion & 0x000000FF));
  
  err = EasyCloudServiceInit(&easyCloudContext);
  require_noerr_action( err, exit, cloud_if_log("ERROR: EasyCloud service init failed.") );
  
  // start cloud service
  err = EasyCloudServiceStart(&easyCloudContext);
  require_noerr_action( err, exit, cloud_if_log("ERROR: EasyCloud service start failed.") );
  return kNoErr;
  
exit:
  return err;
}

OSStatus MVDCloudInterfaceSend(unsigned char *inBuf, unsigned int inBufLen)
{
  cloud_if_log_trace();
  OSStatus err = kUnknownErr;

  cloud_if_log("MVD => Cloud[publish]:[%d]=%.*s", inBufLen, inBufLen, inBuf);
  err = EasyCloudPublish(&easyCloudContext, inBuf, inBufLen);
  require_noerr_action( err, exit, cloud_if_log("ERROR: MVDCloudInterfaceSend failed! err=%d", err) );
  return kNoErr;
  
exit:
  return err;
}

OSStatus MVDCloudInterfaceSendto(const char* topic, unsigned char *inBuf, unsigned int inBufLen)
{
  cloud_if_log_trace();
  OSStatus err = kUnknownErr;

  cloud_if_log("MVD => Cloud[%s]:[%d]=%.*s", topic, inBufLen, inBufLen, inBuf);
  err = EasyCloudPublishto(&easyCloudContext, topic, inBuf, inBufLen);
  require_noerr_action( err, exit, cloud_if_log("ERROR: MVDCloudInterfaceSendto failed! err=%d", err) );
  return kNoErr;
  
exit:
  return err;
}

OSStatus MVDCloudInterfaceSendtoChannel(const char* channel, unsigned char *inBuf, unsigned int inBufLen)
{
  cloud_if_log_trace();
  OSStatus err = kUnknownErr;

  cloud_if_log("MVD => Cloud[%s]:[%d]=%.*s", channel, inBufLen, inBufLen, inBuf);
  if(NULL == channel){
    err = EasyCloudPublish(&easyCloudContext, inBuf, inBufLen);
  }
  else{
    err = EasyCloudPublishtoChannel(&easyCloudContext, channel, inBuf, inBufLen);
  }
  require_noerr_action( err, exit, cloud_if_log("ERROR: MVDCloudInterfaceSendtoChannel failed! err=%d", err) );
  return kNoErr;
  
exit:
  return err;
}

OSStatus MVDCloudInterfaceDevActivate(mico_Context_t* const inContext,
                                      MVDActivateRequestData_t devActivateRequestData)
{
  cloud_if_log_trace();
  OSStatus err = kUnknownErr;
  easycloud_service_state_t cloudServiceState = EASYCLOUD_STOPPED;
  
  cloud_if_log("Device activate...");
   
  //check status
  cloudServiceState = EasyCloudServiceState(&easyCloudContext);
  if (EASYCLOUD_STOPPED == cloudServiceState){
    return kStateErr;
  }
  
#ifdef MVD_LOGINID_DEVPASS_CHECK 
  // login_id/dev_passwd set(not default value) ?
  if((0 != strncmp((char*)DEFAULT_LOGIN_ID,
                   inContext->flashContentInRam.appConfig.virtualDevConfig.loginId,       
                   strlen((char*)DEFAULT_LOGIN_ID))) ||
     (0 != strncmp((char*)DEFAULT_DEV_PASSWD,
                   inContext->flashContentInRam.appConfig.virtualDevConfig.devPasswd,
                   strlen((char*)DEFAULT_DEV_PASSWD))))
  {
    // login_id/dev_passwd ok ?
    if((0 != strncmp(inContext->flashContentInRam.appConfig.virtualDevConfig.loginId, 
                     devActivateRequestData.loginId, 
                     strlen(inContext->flashContentInRam.appConfig.virtualDevConfig.loginId))) ||
       (0 != strncmp(inContext->flashContentInRam.appConfig.virtualDevConfig.devPasswd, 
                     devActivateRequestData.devPasswd, 
                     strlen(inContext->flashContentInRam.appConfig.virtualDevConfig.devPasswd))))
    {
      // devPass err
      cloud_if_log("ERROR: MVDCloudInterfaceDevActivate: loginId/devPasswd mismatch!");
      return kMismatchErr;
    }
  }
  cloud_if_log("MVDCloudInterfaceDevActivate: loginId/devPasswd ok!");
  
  //ok, set cloud context
  strncpy(easyCloudContext.service_config_info.loginId, 
          devActivateRequestData.loginId, MAX_SIZE_LOGIN_ID);
  strncpy(easyCloudContext.service_config_info.devPasswd, 
          devActivateRequestData.devPasswd, MAX_SIZE_DEV_PASSWD);
#endif
  strncpy(easyCloudContext.service_config_info.userToken, 
          devActivateRequestData.user_token, MAX_SIZE_USER_TOKEN);
    
  // activate request
  err = EasyCloudActivate(&easyCloudContext);
  require_noerr_action(err, exit, 
                       cloud_if_log("ERROR: MVDCloudInterfaceDevActivate failed! err=%d", err) );
  
  // write activate data back to flash
  mico_rtos_lock_mutex(&inContext->flashContentInRam_mutex);
  inContext->flashContentInRam.appConfig.virtualDevConfig.isActivated = true;
  strncpy(inContext->flashContentInRam.appConfig.virtualDevConfig.deviceId,
          easyCloudContext.service_status.deviceId, MAX_SIZE_DEVICE_ID);
  strncpy(inContext->flashContentInRam.appConfig.virtualDevConfig.masterDeviceKey,
          easyCloudContext.service_status.masterDeviceKey, MAX_SIZE_DEVICE_KEY);

#ifdef MVD_LOGINID_DEVPASS_CHECK  
  strncpy(inContext->flashContentInRam.appConfig.virtualDevConfig.loginId,
          easyCloudContext.service_config_info.loginId, MAX_SIZE_LOGIN_ID);
  strncpy(inContext->flashContentInRam.appConfig.virtualDevConfig.devPasswd,
          easyCloudContext.service_config_info.devPasswd, MAX_SIZE_DEV_PASSWD);
#endif
  strncpy(inContext->flashContentInRam.appConfig.virtualDevConfig.userToken,
          easyCloudContext.service_config_info.userToken, MAX_SIZE_USER_TOKEN);

  err = MICOUpdateConfiguration(inContext);
  mico_rtos_unlock_mutex(&inContext->flashContentInRam_mutex);
  require_noerr_action(err, exit, 
                       cloud_if_log("ERROR: activate write flash failed! err=%d", err) );
  
  return kNoErr;
  
exit:
  return err;
}

OSStatus MVDCloudInterfaceDevAuthorize(mico_Context_t* const inContext,
                                       MVDAuthorizeRequestData_t devAuthorizeReqData)
{
  cloud_if_log_trace();
  OSStatus err = kUnknownErr;
  easycloud_service_state_t cloudServiceState = EASYCLOUD_STOPPED;
  
  cloud_if_log("Device authorize...");

  cloudServiceState = EasyCloudServiceState(&easyCloudContext);
  if (EASYCLOUD_STOPPED == cloudServiceState){
    return kStateErr;
  }
  
  #ifdef MVD_LOGINID_DEVPASS_CHECK  
  // dev_passwd ok ?
  if(0 != strncmp(inContext->flashContentInRam.appConfig.virtualDevConfig.devPasswd, 
                  devAuthorizeReqData.devPasswd, 
                  strlen(inContext->flashContentInRam.appConfig.virtualDevConfig.devPasswd)))
  {
    // devPass err
    cloud_if_log("ERROR: MVDCloudInterfaceDevAuthorize: devPasswd mismatch!");
    return kMismatchErr;
  }
  cloud_if_log("MVDCloudInterfaceDevAuthorize: devPasswd ok!");
  
  //ok, set cloud context
  strncpy(easyCloudContext.service_config_info.loginId, 
          devAuthorizeReqData.loginId, MAX_SIZE_LOGIN_ID);
  strncpy(easyCloudContext.service_config_info.devPasswd, 
          devAuthorizeReqData.devPasswd, MAX_SIZE_DEV_PASSWD);
#endif
  strncpy(easyCloudContext.service_config_info.userToken, 
          devAuthorizeReqData.user_token, MAX_SIZE_USER_TOKEN);
  
  err = EasyCloudAuthorize(&easyCloudContext);
  require_noerr_action( err, exit, cloud_if_log("ERROR: authorize failed! err=%d", err) );
  
  // write back to flash
//  mico_rtos_lock_mutex(&inContext->flashContentInRam_mutex);
//  strncpy(inContext->flashContentInRam.appConfig.virtualDevConfig.userToken,
//          easyCloudContext.service_config_info.userToken, MAX_SIZE_USER_TOKEN);
//  err = MICOUpdateConfiguration(inContext);
//  mico_rtos_unlock_mutex(&inContext->flashContentInRam_mutex);
//  require_noerr_action( err, exit, cloud_if_log("ERROR: authorize write flash failed! err=%d", err) );
  
  return kNoErr;
  
exit:
  return err;
}

OSStatus MVDCloudInterfaceDevFirmwareUpdate(mico_Context_t* const inContext,
                                            MVDOTARequestData_t devOTARequestData)
{
  cloud_if_log_trace();
  OSStatus err = kUnknownErr;
  // set OTA flash address
  ecs_ota_flash_params_t ota_flashParams = {
    MICO_FLASH_FOR_UPDATE,
    UPDATE_START_ADDRESS,
    UPDATE_END_ADDRESS,
    UPDATE_FLASH_SIZE
  };

  cloud_if_log("Update firmware...");
 
#ifdef MVD_LOGINID_DEVPASS_CHECK  
  // login_id/dev_passwd ok ?
  if((0 != strncmp(inContext->flashContentInRam.appConfig.virtualDevConfig.loginId, 
                   devOTARequestData.loginId, 
                   strlen(inContext->flashContentInRam.appConfig.virtualDevConfig.loginId))) ||
     (0 != strncmp(inContext->flashContentInRam.appConfig.virtualDevConfig.devPasswd, 
                   devOTARequestData.devPasswd, 
                   strlen(inContext->flashContentInRam.appConfig.virtualDevConfig.devPasswd))))
  {
    // devPass err
    cloud_if_log("ERROR: MVDCloudInterfaceDevFirmwareUpdate: loginId/devPasswd mismatch!");
    return kMismatchErr;
  }
  cloud_if_log("MVDCloudInterfaceDevFirmwareUpdate: loginId/devPasswd ok!");
#endif
  
  //get latest rom version, file_path, md5
  err = EasyCloudGetLatestRomVersion(&easyCloudContext);
  require_noerr_action( err, exit, cloud_if_log("ERROR: EasyCloudGetLatestRomVersion failed! err=%d", err) );
  
  //FW version compare
  cloud_if_log("currnt_version=%s", inContext->flashContentInRam.appConfig.virtualDevConfig.romVersion);
  cloud_if_log("latestRomVersion=%s", easyCloudContext.service_status.latestRomVersion);
  cloud_if_log("bin_file=%s", easyCloudContext.service_status.bin_file);
  cloud_if_log("bin_md5=%s", easyCloudContext.service_status.bin_md5);
  
#ifdef MVD_FW_UPDAETE_VERSION_CHECK  
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
  
  //get rom data && write to flash
  err = EasyCloudGetRomData(&easyCloudContext, ota_flashParams);
  require_noerr_action( err, exit, 
                       cloud_if_log("ERROR: EasyCloudGetRomData failed! err=%d", err) );
  
  //update rom version in flash
  mico_rtos_lock_mutex(&inContext->flashContentInRam_mutex);
  memset(inContext->flashContentInRam.appConfig.virtualDevConfig.romVersion,
         0, MAX_SIZE_FW_VERSION);
  strncpy(inContext->flashContentInRam.appConfig.virtualDevConfig.romVersion,
          easyCloudContext.service_status.latestRomVersion, 
          strlen(easyCloudContext.service_status.latestRomVersion));
  inContext->appStatus.virtualDevStatus.RecvRomFileSize = easyCloudContext.service_status.bin_file_size;

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
  
#ifdef MVD_LOGINID_DEVPASS_CHECK
  // login_id/dev_passwd ok ?
  if((0 != strncmp(inContext->flashContentInRam.appConfig.virtualDevConfig.loginId, 
                   devResetRequestData.loginId, 
                   strlen(inContext->flashContentInRam.appConfig.virtualDevConfig.loginId))) ||
     (0 != strncmp(inContext->flashContentInRam.appConfig.virtualDevConfig.devPasswd, 
                   devResetRequestData.devPasswd, 
                   strlen(inContext->flashContentInRam.appConfig.virtualDevConfig.devPasswd))))
  {
    // devPass err
    cloud_if_log("ERROR: MVDCloudInterfaceResetCloudDevInfo: loginId/devPasswd mismatch!");
    return kMismatchErr;
  }
  cloud_if_log("MVDCloudInterfaceResetCloudDevInfo: loginId/devPasswd ok!");
#endif
  
  err = EasyCloudDeviceReset(&easyCloudContext);
  require_noerr_action( err, exit, cloud_if_log("ERROR: EasyCloudDeviceReset failed! err=%d", err) );
  
  mico_rtos_lock_mutex(&inContext->flashContentInRam_mutex);
  inContext->flashContentInRam.appConfig.virtualDevConfig.isActivated = false;  // need to reActivate
  sprintf(inContext->flashContentInRam.appConfig.virtualDevConfig.deviceId, DEFAULT_DEVICE_ID);
  sprintf(inContext->flashContentInRam.appConfig.virtualDevConfig.masterDeviceKey, DEFAULT_DEVICE_KEY);
  sprintf(inContext->flashContentInRam.appConfig.virtualDevConfig.loginId, DEFAULT_LOGIN_ID);
  sprintf(inContext->flashContentInRam.appConfig.virtualDevConfig.devPasswd, DEFAULT_DEV_PASSWD);
  //sprintf(inContext->flashContentInRam.appConfig.virtualDevConfig.userToken, DEFAULT_USER_TOKEN);
  sprintf(inContext->flashContentInRam.appConfig.virtualDevConfig.userToken, inContext->micoStatus.mac);
  //inContext->appStatus.virtualDevStatus.isCloudConnected = false;
  MICOUpdateConfiguration(inContext);
  mico_rtos_unlock_mutex(&inContext->flashContentInRam_mutex);
  
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
  
#ifdef MVD_LOGINID_DEVPASS_CHECK
  // login_id/dev_passwd ok ?
  if((0 != strncmp(inContext->flashContentInRam.appConfig.virtualDevConfig.loginId, 
                   devOTARequestData.loginId, 
                   strlen(inContext->flashContentInRam.appConfig.virtualDevConfig.loginId))) ||
     (0 != strncmp(inContext->flashContentInRam.appConfig.virtualDevConfig.devPasswd, 
                   devOTARequestData.devPasswd, 
                   strlen(inContext->flashContentInRam.appConfig.virtualDevConfig.devPasswd))))
  {
    // devPass err
    cloud_if_log("ERROR: MVDCloudInterfaceGetFile: loginId/devPasswd mismatch!");
    return kMismatchErr;
  }
  cloud_if_log("MVDCloudInterfaceGetFile: loginId/devPasswd ok!");
#endif

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
  
  // set host/port if bin_file is a relative path like: "/path/to/file", and
  //   port is easyCloudContext.service_config_info.cloudServerPort as default(80)
//  memset((void*)easyCloudContext.service_config_info.cloudServerDomain, 0, MAX_SIZE_DOMAIN_NAME);
//  strncpy(easyCloudContext.service_config_info.cloudServerDomain, 
//          "test.oznerwater.com", strlen("test.oznerwater.com"));
  
  // get file data
  err = EasyCloudGetRomData(&easyCloudContext);
  require_noerr_action( err, exit, 
                       cloud_if_log("ERROR: EasyCloudGetRomData failed! err=%d", err) );
  
  // notify, record file size for user
   cloud_if_log("Get file from server done! file_size=%lld",
                easyCloudContext.service_status.bin_file_size);
   inContext->appStatus.virtualDevStatus.RecvRomFileSize = easyCloudContext.service_status.bin_file_size;
  
  return kNoErr;
  
exit:
  return err;
}

//OSStatus MVDCloudInterfaceStop(mico_Context_t* const inContext)
//{  
//  cloud_if_log_trace();
//  OSStatus err = kUnknownErr;
//  
//  cloud_if_log("MVDCloudInterfaceStop");
//  // stop EasyCloud service
//  err = EasyCloudServiceStop(&easyCloudContext);
//  require_noerr_action( err, exit, 
//                       cloud_if_log("ERROR: EasyCloudServiceStop failed! err=%d", err) );
//  
//exit:
//  return err;
//}
