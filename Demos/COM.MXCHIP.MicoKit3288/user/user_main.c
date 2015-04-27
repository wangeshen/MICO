/**
  ******************************************************************************
  * @file    user_main.c 
  * @author  Eshen Wang
  * @version V1.0.0
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
#include "fogcloud_msg_dispatch.h"

#include "user_properties.h"
#include "user_params_storage.h"
#include "drivers/uart.h"
#include "drivers/rgb_led.h"

#define user_log(M, ...) custom_log("USER", M, ##__VA_ARGS__)
#define user_log_trace() custom_log_trace("USER")

/* properties defined in user_properties.c by user
 */
// device services &&¡¡properties table
extern struct mico_service_t  service_table[];
// user params
extern user_context_t user_context;
// global flag to indacate update user params in flash
volatile bool user_params_need_update = false;


/* MICO user callback: Restore default configuration provided by user
 * called when Easylink buttion long pressed
 */
void userRestoreDefault_callback(mico_Context_t *mico_context)
{
  user_log("INFO: restore user configuration.");
  userParams_RestoreDefault(mico_context, &user_context);
}

/* FogCloud message receive callback: handle cloud messages here
 */
OSStatus user_fogcloud_msg_handler(mico_Context_t* mico_context, 
                            const char* topic, const unsigned int topicLen,
                            unsigned char *inBuf, unsigned int inBufLen)
{
  user_log_trace();
  OSStatus err = kUnknownErr;
  mico_fogcloud_msg_t fogcloud_msg;
  int retCode = MSG_PROP_UNPROCESSED;
  
  if((NULL == mico_context) || (NULL == topic) || (0 == topicLen) ) {
    user_log("ERROR: mico_cloudmsg_dispatch params error, err=%d", err);
    return kParamErr;
  }
  
  fogcloud_msg.topic = topic;
  fogcloud_msg.topic_len = topicLen;
  fogcloud_msg.data = inBuf;
  fogcloud_msg.data_len = inBufLen;
  
  err = mico_fogcloud_msg_dispatch(mico_context, service_table, &fogcloud_msg, &retCode);    
  if(kNoErr != err){
    user_log("ERROR: mico_cloudmsg_dispatch error, err=%d", err);
  }
  else
  {
    if(MSG_PROP_WROTE == retCode){
      user_params_need_update = true;  // user params need update in flash
    }
  }
  
  return err;
}

/* user main function, called by AppFramework after FogCloud connected.
 */
OSStatus user_main( mico_Context_t * const mico_context )
{
  user_log_trace();
  OSStatus err = kNoErr;
  
  require_action(mico_context, exit, err = kParamErr);
  
  /* read user config params from flash */
  if(NULL == user_context.config_mutex){
    mico_rtos_init_mutex(&user_context.config_mutex);
  }
  err = userParams_Read(mico_context, &user_context);
  require_noerr_action( err, exit, user_log("ERROR: read user param from flash failed! err=%d.", err) );
  
  /* user modules init */
  // uart init
  err = user_uartInit(mico_context);
  require_noerr_action( err, exit, user_log("ERROR: user uart init failed! err = %d.", err) );
  
  // ADC init
  err = MicoAdcInitialize(MICO_ADC_1, 3);
  require_noerr_action( err, exit, user_log("ERROR: MicoAdcInitialize ADC1 err = %d.", err) );
  
  // LED init
  if(user_context.config.rgb_led_sw){
    rgb_led_open((float)(user_context.config.rgb_led_hues), 
                 (float)(user_context.config.rgb_led_saturation), 
                 (float)(user_context.config.rgb_led_brightness));
  }
  else{
    rgb_led_close();
  }
  
  /* start properties notify task */
  err = mico_start_properties_notify(mico_context, service_table, 
                                     MICO_PROPERTIES_NOTIFY_INTERVAL_MS, 
                                     STACK_SIZE_NOTIFY_THREAD);
  require_noerr_action( err, exit, user_log("ERROR: mico_start_properties_notify err = %d.", err) );
    
  /* main loop */
  while(1){
    
    // update config params in flash
    if(user_params_need_update){
      err = userParams_Update(mico_context, &user_context);
      if(kNoErr == err){
        user_params_need_update = false;
      }
      else{
        user_log("ERROR: update user config into flash failed! err = %d.", err);
      }
    }
    
    // blink system led
    MicoGpioOutputTrigger((mico_gpio_t)MICO_SYS_LED);
    mico_thread_msleep(1000);
  }

  // never getting here only if fatal error.
exit:
  user_log("ERROR: user_main exit, err = %d.", err);
  return err;
}
