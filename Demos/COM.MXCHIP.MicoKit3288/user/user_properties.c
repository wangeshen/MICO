/**
******************************************************************************
* @file    user_properties.c 
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
#include "properties.h"
#include "user_properties.h"
#include "JSON-C/json.h"
#include "drivers/hsb2rgb_led.h"
#include "drivers/uart.h"

#define properties_user_log(M, ...) custom_log("DEV_PROPERTIES_USER", M, ##__VA_ARGS__)
#define properties_user_log_trace() custom_log_trace("DEV_PROPERTIES_USER")

/*******************************************************************************
* PROPERTIES
*******************************************************************************/

// system data type length(fixed)
uint32_t bool_len = sizeof(bool);
uint32_t int_len = sizeof(int);
uint32_t float_len = sizeof(float);

/*******************************************************************************
 * user context
 * context.config: user property data, stored in flash extra param area
 * context.status: user property status
 ******************************************************************************/
user_context_t user_context = {
  .config.light_sensor_event = true,
  .config.uart_rx_event = true,
};

/*******************************************************************************
 * DESCRIPTION: get/set/notify_check function defined for each property;
 * NOTE:        a property must has get/set function to read/write property, and
 *              must has notify_check function, if property need notify.
 *              get: get && return hardware status, reutrn 0 if succeed.
 *              set: set hardware, return 0 if operation succeed.
 *              notify_check: check property changes, returned changed value && set notify.
 * RETURN:
 *              = 0: get/set success;
 *              = 1: returned by notify_check function, property need to update;
 *              < 0: operation failed.
 ******************************************************************************/

/*
 * MODULE: dev_info 
 */

// (COMMON FUNCTION) string property read function
int string_get(struct mico_prop_t *prop, void *arg, void *val, uint32_t *val_len)
{
  int ret = 0;
  
  if((NULL == prop) || (NULL == val)){
    return -1;
  }
  
  // no hardware operation, just return ok.
  
  properties_user_log("string_get: val=%s, val_len=%d.", (char*)val, *val_len);
  
  return ret;  // return 0, succeed.
}

// (COMMON FUNCTION) string property write function
int string_set(struct mico_prop_t *prop, void *arg, void *val, uint32_t val_len)
{
  int ret = 0;
  
  if(NULL == prop){
    return -1;
  }
  
  // no hardware operation, but string length must ok.
  if(val_len > prop->maxStringLen){
    return -1;
  }
  
  properties_user_log("string_set: val=%s, val_len=%d.", (char*)prop->value, *(prop->value_len));
  
  return ret;  // return 0, succeed.
}

/*
 * MODULE: RGB LED
 */

// swtich set function
int rgb_led_sw_set(struct mico_prop_t *prop, void *arg, void *val, uint32_t val_len)
{
  int ret = 0;
  bool set_sw_state = *((bool*)val);
  
  user_context_t *uct = (user_context_t*)arg;
  //properties_user_log("rgb_led_sw_set: val=%d, val_len=%d.", *((bool*)val), val_len);
  //properties_user_log("h=%d, s=%d, b=%d", uct->config.rgb_led_hues, uct->config.rgb_led_saturation, uct->config.rgb_led_brightness);
  
  // control hardware
  if(set_sw_state){
    properties_user_log("Open LED.");
    hsb2rgb_led_open((float)uct->config.rgb_led_hues, (float)uct->config.rgb_led_saturation, (float)uct->config.rgb_led_brightness);
  }
  else{
    properties_user_log("Close LED.");
    hsb2rgb_led_close();
  }
  
  return ret;  // return 0, succeed.
}

// swtich get function
int rgb_led_sw_get(struct mico_prop_t *prop, void *arg, void *val, uint32_t *val_len)
{
  int ret = 0;
  
  // no hardware status to read, just return prop->value
  *((bool*)val) = *((bool*)prop->value);  
  *val_len = *(prop->value_len);
  
  properties_user_log("rgb_led_sw_get: val=%d, val_len=%d.", *((bool*)val), *((uint32_t*)val_len) );
  
  return ret;  // return 0, succeed.
}

// hues set function
int rgb_led_hues_set(struct mico_prop_t *prop, void *arg, void *val, uint32_t val_len)
{
  int ret = 0;
  int hues = *((int*)val);
  user_context_t *uct = (user_context_t*)arg;
  
  properties_user_log("rgb_led_hues_set: val=%d, val_len=%d.", *((int*)val), val_len);
  
  // control hardware
  if(uct->config.rgb_led_sw){
    hsb2rgb_led_open((float)hues, (float)uct->config.rgb_led_saturation, (float)uct->config.rgb_led_brightness);
  }
  else{
    hsb2rgb_led_close();
  }
  
  return ret;  // return 0, succeed.
}

// hues get function
int rgb_led_hues_get(struct mico_prop_t *prop, void *arg, void *val, uint32_t *val_len)
{
  int ret = 0;
  
  // get hardware status, no hardware status to read, just return prop->value
  *((int*)val) = *((int*)prop->value);  
  *val_len = *(prop->value_len);
  
  properties_user_log("rgb_led_hues_get: val=%d, val_len=%d.", *((int*)val), *(uint32_t*)val_len );
  
  return ret;  // return 0, succeed.
}

// saturation set function
int rgb_led_saturation_set(struct mico_prop_t *prop, void *arg, void *val, uint32_t val_len)
{
  int ret = 0;
  int saturation = *((int*)val);
  user_context_t *uct = (user_context_t*)arg;
  
  properties_user_log("rgb_led_saturation_set: val=%d, val_len=%d.", *((int*)val), val_len);
  
  // control hardware
  if(uct->config.rgb_led_sw){
    hsb2rgb_led_open((float)uct->config.rgb_led_hues, (float)saturation, (float)uct->config.rgb_led_brightness);
  }
  else{
    hsb2rgb_led_close();
  }
  
  return ret;  // return 0, succeed.
}

// saturation get function
int rgb_led_saturation_get(struct mico_prop_t *prop, void *arg, void *val, uint32_t *val_len)
{
  int ret = 0;
  
  // get hardware status, no hardware status to read, just return prop->value
  *((int*)val) = *((int*)prop->value);  
  *val_len = *(prop->value_len);
  
  properties_user_log("rgb_led_saturation_get: val=%d, val_len=%d.", *((int*)val), *(uint32_t*)val_len );
  
  return ret;  // return 0, succeed.
}

// brightness set function
int rgb_led_brightness_set(struct mico_prop_t *prop, void *arg, void *val, uint32_t val_len)
{
  int ret = 0;
  int brightness = *((int*)val);
  user_context_t *uct = (user_context_t*)arg;
  
  properties_user_log("rgb_led_brightness_set: val=%d, val_len=%d.", *((int*)val), val_len);
  
  // control hardware
  if(uct->config.rgb_led_sw){
    hsb2rgb_led_open((float)uct->config.rgb_led_hues, (float)uct->config.rgb_led_saturation, (float)brightness);
  }
  else{
    hsb2rgb_led_close();
  }
  
  return ret;  // return 0, succeed.
}

// brightness get function
int rgb_led_brightness_get(struct mico_prop_t *prop, void *arg, void *val, uint32_t *val_len)
{
  int ret = 0;
  
  // get hardware status, no hardware status to read, just return prop->value
  *((int*)val) = *((int*)prop->value);  
  *val_len = *(prop->value_len);
  
  properties_user_log("rgb_led_brightness_get: val=%d, val_len=%d.", *((int*)val), *(uint32_t*)val_len );
  
  return ret;  // return 0, succeed.
}


/*
 * MODULE: ADC 
 */

// get adc data function
int light_sensor_data_get(struct mico_prop_t *prop, void *arg, void *val, uint32_t *val_len)
{
  int ret = 0;
  uint16_t light_sensor_data = 0;
  OSStatus err = kUnknownErr;
  
  // get ADC data
  err = MicoAdcInitialize(MICO_ADC_1, 3);
  if(kNoErr != err){
    return -1;
  }
  err = MicoAdcTakeSample(MICO_ADC_1, &light_sensor_data);
  if(kNoErr == err){
    *((uint16_t*)val) = light_sensor_data;
    *val_len = sizeof(light_sensor_data);
    ret = 0;   // get data succeed
  }
  else{
    ret = -1;  // get data error
  }
  
  return ret;
}

// notify check function for adc
int notify_check_light_sensor_data(struct mico_prop_t *prop, void *arg, void *val, uint32_t *val_len)
{
  int ret = 0;
  uint16_t light_sensor_data = 0;
  uint32_t light_sensor_data_len = 0;
  
  // get adc data
  ret = prop->get(prop, arg, &light_sensor_data, &light_sensor_data_len);
  if(0 != ret){
    return -1;   // get value error
  }
  
  // update check (diff get_data and prop->value)
  //if(light_sensor_data != *((uint16_t*)(prop->value))){  // changed
  if( (((int)light_sensor_data - *((int*)(prop->value))) >= 50) || ((*((int*)(prop->value)) - (int)light_sensor_data) >= 50) ){  // abs >=10
    properties_user_log("light_sensor_data changed: %d -> %d", *((int*)prop->value), (int)light_sensor_data);   
    // return new value to update prop value && len
    *((int*)val) = (int)light_sensor_data;  
    *val_len = light_sensor_data_len;
    ret = 1;  // value changed, need to send notify message
  }
  else{
    ret = 0;  // not changed, not need to notify
  }
  
  return ret;
}

// get function of adc data notify event flag 
int event_status_get(struct mico_prop_t *prop, void *arg, void *val, uint32_t *val_len)
{
  // get event value
  *(bool*)val = *((bool*)prop->value);
  *val_len = *(prop->value_len);
  
  return 0;  // get ok
}

// set function of adc data notify event flag 
int event_status_set(struct mico_prop_t *prop, void *arg, void *val, uint32_t val_len)
{
  // set event value
  *((bool*)prop->value) = *((bool*)val);
  *(prop->value_len) = val_len;
  
  return 0;  // get ok
}

/*
 * MODULE: UART 
 */

// get function: recv uart data
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

// set function: send data to uart
int uart_data_send(struct mico_prop_t *prop, void *arg, void *val, uint32_t val_len)
{
  int ret = 0;
  OSStatus err = kUnknownErr;
  uint32_t send_len = 0;
  
  if(NULL == prop){
    return -1;
  }
  
  // send data
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

// notify_check function: check uart data recv
int uart_data_recv_check(struct mico_prop_t *prop, void *arg, void *val, uint32_t *val_len)
{
  int ret = 0;
  
  // get adc data
  ret = prop->get(prop, arg, val, val_len);
  if(0 != ret){
    return -1;   // get value error
  }
  
  // need notify (uart data recieved)
  ret = 1;
  
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
        .value = &(user_context.config.dev_name),
        .value_len = &(user_context.config.dev_name_len),
        .format = MICO_PROP_TYPE_STRING,
        .perms = (MICO_PROP_PERMS_RO | MICO_PROP_PERMS_WO),
        .get = string_get,                  // get string func to get device name
        .set = string_set,                  // set sring func to change device name
        .notify_check = NULL,               // not notifiable
        .arg = &(user_context.config.dev_name),            // get/set string pointer (device name)
        .event = NULL,                      // not notifiable
        .hasMeta = false,                   // no max/min/step
        .maxStringLen = MAX_DEVICE_NAME_SIZE,  // max length of device name string
        .unit = NULL                        // no unit
      },
      [1] = {
        .type = "public.map.property.manufacturer",  // device manufacturer uuid
        .value = &(user_context.config.dev_manufacturer),
        .value_len = &(user_context.config.dev_manufacturer_len),
        .format = MICO_PROP_TYPE_STRING,
        .perms = MICO_PROP_PERMS_RO,
        .get = string_get,                  // get string func to get manufacturer
        .set = NULL      ,                  // set sring func to change manufacturer
        .notify_check = NULL,               // not notifiable
        .arg = &(user_context.config.dev_manufacturer),
        .event = NULL,                      // not notifiable
        .hasMeta = false, 
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
        .value = &(user_context.config.rgb_led_sw),
        .value_len = &bool_len,                // bool type len
        .format = MICO_PROP_TYPE_BOOL,
        .perms = (MICO_PROP_PERMS_RO | MICO_PROP_PERMS_WO),
        .get = rgb_led_sw_get,              // get led switch status function
        .set = rgb_led_sw_set,              // set led switch status function
        .notify_check = NULL,               // not notifiable
        .arg = &user_context,               // user context
        .event = NULL,
        .hasMeta = false,    
        .maxStringLen = 0,
        .unit = NULL
      },
      [1] = {
        .type = "public.map.property.hues",  // led hues
        .value = &(user_context.config.rgb_led_hues),
        .value_len = &int_len,               // int type len
        .format = MICO_PROP_TYPE_INT,
        .perms = (MICO_PROP_PERMS_RO | MICO_PROP_PERMS_WO),
        .get = rgb_led_hues_get,
        .set = rgb_led_hues_set,
        .notify_check = NULL,               // not notifiable
        .arg = &user_context,               // user context
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
        .value = &(user_context.config.rgb_led_saturation),
        .value_len = &int_len,
        .format = MICO_PROP_TYPE_INT,
        .perms = (MICO_PROP_PERMS_RO | MICO_PROP_PERMS_WO),
        .get = rgb_led_saturation_get,
        .set = rgb_led_saturation_set,
        .notify_check = NULL,                     // not notifiable
        .arg = &user_context,                     // user context
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
        .value = &(user_context.config.rgb_led_brightness),
        .value_len = &int_len,
        .format = MICO_PROP_TYPE_INT,
        .perms = (MICO_PROP_PERMS_RO | MICO_PROP_PERMS_WO),
        .get = rgb_led_brightness_get,
        .set = rgb_led_brightness_set,
        .arg = &user_context,                      // user context
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
    .type = "public.map.service.light_sensor",         //  service 3: light sensor (ADC)
    .properties = {
      [0] = {
        .type = "public.map.property.value",  // adc value uuid
        .value = &(user_context.status.light_sensor_data),
        .value_len = &int_len,
        .format = MICO_PROP_TYPE_INT,
        .perms = (MICO_PROP_PERMS_RO | MICO_PROP_PERMS_EV),
        .get = light_sensor_data_get,
        .set = NULL,
        .notify_check = notify_check_light_sensor_data,  // check notify for adc data
        .arg = &user_context,                   // user context
        .event = &(user_context.config.light_sensor_event),  // event flag
        .hasMeta = true,
        .maxValue.intValue = 4095,
        .minValue.intValue = 0,
        .minStep.intValue = 1,
        .maxStringLen = 0,
        .unit = NULL
      },
      [1] = {
        .type = "public.map.property.event",    // adc value event (uuid)
        .value = &(user_context.config.light_sensor_event),
        .value_len = &bool_len,
        .format = MICO_PROP_TYPE_BOOL,
        .perms = (MICO_PROP_PERMS_RO | MICO_PROP_PERMS_WO),
        .get = event_status_get,
        .set = event_status_set,
        .notify_check = NULL,
        .arg = &user_context,                   // user context
        .event = NULL,
        .hasMeta = false,
        .maxStringLen = 0,
        .unit = NULL
      },
      [2] = {NULL}
    }
  },
  [3] = {
    .type = "public.map.service.uart",          //  service 3: ADC (uuid)
    .properties = {
      [0] = {
        .type = "public.map.property.message",  // uart message uuid
        .value = &(user_context.status.uart_rx_buf),
        .value_len = &(user_context.status.uart_rx_data_len),
        .format = MICO_PROP_TYPE_STRING,
        .perms = ( MICO_PROP_PERMS_WO | MICO_PROP_PERMS_EV ),
        .get = uart_data_recv,
        .set = uart_data_send,
        .notify_check = uart_data_recv_check,   // check recv data
        .arg = &user_context,
        .event = &(user_context.config.uart_rx_event),  // event flag
        .hasMeta = false,
        .maxStringLen = MAX_USER_UART_BUF_SIZE,
        .unit = "byte"
      },
      [1] = {NULL}
    },
  },
  [4] = {NULL}
};
