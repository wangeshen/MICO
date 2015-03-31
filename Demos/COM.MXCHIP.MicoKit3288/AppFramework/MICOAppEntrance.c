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
#include "MicoFogCloud.h"
#include "user_main.h"

#define app_log(M, ...) custom_log("APP", M, ##__VA_ARGS__)
#define app_log_trace() custom_log_trace("APP")

/* default callback function */
WEAK OSStatus user_main( mico_Context_t * const inContext )
{
  app_log("ERROR: user_main undefined!");
  return kNotHandledErr;
}

WEAK void userRestoreDefault_callback(mico_Context_t *inContext)
{
}

/* user thread created by MICO */
void user_thread(void* arg)
{
  OSStatus err = kUnknownErr;
  mico_Context_t *inContext = (mico_Context_t *)arg;
  
  // wait semaphore for cloud connection
  mico_fogcloud_waitfor_connect(inContext, MICO_WAIT_FOREVER);  // block to wait fogcloud connect
  app_log("fogcloud connected.");
  
  // loop in user mian function && must not return
  err = user_main(inContext);
  
  // never get here only if user work err && exit.
  app_log("ERROR: user thread exit err=%d.", err);
  mico_rtos_delete_thread(NULL);
  return;
}

OSStatus startUserThread(mico_Context_t *inContext)
{
  app_log_trace();
  OSStatus err = kNoErr;
  require_action(inContext, exit, err = kParamErr);
  
  err = mico_rtos_create_thread(NULL, MICO_APPLICATION_PRIORITY, "user_main", 
                                user_thread, STACK_SIZE_USER_MAIN_THREAD, 
                                inContext );
  
exit:
  return err;
}

/* MICO system callback: Restore default configuration provided by application */
void appRestoreDefault_callback(mico_Context_t *inContext)
{
  inContext->flashContentInRam.appConfig.configDataVer = CONFIGURATION_VERSION;
  inContext->flashContentInRam.appConfig.localServerPort = LOCAL_PORT;
  
  // restore fogcloud config
  MicoFogCloudRestoreDefault(inContext);
  // restore user config
  userRestoreDefault_callback(inContext);
}

/* MICO APP entrance */
OSStatus MICOStartApplication( mico_Context_t * const inContext )
{
  app_log_trace();
  OSStatus err = kNoErr;
    
  require_action(inContext, exit, err = kParamErr);
  
  /* Bonjour for service searching */
  if(inContext->flashContentInRam.micoSystemConfig.bonjourEnable == true) {
    MICOStartBonjourService( Station, inContext );
  }

  /* start cloud service */
#if (MICO_CLOUD_TYPE == CLOUD_FOGCLOUD)
  err = MicoStartFogCloudService( inContext );
  require_noerr_action( err, exit, app_log("ERROR: Unable to start FogCloud service.") );
#elif (CLOUD_ALINK)
  app_log("MICO CloudService: Ali.");
#elif (MICO_CLOUD_TYPE == CLOUD_NO)
  app_log("MICO CloudService disabled.");
#else
  #error "MICO cloud service type is not defined"?
#endif
  
  /* start user thread */
  err = startUserThread( inContext );
  require_noerr_action( err, exit, app_log("ERROR: start user thread failed!") );

exit:
  return err;
}
