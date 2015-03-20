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
#include "JSON-C/json.h"
//#include "MICOAppDefine.h"

#define properties_log(M, ...) custom_log("DEV_PROPERTIES", M, ##__VA_ARGS__)
#define properties_log_trace() custom_log_trace("DEV_PROPERTIES")

/*******************************************************************************
* PROPERTIES DEFINE
*******************************************************************************/

// device info service
mico_dev_service_node_t dev_info_service = {
  "dev_info",
  1,              // iid
  NULL,
  2              // prop_table len
};

mico_dev_property_node_t service_dev_info_prop_table[] = {
  {"name", 2, "MicoKit3288", 11, PROP_TYPE_STRING},
  {"manufactory", 3, "MXCHIP", 6, PROP_TYPE_STRING}
};

// rgb led service
typedef struct _rgb_led_t {
int hsb_h;
int hsb_s;
int hsb_b;
bool hsb_sw;
}rgb_led_t;

rgb_led_t rgb_led = {360, 100, 100, false}; 

mico_dev_service_node_t rgb_led_service = {
  "rgb_led",
  4,            // iid
  NULL,
  5             // prop_table len
};

mico_dev_property_node_t service_rgb_led_prop_table[] = {
  {"name", 5, "RGB_LED", 7, PROP_TYPE_STRING},
  {"hsb-h", 6, &rgb_led.hsb_h, 4, PROP_TYPE_INT},
  {"hsb-s", 7, &rgb_led.hsb_s, 4, PROP_TYPE_INT},
  {"hsb-b", 8, &rgb_led.hsb_b, 4, PROP_TYPE_INT},
  {"sw", 9, &rgb_led.hsb_sw, 1, PROP_TYPE_BOOL}
};

// adc service
typedef struct _adc_t {
  int value;
}adc_t;

adc_t adc = {0};

mico_dev_service_node_t adc_service = {
  "adc",
  10,                // iid
  NULL,
  2                  // prop_table len
};

mico_dev_property_node_t service_adc_prop_table[] = {
  {"name", 11, "STM32-ADC", 9, PROP_TYPE_STRING},
  {"value", 12, &adc.value, 4, PROP_TYPE_INT}
};

// device service table
mico_dev_service_node_t service_table[1];

OSStatus create_service_table(void)
{
  OSStatus err = kNoErr;
  properties_log_trace();

  dev_info_service.property_table = service_dev_info_prop_table;
  //rgb_led_service.property_table = service_rgb_led_prop_table;
  //adc_service.property_table = service_adc_prop_table;
  
  service_table[0] = dev_info_service;
  //service_table[1] = rgb_led_service;
  //service_table[2] = adc_service;
 
  return err;
}

/*******************************************************************************
* PROPERTIES API
*******************************************************************************/
OSStatus add_top(json_object **outTopMenu, char* const service_name, json_object* services)
{
  OSStatus err;
  json_object *object;
  err = kNoErr;

  object = json_object_new_object();
  require_action(object, exit, err = kNoMemoryErr);

  json_object_object_add(object, service_name, services);
  *outTopMenu = object;
  
exit:
  return err;
}

OSStatus add_service(json_object* services, 
                     const char* type_name,  const char* type_content, 
                     const char* iid_name,  int iid_content,
                     const char* properties_name,  json_object *properties)
{
  OSStatus err;
  json_object *object;
  err = kNoErr;
  
  object = json_object_new_object();
  require_action(object, exit, err = kNoMemoryErr);
  json_object_object_add(object, type_name, json_object_new_string(type_content));
  json_object_object_add(object, iid_name, json_object_new_int(iid_content));
  json_object_object_add(object, properties_name, properties);
  json_object_array_add(services, object);
  
exit:
  return err;
}

OSStatus add_properties(json_object* properties,  mico_dev_property_node_t *properties_table)
{
  OSStatus err;
  json_object *object;
  err = kNoErr;
  
  object = json_object_new_object();
  require_action(object, exit, err = kNoMemoryErr);
  
  json_object_object_add(object, "type", json_object_new_string(properties_table->type));
  json_object_object_add(object, "iid", json_object_new_int(properties_table->iid));
  
  switch(properties_table->format){
  case PROP_TYPE_INT:{
    //properties_log("value=%d", *((int*)(properties_table->value)));
    json_object_object_add(object, "value", json_object_new_int( *((int*)(properties_table->value)) ) );
    json_object_object_add(object, "format", json_object_new_string("int"));
    break;
  }
  case PROP_TYPE_STRING:{
    json_object_object_add(object, "value", json_object_new_string( (char*)(properties_table->value)) );
    json_object_object_add(object, "format", json_object_new_string("string"));
    break;
  }
  case PROP_TYPE_FLOAT:{
    json_object_object_add(object, "value", json_object_new_double( *((double*)(properties_table->value))) );
    json_object_object_add(object, "format", json_object_new_string("float"));
    break;
  }
  case PROP_TYPE_BOOL:{
    json_object_object_add(object, "value", json_object_new_boolean( *((bool*)(properties_table->value))) );
    json_object_object_add(object, "format", json_object_new_string("bool"));
    break;
  }
  default:{
    err= kParamErr;
    break;
  }
  }
  
  json_object_object_add(object, "value_len", json_object_new_int(properties_table->value_len));

  json_object_array_add(properties, object);
  
exit:
  if(err != kNoErr && object){
    json_object_put(object);
    object = NULL;
  }
  return err;
}


json_object* create_dev_info_json_object(mico_dev_service_node_t service_table[], int size)
{
  OSStatus err = kNoErr;
  properties_log_trace();
  json_object *properties = NULL, *services = NULL, *mainObject = NULL;
  int i = 0, j = 0;
  int service_num = 0;
  int property_num = 0;
  
  services = json_object_new_array();
  require( services, exit );
  err = add_top(&mainObject, "services", services);
  
  service_num = size/sizeof(mico_dev_service_node_t);
  //properties_log("service_table=%d, service_node=%d", size, sizeof(mico_dev_service_node_t));
  properties_log("service_num=%d", service_num);
  for(i = 0; i < service_num; i++){
    properties = json_object_new_array();
    require( properties, exit );
    err = add_service(services, "type", service_table[i].type, 
                      "iid", service_table[i].iid, "properties", properties);
    require_noerr( err, exit );
    
    //property_num = (sizeof(service_table[i].property_table)/sizeof(mico_dev_property_node_t));
    //properties_log("sizeof(property_table)=%d", sizeof(service_table[i].property_table));
    property_num = service_table[i].property_num;
    properties_log("property_num=%d", property_num);
    for(j = 0; j < property_num; j++){
      err = add_properties(properties, &service_table[i].property_table[j]);
      require_noerr( err, exit );
    }
  }
  
exit:
  if(err != kNoErr && mainObject){
    json_object_put(mainObject);
    mainObject = NULL;
  }
  return mainObject;
}