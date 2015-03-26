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
//#include "MICOAppDefine.h"
#include "JSON-C/json.h"

#ifndef __MICO_DEVICE_PROPERTIES_H_
#define __MICO_DEVICE_PROPERTIES_H_

#define MAX_PROP_NUMBER      (10+1)
#define MAX_SERVICE_NUMBER   (10+1)

// property data type
enum mico_prop_data_type_t{
  MICO_PROP_TYPE_INT = 0,
  MICO_PROP_TYPE_FLOAT = 1,
  MICO_PROP_TYPE_BOOL = 2,
  MICO_PROP_TYPE_STRING = 3,
  MICO_PROP_TYPE_MAX
};

// property data attribute
#define MICO_PROP_PERMS_RO    0x01
#define MICO_PROP_PERMS_WO    0x02
#define MICO_PROP_PERMS_RW    0x03
#define MICO_PROP_PERMS_EV    0x04

// status code

#define MICO_PROP_READ_STATUS        "status"
#define MICO_PROP_WRITE_STATUS       "status"
#define MICO_PROP_READ_SUCCESS        0
#define MICO_PROP_WRITE_SUCCESS       0
#define MICO_PROP_READ_FAILED         -70401
#define MICO_PROP_WRITE_FAILED        -70402
#define MICO_PROP_WRITE_NOT_ALLOWED   -70403
#define MICO_PROP_NOT_FOUND           -70404

// property type
struct mico_prop_t{
  const char *type;
  //uint32_t iid;                // auto calcuate
  void* value;                   // current property data
  uint32_t *value_len;           // current property data len
  enum mico_prop_data_type_t format;                    // bool, int, float, string
  uint8_t perms;                 // data permission: RO | WO | RW | EV
  int (*set)(struct mico_prop_t *prop, void *arg, void *val, uint32_t val_len);  // hardware operation, update prop.value if succeed.
  int (*get)(struct mico_prop_t *prop, void *arg, void *val, uint32_t *val_len);  // get hardware status value
  void *arg;                     // arg for set or get or notify function
  int (*notify_check)(struct mico_prop_t *prop, void *arg, void *val, uint32_t *val_len);  // check data to notify
  bool *event;                   // notification status flag
  void *maxValue;                // max value for int or float
  void *minValue;                // min value for int or float
  void *minStep;                 // min step for int or float
  uint32_t maxLen;               // max length for string
};

// service type
 struct mico_service_t {
  const char *type;
  //int iid£»        // auto calculate
  struct mico_prop_t  properties[MAX_PROP_NUMBER];
};

// property notify
OSStatus  mico_property_notify(mico_Context_t * const inContext, struct mico_service_t *service_table);

// read multiple properties
json_object*  mico_read_properties(struct mico_service_t *service_table, 
                               json_object *prop_read_list_obj);
// write multiple properties
json_object*  mico_write_properties(struct mico_service_t *service_table, 
                               json_object *prop_write_list_obj);

// create dev_info json data
json_object* create_dev_info_json_object(struct mico_service_t service_table[]);
OSStatus add_top(json_object **outTop, char* const service_name, json_object* services);
OSStatus add_service(json_object* services, 
                     const char* type_name,  const char* type_content, 
                     const char* iid_name,  int iid_value,
                     const char* properties_name,  json_object *properties);
OSStatus add_property(json_object* properties,  struct mico_prop_t property, int iid);

#endif // __MICO_DEVICE_PROPERTIES_H_
