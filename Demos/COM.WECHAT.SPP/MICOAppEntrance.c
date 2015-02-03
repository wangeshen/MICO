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
#include "MVDCloudTest.h"


#define app_log(M, ...) custom_log("APP", M, ##__VA_ARGS__)
#define app_log_trace() custom_log_trace("APP")


#define APP_CLOUD_CONNECTED_MSG_2CLOUD     "{\"APPCloud\":\"connected\"}"
#define APP_CLOUD_CONNECTED_MSG_2MCU       "[APP]Cloud status: connected\r\n"
#define APP_CLOUD_DISCONNECTED_MSG_2MCU    "[APP]Cloud status: disconnected\r\n"
#define APP_DEVICE_INACTIVATED_MSG_2MCU    "[APP]Device status: inactivate\r\n"

/* test define */
#define MVD_CLOUD_TEST_RECV_MSG_SIZE             100      // byte
#define MVD_CLOUD_TEST_RECV_MSG_PERIOD           300      // s
#define MVD_CLOUD_TEST_RECV_MSG_INTERVAL         100      // ms

#define MVD_CLOUD_TEST_SEND_MSG_SIZE             100      // byte
#define MVD_CLOUD_TEST_SEND_MSG_PERIOD           300      // s
#define MVD_CLOUD_TEST_SEND_MSG_INTERVAL         500      // ms

#define MVD_CLOUD_TEST_RECV_MSG_RATE             0.99     // recv msg count rate >= 99%

uint64_t cloud_test_data_cnt = 0;
static uint64_t check_recv_data_len = 0;
static char recv_data_cnt_str[64] = {0};
static char total_recv_data_cnt_str[64] = {0};

// send data check
uint64_t cloud_test_echo_data_cnt = 0;
static char recv_echo_data_cnt_str[64] = {0};

static mico_thread_t mvd_test_thread_handler = NULL;

static OSStatus mvd_transfer_test(mico_Context_t *inContext)
{
  OSStatus err = kUnknownErr;
  
  /* Cloud recv test */
  //app_log("[MVD_TEST][CLOUD RECV]start");
  MVDSendMsg2Device(inContext, 
                    "[MVD_TEST][CLOUD RECV]start ...\r\n", 
                    strlen("[MVD_TEST][CLOUD RECV]start ...\r\n"));
  cloud_test_data_cnt = 0;
  err = MVDCloudTest_StartRecv(inContext->flashContentInRam.appConfig.virtualDevConfig.deviceId,
                               MVD_CLOUD_TEST_RECV_MSG_SIZE,
                               MVD_CLOUD_TEST_RECV_MSG_PERIOD, 
                               MVD_CLOUD_TEST_RECV_MSG_INTERVAL);
  require_noerr( err, exit );
  
  /* start send test at the same time */
  // server will echo msg when startTest
  err = MVDCloudTest_StartSend(inContext,
                         MVD_CLOUD_TEST_SEND_MSG_SIZE, 
                         MVD_CLOUD_TEST_SEND_MSG_PERIOD,
                         MVD_CLOUD_TEST_SEND_MSG_INTERVAL);
  
  // timeout for stopping test process
  mico_thread_sleep(MVD_CLOUD_TEST_RECV_MSG_PERIOD + 5);
  err = MVDCloudTest_StopRecv(inContext->flashContentInRam.appConfig.virtualDevConfig.deviceId);
  require_noerr( err, exit );
  //app_log("[MVD_TEST]CLOUD RECV]stopped");
  MVDSendMsg2Device(inContext,
                    "[MVD_TEST][CLOUD RECV]stopped\r\n", 
                    strlen("[MVD_TEST][CLOUD RECV]stopped\r\n"));
  
  // check test ok?
  check_recv_data_len = (MVD_CLOUD_TEST_RECV_MSG_SIZE*1000*MVD_CLOUD_TEST_RECV_MSG_PERIOD/MVD_CLOUD_TEST_RECV_MSG_INTERVAL);
  sprintf(recv_data_cnt_str, "[MVD_TEST][CLOUD RECV]recv=%lld\t", cloud_test_data_cnt);
  sprintf(total_recv_data_cnt_str, "[MVD_TEST][CLOUD RECV]Total=%lld\r\n", check_recv_data_len);
  
  if((check_recv_data_len >= cloud_test_data_cnt) && 
     (cloud_test_data_cnt >= (uint64_t)((long)check_recv_data_len * MVD_CLOUD_TEST_RECV_MSG_RATE))){
       //app_log("[MVD_TEST][CLOUD RECV]test OK!");
       err = MVDSendMsg2Device(inContext,
                               "[MVD_TEST][CLOUD RECV]test OK!\r\n", 
                               strlen("[MVD_TEST][CLOUD RECV]test OK!\r\n"));
     }
  else{
    //app_log("[MVD_TEST][CLOUD RECV]test FAILED!");
    err = MVDSendMsg2Device(inContext,
                            "[MVD_TEST][CLOUD RECV]test FAILED!\r\n", 
                            strlen("[MVD_TEST][CLOUD RECV]test FAILED!\r\n"));
    app_log("[MVD_TEST]Recv: %lld/%lld", cloud_test_data_cnt, check_recv_data_len);
    MVDSendMsg2Device(inContext, (unsigned char*)recv_data_cnt_str, strlen(recv_data_cnt_str));
    MVDSendMsg2Device(inContext, (unsigned char*)total_recv_data_cnt_str, strlen(total_recv_data_cnt_str));
  }
  
  // check echo data len
  sprintf(recv_echo_data_cnt_str, "[MVD_TEST][CLOUD RECV]Recv_echo=%lld\r\n", cloud_test_echo_data_cnt);
  MVDSendMsg2Device(inContext, (unsigned char*)recv_echo_data_cnt_str, strlen(recv_echo_data_cnt_str));
  
  app_log("[MVD_TEST]All done!");
  err = kNoErr;
  
exit:
  return err;
}

void mvd_test_thread(void* arg){
  OSStatus err = kUnknownErr;
  mico_Context_t* inContext = (mico_Context_t*)arg;
  bool isConnected = false;
  micoMemInfo_t *memInfo = NULL;
  char freeMemString[64] = {0};
  
  while(1){
    /* system memory check */
    memInfo = mico_memory_info();
    app_log("[MVD_TEST]System memory: %d", memInfo->free_memory);
    
    /* check cloud status && send msg test */
    if(MVDIsActivated(inContext)){
      app_log("[MVD_TEST]Device status: activated");
      /* get device_id */
      app_log("[MVD_TEST]Device_id: %s", MVDGetDeviceID(inContext));
      if(MVDCloudIsConnect(inContext)){
        app_log("[MVD_TEST]Cloud status: connected");
        if(false == isConnected){
          //app_log("[MVD_TEST]Cloud status: connected");
          // send msg to cloud default channel
          err = MVDSendMsg2Cloud(inContext, NULL,
                                 APP_CLOUD_CONNECTED_MSG_2CLOUD, 
                                 strlen(APP_CLOUD_CONNECTED_MSG_2CLOUD));
          if(kNoErr != err){
            app_log("[MVD_TEST]ERROR: send msg to cloud err=%d.", err);
          }
          // send msg to cloud status channel
          err = MVDSendMsg2Cloud(inContext, PUBLISH_TOPIC_CHANNEL_STATUS,
                                 APP_CLOUD_CONNECTED_MSG_2CLOUD, 
                                 strlen(APP_CLOUD_CONNECTED_MSG_2CLOUD));
          if(kNoErr != err){
            app_log("[MVD_TEST]ERROR: send msg to channel[%s] err=%d.", 
                    PUBLISH_TOPIC_CHANNEL_STATUS, err);
          }
          // send msg to MCU
          err = MVDSendMsg2Device(inContext, APP_CLOUD_CONNECTED_MSG_2MCU, 
                                  strlen(APP_CLOUD_CONNECTED_MSG_2MCU));
          if(kNoErr != err){
            app_log("[MVD_TEST]ERROR: send msg to MCU err=%d.", err);
          }
          isConnected = true;
          
          // data transfer test
          //err = mvd_transfer_test(inContext);
          
          memInfo = mico_memory_info();
          sprintf(freeMemString, "[MVD_TEST]System memory: %d\r\n", memInfo->free_memory);
          err = MVDSendMsg2Device(inContext, (unsigned char*)freeMemString, strlen(freeMemString));
          if(kNoErr == err){
            goto exit;
          }
        }
      }
      else{
        app_log("[MVD_TEST]Cloud status: disconnected");
        if(isConnected){
          //app_log("[MVD_TEST]Cloud status: disconnected");
          // send msg to MCU
          err = MVDSendMsg2Device(inContext, APP_CLOUD_DISCONNECTED_MSG_2MCU, 
                                  strlen(APP_CLOUD_DISCONNECTED_MSG_2MCU));
          if(kNoErr != err){
            app_log("[MVD_TEST]ERROR: send msg to MCU err=%d.", err);
          }
          isConnected = false;
        }
      }
    }
    else{
      app_log("[MVD_TEST]Device status: inactivate");
      // send msg to MCU
      err = MVDSendMsg2Device(inContext, APP_DEVICE_INACTIVATED_MSG_2MCU, 
                              strlen(APP_DEVICE_INACTIVATED_MSG_2MCU));
      if(kNoErr != err){
        app_log("[MVD_TEST]ERROR: send msg to MCU err=%d.", err);
      }
    }
    
    mico_thread_sleep(5);
  }
  
exit:
  mico_rtos_delete_thread(NULL);
  return;
}

OSStatus start_mvd_test(void* arg)
{
  mico_Context_t* inContext = (mico_Context_t*)arg;
  return mico_rtos_create_thread(&mvd_test_thread_handler, 
                                 MICO_APPLICATION_PRIORITY, 
                                 "MVD_test", mvd_test_thread, 0x800, inContext );
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
  
  /* mvd test */
  //err =  start_mvd_test(inContext);
  //require_noerr_action( err, exit, app_log("ERROR: start mvd_test thread failed!") );

exit:
  return err;
}

