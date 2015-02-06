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
//#include "EasyCloudUtils.h"
#include "MVDCloudTest.h"

#define mvd_log(M, ...) custom_log("MVD", M, ##__VA_ARGS__)
#define mvd_log_trace() custom_log_trace("MVD")


//#define DEFAULT_MVD_CLOUD_CONNECTED_MSG_2CLOUD     "{\"MVDCloud\":\"connected\"}"
//#define DEFAULT_MVD_CLOUD_CONNECTED_MSG_2MCU       "[MVD]Cloud status: connected\r\n"
//#define DEFAULT_MVD_CLOUD_DISCONNECTED_MSG_2MCU    "[MVD]Cloud status: disconnected\r\n"


/////////////////////////////////////////////////////
extern uint64_t cloud_test_data_cnt;
extern uint64_t cloud_test_echo_data_cnt;
////////////////////////////////////////////////////

//static bool _is_wifi_station_on = false;
static mico_semaphore_t _is_wifi_station_on_sem = NULL;
static mico_semaphore_t _reset_cloud_info_sem = NULL;

void mvdNotify_WifiStatusHandler(WiFiEvent event, mico_Context_t * const inContext)
{
  mvd_log_trace();
  (void)inContext;
  switch (event) {
  case NOTIFY_STATION_UP:
    //_is_wifi_station_on = true;
    if(NULL == _is_wifi_station_on_sem) {
      mico_rtos_init_semaphore(&_is_wifi_station_on_sem, 1);
    }
    mico_rtos_set_semaphore(&_is_wifi_station_on_sem);
    break;
  case NOTIFY_STATION_DOWN:
    //_is_wifi_station_on = false;
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

void MVDCloudTestThread(void *arg)
{
  OSStatus err = kUnknownErr;
  mico_Context_t *inContext = (mico_Context_t *)arg;
  int test_if_cnt = 0;
  
  /* wait for wifi station connect */
  // Regisist wifi connected notifications
  err = MICOAddNotification( mico_notify_WIFI_STATUS_CHANGED, (void *)mvdNotify_WifiStatusHandler );
  require_noerr_action( err, exit,
                       mvd_log("ERROR: MICOAddNotification [mico_notify_WIFI_STATUS_CHANGED] failed!")); 
  
  // wait wifi connect semaphore
  if(NULL == _is_wifi_station_on_sem) {
    err = mico_rtos_init_semaphore(&_is_wifi_station_on_sem, 1);
  }
  require_noerr_action( err, exit,
                       mvd_log("ERROR: mico_rtos_init_semaphore [_is_wifi_station_on_sem] failed!"));
  while(kNoErr != mico_rtos_get_semaphore(&_is_wifi_station_on_sem, MICO_WAIT_FOREVER));
  mvd_log("wifi station connected.");
     
  /* interface test */
  for(test_if_cnt = 0; test_if_cnt < 3; test_if_cnt++){
    err = easycloud_if_test(inContext, false);
    require_noerr_action(err, exit, 
                         mvd_log("ERROR: easycloud_if_test failed!") );
  }
  
  /* transmission test */
  err = easycloud_transmission_test(inContext);
  require_noerr_action(err, exit, 
                       mvd_log("ERROR: easycloud_transmission_test failed!") );
  
  /* OTA test */
  err = easycloud_ota_test(inContext);
  require_noerr_action(err, exit, 
                       mvd_log("ERROR: easycloud_ota_test failed!") );
  
  mvd_log("[SUCCESS]MVD cloud test all done!");
  mico_rtos_delete_thread(NULL);
  return;
  
exit:
  mvd_log("[FAILED] MVDCloudTestThread exit err=%d",err);
  mico_rtos_delete_thread(NULL);
  return;
}

void MVDDevCloudInfoReset(void *arg)
{
  OSStatus err = kUnknownErr;
  MVDResetRequestData_t devDefaultResetData;
  mico_Context_t *inContext = (mico_Context_t *)arg;
  uint8_t cnt = 3;

  /* cloud reset */
  do{
    mvd_log("[MVD]Device reset EasyCloud info [%d]try ...", 4-cnt);
    memset((void*)&devDefaultResetData, 0, sizeof(devDefaultResetData));
    strncpy(devDefaultResetData.loginId,
            inContext->flashContentInRam.appConfig.virtualDevConfig.loginId,
            MAX_SIZE_LOGIN_ID);
    strncpy(devDefaultResetData.devPasswd,
            inContext->flashContentInRam.appConfig.virtualDevConfig.devPasswd,
            MAX_SIZE_DEV_PASSWD);
    strncpy(devDefaultResetData.user_token,
            inContext->micoStatus.mac,
            MAX_SIZE_USER_TOKEN);
    err = MVDResetCloudDevInfo(inContext, devDefaultResetData);
    if(kNoErr == err){
      mvd_log("[MVD]Device reset EasyCloud info [OK]");
    }
    else{
      mvd_log("[MVD]Device EasyCloud info [FAILED]");
    }
    
    cnt--;
  }while((kNoErr != err) && (cnt > 0));
  
  // send ok semaphore
  mico_rtos_set_semaphore(&_reset_cloud_info_sem);
  
  mvd_log("[MVD]Thread MVDDevCloudInfoReset EXIT: exit code=%d",err);
  mico_rtos_delete_thread(NULL);
  return;
}

/*******************************************************************************
 * virtual device interfaces init
 ******************************************************************************/

void MVDRestoreDefault(mico_Context_t* const context)
{
  // start a thread to reset device EasyCloud info
  mico_rtos_init_semaphore(&_reset_cloud_info_sem, 1);
  mico_rtos_create_thread(NULL, MICO_APPLICATION_PRIORITY, "MVD resetCloudInfo", 
                                MVDDevCloudInfoReset, 0x800, 
                                context );
  mico_rtos_get_semaphore(&_reset_cloud_info_sem, 5000);  // 5s timeout
  
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
  mvd_log("[MVD]Device local config reset [OK]");
}

OSStatus MVDStart(mico_Context_t* const inContext)
{
  OSStatus err = kUnknownErr;
  
  // init MVD status
  inContext->appStatus.virtualDevStatus.isCloudConnected = false;
  inContext->appStatus.virtualDevStatus.RecvRomFileSize = 0;
  
  // init USART
  err = MVDDevInterfaceInit(inContext);
  require_noerr_action(err, exit, 
                       mvd_log("ERROR: virtual device mcu interface init failed!") );
  
  // cloud test thread
  err = mico_rtos_create_thread(NULL, MICO_APPLICATION_PRIORITY, "MVD_Cloud_test", 
                                MVDCloudTestThread, STACK_SIZE_MVD_CLOUD_TEST_THREAD, 
                                inContext );
  require_noerr_action(err, exit, 
                       mvd_log("ERROR: create MVD cloud test thread failed!") );
  
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
  // must be activated first
  if(!MVDIsActivated(context)){
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
  
  /////////////////////////////// for test//////////////////////////////////////
  if('z' != inBuf[0]){
    // recv data "xxx..." from server
    cloud_test_data_cnt += inBufLen;
    mvd_log("[MVD]recv_cnt = [%d/%lld]", inBufLen, cloud_test_data_cnt);
    //err = MVDCloudInterfaceSend(inBuf, inBufLen); // response to cloud
    err = kNoErr;
  }
  else {
    // echo data "zzz..."
    cloud_test_echo_data_cnt += inBufLen;
    err = kNoErr;
  }
  //////////////////////////////////////////////////////////////////////////////
  
  //err = MVDDevInterfaceSend(inBuf, inBufLen); // transfer raw data to MCU
  //require_noerr_action( err, exit, mvd_log("ERROR: send to MCU error! err=%d", err) );
  //return kNoErr;
  
exit:
  return err;
}

// handle message from MCU
OSStatus MVDDeviceMsgProcess(mico_Context_t* const context, 
                             uint8_t *inBuf, unsigned int inBufLen)
{
  mvd_log_trace();
  OSStatus err = kUnknownErr;
  
  //////////////////////////////////for test///////////////////////////////////
  MVDActivateRequestData_t devDefaultActivateData;
  MVDAuthorizeRequestData_t devDefaultAuthorizeData;
  MVDResetRequestData_t devCloudResetData;
  MVDOTARequestData_t OTAData;
  MVDDownloadFileRequestData_t devGetFileRequestData;
  
  json_object *new_obj = NULL;
  uint8_t recv_param_check = 0;  // must recv all three params
  
  memset((void*)(&devDefaultActivateData), 0, sizeof(devDefaultActivateData));
  memset((void*)(&devDefaultAuthorizeData), 0, sizeof(devDefaultAuthorizeData));
  memset((void*)(&devCloudResetData), 0, sizeof(devCloudResetData));
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
    else if(0 == strncmp("Activate", (const char*)inBuf, inBufLen)){
    // authorize
    mvd_log("Device Activate ...");
    memset((void*)&devDefaultActivateData, 0, sizeof(devDefaultActivateData));
    strncpy(devDefaultActivateData.loginId,
            context->flashContentInRam.appConfig.virtualDevConfig.loginId,
            MAX_SIZE_LOGIN_ID);
    strncpy(devDefaultActivateData.devPasswd,
            context->flashContentInRam.appConfig.virtualDevConfig.devPasswd,
            MAX_SIZE_DEV_PASSWD);
    strncpy(devDefaultActivateData.user_token,
            "mxchip-authorize-test",
            MAX_SIZE_USER_TOKEN);
    err = MVDCloudInterfaceDevActivate(context, devDefaultActivateData);
    if(kNoErr == err){
      mvd_log("Device Activate [OK]");
    }
    else{
      mvd_log("Device Activate [FAILED]");
    }
  }
  else if(0 == strncmp("Authorize", (const char*)inBuf, inBufLen)){
    // authorize
    mvd_log("Device authorize ...");
    memset((void*)&devDefaultAuthorizeData, 0, sizeof(devDefaultAuthorizeData));
    strncpy(devDefaultAuthorizeData.loginId,
            context->flashContentInRam.appConfig.virtualDevConfig.loginId,
            MAX_SIZE_LOGIN_ID);
    strncpy(devDefaultAuthorizeData.devPasswd,
            context->flashContentInRam.appConfig.virtualDevConfig.devPasswd,
            MAX_SIZE_DEV_PASSWD);
    strncpy(devDefaultAuthorizeData.user_token,
            "mxchip-authorize-test",
            MAX_SIZE_USER_TOKEN);
    err = MVDCloudInterfaceDevAuthorize(context, devDefaultAuthorizeData);
    if(kNoErr == err){
      mvd_log("Device authorize [OK]");
    }
    else{
      mvd_log("Device authorize [FAILED]");
    }
  }
  else if(0 == strncmp("Reset", (const char*)inBuf, inBufLen)){
    // authorize
    mvd_log("Device Cloud Reset ...");
    memset((void*)&devCloudResetData, 0, sizeof(devCloudResetData));
    strncpy(devCloudResetData.loginId,
            context->flashContentInRam.appConfig.virtualDevConfig.loginId,
            MAX_SIZE_LOGIN_ID);
    strncpy(devCloudResetData.devPasswd,
            context->flashContentInRam.appConfig.virtualDevConfig.devPasswd,
            MAX_SIZE_DEV_PASSWD);
    strncpy(devCloudResetData.user_token,
            "mxchip-reset-test",
            MAX_SIZE_USER_TOKEN);
    err = MVDCloudInterfaceResetCloudDevInfo(context, devCloudResetData);
    if(kNoErr == err){
      mvd_log("Device Cloud Reset [OK]");
    }
    else{
      mvd_log("Device Cloud Reset [FAILED]");
    }
  }
  //////////////////////////////////////////////////////////////////////////////
  else{
    err = MVDCloudInterfaceSend(inBuf, inBufLen);  // transfer raw data to Cloud
    require_noerr_action( err, exit, mvd_log("ERROR: send to cloud error! err=%d", err) );
  }
  
exit:
  return err;
}

