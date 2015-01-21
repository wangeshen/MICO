/**
  ******************************************************************************
  * @file    MICOAppEntrance.c 
  * @author  William Xu
  * @version V1.0.0
  * @date    05-May-2014
  * @brief   Mico application entrance, addd user application functons and threads.
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
#include "MicoPlatform.h"

#include "MicoVirtualDevice.h"


#define app_log(M, ...) custom_log("APP", M, ##__VA_ARGS__)
#define app_log_trace() custom_log_trace("APP")


static mico_thread_t user_thread_handler = NULL;

#define APP_CLOUD_CONNECTED_MSG_2CLOUD     "{\"APPCloud\":\"connected\"}"
#define APP_CLOUD_CONNECTED_MSG_2MCU       "[APP]Cloud connected!\r\n"
#define APP_CLOUD_DISCONNECTED_MSG_2MCU    "[APP]Cloud disconnected!\r\n"
#define APP_DEVICE_INACTIVATED_MSG_2MCU    "[APP]Device unactivated!\r\n"

void user_thread(void* arg){
  OSStatus err = kUnknownErr;
  mico_Context_t* inContext = (mico_Context_t*)arg;
  
  while(1){
    /* check cloud status && send msg test */
    if(MVDIsActivated(inContext)){
      app_log("[APP]device is activated.");
      if(MVDCloudIsConnect(inContext)){
        app_log("[APP]cloud is connected, send msg to Cloud && MCU.");
        // send msg to cloud status channel
        err = MVDSendMsg2Cloud(inContext, PUBLISH_TOPIC_CHANNEL_STATUS,
                               APP_CLOUD_CONNECTED_MSG_2CLOUD, 
                               strlen(APP_CLOUD_CONNECTED_MSG_2CLOUD));
        if(kNoErr != err){
          app_log("[APP]ERROR: send msg to cloud err=%d.", err);
        }
        // send msg to MCU
        err = MVDSendMsg2Device(inContext, APP_CLOUD_CONNECTED_MSG_2MCU, 
                                strlen(APP_CLOUD_CONNECTED_MSG_2MCU));
        if(kNoErr != err){
          app_log("[APP]ERROR: send msg to MCU err=%d.", err);
        }
      }
      else{
        app_log("[APP]cloud is not connected, send msg to MCU.");
        // send msg to MCU
        err = MVDSendMsg2Device(inContext, APP_CLOUD_DISCONNECTED_MSG_2MCU, 
                                strlen(APP_CLOUD_DISCONNECTED_MSG_2MCU));
        if(kNoErr != err){
          app_log("[APP]ERROR: send msg to MCU err=%d.", err);
        }
      }
    }
    else{
      app_log("[APP]device is not activated, send msg to MCU.");
      // send msg to MCU
      err = MVDSendMsg2Device(inContext, APP_DEVICE_INACTIVATED_MSG_2MCU, 
                              strlen(APP_DEVICE_INACTIVATED_MSG_2MCU));
      if(kNoErr != err){
        app_log("[APP]ERROR: send msg to MCU err=%d.", err);
      }
    }
    
    mico_thread_sleep(5);
  }
}

OSStatus start_user_work(void* arg)
{
  mico_Context_t* inContext = (mico_Context_t*)arg;
  return mico_rtos_create_thread(&user_thread_handler, 
                                 MICO_APPLICATION_PRIORITY, 
                                 "user", user_thread, 0x500, inContext );
}


/* MICO system callback: Restore default configuration provided by application */
void appRestoreDefault_callback(mico_Context_t *inContext)
{
  inContext->flashContentInRam.appConfig.configDataVer = CONFIGURATION_VERSION;
  inContext->flashContentInRam.appConfig.localServerPort = LOCAL_PORT;
  
  // restore virtual device config
  MVDRestoreDefault(inContext);
}

OSStatus MICOStartApplication( mico_Context_t * const inContext )
{
  app_log_trace();
  OSStatus err = kNoErr;
    
  require_action(inContext, exit, err = kParamErr);
  
  /*Bonjour for service searching*/
  if(inContext->flashContentInRam.micoSystemConfig.bonjourEnable == true)
    MICOStartBonjourService( Station, inContext );

  /* start virtual device */
  err = MVDInit(inContext);
  require_noerr_action( err, exit, app_log("ERROR: virtual device start failed!") );
  
  /* user test */
  err =  start_user_work(inContext);
  require_noerr_action( err, exit, app_log("ERROR: start user work failed!") );

exit:
  return err;
}

