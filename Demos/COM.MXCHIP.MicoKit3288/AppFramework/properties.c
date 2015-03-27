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
#include "StringUtils.h"
//#include "MICOAppDefine.h"
#include "MicoFogCloud.h"

#define properties_log(M, ...) custom_log("DEV_PROPERTIES", M, ##__VA_ARGS__)
#define properties_log_trace() custom_log_trace("DEV_PROPERTIES")


/*******************************************************************************
* PROPERTIES API
*******************************************************************************/

int notify_check_default(struct mico_prop_t *prop, void *arg, void *val, uint32_t *val_len)
{
  int ret = 0;
  properties_log("prop: %s, do default notify_check.", prop->type);
  return ret;
}

/* property update check
* check update status of all properties in service_table
* input: mico context;
*        service table
* return: json object contains properties updated, like {k:v, k:v}
*         if no update or error, return NULL
*/
json_object* mico_properties_update_check(mico_Context_t * const inContext, struct mico_service_t *service_table)
{
  int i = 0; 
  int j = 0;
  int ret = 0;
  int iid = 1;
  char iid_str[16] = {0};
  json_object *notify_obj = NULL;
  int updated_prop_cnt = 0;
  
  require(inContext, exit);
  require(service_table, exit);
  
  notify_obj = json_object_new_object();
  require(notify_obj, exit);
  
  properties_log("properties update check...");
  for(i = 0; NULL != service_table[i].type; i++){
    iid++;  // next service or property
    for(j = 0; NULL != service_table[i].properties[j].type; j++){
      memset((void*)iid_str, '\0', sizeof(iid_str));
      Int2Str((uint8_t*)iid_str, iid);
      
      if( MICO_PROP_PERMS_EV & (service_table[i].properties[j].perms)){  // prop has event
        if((NULL != service_table[i].properties[j].event) && (*(service_table[i].properties[j].event))){  // prop event on
          if(NULL != service_table[i].properties[j].notify_check){
            // do prop update check
            ret = service_table[i].properties[j].notify_check(&(service_table[i].properties[j]), 
                                                              service_table[i].properties[j].arg, NULL, NULL);
            if(1 == ret){
              // prop updated, add to notify list
              switch(service_table[i].properties[j].format){
              case MICO_PROP_TYPE_INT:{
                json_object_object_add(notify_obj, iid_str, json_object_new_int(*((int*)service_table[i].properties[j].value)));
                updated_prop_cnt++;
                break;
              }
              case MICO_PROP_TYPE_FLOAT:{
                json_object_object_add(notify_obj, iid_str, json_object_new_double(*((float*)service_table[i].properties[j].value)));
                updated_prop_cnt++;
                break;
              }
              case MICO_PROP_TYPE_STRING:{
                json_object_object_add(notify_obj, iid_str, json_object_new_string((char*)service_table[i].properties[j].value));
                updated_prop_cnt++;
                break;
              }
              case MICO_PROP_TYPE_BOOL:{
                json_object_object_add(notify_obj, iid_str, json_object_new_boolean(*((bool*)service_table[i].properties[j].value)));
                updated_prop_cnt++;
                break;
              }
              default:
                properties_log("ERROR: prop format unsupport!");
                break;
              }
            }
          }
          else{
            // use defalut update check(prop changed), if user not set func prop->notify_check
            ret = notify_check_default(&(service_table[i].properties[j]),NULL,NULL,NULL);
            if(1 == ret){
              // prop value changed
            }
          }
        }
      }
      iid++;  // next property
    }
  }
  
exit:
  if( (NULL != notify_obj) && ( 0 == updated_prop_cnt)){
    json_object_put(notify_obj);
    notify_obj = NULL;
  }
  return notify_obj;
}

// add json object if read success, if faild no return json object add, return err.
OSStatus mico_property_read_create(struct mico_service_t *service_table,  
                                   const char *key, int iid, json_object *outJsonObj)
{
  OSStatus err = kNotFoundErr;
  int i = 0; 
  int j = 0;
  int iid_tmp = 1;
  char iid_str[16] = {0};
  
  require_action( service_table, exit, err = kParamErr);
  require_action( outJsonObj, exit, err = kParamErr);
  
  for(i = 0; NULL != service_table[i].type; i++){
    if(iid == iid_tmp){  // if read a service, get all properties of the service
      properties_log("service got: %s, iid=%d", service_table[i].type, iid_tmp);
      iid_tmp++;   // jump to prop iid
      for(j = 0; NULL != service_table[i].properties[j].type; j++){
        if( MICO_PROP_PERMS_RO & (service_table[i].properties[j].perms)){
          // add iid as response key
          memset(iid_str, '\0', sizeof(iid_str));
          Int2Str((uint8_t*)iid_str, iid_tmp);
          // add response value
          err = kNoErr;
          switch(service_table[i].properties[j].format){
          case MICO_PROP_TYPE_INT:{
            properties_log("prop got: %s, iid=%d, value=%d", 
                           service_table[i].properties[j].type, iid_tmp, 
                           *((int*)service_table[i].properties[j].value));
            // prop->get (read hardware status)
            if(NULL != service_table[i].properties[j].get){
              service_table[i].properties[j].get(&service_table[i].properties[j], service_table[i].properties[j].arg,
                                                 service_table[i].properties[j].value, service_table[i].properties[j].value_len);
            }
            // return value
            json_object_object_add(outJsonObj, iid_str, json_object_new_int(*((int*)service_table[i].properties[j].value)));
            break;
          }
          case MICO_PROP_TYPE_FLOAT:{
            properties_log("prop got: %s, iid=%d, value=%f", 
                           service_table[i].properties[j].type, iid_tmp, 
                           *((float*)service_table[i].properties[j].value));
            // prop->get (read hardware status)
            if(NULL != service_table[i].properties[j].get){
              service_table[i].properties[j].get(&service_table[i].properties[j], service_table[i].properties[j].arg,
                                                 service_table[i].properties[j].value, service_table[i].properties[j].value_len);
            }
            // return value
            json_object_object_add(outJsonObj, iid_str, json_object_new_double(*((float*)service_table[i].properties[j].value)));
            break;
          }
          case MICO_PROP_TYPE_STRING:{
            properties_log("prop got: %s, iid=%d, value=%s", 
                           service_table[i].properties[j].type, iid_tmp, 
                           (char*)service_table[i].properties[j].value);
            // prop->get (read hardware status)
            if(NULL != service_table[i].properties[j].get){
              service_table[i].properties[j].get(&service_table[i].properties[j], service_table[i].properties[j].arg,
                                                 service_table[i].properties[j].value, service_table[i].properties[j].value_len);
            }
            // return value
            json_object_object_add(outJsonObj, iid_str, json_object_new_string((char*)service_table[i].properties[j].value));
            break;
          }
          case MICO_PROP_TYPE_BOOL:{
            properties_log("prop got: %s, iid=%d, value=%d", 
                           service_table[i].properties[j].type, iid_tmp, 
                           *((bool*)service_table[i].properties[j].value));
            // prop->get (read hardware status)
            if(NULL != service_table[i].properties[j].get){
              service_table[i].properties[j].get(&service_table[i].properties[j], service_table[i].properties[j].arg,
                                                 service_table[i].properties[j].value, service_table[i].properties[j].value_len);
            }
            // return value
            json_object_object_add(outJsonObj, iid_str, json_object_new_boolean(*((bool*)service_table[i].properties[j].value)));
            break;
          }
          default:
            properties_log("ERROR: property format unsupported!");
            err = kReadErr;
            break;
          }
        }
        else{
          properties_log("ERROR: property is not readable!");
          err = kNotReadableErr;
        }
        iid_tmp++;
      }
      return err;
    }
    else{
      iid_tmp++;  // next service or property
    }
    
    // if read single property
    for(j = 0; NULL != service_table[i].properties[j].type; j++){
      if(iid == iid_tmp){
        if( MICO_PROP_PERMS_RO & (service_table[i].properties[j].perms)){
          memset(iid_str, '\0', sizeof(iid_str));
          Int2Str((uint8_t*)iid_str, iid_tmp);
          err = kNoErr;
          switch(service_table[i].properties[j].format){
          case MICO_PROP_TYPE_INT:{
            properties_log("prop got: %s, iid=%d, value=%d", 
                           service_table[i].properties[j].type, iid_tmp, 
                           *((int*)service_table[i].properties[j].value));
            // prop->get (read hardware status)
            if(NULL != service_table[i].properties[j].get){
              service_table[i].properties[j].get(&service_table[i].properties[j], service_table[i].properties[j].arg,
                                                 service_table[i].properties[j].value, service_table[i].properties[j].value_len);
            }
            // return value
            json_object_object_add(outJsonObj, iid_str, json_object_new_int(*((int*)service_table[i].properties[j].value)));
            break;
          }
          case MICO_PROP_TYPE_FLOAT:{
            properties_log("prop got: %s, iid=%d, value=%f", 
                           service_table[i].properties[j].type, iid_tmp, 
                           *((float*)service_table[i].properties[j].value));
            // prop->get (read hardware status)
            if(NULL != service_table[i].properties[j].get){
              service_table[i].properties[j].get(&service_table[i].properties[j], service_table[i].properties[j].arg,
                                                 service_table[i].properties[j].value, service_table[i].properties[j].value_len);
            }
            // return value
            json_object_object_add(outJsonObj, iid_str, json_object_new_double(*((float*)service_table[i].properties[j].value)));
            break;
          }
          case MICO_PROP_TYPE_STRING:{
            properties_log("prop got: %s, iid=%d, value=%s", 
                           service_table[i].properties[j].type, iid_tmp, 
                           (char*)service_table[i].properties[j].value);
            // prop->get (read hardware status)
            if(NULL != service_table[i].properties[j].get){
              service_table[i].properties[j].get(&service_table[i].properties[j], service_table[i].properties[j].arg,
                                                 service_table[i].properties[j].value, service_table[i].properties[j].value_len);
            }
            // return value
            json_object_object_add(outJsonObj, iid_str, json_object_new_string((char*)service_table[i].properties[j].value));
            break;
          }
          case MICO_PROP_TYPE_BOOL:{
            properties_log("prop got: %s, iid=%d, value=%d", 
                           service_table[i].properties[j].type, iid_tmp, 
                           *((bool*)service_table[i].properties[j].value));
            // prop->get (read hardware status)
            if(NULL != service_table[i].properties[j].get){
              service_table[i].properties[j].get(&service_table[i].properties[j], service_table[i].properties[j].arg,
                                                 service_table[i].properties[j].value, service_table[i].properties[j].value_len);
            }
            // return value
            json_object_object_add(outJsonObj, iid_str, json_object_new_boolean(*((bool*)service_table[i].properties[j].value)));
            break;
          }
          default:
            properties_log("ERROR: property format unsupported !");
            err = kReadErr;
            break;
          }
        }
        else{
          properties_log("ERROR: property is not readable!");
          err = kNotReadableErr;
        }
        return err;
      }
      iid_tmp++;   // next property
    }
  }
  
exit:
  return err;
}

OSStatus mico_property_write_create(struct mico_service_t *service_table, 
                                    char *key, json_object *val, json_object *outJsonObj)
{
  OSStatus err = kNotFoundErr;
  int i = 0; 
  int j = 0;
  int iid = 0;
  int iid_tmp = 1;
  int ret = 0;
  
  int int_value = 0;
  double float_value = 0;
  bool boolean_value = false;
  
  require_action(service_table, exit, err = kParamErr);
  require_action(key, exit, err = kParamErr);
  require_action(val, exit, err = kParamErr);
  require_action(outJsonObj, exit, err = kParamErr);
  
  Str2Int((uint8_t*)key, &iid);
  properties_log("properties write iid=%d.", iid);
  for(i = 0; NULL != service_table[i].type; i++){
    if(iid == iid_tmp){  // if is a service, error operation
      properties_log("ERROR: can not write service: %s, iid=%d", service_table[i].type, iid_tmp);
      json_object_object_add(outJsonObj, key, json_object_new_int(MICO_PROP_WRITE_NOT_ALLOWED));
      return kNotWritableErr;
    }
    else{
      iid_tmp++;  // next service or property
    }
    
    // if write single property
    for(j = 0; NULL != service_table[i].properties[j].type; j++){
      if(iid == iid_tmp){
        if( (MICO_PROP_PERMS_WO & (service_table[i].properties[j].perms)) >> 1){
          // can write
          switch(service_table[i].properties[j].format){
          case MICO_PROP_TYPE_INT:{
            properties_log("prop got: %s, iid=%d, value=%d", 
                           service_table[i].properties[j].type, iid_tmp, 
                           *((int*)service_table[i].properties[j].value));
            // property set(hardware operation)
            int_value = json_object_get_int(val);
            if(NULL != service_table[i].properties[j].set){
              ret = service_table[i].properties[j].set(&service_table[i].properties[j],
                                                       service_table[i].properties[j].arg,
                                                       (void*)&int_value, sizeof(int));
              if (0 != ret){
                json_object_object_add(outJsonObj, key, json_object_new_int(MICO_PROP_WRITE_FAILED));
                err = kWriteErr;
              }
              else{
                // update property value
                *((int*)service_table[i].properties[j].value) =  int_value;
                err = kNoErr;
              }
            }
            else{
              // update property value
              *((int*)service_table[i].properties[j].value) =  int_value;
              err = kNoErr;
            }
            break;
          }
          case MICO_PROP_TYPE_FLOAT:{
            properties_log("prop got: %s, iid=%d, value=%f", 
                           service_table[i].properties[j].type, iid_tmp, 
                           *((float*)service_table[i].properties[j].value));
            // property set(hardware operation)
            float_value = json_object_get_double(val);
            if(NULL != service_table[i].properties[j].set){
              ret = service_table[i].properties[j].set(&service_table[i].properties[j], 
                                                       service_table[i].properties[j].arg,
                                                       (void*)&float_value, sizeof(double));
              if (0 != ret){
                json_object_object_add(outJsonObj, key, json_object_new_int(MICO_PROP_WRITE_FAILED));
                err = kWriteErr;
              }
              else{
                // update property value
                *((float*)service_table[i].properties[j].value) =  float_value;
                err = kNoErr;
              }
            }
            else{
              // update property value
              *((float*)service_table[i].properties[j].value) =  float_value;
              err = kNoErr;
            }
            break;
          }
          case MICO_PROP_TYPE_STRING:{
            properties_log("prop got: %s, iid=%d, value=%s", 
                           service_table[i].properties[j].type, iid_tmp, 
                           (char*)service_table[i].properties[j].value);
            // property set(hardware operation)
            if(NULL != service_table[i].properties[j].set){
              ret = service_table[i].properties[j].set(&service_table[i].properties[j], 
                                                       service_table[i].properties[j].arg,
                                                       (void*)(json_object_get_string(val)), strlen(json_object_get_string(val)));
              if (0 != ret){
                json_object_object_add(outJsonObj, key, json_object_new_int(MICO_PROP_WRITE_FAILED));
                err = kWriteErr;
              }
              else{
                // update property value
                memset((char*)(service_table[i].properties[j].value), '\0', strlen((char*)(service_table[i].properties[j].value)));
                strncpy((char*)(service_table[i].properties[j].value), json_object_get_string(val), strlen(json_object_get_string(val)));
                err = kNoErr;
              }
            }else{
              // update property value
              memset((char*)(service_table[i].properties[j].value), '\0', strlen((char*)(service_table[i].properties[j].value)));
              strncpy((char*)(service_table[i].properties[j].value), json_object_get_string(val), strlen(json_object_get_string(val)));
              err = kNoErr;
            }
            break;
          }
          case MICO_PROP_TYPE_BOOL:{
            properties_log("prop got: %s, iid=%d, value=%d", 
                           service_table[i].properties[j].type, iid_tmp, 
                           *((bool*)service_table[i].properties[j].value));
            // property set(hardware operation)
            boolean_value = json_object_get_boolean(val);
            if(NULL != service_table[i].properties[j].set){
              ret = service_table[i].properties[j].set(&service_table[i].properties[j], 
                                                       service_table[i].properties[j].arg,
                                                       (void*)&boolean_value, sizeof(bool));
              if (0 != ret){
                json_object_object_add(outJsonObj, key, json_object_new_int(MICO_PROP_WRITE_FAILED));
                err = kWriteErr;
              }
              else{
                // update property value
                *((bool*)service_table[i].properties[j].value) =  boolean_value;
                err = kNoErr;
              }
            }else{
              // update property value
              *((bool*)service_table[i].properties[j].value) =  boolean_value;
              err = kNoErr;
            }
            break;
          }
          default:
            properties_log("ERROR: Unsupported format!");
            json_object_object_add(outJsonObj, key, json_object_new_int(MICO_PROP_WRITE_FAILED));
            err = kWriteErr;
            break;
          }
        }
        else{
          properties_log("ERROR: property is read only!");
          json_object_object_add(outJsonObj, key, json_object_new_int(MICO_PROP_WRITE_NOT_ALLOWED));
          err = kNotWritableErr;
        }
        return err;
      }
      iid_tmp++;   // prop iid +1
    }
  }
  
  // property not found
  if(kNotFoundErr == err){
    properties_log("ERROR: property not found!");
    json_object_object_add(outJsonObj, key, json_object_new_int(MICO_PROP_NOT_FOUND));
  }
  
exit:
  return err;
}


OSStatus add_top(json_object **outTop, char* const service_name, json_object* services)
{
  OSStatus err = kNoErr;
  json_object *object;
  
  object = json_object_new_object();
  
  
  json_object_object_add(object, service_name, services);
  *outTop = object;
  
  return err;
}

OSStatus add_service(json_object* services, 
                     const char* type_name,  const char* type_content, 
                     const char* iid_name,  int iid_value,
                     const char* properties_name,  json_object *properties)
{
  OSStatus err;
  json_object *object;
  err = kNoErr;
  
  object = json_object_new_object();
  require_action(object, exit, err = kNoResourcesErr);
  json_object_object_add(object, type_name, json_object_new_string(type_content));
  json_object_object_add(object, iid_name, json_object_new_int(iid_value));
  json_object_object_add(object, properties_name, properties);
  json_object_array_add(services, object);
  
exit:
  return err;
}

OSStatus add_property(json_object* properties,  struct mico_prop_t property, int iid)
{
  OSStatus err = kUnknownErr;
  json_object *object, *perms_array;
  
  object = json_object_new_object();
  require_action(object, exit, err = kNoResourcesErr);
  
  // type &&¡¡iid
  json_object_object_add(object, "type", json_object_new_string(property.type));
  json_object_object_add(object, "iid", json_object_new_int(iid));
  
  // value && format
  switch(property.format){
  case MICO_PROP_TYPE_INT:{
    json_object_object_add(object, "value", json_object_new_int( *((int*)(property.value)) ) );
    json_object_object_add(object, "format", json_object_new_string("int"));
    break;
  }
  case MICO_PROP_TYPE_STRING:{
    json_object_object_add(object, "value", json_object_new_string( (char*)(property.value)) );
    json_object_object_add(object, "format", json_object_new_string("string"));
    break;
  }
  case MICO_PROP_TYPE_FLOAT:{
    json_object_object_add(object, "value", json_object_new_double( *((double*)(property.value))) );
    json_object_object_add(object, "format", json_object_new_string("float"));
    break;
  }
  case MICO_PROP_TYPE_BOOL:{
    json_object_object_add(object, "value", json_object_new_boolean( *((bool*)(property.value))) );
    json_object_object_add(object, "format", json_object_new_string("bool"));
    break;
  }
  default:{
    err= kParamErr;
    //break;
    goto exit;
  }
  }
  
  // value_len
  //json_object_object_add(object, "value_len", json_object_new_int(*((uint32_t*)(property.value_len))));
  
  // perms
  perms_array = json_object_new_array();
  require_action(perms_array, exit, err = kNoResourcesErr);
  
  if( MICO_PROP_PERMS_RO & (property.perms) ){
    json_object_array_add(perms_array, json_object_new_string("pr"));
  }
  if( (MICO_PROP_PERMS_WO & (property.perms)) >> 1 ) {
    json_object_array_add(perms_array, json_object_new_string("pw"));
  }
  if( (MICO_PROP_PERMS_EV & (property.perms)) >> 2 ) {
    json_object_array_add(perms_array, json_object_new_string("ev"));
  }
  json_object_object_add(object, "perms", perms_array);
  
  // add to property table
  json_object_array_add(properties, object);
  err = kNoErr;
  
exit:
  if(err != kNoErr){
    if (NULL != object){
      json_object_put(object);
      object = NULL;
    }
    if(NULL != perms_array){
      json_object_put(perms_array);
      perms_array = NULL;
    }
  }
  return err;
}

json_object* create_dev_info_json_object(struct mico_service_t service_table[])
{
  OSStatus err = kUnknownErr;
  properties_log_trace();
  json_object *properties = NULL, *services = NULL, *mainObject = NULL;
  int i = 0, j = 0;
  const char *pServiceType = NULL;
  const char *pPropertyType = NULL;
  int iid = 1;
  
  services = json_object_new_array();
  require( services, exit );
  err = add_top(&mainObject, "services", services);
  
  for(i = 0, pServiceType = service_table[0].type; NULL != pServiceType; ){
    properties = json_object_new_array();
    require_action( properties, exit, err = kNoResourcesErr );
    err = add_service(services, "type", pServiceType, 
                      "iid", iid, "properties", properties);
    require_noerr( err, exit );
    iid++;
    
    for(j = 0, pPropertyType = service_table[i].properties[0].type;  NULL != pPropertyType; ){
      err = add_property(properties, service_table[i].properties[j], iid);
      require_noerr( err, exit );
      iid++;
      j++;
      pPropertyType = service_table[i].properties[j].type;
    }
    
    i++;
    pServiceType = service_table[i].type;
  }
  
exit:
  if(err != kNoErr && mainObject){
    json_object_put(mainObject);
    mainObject = NULL;
  }
  return mainObject;
}

/* read multiple properties;
* input:  json object of property iids to read, like {"1":1, "2":2}, 
*   NOTE: function get iid from value of key:value pair.
* return: success read properties json object like {"1":100, "2":99}
*         if no property read success, return null object "{}",
*         if error, return value is NULL.
*/
json_object*  mico_read_properties(struct mico_service_t *service_table, 
                                   json_object *prop_read_list_obj)
{
  json_object *outJsonObj = NULL;
  int iid = 0;
  
  require( service_table, exit );
  require( prop_read_list_obj, exit );
  
  outJsonObj = json_object_new_object();
  require( outJsonObj, exit );
  
  json_object_object_foreach(prop_read_list_obj, key, val) {
    iid = json_object_get_int(val);
    mico_property_read_create(service_table, key, iid, outJsonObj);
  }
  
exit:
  return outJsonObj;
}

/* write multiple properties
* input:  json object of property iids to write, like {"1":100, "2":99},
* return: if all write succeed return status {"status": 0},
*         else return properties && status paris of properties wrote failed, like: {"1": -70401}
*         if error, return value is NULL.
*/
json_object*  mico_write_properties(struct mico_service_t *service_table, 
                                    json_object *prop_write_list_obj)
{
  OSStatus err = kUnknownErr;
  json_object *outJsonObj = NULL;
  bool all_write_succeed = true;
  
  require( service_table, exit );
  require( prop_write_list_obj, exit );
  
  outJsonObj = json_object_new_object();
  require( outJsonObj, exit );
  
  json_object_object_foreach(prop_write_list_obj, key, val) {
    err = mico_property_write_create(service_table, key, val, outJsonObj);
    if(kNoErr != err){
      all_write_succeed = false;  // not all property write success
    }
  }
  
  // all properties wrote success report
  if(all_write_succeed){
    json_object_object_add(outJsonObj, MICO_PROP_WRITE_STATUS, json_object_new_int(MICO_PROP_WRITE_SUCCESS));
  }
  
exit:
  return outJsonObj;
}
