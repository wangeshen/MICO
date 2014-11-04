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

#include "MicoCloudService.h"
//#include "RGB.h"

#include "MICOAppDefine.h"
#include "StringUtils.h"
#include "SppProtocol.h"


#define app_log(M, ...) custom_log("APP", M, ##__VA_ARGS__)
#define app_log_trace() custom_log_trace("APP")

volatile ring_buffer_t  rx_buffer;
volatile uint8_t        rx_data[UART_BUFFER_LENGTH];

//user recived data handler
void userAppMessageArrivedHandler(unsigned char *Msg, unsigned int len)
{
  unsigned int msglen = len;
  //note: get data just for length=len is valid, because Msg is just a buf pionter.
  app_log("userApp send to UART: [%d]=%.*s", len, len, Msg);
  
  sppWlanCommandProcessForCloud(Msg, (int*)(&msglen), UART_FOR_APP);
}

/*
void _userApp_thread(void *arg)
{ 
  micoMemInfo_t *memInfo = NULL;
  char freeMemStr[16] = {0};
  
  app_log("_userApp_thread start.");
  
  while(1)
  {    
    memInfo = mico_memory_info();
    app_log("system free mem[user thread]=%d", memInfo->free_memory);
    
    if(CLOUD_SERVICE_STATUS_CONNECTED == MicoCloudServiceState()) {
      app_log("_userApp_thread running: cloud service connected, upload data...");
      sprintf(freeMemStr, "freeMem=%16d", memInfo->free_memory);
      //MicoCloudServiceUpload((unsigned char*)freeMemStr, strlen(freeMemStr));
    }
    else {
      app_log("_userApp_thread running: cloud service not connected!");
    }
    
    mico_thread_sleep(10);
  }
}

OSStatus userAppStart(mico_Context_t *inContext)
{
  return mico_rtos_create_thread(NULL, MICO_APPLICATION_PRIORITY, "userApp", _userApp_thread, 0x400, inContext );
}
*/

/* MICO system callback: Restore default configuration provided by application */
void appRestoreDefault_callback(mico_Context_t *inContext)
{
  inContext->flashContentInRam.appConfig.configDataVer = CONFIGURATION_VERSION;
  inContext->flashContentInRam.appConfig.localServerPort = LOCAL_PORT;
  inContext->flashContentInRam.appConfig.localServerEnable = true;
  inContext->flashContentInRam.appConfig.USART_BaudRate = 115200;
  inContext->flashContentInRam.appConfig.remoteServerEnable = true;
  sprintf(inContext->flashContentInRam.appConfig.remoteServerDomain, DEAFULT_REMOTE_SERVER);
  inContext->flashContentInRam.appConfig.remoteServerPort = DEFAULT_REMOTE_SERVER_PORT;
  
  inContext->flashContentInRam.appConfig.isAcitivated = false;
  sprintf(inContext->flashContentInRam.appConfig.cloudServerDomain, DEFAULT_CLOUD_SERVER);
  inContext->flashContentInRam.appConfig.cloudServerPort = DEFAULT_CLOUD_PORT;
  sprintf(inContext->flashContentInRam.appConfig.mqttServerDomain, DEFAULT_MQTT_SERVER); 
  inContext->flashContentInRam.appConfig.mqttServerPort = DEFAULT_MQTT_PORT;
  inContext->flashContentInRam.appConfig.mqttkeepAliveInterval = DEFAULT_MQTT_CLLIENT_KEEPALIVE_INTERVAL;
  sprintf(inContext->flashContentInRam.appConfig.product_id, DEFAULT_PRODUCT_ID);
  sprintf(inContext->flashContentInRam.appConfig.product_key, DEFAULT_PRODUCT_KEY);
  sprintf(inContext->flashContentInRam.appConfig.user_token, DEFAULT_USER_TOKEN);
  sprintf(inContext->flashContentInRam.appConfig.device_id, DEFAULT_DEVICE_ID);
  sprintf(inContext->flashContentInRam.appConfig.master_device_key, DEFAULT_DEVICE_KEY);
}

OSStatus MICOStartApplication( mico_Context_t * const inContext )
{
  app_log_trace();
  OSStatus err = kNoErr;
  mico_uart_config_t uart_config;
  //micoMemInfo_t *memInfo = NULL;
  
  //cloud service config info
  cloud_service_config_t cloud_service_config = {
    inContext->flashContentInRam.appConfig.cloudServerDomain, inContext->flashContentInRam.appConfig.cloudServerPort,
    inContext->micoStatus.mac, 
    inContext->flashContentInRam.appConfig.product_id, inContext->flashContentInRam.appConfig.product_key, 
    inContext->flashContentInRam.appConfig.user_token,
    inContext->flashContentInRam.appConfig.mqttServerDomain, inContext->flashContentInRam.appConfig.mqttServerPort, 
    userAppMessageArrivedHandler, inContext->flashContentInRam.appConfig.mqttkeepAliveInterval
  };
  app_log("MAC=%s", (char*)inContext->micoStatus.mac);
  
  require_action(inContext, exit, err = kParamErr);
  
  sppProtocolInitForCloud( inContext );

  /*Bonjour for service searching*/
  if(inContext->flashContentInRam.micoSystemConfig.bonjourEnable == true){
    MICOStartBonjourService( Station, inContext );
  }
  
  /*UART receive thread*/
  uart_config.baud_rate    = inContext->flashContentInRam.appConfig.USART_BaudRate;
  uart_config.data_width   = DATA_WIDTH_8BIT;
  uart_config.parity       = NO_PARITY;
  uart_config.stop_bits    = STOP_BITS_1;
  uart_config.flow_control = FLOW_CONTROL_DISABLED;
  if(inContext->flashContentInRam.micoSystemConfig.mcuPowerSaveEnable == true)
    uart_config.flags = UART_WAKEUP_ENABLE;
  else
    uart_config.flags = UART_WAKEUP_DISABLE;
  ring_buffer_init  ( (ring_buffer_t *)&rx_buffer, (uint8_t *)rx_data, UART_BUFFER_LENGTH );
  MicoUartInitialize( UART_FOR_APP, &uart_config, (ring_buffer_t *)&rx_buffer );
  err = mico_rtos_create_thread(NULL, MICO_APPLICATION_PRIORITY, "UART Recv", uartRecv_thread, 0x2A0, (void*)inContext );
  require_noerr_action( err, exit, app_log("ERROR: Unable to start the uart recv thread.") );
  
//  memInfo = mico_memory_info();
//  app_log("system free mem[no cloud service]=%d", memInfo->free_memory);
  
  /*start cloud service*/
  MicoCloudServiceInit(cloud_service_config);
  err = MicoCloudServiceStart(inContext);
  require_noerr_action( err, exit, app_log("ERROR: Unable to start cloud service thread.") );

//  memInfo = mico_memory_info();
//  app_log("system free mem[cloud service stated]=%d", memInfo->free_memory);

  app_log("Mico Cloud Service library version: %s", MicoCloudServiceVersion());
  
  /*user app work thread*/
//  err = userAppStart(inContext);
//  require_noerr_action( err, exit, app_log("ERROR: Unable to start userApp thread.") );
  
exit:
  return err;
}
