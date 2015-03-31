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
//#include "MICOAppDefine.h"
#include "user_main.h"
#include "MicoFogCloud.h"
#include "user_uart.h"
#include "msg_dispatch.h"
#include "properties.h"

#define user_log(M, ...) custom_log("USER", M, ##__VA_ARGS__)
#define user_log_trace() custom_log_trace("USER")

extern struct mico_service_t  service_table[];


/* MICO user callback: Restore default configuration provided by user
 * called when Easylink buttion long pressed
 */
void userRestoreDefault_callback(mico_Context_t *inContext)
{
  user_log("restore user config.");
}

// handle cloud msg here, for example: send to USART or echo to cloud
OSStatus user_fogcloud_msg_handler(mico_Context_t* context, 
                            const char* topic, const unsigned int topicLen,
                            unsigned char *inBuf, unsigned int inBufLen)
{
  user_log_trace();
  OSStatus err = kUnknownErr;
  mico_fogcloud_msg_t fogcloud_msg;
  
  if((NULL == context) || (NULL == topic) || (0 == topicLen) ) {
    user_log("ERROR: mico_cloudmsg_dispatch params error, err=%d", err);
    return kParamErr;
  }
  
  fogcloud_msg.topic = topic;
  fogcloud_msg.topic_len = topicLen;
  fogcloud_msg.data = inBuf;
  fogcloud_msg.data_len = inBufLen;
  
  err = mico_cloudmsg_dispatch(context, &fogcloud_msg);    
  if(kNoErr != err){
    user_log("ERROR: mico_cloudmsg_dispatch error, err=%d", err);
  }
  else
  {}
  
  return err;
}

OSStatus user_main( mico_Context_t * const inContext )
{
  user_log_trace();
  OSStatus err = kNoErr;
  
  user_log("Enter user_main.");
    
  require_action(inContext, exit, err = kParamErr);
  
  // uart init
  //err = user_uartInit(inContext);
  //require_noerr_action( err, exit, user_log("ERROR: user uart init failed!") );
  
  /* ADC init */
  err = MicoAdcInitialize(MICO_ADC_1, 3);
  require_noerr_action( err, exit, user_log("ERROR: MicoAdcInitialize ADC1 err=%d", err) );
  
  /* LED init */
  // ...
    
  // loop for handling msg
  while(1){
    //user_log("user_main working...");
    mico_thread_msleep(500);
    
    // prop notify
    err = mico_properties_notify(inContext);
    if(kNoErr != err){
        user_log("ERROR: properties notify failed! err = %d", err);
      }
  }

  // never get here only if fatal err && exit.
exit:
  user_log("ERROR: user_main exit, err=%d.", err);
  return err;
}
