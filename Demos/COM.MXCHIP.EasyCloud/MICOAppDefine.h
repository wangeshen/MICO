/**
  ******************************************************************************
  * @file    MICOAppDefine.h 
  * @author  William Xu
  * @version V1.0.0
  * @date    05-May-2014
  * @brief   This file create a TCP listener thread, accept every TCP client
  *          connection and create thread for them.
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


#ifndef __MICOAPPDEFINE_H
#define __MICOAPPDEFINE_H

#include "Common.h"
#include "MicoVirtualDeviceDef.h"


#define APP_INFO   "mxchipWNet Easycloud Demo based on MICO OS"

#define FIRMWARE_REVISION   "MICO_EASYCLOUD_1"
#define MANUFACTURER        "MXCHIP Inc."
#define SERIAL_NUMBER       "201411205"
#define PROTOCOL            "com.mxchip.easycloud"

/* Wi-Fi configuration mode */
//#define MICO_CONFIG_MODE CONFIG_MODE_AIRKISS
#define MICO_CONFIG_MODE CONFIG_MODE_EASYLINK_PLUS

/*User provided configurations*/
#define CONFIGURATION_VERSION               0x00000001 // if default configuration is changed, update this number
#define LOCAL_PORT                          8080
#define BONJOUR_SERVICE                     "_easylink._tcp.local."

/* product type */
#define DEFAULT_PRODUCT_ID        "d7d5fb56-cdb9-45d7-af8c-9a9a880a674a"
#define DEFAULT_PRODUCT_KEY       "c1272927-e573-4ff8-a62b-08f358d46571"
#define DEFAULT_ROM_VERSION       "v0.2.2"

/*Application's configuration stores in flash*/
typedef struct
{
  uint32_t          configDataVer;    // config param update number
  uint32_t          localServerPort;  // for bonjour service port

  virtual_device_config_t virtualDevConfig;  //virtual device settings
} application_config_t;

/*Running status*/
typedef struct _current_app_status_t {
  virtual_device_status_t virtualDevStatus;  //virtual device status
} current_app_status_t;


#endif

