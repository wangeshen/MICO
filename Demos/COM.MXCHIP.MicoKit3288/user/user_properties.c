/**
******************************************************************************
* @file    properties.c 
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
#include "properties.h"
#include "user_properties.h"
#include "JSON-C/json.h"
#include "rgb_led.h"
#include "user_uart.h"

#define properties_user_log(M, ...) custom_log("DEV_PROPERTIES_USER", M, ##__VA_ARGS__)
#define properties_user_log_trace() custom_log_trace("DEV_PROPERTIES_USER")

/*******************************************************************************
* PROPERTIES
*******************************************************************************/

// fixed data type length
uint32_t bool_len = sizeof(bool);
uint32_t int_len = sizeof(int);
uint32_t float_len = sizeof(float);

/************dev_info************************/

// dev_info data
struct dev_info_t dev_info = {
  .name = "MicoKit3288",
  .name_len = 11,
  .manufacturer = "MXCHIP",
  .manufacturer_len = 6
};

// dev_info set/get function
int string_get(struct mico_prop_t *prop, void *arg, void *val, uint32_t *val_len)
{
  int ret = 0;
  
  if((NULL == prop) || (NULL == val)){
    return -1;
  }
  
  // may read from flash if nessary
  uint32_t copy_len = ((*val_len > *(prop->value_len)) ? *(prop->value_len) : *val_len);
  //memset(val, '\0', *val_len);
  strncpy(val, (char*)prop->value, copy_len);
  *val_len = copy_len;
  
  properties_user_log("string_get: val=%s, val_len=%d.", (char*)val, *val_len);
  
  return ret;
}

int string_set(struct mico_prop_t *prop, void *arg, void *val, uint32_t val_len)
{
  int ret = 0;
  
  if(NULL == prop){
    return -1;
  }
  
  // write string (write to flash if nessary)
  uint32_t copy_len = ((val_len > prop->maxStringLen) ? prop->maxStringLen : val_len);
  memset(prop->value, '\0', prop->maxStringLen);
  strncpy((char*)prop->value, val, copy_len);
  *(prop->value_len) = copy_len;
  
  properties_user_log("string_set: val=%s, val_len=%d.", (char*)prop->value, *(prop->value_len));
  
  return ret;
}

/************rgb_led************************/

// led data
struct rgb_led_t rgb_led = {
  .sw = false,
  .hues = 0,
  .saturation = 0,
  .brightness = 0
};

// swtich function
int rgb_led_sw_set(struct mico_prop_t *prop, void *arg, void *val, uint32_t val_len)
{
  int ret = 0;
  bool set_sw_state = *((bool*)val);
  float color[3] = {0};
  properties_user_log("rgb_led_sw_set: val=%d, val_len=%d.", *((bool*)val), val_len);
  properties_user_log("h=%d, s=%d, b=%d", rgb_led.hues, rgb_led.saturation, rgb_led.brightness);
  // control hardware
  if(set_sw_state){
    properties_user_log("Open LED.");
    H2R_HSBtoRGB((float)rgb_led.hues, (float)rgb_led.saturation, (float)rgb_led.brightness, color);
    OpenLED_RGB(color);
  }
  else{
    properties_user_log("Close LED.");
    //CloseLED_RGB();   // bug ??? set bright = 0 instead for temp
    H2R_HSBtoRGB((float)rgb_led.hues, (float)rgb_led.saturation, (float)0, color);
    OpenLED_RGB(color);
  }
  
  return ret;
}

int rgb_led_sw_get(struct mico_prop_t *prop, void *arg, void *val, uint32_t *val_len)
{
  int ret = 0;
  // read hardware status, no hardware status to read, just return prop->value
  *((bool*)val) = *((bool*)prop->value);  
  *val_len = *(prop->value_len);
  
  properties_user_log("rgb_led_sw_get: val=%d, val_len=%d.", *((bool*)val), *((uint32_t*)val_len) );
  
  return ret;
}

// hues function
int rgb_led_hues_set(struct mico_prop_t *prop, void *arg, void *val, uint32_t val_len)
{
  int ret = 0;
  float color[3] = {0};
  int hues = *((int*)val);
  struct rgb_led_t *rgb_led = (struct rgb_led_t*)arg;
  properties_user_log("rgb_led_hues_set: val=%d, val_len=%d.", *((int*)val), val_len);
  
  // control hardware
  if(rgb_led->sw){
    H2R_HSBtoRGB((float)hues, (float)rgb_led->saturation, (float)rgb_led->brightness, color);
  }else
  {
    H2R_HSBtoRGB((float)hues, (float)rgb_led->saturation, (float)0, color);
  }
  OpenLED_RGB(color);
  
  return ret;
}

int rgb_led_hues_get(struct mico_prop_t *prop, void *arg, void *val, uint32_t *val_len)
{
  int ret = 0;
  // get hardware status, no hardware status to read, just return prop->value
  *((int*)val) = *((int*)prop->value);  
  *val_len = *(prop->value_len);
  
  properties_user_log("rgb_led_hues_get: val=%d, val_len=%d.", *((int*)val), *(uint32_t*)val_len );
  
  return ret;
}

// saturation function
int rgb_led_saturation_set(struct mico_prop_t *prop, void *arg, void *val, uint32_t val_len)
{
  int ret = 0;
  float color[3] = {0};
  int saturation = *((int*)val);
  struct rgb_led_t *rgb_led = (struct rgb_led_t*)arg;
  properties_user_log("rgb_led_saturation_set: val=%d, val_len=%d.", *((int*)val), val_len);
  
  // control hardware
  if(rgb_led->sw){
    H2R_HSBtoRGB((float)rgb_led->hues, (float)saturation, (float)rgb_led->brightness, color);
  }
  else{
    H2R_HSBtoRGB((float)rgb_led->hues, (float)saturation, (float)0, color);
  }
  OpenLED_RGB(color);
  
  return ret;
}

int rgb_led_saturation_get(struct mico_prop_t *prop, void *arg, void *val, uint32_t *val_len)
{
  int ret = 0;
  // get hardware status, no hardware status to read, just return prop->value
  *((int*)val) = *((int*)prop->value);  
  *val_len = *(prop->value_len);
  
  properties_user_log("rgb_led_saturation_get: val=%d, val_len=%d.", *((int*)val), *(uint32_t*)val_len );
  
  return ret;
}

// brightness function
int rgb_led_brightness_set(struct mico_prop_t *prop, void *arg, void *val, uint32_t val_len)
{
  int ret = 0;
  float color[3] = {0};
  int brightness = *((int*)val);
  struct rgb_led_t *rgb_led = (struct rgb_led_t*)arg;
  properties_user_log("rgb_led_brightness_set: val=%d, val_len=%d.", *((int*)val), val_len);
  
  // control hardware
  if(rgb_led->sw){
    H2R_HSBtoRGB((float)rgb_led->hues, (float)rgb_led->saturation, (float)brightness, color);
  }
  else{
    H2R_HSBtoRGB((float)rgb_led->hues, (float)rgb_led->saturation, (float)0, color);
  }
  OpenLED_RGB(color);
  
  return ret;
}

int rgb_led_brightness_get(struct mico_prop_t *prop, void *arg, void *val, uint32_t *val_len)
{
  int ret = 0;
  // get hardware status, no hardware status to read, just return prop->value
  *((int*)val) = *((int*)prop->value);  
  *val_len = *(prop->value_len);
  
  properties_user_log("rgb_led_brightness_get: val=%d, val_len=%d.", *((int*)val), *(uint32_t*)val_len );
  
  return ret;
}

/************adc************************/
// adc data
struct adc_t adc = {
  .data = 0,
  .event = true
};

// get adc data function
int adc_data_get(struct mico_prop_t *prop, void *arg, void *val, uint32_t *val_len)
{
  int ret = 0;
  uint16_t adc_data = 0;
  OSStatus err = kUnknownErr;
  
  // get ADC data
  err = MicoAdcTakeSample(MICO_ADC_1, &adc_data);
  if(kNoErr == err){
    *((uint16_t*)val) = adc_data;
    *val_len = sizeof(adc_data);
    ret = 0;   // get data succeed
  }
  else{
    ret = -1;  // get data error
  }
  
  return ret;
}

// notify check function of adc
int notify_check_adc_data(struct mico_prop_t *prop, void *arg, void *val, uint32_t *val_len)
{
  int ret = 0;
  uint16_t adc_data = 0;
  uint32_t adc_data_len = 0;
  
  // get adc data
  ret = prop->get(prop, arg, &adc_data, &adc_data_len);
  if(0 != ret){
    return -1;   // get value error
  }
  
  // update check (diff get_data and prop->value)
  //if(adc_data != *((uint16_t*)(prop->value))){  // changed
  if( (((int)adc_data - *((int*)(prop->value))) >= 10) || ((*((int*)(prop->value)) - (int)adc_data) >= 10) ){  // abs >=10
    properties_user_log("adc_data changed: %d -> %d", *((int*)prop->value), (int)adc_data);   
    ret = 1;  // value changed, need to send notify message
  }
  else{
    ret = 0;  // not changed, not need to notify
  }
  
  // return new value to update prop value && len
  *((int*)val) = (int)adc_data;  
  *val_len = adc_data_len;
  
  return ret;
}

int event_status_get(struct mico_prop_t *prop, void *arg, void *val, uint32_t *val_len)
{
  // get event value
  *(bool*)val = *((bool*)prop->value);
  *val_len = *(prop->value_len);
  
  return 0;  // get ok
}

int event_status_set(struct mico_prop_t *prop, void *arg, void *val, uint32_t val_len)
{
  // set event value
  *((bool*)prop->value) = *((bool*)val);
  *(prop->value_len) = val_len;
  
  return 0;  // get ok
}

/******************* uart for user ***************/
struct uart_t user_uart = {
  .rx_buf = {0},
  .rx_data_len = 0,
  .rx_event = true   // true for always recv data
};

// get: recv uart data
int uart_data_recv(struct mico_prop_t *prop, void *arg, void *val, uint32_t *val_len)
{
  int ret = 0;  
  uint32_t recv_len = 0;
  
  if((NULL == prop) || (NULL == val)){
    return -1;
  }
  
  // recv data from uart
  memset(val, '\0', prop->maxStringLen);
  recv_len = user_uartRecv((unsigned char*)val, MAX_USER_UART_BUF_SIZE);
  if(recv_len > 0){
    *val_len = recv_len;
    ret = 0;
    properties_user_log("uart_data_recv: val=%s, val_len=%d.", (char*)val, *val_len);
  }
  else{
    *val_len = 0;
    ret = -1;
  }
  
  return ret;
}

// set: send data to uart
int uart_data_send(struct mico_prop_t *prop, void *arg, void *val, uint32_t val_len)
{
  int ret = 0;
  OSStatus err = kUnknownErr;
  uint32_t send_len = 0;
  
  if(NULL == prop){
    return -1;
  }
  
  // write string (write to flash if nessary)
  send_len = ((val_len > prop->maxStringLen) ? prop->maxStringLen : val_len);
  err = user_uartSend((unsigned char*)val, send_len);
  if(kNoErr == err){
    properties_user_log("uart_data_send: val=%s, val_len=%d.", (char*)val, val_len);
    ret = 0;  // set ok
  }
  else{
   ret = -1;
  }
  
  return ret;
}

// notify_check: check uart data recv
int uart_data_recv_check(struct mico_prop_t *prop, void *arg, void *val, uint32_t *val_len)
{
  int ret = 0;
  
  // get adc data
  ret = prop->get(prop, arg, val, val_len);
  if(0 != ret){
    return -1;   // get value error
  }
  
  // update check (uart data recieved)
  ret = 1;
  
  // return new value to update prop value && len
//  *((int*)val) = (int)adc_data;  
//  *val_len = adc_data_len;
  
  return ret;
}

/*******************************************************************************
* service_table: list all serivices && properties for the device
 ******************************************************************************/
const struct mico_service_t  service_table[] = {
  [0] = {
    .type = "public.map.service.dev_info",    // service 1: dev_info (uuid)
    .properties = {
      [0] = {
        .type = "public.map.property.name",  // device name uuid
        .value = &(dev_info.name),
        .value_len = &(dev_info.name_len),
        .format = MICO_PROP_TYPE_STRING,
        .perms = (MICO_PROP_PERMS_RO | MICO_PROP_PERMS_WO),
        .get = string_get,                  // get string func to get device name
        .set = string_set,                  // set sring func to change device name
        .notify_check = NULL,               // not notifiable
        .arg = &(dev_info.name),            // get/set string pointer (device name)
        .event = NULL,                      // not notifiable
        .hasMeta = false,                   // no max/min/step
//        .maxValue.intValue = 0,
//        .minValue.intValue = 0,
//        .minStep.intValue = 0,  
        .maxStringLen = MAX_DEVICE_NAME_SIZE,  // max length of device name string
        .unit = NULL                        // no unit
      },
      [1] = {
        .type = "public.map.property.manufacturer",  // device manufacturer uuid
        .value = &(dev_info.manufacturer),
        .value_len = &(dev_info.manufacturer_len),
        .format = MICO_PROP_TYPE_STRING,
        .perms = MICO_PROP_PERMS_RO,
        .get = string_get,                  // get string func to get manufacturer
        .set = NULL      ,                  // set sring func to change manufacturer
        .notify_check = NULL,               // not notifiable
        .arg = &(dev_info.manufacturer),
        .event = NULL,                      // not notifiable
        .hasMeta = false,
//        .maxValue.intValue = 0,
//        .minValue.intValue = 0,
//        .minStep.intValue = 0,  
        .maxStringLen = MAX_DEVICE_MANUFACTURER_SIZE,  // max length of device manufacturer
        .unit = NULL                        // no unit
      },
      [2] = {NULL}                          // end flag
    }
  },
  [1] = {
    .type = "public.map.service.rgb_led",       // service 2: rgb led (uuid)
    .properties = {
      [0] = {
        .type = "public.map.property.switch",  // led switch uuid
        .value = &(rgb_led.sw),
        .value_len = &bool_len,             // bool type len
        .format = MICO_PROP_TYPE_BOOL,
        .perms = (MICO_PROP_PERMS_RO | MICO_PROP_PERMS_WO),
        .get = rgb_led_sw_get,              // get led switch status function
        .set = rgb_led_sw_set,              // set led switch status function
        .notify_check = NULL,               // not notifiable
        .arg = &rgb_led,                   // led data
        .event = NULL,
        .hasMeta = false,
//        .maxValue.intValue = 0,
//        .minValue.intValue = 0,
//        .minStep.intValue = 0,     
        .maxStringLen = 0,
        .unit = NULL
      },
      [1] = {
        .type = "public.map.property.hues",  // led hues
        .value = &(rgb_led.hues),
        .value_len = &int_len,   // int type len
        .format = MICO_PROP_TYPE_INT,
        .perms = (MICO_PROP_PERMS_RO | MICO_PROP_PERMS_WO),
        .get = rgb_led_hues_get,
        .set = rgb_led_hues_set,
        .notify_check = NULL,               // not notifiable
        .arg = &rgb_led,  // led hues value
        .event = NULL,
        .hasMeta = true,
        .maxValue.intValue = 360,
        .minValue.intValue = 0,
        .minStep.intValue = 1,
        .maxStringLen = 0,
        .unit = "degree"
      },
      [2] = {
        .type = "public.map.property.saturation",  // led saturation
        .value = &(rgb_led.saturation),
        .value_len = &int_len,
        .format = MICO_PROP_TYPE_INT,
        .perms = (MICO_PROP_PERMS_RO | MICO_PROP_PERMS_WO),
        .get = rgb_led_saturation_get,
        .set = rgb_led_saturation_set,
        .notify_check = NULL,               // not notifiable
        .arg = &rgb_led,  // led saturation value
        .event = NULL,
        .hasMeta = true,
        .maxValue.intValue = 100,
        .minValue.intValue = 0,
        .minStep.intValue = 1,
        .maxStringLen = 0,
        .unit = "percentage"
      },
      [3] = {
        .type = "public.map.property.brightness",  // led brightness
        .value = &(rgb_led.brightness),
        .value_len = &int_len,
        .format = MICO_PROP_TYPE_INT,
        .perms = (MICO_PROP_PERMS_RO | MICO_PROP_PERMS_WO),
        .get = rgb_led_brightness_get,
        .set = rgb_led_brightness_set,
        .arg = &rgb_led,  // led brightness value
        .event = NULL,
        .hasMeta = true,
        .maxValue.intValue = 100,
        .minValue.intValue = 0,
        .minStep.intValue = 1,
        .maxStringLen = 0,
        .unit = "percentage"
      },
      [4] = {NULL}
    }
  },
  [2] = {
    .type = "public.map.service.adc",   //  service 3: ADC (uuid)
    .properties = {
      [0] = {
        .type = "public.map.property.value",  // adc value uuid
        .value = &(adc.data),
        .value_len = &int_len,
        .format = MICO_PROP_TYPE_INT,
        .perms = (MICO_PROP_PERMS_RO | MICO_PROP_PERMS_EV),
        .get = adc_data_get,
        .set = NULL,
        .notify_check = notify_check_adc_data,  // check notify for adc data
        .arg = &adc,         // adc sample data
        .event = &(adc.event),      // event flag
        .hasMeta = true,
        .maxValue.intValue = 4095,
        .minValue.intValue = 0,
        .minStep.intValue = 1,
        .maxStringLen = 0,
        .unit = NULL
      },
      [1] = {
        .type = "public.map.property.event",  // adc value event (uuid)
        .value = &(adc.event),
        .value_len = &bool_len,
        .format = MICO_PROP_TYPE_BOOL,
        .perms = (MICO_PROP_PERMS_RO | MICO_PROP_PERMS_WO),
        .get = event_status_get,
        .set = event_status_set,
        .notify_check = NULL,
        .arg = &adc,           // adc data
        .event = NULL,
        .hasMeta = false,
//        .maxValue.intValue = 0,
//        .minValue.intValue = 0,
//        .minStep.intValue = 0,
        .maxStringLen = 0,
        .unit = NULL
      },
      [2] = {NULL}
    }
  },
  [3] = {
    .type = "public.map.service.uart",   //  service 3: ADC (uuid)
    .properties = {
      [0] = {
        .type = "public.map.property.message",  // uart message uuid
        .value = &(user_uart.rx_buf),
        .value_len = &(user_uart.rx_data_len),
        .format = MICO_PROP_TYPE_STRING,
        .perms = (MICO_PROP_PERMS_RO | MICO_PROP_PERMS_WO | MICO_PROP_PERMS_EV),
        .get = uart_data_recv,
        .set = uart_data_send,
        .notify_check = uart_data_recv_check,   // check recv data
        .arg = &user_uart,
        .event = &(user_uart.rx_event),       // event flag
        .hasMeta = false,
//        .maxValue.intValue = 4095,
//        .minValue.intValue = 0,
//        .minStep.intValue = 1,
        .maxStringLen = MAX_USER_UART_BUF_SIZE,
        .unit = "byte"
      },
      [1] = {NULL}
    },
  },
  [4] = {NULL}
};
