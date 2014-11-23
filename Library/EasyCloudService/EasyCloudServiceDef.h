/**
******************************************************************************
* @file    EasyCloudServiceDef.h 
* @author  Eshen Wang
* @version V0.2.0
* @date    23-Nov-2014
* @brief   This header contains the public defines for EasyCloud service. 
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


#ifndef __EASYEASYCLOUD_SERVICE_DEF_H_
#define __EASYEASYCLOUD_SERVICE_DEF_H_


/*******************************************************************************
 * DEFINES
 ******************************************************************************/

// string length in bytes
#define MAX_SIZE_DOMAIN_NAME            64
   
#define MAX_SIZE_PRODUCT_ID             37
#define MAX_SIZE_PRODUCT_KEY            37

#define MAX_SIZE_LOGIN_ID               37
#define MAX_SIZE_DEV_PASSWD             37
   
#define MAX_SIZE_USER_TOKEN             33
#define MAX_SIZE_DEVICE_TOKEN           33
   
#define MAX_SIZE_DEVICE_ID              37
#define MAX_SIZE_DEVICE_KEY             37
   
#define MAX_SIZE_SUBSCRIBE_TOPIC        256
#define MAX_SIZE_PUBLISH_TOPIC          256

// default values for easycloud server
#define DEFAULT_CLOUD_SERVER            "api.easylink.io"
#define DEFAULT_CLOUD_PORT              80

// default values for MQTT server
#define DEFAULT_MQTT_SERVER             "api.easylink.io"
#define DEFAULT_MQTT_PORT               1883
#define DEFAULT_MQTT_PORT_SSL           8883
// in seconds, here set 60s
#define DEFAULT_MQTT_CLLIENT_KEEPALIVE_INTERVAL    60
   
#define DEFAULT_DEVICE_ID               "none"
#define DEFAULT_DEVICE_KEY              "none"
 

 /*******************************************************************************
 * STRUCTURES
 ******************************************************************************/

typedef enum {
  EASYCLOUD_STOPPED = 1,      //service stopped
  EASYCLOUD_STARTED = 2,      //service start up
  EASYCLOUD_CONNECTED = 3,    //service work ok
  EASYCLOUD_DISCONNECTED = 4  //service diconnected from server
} easycloudServiceState;

typedef struct _easycloud_service_status_t {
  easycloudServiceState state;
  bool isActivated;
  char deviceId[MAX_SIZE_DEVICE_ID];
  char masterDeviceKey[MAX_SIZE_DEVICE_KEY];
} easycloud_service_status_t;
 
//message recived callback function prototype
typedef void (*easycloudMsgRecvCallBack)(void* const context, 
                                         unsigned char* msg, unsigned int msgLen);
typedef void (*easycloudStatusChangedCallback)(void* const context,
                                               easycloud_service_status_t serviceStateInfo);

typedef struct _easycloud_service_config_t {
  //easycloud server info
  char                 *host;
  uint16_t              port;
  
  //device info
  char                 *bssid;
  char                 *productId;
  char                 *productKey;
  
  //user info
  char                 *loginId;
  char                 *password;
  char                 *userToken;
  
  //mqtt client info
  char                 *mqttServerHost;
  uint16_t              mqttServerPort;
  uint16_t              mqttKeepAliveInterval;
  easycloudMsgRecvCallBack  msgRecvhandler;  // message received callback
  
  easycloudStatusChangedCallback statusNotify;  // cloud servie status changed callback
  void                 *context;  // app context
} easycloud_service_config_t;

typedef struct _easycloud_service_context_t {
  /*easycloud service config info*/
  easycloud_service_config_t service_config_info;
  
  /*easycloud service running status*/
  easycloud_service_status_t service_status;
} easycloud_service_context_t;


#endif
