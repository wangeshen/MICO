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

// hues function
int rgb_led_hues_set(struct mico_prop_t *prop, void *arg, void *val, uint32_t val_len)
{
  int ret = 0;
  properties_user_log("rgb_led_hues_set: val=%d, val_len=%d.", *((int*)val), val_len);
  *((int*)prop->value) = *((int*)val);
  
  return ret;
}

int rgb_led_hues_get(struct mico_prop_t *prop, void *arg, void *val, uint32_t *val_len)
{
  int ret = 0;
  properties_user_log("rgb_led_hues_get: val=%d, val_len=%d.", *((int*)prop->value), *(uint32_t*)prop->value_len );
  return ret;
}

// saturation function
int rgb_led_saturation_set(struct mico_prop_t *prop, void *arg, void *val, uint32_t val_len)
{
  int ret = 0;
  properties_user_log("rgb_led_saturation_set: val=%d, val_len=%d.", *((int*)val), val_len);
  *((int*)prop->value) = *((int*)val);
  
  return ret;
}

int rgb_led_saturation_get(struct mico_prop_t *prop, void *arg, void *val, uint32_t *val_len)
{
  int ret = 0;
  properties_user_log("rgb_led_saturation_get: val=%d, val_len=%d.", *((int*)prop->value), *(uint32_t*)prop->value_len );
  return ret;
}

// brightness function
int rgb_led_brightness_set(struct mico_prop_t *prop, void *arg, void *val, uint32_t val_len)
{
  int ret = 0;
  properties_user_log("rgb_led_brightness_set: val=%d, val_len=%d.", *((int*)val), val_len);
  *((int*)prop->value) = *((int*)val);
  
  return ret;
}

int rgb_led_brightness_get(struct mico_prop_t *prop, void *arg, void *val, uint32_t *val_len)
{
  int ret = 0;
  properties_user_log("rgb_led_brightness_get: val=%d, val_len=%d.", *((int*)prop->value), *(uint32_t*)prop->value_len );
  return ret;
}

/************adc************************/
// adc data
struct adc_t adc = {
  .data = 0,
  .event = true
};

// get adc data function
int adc_get(struct mico_prop_t *prop, void *arg, void *val, uint32_t *val_len)
{
  int ret = 0;
  uint16_t adc_data = 0;
  OSStatus err = kUnknownErr;

  // get ADC data  ==> prop->value
  err = MicoAdcTakeSample(MICO_ADC_1, &adc_data);
  if(kNoErr == err){
    *((int*)val) = (int)adc_data;
    *val_len = sizeof(int);
    
    // update prop->value
    //*((int*)prop->value) = (int)adc_data;
    //properties_user_log("adc get: val=%d, val_len=%d.", *((int*)prop->value), *((uint32_t*)(prop->value_len)) );
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
  int adc_data = 0;
  uint32_t adc_data_len = 0;
  
  // get adc data
  ret = prop->get(prop, NULL, &adc_data, &adc_data_len);
  if(0 != ret){
    return -1;   // get value error
  }
  
  // check notify
  //if(adc_data != *((int*)(prop->value))){  // changed
  if( ((adc_data - *((int*)(prop->value))) >= 20) || ((*((int*)(prop->value)) - adc_data) >= 20) ){  // abs >=10
    properties_user_log("adc_data changed: %d -> %d", *((int*)prop->value), adc_data);
    // update prop->value
    *((int*)prop->value) = adc_data;
    *(prop->value_len) = adc_data_len;
    ret = 1;  // value changed, need to send notify message
  }
  else{
    ret = 0;  // not changed, not need to notify
  }
  
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
        .perms = (uint8_t)(MICO_PROP_PERMS_RO),
        .get = NULL,                        // get function defined by user
        .set = NULL,                        // RO£¬write not available
        .arg = &(dev_info.name),        // user defined string to stroe get value
      },
      [1] = {
        .type = "public.map.property.manufactory",  // device manufactory
        .value = &(dev_info.manufactory),
        .value_len = &(dev_info.manufactory_len),
        .format = MICO_PROP_TYPE_STRING,
        .perms = (uint8_t)(MICO_PROP_PERMS_RO),
        .get = NULL,
        .set = NULL,
        .arg = &(dev_info.manufactory),
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
        .value_len = &bool_len,   // bool type len
        .format = MICO_PROP_TYPE_BOOL,
        .perms = (uint8_t)(MICO_PROP_PERMS_RW),
        .get = rgb_led_sw_get,    // get led switch status function
        .set = rgb_led_sw_set,    // set led switch status function
        .arg = &(rgb_led.sw),     // led switch status
      },
      [1] = {
        .type = "public.map.property.hues",  // led hues
        .value = &(rgb_led.hues),
        .value_len = &int_len,   // int type len
        .format = MICO_PROP_TYPE_INT,
        .perms = (uint8_t)(MICO_PROP_PERMS_RW),
        .get = rgb_led_hues_get,
        .set = rgb_led_hues_set,
        .arg = &(rgb_led.hues),  // led hues value
      },
      [2] = {
        .type = "public.map.property.saturation",  // led saturation
        .value = &(rgb_led.saturation),
        .value_len = &int_len,
        .format = MICO_PROP_TYPE_INT,
        .perms = (uint8_t)(MICO_PROP_PERMS_RW),
        .get = rgb_led_saturation_get,
        .set = rgb_led_saturation_set,
        .arg = &(rgb_led.saturation),  // led saturation value
      },
      [3] = {
        .type = "public.map.property.brightness",  // led brightness
        .value = &(rgb_led.brightness),
        .value_len = &int_len,
        .format = MICO_PROP_TYPE_INT,
        .perms = (uint8_t)(MICO_PROP_PERMS_RW),
        .get = rgb_led_brightness_get,
        .set = rgb_led_brightness_set,
        .arg = &(rgb_led.brightness),  // led brightness value
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
        .perms = (uint8_t)(MICO_PROP_PERMS_RO | MICO_PROP_PERMS_EV),
        .get = adc_get,
        .set = NULL,
        .arg = &(adc.data),         // adc sample data
        .event = &(adc.event),      // event flag
        .notify_check = notify_check_adc_data,  // check notify for adc data
      },
      [1] = {NULL}
    }
  },
  [3] = {NULL}
};
