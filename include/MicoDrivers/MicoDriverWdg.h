/**
  ******************************************************************************
  * @file    MICORTOS.h 
  * @author  William Xu
  * @version V1.0.0
  * @date    05-May-2014
  * @brief   This file provides all the headers of GPIO operation functions.
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

#ifndef __MICODRIVERWDG_H__
#define __MICODRIVERWDG_H__

#pragma once
#include "Common.h"
#include "platform.h"

/** @addtogroup MICO_PLATFORM
* @{
*/

/******************************************************
 *                   Macros
 ******************************************************/  

#define PlatformWDGInitialize   MicoWdgInitialize /**< For API compatiable with older version */
#define PlatformWDGReload       MicoWdgReload     /**< For API compatiable with older version */
#define PlatformWDGFinalize     MicoWdgFinalize   /**< For API compatiable with older version */

/******************************************************
 *@cond              Enumerations
 ******************************************************/

/******************************************************
 *@endcond         Type Definitions
 ******************************************************/

 /******************************************************
 *                    Structures
 ******************************************************/


/******************************************************
 *                     Variables
 ******************************************************/

/******************************************************
 * @endcond           Function Declarations
 ******************************************************/

/** @defgroup MICO_WDG MICO WDG Driver
* @brief  Hardware watch dog Functions (WDG) Functions
* @note   These Functions are used by MICO's system monitor service, If any defined system monitor 
*          cannot be reloaded for some reason, system monitor service use this hardware watch dog 
*          to perform a system reset. So the watch dog reload time should be greater than system 
*          monitor's refresh time.
* @{
*/

/**
 * This function will initialize the on board CPU hardware watch dog
 *
 * @param timeout        : Watchdag timeout, application should call MicoWdgReload befor timeout.
 *
 * @return    kNoErr        : on success.
 * @return    kGeneralErr   : if an error occurred with any step
 */
OSStatus MicoWdgInitialize( uint32_t timeout );

/**
 * Reload watchdog counter.
 *
 * @param     none
 * @return    none
 */
void MicoWdgReload( void );

/**
 * This function performs any platform-specific cleanup needed for hardware watch dog.
 *
 * @param     none
 *
 * @return    kNoErr        : on success.
 * @return    kGeneralErr   : if an error occurred with any step
 */
OSStatus MicoWdgFinalize( void );

/** @} */
/** @} */

#endif


