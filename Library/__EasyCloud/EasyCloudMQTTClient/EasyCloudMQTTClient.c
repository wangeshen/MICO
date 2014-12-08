/**
******************************************************************************
* @file    EasyCloudMQTTClient.c
* @author  Eshen Wang
* @version V0.1.0
* @date    21-Nov-2014
* @brief   This file contains functions implementation of MQTT client based
           on MICO platform. 
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
//#include "MICONotificationCenter.h"
#include "EasyCloudMQTTClient.h"

//#ifdef debug_out
//#define  _debug_out debug_out
//#else
#define _debug_out(format, ...) do {;}while(0)

#define mico_mqtt_client_log(M, ...) custom_log("MQTTClient", M, ##__VA_ARGS__)
#define mico_mqtt_client_log_trace() custom_log_trace("MQTTClient")
//#endif

/*******************************************************************************
 * VARIABLES
 ******************************************************************************/

static mqtt_client_context_t mqttClientContext = {0};
static mico_mutex_t mqttClientContext_mutex = NULL;

mico_thread_t mqttClientThreadHandle = NULL;

//low level mqtt client
static Network n;
static Client c;

/*******************************************************************************
* FUNCTIONS
*******************************************************************************/

static void mqttClientThread(void *arg);

/*******************************************************************************
* IMPLEMENTATIONS
*******************************************************************************/

void EasyCloudMQTTClientInit(mqtt_client_config_t init)
{
  mico_mqtt_client_log("mqtt client init");
  if(mqttClientContext_mutex == NULL)
    mico_rtos_init_mutex( &mqttClientContext_mutex );
  
  mico_rtos_lock_mutex( &mqttClientContext_mutex );
  mqttClientContext.client_config_info = init;
  mqttClientContext.client_status.state = MQTT_CLIENT_STATUS_STOPPED;
  mico_rtos_unlock_mutex( &mqttClientContext_mutex );
}

OSStatus EasyCloudMQTTClientStart(void)
{
  return mico_rtos_create_thread(&mqttClientThreadHandle, 
                                 MICO_APPLICATION_PRIORITY, 
                                 "MQTT Client", mqttClientThread, 
                                 STACK_SIZE_MQTT_CLIENT_THREAD, NULL);
}

/* MQTT client callback function for message arrived
 * this funcion will call user registered callback function at last.
 */
void messageArrived(MessageData* md)
{
  //MQTT client msg handle
  MQTTMessage* message = md->message;
//  mico_mqtt_client_log("messageArrived: %.*s\t %.*s",
//                       md->topicName->lenstring.len, md->topicName->lenstring.data,
//                       (int)message->payloadlen, (char*)message->payload);
  
  //call user registered handler
  mqttClientContext.client_config_info.hmsg(mqttClientContext.client_config_info.context,
                                            (unsigned char*)message->payload, 
                                            (unsigned int)message->payloadlen);
}

static void mqttClientThread(void *arg)
{
  mico_mqtt_client_log_trace();
  //mico_Context_t *Context = arg;
  //OSStatus err = kUnknownErr;
  
  int rc = -1;
  unsigned char buf[DEFAULT_MICO_MQTT_BUF_SIZE];
  unsigned char readbuf[DEFAULT_MICO_MQTT_READBUF_SIZE];
  
  MQTTPacket_connectData connectData = MQTTPacket_connectData_initializer; 
  MQTTPacket_connectData connectDataCopy = MQTTPacket_connectData_initializer; 
  //Network n;
  //Client c;
  
  mico_mqtt_client_log("mqttClientThread start...");
    
  /* socket connect */
  NewNetwork(&n);
  
MQTTClientRestart:
  memset(&c, 0, sizeof(c));
  memset(buf, 0, sizeof(buf));
  memset(readbuf, 0, sizeof(readbuf));
  memset(&connectData, 0, sizeof(connectData));
  memcpy(&connectData, &connectDataCopy, sizeof(connectData));
  
  mico_rtos_lock_mutex( &mqttClientContext_mutex );
  mqttClientContext.client_status.state = MQTT_CLIENT_STATUS_STARTED;
  mico_rtos_unlock_mutex( &mqttClientContext_mutex );

  /* 1. socket connect */
  while(1) {
    if ( MQTT_CLIENT_STATUS_STOPPED == EasyCloudMQTTClientState()){
      goto client_stop;
    }
    
    mico_mqtt_client_log("MQTT socket network connect...");
    rc = ConnectNetwork(&n, mqttClientContext.client_config_info.host, 
                        mqttClientContext.client_config_info.port);
    if (rc >= 0)
      break;
 
    mico_mqtt_client_log("MQTT socket network connect failed, rc = %d\r\nRetry after 3 seconds...", rc);
    mico_thread_sleep(1);
  }
  mico_mqtt_client_log("MQTT socket network connect OK!");

  /* 2. send mqtt client connect request */
  MQTTClient(&c, &n, DEFAULT_MICO_MQTT_CMD_TIMEOUT, 
             buf, DEFAULT_MICO_MQTT_BUF_SIZE, 
             readbuf, DEFAULT_MICO_MQTT_READBUF_SIZE);

  connectData.willFlag = 0;
  connectData.MQTTVersion = 3;
  connectData.clientID.cstring = mqttClientContext.client_config_info.client_id;
  connectData.username.cstring = mqttClientContext.client_config_info.username;
  connectData.password.cstring = mqttClientContext.client_config_info.password;
  connectData.keepAliveInterval = mqttClientContext.client_config_info.keepAliveInterval;
  connectData.cleansession = 1;
  
  while(1) {
    if ( MQTT_CLIENT_STATUS_STOPPED == EasyCloudMQTTClientState()){
      goto client_stop;
    }
    
    mico_mqtt_client_log("MQTT client connect...");
    rc = MQTTConnect(&c, &connectData);
    if(SUCCESS == rc)
      break;
    
    mico_mqtt_client_log("MQTT client connect failed, rc = %d\r\nRetry after 3 seconds...", rc);
    mico_thread_sleep(1);
  }
  mico_mqtt_client_log("MQTT client connect OK!");
  
  /* 3. subscribe request */
  //mico_mqtt_client_log("subscribing to %s", mqtt_client_config.subtopic);
  while(1) {
    if ( MQTT_CLIENT_STATUS_STOPPED == EasyCloudMQTTClientState()){
      goto client_stop;
    }
    
    //mico_mqtt_client_log("MQTT client subscribe [%s]", mqttClientContext.client_config_info.subtopic);
    rc = MQTTSubscribe(&c, mqttClientContext.client_config_info.subtopic, 
                       mqttClientContext.client_config_info.subscribe_qos , 
                       messageArrived);
    if (SUCCESS == rc)
      break;
    
    mico_mqtt_client_log("MQTT client subscribe [%s] failed, rc = %d\r\nRetry after 3 seconds...",
                         mqttClientContext.client_config_info.subtopic, rc);
    mico_thread_sleep(1);
  }
  mico_mqtt_client_log("MQTT client subscribed [%s] OK!", mqttClientContext.client_config_info.subtopic);
  
  /* 4. set running state */
  mico_rtos_lock_mutex( &mqttClientContext_mutex );
  mqttClientContext.client_status.state = MQTT_CLIENT_STATUS_CONNECTED;
  mico_rtos_unlock_mutex( &mqttClientContext_mutex );
  
  while (1) {
    if ( MQTT_CLIENT_STATUS_STOPPED == EasyCloudMQTTClientState()){
      goto client_stop;
    }
    
    /* subscribe read */
    //mico_mqtt_client_log("MQTT client running...");
    rc = MQTTYield(&c, (int)DEFAULT_MICO_MQTT_YIELD_TMIE);  //keepalive failed will return FAILURE
    if (SUCCESS != rc) {
      MQTTDisconnect(&c);
      n.disconnect(&n);
      
      mico_rtos_lock_mutex( &mqttClientContext_mutex );
      mqttClientContext.client_status.state = MQTT_CLIENT_STATUS_DISCONNECTED;
      mico_rtos_unlock_mutex( &mqttClientContext_mutex );
      
      mico_mqtt_client_log("MQTT client disconnected,reconnect after 3 seconds...");
      mico_thread_sleep(3);
      goto MQTTClientRestart;
    }
  }
  
  /* stop */
client_stop:
  MQTTDisconnect(&c);
  n.disconnect(&n);
  
  mico_rtos_lock_mutex( &mqttClientContext_mutex );
  mqttClientContext.client_status.state = MQTT_CLIENT_STATUS_STOPPED;
  mico_rtos_unlock_mutex( &mqttClientContext_mutex );
  
  mico_mqtt_client_log("MQTT client stopped.");

  mico_rtos_delete_thread(NULL);
  mqttClientThreadHandle = NULL;
  return;
}


OSStatus EasyCloudMQTTClientPublish(const unsigned char* msg, int msglen)
{
  OSStatus err = kUnknownErr;
  int ret = 0;

  MQTTMessage publishData =  MQTTMessage_publishData_initializer;
  
  if(msglen <= 0 || msglen > MAX_UPLOAD_MESSAGE_SIZE)
    return kParamErr;
  
  if(0 == c.isconnected)
    return kStateErr;
    
  //upload data qos1: at most once.
  publishData.qos = QOS0;
  publishData.payload = (void*)msg;
  publishData.payloadlen = msglen;
  
  mico_rtos_lock_mutex( &mqttClientContext_mutex );
  ret = MQTTPublish(&c, mqttClientContext.client_config_info.pubtopic, &publishData);
  mico_rtos_unlock_mutex( &mqttClientContext_mutex );
  
  if (SUCCESS == ret)
    err = kNoErr;
  else
    err = kUnknownErr;
  
  return err;
}

OSStatus EasyCloudMQTTClientStop(void)
{
  OSStatus err = kNoErr;
    
  mico_rtos_lock_mutex( &mqttClientContext_mutex );
  mqttClientContext.client_status.state = MQTT_CLIENT_STATUS_STOPPED;
  mico_rtos_unlock_mutex( &mqttClientContext_mutex );
  
//  MQTTDisconnect(&c);
//  n.disconnect(&n);

  if (NULL != mqttClientThreadHandle) {
    err = mico_rtos_thread_join(&mqttClientThreadHandle);
    if (kNoErr != err)
      return err;
  }
  
  if (NULL != mqttClientThreadHandle) {
    err = mico_rtos_delete_thread(&mqttClientThreadHandle);
    if (kNoErr != err){
      return err;
    }
    mqttClientThreadHandle = NULL;
  }
  
  if (kNoErr == err)
    mico_mqtt_client_log("MQTT client stopped.");
  
  return err;
}

mqttClientState EasyCloudMQTTClientState(void)
{
  mqttClientState state = MQTT_CLIENT_STATUS_STOPPED;
  
  mico_rtos_lock_mutex( &mqttClientContext_mutex );
  state = mqttClientContext.client_status.state;
  mico_rtos_unlock_mutex( &mqttClientContext_mutex );
  
  return state;
}


//TODO ???

//OSStatus EasyCloudMQTTClientSubscribe(const char* subtopic, enum QoS qos, messageHandler hmsg)
//{
//  OSStatus err = kUnknownErr;
//  
//  return err;
//}
//
//OSStatus EasyCloudMQTTClientUnsubscribe (const char* unsubtopic)
//{
//  OSStatus err = kUnknownErr;
//  
//  return err;
//}


