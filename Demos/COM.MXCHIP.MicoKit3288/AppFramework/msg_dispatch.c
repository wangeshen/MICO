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


extern mico_dev_service_node_t service_table[1];

// handle cloud msg here, for example: send to USART or echo to cloud
OSStatus mico_cloudmsg_dispatch(mico_Context_t* context, mico_fogcloud_msg *cloud_msg)
{
  msg_dispatch_log_trace();
  OSStatus err = kUnknownErr;
  char* topic_ptr = NULL;
  char* session_id = NULL;
  json_object *dev_info_json_object = NULL;
  
  if((NULL == context) || (NULL == cloud_msg->topic) || (0 == cloud_msg->topic_len) ) {
       return kParamErr;
  }
  
  // parse topic: <device_id>/in, /<read/write>, /<session_id>
  topic_ptr = (char*)(cloud_msg->topic) + strlen(context->flashContentInRam.appConfig.fogcloudConfig.deviceId) + 3;
  if( 0 == strncmp((char*)FOGCLOUD_MSG_TOPIC_IN_READ, topic_ptr, strlen((char*)FOGCLOUD_MSG_TOPIC_IN_READ)) ){
    // from /read
    session_id = topic_ptr + strlen((char*)FOGCLOUD_MSG_TOPIC_IN_READ);
    msg_dispatch_log("Recv from: %s, session_id: %s, data[%d]: %s", 
                     FOGCLOUD_MSG_TOPIC_IN_READ, 
                     session_id, 
                     cloud_msg->data_len, cloud_msg->data);
    // parse json data:  { iid:0, iid:0, ...}
    // create response json data
    // send response
    err = kNoErr;
  }
  else if( 0 == strncmp((char*)FOGCLOUD_MSG_TOPIC_IN_WRITE, topic_ptr, strlen((char*)FOGCLOUD_MSG_TOPIC_IN_WRITE)) ){
    // from /write
    session_id = topic_ptr + strlen((char*)FOGCLOUD_MSG_TOPIC_IN_WRITE);
    msg_dispatch_log("Recv from: %s, session_id: %s, data[%d]: %s", 
                     FOGCLOUD_MSG_TOPIC_IN_WRITE, 
                     session_id,
                     cloud_msg->data_len, cloud_msg->data);
    // parse json data:  { iid:1, iid:2, ...}
    // create response status json data
    // send response
    err = kNoErr;
  }
  else if( 0 == strncmp((char*)FOGCLOUD_MSG_TOPIC_IN_CHAT, topic_ptr, strlen((char*)FOGCLOUD_MSG_TOPIC_IN_CHAT)) ){
    // from /chat
    session_id = topic_ptr + strlen((char*)FOGCLOUD_MSG_TOPIC_IN_CHAT);
    msg_dispatch_log("Recv from: %s, session_id: %s, data[%d]: %s", 
                     FOGCLOUD_MSG_TOPIC_IN_CHAT, 
                     session_id,
                     cloud_msg->data_len, cloud_msg->data);
    // send to usart
    user_uartSend(cloud_msg->data, cloud_msg->data_len);
    
    err = kNoErr;
  }
  else if( 0 == strncmp((char*)FOGCLOUD_MSG_TOPIC_IN_INFO, topic_ptr, strlen((char*)FOGCLOUD_MSG_TOPIC_IN_INFO)) ){
    // from /info
    session_id = topic_ptr + strlen((char*)FOGCLOUD_MSG_TOPIC_IN_INFO);
    msg_dispatch_log("Recv from: %s, session_id: %s, data[%d]: %s", 
                     FOGCLOUD_MSG_TOPIC_IN_INFO, 
                     session_id, 
                     cloud_msg->data_len, cloud_msg->data);
    // create report json data
    create_service_table();
    dev_info_json_object = create_dev_info_json_object(service_table, sizeof(service_table));
    if(NULL != dev_info_json_object){
      msg_dispatch_log("report data: %s", json_object_to_json_string(dev_info_json_object));
      err = MicoFogCloudMsgSend(context, FOGCLOUD_MSG_TOPIC_OUT_INFO, 
                                (unsigned char*)json_object_to_json_string(dev_info_json_object), 
                                strlen(json_object_to_json_string(dev_info_json_object)));
    }
    else{
      msg_dispatch_log("ERROR: create json object error!");
      err = kResponseErr;
    }
    
    if(NULL != dev_info_json_object){
      json_object_put(dev_info_json_object);
      dev_info_json_object = NULL;
    }
  }
  else{
    // unknown topic, ignore msg
    err = kUnsupportedErr;
    msg_dispatch_log("ERROR: Message from unknown topic: %.*s \t data[%d]: %s, ignored.", 
                     cloud_msg->topic_len, cloud_msg->topic,
                     cloud_msg->data_len, cloud_msg->data);
  }
  
  return err;
}

