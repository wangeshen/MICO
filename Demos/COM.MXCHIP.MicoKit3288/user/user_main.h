/**
  ******************************************************************************
  * @file    user_main.h
  * @author  Eshen Wang
  * @version V0.1.0
  * @date    17-Mar-2015
  * @brief   user main functons prototype in user_main thread.
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

#ifndef __MICO_USER_MAIN_H_
#define __MICO_USER_MAIN_H_

#include "MICODefine.h"

// cloud msg recv handler
OSStatus user_fogcloud_msg_handler(mico_Context_t* context, 
                            const char* topic, const unsigned int topicLen,
                            unsigned char *inBuf, unsigned int inBufLen);

// MICO user config callback: Restore default configuration provided by application
void userRestoreDefault_callback(mico_Context_t *inContext);

// user main function
OSStatus user_main( mico_Context_t * const inContext );

#endif  // __MICO_USER_MAIN_H_
