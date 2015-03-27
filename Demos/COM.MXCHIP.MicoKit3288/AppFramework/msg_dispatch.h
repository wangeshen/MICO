/**
  ******************************************************************************
  * @file    msg_dispatch.h
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

#ifndef __MICO_FOGCLOUD_MSG_DISPATCH_H_
#define __MICO_FOGCLOUD_MSG_DISPATCH_H_

// recv topic
#define FOGCLOUD_MSG_TOPIC_IN_READ       "/read/"
#define FOGCLOUD_MSG_TOPIC_IN_WRITE      "/write/"
#define FOGCLOUD_MSG_TOPIC_IN_CHAT       "/chat/"
#define FOGCLOUD_MSG_TOPIC_IN_INFO       "/info/"

// send topic
#define FOGCLOUD_MSG_TOPIC_OUT_READ      "/read/"
#define FOGCLOUD_MSG_TOPIC_OUT_WRITE     "/write/"
#define FOGCLOUD_MSG_TOPIC_OUT_CHAT      "/chat/"
#define FOGCLOUD_MSG_TOPIC_OUT_INFO      "/info/"

// msg data
typedef struct _mico_fogcloud_msg_t{
  const char *topic;
  unsigned int topic_len;
  unsigned char *data;
  unsigned int data_len;
}mico_fogcloud_msg_t;

// handle cloud msg here, for example: send to USART or echo to cloud
OSStatus mico_cloudmsg_dispatch(mico_Context_t* context, mico_fogcloud_msg_t *cloud_msg);

// property notify check
OSStatus mico_properties_notify(mico_Context_t * const inContext);

#endif // __MICO_FOGCLOUD_MSG_DISPATCH_H_