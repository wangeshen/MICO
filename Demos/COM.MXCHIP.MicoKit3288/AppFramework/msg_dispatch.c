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

extern struct mico_service_t  service_table[];

// handle cloud msg here, for example: send to USART or echo to cloud
OSStatus mico_cloudmsg_dispatch(mico_Context_t* context, mico_fogcloud_msg_t *cloud_msg)
{
  msg_dispatch_log_trace();
  OSStatus err = kNoErr;
  char* topic_ptr = NULL;
  char* session_id = NULL;  // just for debug log
  
  json_object *recv_json_object = NULL;
  char* response_topic = NULL;
  json_object *response_json_obj = NULL;
  const char *response_json_string = NULL;
  
  if((NULL == context) || (NULL == cloud_msg->topic) || (0 == cloud_msg->topic_len) ) {
       return kParamErr;
  }
  
  // strip <device_id>/in
  topic_ptr = (char*)(cloud_msg->topic) + strlen(context->flashContentInRam.appConfig.fogcloudConfig.deviceId) + 3;
  
  // parse sub topic string
  if( 0 == strncmp((char*)FOGCLOUD_MSG_TOPIC_IN_READ, topic_ptr, strlen((char*)FOGCLOUD_MSG_TOPIC_IN_READ)) ){
    // from /read
    session_id = topic_ptr + strlen((char*)FOGCLOUD_MSG_TOPIC_IN_READ);
    msg_dispatch_log("Recv from: %s, session_id: %s, data[%d]: %s", 
                     FOGCLOUD_MSG_TOPIC_IN_READ, 
                     session_id, cloud_msg->data_len, cloud_msg->data);
    
    // read properties && create response json object
    recv_json_object = json_tokener_parse((const char*)(cloud_msg->data));
    require_action(recv_json_object, exit, err = kFormatErr);
    msg_dispatch_log("Recv read object=%s", json_object_to_json_string(recv_json_object));
    response_json_obj = mico_read_properties(context, service_table, recv_json_object);
    
    // send reponse for read data
    if(NULL != response_json_obj){
      response_json_string = json_object_to_json_string(response_json_obj);
      response_topic = ECS_str_replace(response_topic, cloud_msg->topic, 
                                       cloud_msg->topic_len, "/in/", "/out/");
      if(NULL == response_topic){
        msg_dispatch_log("create reponse topic err!");
        err = kUnsupportedErr;
        goto exit;
      }
      err = MicoFogCloudMsgSend(context, response_topic, 
                                (unsigned char*)response_json_string, strlen(response_json_string));
    }
    else{
      msg_dispatch_log("ERROR: read properties error!");
      err = kReadErr;
    }
  }
  else if( 0 == strncmp((char*)FOGCLOUD_MSG_TOPIC_IN_WRITE, topic_ptr, strlen((char*)FOGCLOUD_MSG_TOPIC_IN_WRITE)) ){
    // from /write
    session_id = topic_ptr + strlen((char*)FOGCLOUD_MSG_TOPIC_IN_WRITE);
    msg_dispatch_log("Recv from: %s, session_id: %s, data[%d]: %s", 
                     FOGCLOUD_MSG_TOPIC_IN_WRITE, 
                     session_id, cloud_msg->data_len, cloud_msg->data);
    
    // write properties && create response json object
    recv_json_object = json_tokener_parse((const char*)(cloud_msg->data));
    require_action(recv_json_object, exit, err = kFormatErr);
    msg_dispatch_log("Recv read object=%s", json_object_to_json_string(recv_json_object));
    response_json_obj = mico_write_properties(context, service_table, recv_json_object);
    
    // send reponse for write status
    if(NULL != response_json_obj){
      response_json_string = json_object_to_json_string(response_json_obj);
      response_topic = ECS_str_replace(response_topic, cloud_msg->topic, 
                                       cloud_msg->topic_len, "/in/", "/out/");
      if(NULL == response_topic){
        msg_dispatch_log("create reponse topic err!");
        err = kUnsupportedErr;
        goto exit;
      }
      err = MicoFogCloudMsgSend(context, response_topic, 
                                (unsigned char*)response_json_string, strlen(response_json_string));
    }
    else{
      msg_dispatch_log("ERROR: write properties error!");
      err = kWriteErr;
    }
  }
  else if( 0 == strncmp((char*)FOGCLOUD_MSG_TOPIC_IN_CHAT, topic_ptr, strlen((char*)FOGCLOUD_MSG_TOPIC_IN_CHAT)) ){
    // from /chat
    session_id = topic_ptr + strlen((char*)FOGCLOUD_MSG_TOPIC_IN_CHAT);
    msg_dispatch_log("Recv from: %s, session_id: %s, data[%d]: %s", 
                     FOGCLOUD_MSG_TOPIC_IN_CHAT, 
                     session_id,
                     cloud_msg->data_len, cloud_msg->data);
    
    // just send to message to usart
    err = user_uartSend(cloud_msg->data, cloud_msg->data_len);
  }
  else if( 0 == strncmp((char*)FOGCLOUD_MSG_TOPIC_IN_INFO, topic_ptr, strlen((char*)FOGCLOUD_MSG_TOPIC_IN_INFO)) ){
    // from /info
    session_id = topic_ptr + strlen((char*)FOGCLOUD_MSG_TOPIC_IN_INFO);
    msg_dispatch_log("Recv from: %s, session_id: %s, data[%d]: %s", 
                     FOGCLOUD_MSG_TOPIC_IN_INFO, 
                     session_id, 
                     cloud_msg->data_len, cloud_msg->data);
    
    // create report json data
    response_json_obj = create_dev_info_json_object(service_table);
    if(NULL != response_json_obj){
      response_json_string = json_object_to_json_string(response_json_obj);
      msg_dispatch_log("report data: %s", response_json_string);
      response_topic = ECS_str_replace(response_topic, cloud_msg->topic, 
                                       cloud_msg->topic_len, "/in/", "/out/");
      if(NULL == response_topic){
        msg_dispatch_log("create reponse topic err!");
        err = kUnsupportedErr;
        goto exit;
      }
      err = MicoFogCloudMsgSend(context, response_topic, 
                                (unsigned char*)response_json_string, strlen(response_json_string));
    }
    else{
      msg_dispatch_log("ERROR: create json object error!");
      err = kResponseErr;
    }
  }
  else{
    // unknown topic, ignore msg
    err = kUnsupportedErr;
    msg_dispatch_log("ERROR: Message from unknown topic: %.*s \t data[%d]: %s, ignored.", 
                     cloud_msg->topic_len, cloud_msg->topic,
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
  if(NULL != response_topic){
    free(response_topic);
    response_topic = NULL;
  }
  return err;
}
