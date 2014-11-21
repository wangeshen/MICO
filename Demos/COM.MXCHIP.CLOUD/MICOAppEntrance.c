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


#define app_log(M, ...) custom_log("APP", M, ##__VA_ARGS__)
#define app_log_trace() custom_log_trace("APP")


void userAppThread(void *arg)
{
  mico_Context_t *inContext = (mico_Context_t *)arg;
  micoMemInfo_t *memInfo = NULL;
  app_log("userApp working thread start.");
  
  while(1)
  {    
    memInfo = mico_memory_info();
    app_log("system free mem[userApp]=%d", memInfo->free_memory);
    
    if(inContext->appStatus.virtualDevStatus.isCloudConnected)
    {
      app_log("cloud service working...");
    }
    else
    {
      app_log("cloud service stopped.");
    }
    
    mico_thread_sleep(10);
  }
}

OSStatus userAppStart(mico_Context_t *inContext)
{
  return mico_rtos_create_thread(NULL, MICO_APPLICATION_PRIORITY, "userApp", userAppThread, 0x400, inContext );
}


/* MICO system callback: Restore default configuration provided by application */
void appRestoreDefault_callback(mico_Context_t *inContext)
{
  inContext->flashContentInRam.appConfig.configDataVer = CONFIGURATION_VERSION;
  inContext->flashContentInRam.appConfig.localServerPort = LOCAL_PORT;
  inContext->flashContentInRam.appConfig.localServerEnable = true;
  inContext->flashContentInRam.appConfig.remoteServerEnable = true;
  sprintf(inContext->flashContentInRam.appConfig.remoteServerDomain, DEAFULT_REMOTE_SERVER);
  inContext->flashContentInRam.appConfig.remoteServerPort = DEFAULT_REMOTE_SERVER_PORT;
  
  //restore virtual device info
  inContext->flashContentInRam.appConfig.virtualDevConfig.USART_BaudRate = 115200;
  inContext->flashContentInRam.appConfig.virtualDevConfig.isActivated = false;
  sprintf(inContext->flashContentInRam.appConfig.virtualDevConfig.cloudServerDomain, DEFAULT_CLOUD_SERVER);
  inContext->flashContentInRam.appConfig.virtualDevConfig.cloudServerPort = DEFAULT_CLOUD_PORT;
  sprintf(inContext->flashContentInRam.appConfig.virtualDevConfig.mqttServerDomain, DEFAULT_MQTT_SERVER); 
  inContext->flashContentInRam.appConfig.virtualDevConfig.mqttServerPort = DEFAULT_MQTT_PORT;
  inContext->flashContentInRam.appConfig.virtualDevConfig.mqttkeepAliveInterval = DEFAULT_MQTT_CLLIENT_KEEPALIVE_INTERVAL;
  sprintf(inContext->flashContentInRam.appConfig.virtualDevConfig.productId, DEFAULT_PRODUCT_ID);
  sprintf(inContext->flashContentInRam.appConfig.virtualDevConfig.productKey, DEFAULT_PRODUCT_KEY);
  sprintf(inContext->flashContentInRam.appConfig.virtualDevConfig.loginId, DEFAULT_LOGIN_ID);
  sprintf(inContext->flashContentInRam.appConfig.virtualDevConfig.devPasswd, DEFAULT_DEV_PASSWD);
  sprintf(inContext->flashContentInRam.appConfig.virtualDevConfig.userToken, DEFAULT_USER_TOKEN);
  sprintf(inContext->flashContentInRam.appConfig.virtualDevConfig.deviceId, DEFAULT_DEVICE_ID);
  sprintf(inContext->flashContentInRam.appConfig.virtualDevConfig.masterDeviceKey, DEFAULT_DEVICE_KEY);
}

OSStatus MICOStartApplication( mico_Context_t * const inContext )
{
  app_log_trace();
  OSStatus err = kNoErr;
  micoMemInfo_t *memInfo = NULL;
  
  require_action(inContext, exit, err = kParamErr);

  /*Bonjour for service searching*/
  if(inContext->flashContentInRam.micoSystemConfig.bonjourEnable == true){
    MICOStartBonjourService( Station, inContext );
  }
  
  memInfo = mico_memory_info();
  app_log("system free mem[MICO]=%d", memInfo->free_memory);
  
  /* start virtual device */
  err = MVDInit(inContext);
  require_noerr_action( err, exit, app_log("ERROR: virtual device start failed!") );
  
  /* user app working thread */
  err = userAppStart(inContext);
  require_noerr_action( err, exit, app_log("ERROR: Unable to start userApp thread.") );
  
exit:
  return err;
}
