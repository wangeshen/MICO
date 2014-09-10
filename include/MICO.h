/**
  ******************************************************************************
  * @file    MICO.h 
  * @author  William Xu
  * @version V1.0.0
  * @date    05-May-2014
  * @brief   This file provides all the headers of APIs provided by MICO.
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

/** \mainpage MICO 

  This documentation describes the MICO APIs.
  It consists of:
     - MICO Hardware Abstract Layer   
     - MICO RTOS and Timer APIs       
     - MICO Wi-Fi Connectivith APIs   
     - MICO BSD Socket APIs           
 */

#ifndef __MICO_H_
#define __MICO_H_

#include "Debug.h"
#include "Common.h" 
#include "MicoRTOS.h"
#include "MicoWlan.h"
#include "MicoSocket.h"
#include "MicoAlgorithm.h"

#define MicoGetRfVer                wlan_driver_version
#define MicoGetVer                  system_lib_version
#define MicoInit                    mxchipInit
#define MicoGetMemoryInfo           mico_memory_info

/** \defgroup MICO_Core_APIs MICO Core APIs
  This file defines all structures and symbols for MICO core:
   - MICO Core exported symbols
   - MICO Core exported functions
 */

/** @addtogroup MICO_Core_APIs
  * @{
  */

/** \defgroup MICO_Init_Info Initialization and Tools
  @{
 */

typedef struct  {
  int num_of_chunks;  /**< number of free chunks*/
  int total_memory;  /**< maximum total allocated space*/
  int allocted_memory; /**< total allocated space*/
  int free_memory; /**< total free space*/
} micoMemInfo_t;

/**
  * @brief  Get RF driver's version.
  *
  * @note   Create a memery buffer to store the version characters.
  *         THe input buffer length should be 40 bytes at least.
  * @note   This must be executed after micoInit().
  * @param  inVersion: Buffer address to store the RF driver. 
  * @param  inLength: Buffer size. 
  *
  * @return None
  */
void MicoGetRfVer( char* outVersion, uint8_t inLength );

/**
  * @brief  Get MICO's version.
  *
  * @param  None 
  *
  * @return Point to the MICO's version string.
  */
char* MicoGetVer( void );

/**
  * @brief  Initialize the TCPIP stack thread, RF driver thread, and other
            supporting threads needed for wlan connection. Do some necessary
            initialization
  *
  * @param  None
  *
  * @return None
  */
void MicoInit( void );

/**
  * @brief  Get memory usage information
  *
  * @param  None 
  *
  * @return Point to structure of memory usage information in heap
  */
micoMemInfo_t* MicoGetMemoryInfo( void );

#endif /* __MICO_H_ */

/**
  * @}
  */

/**
  * @}
  */

