/**
******************************************************************************
* @file    MicoVirtualDevice.h 
* @author  Eshen Wang
* @version V0.2.0
* @date    21-Nov-2014
* @brief   This header contains the interfaces 
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

#ifndef __HAPROTOCOL_H_
#define __HAPROTOCOL_H_

#include "EasyCloudServiceDef.h"


/*******************************************************************************
 * DEFINES
 ******************************************************************************/

// default device settings
#define DEFAULT_PRODUCT_ID               "f315fea0-50fc-11e4-b6fc-f23c9150064b"
#define DEFAULT_PRODUCT_KEY              "41a71625-5519-11e4-ad4e-f23c9150064b"

#define DEFAULT_LOGIN_ID                 "none"
#define DEFAULT_DEV_PASSWD               "none"
#define DEFAULT_USER_TOKEN               "none"

#define STACK_SIZE_USART_RECV_THREAD     0x500


/*******************************************************************************
 * STRUCTURES
 ******************************************************************************/

/* device configuretion stores in flash */
typedef struct
{
  /*IO settings*/
  uint32_t          USART_BaudRate;
  
  /* EasyCloud settings (opt) */
  char              cloudServerDomain[MAX_SIZE_DOMAIN_NAME];
  int               cloudServerPort;
  char              mqttServerDomain[MAX_SIZE_DOMAIN_NAME];
  int               mqttServerPort;
  uint16_t          mqttkeepAliveInterval;
  
  /* product properties */
  char              productId[MAX_SIZE_PRODUCT_ID];
  char              productKey[MAX_SIZE_PRODUCT_KEY];
  
  /* device settings */
  char              loginId[MAX_SIZE_LOGIN_ID];            // user login id
  char              devPasswd[MAX_SIZE_DEV_PASSWD];        // master device password set by user
  char              userToken[MAX_SIZE_USER_TOKEN];        // user token
  /* device properties stored in flash */
  bool              isActivated;                           // device activate status, RO
  char              deviceId[MAX_SIZE_DEVICE_ID];          // get from cloud server, RO
  char              masterDeviceKey[MAX_SIZE_DEVICE_KEY];  // get from cloud server, RO
} virtual_device_config_t;

/* device status */
typedef struct 
{
  /* cloud service connect */
  bool              isCloudConnected;
} virtual_device_status_t;


/*******************************************************************************
 * INTERFACES
 ******************************************************************************/

//MICO APP interfaces
OSStatus MVDInit(void* const context);

//device interfaces
OSStatus MVDDeviceMsgProcess(void* context, uint8_t *inBuf, unsigned int inBufLen);

//Cloud service interfaces
OSStatus MVDCloudMsgProcess(void* context, unsigned char *inBuf, unsigned int inBufLen);

//OTA
OSStatus MVDFirmwareUpdate(void* context);
//activate
OSStatus MVDActivate(void* context);
//authorize
OSStatus MVDAuthorize(void* context);

#endif
