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

// property notify task
OSStatus  mico_property_notify(mico_Context_t * const inContext, struct mico_service_t *service_table)
{
  OSStatus err = kUnknownErr;
  int i = 0; 
  int j = 0;
  int ret = 0;
  
  //properties_log("properties notify task...");
  for(i = 0; NULL != service_table[i].type; i++)
  {
    for(j = 0; NULL != service_table[i].properties[j].type; j++){
      // call property notify_check func
      if((NULL != service_table[i].properties[j].event) && (*(service_table[i].properties[j].event))){
        if(NULL != service_table[i].properties[j].notify_check){
          ret = service_table[i].properties[j].notify_check(&(service_table[i].properties[j]), NULL, NULL, NULL);
          if(1 == ret){
            //            properties_log("[Notify]service[%d]: %s, property[%d]: %s", 
            //                           i, service_table[i].type,
            //                           j, service_table[i].properties[j].type);
          }
        }
        else{
          notify_check_default(&(service_table[i].properties[j]),NULL,NULL,NULL);
        }
      }
    }
    //properties_log("property num=%d", j);
  }
  //properties_log("service num=%d", i);
  
  err = kNoErr;
  return err;
}

OSStatus mico_property_read_create(struct mico_service_t *service_table, int iid, 
                                   const char *key, json_object *outJsonObj)
{
  OSStatus err = kUnknownErr;
  int i = 0; 
  int j = 0;
  int iid_tmp = 1;
  char iid_str[16] = {0};
  
  //properties_log("properties read iid=%d.", iid);
  for(i = 0; NULL != service_table[i].type; i++){
    // if read a service, get all properties of the service
    if(iid == iid_tmp){
      properties_log("service got: %s, iid=%d", service_table[i].type, iid_tmp);
      iid_tmp++;   // jump to prop iid
      for(j = 0; NULL != service_table[i].properties[j].type; j++){
        //        properties_log("service[%d]: %s, property[%d]: %s", 
        //                       i, service_table[i].type,
        //                       j, service_table[i].properties[j].type);
        switch(service_table[i].properties[j].format){
        case MICO_PROP_TYPE_INT:{
          properties_log("prop got: %s, iid=%d, value=%d", 
                         service_table[i].properties[j].type, iid_tmp, 
                         *((int*)service_table[i].properties[j].value));
          memset(iid_str, '\0', sizeof(iid_str));
          Int2Str((uint8_t*)iid_str, iid_tmp);
          json_object_object_add(outJsonObj, iid_str, json_object_new_int(*((int*)service_table[i].properties[j].value)));
          break;
        }
        case MICO_PROP_TYPE_FLOAT:{
          properties_log("prop got: %s, iid=%d, value=%f", 
                         service_table[i].properties[j].type, iid_tmp, 
                         *((float*)service_table[i].properties[j].value));
          memset(iid_str, '\0', sizeof(iid_str));
          Int2Str((uint8_t*)iid_str, iid_tmp);
          json_object_object_add(outJsonObj, iid_str, json_object_new_double(*((float*)service_table[i].properties[j].value)));
          break;
        }
        case MICO_PROP_TYPE_STRING:{
          properties_log("prop got: %s, iid=%d, value=%s", 
                         service_table[i].properties[j].type, iid_tmp, 
                         (char*)service_table[i].properties[j].value);
          memset(iid_str, '\0', sizeof(iid_str));
          Int2Str((uint8_t*)iid_str, iid_tmp);
          json_object_object_add(outJsonObj, iid_str, json_object_new_string((char*)service_table[i].properties[j].value));
          break;
        }
        case MICO_PROP_TYPE_BOOL:{
          properties_log("prop got: %s, iid=%d, value=%d", 
                         service_table[i].properties[j].type, iid_tmp, 
                         *((bool*)service_table[i].properties[j].value));
          memset(iid_str, '\0', sizeof(iid_str));
          Int2Str((uint8_t*)iid_str, iid_tmp);
          json_object_object_add(outJsonObj, iid_str, json_object_new_boolean(*((bool*)service_table[i].properties[j].value)));
          break;
        }
        default:
          properties_log("ERROR: Unsupported format!");
          break;
        }
        iid_tmp++;
      }
      return kNoErr;
    }
    iid_tmp++;     // service iid +1
    
    // if read single property
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
          json_object_object_add(outJsonObj, key, json_object_new_int(*((int*)service_table[i].properties[j].value)));
          break;
        }
        case MICO_PROP_TYPE_FLOAT:{
          properties_log("prop got: %s, iid=%d, value=%f", 
                         service_table[i].properties[j].type, iid_tmp, 
                         *((float*)service_table[i].properties[j].value));
          json_object_object_add(outJsonObj, key, json_object_new_double(*((float*)service_table[i].properties[j].value)));
          break;
        }
        case MICO_PROP_TYPE_STRING:{
          properties_log("prop got: %s, iid=%d, value=%s", 
                         service_table[i].properties[j].type, iid_tmp, 
                         (char*)service_table[i].properties[j].value);
          json_object_object_add(outJsonObj, key, json_object_new_string((char*)service_table[i].properties[j].value));
          break;
        }
        case MICO_PROP_TYPE_BOOL:{
          properties_log("prop got: %s, iid=%d, value=%d", 
                         service_table[i].properties[j].type, iid_tmp, 
                         *((bool*)service_table[i].properties[j].value));
          json_object_object_add(outJsonObj, key, json_object_new_boolean(*((bool*)service_table[i].properties[j].value)));
          break;
        }
        default:
          properties_log("ERROR: Unsupported format!");
          break;
        }
        return kNoErr;
      }
      iid_tmp++;   // prop iid +1
    }
  }
  
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


OSStatus add_top(json_object **outTop, char* const service_name, json_object* services)
{
  OSStatus err;
  json_object *object;
  err = kNoErr;
  
  object = json_object_new_object();
  require_action(object, exit, err = kNoMemoryErr);
  
  json_object_object_add(object, service_name, services);
  *outTop = object;
  
exit:
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

// read multiple properties
json_object*  mico_read_properties(mico_Context_t * const inContext, 
                                   struct mico_service_t *service_table, 
                                   json_object *prop_read_list_obj)
{
  OSStatus err = kUnknownErr;
  json_object *outJsonObj = NULL;
  int iid = 0;
  require( prop_read_list_obj, exit );
  require( service_table, exit );
  
  outJsonObj = json_object_new_object();
  require( outJsonObj, exit );
  
  json_object_object_foreach(prop_read_list_obj, key, val) {
    iid = json_object_get_int(val);
    err = mico_property_read_create(service_table, iid, key, outJsonObj);
    if(kNoErr != err){
      // add read status code
      json_object_object_add(outJsonObj, key, json_object_new_int(MICO_PROP_READ_FAILED));
    }
  }
  
exit:
  return outJsonObj;
}

// write multiple properties
json_object*  mico_write_properties(mico_Context_t * const inContext, 
                                    struct mico_service_t *service_table, 
                                    json_object *prop_write_list_obj)
{
  //  OSStatus err = kUnknownErr;
  json_object *outJsonObj = NULL;
  
exit:
  return outJsonObj;
}
