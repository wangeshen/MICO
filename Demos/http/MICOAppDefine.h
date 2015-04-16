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

#include "MICO.h"
#include "Common.h"

#define APP_INFO            "mxchipWNet http/https Demo"

#define FIRMWARE_REVISION   "MICO_HTTP_1_0"
#define MANUFACTURER        "MXCHIP Inc."
#define SERIAL_NUMBER       "20140415"
#define PROTOCOL            "com.mxchip.http"

/*Application's configuration stores in flash*/
typedef struct {
  uint32_t          reserved;
} application_config_t;

/*Running status*/
typedef struct _current_app_status_t {
  uint32_t          reserved;
} current_app_status_t;

#endif

