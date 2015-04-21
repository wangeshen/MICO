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

#include "MICODefine.h"

#ifndef __MICO_DEVICE_PROPERTIES_USER_H_
#define __MICO_DEVICE_PROPERTIES_USER_H_

/*******************************************************************************
 * MODULE DATA DEFINE
 ******************************************************************************/

#define MAX_DEVICE_NAME_SIZE         16
#define MAX_DEVICE_MANUFACTURER_SIZE  16

#define MAX_USER_UART_BUF_SIZE       512
   
// device info
 typedef struct _dev_info_t {
  char name[MAX_DEVICE_NAME_SIZE+1];
  char manufacturer[MAX_DEVICE_MANUFACTURER_SIZE+1];
  
  uint32_t name_len;
  uint32_t manufacturer_len;
}dev_info_t;

// rgb led
 typedef struct _rgb_led_t {
  bool sw;
  int hues;
  int saturation;
  int brightness;
}rgb_led_t;

// adc
 typedef struct _adc_t {
  int data;
  bool event;   // event flag
}adc_t;

// uart
typedef struct _uart_t {
  char rx_buf[MAX_USER_UART_BUF_SIZE];   // use a buffer to store data received
  uint32_t rx_data_len;                  // uart data len received
  bool rx_event;                       // recv notify flag
}uart_t;

typedef struct _user_context_t {
  dev_info_t dev_info;
  rgb_led_t rgb_led;
  adc_t adc;
  uart_t uart;
}user_context_t;

#endif // __MICO_DEVICE_PROPERTIES_USER_H_
