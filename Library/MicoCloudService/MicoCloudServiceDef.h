/**
******************************************************************************
* @file    MicoCloudServiceDef.h 
* @author  Eshen Wang
* @version V1.0.0
* @date    31-Oct-2014
* @brief   This header contains the user define for cloud service based
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

#ifndef __MICO_CLOUD_SERVICE_DEF_H_
#define __MICO_CLOUD_SERVICE_DEF_H_

/*******************************************************************************
 * DEFINES
 ******************************************************************************/

// string length in byte (can not modify)
#define MAX_PRODUCT_ID_STRLEN           37
#define MAX_PRODUCT_KEY_STRLEN          37
   
#define MAX_DEVICE_TOKEN_STRLEN         33
#define MAX_USER_TOKEN_STRLEN           33
   
#define MAX_DEVICE_ID_STRLEN            37
#define MAX_DEVICE_KEY_STRLEN           37
   
#define MAX_SUBSCRIBE_TOPIC_STRLEN      256
#define MAX_PUBLISH_TOPIC_STRLEN        256

//cloud server
#define DEFAULT_CLOUD_SERVER            "api.easylink.io"
#define DEFAULT_CLOUD_PORT              80

//MQTT server
#define DEFAULT_MQTT_SERVER             "api.easylink.io"
#define DEFAULT_MQTT_PORT               1883
// in second, here set 1 minite
#define DEFAULT_MQTT_CLLIENT_KEEPALIVE_INTERVAL    60

//request type
#define DEVICE_ACTIVATE             0
#define DEVICE_AUTHORIZE            1
   
#define DEFAULT_DEVICE_ACTIVATE_URL      "/v1/device/activate"
#define DEFAULT_DEVICE_AUTHORIZE_URL     "/v1/device/authorize"

//device info
#define DEFAULT_PRODUCT_ID               "f315fea0-50fc-11e4-b6fc-f23c9150064b"
#define DEFAULT_PRODUCT_KEY              "41a71625-5519-11e4-ad4e-f23c9150064b"
#define DEFAULT_USER_TOKEN               "00000000"
#define DEFAULT_DEVICE_ID                "00000000"
#define DEFAULT_DEVICE_KEY               "00000000"

//MQTT client message recived handler prototype(can not modify).
typedef void (*msgRecvHandler)(unsigned char* Msg, unsigned int len);


#endif
