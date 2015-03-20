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

#ifndef __MICO_DEVICE_PROPERTIES_H_
#define __MICO_DEVICE_PROPERTIES_H_

// property data type
typedef enum {
  ACCESSORY_TYPE_NULL = 0,
  ACCESSORY_TYPE_RESERVED = 1,
  ACCESSORY_TYPE_SERVICE = 2,
  ACCESSORY_TYPE_PROPERTY = 3,
  ACCESSORY_TYPE_MAX
}mico_accessory_type_t;

// property data type
typedef enum {
  PROP_TYPE_INT = 0,
  PROP_TYPE_FLOAT = 1,
  PROP_TYPE_BOOL = 2,
  PROP_TYPE_STRING = 3,
  PROP_TYPE_MAX
}mico_prop_data_type_t;

// property data attribute
typedef enum {
  PROP_PERMS_RO = 0x01,
  PROP_PERMS_WO = 0x02,
  PROP_PERMS_RW = 0x03,
  PROP_PERMS_EV = 0x04,
  PROP_PERMS_MAX
}mico_prop_permission_type_t;

// property node
typedef struct _mico_dev_property_node_t{
  const char *type;              // UID
  const uint32_t iid;            // internal id in current device
  void *value;                   // property data value
  uint32_t value_len;            // data value len, int = sizefo(int), string = strlen("xxx")
  mico_prop_data_type_t format;    // data type, int, float, bool, ...
  uint8_t perms;                 // RO | WO | RW | EV
  int(*set)(void *arg);          // property set
  int(*get)(void *arg);          // property get
  void *maxValue;                // max value for int or float
  void *minValue;                // min value for int or float
  void *minStep;                 // min step for int or float
  uint32_t maxLen;               // max length for string
  bool event;                    // notification enable status
}mico_dev_property_node_t;

// service node
typedef struct _mico_dev_service_node_t{
  const char *type;
  int iid;
  mico_dev_property_node_t *property_table;
  int property_num;        // property number
}mico_dev_service_node_t;

OSStatus create_service_table(void);
json_object* create_dev_info_json_object(mico_dev_service_node_t service_table[], int size);

#endif // __MICO_DEVICE_PROPERTIES_H_
