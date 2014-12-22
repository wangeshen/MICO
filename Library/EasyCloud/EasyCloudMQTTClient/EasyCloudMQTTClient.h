/**
******************************************************************************
* @file    EasyCloudMQTTClient.h 
* @author  EShen Wang
* @version V1.0.0
* @date    21-Nov-2014
* @brief   This header contains function prototypes for MQTT client. based
           on MICO platform
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


#ifndef __EASYCLOUD_MQTT_CLIENT_H_
#define __EASYCLOUD_MQTT_CLIENT_H_

#include "Common.h"
#include "MQTTClient.h"

/*******************************************************************************
* DEFINES
*******************************************************************************/

//in ms
#define DEFAULT_MICO_MQTT_YIELD_TMIE          1000
//in ms
#define DEFAULT_MICO_MQTT_CMD_TIMEOUT         500
//in byte
#define MAX_UPLOAD_MESSAGE_SIZE               256
#define DEFAULT_MICO_MQTT_BUF_SIZE            (MAX_UPLOAD_MESSAGE_SIZE + 45)
#define DEFAULT_MICO_MQTT_READBUF_SIZE        256

#define STACK_SIZE_MQTT_CLIENT_THREAD         0x800

/*******************************************************************************
* STRUCTURES
*******************************************************************************/

typedef void (*mqttMsgArrivedHandler)(void* context, 
                                      const char* topic,
                                      unsigned char* Msg, unsigned int len);

typedef enum {
  MQTT_CLIENT_STATUS_STOPPED = 1,  //client stopped
  MQTT_CLIENT_STATUS_STARTED = 2,  //client start up
  MQTT_CLIENT_STATUS_CONNECTED = 3,  //client work ok
  MQTT_CLIENT_STATUS_DISCONNECTED = 4,  //client disconnect
} mqttClientState;

typedef struct _mqtt_client_config_t {
  /*server info*/
  char *host;
  uint16_t port;
  
  /*client opts*/
  char * client_id;
  enum QoS subscribe_qos;
  char *username;
  char *password;
  unsigned short keepAliveInterval;
  
  /*topics*/
  char *pubtopic;
  char *subtopic;
  
  /*message arrived callback*/
  mqttMsgArrivedHandler hmsg;
  //user context
  void* context;
} mqtt_client_config_t;

typedef struct _mqtt_client_status_t {
  mqttClientState state;
} mqtt_client_status_t;

typedef struct _mqtt_client_context_t {
  mqtt_client_config_t client_config_info;
  /* running status */
  mqtt_client_status_t client_status;
} mqtt_client_context_t;

/*******************************************************************************
* USER INTERFACES
*******************************************************************************/

void EasyCloudMQTTClientInit(mqtt_client_config_t init);
OSStatus EasyCloudMQTTClientStart(void);
OSStatus EasyCloudMQTTClientStop(void);

OSStatus EasyCloudMQTTClientPublish(const unsigned char* msg, int msglen);
OSStatus EasyCloudMQTTClientPublishto(const char* topic, const unsigned char* msg, int msglen);
mqttClientState EasyCloudMQTTClientState(void);

//not implement
//OSStatus EasyCloudMQTTClientSubscribe(const char* subtopic, enum QoS qos, messageHandler hmsg);
//OSStatus EasyCloudMQTTClientUnsubscribe(const char* unsubtopic);

#endif
