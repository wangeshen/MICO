/**
  ******************************************************************************
  * @file    user_properties.h
  * @author  Eshen Wang
  * @version V1.0.0
  * @date    18-Mar-2015
  * @brief   device properties data.
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

#ifndef __MICO_DEVICE_PROPERTIES_USER_H_
#define __MICO_DEVICE_PROPERTIES_USER_H_

#include "MICODefine.h"

/*******************************************************************************
 * DEFINES
 ******************************************************************************/
#define MAX_DEVICE_NAME_SIZE            16
#define MAX_DEVICE_MANUFACTURER_SIZE    16
#define MAX_USER_UART_BUF_SIZE          512
 
/*******************************************************************************
 * USER CONTEXT
 ******************************************************************************/

// user module config params in flash
typedef struct _user_config_t {
  // dev_info
  char dev_name[MAX_DEVICE_NAME_SIZE+1];
  uint32_t dev_name_len; 
  char dev_manufacturer[MAX_DEVICE_MANUFACTURER_SIZE+1];
  uint32_t dev_manufacturer_len;
  
  // rgb led
  bool rgb_led_sw;
  int rgb_led_hues;
  int rgb_led_saturation;
  int rgb_led_brightness;
  
  // light sensor (ADC1_4)
  bool light_sensor_event;                    // upload event flag
    
  // infrared_reflective sensor (ADC1_1)
  bool infrared_reflective_event;             // upload event flag
  
  // uart
  bool uart_rx_event;                         // recv notify flag
  
}user_config_t;

// user module status
typedef struct _user_status_t {
  bool user_config_need_update;               // if set, user context config need to write back to flash.
  
  //  light sensor (ADC1_4)
  int light_sensor_data;
  
  // infrared reflective sensor
  int infrared_reflective_data;
  
  // uart
  char uart_rx_buf[MAX_USER_UART_BUF_SIZE];   // use a buffer to store data received
  uint32_t uart_rx_data_len;                  // uart data len received
}user_status_t;

// user context
typedef struct _user_context_t {
  user_config_t config;                       // config params in flash
  mico_mutex_t config_mutex;                  // mutex for write flash
  
  user_status_t status;                       // running status
}user_context_t;

#endif // __MICO_DEVICE_PROPERTIES_USER_H_
