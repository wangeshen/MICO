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

// fixed data type length
uint32_t bool_len = sizeof(bool);
uint32_t int_len = sizeof(int);
uint32_t float_len = sizeof(float);

/************dev_info************************/

// dev_info data
struct dev_info_t dev_info = {
  .name = "MicoKit3288",
  .name_len = 11,
  .manufactory = "MXCHIP",
  .manufactory_len = 6
};

// no set/get function
int string_get(struct mico_prop_t *prop, void *arg, void *val, uint32_t *val_len)
{
  int ret = 0;

  if((NULL == prop) || (NULL == val)){
    return -1;
  }
  
  uint32_t copy_len = ((*val_len > *(prop->value_len)) ? *(prop->value_len) : *val_len);
  strncpy(val, (char*)prop->value, copy_len);
  *val_len = copy_len;
  
  properties_user_log("string_get: val=%s, val_len=%d.", (char*)val, *val_len);
  
  return ret;
}

int string_set(struct mico_prop_t *prop, void *arg, void *val, uint32_t val_len)
{
  int ret = 0;

  if((NULL == prop) || (NULL == val)){
    return -1;
  }
  
  uint32_t copy_len = ((val_len > *(prop->value_len)) ? *(prop->value_len) : val_len);
  memset(prop->value, '\0', *(prop->value_len));
  strncpy((char*)prop->value, val, copy_len);
  *(prop->value_len) = copy_len;
  
  properties_user_log("string_set: val=%s, val_len=%d.", (char*)prop->value, prop->value_len);
  
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
  properties_user_log("rgb_led_sw_set: val=%d, val_len=%d.", *((bool*)val), val_len);
  // control hardware
  *((bool*)(prop->value)) = *((bool*)val);  // for test
  *(prop->value_len) = val_len;
  
  return ret;
}

int rgb_led_sw_get(struct mico_prop_t *prop, void *arg, void *val, uint32_t *val_len)
{
  int ret = 0;
  // read hardware status
  *((bool*)val) = *(bool*)(prop->value);  // for test
  *val_len = *(prop->value_len);
  
  properties_user_log("rgb_led_sw_get: val=%d, val_len=%d.", *((bool*)val), *((uint32_t*)val_len) );

  return ret;
}

// hues function
int rgb_led_hues_set(struct mico_prop_t *prop, void *arg, void *val, uint32_t val_len)
{
  int ret = 0;
  properties_user_log("rgb_led_hues_set: val=%d, val_len=%d.", *((int*)val), val_len);
  // control hardware
  *((int*)(prop->value)) = *((int*)val);  // for test
  *(prop->value_len) = val_len;
  
  return ret;
}

int rgb_led_hues_get(struct mico_prop_t *prop, void *arg, void *val, uint32_t *val_len)
{
  int ret = 0;
  // get hardware status
  *((int*)val) = *(int*)(prop->value);  // for test
  *val_len = *(prop->value_len);
  
  properties_user_log("rgb_led_hues_get: val=%d, val_len=%d.", *((int*)val), *(uint32_t*)val_len );
  
  return ret;
}

// saturation function
int rgb_led_saturation_set(struct mico_prop_t *prop, void *arg, void *val, uint32_t val_len)
{
  int ret = 0;
  properties_user_log("rgb_led_saturation_set: val=%d, val_len=%d.", *((int*)val), val_len);
  // control hardware
  *((int*)(prop->value)) = *((int*)val);  // for test
  *(prop->value_len) = val_len;
  
  return ret;
}

int rgb_led_saturation_get(struct mico_prop_t *prop, void *arg, void *val, uint32_t *val_len)
{
  int ret = 0;
  // get hardware status
  *(int*)val = *(int*)(prop->value);  // for test
  *val_len = *(prop->value_len);
  properties_user_log("rgb_led_saturation_get: val=%d, val_len=%d.", *((int*)val), *(uint32_t*)val_len );
  
  return ret;
}

// brightness function
int rgb_led_brightness_set(struct mico_prop_t *prop, void *arg, void *val, uint32_t val_len)
{
  int ret = 0;
  properties_user_log("rgb_led_brightness_set: val=%d, val_len=%d.", *((int*)val), val_len);
  // control hardware
  *((int*)(prop->value)) = *((int*)val);  // for test
  *(prop->value_len) = val_len;
  
  return ret;
}

int rgb_led_brightness_get(struct mico_prop_t *prop, void *arg, void *val, uint32_t *val_len)
{
  int ret = 0;
  // get hardware status
  *(int*)val = *(int*)(prop->value);  // for test
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

/************ service_table ***********/
const struct mico_service_t  service_table[] = {
  [0] = {  // dev_info
    .type = "public.map.service.dev_info",    // dev_info  uuid
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
        .maxStringLen = 16,                 // max length of device name string
        .unit = NULL                        // no unit
      },
      [1] = {
        .type = "public.map.property.manufactory",  // device manufactory uuid
        .value = &(dev_info.manufactory),
        .value_len = &(dev_info.manufactory_len),
        .format = MICO_PROP_TYPE_STRING,
        .perms = MICO_PROP_PERMS_RO,
        .get = string_get,                  // get string func to get manufactory
        .set = string_set,                  // set sring func to change manufactory
        .notify_check = NULL,               // not notifiable
        .arg = &(dev_info.manufactory),
        .event = NULL,                      // not notifiable
        .hasMeta = false,
        .maxStringLen = 16,                 // max length of device manufactory
        .unit = NULL                        // no unit
      },
      [2] = {NULL}                          // end flag
    }
  },
  [1] = {
    .type = "public.map.service.led",       // rgb led uuid
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
        .arg = &(rgb_led.sw),               // led switch status
        .event = NULL,
        .hasMeta = false,
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
        .arg = &(rgb_led.hues),  // led hues value
        .event = NULL,
        .hasMeta = true,
        .maxValue.intValue = 360,
        .minValue.intValue = 0,
        .minStep.intValue = 1,
        .maxStringLen = 0,
        .unit = "h"
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
        .arg = &(rgb_led.saturation),  // led saturation value
        .event = NULL,
        .hasMeta = true,
        .maxValue.intValue = 100,
        .minValue.intValue = 0,
        .minStep.intValue = 1,
        .maxStringLen = 0,
        .unit = "s"
      },
      [3] = {
        .type = "public.map.property.brightness",  // led brightness
        .value = &(rgb_led.brightness),
        .value_len = &int_len,
        .format = MICO_PROP_TYPE_INT,
        .perms = (MICO_PROP_PERMS_RO | MICO_PROP_PERMS_WO),
        .get = rgb_led_brightness_get,
        .set = rgb_led_brightness_set,
        .arg = &(rgb_led.brightness),  // led brightness value
        .event = NULL,
        .hasMeta = true,
        .maxValue.intValue = 100,
        .minValue.intValue = 0,
        .minStep.intValue = 1,
        .maxStringLen = 0,
        .unit = "b"
      },
      [4] = {NULL}
    }
  },
  [2] = {
    .type = "public.map.service.adc",   //  ADC uuid
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
        .arg = &(adc.data),         // adc sample data
        .event = &(adc.event),      // event flag
        .hasMeta = true,
        .maxValue.intValue = 4095,
        .minValue.intValue = 0,
        .minStep.intValue = 1,
        .maxStringLen = 0,
        .unit = NULL
      },
      [1] = {NULL}
    }
  },
  [3] = {NULL}
};
