/**
  ******************************************************************************
  * @file    MVDCloudTest.c 
  * @author  WangEshen
  * @version V1.0.0
  * @date    22-Jan-2015
  * @brief   MVD cloud test.
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

#include "MVDCloudTest.h"
//#include "MicoVirtualDevice.h"
#include "MVDCloudInterfaces.h"
//#include "EasyCloudService.h"
#include "EasyCloudUtils.h"
#include "SocketUtils.h"
#include "StringUtils.h"

#define mvd_cloud_test_log(M, ...) custom_log("MVD_CLOUD_TEST", M, ##__VA_ARGS__)
#define mvd_cloud_test_log_trace() custom_log_trace("MVD_CLOUD_TEST")

#define MVD_CLOUD_TEST_SEND_DATA              "MVD Cloud send data to default topic: device_id/out."
#define MVD_CLOUD_TEST_SENDTO_TOPIC           "user_test"
#define MVD_CLOUD_TEST_SENDTO_TOPIC_DATA      "MVD Cloud send data to user-defined topic: user_test."
#define MVD_CLOUD_TEST_SENDTO_CHANNEL         "test"
#define MVD_CLOUD_TEST_SENDTO_CHANNEL_DATA    "MVD Cloud send data to sub-channel: device_id/out/test"

typedef struct _test_parms_t{
  mico_Context_t* inContext;
  uint32_t msg_length;
  uint32_t period_s;
  uint32_t interval_ms;
}test_params_t;

test_params_t test_params;


OSStatus MVDCloudTest_StartRecv(const char* device_id,
                                uint32_t msg_length, 
                                uint32_t period_s, uint32_t interval_ms)
{
  OSStatus err = kUnknownErr;
  int tcpClient_fd = -1;
  char ipstr[16];
  struct sockaddr_t addr;
  fd_set readfds;
  struct timeval_t t;
  
  /* create http request data */
  uint8_t *httpRequestData = NULL;
  size_t httpRequestDataLen = 0;
  ECS_HTTPHeader_t *httpHeader = NULL;
  
  const char* host = EASYCLOUD_TEST_SERVER;
  uint16_t port = EASYCLOUD_TEST_PORT;
  char* request_url = NULL;
  uint32_t request_url_len = 0;
  
  // set request_url = "/startTest?id=xxx&length=xxx&deadline=xxx&interval=xxx"
  char *param_deviceId = "?id=";
  char *param_length = "&length=";
  char *param_period = "&deadline=";
  char *param_interval = "&interval=";
  
  char string_msgLen[32] = {0};
  char string_periodLen[32] = {0};
  char string_intervalLen[32] = {0};
  
  uint32_t deviceIdLen = 0;
  uint32_t msgLengthLen = 0;
  uint32_t periodLen = 0;
  uint32_t intervalLen = 0;
  
  deviceIdLen = strlen(device_id);
  Int2Str((uint8_t*)string_msgLen, msg_length);
  msgLengthLen = strlen(string_msgLen);
  Int2Str((uint8_t*)string_periodLen, period_s);
  periodLen = strlen(string_periodLen);
  Int2Str((uint8_t*)string_intervalLen, interval_ms);
  intervalLen = strlen(string_intervalLen);
  
  request_url_len = strlen(EASYCLOUD_TEST_URL_START_RECV) + \
                          strlen(param_deviceId) + deviceIdLen + \
                          strlen(param_length) + msgLengthLen + \
                          strlen(param_period) + periodLen + \
                          strlen(param_interval) + intervalLen;
 
  request_url = (char*)malloc(request_url_len+1);
  if (NULL == request_url){
    goto exit;
  }
  memset(request_url, 0, request_url_len+1);
  strncpy(request_url, EASYCLOUD_TEST_URL_START_RECV, strlen(EASYCLOUD_TEST_URL_START_RECV));
  strncat(request_url, param_deviceId, strlen(param_deviceId));
  strncat(request_url, device_id, deviceIdLen);
  strncat(request_url, param_length, strlen(param_length));
  strncat(request_url, string_msgLen, msgLengthLen);
  strncat(request_url, param_period, strlen(param_period));
  strncat(request_url, string_periodLen, periodLen);
  strncat(request_url, param_interval, strlen(param_interval));
  strncat(request_url, string_intervalLen, intervalLen);
  
  mvd_cloud_test_log("request: [%.*s]", request_url_len, request_url);
  
  httpHeader = ECS_HTTPHeaderCreate();
  require_action( httpHeader, exit, err = kNoMemoryErr );
  ECS_HTTPHeaderClear( httpHeader );
  
  //create tcp connect
  mvd_cloud_test_log("tcp client start to connect...");
  err = gethostbyname((char *)host, (uint8_t *)ipstr, 16);
  require_noerr(err, exit);
  mvd_cloud_test_log("cloud service host:%s, ip: %s", host, ipstr);
  
  tcpClient_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  require(tcpClient_fd != -1, exit);
  
  addr.s_ip = inet_addr(ipstr); 
  addr.s_port = port;
  
  err = connect(tcpClient_fd, &addr, sizeof(addr));
  require_noerr_quiet(err, exit);
  
  mvd_cloud_test_log("EasyCloud server connected at port=%d, fd=%d", 
                         port,
                         tcpClient_fd);
  
  // send request data
  mvd_cloud_test_log("tcp client send activate request...");
  err = ECS_CreateHTTPMessageEx(ECS_kHTTPGetMethod, 
                            host, request_url,
                            ECS_kMIMEType_JSON, 
                            NULL, 0,
                            &httpRequestData, &httpRequestDataLen);
  require_noerr( err, exit );
  // fix http request data
  memset(httpRequestData, 0, httpRequestDataLen);
  sprintf((char*)httpRequestData, "GET %s HTTP/1.1\r\nHost:%s\r\nCache-Control: no-cache\r\n\r\n", request_url, host);
  //sprintf((char*)httpRequestData, "GET %s HTTP/1.1\r\nHost:api.easylink.io\r\n\r\n", request_url);
  httpRequestDataLen = strlen((const char*)httpRequestData);
  ///////////////////////////////////////////////////
  
  mvd_cloud_test_log("send http package: len=%d,\r\n%s", httpRequestDataLen, httpRequestData);
  
  err = SocketSend( tcpClient_fd, httpRequestData, httpRequestDataLen );
  if (httpRequestData != NULL) {
    free(httpRequestData);
    httpRequestDataLen = 0;
  }
  require_noerr( err, exit );
  
  // get http response
  FD_ZERO(&readfds);
  FD_SET(tcpClient_fd, &readfds);
  t.tv_sec = 3;
  t.tv_usec = 0;
  err = select(1, &readfds, NULL, NULL, &t);
  require(err >= 1, exit);
  
  if (FD_ISSET(tcpClient_fd, &readfds)) {
    err = ECS_SocketReadHTTPHeader( tcpClient_fd, httpHeader );             
    switch ( err )
    {
    case kNoErr:{
      // statusCode check
      if(ECS_kStatusOK != httpHeader->statusCode){
        mvd_cloud_test_log("ERROR: server response statusCode=%d", 
                              httpHeader->statusCode);
        err = kRequestErr;
        goto exit;
      }
      
      mvd_cloud_test_log("startTest done!");
      err = kNoErr;
      break;
    }
    
    default:{
      mvd_cloud_test_log("ERROR: HTTP Header parse internal error: %d", err);
      break;
    }
    }    
  }

exit:
  mvd_cloud_test_log("Exit: EasyCloud tcp client exit err = %d", err);
  ECS_HTTPHeaderClear( httpHeader );
  if(httpHeader) free(httpHeader);
  if(tcpClient_fd != -1){
    close(tcpClient_fd);
    tcpClient_fd = -1;
  }
  if(NULL != request_url){
    free(request_url);
    request_url = NULL;
  }
  
  return err;
}

OSStatus MVDCloudTest_StopRecv(const char* device_id)
{
  OSStatus err = kUnknownErr;
  int tcpClient_fd = -1;
  char ipstr[16];
  struct sockaddr_t addr;
  fd_set readfds;
  struct timeval_t t;
  
  /* create http request data */
  uint8_t *httpRequestData = NULL;
  size_t httpRequestDataLen = 0;
  ECS_HTTPHeader_t *httpHeader = NULL;
  
  const char* host = EASYCLOUD_TEST_SERVER;
  uint16_t port = EASYCLOUD_TEST_PORT;
  char* request_url = NULL;
  uint32_t request_url_len = 0;
  
  // set request_url = "/stopTest?name=<device_id>"
  char *param_deviceId = "?name=";
  uint32_t deviceIdLen = 0;
  
  deviceIdLen = strlen(device_id);
  request_url_len = strlen(EASYCLOUD_TEST_URL_STOP_RECV) + \
                          strlen(param_deviceId) + deviceIdLen;
 
  request_url = (char*)malloc(request_url_len+1);
  if (NULL == request_url){
    goto exit;
  }
  memset(request_url, 0, request_url_len+1);
  strncpy(request_url, EASYCLOUD_TEST_URL_STOP_RECV, strlen(EASYCLOUD_TEST_URL_STOP_RECV));
  strncat(request_url, param_deviceId, strlen(param_deviceId));
  strncat(request_url, device_id, deviceIdLen);
  
  mvd_cloud_test_log("request: [%.*s]", request_url_len, request_url);
  
  httpHeader = ECS_HTTPHeaderCreate();
  require_action( httpHeader, exit, err = kNoMemoryErr );
  ECS_HTTPHeaderClear( httpHeader );
  
  //create tcp connect
  mvd_cloud_test_log("tcp client start to connect...");
  err = gethostbyname((char *)host, (uint8_t *)ipstr, 16);
  require_noerr(err, exit);
  mvd_cloud_test_log("cloud service host:%s, ip: %s", host, ipstr);
  
  tcpClient_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  require(tcpClient_fd != -1, exit);
  
  addr.s_ip = inet_addr(ipstr); 
  addr.s_port = port;
  
  err = connect(tcpClient_fd, &addr, sizeof(addr));
  require_noerr_quiet(err, exit);
  
  mvd_cloud_test_log("EasyCloud server connected at port=%d, fd=%d", 
                         port,
                         tcpClient_fd);
  
  // send request data
  mvd_cloud_test_log("tcp client send activate request...");
  err = ECS_CreateHTTPMessageEx(ECS_kHTTPGetMethod, 
                            host, request_url,
                            ECS_kMIMEType_JSON, 
                            NULL, 0,
                            &httpRequestData, &httpRequestDataLen);
  require_noerr( err, exit );
  // fix http request data
  memset(httpRequestData, 0, httpRequestDataLen);
  sprintf((char*)httpRequestData, "GET %s HTTP/1.1\r\nHost:%s\r\nCache-Control: no-cache\r\n\r\n", request_url, host);
  //sprintf((char*)httpRequestData, "GET %s HTTP/1.1\r\nHost:api.easylink.io\r\n\r\n", request_url);
  httpRequestDataLen = strlen((const char*)httpRequestData);
  ///////////////////////////////////////////////////
  
  mvd_cloud_test_log("send http package: len=%d,\r\n%s", httpRequestDataLen, httpRequestData);
  
  err = SocketSend( tcpClient_fd, httpRequestData, httpRequestDataLen );
  if (httpRequestData != NULL) {
    free(httpRequestData);
    httpRequestDataLen = 0;
  }
  require_noerr( err, exit );
  
  // get http response
  FD_ZERO(&readfds);
  FD_SET(tcpClient_fd, &readfds);
  t.tv_sec = 3;
  t.tv_usec = 0;
  err = select(1, &readfds, NULL, NULL, &t);
  require(err >= 1, exit);
  
  if (FD_ISSET(tcpClient_fd, &readfds)) {
    err = ECS_SocketReadHTTPHeader( tcpClient_fd, httpHeader );             
    switch ( err )
    {
    case kNoErr:{
      // statusCode check
      if(ECS_kStatusOK != httpHeader->statusCode){
        mvd_cloud_test_log("ERROR: server response statusCode=%d", 
                              httpHeader->statusCode);
        err = kRequestErr;
        goto exit;
      }
      
      mvd_cloud_test_log("stopTest ok!");
      err = kNoErr;
      break;
    }
    
    default:{
      mvd_cloud_test_log("ERROR: HTTP Header parse internal error: %d", err);
      break;
    }
    }    
  }

exit:
  mvd_cloud_test_log("Exit: EasyCloud tcp client exit err = %d", err);
  ECS_HTTPHeaderClear( httpHeader );
  if(httpHeader) free(httpHeader);
  if(tcpClient_fd != -1){
    close(tcpClient_fd);
    tcpClient_fd = -1;
  }
  if(NULL != request_url){
    free(request_url);
    request_url = NULL;
  }
  
  return err;
}

void mvd_test_send_thread(void* arg)
{
  OSStatus err = kUnknownErr;
  char* sendMsg = NULL;
  test_params_t *test_params = NULL;
  int64_t send_cnt = 0;
  int64_t send_ok_cnt = 0;
  char send_ok_string[64] = {0};
  
  test_params = (test_params_t*)arg;
  sendMsg = (char*)malloc(test_params->msg_length + 1);
  if(NULL == sendMsg){
    goto exit;
  }
  memset(sendMsg, 'z', test_params->msg_length);
  sendMsg[test_params->msg_length] = '\0';
  
  send_cnt = (int64_t)((test_params->period_s*1000)/test_params->interval_ms);
  mvd_cloud_test_log("send_cnt = %lld", send_cnt);
  
  while(send_cnt > 0){
    err = MVDSendMsg2Cloud(test_params->inContext, NULL, (unsigned char*)sendMsg, test_params->msg_length);
    if(kNoErr == err){
      send_ok_cnt++;
    }
    send_cnt--;
    mvd_cloud_test_log("send_ok_cnt = %lld", send_ok_cnt);
    mvd_cloud_test_log("send_cnt = %lld", send_cnt);
    mico_thread_msleep(test_params->interval_ms);
  }
  
  sprintf(send_ok_string, "send_ok_cnt = %lld", send_ok_cnt);
  MVDSendMsg2Device(test_params->inContext, (unsigned char*)send_ok_string, strlen(send_ok_string));
  
  
exit:
  if(NULL != sendMsg){
    free(sendMsg);
    sendMsg = NULL;
  }
  mico_rtos_delete_thread(NULL);
  return;
}

OSStatus MVDCloudTest_StartSend(mico_Context_t* inContext,
                                uint32_t msg_length, 
                                uint32_t period_s, uint32_t interval_ms)
{
  //OSStatus err = kUnknownErr;
  
  if((0 == msg_length) || (0 == period_s) || (0 == interval_ms) ||
     (interval_ms >= (period_s*1000)) || (NULL == inContext)){
       return kParamErr;
  }
  
  test_params.inContext = inContext;
  test_params.msg_length = msg_length;
  test_params.period_s = period_s;
  test_params.interval_ms = interval_ms;
  
  return mico_rtos_create_thread(NULL, 
                                 MICO_APPLICATION_PRIORITY, 
                                 "MVDCloudTest_SendThread", mvd_test_send_thread, 0x800, (void*)(&test_params) );
}


/*******************************************************************************
* EasyCloud test functions
*******************************************************************************/
  
/* interface test */
OSStatus easycloud_if_test(mico_Context_t* context)
{
  OSStatus err = kUnknownErr;
  mico_Context_t *inContext = (mico_Context_t*)context;
  MVDActivateRequestData_t devActivateData;
  MVDAuthorizeRequestData_t devAuthorizeData;
  MVDResetRequestData_t devCloudResetData;
  micoMemInfo_t *memInfo = NULL;
  easycloud_service_state_t service_running_state = EASYCLOUD_STOPPED;
  
  int activate_retry_cnt = 3;
  int authorize_retry_cnt = 3;
  int cloud_reset_retry_cnt = 3;
  
  mvd_cloud_test_log("=========== EasyCloud interfaces test ===========");
  /* 1. lib version */
  err = MVDCloudInterfacePrintVersion();
  require_noerr_action(err, exit, 
                       mvd_cloud_test_log("[FAILED] MVDCloudInterfacePrintVersion"));
  mvd_cloud_test_log("[SUCCESS] MVDCloudInterfacePrintVersion");
  
  /* 2. init */
  err = MVDCloudInterfaceInit(context);
  require_noerr_action(err, exit, 
                       mvd_cloud_test_log("[FAILED] MVDCloudInterfaceInit"));
  mvd_cloud_test_log("[SUCCESS] MVDCloudInterfaceInit");
  
  /* 3. activate */
  while(activate_retry_cnt > 0){
    memset((void*)&devActivateData, 0, sizeof(devActivateData));
    strncpy(devActivateData.loginId,
            inContext->flashContentInRam.appConfig.virtualDevConfig.loginId,
            MAX_SIZE_LOGIN_ID);
    strncpy(devActivateData.devPasswd,
            inContext->flashContentInRam.appConfig.virtualDevConfig.devPasswd,
            MAX_SIZE_DEV_PASSWD);
    strncpy(devActivateData.user_token,
            inContext->micoStatus.mac,
            MAX_SIZE_USER_TOKEN);
    err = MVDCloudInterfaceDevActivate(inContext, devActivateData);
    require_noerr_action(err, exit, 
                         mvd_cloud_test_log("[FAILED] MVDCloudInterfaceDevActivate [%d]", activate_retry_cnt));
    mvd_cloud_test_log("[SUCCESS] MVDCloudInterfaceDevActivate [%d]", activate_retry_cnt);
    
    activate_retry_cnt--;
  }
  
  /* 4. authorize */
  while(authorize_retry_cnt > 0){
    memset((void*)&devAuthorizeData, 0, sizeof(devAuthorizeData));
    strncpy(devAuthorizeData.loginId,
            inContext->flashContentInRam.appConfig.virtualDevConfig.loginId,
            MAX_SIZE_LOGIN_ID);
    strncpy(devAuthorizeData.devPasswd,
            inContext->flashContentInRam.appConfig.virtualDevConfig.devPasswd,
            MAX_SIZE_DEV_PASSWD);
    strncpy(devAuthorizeData.user_token,
            inContext->micoStatus.mac,
            MAX_SIZE_USER_TOKEN);
    err = MVDCloudInterfaceDevAuthorize(inContext, devAuthorizeData);
    require_noerr_action(err, exit, 
                         mvd_cloud_test_log("[FAILED] MVDCloudInterfaceDevAuthorize [%d]",authorize_retry_cnt));
    mvd_cloud_test_log("[SUCCESS] MVDCloudInterfaceDevAuthorize [%d]", authorize_retry_cnt);
    
    authorize_retry_cnt--;
  }
  
  /* 5. start */
  err = MVDCloudInterfaceStart(inContext);
  require_noerr_action(err, exit, 
                       mvd_cloud_test_log("[FAILED] MVDCloudInterfaceStart"));
  mvd_cloud_test_log("[SUCCESS] MVDCloudInterfaceStart");
  
  /* 6. wait servie connect */
  do{
    service_running_state = MVDCloudInterfaceGetState();
    mico_thread_sleep(1);
  }while(EASYCLOUD_CONNECTED != service_running_state);
  mvd_cloud_test_log("MVDCloudInterfaceGetState: connected.");
  
  /* 7. send data */
  // default topic: device_id/out
  err = MVDCloudInterfaceSend(MVD_CLOUD_TEST_SEND_DATA, 
                              strlen(MVD_CLOUD_TEST_SEND_DATA));
  require_noerr_action(err, exit, 
                       mvd_cloud_test_log("[FAILED] MVDCloudInterfaceSend"));
  mvd_cloud_test_log("[SUCCESS] MVDCloudInterfaceSend");
  
  // sub-channel: device_id/out/<channel>
  err = MVDCloudInterfaceSendtoChannel(MVD_CLOUD_TEST_SENDTO_CHANNEL, 
                                       MVD_CLOUD_TEST_SENDTO_CHANNEL_DATA, 
                                       strlen(MVD_CLOUD_TEST_SENDTO_CHANNEL_DATA));
  require_noerr_action(err, exit, 
                       mvd_cloud_test_log("[FAILED] MVDCloudInterfaceSendtoChannel"));
  mvd_cloud_test_log("[SUCCESS] MVDCloudInterfaceSendtoChannel");
  
  // user-defined topic
  err = MVDCloudInterfaceSendto(MVD_CLOUD_TEST_SENDTO_TOPIC, 
                                MVD_CLOUD_TEST_SENDTO_TOPIC_DATA, 
                                strlen(MVD_CLOUD_TEST_SENDTO_TOPIC_DATA));
  require_noerr_action(err, exit, 
                       mvd_cloud_test_log("[FAILED] MVDCloudInterfaceSendto"));
  mvd_cloud_test_log("[SUCCESS] MVDCloudInterfaceSendto");
  
  //8. stop
  err = MVDCloudInterfaceStop(inContext);
  require_noerr_action(err, exit, 
                       mvd_cloud_test_log("[FAILED] MVDCloudInterfaceStop"));
  mvd_cloud_test_log("[SUCCESS] MVDCloudInterfaceStop");
  
  //9. reset
  while(cloud_reset_retry_cnt > 0){
    memset((void*)&devCloudResetData, 0, sizeof(devCloudResetData));
    strncpy(devCloudResetData.loginId,
            inContext->flashContentInRam.appConfig.virtualDevConfig.loginId,
            MAX_SIZE_LOGIN_ID);
    strncpy(devCloudResetData.devPasswd,
            inContext->flashContentInRam.appConfig.virtualDevConfig.devPasswd,
            MAX_SIZE_DEV_PASSWD);
    strncpy(devCloudResetData.user_token,
            inContext->micoStatus.mac,
            MAX_SIZE_USER_TOKEN);
    err = MVDCloudInterfaceResetCloudDevInfo(inContext, devCloudResetData);
    require_noerr_action(err, exit, 
                         mvd_cloud_test_log("[FAILED] MVDCloudInterfaceResetCloudDevInfo [%d]", cloud_reset_retry_cnt));
    mvd_cloud_test_log("[SUCCESS] MVDCloudInterfaceResetCloudDevInfo [%d]", cloud_reset_retry_cnt);
    
    cloud_reset_retry_cnt--;
  }
  
  //10. deinit 
  err = MVDCloudInterfaceDeinit(inContext);
  require_noerr_action(err, exit, 
                       mvd_cloud_test_log("[FAILED] MVDCloudInterfaceDeinit"));
  mvd_cloud_test_log("[SUCCESS] MVDCloudInterfaceDeinit");
  
  //11. system memory info
  memInfo = mico_memory_info();
  mvd_cloud_test_log("[TEST_IF_QUIT]System free memory: %d", memInfo->free_memory);
  //sprintf(freeMemString, "[MVD_TEST]System memory: %d\r\n", memInfo->free_memory);
  
  err = kNoErr;
  mvd_cloud_test_log("MVD Cloud interfaces test [OK]");
  return err;
  
exit:
  mvd_cloud_test_log("MVD Cloud interfaces test [FAILED]");
  return err;
}

/* transmission test */
OSStatus easycloud_transmission_test(mico_Context_t* context)
{
  OSStatus err = kUnknownErr;
  mvd_cloud_test_log("=========== EasyCloud transmission test ===========");
  //11. echo
  
  err = kNoErr;
  mvd_cloud_test_log("MVD Cloud transmission test [OK]");
  return err;
}

/* OTA test */
OSStatus easycloud_ota_test(mico_Context_t* context)
{
  OSStatus err = kUnknownErr;
  mvd_cloud_test_log("=========== EasyCloud OTA test ===========");
  //12. getromversion
  
  //13. getromdata
  
  err = kNoErr;
   mvd_cloud_test_log("MVD Cloud OTA test [OK]");
  return err;
}
