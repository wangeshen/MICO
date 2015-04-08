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
#include "user_main.h"
#include "MicoFogCloud.h"
#include "msg_dispatch.h"
#include "user_properties.h"
#include "uart.h"

#define user_log(M, ...) custom_log("USER", M, ##__VA_ARGS__)
#define user_log_trace() custom_log_trace("USER")

// device services &&¡¡properties table defined by user in user_properties.c
extern struct mico_service_t  service_table[];


/* MICO user callback: Restore default configuration provided by user
 * called when Easylink buttion long pressed
 */
void userRestoreDefault_callback(mico_Context_t *inContext)
{
  user_log("restore user config.");
}

/* FogCloud message receive callback: handle cloud messages here
 */
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
  
  err = mico_cloudmsg_dispatch(context, service_table, &fogcloud_msg);    
  if(kNoErr != err){
    user_log("ERROR: mico_cloudmsg_dispatch error, err=%d", err);
  }
  else
  {}
  
  return err;
}

/* user main function, called by AppFramework after FogCloud connected.
 */
OSStatus user_main( mico_Context_t * const inContext )
{
  user_log_trace();
  OSStatus err = kNoErr;
  require_action(inContext, exit, err = kParamErr);
  
  /* uart init */
  err = user_uartInit(inContext);
  require_noerr_action( err, exit, user_log("ERROR: user uart init failed!") );
  
  /* ADC init */
  err = MicoAdcInitialize(MICO_ADC_1, 3);
  require_noerr_action( err, exit, user_log("ERROR: MicoAdcInitialize ADC1 err=%d", err) );
  
  /* LED init */
  // ...
  
  /* start properties notify task */
  err = mico_start_properties_notify(inContext, service_table, 
                                     MICO_PROPERTIES_NOTIFY_INTERVAL, 
                                     STACK_SIZE_NOTIFY_THREAD);
  require_noerr_action( err, exit, user_log("ERROR: mico_start_properties_notify err=%d", err) );
    
  // loop
  while(1){
    user_log("user_main working...");
    mico_thread_msleep(10000);
  }

  // never getting here only if fatal error.
exit:
  user_log("ERROR: user_main exit, err=%d.", err);
  return err;
}
