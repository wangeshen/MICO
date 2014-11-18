
#include <stdio.h>
#include "MICOAppDefine.h"
#include "HaProtocol.h"
#include "SocketUtils.h"
#include "StringUtils.h"
#include "RingBufferUtils.h"
#include "debug.h"
#include "MicoPlatform.h"
#include "platform_common_config.h"
#include "MICONotificationCenter.h"
#include "MicoCloudService.h"

#define ha_log(M, ...) custom_log("HA Command", M, ##__VA_ARGS__)
#define ha_log_trace() custom_log_trace("HA Command")

static cloud_service_status_t cloudStatusInfo;
static uint32_t network_state = 0;
static mico_mutex_t _mutex;

static OSStatus check_sum(void *inData, uint32_t inLen);
static uint16_t _calc_sum(void *data, uint32_t len);
static mico_thread_t    _report_status_thread_handler = NULL;
static mico_semaphore_t _report_status_sem = NULL;
static void _report_status_thread(void *arg);

volatile ring_buffer_t  rx_buffer;
volatile uint8_t        rx_data[UART_BUFFER_LENGTH];

extern void uartRecv_thread(void *inContext);


//user recived data handler
void userAppMessageArrivedHandler(unsigned char *Msg, unsigned int len)
{
  unsigned int msglen = len;
  
  haWlanCommandProcess(Msg, (int*)(&msglen));
}


void MicoCloudServiceStatusChangedCallback(cloud_service_status_t serviceStateInfo)
{
  cloudStatusInfo = serviceStateInfo;
  if (CLOUD_SERVICE_STATUS_CONNECTED == serviceStateInfo.state){
    ha_log("cloud service connected!");
    set_network_state(CLOUD_CONNECT, 1);
  }
  else{
    ha_log("cloud service disconnected!");
    set_network_state(CLOUD_CONNECT, 0);
  }
}
  

void haNotify_WifiStatusHandler(int event, mico_Context_t * const inContext)
{
  ha_log_trace();
  (void)inContext;
  switch (event) {
  case NOTIFY_STATION_UP:
    set_network_state(STA_CONNECT, 1);
    break;
  case NOTIFY_STATION_DOWN:
    set_network_state(STA_CONNECT, 0);
    break;
  default:
    break;
  }
  return;
}


int is_network_state(int state)
{
  if ((network_state & state) == 0)
    return 0;
  else
    return 1;
}


/* Get system status */
static void _get_status(mxchip_state_t *cmd, mico_Context_t * const inContext)
{
  ha_log_trace();

  uint16_t cksum;
  LinkStatusTypeDef ap_state;

  cmd->flag = 0x00BB;
  cmd->cmd = 0x8008;
  cmd->cmd_status = CMD_OK;
  cmd->datalen = sizeof(mxchip_state_t) - 10;

  cmd->status.uap_state = is_network_state(UAP_START);
  cmd->status.sta_state = is_network_state(STA_CONNECT);
  if ((is_network_state(STA_CONNECT) == 1) && (CLOUD_SERVICE_STATUS_CONNECTED == MicoCloudServiceState()))
    cmd->status.cloud_status = 1;
  else
    cmd->status.cloud_status = 0;

  CheckNetLink(&ap_state);
  cmd->status.wifi_strength = ap_state.wifi_strength;
  strncpy(cmd->status.ip, inContext->micoStatus.localIp, maxIpLen);
  strncpy(cmd->status.mask, inContext->micoStatus.netMask, maxIpLen);
  strncpy(cmd->status.gw, inContext->micoStatus.gateWay, maxIpLen);
  strncpy(cmd->status.dns, inContext->micoStatus.dnsServer, maxIpLen);
  strncpy(cmd->status.mac, inContext->micoStatus.mac, 18);

  cksum = _calc_sum(cmd, sizeof(mxchip_state_t) - 2);
  cmd->cksum = cksum;
}


void set_network_state(int state, int on)
{
  ha_log_trace();
  mico_rtos_lock_mutex(&_mutex);
  if (on)
    network_state |= state;
  else {
    network_state &= ~state;
    if (state == STA_CONNECT)
      network_state &= ~CLOUD_CONNECT;
  }
  
  if ((state == STA_CONNECT) || (state == CLOUD_CONNECT)){
    mico_rtos_set_semaphore(&_report_status_sem);
  }
  mico_rtos_unlock_mutex(&_mutex);
}


OSStatus haProtocolInit(mico_Context_t * const inContext)
{
  ha_log_trace();
  OSStatus err = kUnknownErr;
  mico_uart_config_t uart_config;
  int cloudServiceLibVersion = 0;

  //cloud service config info
  cloud_service_config_t cloud_service_config = {
    inContext->flashContentInRam.appConfig.cloudServerDomain,
    inContext->flashContentInRam.appConfig.cloudServerPort,
    inContext->micoStatus.mac,
    inContext->flashContentInRam.appConfig.product_id,
    inContext->flashContentInRam.appConfig.product_key,
    inContext->flashContentInRam.appConfig.user_token,
    inContext->flashContentInRam.appConfig.mqttServerDomain,
    inContext->flashContentInRam.appConfig.mqttServerPort,
    userAppMessageArrivedHandler, inContext->flashContentInRam.appConfig.mqttkeepAliveInterval
  };
  
  /*============================================================================
   * init MCU USART interface
   *==========================================================================*/

  mico_rtos_init_mutex(&_mutex);
  mico_rtos_init_semaphore(&_report_status_sem, 1);
  
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
  err = mico_rtos_create_thread(NULL, MICO_APPLICATION_PRIORITY, "UART Recv", uartRecv_thread, 0x500, (void*)inContext );
  require_noerr_action( err, exit, ha_log("ERROR: Unable to start the uart recv thread.") );
  
  /* wifi module status report thread */
  err = mico_rtos_create_thread(&_report_status_thread_handler, MICO_APPLICATION_PRIORITY, "Report", _report_status_thread, 0x500, (void*)inContext );
  require_noerr_action( err, exit, ha_log("ERROR: Unable to start the status report thread.") );

  /* Regisist notifications */
  err = MICOAddNotification( mico_notify_WIFI_STATUS_CHANGED, (void *)haNotify_WifiStatusHandler );
  require_noerr( err, exit );
  
  /*============================================================================
   * init cloud service interface
   *==========================================================================*/

  cloudServiceLibVersion = MicoCloudServiceVersion();
  ha_log("Mico Cloud Service library version: %d.%d.%d", (cloudServiceLibVersion >> 16)&0xFF, (cloudServiceLibVersion >>8)&0xFF, cloudServiceLibVersion&0xFF);
  
  /*start cloud service*/
  MicoCloudServiceInit(cloud_service_config);
  err = MicoCloudServiceStart();
  require_noerr_action( err, exit, ha_log("ERROR: Unable to start cloud service thread.") );
  
exit:
  return err;
}


void _report_status_thread(void *arg)
{
  mxchip_state_t cmd;
  mico_Context_t *inContext = (mico_Context_t*)arg;

  while(1){
    mico_rtos_get_semaphore(&_report_status_sem, MICO_WAIT_FOREVER);
    //write cloud info back to flash
    ha_log("report thread: cloud service status changed.");
    mico_rtos_lock_mutex(&inContext->flashContentInRam_mutex);
    inContext->appStatus.isCloudServiceConnected = is_network_state(CLOUD_CONNECT);
    inContext->flashContentInRam.appConfig.isActivated = cloudStatusInfo.isActivated;
    strncpy(inContext->flashContentInRam.appConfig.device_id, cloudStatusInfo.device_id, MAX_DEVICE_ID_STRLEN);
    strncpy(inContext->flashContentInRam.appConfig.master_device_key, cloudStatusInfo.master_device_key, MAX_DEVICE_KEY_STRLEN);
    mico_rtos_unlock_mutex(&inContext->flashContentInRam_mutex);
    MICOUpdateConfiguration(inContext);
    //report to USART device
    ha_log("report thread: send cloud service status to USART device.");
    _get_status(&cmd, inContext);
    MicoUartSend(UART_FOR_APP,(uint8_t *)&cmd, sizeof(mxchip_state_t));
  }
}


OSStatus haWlanCommandProcess(unsigned char *inBuf, int *inBufLen)
{
  ha_log_trace();
  OSStatus err = kUnknownErr;

  ha_log("Cloud => MCU:[%d]\t%.*s", *inBufLen, *inBufLen, inBuf);
  err = MicoUartSend(UART_FOR_APP, inBuf, *inBufLen);
  *inBufLen = 0;
  
  return err;
}


OSStatus haUartCommandProcess(uint8_t *inBuf, int inBufLen, mico_Context_t * const inContext)
{
  ha_log_trace();
  OSStatus err = kNoErr;
  int control = 0;
  mxchip_cmd_head_t *cmd_header = NULL;
  uint16_t cksum = 0;
  uint32_t com2net_data_len = 0;
  uint8_t *com2net_data = NULL;
  uint8_t *p = NULL;
  int datalen = 0;
  
  cmd_header = (mxchip_cmd_head_t *)inBuf;
  p = inBuf;
  
  datalen = p[6] + (p[7]<<8);
  if ((p[0] != 0xBB) || (p[1] != 0x00) ||
      ((datalen + HA_CMD_HEAD_SIZE + 2) != inBufLen) ||
      kNoErr != check_sum(inBuf, HA_CMD_HEAD_SIZE + datalen + 2)){
        //just transfer raw data to cloud
        goto data_transfer;
  }
  
  /* process format package */
  switch(cmd_header->cmd) {
  case CMD_COM2NET:
    cmd_header->cmd |= 0x8000;
    com2net_data_len = cmd_header->datalen;
    com2net_data = inBuf + HA_CMD_HEAD_SIZE;
    /* upload format data recived from uart to cloud */
    //ha_log("MCU => Cloud:[%d]\r\n%.*s", com2net_data_len, com2net_data_len, com2net_data);
    err = MicoCloudServiceUpload(com2net_data, com2net_data_len);
    break;
  case CMD_GET_STATUS:
    _get_status((mxchip_state_t*)inBuf, inContext);
    //ha_log("get status: [%d]\r\n%.*s",  sizeof(mxchip_state_t),  sizeof(mxchip_state_t), inBuf);
    err = MicoUartSend(UART_FOR_APP, inBuf, sizeof(mxchip_state_t));
    break;
  //module control cmd
  case CMD_CONTROL:
    if (cmd_header->datalen != 1)
      break;
    control = inBuf[8];
    switch(control) {
    case 1: 
      inContext->micoStatus.sys_state = eState_Software_Reset;
      require(inContext->micoStatus.sys_state_change_sem, exit);
      mico_rtos_set_semaphore(&inContext->micoStatus.sys_state_change_sem);
      break;
    case 2:
      MICORestoreDefault(inContext);
      inContext->micoStatus.sys_state = eState_Software_Reset;
      require(inContext->micoStatus.sys_state_change_sem, exit);
      mico_rtos_set_semaphore(&inContext->micoStatus.sys_state_change_sem);
      break;
    case 3:
      inContext->micoStatus.sys_state = eState_Wlan_Powerdown;
      require(inContext->micoStatus.sys_state_change_sem, exit);
      mico_rtos_set_semaphore(&inContext->micoStatus.sys_state_change_sem);
      break;
    case 4:
      break;
    case 5: 
      //micoWlanStartEasyLink(120);
      if(inContext->flashContentInRam.micoSystemConfig.configured == allConfigured){
        inContext->flashContentInRam.micoSystemConfig.configured = wLanUnConfigured;
        MICOUpdateConfiguration(inContext);
      }
      inContext->micoStatus.sys_state = eState_Software_Reset;
      require(inContext->micoStatus.sys_state_change_sem, exit);
      mico_rtos_set_semaphore(&inContext->micoStatus.sys_state_change_sem);
      break;
    case 6:
      err = MicoCloudServiceStart();
      require_noerr_action( err, exit, ha_log("ERROR: Unable to start cloud service thread, err=%d", err) );
      break;
    case 7:
      err = MicoCloudServiceStop();
      require_noerr_action( err, exit, ha_log("ERROR: Unable to stop cloud service thread, err=%d", err) );
      break;
    default:
      goto data_transfer;
      break;
    }
    // response
    cmd_header->cmd |= 0x8000;
    cmd_header->cmd_status = CMD_OK;
    cmd_header->datalen = 0;
    cksum = _calc_sum(inBuf, 8);
    inBuf[8] = cksum & 0x00ff;
    inBuf[9] = (cksum & 0x0ff00) >> 8;
    err = MicoUartSend(UART_FOR_APP, inBuf, 10);
    break;
  default:
    goto data_transfer;
    break;
  }
   
exit:
    return err;
    
data_transfer:
  ha_log("MCU => Cloud:[%d]\t%.*s", inBufLen, inBufLen, inBuf);
  err = MicoCloudServiceUpload(inBuf, inBufLen);
  return err;
}


OSStatus check_sum(void *inData, uint32_t inLen)  
{
  ha_log_trace();

  uint16_t *sum;
  uint8_t *p = (uint8_t *)inData;
  
  uint16_t checksum = 0;

  //return kNoErr;
  
  // TODO: real cksum
  p += inLen - 2;

  sum = (uint16_t *)p;
  //ha_log("sum=%d", *sum);

  checksum = _calc_sum(inData, inLen - 2);
  //ha_log("checksum=%d", checksum);
  
  if (checksum != *sum) {  // check sum error    
    return kChecksumErr;
  }
  return kNoErr;
}

uint16_t _calc_sum(void *inData, uint32_t inLen)
{
  ha_log_trace();
  uint32_t cksum = 0;
  uint16_t *p = inData;

  while (inLen > 1)
  {
    cksum += *p++;
    inLen -= 2;
  }
  if (inLen)
  {
    cksum += *(uint8_t *)p;
  }
  cksum = (cksum >> 16) + (cksum & 0xffff);
  cksum += (cksum >>16);

  return ~cksum;
}




