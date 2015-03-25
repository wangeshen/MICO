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
//#include "MICOAppDefine.h"
#include "properties.h"
#include "properties_user.h"
#include "JSON-C/json.h"

#define properties_user_log(M, ...) custom_log("DEV_PROPERTIES_USER", M, ##__VA_ARGS__)
#define properties_user_log_trace() custom_log_trace("DEV_PROPERTIES_USER")

/*******************************************************************************
* PROPERTIES
*******************************************************************************/

// fixed type data len
uint32_t bool_len = sizeof(bool);
uint32_t int_len = sizeof(int);
uint32_t float_len = sizeof(float);

/************dev_info************************/

struct dev_info_t dev_info = {
  .name = "MicoKit3288",
  .name_len = 11,
  .manufactory = "MXCHIP",
  .manufactory_len = 6
};

/************rgb_led************************/
struct rgb_led_t rgb_led = {
  .sw = false,
  .sw_ev = true,
  .hues = 0,
  .saturation = 0,
  .brightness = 0
};

int rgb_led_sw_set(struct mico_prop_t *prop, void *arg, void *val, uint32_t val_len)
{
  int ret = 0;
  properties_user_log("rgb_led set: val=%d, val_len=%d.", *((bool*)val), val_len);
  *((bool*)prop->value) = *((bool*)val);
  
  return ret;
}

int rgb_led_sw_get(struct mico_prop_t *prop, void *arg, void *val, uint32_t *val_len)
{
  int ret = 0;
  properties_user_log("rgb_led get: val=%d, val_len=%d.", *((bool*)prop->value), *(uint32_t*)prop->value_len );
  return ret;
}

int notify_rgb_led_switch(struct mico_prop_t *prop, void *arg, void *val, uint32_t *val_len)
{
  int ret = 0;
  properties_user_log("rgb_led switch notify: %d", *((bool*)prop->value));
  //*(prop->event) = false;
  //properties_user_log("notify disabled.");
  //*((int*)(prop->value)) = 360;
  return ret;
}

/************adc************************/
struct adc_t adc = {
  .data = 0,
  .event = true
};

int notify_adc_data(struct mico_prop_t *prop, void *arg, void *val, uint32_t *val_len)
{
  int ret = 0;
  properties_user_log("adc_data notify: %d", *((int*)prop->value));
  //*(prop->event) = false;
  //properties_user_log("notify disabled.");
  *((int*)(prop->value)) += 1;
  return ret;
}

/************ service_table ***********/
const struct mico_service_t  service_table[] = {
  [0] = {  // dev_info
    .type = "public.map.service.dev_info",    // dev_info
    .properties = {
      [0] = {
        .type = "public.map.property.name",  // device name
        .value = &(dev_info.name),
        .value_len = &(dev_info.name_len),
        .format = MICO_PROP_TYPE_STRING,
        .perms = (uint8_t)MICO_PROP_PERMS_RO,
        .get = NULL,                        // get function defined by user
        .set = NULL,                        // RO£¬write not available
        .arg = &(dev_info.name),        // user defined string to stroe get value
      },
      [1] = {
        .type = "public.map.property.manufactory",  // device manufactory
        .value = &(dev_info.manufactory),
        .value_len = &(dev_info.manufactory_len),
        .format = MICO_PROP_TYPE_STRING,
        .perms = (uint8_t)MICO_PROP_PERMS_RO,
        .get = NULL,    // get function defined by user
        .set = NULL,            // RO£¬write not available
        .arg = &(dev_info.manufactory),        // user defined string to stroe get value
      },
      [2] = {NULL}
    }
  },
  [1] = {
    .type = "public.map.service.led",   // rgb led
    .properties = {
      [0] = {
        .type = "public.map.property.switch",  // led switch
        .value = &(rgb_led.sw),
        .value_len = &bool_len,
        .format = MICO_PROP_TYPE_BOOL,
        .perms = (uint8_t)(MICO_PROP_PERMS_RW),
        .get = rgb_led_sw_get,    // get function defined by user
        .set = rgb_led_sw_set,    // set functions defined by user
        .arg = &(rgb_led.sw),      // user defined string to stroe get value
        .notify = notify_rgb_led_switch,
        .event = &(rgb_led.sw_ev)
      },
      [1] = {
        .type = "public.map.property.hues",  // led hues
        .value = &(rgb_led.hues),
        .value_len = &int_len,    // int, 4 bytes
        .format = MICO_PROP_TYPE_INT,
        .perms = (uint8_t)MICO_PROP_PERMS_RW,
        .get = NULL,    // get function defined by user
        .set = NULL,    // set functions defined by user
        .arg = &(rgb_led.hues),      // user defined string to stroe get value
      },
      [2] = {
        .type = "public.map.property.saturation",  // led saturation
        .value = &(rgb_led.saturation),
        .value_len = &int_len,    // int, 4 bytes
        .format = MICO_PROP_TYPE_INT,
        .perms = (uint8_t)MICO_PROP_PERMS_RW,
        .get = NULL,    // get function defined by user
        .set = NULL,    // set functions defined by user
        .arg = &(rgb_led.saturation),      // user defined string to stroe get value
      },
      [3] = {
        .type = "public.map.property.brightness",  // led brightness
        .value = &(rgb_led.brightness),
        .value_len = &int_len,    // int, 4 bytes
        .format = MICO_PROP_TYPE_INT,
        .perms = (uint8_t)MICO_PROP_PERMS_RW,
        .get = NULL,    // get function defined by user
        .set = NULL,    // set functions defined by user
        .arg = &(rgb_led.brightness),      // user defined string to stroe get value
      },
      [4] = {NULL}
    }
  },
  [2] = {
    .type = "public.map.service.adc",   //  ADC
    .properties = {
      [0] = {
        .type = "public.map.property.value",  // adc value
        .value = &(adc.data),
        .value_len = &int_len,
        .format = MICO_PROP_TYPE_INT,
        .perms = (uint8_t)(MICO_PROP_PERMS_RW | MICO_PROP_PERMS_EV),
        .get = NULL,    // get function defined by user
        .set = NULL,          // RO, set functions not available
        .arg = &(adc.data),      // user defined string to stroe get value
        .event = &(adc.event),    // event flag
        .notify = notify_adc_data  // create notify data adc value change
      },
      [1] = {NULL}
    }
  },
  [3] = {NULL}
};
