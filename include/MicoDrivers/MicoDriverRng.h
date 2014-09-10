/**
  ******************************************************************************
  * @file    MICODriverRng.h 
  * @author  William Xu
  * @version V1.0.0
  * @date    05-May-2014
  * @brief   This file provides all the headers of RNG operation functions.
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

#ifndef __MICODRIVERRNG_H__
#define __MICODRIVERRNG_H__

#pragma once
#include "Common.h"
#include "platform.h"



/** @addtogroup MICO_PLATFORM
* @{
*/

/******************************************************
 *@cond             Macros
 ******************************************************/  

#define PlatformRandomBytes MicoRandomNumberRead   /**< For API compatiable with older version */

/******************************************************
 *@endcond           Enumerations
 ******************************************************/


/******************************************************
 *                 Type Definitions
 ******************************************************/

 /******************************************************
 *                 Function Declarations
 ******************************************************/
 
/** @defgroup MICO_RNG MICO RNG Driver
 * @brief  Random Number Generater(RNG) Functions
 * @{
 */

/** Fill in a memory buffer with random data
 *
 * @param inBuffer        : Point to a valid memory buffer, this function will fill 
                            in this memory with random numbers after excuted
 * @param inByteCount     : Length of the memory buffer (bytes)
 *
 * @return    kNoErr        : on success.
 * @return    kGeneralErr   : if an error occurred with any step
 */
OSStatus MicoRandomNumberRead( void *inBuffer, int inByteCount );

/** @} */
/** @} */

#endif


