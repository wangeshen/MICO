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
//#include "drivers/rgb_led.h"
#include "drivers/p9813.h"
#include "drivers/oled.h"

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

volatile bool dc_motor_runnig = false;
USED void PlatformKey2ButtonClickedCallback(void)
{
  user_log_trace();
  user_log("PlatformKey2ButtonClickedCallback");
  
  if(dc_motor_runnig){
    dc_motor_runnig = false;
  }
  else{
    dc_motor_runnig = true;
  }
  userDCMotor(false);
  return;
}

USED void PlatformKey2ButtonLongPressedCallback(void)
{
  user_log_trace();
  user_log("PlatformKey2ButtonLongPressedCallback");
   userDCMotor(true);
  return;
}

/* user main function, called by AppFramework after FogCloud connected.
 */
OSStatus user_main( mico_Context_t * const mico_context )
{
  user_log_trace();
  OSStatus err = kNoErr;
  int32_t bme280_temp = 0;
  uint32_t bme280_hum = 0;
  uint32_t bme280_press = 0;
  char temp_hum_str[32];
  
  uint16_t blue = 0;
  uint16_t green = 0;
  uint16_t red = 0;
  int i = 0;
  
  uint16_t light_data = 0;
  uint16_t infared_data = 0;
  
  require_action(mico_context, exit, err = kParamErr);
  
  /* read user config params from flash */
//  if(NULL == user_context.config_mutex){
//    mico_rtos_init_mutex(&user_context.config_mutex);
//  }
//  err = userParams_Read(mico_context, &user_context);
//  require_noerr_action( err, exit, user_log("ERROR: read user param from flash failed! err=%d.", err) );
//  
  /* user modules init */
  // uart init
  //err = user_uartInit(mico_context);
  //require_noerr_action( err, exit, user_log("ERROR: user uart init failed! err = %d.", err) );
  
  // DC Motor pin
  MicoGpioInitialize( (mico_gpio_t)DC_MOTOR_PIN, OUTPUT_PUSH_PULL );
  
  // Light sensor (ADC1_4) init
  err = MicoAdcInitialize(MICO_ADC_1, 3);
  require_noerr_action( err, exit, user_log("ERROR: MicoAdcInitialize ADC1_4 err = %d.", err) );
  
  // Light sensor (ADC1_1) init
//  err = MicoAdcInitialize(MICO_ADC_2, 3);
//  require_noerr_action( err, exit, user_log("ERROR: MicoAdcInitialize ADC1_1 err = %d.", err) );
//  
  // OLED init
  OLED_Init();
  OLED_Clear();
  OLED_ShowString(0,0,"   M X C H I P");  
  
  // Environmental Sensor init
  err = bme280_sensor_init();
  require_noerr_action( err, exit, user_log("ERROR: bme280_sensor_init err = %d.", err) );
  
    // RGB LED init
//  if(user_context.config.rgb_led_sw){
//    rgb_led_open((float)(user_context.config.rgb_led_hues), 
//                 (float)(user_context.config.rgb_led_saturation), 
//                 (float)(user_context.config.rgb_led_brightness));
//  }
//  else{
//    rgb_led_close();
//  }
  
  /* new rgb led */
  //rgb_led_init();
  //rgb_led_open(0,0,0);
  
  /* start properties notify task */
 // err = mico_start_properties_notify(mico_context, service_table, 
 //                                    MICO_PROPERTIES_NOTIFY_INTERVAL_MS, 
 //                                    STACK_SIZE_NOTIFY_THREAD);
 // require_noerr_action( err, exit, user_log("ERROR: mico_start_properties_notify err = %d.", err) );
    
  /* main loop */
  while(1){
    
    /*---------- update config params in flash ---------------*/
//    if(user_params_need_update){
//      err = userParams_Update(mico_context, &user_context);
//      if(kNoErr == err){
//        user_params_need_update = false;
//      }
//      else{
//        user_log("ERROR: update user config into flash failed! err = %d.", err);
//      }
//    }
    
    mico_thread_msleep(1000);
    
    /*---------------------- sensor data -------------------*/
    
    // H/T/P
    err = bme280_data_readout(&bme280_temp, &bme280_press, &bme280_hum);
    user_log("BME280: nT=%d, nH=%d, nP=%d", bme280_temp, bme280_hum, bme280_press);
    sprintf(temp_hum_str, "T: %d C   H: %d%%", bme280_temp/100,  bme280_hum/1024);
    user_log("BME280: %s", temp_hum_str);
    
    // OLED display
    OLED_ShowString(2,3,(uint8_t*)DEFAULT_DEVICE_NAME); 
    OLED_ShowString(0,6,(uint8_t*)temp_hum_str);
    
    // RGB LED
    if(0 == i){
      blue = 0;
      green = 0;
      red = 255;
    }
    else if(1 == i){
      blue = 0;
      green = 255;
      red = 0;
    }
    else if(2 == i){
      blue = 255;
      green = 0;
      red = 0;
    }
    i++;
    if(i >= 3){
      i=0;
    }
    //rgb_led_open(blue,green,red);
    
    // Light (ADC1_4) test
    err = MicoAdcTakeSample(MICO_ADC_1, &light_data);
    if(kNoErr == err){
      user_log("Light(ADC1_4): %d", light_data);
    }
    
    // Infrared Reflective (ADC1_1) test
//    err = MicoAdcTakeSample(MICO_ADC_2, &infared_data);
//    if(kNoErr == err){
//      user_log("Infared(ADC1_1): %d", infared_data);
//    }
    
  }  // while
  
  // never getting here only if fatal error.
exit:
  bme280_sensor_deinit();
  user_log("ERROR: user_main exit, err = %d.", err);  // reboot ???
  return err;
}
