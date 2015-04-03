/**
  ******************************************************************************
  * @file    properties.h
  * @author  Eshen Wang
  * @version V0.1.0
  * @date    18-Mar-2015
  * @brief   device properties operations.
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
#define MAX_DEVICE_MANUFACTORY_SIZE  16

#define MAX_USER_UART_BUF_SIZE       512
   
// device info
 struct dev_info_t {
  char name[MAX_DEVICE_NAME_SIZE+1];
  char manufactory[MAX_DEVICE_MANUFACTORY_SIZE+1];
  
  uint32_t name_len;
  uint32_t manufactory_len;
};

// rgb led
 struct rgb_led_t {
  bool sw;
  int hues;
  int saturation;
  int brightness;
};

// adc
 struct adc_t {
  int data;
  bool event;   // event flag
};

// uart
 struct uart_t {
  char buf[MAX_USER_UART_BUF_SIZE];   // use a buffer to store data to send or received
  uint32_t data_len;                  // uart data len to send or received
  bool recv_event;                    //
};

#endif // __MICO_DEVICE_PROPERTIES_USER_H_
