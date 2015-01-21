/**
******************************************************************************
* @file    EasyCloudService.c
* @author  Eshen Wang
* @version V0.2.0
* @date    23-Nov-2014
* @brief   This file contains implementation of EasyCloud service.
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

#include "MICO.h"

#include "JSON-C/json.h"
#include "SocketUtils.h"
#include "HTTPUtils.h"
#include "StringUtils.h"

#include "EasyCloudService.h"
#include "EasyCloudMQTTClient.h"
#include "EasyCloudUtils.h"


//#define  debug_out 

//#ifdef debug_out
//#define  _debug_out debug_out
//#else
#define _debug_out(format, ...) do {;}while(0)

#define easycloud_service_log(M, ...) custom_log("EasyCloudService", M, ##__VA_ARGS__)
#define easycloud_service_log_trace() custom_log_trace("EasyCloudService")
//#endif

/*******************************************************************************
 * DEFINES
 ******************************************************************************/

//cloud request type
typedef enum {
  DEVICE_ACTIVATE = 0,
  DEVICE_AUTHORIZE,
  DEVICE_RESET,
} service_request_type_t;

//cloud request url
#define DEFAULT_DEVICE_ACTIVATE_URL             "/v1/device/activate"
#define DEFAULT_DEVICE_AUTHORIZE_URL            "/v1/device/authorize"
#define DEFAULT_DEVICE_RESET_URL                "/v1/device/reset"
#define DEFAULT_ROM_GET_VERSION_URL             "/v1/rom/lastversion"

#define STACK_SIZE_EASYCLOUD_SERVICE_MAIN_THREAD    0x800

/*******************************************************************************
 * VARIABLES
 ******************************************************************************/

static mico_thread_t easyCloudServiceThreadHandle = NULL;

extern char rom_file_md5[32];

/*******************************************************************************
 * STATIC FUNCTIONS
 ******************************************************************************/

static void easyCloudServiceThread(void *arg);

//==============================================================================
static OSStatus get_rom_data(char *host, uint16_t port,
                             char bin_file[MAX_SIZE_FILE_PATH],
                             char bin_md5[MAX_SIZE_FILE_MD5],
                             uint64_t *out_bin_file_size);

static OSStatus get_rom_version(char *host, uint16_t port, char *request_url,
                                char out_version[MAX_SIZE_FW_VERSION],
                                char out_bin_file[MAX_SIZE_FILE_PATH],
                                char out_bin_md5[MAX_SIZE_FILE_MD5]);

static OSStatus _parseRomVersionResponseMessage(int fd,
                              HTTPHeader_t* inHeader, 
                              char out_version[MAX_SIZE_FW_VERSION],
                              char out_bin_file[MAX_SIZE_FILE_PATH],
                              char out_bin_md5[MAX_SIZE_FILE_MD5]);

static OSStatus _configRomVersionIncommingJsonMessage(const char *input,
                              unsigned int len,
                              char out_version[MAX_SIZE_FW_VERSION],
                              char out_bin_file[MAX_SIZE_FILE_PATH],
                              char out_bin_md5[MAX_SIZE_FILE_MD5]);
//==============================================================================

static OSStatus device_activate_authorize(service_request_type_t request_type, 
                                          char *host,
                                          uint16_t port,
                                          char *request_url, 
                                          char *product_id,
                                          char *bssid, 
                                          char *device_token,
                                          char *user_token, 
                                          char out_device_id[MAX_SIZE_DEVICE_ID], 
                                          char out_master_device_key[MAX_SIZE_DEVICE_KEY]);

static OSStatus _parseResponseMessage(int fd, HTTPHeader_t* inHeader, 
                                      char out_device_id[MAX_SIZE_DEVICE_ID], 
                                      char out_master_device_key[MAX_SIZE_DEVICE_KEY]);

static OSStatus _configIncommingJsonMessage(const char *input,
                                            unsigned int len, 
                                            char out_device_id[MAX_SIZE_DEVICE_ID], 
                                            char out_master_device_key[MAX_SIZE_DEVICE_KEY]);

static OSStatus calculate_device_token(char* bssid, 
                                       char* product_key, 
                                       char out_device_token[32]);

//static OSStatus _cal_user_token(const char* bssid, 
//                                const char* login_id, 
//                                const char * login_passwd, 
//                                unsigned char out_user_token[32]);

/*******************************************************************************
 * EXTERNS
 ******************************************************************************/


/*******************************************************************************
 * IMPLEMENTATIONS
 ******************************************************************************/

OSStatus EasyCloudServiceInit(easycloud_service_context_t* const context)
{
  if (NULL == context){
    return kParamErr;
  }
  
  // init default server config
  strncpy(context->service_config_info.cloudServerDomain, 
          (char*)DEFAULT_CLOUD_SERVER, strlen((char*)DEFAULT_CLOUD_SERVER));
  context->service_config_info.cloudServerPort = DEFAULT_CLOUD_PORT;
  strncpy(context->service_config_info.mqttServerDomain, 
          (char*)DEFAULT_MQTT_SERVER, strlen((char*)DEFAULT_MQTT_SERVER));
  context->service_config_info.mqttServerPort = DEFAULT_MQTT_PORT;
  context->service_config_info.mqttKeepAliveInterval = DEFAULT_MQTT_CLLIENT_KEEPALIVE_INTERVAL;
  
  // init status
  context->service_status.state = EASYCLOUD_STOPPED;
  context->service_status.bin_file_size = 0;
  
  return kNoErr;
}

OSStatus EasyCloudServiceDeInit(easycloud_service_context_t* const context)
{ 
  OSStatus err = kNoErr;
  return err;
}


OSStatus EasyCloudServiceStart(easycloud_service_context_t *context)
{
  if (NULL == context){
    return kParamErr;
  }
  if (EASYCLOUD_STOPPED != context->service_status.state){
    return kAlreadyInUseErr;
  }
 
  return mico_rtos_create_thread(&easyCloudServiceThreadHandle, 
                                 MICO_APPLICATION_PRIORITY, 
                                 "EasyCloud service", easyCloudServiceThread, 
                                 STACK_SIZE_EASYCLOUD_SERVICE_MAIN_THREAD, 
                                 (void*)context);
}


OSStatus EasyCloudServiceStop(easycloud_service_context_t *context)
{
  OSStatus err = kNoErr;
  
  if (NULL == context){
    return kParamErr;
  }
  if (EASYCLOUD_STOPPED == context->service_status.state){
    return kNotInUseErr;
  }

  if (NULL != easyCloudServiceThreadHandle) {
    err = mico_rtos_thread_join(&easyCloudServiceThreadHandle);
    if (kNoErr != err)
      return err;
  }
  
  if (NULL != easyCloudServiceThreadHandle) {
    err = mico_rtos_delete_thread(&easyCloudServiceThreadHandle);
    if (kNoErr != err){
      return err;
    }
    easyCloudServiceThreadHandle = NULL;
  }
  
  context->service_status.state = EASYCLOUD_STOPPED;
  easycloud_service_log("EasyCloud service stopped.");
  
  return err;
}


easycloud_service_state_t EasyCloudServiceState(easycloud_service_context_t *context)
{
  if (NULL == context){
    return EASYCLOUD_STOPPED;
  }
  
  return context->service_status.state;
}


OSStatus EasyCloudPublish(easycloud_service_context_t *context, 
                          const unsigned char *msg, unsigned int msgLen)
{
  int ret = kUnknownErr;
  //char *pubtopic = mqtt_client_config_info.pubtopic;

  if (NULL == context || NULL == msg || 0 == msgLen){
    return kParamErr;
  }
  
  if (EASYCLOUD_CONNECTED != context->service_status.state){
    return kStateErr;
  }
  
  ret = EasyCloudMQTTClientPublish(msg, msgLen);
  return ret;
}

OSStatus EasyCloudPublishto(easycloud_service_context_t* const context, 
                            const char* topic, 
                            const unsigned char *msg, unsigned int msgLen)
{
  int ret = kUnknownErr;
  //char *pubtopic = mqtt_client_config_info.pubtopic;

  if (NULL == context || topic == NULL || NULL == msg || 0 == msgLen){
    return kParamErr;
  }
  
  if (EASYCLOUD_CONNECTED != context->service_status.state){
    return kStateErr;
  }
  
  ret = EasyCloudMQTTClientPublishto(topic, msg, msgLen);
  return ret;
}

OSStatus EasyCloudPublishtoChannel(easycloud_service_context_t* const context, 
                            const char* channel, 
                            const unsigned char *msg, unsigned int msgLen)
{
  int ret = kUnknownErr;
  //char *pubtopic = mqtt_client_config_info.pubtopic;

  if (NULL == context || NULL == channel || NULL == msg || 0 == msgLen){
    return kParamErr;
  }
  
  if (EASYCLOUD_CONNECTED != context->service_status.state){
    return kStateErr;
  }
  
  ret = EasyCloudMQTTClientPublishtoChannel(channel, msg, msgLen);
  return ret;
}


OSStatus EasyCloudActivate(easycloud_service_context_t *context)
{
  OSStatus err = kUnknownErr;
  char device_token[MAX_SIZE_DEVICE_TOKEN] = {0};
  
  if (NULL == context){
    return kParamErr;
  }
  
  //cal device_token = MD5(bssid + product_key)
  err = calculate_device_token(context->service_config_info.bssid, 
                               context->service_config_info.productKey,
                               device_token);
  require_noerr( err, exit);
  easycloud_service_log("activate: device_token[%d]=%s",
                        strlen(device_token), device_token);
  
  //activate
  err = device_activate_authorize(DEVICE_ACTIVATE, 
                                  context->service_config_info.cloudServerDomain,
                                  context->service_config_info.cloudServerPort,
                                  (char *)DEFAULT_DEVICE_ACTIVATE_URL,
                                  context->service_config_info.productId, 
                                  context->service_config_info.bssid,
                                  device_token,
                                  context->service_config_info.userToken,
                                  context->service_status.deviceId, 
                                  context->service_status.masterDeviceKey);
  require_noerr( err, exit);
  context->service_status.isActivated = true;
  return kNoErr;
  
exit:
  return err; 
}


OSStatus EasyCloudAuthorize(easycloud_service_context_t *context)
{
  OSStatus err = kUnknownErr;
  char device_token[MAX_SIZE_DEVICE_TOKEN] = {0};
  
  if (NULL == context){
    return kParamErr;
  }
  
  //cal device_token = MD5(bssid + product_key)
  err = calculate_device_token(context->service_config_info.bssid,
                               context->service_config_info.productKey,
                               device_token);
  require_noerr(err, exit);
  easycloud_service_log("authorize: device_token[%d]=%s",
                        strlen(device_token), device_token);
  
  //authorize
  err = device_activate_authorize(DEVICE_AUTHORIZE, 
                                  context->service_config_info.cloudServerDomain,
                                  context->service_config_info.cloudServerPort,
                                  (char *)DEFAULT_DEVICE_AUTHORIZE_URL,
                                  context->service_config_info.productId,
                                  context->service_config_info.bssid,
                                  device_token,
                                  context->service_config_info.userToken,
                                  context->service_status.deviceId,
                                  context->service_status.masterDeviceKey);
  require_noerr(err, exit);
  context->service_status.isActivated = true;
  
  return kNoErr;
  
exit:
  return err;
}


OSStatus EasyCloudGetLatestRomVersion(easycloud_service_context_t* const context)
{
  OSStatus err = kUnknownErr;
  
  //set purl = URL?product_id=xxxxx
  char *purl = NULL;
  unsigned int purlLen = 0;
  char *param = NULL;
  char *paramH = "product_id=";
  unsigned int productIdLen = strlen(context->service_config_info.productId);
  unsigned int paramLen = strlen(paramH) + productIdLen;
  
  param = (char*)malloc(paramLen);
  if (NULL == param){
    goto exit;
  }
  memset(param, 0, paramLen);
  strncpy(param, paramH, strlen(paramH));
  strncat(param, context->service_config_info.productId, productIdLen);
  
  purlLen = strlen((char *)DEFAULT_ROM_GET_VERSION_URL) + 1 + strlen(param);
  purl = (char*)malloc(purlLen);
  if (NULL == purl){
    goto exit;
  }
  memset(purl, 0, purlLen);
  strncpy(purl, (char *)DEFAULT_ROM_GET_VERSION_URL, strlen((char *)DEFAULT_ROM_GET_VERSION_URL));
  strncat(purl, "?", 1);
  strncat(purl, param, paramLen);
  
  easycloud_service_log("EasyCloud update firmware, url=[%s]", purl);
  
  err = get_rom_version(context->service_config_info.cloudServerDomain,
                        context->service_config_info.cloudServerPort,
                        purl,
                        context->service_status.latestRomVersion,
                        context->service_status.bin_file,
                        context->service_status.bin_md5);

exit:
  if(NULL != param){
    free(param);
    param = NULL;
  }
  if(NULL != purl){
    free(purl);
    purl = NULL;
  }
  return err;
}

OSStatus EasyCloudGetRomData(easycloud_service_context_t* const context)
{
  OSStatus err = kUnknownErr;
  err = get_rom_data(context->service_config_info.cloudServerDomain,
                     context->service_config_info.cloudServerPort,
                     context->service_status.bin_file,
                     context->service_status.bin_md5,
                     &(context->service_status.bin_file_size));
    
  return err;
}

OSStatus EasyCloudDeviceReset(easycloud_service_context_t* const context)
{
  OSStatus err = kUnknownErr;
  char device_token[MAX_SIZE_DEVICE_TOKEN] = {0};
  
  if (NULL == context){
    return kParamErr;
  }
  
  int remoteTcpClient_fd = -1;
  char ipstr[16];
  struct sockaddr_t addr;
  fd_set readfds;
  
  struct timeval_t t;
  t.tv_sec = 3;
  t.tv_usec = 0;
  
  /* create device reset http request data */
  uint8_t *httpRequestData = NULL;
  size_t httpRequestDataLen = 0;
  HTTPHeader_t *httpHeader = NULL;
  
  char *json_str = NULL;
  size_t json_str_len = 0;
  json_object *object;
  
  //cal device_token = MD5(bssid + product_key)
  err = calculate_device_token(context->service_config_info.bssid,
                               context->service_config_info.productKey,
                               device_token);
  require_noerr(err, exit);
  easycloud_service_log("authorize: device_token[%d]=%s",
                        strlen(device_token), device_token);
  
  object = json_object_new_object();
  require_action(object, exit, err = kNoMemoryErr);
  json_object_object_add(object, "product_id", 
                         json_object_new_string(context->service_config_info.productId)); 
  json_object_object_add(object, "bssid", 
                         json_object_new_string(context->service_config_info.bssid)); 
  json_object_object_add(object, "device_token",
                         json_object_new_string(device_token)); 
  //json_object_object_add(object, "encrypt_method", json_object_new_string(encrypt_method));
  
  json_str = (char*)json_object_to_json_string(object);
  json_str_len = strlen(json_str);
  easycloud_service_log("json_str=%s", json_str);
  
  httpHeader = HTTPHeaderCreate();
  require_action( httpHeader, exit, err = kNoMemoryErr );
  HTTPHeaderClear( httpHeader );
  
  //create tcp connect
  easycloud_service_log("tcp client start to connect...");
  err = gethostbyname(context->service_config_info.cloudServerDomain, (uint8_t *)ipstr, 16);
  require_noerr(err, exit);
  easycloud_service_log("cloud service host:%s, ip: %s", context->service_config_info.cloudServerDomain, ipstr);
  
  remoteTcpClient_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  require(remoteTcpClient_fd != -1, exit);
  
  addr.s_ip = inet_addr(ipstr); 
  addr.s_port = context->service_config_info.cloudServerPort;
  
  err = connect(remoteTcpClient_fd, &addr, sizeof(addr));
  require_noerr_quiet(err, exit);
  
  easycloud_service_log("EasyCloud server connected at port=%d, fd=%d", 
                         context->service_config_info.cloudServerPort,
                         remoteTcpClient_fd);
  
  // send request data
  easycloud_service_log("tcp client send activate request...");
  err = CreateHTTPMessageEx(kHTTPPostMethod, 
                            context->service_config_info.cloudServerDomain,
                            (char*)DEFAULT_DEVICE_RESET_URL,
                            kMIMEType_JSON, 
                            (uint8_t*)json_str, json_str_len,
                            &httpRequestData, &httpRequestDataLen);
  require_noerr( err, exit );
  easycloud_service_log("send http package: len=%d,\r\n%s", httpRequestDataLen, httpRequestData);
  
  err = SocketSend( remoteTcpClient_fd, httpRequestData, httpRequestDataLen );
  if (httpRequestData != NULL) {
    free(httpRequestData);
    httpRequestDataLen = 0;
  }
  require_noerr( err, exit );
  
  // get http response
  FD_ZERO(&readfds);
  FD_SET(remoteTcpClient_fd, &readfds);
  err = select(1, &readfds, NULL, NULL, &t);
  require(err >= 1, exit);
  //easycloud_service_log("select return ok.");
  
  if (FD_ISSET(remoteTcpClient_fd, &readfds)) {
    err = SocketReadHTTPHeader( remoteTcpClient_fd, httpHeader );             
    switch ( err )
    {
    case kNoErr:
      //easycloud_service_log("read httpheader OK!");
      //easycloud_service_log("httpHeader->buf:\r\n%s", httpHeader->buf);
      
      // statusCode check
      if(kStatusOK != httpHeader->statusCode){
        easycloud_service_log("ERROR: server response statusCode=%d", httpHeader->statusCode);
        err = kRequestErr;
        goto exit;
      }
      
      // Read the rest of the HTTP body if necessary
      err = SocketReadHTTPBody( remoteTcpClient_fd, httpHeader );
      require_noerr(err, exit);
      easycloud_service_log("read httpBody OK!");
      easycloud_service_log("httpHeader->buf:\r\n%s", httpHeader->buf);
      
      // parse response status
      if (kStatusOK != httpHeader->statusCode){
        err = kResponseErr;
        goto exit;
      }
      
      goto exit_success;
      break;
      
    case EWOULDBLOCK:
      easycloud_service_log("ERROR: read blocked!");
      // NO-OP, keep reading
      goto exit;
      break;
      
    case kNoSpaceErr:
      easycloud_service_log("ERROR: Cannot fit HTTPHeader.");
      goto exit;
      break;
      
    case kConnectionErr:
      // NOTE: kConnectionErr from SocketReadHTTPHeader means it's closed
      easycloud_service_log("ERROR: Connection closed.");
      goto exit;
      break;
      
    default:
      easycloud_service_log("ERROR: HTTP Header parse internal error: %d", err);
      goto exit; 
    }    
  }

exit_success:
  HTTPHeaderClear( httpHeader );
  if(httpHeader) free(httpHeader);
  if(remoteTcpClient_fd != -1){
    close(remoteTcpClient_fd);
    remoteTcpClient_fd = -1;
  }
  if(NULL != object){
    json_object_put(object);
    object = NULL;
  }
  return kNoErr;
  
exit:
  easycloud_service_log("Exit: EasyCloud tcp client exit with err = %d", err);
  HTTPHeaderClear( httpHeader );
  if(httpHeader) free(httpHeader);
  if(remoteTcpClient_fd != -1){
    close(remoteTcpClient_fd);
    remoteTcpClient_fd = -1;
  }
  if(NULL != object){
    json_object_put(object);
    object = NULL;
  }
  
  return err;
}


/* cloud service main thread */
void easyCloudServiceThread(void *arg)
{
  easycloud_service_context_t* easyCloudContext = (easycloud_service_context_t*)arg;
  OSStatus err = kUnknownErr;

  char subscribe_topic[MAX_SIZE_SUBSCRIBE_TOPIC] = {0};
  char publish_topic[MAX_SIZE_PUBLISH_TOPIC] = {0};
  mqtt_client_config_t mqtt_client_config_info = {0};
  
  easyCloudContext->service_status.state = EASYCLOUD_STARTED;
  easycloud_service_log("service start...");
  
  /* 1. Device active or authorize
   *    use easyCloudContext->service_config_info to get [device_id:device_key], 
   *    which will be used to set MQTT client (mqtt_client_config_info)
   */
  easycloud_service_log("service wait for activate...");
  while(1){
    if(EASYCLOUD_STOPPED == easyCloudContext->service_status.state){
      goto exit;
    }
    if (easyCloudContext->service_status.isActivated){
      break;
    }
    else{
      mico_thread_sleep(1);
    }
  }
  easycloud_service_log("device activated.");
  
  /* 2. set publish && subscribe topic for MQTT client
   *      subscribe_topic = device_id/in/#
   *      publish_topic  = device_id/out  (default)
   */
  memset(subscribe_topic, 0, sizeof(subscribe_topic));
  strncpy(subscribe_topic,
          easyCloudContext->service_status.deviceId,
          strlen(easyCloudContext->service_status.deviceId));
  strncat(subscribe_topic, "/in/#", 5);
  
  memset(publish_topic, 0, sizeof(publish_topic));
  strncpy(publish_topic,
          easyCloudContext->service_status.deviceId,
          strlen(easyCloudContext->service_status.deviceId));
  strncat(publish_topic, "/out", 4);
  
  easycloud_service_log("subscribe_topic=%s\tpublish_topic=%s", subscribe_topic, publish_topic);
  
  /* 3. start MQTT client */
ReStartMQTTClient:
  easycloud_service_log("start MQTT client...");
  mqtt_client_config_info.host = easyCloudContext->service_config_info.mqttServerDomain;
  mqtt_client_config_info.port = easyCloudContext->service_config_info.mqttServerPort;
  mqtt_client_config_info.keepAliveInterval = easyCloudContext->service_config_info.mqttKeepAliveInterval;
  mqtt_client_config_info.client_id = easyCloudContext->service_config_info.bssid;
  mqtt_client_config_info.subscribe_qos = QOS2;  //here for subscribe qos
  mqtt_client_config_info.username = easyCloudContext->service_status.masterDeviceKey;
  mqtt_client_config_info.password = "null";   //server not check temporary
  mqtt_client_config_info.subtopic = subscribe_topic;
  mqtt_client_config_info.pubtopic = publish_topic;
  mqtt_client_config_info.hmsg = easyCloudContext->service_config_info.msgRecvhandler;  //msg recv user handler
  mqtt_client_config_info.context = easyCloudContext->service_config_info.context;
  
  EasyCloudMQTTClientInit(mqtt_client_config_info);
  err = EasyCloudMQTTClientStart();
  require_noerr( err, exit );
  
  /* 3. wait for MQTT client start up. */
  easycloud_service_log("wait for MQTT client connect...");
  while(1){
    if(EASYCLOUD_STOPPED == easyCloudContext->service_status.state){
      goto cloud_service_stop;
    }
    
    if(MQTT_CLIENT_STATUS_CONNECTED == EasyCloudMQTTClientState()){
      break;
    }
    else{
      mico_thread_sleep(1);
    }
  }
  easyCloudContext->service_status.state = EASYCLOUD_CONNECTED;
  easycloud_service_log("service connectecd.");
  
  /* 4. cloud service started callback, notify user. */
  easyCloudContext->service_config_info.statusNotify(easyCloudContext->service_config_info.context,
                                                    easyCloudContext->service_status);
    
  /* service loop */
  while(1) {
    if(EASYCLOUD_STOPPED == easyCloudContext->service_status.state){
      goto cloud_service_stop;
    }
    
    //check mqtt client state
    switch(EasyCloudMQTTClientState()){
    case MQTT_CLIENT_STATUS_DISCONNECTED:
      if (EASYCLOUD_DISCONNECTED != easyCloudContext->service_status.state){
        easycloud_service_log("MQTT client disconnected!");
        easyCloudContext->service_status.state = EASYCLOUD_DISCONNECTED;
        easyCloudContext->service_config_info.statusNotify(easyCloudContext->service_config_info.context,
                                                          easyCloudContext->service_status);
      }
      break;
    case MQTT_CLIENT_STATUS_STOPPED:
      if (EASYCLOUD_DISCONNECTED != easyCloudContext->service_status.state){
        easycloud_service_log("MQTT client stopped! try restarting it after 3 seconds...");
        easyCloudContext->service_status.state = EASYCLOUD_DISCONNECTED;
        easyCloudContext->service_config_info.statusNotify(easyCloudContext->service_config_info.context,
                                                          easyCloudContext->service_status);
        mico_thread_sleep(3);
        goto ReStartMQTTClient;
      }
      break;
    case MQTT_CLIENT_STATUS_STARTED:
      if (EASYCLOUD_DISCONNECTED != easyCloudContext->service_status.state){
        easycloud_service_log("MQTT client connecting...");
        easyCloudContext->service_status.state = EASYCLOUD_DISCONNECTED;
        easyCloudContext->service_config_info.statusNotify(easyCloudContext->service_config_info.context,
                                                          easyCloudContext->service_status);
      }
      break;
    case MQTT_CLIENT_STATUS_CONNECTED:
      if (EASYCLOUD_CONNECTED != easyCloudContext->service_status.state){
        easyCloudContext->service_status.state = EASYCLOUD_CONNECTED;
        easycloud_service_log("MQTT client connected!");
        easyCloudContext->service_config_info.statusNotify(easyCloudContext->service_config_info.context,
                                                          easyCloudContext->service_status);
      }
      break;
    default:
      break;
    }
    
    //easycloud_service_log("cloud service runing...");
    mico_thread_sleep(3);
  }
  
cloud_service_stop:
  EasyCloudMQTTClientStop();
  
exit:
  easyCloudContext->service_status.state = EASYCLOUD_STOPPED;
  easyCloudContext->service_config_info.statusNotify(easyCloudContext->service_config_info.context,
                                                     easyCloudContext->service_status);
  easycloud_service_log("Exit: EasyCloud thread exit with err = %d", err);
  mico_rtos_delete_thread(NULL);
  easyCloudServiceThreadHandle = NULL;
  return;
}


static OSStatus device_activate_authorize(service_request_type_t request_type,
                                          char *host,
                                          uint16_t port,
                                          char *request_url,
                                          char *product_id,
                                          char *bssid,
                                          char *device_token,
                                          char *user_token,
                                          char out_device_id[MAX_SIZE_DEVICE_ID],
                                          char out_master_device_key[MAX_SIZE_DEVICE_KEY])
{
  OSStatus err = kUnknownErr;
  
  int remoteTcpClient_fd = -1;
  char ipstr[16];
  struct sockaddr_t addr;
  fd_set readfds;
  
  struct timeval_t t;
  t.tv_sec = 3;
  t.tv_usec = 0;
  
  /* create activate or authorize http request data */
  uint8_t *httpRequestData = NULL;
  size_t httpRequestDataLen = 0;
  HTTPHeader_t *httpHeader = NULL;
  
  char *json_str = NULL;
  size_t json_str_len = 0;
  json_object *object;
  
  object = json_object_new_object();
  require_action(object, exit, err = kNoMemoryErr);
  json_object_object_add(object, "product_id", json_object_new_string(product_id)); 
  json_object_object_add(object, "bssid", json_object_new_string(bssid)); 
  json_object_object_add(object, "device_token", json_object_new_string(device_token)); 
  json_object_object_add(object, "user_token", json_object_new_string(user_token));
  
  json_str = (char*)json_object_to_json_string(object);
  json_str_len = strlen(json_str);
  easycloud_service_log("json_str=%s", json_str);
  
  easycloud_service_log("request type(activated=0, authorize=1) = [%d].", request_type);
  
  httpHeader = HTTPHeaderCreate();
  require_action( httpHeader, exit, err = kNoMemoryErr );
  HTTPHeaderClear( httpHeader );
  
  //create tcp connect
  easycloud_service_log("tcp client start to connect...");
  err = gethostbyname((char *)host, (uint8_t *)ipstr, 16);
  require_noerr(err, exit);
  easycloud_service_log("cloud service host:%s, ip: %s", host, ipstr);
  
  remoteTcpClient_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  require(remoteTcpClient_fd != -1, exit);
  
  addr.s_ip = inet_addr(ipstr); 
  addr.s_port = port;
  
  err = connect(remoteTcpClient_fd, &addr, sizeof(addr));
  require_noerr_quiet(err, exit);
  
  easycloud_service_log("EasyCloud server connected at port=%d, fd=%d", 
                         port,
                         remoteTcpClient_fd);
  
  // send request data
  easycloud_service_log("tcp client send activate request...");
  err = CreateHTTPMessageEx(kHTTPPostMethod, 
                            host, request_url,
                            kMIMEType_JSON, 
                            (uint8_t*)json_str, json_str_len,
                            &httpRequestData, &httpRequestDataLen);
  require_noerr( err, exit );
  easycloud_service_log("send http package: len=%d,\r\n%s", httpRequestDataLen, httpRequestData);
  
  err = SocketSend( remoteTcpClient_fd, httpRequestData, httpRequestDataLen );
  if (httpRequestData != NULL) {
    free(httpRequestData);
    httpRequestDataLen = 0;
  }
  require_noerr( err, exit );
  
  // get http response
  FD_ZERO(&readfds);
  FD_SET(remoteTcpClient_fd, &readfds);
  err = select(1, &readfds, NULL, NULL, &t);
  require(err >= 1, exit);
  //easycloud_service_log("select return ok.");
  
  if (FD_ISSET(remoteTcpClient_fd, &readfds)) {
    err = SocketReadHTTPHeader( remoteTcpClient_fd, httpHeader );             
    switch ( err )
    {
    case kNoErr:
      //easycloud_service_log("read httpheader OK!");
      //easycloud_service_log("httpHeader->buf:\r\n%s", httpHeader->buf);
      
      // statusCode check
      if(kStatusOK != httpHeader->statusCode){
        easycloud_service_log("ERROR: server response statusCode=%d", 
                              httpHeader->statusCode);
        err = kRequestErr;
        goto exit;
      }
      
      // Read the rest of the HTTP body if necessary
      err = SocketReadHTTPBody( remoteTcpClient_fd, httpHeader );
      require_noerr(err, exit);
      easycloud_service_log("read httpBody OK!");
      easycloud_service_log("httpHeader->buf:\r\n%s", httpHeader->buf);
      
      // parse recived extra data to get devicd_id && master_device_key.
      err = _parseResponseMessage( remoteTcpClient_fd, httpHeader, out_device_id, out_master_device_key );
      HTTPHeaderClear(httpHeader);  // Reuse HTTPHeader
      easycloud_service_log("out_device_id=%s", out_device_id);
      easycloud_service_log("out_master_device_key=%s", out_master_device_key);
      require_noerr( err, exit );
      
      easycloud_service_log("device (activated=0, authorize=1) [%d] done!", request_type);
      goto exit_success;
      break;
      
    case EWOULDBLOCK:
      easycloud_service_log("ERROR: read blocked!");
      // NO-OP, keep reading
      goto exit;
      break;
      
    case kNoSpaceErr:
      easycloud_service_log("ERROR: Cannot fit HTTPHeader.");
      goto exit;
      break;
      
    case kConnectionErr:
      // NOTE: kConnectionErr from SocketReadHTTPHeader means it's closed
      easycloud_service_log("ERROR: Connection closed.");
      goto exit;
      break;
      
    default:
      easycloud_service_log("ERROR: HTTP Header parse internal error: %d", err);
      goto exit; 
    }    
  }

exit_success:
  HTTPHeaderClear( httpHeader );
  if(httpHeader) free(httpHeader);
  if(remoteTcpClient_fd != -1){
    close(remoteTcpClient_fd);
    remoteTcpClient_fd = -1;
  }
  if(NULL != object){
    json_object_put(object);
    object = NULL;
  }
  return kNoErr;
  
exit:
  easycloud_service_log("Exit: EasyCloud tcp client exit with err = %d", err);
  HTTPHeaderClear( httpHeader );
  if(httpHeader) free(httpHeader);
  if(remoteTcpClient_fd != -1){
    close(remoteTcpClient_fd);
    remoteTcpClient_fd = -1;
  }
  if(NULL != object){
    json_object_put(object);
    object = NULL;
  }
  
  return err;
}


static OSStatus get_rom_data(char *host, uint16_t port,
                             char bin_file[MAX_SIZE_FILE_PATH],
                             char bin_md5[MAX_SIZE_FILE_MD5],
                             uint64_t *out_bin_file_size)
{
  OSStatus err = kUnknownErr;
  easycloud_service_log("get_rom_data: bin_file=%s", bin_file);
  easycloud_service_log("get_rom_data: bin_md5=%s", bin_md5);
  
  char* request_url = NULL;
  
  int tcpClient_fd = -1;
  char ipstr[16];
  struct sockaddr_t addr;
  fd_set readfds;
  
  // select timeout
  struct timeval_t t;
  t.tv_sec = 3;
  t.tv_usec = 0;
  
  // send/recv timemout
  int retVal = -1;
  int nNetTimeout = 10000;  // 10s
  
  /* create activate or authorize http request data */
  uint8_t *httpRequestData = NULL;
  size_t httpRequestDataLen = 0;
  HTTPHeader_t *httpHeader = NULL;
  
  easycloud_service_log("request: [%s]", bin_file);
  
  httpHeader = HTTPHeaderCreate();
  require_action( httpHeader, exit, err = kNoMemoryErr );
  HTTPHeaderClear( httpHeader );
  
  //create tcp connect
  easycloud_service_log("tcp client start to connect...");
  err = gethostbyname((char *)host, (uint8_t *)ipstr, 16);
  require_noerr(err, exit);
  easycloud_service_log("cloud service host:%s, ip: %s", host, ipstr);
  
  tcpClient_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  require(tcpClient_fd != -1, exit);
  
  retVal = setsockopt(tcpClient_fd, SOL_SOCKET, SO_SNDTIMEO, (char *)&nNetTimeout,sizeof(int));
  if( retVal < 0 ) {
    // error
    easycloud_service_log("setsockopt error:[%d]", retVal);
    goto exit;
  }
  
  retVal = setsockopt(tcpClient_fd, SOL_SOCKET, SO_RCVTIMEO, (char *)&nNetTimeout,sizeof(int));
  if( retVal < 0 ) {
    // error
    easycloud_service_log("setsockopt error:[%d]", retVal);
    goto exit;
  }
  
  addr.s_ip = inet_addr(ipstr); 
  addr.s_port = port;
  
  err = connect(tcpClient_fd, &addr, sizeof(addr));
  require_noerr_quiet(err, exit);
  
  easycloud_service_log("EasyCloud server connected at port=%d, fd=%d", 
                         port,
                         tcpClient_fd);
  
  //strip http://host from bin_file
  request_url = strstr(bin_file, host) + strlen(host);
  
  // send request data
  easycloud_service_log("tcp client send activate request...");
  err = CreateHTTPMessageEx(kHTTPGetMethod, 
                            host, request_url,
                            kMIMEType_JSON, 
                            NULL, 0,
                            &httpRequestData, &httpRequestDataLen);
  require_noerr( err, exit );
  easycloud_service_log("send http package: len=%d,\r\n%s", httpRequestDataLen, httpRequestData);
  
  err = SocketSend( tcpClient_fd, httpRequestData, httpRequestDataLen );
  if (httpRequestData != NULL) {
    free(httpRequestData);
    httpRequestDataLen = 0;
  }
  require_noerr( err, exit );
  
  // get http response
  FD_ZERO(&readfds);
  FD_SET(tcpClient_fd, &readfds);
  err = select(1, &readfds, NULL, NULL, &t);
  require(err >= 1, exit);
  //easycloud_service_log("select return ok.");
  
  if (FD_ISSET(tcpClient_fd, &readfds)) {
    err = SocketReadHTTPHeaderEx( tcpClient_fd, httpHeader );             
    switch ( err )
    {
    case kNoErr:
      //easycloud_service_log("read httpheader OK!");
      easycloud_service_log("httpHeader->buf:\r\n%s", httpHeader->buf);
      
      // statusCode check
      if(kStatusOK != httpHeader->statusCode){
        easycloud_service_log("ERROR: server response statusCode=%d", 
                              httpHeader->statusCode);
        err = kRequestErr;
        goto exit;
      }
      
      // Read the rest of the HTTP body if necessary
      err = SocketReadHTTPBodyEx( tcpClient_fd, httpHeader );
      require_noerr(err, exit);
      
      easycloud_service_log("read httpBody OK!");
      //easycloud_service_log("httpHeader->buf:\r\n%s", httpHeader->buf);
      HTTPHeaderClear(httpHeader);  // Reuse HTTPHeader
      
      // check md5
      if(0 != strncmp(bin_md5, rom_file_md5, strlen(bin_md5))){
        easycloud_service_log("ERROR: rom md5 checksum err!!");
        err = kChecksumErr;
        goto exit;
      }
      //return file size
      *out_bin_file_size = httpHeader->contentLength;
      
      //HTTPHeaderClear(httpHeader);  // Reuse HTTPHeader
      //require_noerr( err, exit );
      
      easycloud_service_log("get rom data done!");
      goto exit_success;
      break;
      
    case EWOULDBLOCK:
      easycloud_service_log("ERROR: read blocked!");
      // NO-OP, keep reading
      goto exit;
      break;
      
    case kNoSpaceErr:
      easycloud_service_log("ERROR: Cannot fit HTTPHeader.");
      goto exit;
      break;
      
    case kConnectionErr:
      // NOTE: kConnectionErr from SocketReadHTTPHeader means it's closed
      easycloud_service_log("ERROR: Connection closed.");
      goto exit;
      break;
      
    default:
      easycloud_service_log("ERROR: HTTP Header parse internal error: %d", err);
      goto exit; 
    }    
  }

exit_success:
  HTTPHeaderClear( httpHeader );
  if(httpHeader) free(httpHeader);
  if(tcpClient_fd != -1){
    close(tcpClient_fd);
    tcpClient_fd = -1;
  }
  easycloud_service_log("get_rom_data: success!");
  return kNoErr;
  
exit:
  *out_bin_file_size = 0;
  easycloud_service_log("Exit: EasyCloud tcp client exit with err = %d", err);
  HTTPHeaderClear( httpHeader );
  if(httpHeader) free(httpHeader);
  if(tcpClient_fd != -1){
    close(tcpClient_fd);
    tcpClient_fd = -1;
  }
  return err;
}

static OSStatus get_rom_version(char *host, uint16_t port, char *request_url,
                              char out_version[MAX_SIZE_FW_VERSION],
                              char out_bin_file[MAX_SIZE_FILE_PATH],
                              char out_bin_md5[MAX_SIZE_FILE_MD5])
{
  OSStatus err = kUnknownErr;
  
  int tcpClient_fd = -1;
  char ipstr[16];
  struct sockaddr_t addr;
  fd_set readfds;
  
  struct timeval_t t;
  t.tv_sec = 3;
  t.tv_usec = 0;
  
  /* create activate or authorize http request data */
  uint8_t *httpRequestData = NULL;
  size_t httpRequestDataLen = 0;
  HTTPHeader_t *httpHeader = NULL;
  
  easycloud_service_log("request: [%s]", request_url);
  
  httpHeader = HTTPHeaderCreate();
  require_action( httpHeader, exit, err = kNoMemoryErr );
  HTTPHeaderClear( httpHeader );
  
  //create tcp connect
  easycloud_service_log("tcp client start to connect...");
  err = gethostbyname((char *)host, (uint8_t *)ipstr, 16);
  require_noerr(err, exit);
  easycloud_service_log("cloud service host:%s, ip: %s", host, ipstr);
  
  tcpClient_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  require(tcpClient_fd != -1, exit);
  
  addr.s_ip = inet_addr(ipstr); 
  addr.s_port = port;
  
  err = connect(tcpClient_fd, &addr, sizeof(addr));
  require_noerr_quiet(err, exit);
  
  easycloud_service_log("EasyCloud server connected at port=%d, fd=%d", 
                         port,
                         tcpClient_fd);
  
  // send request data
  easycloud_service_log("tcp client send activate request...");
  err = CreateHTTPMessageEx(kHTTPGetMethod, 
                            host, request_url,
                            kMIMEType_JSON, 
                            NULL, 0,
                            &httpRequestData, &httpRequestDataLen);
  require_noerr( err, exit );
  easycloud_service_log("send http package: len=%d,\r\n%s", httpRequestDataLen, httpRequestData);
  
  err = SocketSend( tcpClient_fd, httpRequestData, httpRequestDataLen );
  if (httpRequestData != NULL) {
    free(httpRequestData);
    httpRequestDataLen = 0;
  }
  require_noerr( err, exit );
  
  // get http response
  FD_ZERO(&readfds);
  FD_SET(tcpClient_fd, &readfds);
  err = select(1, &readfds, NULL, NULL, &t);
  require(err >= 1, exit);
  //easycloud_service_log("select return ok.");
  
  if (FD_ISSET(tcpClient_fd, &readfds)) {
    err = SocketReadHTTPHeader( tcpClient_fd, httpHeader );             
    switch ( err )
    {
    case kNoErr:
      //easycloud_service_log("read httpheader OK!");
      //easycloud_service_log("httpHeader->buf:\r\n%s", httpHeader->buf);
      
      // statusCode check
      if(kStatusOK != httpHeader->statusCode){
        easycloud_service_log("ERROR: server response statusCode=%d", 
                              httpHeader->statusCode);
        err = kRequestErr;
        goto exit;
      }
      
      // Read the rest of the HTTP body if necessary
      err = SocketReadHTTPBody( tcpClient_fd, httpHeader );
      require_noerr(err, exit);
      easycloud_service_log("read httpBody OK!");
      easycloud_service_log("httpHeader->buf:\r\n%s", httpHeader->buf);
      
      // parse recived extra data to get devicd_id && master_device_key.
      err = _parseRomVersionResponseMessage( tcpClient_fd, httpHeader, 
                                            out_version, 
                                            out_bin_file, out_bin_md5 );
      HTTPHeaderClear(httpHeader);  // Reuse HTTPHeader
      require_noerr( err, exit );
      
      easycloud_service_log("get rom version done!");
      goto exit_success;
      break;
      
    case EWOULDBLOCK:
      easycloud_service_log("ERROR: read blocked!");
      // NO-OP, keep reading
      goto exit;
      break;
      
    case kNoSpaceErr:
      easycloud_service_log("ERROR: Cannot fit HTTPHeader.");
      goto exit;
      break;
      
    case kConnectionErr:
      // NOTE: kConnectionErr from SocketReadHTTPHeader means it's closed
      easycloud_service_log("ERROR: Connection closed.");
      goto exit;
      break;
      
    default:
      easycloud_service_log("ERROR: HTTP Header parse internal error: %d", err);
      goto exit; 
    }    
  }

exit_success:
  HTTPHeaderClear( httpHeader );
  if(httpHeader) free(httpHeader);
  if(tcpClient_fd != -1){
    close(tcpClient_fd);
    tcpClient_fd = -1;
  }
  return kNoErr;
  
exit:
  easycloud_service_log("Exit: EasyCloud tcp client exit with err = %d", err);
  HTTPHeaderClear( httpHeader );
  if(httpHeader) free(httpHeader);
  if(tcpClient_fd != -1){
    close(tcpClient_fd);
    tcpClient_fd = -1;
  }
  
  return err;
}


static OSStatus _parseResponseMessage(int fd,
                                      HTTPHeader_t* inHeader, 
                                      char out_device_id[MAX_SIZE_DEVICE_ID],
                                      char out_master_device_key[MAX_SIZE_DEVICE_KEY])
{
    OSStatus err = kUnknownErr;
    const char *        value;
    size_t              valueSize;

    easycloud_service_log_trace();

    switch(inHeader->statusCode){
      case kStatusOK:
        //easycloud_service_log("cloud server respond activate status OK!");
        //get content-type for json format data
        err = HTTPGetHeaderField(inHeader->buf, 
                                 inHeader->len, 
                                 "Content-Type", 
                                 NULL, NULL, 
                                 &value, &valueSize,
                                 NULL );
        require_noerr(err, exit);
          
        if( strnicmpx( value, strlen(kMIMEType_JSON), kMIMEType_JSON ) == 0 ){
          //easycloud_service_log("JSON data received!");
          // parse json data
          err = _configIncommingJsonMessage(inHeader->extraDataPtr,
                                            inHeader->extraDataLen,
                                            out_device_id,
                                            out_master_device_key);
          require_noerr( err, exit );
          return kNoErr;
        }
        else{
          return kUnsupportedDataErr;
        }
      break;
      
      default:
        goto exit;
    }

 exit:
    return err;
}


static OSStatus _configIncommingJsonMessage(const char *input,
                                            unsigned int len,
                                            char out_device_id[MAX_SIZE_DEVICE_ID],
                                            char out_master_device_key[MAX_SIZE_DEVICE_KEY])
{
  easycloud_service_log_trace();
  OSStatus err = kUnknownErr;
  json_object *new_obj = NULL;

  //easycloud_service_log("Recv json data input=%s", input);
  new_obj = json_tokener_parse(input);
  //easycloud_service_log("Recv json data=%s", json_object_to_json_string(new_obj));
  require_action(new_obj, exit, err = kUnknownErr);
  //easycloud_service_log("Recv json config object=%s", json_object_to_json_string(new_obj));

  //save each key-value in json data for system.
  json_object_object_foreach(new_obj, key, val) {
    if(!strcmp(key, "device_id")){
      memset((char *)out_device_id, 0, strlen(out_device_id));
      strncpy((char*)out_device_id, json_object_get_string(val), MAX_SIZE_DEVICE_ID);
      //easycloud_service_log("get out_device_id[%d]=%s", strlen(out_device_id), out_device_id);
    }else if(!strcmp(key, "master_device_key")){
      memset((char *)out_master_device_key, 0, strlen(out_master_device_key));
      strncpy(out_master_device_key, json_object_get_string(val), MAX_SIZE_DEVICE_KEY);
      //easycloud_service_log("get out_master_device_key[%d]=%s", strlen(out_master_device_key), out_master_device_key);
    }
    else {
    }
  }

  //free unused memory
  json_object_put(new_obj);
  return kNoErr;

exit:
  return err; 
}

static OSStatus _parseRomVersionResponseMessage(int fd,
                              HTTPHeader_t* inHeader, 
                              char out_version[MAX_SIZE_FW_VERSION],
                              char out_bin_file[MAX_SIZE_FILE_PATH],
                              char out_bin_md5[MAX_SIZE_FILE_MD5])
{
    OSStatus err = kUnknownErr;
    const char *        value;
    size_t              valueSize;

    easycloud_service_log_trace();

    switch(inHeader->statusCode){
      case kStatusOK:
        //easycloud_service_log("cloud server respond activate status OK!");
        //get content-type for json format data
        err = HTTPGetHeaderField(inHeader->buf, 
                                 inHeader->len, 
                                 "Content-Type", 
                                 NULL, NULL, 
                                 &value, &valueSize,
                                 NULL );
        require_noerr(err, exit);
          
        if( strnicmpx( value, strlen(kMIMEType_JSON), kMIMEType_JSON ) == 0 ){
          //easycloud_service_log("JSON data received!");
          // parse json data
          err = _configRomVersionIncommingJsonMessage(inHeader->extraDataPtr,
                                            inHeader->extraDataLen,
                                            out_version, out_bin_file,
                                            out_bin_md5);
          require_noerr( err, exit );
          return kNoErr;
        }
        else{
          return kUnsupportedDataErr;
        }
      break;
      
      default:
        goto exit;
    }

 exit:
    return err;
}


static OSStatus _configRomVersionIncommingJsonMessage(const char *input,
                              unsigned int len,
                              char out_version[MAX_SIZE_FW_VERSION],
                              char out_bin_file[MAX_SIZE_FILE_PATH],
                              char out_bin_md5[MAX_SIZE_FILE_MD5])
{
  easycloud_service_log_trace();
  OSStatus err = kUnknownErr;
  json_object *new_obj = NULL;

  //easycloud_service_log("Recv json data input=%s", input);
  new_obj = json_tokener_parse(input);
  //easycloud_service_log("Recv json data=%s", json_object_to_json_string(new_obj));
  require_action(new_obj, exit, err = kUnknownErr);
  //easycloud_service_log("Recv json config object=%s", json_object_to_json_string(new_obj));

  //save each key-value in json data for system.
  json_object_object_foreach(new_obj, key, val) {
    if(!strcmp(key, "version")){
      memset((char *)out_version, 0, strlen(out_version));
      strncpy((char*)out_version, json_object_get_string(val), MAX_SIZE_FW_VERSION);
      //easycloud_service_log("get out_version[%d]=%s", strlen(out_version), out_version);
    }
    else if(!strcmp(key, "bin")){
      memset((char *)out_bin_file, 0, strlen(out_bin_file));
      strncpy(out_bin_file, json_object_get_string(val), MAX_SIZE_FILE_PATH);
      //easycloud_service_log("get out_bin_file[%d]=%s", strlen(out_bin_file), out_bin_file);
    }
    else if(!strcmp(key, "bin_md5")){
      memset((char *)out_bin_md5, 0, strlen(out_bin_md5));
      strncpy(out_bin_md5, json_object_get_string(val), MAX_SIZE_FILE_MD5);
      //easycloud_service_log("get out_bin_md5[%d]=%s", strlen(out_bin_md5), out_bin_md5);
    }
    else if(!strcmp(key, "error")){
      err = kNotFoundErr;
    }
    else {
    }
  }

  //free unused memory
  json_object_put(new_obj);
  return err;

exit:
  return err; 
}


/* brief: calculate device_token = MD5(bssid + product_id)
 * input:
 *    bssid + product_key
 * output:
 *    device_token
 * return:
 *     return kNoErr if success
 */
static OSStatus calculate_device_token(char *bssid, char *product_key,
                                       char out_device_token[32])
{
  md5_context md5;
  unsigned char *md5_input = NULL;
  unsigned char device_token_16[16];
  char *ptoken32 = NULL;
  unsigned int input_len = 0;
  
  if ((NULL == bssid) || (NULL == product_key) || NULL == out_device_token)
    return kParamErr;
  memset(out_device_token, 0, strlen(out_device_token));
  
  //easycloud_service_log("bssid[%d]=%s", strlen(bssid), bssid);
  //easycloud_service_log("product_key[%d]=%s", strlen(product_key), product_key);

  input_len = strlen(bssid) + strlen(product_key) + 1;
  md5_input = (unsigned char *)malloc(input_len);
  if (NULL == md5_input)
    return kNoMemoryErr;
  memset(md5_input, 0, input_len);
  md5_input[input_len] = '\0';
  
  memcpy(md5_input, bssid, strlen(bssid));
  memcpy((md5_input + strlen(bssid)), product_key, strlen(product_key));
  //easycloud_service_log("md5_input[%d]=%s", strlen((char*)md5_input), (unsigned char*)md5_input);
  
  InitMd5(&md5);
  Md5Update(&md5, md5_input, strlen((char*)md5_input));
  Md5Final(&md5, device_token_16);
  //easycloud_service_log("device_token_16[%d]=%s", sizeof(device_token_16), device_token_16);
  if (NULL != md5_input){
    free(md5_input);
    md5_input = NULL;
  }
  
  //convert hex data to hex string
  ptoken32 = DataToHexStringLowercase(device_token_16,  sizeof(device_token_16));
  //easycloud_service_log("out_device_token[%d]=%s", strlen(ptoken32), ptoken32);
  
  if (NULL != ptoken32){
    strncpy(out_device_token, ptoken32, strlen(ptoken32));
    free(ptoken32);
    ptoken32 = NULL;
  }
  else
    return kNoMemoryErr;
  
  //easycloud_service_log("out_device_token[%d]=%s", strlen(out_device_token), out_device_token);
  
  return kNoErr;
}


///* brief: calculate user_token = MD5(bssid + login_id + login_passwd)
// * input:
// *    bssid + login_id + login_passwd
// * output:
// *    user_token
// * return:
// *     return kNoErr if success
// */
//static OSStatus _cal_user_token(const char* bssid, const char* login_id, const char * login_passwd, unsigned char out_user_token[16])
//{
//   OSStatus err = kNoErr;
//   return err;
//}


