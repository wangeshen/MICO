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
#include "drivers/hsb2rgb_led.h"
#include "drivers/rgb_led.h"
#include "drivers/bme280_user.h"
#include "drivers/oled.h"
#include "drivers/DHT11.h"
#include "drivers/light_sensor.h"
#include "drivers/infrared_reflective.h"
#include "drivers/dc_motor.h"
#include "drivers/keys.h"

#define user_log(M, ...) custom_log("USER", M, ##__VA_ARGS__)
#define user_log_trace() custom_log_trace("USER")

/* properties defined in user_properties.c by user
 */
// device services &&¡¡properties table
extern struct mico_service_t  service_table[];
// user params
extern user_context_t g_user_context;

// global flag to indacate update user params in flash
//volatile bool user_params_need_update = false;


/* MICO user callback: Restore default configuration provided by user
 * called when Easylink buttion long pressed
 */
void userRestoreDefault_callback(mico_Context_t *mico_context)
{
  user_log("INFO: restore user configuration.");
  userParams_RestoreDefault(mico_context, &g_user_context);
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
      g_user_context.status.user_config_need_update = true;  // user params need update in flash
    }
  }
  
  return err;
}

/*---------------------------- user function ---------------------------------*/

OSStatus user_modules_init(void)
{
  OSStatus err = kUnknownErr;
  
  // init user key1 && key2
  user_key1_init();
  user_key2_init();
  
  // init OLED
  OLED_Init();
  
  // init RGB LED(P9813)
  hsb2rgb_led_init();
  
  // init DC Motor(GPIO)
  dc_motor_init();
  
  // init Light sensor(ADC)
  light_sensor_init();
  
  // init infrared sensor(ADC)
  infrared_reflective_init();
  
  // init T/H/P sensor(bme280)
  err = bme280_sensor_init();
  require_noerr_action( err, exit, user_log("ERROR: bme280_sensor_init err=%d.", err) );
  
  // init DHT11
  DHT11_init();
  
  // init proximity sensor(I2C)
 
exit:
  return err;
}

OSStatus user_settings_recovery(mico_Context_t *mico_context, user_context_t *user_context)
{
  OSStatus err = kNoErr;
  
  /* read user config from flash */
  if(NULL == user_context->config_mutex){
    err = mico_rtos_init_mutex(&user_context->config_mutex);
    require_noerr_action( err, exit, user_log("ERROR: mico_rtos_init_mutex (user_context->config_mutex) err=%d.", err) );
  }
  err = userParams_Read(mico_context, user_context);
  require_noerr_action( err, exit, user_log("ERROR: userParams_Read err=%d.", err) );
  
  /* reset user status */
  user_context->status.user_config_need_update = false;
  user_context->status.light_sensor_data = 0;
  user_context->status.uart_rx_data_len = 0;
  
  /* set initial state of user modules */ 
  // OLED
  OLED_Clear();
  OLED_ShowString(20,0,"M X C H I P");
  OLED_ShowString(20,3,(uint8_t*)DEFAULT_DEVICE_NAME); 
  OLED_ShowString(8,6,"T: 0C  H: 0%");
  
  // RGB LED
  hsb2rgb_led_open(user_context->config.rgb_led_hues,
                   user_context->config.rgb_led_saturation,
                   user_context->config.rgb_led_brightness);
  rgb_led_open(0, 0, 255);  // red for test
  
  // DC Motor
  dc_motor_set(user_context->config.dc_motor_switch);
  
exit:  
  return err;
}

OSStatus user_settings_update(mico_Context_t *mico_context, user_context_t *user_context)
{
  OSStatus err = kUnknownErr;
  
  if(user_context->status.user_config_need_update){
    err = userParams_Update(mico_context, user_context);
    if(kNoErr == err){
      user_context->status.user_config_need_update = false;
    }
    else{
      user_log("ERROR: userParams_Update err = %d.", err);
    }
  }
  else{
    err = kNoErr;
  }
  
  return err;
}

void user_running(user_context_t *user_context)
{
//    OSStatus err = kUnknownErr;
//  uint8_t dht11_data[4] = {0};
//  int ret = 0;
//  int dht11_temp = 0;
//  int dht11_hum = 0;
//  char temp_hum_str[64] = {0};
  
  //  uint16_t light_data = 0;
//  uint16_t infrared_data = 0;
  
//  int32_t bme280_temp_data = 0;
//  uint32_t bme280_hum_data = 0;
//  uint32_t bme280_pressure_data = 0;
//  char bme280_temp_hum_str[128] = {0};

  
//  user_log(">>>>>>>>>>>>>DHT11");
//  ret = DHT11_read(dht11_data);
//  user_log(">>>>>>>>>>>>>DHT11 ret=%d", ret);
//  
//  dht11_hum = dht11_data[0];
//  dht11_temp = dht11_data[2];
//    
//  memset(temp_hum_str, 0, sizeof(temp_hum_str));
//  sprintf(temp_hum_str, "T: %dC  H: %d%%", dht11_temp, dht11_hum);
//  user_log("T=%d, H=%d.", dht11_temp, dht11_hum);
//  OLED_ShowString(8,3,(uint8_t*)temp_hum_str);
  

//  light_sensor_read(&light_data);
//  user_log("Light D=%d.", light_data);
//  
//  infrared_reflective_read(&infrared_data);
//  user_log("Infrared D=%d.", infrared_data);
  
  
//  err = bme280_data_readout(&bme280_temp_data, &bme280_pressure_data, &bme280_hum_data);
//  if(kNoErr != err){
//    user_log("ERROR: bme280_data_readout err=%d. ", err);
//  }
//  else{
//    sprintf(bme280_temp_hum_str, "T: %dC  H: %d%%", bme280_temp_data/100,  bme280_hum_data/1024);
//    // display H/T on OLED
//    OLED_ShowString(8,6,(uint8_t*)bme280_temp_hum_str);
//    user_log("bme280 T=%d, H=%d, P=%d.", bme280_temp_data/100, bme280_hum_data/1024, bme280_pressure_data/256);
//  }
    
      
  // display H/T on OLED
  char temp_hum_str[64] = {0};
  sprintf(temp_hum_str, "T: %dC  H: %d%%", 
          user_context->status.temperature, user_context->status.humidity);
  user_log("T=%d, H=%d.",  
           user_context->status.temperature, user_context->status.humidity);
  
  OLED_ShowString(5,6,(uint8_t*)temp_hum_str);
}

/* user main function, called by AppFramework after FogCloud connected.
 */
OSStatus user_main( mico_Context_t * const mico_context )
{
  user_log_trace();
  OSStatus err = kUnknownErr;
  //user_context_t* p_user_context = &user_context;
  
  /* init user modules (pins && sensor init)*/
  err = user_modules_init();
  require_noerr_action( err, exit, user_log("ERROR: user_modules_init err=%d.", err) );
  
  /* recovery user settings from flash && set initail state of user modules */
  err = user_settings_recovery(mico_context, &g_user_context);
  require_noerr_action( err, exit, user_log("ERROR: user_settings_recovery err=%d.", err) );
  
  /* start properties notify task */
  err = mico_start_properties_notify(mico_context, service_table, 
                                     MICO_PROPERTIES_NOTIFY_INTERVAL_MS, 
                                     STACK_SIZE_NOTIFY_THREAD);
  require_noerr_action( err, exit, user_log("ERROR: mico_start_properties_notify err = %d.", err) );
  
  while(1){
    /* save user settings into flash */
    err = user_settings_update(mico_context, &g_user_context);
    require_noerr_action( err, exit, user_log("ERROR: user_settings_update err=%d.", err) );
    
    /* user thread running state */
    user_running(&g_user_context);
    
    /* check every 1 seconds */
    mico_thread_msleep(1000);
  }
  
exit:
  user_log("ERROR: user_main exit with err=%d", err);
  return err;
}

/*---------------------------------- old -------------------------------------*/
//  user_log_trace();
//  OSStatus err = kNoErr;
//  int32_t bme280_temp = 0;
//  uint32_t bme280_hum = 0;
//  uint32_t bme280_press = 0;
//  char temp_hum_str[32];
//  
//  float hue = 0;
//  float sat = 0;
//  float bri = 0;
//  int i = 0;
//  
//  uint16_t light_data = 0;
//  uint16_t infared_data = 0;
//  
//  require_action(mico_context, exit, err = kParamErr);
//  
//  /* read user config params from flash */
//  if(NULL == user_context.config_mutex){
//    mico_rtos_init_mutex(&user_context.config_mutex);
//  }
//  err = userParams_Read(mico_context, &user_context);
//  require_noerr_action( err, exit, user_log("ERROR: read user param from flash failed! err=%d.", err) );
//  
//  /* user modules init */
//  // uart init
//  err = user_uartInit(mico_context);
//  require_noerr_action( err, exit, user_log("ERROR: user uart init failed! err = %d.", err) );
//  
//  // DC Motor pin
//  //MicoGpioInitialize( (mico_gpio_t)DC_MOTOR, OUTPUT_PUSH_PULL );
//  
//  // Light sensor (ADC1_4) init
//  //err = MicoAdcInitialize(MICO_ADC_1, 3);
//  //require_noerr_action( err, exit, user_log("ERROR: MicoAdcInitialize ADC1_4 err = %d.", err) );
//  
//  // Light sensor (ADC1_1) init
//  //err = MicoAdcInitialize(MICO_ADC_2, 3);
//  //require_noerr_action( err, exit, user_log("ERROR: MicoAdcInitialize ADC1_1 err = %d.", err) );
//  
//  // OLED init
//  OLED_Init();
//  OLED_Clear();
//  OLED_ShowString(10,0,"M X C H I P");
//  OLED_ShowString(8,3,(uint8_t*)DEFAULT_DEVICE_NAME); 
//  OLED_ShowString(6,6,"T: 25 C, H: 49%");
//  
//  /* new rgb led */
//  hsb2rgb_led_init();
//  //hsb2rgb_led_open(240,100,100);
//  if(user_context.config.rgb_led_sw){
//    hsb2rgb_led_open((float)(user_context.config.rgb_led_hues), 
//                 (float)(user_context.config.rgb_led_saturation), 
//                 (float)(user_context.config.rgb_led_brightness));
//  }
//  else{
//    rgb_led_close();
//  }
//  rgb_led_open(0,0,0);  // close
//  
//  // Environmental Sensor init
//  //err = bme280_sensor_init();
//  //require_noerr_action( err, exit, user_log("ERROR: bme280_sensor_init err = %d.", err) );
//  
//    // RGB LED init
////  if(user_context.config.rgb_led_sw){
////    rgb_led_open((float)(user_context.config.rgb_led_hues), 
////                 (float)(user_context.config.rgb_led_saturation), 
////                 (float)(user_context.config.rgb_led_brightness));
////  }
////  else{
////    rgb_led_close();
////  }
//  
//  
//  /* start properties notify task */
//  err = mico_start_properties_notify(mico_context, service_table, 
//                                     MICO_PROPERTIES_NOTIFY_INTERVAL_MS, 
//                                     STACK_SIZE_NOTIFY_THREAD);
//  require_noerr_action( err, exit, user_log("ERROR: mico_start_properties_notify err = %d.", err) );
//    
//  /* main loop */
//  while(1){
//    
//    /*---------- update config params in flash ---------------*/
//    if(user_params_need_update){
//      err = userParams_Update(mico_context, &user_context);
//      if(kNoErr == err){
//        user_params_need_update = false;
//      }
//      else{
//        user_log("ERROR: update user config into flash failed! err = %d.", err);
//      }
//    }
//    
//    mico_thread_msleep(1000);
//    
//    /*---------------------- sensor data -------------------*/
//
///*    
//    // H/T/P
////    err = bme280_data_readout(&bme280_temp, &bme280_press, &bme280_hum);
////    user_log("BME280: nT=%d, nH=%d, nP=%d", bme280_temp, bme280_hum, bme280_press);
////    sprintf(temp_hum_str, "T: %d C   H: %d%%", bme280_temp/100,  bme280_hum/1024);
////    user_log("BME280: %s", temp_hum_str);
//    
//    // OLED display
//    //OLED_ShowString(0,6,(uint8_t*)temp_hum_str);
//*/    
//    // RGB LED
//    if(0 == i){
//      hue = 0;
//      sat = 100;
//      bri = 100;
//      rgb_led_open(254,0,0);  // blue
//    }
//    else if(1 == i){
//      hue = 120;
//      sat = 100;
//      bri = 100;
//      rgb_led_open(0,255,0);  // green
//    }
//    else if(2 == i){
//      hue = 240;
//      sat = 100;
//      bri = 100;
//      rgb_led_open(0,0,50);  // red
//    }
//    i++;
//    if(i >= 3){
//      i=0;
//    }
//    //rgb_led_open(hue, sat, bri);
///*    
//    // Light (ADC1_4) test
////    err = MicoAdcInitialize(MICO_ADC_1, 3);
////    //require_noerr_action( err, exit, user_log("ERROR: MicoAdcInitialize ADC1_4 err = %d.", err) );
////    err = MicoAdcTakeSample(MICO_ADC_1, &light_data);
////    if(kNoErr == err){
////      user_log("Light(ADC1_4): %d", light_data);
////    }
////    
////    // Infrared Reflective (ADC1_1) test
////    err = MicoAdcInitialize(MICO_ADC_2, 3);
////    //require_noerr_action( err, exit, user_log("ERROR: MicoAdcInitialize ADC1_1 err = %d.", err) );
////    err = MicoAdcTakeSample(MICO_ADC_2, &infared_data);
////    if(kNoErr == err){
////      user_log("Infared(ADC1_1): %d", infared_data);
////    }
//
//    */    
//  }  // while
//  
//  // never getting here only if fatal error, then system will reboot.
//exit:
//  user_log("ERROR: user_main exit, err = %d.", err);
//  return err;

