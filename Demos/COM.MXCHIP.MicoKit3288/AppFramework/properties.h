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

// property type
struct mico_prop_t{
  const char *type;
  //uint32_t iid;                // auto calcuate
  void* value;
  uint32_t *value_len;   
  enum mico_prop_data_type_t format;                    // bool, int, float, string
  uint8_t perms;                 // RO | WO | RW | EV
  int (*set)(struct mico_prop_t *prop, void *arg, void *val, uint32_t val_len);  // set value, schedure
  int (*get)(struct mico_prop_t *prop, void *arg, void *val, uint32_t *val_len);  // get value, schedure status
  void *arg;                     // arg for set or get or notify watch
  int (*notify)(struct mico_prop_t *prop, void *arg, void *val, uint32_t *val_len);  // create notify data
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
// property read
OSStatus  mico_property_read(mico_Context_t * const inContext, struct mico_service_t *service_table, int iid);
// property write
OSStatus  mico_property_write(mico_Context_t * const inContext, struct mico_service_t *service_table, 
                              int iid, void *val, uint32_t val_len);

#endif // __MICO_DEVICE_PROPERTIES_H_
