/**
  ******************************************************************************
  * @file    MVDCloudTest.h 
  * @author  WangEshen
  * @version V1.0.0
  * @date    22-Jan-2015
  * @brief   MVD Cloud test header file.
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

#ifndef __MVD_CLOUD_TEST_H_
#define __MVD_CLOUD_TEST_H_

#include "Common.h"
//#include "MICODefine.h"

#include "MicoVirtualDevice.h"

/* test params */
#define EASYCLOUD_TEST_SERVER               "api.easylink.io"
#define EASYCLOUD_TEST_PORT                 4151

#define EASYCLOUD_TEST_URL_START_RECV       "/startTest"
#define EASYCLOUD_TEST_URL_STOP_RECV        "/stopTest"

// start request server to send msg to device, then echo to server
OSStatus MVDCloudTest_StartRecv(const char* device_id,
                                uint32_t msg_length, 
                                uint32_t period_s, uint32_t interval_ms);
// stop server test
OSStatus MVDCloudTest_StopRecv(const char* device_id);

// start send data test
OSStatus MVDCloudTest_StartSend(mico_Context_t* inContext,
                                uint32_t msg_length, 
                                uint32_t period_s, uint32_t interval_ms);


/*******************************************************************************
* EasyCloud test functions
*******************************************************************************/
  
/* interface test */
OSStatus easycloud_if_test(mico_Context_t* context);

/* transmission test */
OSStatus easycloud_transmission_test(mico_Context_t* context);

/* OTA test */
OSStatus easycloud_ota_test(mico_Context_t* context);

#endif

