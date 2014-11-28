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

#include "StringUtils.h"

#include "MicoPlatform.h"

#include "MVDCloudInterfaces.h"
#include "MicoVirtualDevice.h"

#define app_log(M, ...) custom_log("APP", M, ##__VA_ARGS__)
#define app_log_trace() custom_log_trace("APP")


void userAppThread(void *arg)
{
  mico_Context_t *inContext = (mico_Context_t *)arg;
  micoMemInfo_t *memInfo = NULL;
  
  int wait_time = 2;  //auto activate after 10s (5*2)
  MVDActivateRequestData_t devDefaultActivateData;
  OSStatus err = kUnknownErr;
  bool connected = false;
  
  app_log("userApp working thread start.");
      
  memInfo = mico_memory_info();
  app_log("[userApp]system free mem=%d", memInfo->free_memory);
  
  while(1)
  {    
    if(inContext->appStatus.virtualDevStatus.isCloudConnected){
      if (!connected){
        app_log("[userApp]cloud service working...");
        //MVDDeviceMsgProcess(inContext, "device connect ok!", 
        //                    strlen("device connect ok!"));
        MVDCloudInterfaceSend("device connect ok!", 
                              strlen("device connect ok!"));
        
        connected = true;
        wait_time = 2;  //recovery value;
      }
    }
    else{
      connected = false; //recovery value;
      if (wait_time > 0){
        app_log("cloud service disconnected, will activate device after %ds...", wait_time*5);
        wait_time--;
      }
      else if(0 == wait_time){
        // auto activate, using default login_id/dev_pass/user_token
        app_log("activate device by userApp...");
        memset((void*)&devDefaultActivateData, 0, sizeof(devDefaultActivateData));
        strncpy(devDefaultActivateData.loginId,
                inContext->flashContentInRam.appConfig.virtualDevConfig.loginId,
                MAX_SIZE_LOGIN_ID);
        strncpy(devDefaultActivateData.devPasswd,
                inContext->flashContentInRam.appConfig.virtualDevConfig.devPasswd,
                MAX_SIZE_DEV_PASSWD);
        strncpy(devDefaultActivateData.user_token,
                inContext->flashContentInRam.appConfig.virtualDevConfig.userToken,
                MAX_SIZE_USER_TOKEN);
        err = MVDActivate(inContext, devDefaultActivateData);
        if(kNoErr == err){
          app_log("device activate success!");
          wait_time = -1;  //activate ok, never do activate again.
        }
        else{
          wait_time = 1;  //reactivate after 5 (5*1) seconds
          app_log("device activate failed, will retry in 5s...");
        }
      }
      else{
      }
    }
    
    mico_thread_sleep(5);
  }
}

OSStatus userAppStart(mico_Context_t *inContext)
{
  return mico_rtos_create_thread(NULL, MICO_APPLICATION_PRIORITY, "userApp", userAppThread, 0x500, inContext );
}


/* MICO system callback: Restore default configuration provided by application */
void appRestoreDefault_callback(mico_Context_t *inContext)
{
  inContext->flashContentInRam.appConfig.configDataVer = CONFIGURATION_VERSION;
  inContext->flashContentInRam.appConfig.localServerPort = LOCAL_PORT;
  inContext->flashContentInRam.appConfig.USART_BaudRate = 115200;

  //restore virtual device config
  MVDRestoreDefault(inContext);
}

OSStatus MICOStartApplication( mico_Context_t * const inContext )
{
  app_log_trace();
  OSStatus err = kNoErr;
  micoMemInfo_t *memInfo = NULL;
  
  require_action(inContext, exit, err = kParamErr);

  /* Bonjour for service searching */
  if(inContext->flashContentInRam.micoSystemConfig.bonjourEnable == true)
    MICOStartBonjourService( Station, inContext );

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




