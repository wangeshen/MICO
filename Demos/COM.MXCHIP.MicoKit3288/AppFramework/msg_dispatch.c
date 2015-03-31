/**
  ******************************************************************************
  * @file    msg_dispatch.c 
  * @author  Eshen Wang
  * @version V0.1.0
  * @date    18-Mar-2015
  * @brief   fogclud msg dispatch.
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
//#include "MICOAppDefine.h"
//#include "user_main.h"
//#include "MicoFogCloud.h"
#include "msg_dispatch.h"
#include "JSON-C/json.h"
#include "user_uart.h"
#include "properties.h"
#include "MicoFogCloud.h"

#define msg_dispatch_log(M, ...) custom_log("MSG_DISPATCH", M, ##__VA_ARGS__)
#define msg_dispatch_log_trace() custom_log_trace("MSG_DISPATCH")

// global device service table
extern struct mico_service_t  service_table[];

// override by user customized topic message handler in user_main.c
WEAK OSStatus user_customized_topic_msg_handler(mico_Context_t* context, 
                                           const char* topic, const unsigned int topicLen,
                                           unsigned char *inBuf, unsigned int inBufLen)
{
  OSStatus err = kNoErr;
  msg_dispatch_log("WARNING: message from user customized topic unhandled!\r\ntopic=%.*s\tmsg=%.*s",
                   topicLen, topic,  inBufLen, inBuf);
  
  return err;
}

// handle cloud msg here, for example: send to USART or echo to cloud
OSStatus mico_cloudmsg_dispatch(mico_Context_t* context, mico_fogcloud_msg_t *cloud_msg)
{
  msg_dispatch_log_trace();
  OSStatus err = kNoErr;
  char* recv_sub_topic_ptr = NULL;
  int recv_sub_topic_len = 0;  
  json_object *recv_json_object = NULL;
  char* response_sub_topic = NULL;
  json_object *response_json_obj = NULL;
  const char *response_json_string = NULL;
  enum mico_prop_sub_type_t read_type = MICO_PROP_SUB_TYPE_UNUSED;
  
  if((NULL == context) || (NULL == cloud_msg->topic) || (0 == cloud_msg->topic_len) ) {
       return kParamErr;
  }
  
  // strip "<device_id>/in"
  recv_sub_topic_ptr = (char*)(cloud_msg->topic) + strlen(context->flashContentInRam.appConfig.fogcloudConfig.deviceId) +3;
  recv_sub_topic_len = (int)cloud_msg->topic_len - (strlen(context->flashContentInRam.appConfig.fogcloudConfig.deviceId) +3);
//  response_sub_topic = ECS_str_replace(response_sub_topic, recv_sub_topic_ptr, 
//                                     recv_sub_topic_len, "/in/", "/out/");
  response_sub_topic = (char*)malloc(recv_sub_topic_len);   // response to where msg come from, remove leading '/'
  if(NULL == response_sub_topic){
    msg_dispatch_log("get reponse topic err!");
    err = kUnsupportedErr;
    goto exit;
  }
  memset(response_sub_topic, '\0', recv_sub_topic_len);
  strncpy(response_sub_topic, recv_sub_topic_ptr + 1, recv_sub_topic_len-1);  // remove leading '/' as send sub-topic
  msg_dispatch_log("recv_sub_topic[%d]=[%.*s]", recv_sub_topic_len, recv_sub_topic_len, recv_sub_topic_ptr);  
  msg_dispatch_log("response_sub_topic[%d]=[%s]", strlen(response_sub_topic), response_sub_topic);  
  
  // parse sub topic string
  if( 0 == strncmp((char*)FOGCLOUD_MSG_TOPIC_IN_READ, recv_sub_topic_ptr, strlen((char*)FOGCLOUD_MSG_TOPIC_IN_READ)) ){
    // from /read
    msg_dispatch_log("Recv from: %.*s, data[%d]: %s",
                     recv_sub_topic_len, recv_sub_topic_ptr,
                     cloud_msg->data_len, cloud_msg->data);
    
    // read properties
    recv_json_object = json_tokener_parse((const char*)(cloud_msg->data));
    require_action(recv_json_object, exit, err = kFormatErr);
    msg_dispatch_log("Recv read object=%s", json_object_to_json_string(recv_json_object));
    //create response json object
    // strip /read, get '/value' or '/event'
    if(0 == strncmp((char*)FOGCLOUD_MSG_TOPIC_IN_SUB_VALUE, recv_sub_topic_ptr + strlen(FOGCLOUD_MSG_TOPIC_IN_READ), 
                    strlen((char*)FOGCLOUD_MSG_TOPIC_IN_SUB_VALUE))){
      read_type = MICO_PROP_SUB_TYPE_VALUE;
    }
    else if(0 == strncmp((char*)FOGCLOUD_MSG_TOPIC_IN_SUB_EVENT, recv_sub_topic_ptr + strlen(FOGCLOUD_MSG_TOPIC_IN_READ), 
                         strlen((char*)FOGCLOUD_MSG_TOPIC_IN_SUB_EVENT))){
      read_type = MICO_PROP_SUB_TYPE_EVENT;
    }
    else {
      msg_dispatch_log("ERROR: unsupported sub topic: %s", recv_sub_topic_ptr);
      err = kUnsupportedErr;
      goto exit;
    }
            
    response_json_obj = mico_read_properties(service_table, recv_json_object, read_type);
    
    // send reponse for read data
    if(NULL == response_json_obj){
      msg_dispatch_log("ERROR: read properties error!");
      json_object_object_add(response_json_obj, "MICO_PROP_READ_STATUS", json_object_new_int(MICO_PROP_CODE_READ_FAILED));
      err = kReadErr;
    }
    response_json_string = json_object_to_json_string(response_json_obj);
    err = MicoFogCloudMsgSend(context, response_sub_topic, 
                              (unsigned char*)response_json_string, strlen(response_json_string));
  }
  else if( 0 == strncmp((char*)FOGCLOUD_MSG_TOPIC_IN_WRITE, recv_sub_topic_ptr, strlen((char*)FOGCLOUD_MSG_TOPIC_IN_WRITE)) ){
    // from /write
    msg_dispatch_log("Recv from: %.*s, data[%d]: %s",
                     recv_sub_topic_len, recv_sub_topic_ptr,
                     cloud_msg->data_len, cloud_msg->data);
    
    // write properties
    recv_json_object = json_tokener_parse((const char*)(cloud_msg->data));
    require_action(recv_json_object, exit, err = kFormatErr);
    msg_dispatch_log("Recv read object=%s", json_object_to_json_string(recv_json_object));
    //create response json object
    // strip '/write', get '/value' or '/event'
    if(0 == strncmp((char*)FOGCLOUD_MSG_TOPIC_IN_SUB_VALUE, recv_sub_topic_ptr + strlen(FOGCLOUD_MSG_TOPIC_IN_WRITE), 
                    strlen((char*)FOGCLOUD_MSG_TOPIC_IN_SUB_VALUE))){
      read_type = MICO_PROP_SUB_TYPE_VALUE;
    }
    else if(0 == strncmp((char*)FOGCLOUD_MSG_TOPIC_IN_SUB_EVENT, recv_sub_topic_ptr + strlen(FOGCLOUD_MSG_TOPIC_IN_WRITE), 
                         strlen((char*)FOGCLOUD_MSG_TOPIC_IN_SUB_EVENT))){
      read_type = MICO_PROP_SUB_TYPE_EVENT;
    }
    else {
      msg_dispatch_log("ERROR: unsupported sub topic: %s", recv_sub_topic_ptr);
      err = kUnsupportedErr;
      goto exit;
    }
    response_json_obj = mico_write_properties(service_table, recv_json_object, read_type);
    
    // send reponse for write status
    if(NULL == response_json_obj){
      msg_dispatch_log("ERROR: write properties error!");
      json_object_object_add(response_json_obj, "MICO_PROP_CODE_WRITE_STATUS", json_object_new_int(MICO_PROP_CODE_WRITE_FAILED));
      err = kWriteErr;
    }
    response_json_string = json_object_to_json_string(response_json_obj);
    err = MicoFogCloudMsgSend(context, response_sub_topic, 
                                (unsigned char*)response_json_string, strlen(response_json_string));
    
  }
  else if( 0 == strncmp((char*)FOGCLOUD_MSG_TOPIC_IN_CHAT, recv_sub_topic_ptr, strlen((char*)FOGCLOUD_MSG_TOPIC_IN_CHAT)) ){
    // from /chat
    msg_dispatch_log("Recv from: %.*s, data[%d]: %s",
                     recv_sub_topic_len, recv_sub_topic_ptr,
                     cloud_msg->data_len, cloud_msg->data);
    
    // just send to message to usart
    //err = user_uartSend(cloud_msg->data, cloud_msg->data_len);
    err = user_customized_topic_msg_handler(context, cloud_msg->topic, cloud_msg->topic_len,
                                      cloud_msg->data, cloud_msg->data_len);
  }
  else if( 0 == strncmp((char*)FOGCLOUD_MSG_TOPIC_IN_INFO, recv_sub_topic_ptr, strlen((char*)FOGCLOUD_MSG_TOPIC_IN_INFO)) ){
    // from /info
    msg_dispatch_log("Recv from: %.*s, data[%d]: %s",
                     recv_sub_topic_len, recv_sub_topic_ptr,
                     cloud_msg->data_len, cloud_msg->data);
    
    // create report json data
    response_json_obj = mico_get_device_info(service_table);
    if(NULL == response_json_obj){
      msg_dispatch_log("ERROR: mico_get_device_info error!");
      json_object_object_add(response_json_obj, "MICO_PROP_CODE_READ_STATUS", json_object_new_int(MICO_PROP_CODE_READ_FAILED));
      err = kResponseErr;
    }
    response_json_string = json_object_to_json_string(response_json_obj);
    msg_dispatch_log("report data: %s", response_json_string);
    err = MicoFogCloudMsgSend(context, response_sub_topic, 
                                (unsigned char*)response_json_string, strlen(response_json_string));
    
  }
  else{
    // unknown topic, ignore msg
    err = kUnsupportedErr;
    msg_dispatch_log("ERROR: Message from unknown topic: %.*s \t data[%d]: %s, ignored.", 
                     recv_sub_topic_len, cloud_msg->topic,
                     cloud_msg->data_len, cloud_msg->data);
  }
  
exit:
  if(NULL != recv_json_object){
    json_object_put(recv_json_object);
    recv_json_object = NULL;
  }
  if(NULL != response_json_obj){
    json_object_put(response_json_obj);
    response_json_obj = NULL;
  }
  if(NULL != response_sub_topic){
    free(response_sub_topic);
    response_sub_topic = NULL;
  }
  return err;
}

// properties notify task
OSStatus  mico_properties_notify(mico_Context_t * const inContext)
{
  OSStatus err = kUnknownErr;
  json_object *notify_obj = NULL;
  const char *notify_json_string = NULL;
  
  require_action(inContext, exit, err = kParamErr);
  
  notify_obj = json_object_new_object();
  require_action(notify_obj, exit, err = kParamErr);
  
  // properties update check
  err = mico_properties_notify_check(inContext, service_table, notify_obj);
  require_noerr(err, exit);
  
  // send notify message to cloud
  if( NULL != (json_object_get_object(notify_obj)->head) ){
    notify_json_string = json_object_to_json_string(notify_obj);
    // notify to topic: <device_id>/out/read
    err = MicoFogCloudMsgSend(inContext, "read/value", 
                              (unsigned char*)notify_json_string, strlen(notify_json_string));
  }
  else{
    // no update msg
    err = kNoErr;
  }

exit:
  if(kNoErr != err){
    msg_dispatch_log("ERROR: mico_properties_notify error, err = %d", err);
  }
  if(NULL != notify_obj){
    json_object_put(notify_obj);
    notify_obj = NULL;
  }
  return err;
}

