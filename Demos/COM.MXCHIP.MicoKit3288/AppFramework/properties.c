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
* PROPERTIES API
*******************************************************************************/

int notify_default(struct mico_prop_t *prop, void *arg, void *val, uint32_t *val_len)
{
  int ret = 0;
  properties_log("default notify, do nothing.");
  return ret;
}

// property notify
OSStatus  mico_property_notify(mico_Context_t * const inContext, struct mico_service_t *service_table)
{
  OSStatus err = kUnknownErr;
  int i = 0; 
  int j = 0;
  
  properties_log("properties notify task...");
  for(i = 0; NULL != service_table[i].type; i++)
  {
    for(j = 0; NULL != service_table[i].properties[j].type; j++){
      properties_log("service[%d]: %s, property[%d]: %s", 
                     i, service_table[i].type,
                     j, service_table[i].properties[j].type);
      
      // call property notify func
      if((NULL != service_table[i].properties[j].event) && (*(service_table[i].properties[j].event))){
        if(NULL != service_table[i].properties[j].notify){
          service_table[i].properties[j].notify(&(service_table[i].properties[j]), NULL, NULL, NULL);
        }
        else{
          notify_default(NULL,NULL,NULL,NULL);
        }
      }
    }
    properties_log("property num=%d", j);
  }
  properties_log("service num=%d", i);
  
  err = kNoErr;
  return err;
}

OSStatus mico_property_read(mico_Context_t * const inContext, struct mico_service_t *service_table, int iid)
{
  OSStatus err = kUnknownErr;
  int i = 0; 
  int j = 0;
  int iid_tmp = 1;
  
  properties_log("properties read iid=%d.", iid);
  for(i = 0; NULL != service_table[i].type; i++){
    // service iid check
    if(iid == iid_tmp){
      properties_log("service got: %s, iid=%d", service_table[i].type, iid_tmp);
      return kNoErr;
    }
    iid_tmp++;     // service iid +1
    
    // prop iid check
    for(j = 0; NULL != service_table[i].properties[j].type; j++){
      if(iid == iid_tmp){
        //        properties_log("service[%d]: %s, property[%d]: %s", 
        //                       i, service_table[i].type,
        //                       j, service_table[i].properties[j].type);
        switch(service_table[i].properties[j].format){
        case MICO_PROP_TYPE_INT:{
          properties_log("prop got: %s, iid=%d, value=%d", 
                         service_table[i].properties[j].type, iid_tmp, 
                         *((int*)service_table[i].properties[j].value));
          break;
        }
        case MICO_PROP_TYPE_FLOAT:{
          properties_log("prop got: %s, iid=%d, value=%f", 
                         service_table[i].properties[j].type, iid_tmp, 
                         *((float*)service_table[i].properties[j].value));
          break;
        }
        case MICO_PROP_TYPE_STRING:{
          properties_log("prop got: %s, iid=%d, value=%s", 
                         service_table[i].properties[j].type, iid_tmp, 
                         (char*)service_table[i].properties[j].value);
          break;
        }
        case MICO_PROP_TYPE_BOOL:{
          properties_log("prop got: %s, iid=%d, value=%d", 
                         service_table[i].properties[j].type, iid_tmp, 
                         *((bool*)service_table[i].properties[j].value));
          break;
        }
        default:
          break;
        }
        return kNoErr;
      }
      iid_tmp++;   // prop iid +1
    }
  }
  
  err = kNoErr;
  return err;
}

OSStatus  mico_property_write(mico_Context_t * const inContext, struct mico_service_t *service_table, 
                              int iid, void *val, uint32_t val_len)
{
  OSStatus err = kUnknownErr;
  int i = 0; 
  int j = 0;
  int iid_tmp = 1;
  
  properties_log("properties write iid=%d.", iid);
  for(i = 0; NULL != service_table[i].type; i++){
    // service iid check
    if(iid == iid_tmp){
      properties_log("service write: %s, iid=%d", service_table[i].type, iid_tmp);
      return kNoErr;
    }
    iid_tmp++;     // service iid +1
    
    // prop iid check
    for(j = 0; NULL != service_table[i].properties[j].type; j++){
      if(iid == iid_tmp){
        //        properties_log("service[%d]: %s, property[%d]: %s", 
        //                       i, service_table[i].type,
        //                       j, service_table[i].properties[j].type);
        switch(service_table[i].properties[j].format){
        case MICO_PROP_TYPE_INT:{
          properties_log("prop write: %s, iid=%d, value=%d", 
                         service_table[i].properties[j].type, iid_tmp, 
                         *((int*)service_table[i].properties[j].value));
          *((int*)service_table[i].properties[j].value) = *((int*)val);
          break;
        }
        case MICO_PROP_TYPE_FLOAT:{
          properties_log("prop got: %s, iid=%d, value=%f", 
                         service_table[i].properties[j].type, iid_tmp, 
                         *((float*)service_table[i].properties[j].value));
          *((float*)service_table[i].properties[j].value) = *((float*)val);
          break;
        }
        case MICO_PROP_TYPE_STRING:{
          properties_log("prop got: %s, iid=%d, value=%s", 
                         service_table[i].properties[j].type, iid_tmp, 
                         (char*)service_table[i].properties[j].value);
          strncpy((char*)service_table[i].properties[j].value, (char*)val, val_len);
          break;
        }
        case MICO_PROP_TYPE_BOOL:{
          properties_log("prop got: %s, iid=%d, value=%d", 
                         service_table[i].properties[j].type, iid_tmp, 
                         *((bool*)service_table[i].properties[j].value));
          *((bool*)service_table[i].properties[j].value) = *((bool*)val);
          break;
        }
        default:
          break;
        }
        return kNoErr;
      }
      iid_tmp++;   // prop iid +1
    }
  }
  
  err = kNoErr;
  return err;
}

/*
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
*/
