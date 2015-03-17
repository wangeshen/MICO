/**
  ******************************************************************************
  * @file    user_main.c 
  * @author  Eshen Wang
  * @version V0.1.0
  * @date    17-Mar-2015
  * @brief   user main functons in user_main thread.
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
#include "MICOAppDefine.h"
#include "user_main.h"
#include "MicoFogCloud.h"
#include "user_uart.h"

#define user_log(M, ...) custom_log("USER", M, ##__VA_ARGS__)
#define user_log_trace() custom_log_trace("USER")


/* MICO user callback: Restore default configuration provided by application */
void userRestoreDefault_callback(mico_Context_t *inContext)
{
  user_log("restore user config.");
}

// handle cloud msg here, for example: send to USART or echo to cloud
OSStatus user_fogcloud_msg_handler(mico_Context_t* context, 
                            const char* topic, const unsigned int topicLen,
                            unsigned char *inBuf, unsigned int inBufLen)
{
  user_log();
  OSStatus err = kUnknownErr;
  char* responseTopic = NULL;
  unsigned char* responseMsg = NULL;
  unsigned char* ptr = NULL;
  int responseMsgLen = 0;
  
  /* send to USART */
  err = user_uartSend(inBuf, inBufLen); // transfer raw data to USART
  require_noerr_action( err, exit, user_log("ERROR: send to uart error! err=%d", err) );
  
  /* echo to cloud */
  // responseTopic = device_id/out, message = [MAC]msg
  responseTopic = ECS_str_replace(responseTopic, topic, topicLen, "/in", "/out");
  responseMsgLen = strlen(context->micoStatus.mac) + 2 + inBufLen;
  responseMsg = (unsigned char*)malloc(responseMsgLen + 1);
  memset(responseMsg, 0x00, responseMsgLen);
  if(NULL == responseMsg){
    err = kNoMemoryErr;
    goto exit;
  }
  ptr = responseMsg;
  memcpy(ptr, "[", 1);
  ptr += 1;
  memcpy(ptr, (const void*)&(context->micoStatus.mac), strlen(context->micoStatus.mac));
  ptr += strlen(context->micoStatus.mac);
  memcpy(ptr, "]", 1);
  ptr += 1;
  memcpy(ptr, inBuf, inBufLen);
  ptr += inBufLen;
  memcpy(ptr, '\0', 1);
  //err = MicoFogCloudSendMsg2Cloud(context, responseTopic, responseMsg, responseMsgLen);
  err = MicoFogCloudSendMsg2Cloud(context, NULL, responseMsg, responseMsgLen);
  if(NULL != responseTopic){
    free(responseTopic);
  }
  if(NULL != responseMsg){
      ptr = NULL;
      free(responseMsg);
  }
  
  return kNoErr;
  
exit:
  return err;
}

OSStatus user_main( mico_Context_t * const inContext )
{
  user_log_trace();
  OSStatus err = kNoErr;
    
  require_action(inContext, exit, err = kParamErr);
  
  // init uart
  //err = user_uartInit(inContext);
  //require_noerr_action( err, exit, user_log("ERROR: user uart init failed!") );
    
  // loop working for user function
  while(1){
    user_log("user_main working...");
    
    // user work
    // ...
    
    mico_thread_sleep(10);
  }

  // never get here only if fatal err && exit.
exit:
  user_log("ERROR: user_main exit, err=%d.", err);
  return err;
}
