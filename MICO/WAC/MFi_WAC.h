/**
  ******************************************************************************
  * @file    Platform.h 
  * @author  William Xu
  * @version V1.0.0
  * @date    05-May-2014
  * @brief   This file provides all the headers to some basic Peripherals.
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

#ifndef __WAC_H
#define __WAC_H

#include "MICODefine.h"

#define BUNDLE_SEED_ID          "C6P64J2MZX"  
#define EA_PROTOCOL             "com.issc.datapath"

/**
 *   @brief Parameters controlled by the platform to configure the WAC process. 
 */
typedef struct
{
    uint8_t macAddress[ 6 ];        /**< REQUIRED: Accessory MAC address, e.g. 00:11:22:33:44:55. */
    
    bool    isUnconfigured;         /**< TRUE/FALSE: whether the accessory is unconfigured. Should be true for current cases. */
    bool    supportsAirPlay;        /**< TRUE/FALSE: whether the accessory supports AirPlay. */
    bool    supportsAirPrint;       /**< TRUE/FALSE: whether the accessory supports AirPrint. */
    bool    supports2_4GHzWiFi;     /**< TRUE/FALSE: whether the accessory supports 2.4 GHz Wi-Fi. */
    bool    supports5GHzWiFi;       /**< TRUE/FALSE: whether the accessory supports 5 GHz Wi-Fi. */
    bool    supportsWakeOnWireless; /**< TRUE/FALSE: whether the accessory supports Wake On Wireless. */
    
    char    *firmwareRevision;      /**< REQUIRED: Version of the accessory's firmware, e.g. 1.0.0. */
    char    *hardwareRevision;      /**< REQUIRED: Version of the accessory's hardware, e.g. 1.0.0. */
    char    *serialNumber;          /**< OPTIONAL: Accessory's serial number. */
    
    char    *name;                  /**< REQUIRED: Name of the accessory. */
    char    *model;                 /**< REQUIRED: Model name of the accessory. */
    char    *manufacturer;          /**< REQUIRED: Manufacturer name of the accessory. */
    
    char    **eaProtocols;          /**< OPTIONAL: Array of EA Protocol strings. */
    uint8_t numEAProtocols;         /**< OPTIONAL: Number of EA Protocol strings contained in the eaProtocols array. */
    char    *eaBundleSeedID;        /**< OPTIONAL: Accessory manufacturer's BundleSeedID. */
    
} WACPlatformParameters_t;

OSStatus startMFiWAC( mico_Context_t * const inContext, WACPlatformParameters_t *inWACPara, int timeOut );

#endif /* __WAC_H */