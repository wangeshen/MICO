/** @file update.h
 * @brief This file provides functions to update the target flash contents according the 
 * update tags written by OTA functions
 *
 * <!-- Copyright 2012 by MXCHIP Corporation. All rights reserved.        *80*-->
 */

#include "Common.h"
/* Upgrade iamge should save this table to flash */
typedef struct  _boot_table_t {
  uint32_t start_address; // the address of the bin saved on flash.
  uint32_t length; // file real length
  uint8_t version[8];
  uint8_t type; // B:bootloader, P:boot_table, A:application, D: 8782 driver
  uint8_t upgrade_type; //u:upgrade, 
  uint8_t reserved[6];
}boot_table_t;



typedef enum
{
  PROGRAM_TARGET_FAILED,
  ERASE_UPDATE_TAG_FAILED,
  ERASE_UPDATE_DATA_FAILED,
} UPDATE_ERROR_TypeDef;

OSStatus update(void);

