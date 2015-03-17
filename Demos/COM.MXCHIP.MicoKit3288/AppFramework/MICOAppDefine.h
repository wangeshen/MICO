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
#include "MicoFogCloudDef.h"


#define APP_INFO   "mxchipWNet MicoKit Demo based on MICO OS"

#define FIRMWARE_REVISION   "MICO_KIT3288_1_0"
#define MANUFACTURER        "MXCHIP Inc."
#define SERIAL_NUMBER       "20150317"
#define PROTOCOL            "com.mxchip.micokit3288"


/* Wi-Fi configuration mode */
#define MICO_CONFIG_MODE CONFIG_MODE_EASYLINK
//#define MICO_CONFIG_MODE CONFIG_MODE_AIRKISS

/* Define MICO cloud type */
#define CLOUD_NO                                (0)
#define CLOUD_FOGCLOUD                          (1)
#define CLOUD_ALINK                             (2)
    
/* MICO cloud service type */
#define MICO_CLOUD_TYPE    CLOUD_FOGCLOUD

#define STACK_SIZE_USER_MAIN_THREAD             0x1000

/*User provided configurations*/
#define CONFIGURATION_VERSION               0x00000003 // if default configuration is changed, update this number
#define LOCAL_PORT                          8080       // bonjour service port
#define BONJOUR_SERVICE                     "_easylink._tcp.local."

/* product type */
#define DEFAULT_PRODUCT_ID                  "d64f517c"
#define DEFAULT_PRODUCT_KEY                 "e935ef56-1d03-4432-9524-8d4a691a26ec"
#define DEFAULT_ROM_VERSION                 "v0.0.1"

/*Application's configuration stores in flash*/
typedef struct
{
  uint32_t          configDataVer;           // config param update number
  uint32_t          localServerPort;         // for bonjour service port

  fogcloud_config_t fogcloudConfig;          // fogcloud settings
} application_config_t;

/*Running status*/
typedef struct _current_app_status_t {
  fogcloud_status_t fogcloudStatus;         // fogcloud status
} current_app_status_t;


#endif

