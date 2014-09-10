/**
  ******************************************************************************
  * @file    MicoPlatform.h 
  * @author  William Xu
  * @version V1.0.0
  * @date    05-May-2014
  * @brief   This file provides all the headers of MCU peripheral operations 
  *          provided by MICO.
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

#ifndef __MICOPLATFORM_H__
#define __MICOPLATFORM_H__

#pragma once

#include "common.h"

#include "RingBufferUtils.h"
#include "platform.h" /* This file is unique for each platform */

#include "MicoDrivers/MICODriverUART.h"
#include "MicoDrivers/MICODriverGpio.h"
#include "MicoDrivers/MICODriverPwm.h"
#include "MicoDrivers/MICODriverSpi.h"
#include "MicoDrivers/MICODriverI2c.h"
#include "MicoDrivers/MICODriverRtc.h"
#include "MicoDrivers/MICODriverWdg.h"
#include "MicoDrivers/MICODriverAdc.h"
#include "MicoDrivers/MICODriverRng.h"

#ifdef __cplusplus
extern "C" {
#endif

#define mico_mcu_powersave_config MicoMcuPowerSaveConfig

/** @defgroup MICO_PLATFORM  MICO Hardware Abstract Layer APIs
* @brief Control hardware peripherals on different platfroms using standard HAL API functions
* 
*/

/** @addtogroup MICO_PLATFORM
  * @{
  */

/** \defgroup platform_misc Task switching, Reboot, and Power save modes
  @{
 */

#define ENABLE_INTERRUPTS   __asm("CPSIE i")  /**< Enable interrupts to start task switching in MICO RTOS. */
#define DISABLE_INTERRUPTS  __asm("CPSID i")  /**< Disable interrupts to stop task switching in MICO RTOS. */


/** @brief    Software reboot the MICO hardware
  *
  * @param    none
  * @return   none
  */
void MicoSystemReboot(void);

/** @brief    Software reboot the MICO hardware
  *
  * @param    none
  * @return   none
  */
void MicoSystemStandBy(void);

/** @brief    Enables the MCU to enter deep sleep mode when all threads are suspended.
  *
  * @note:    When all threads are suspended, mcu can shut down some peripherals to 
  *           save power. For example, STM32 enters STOP mode in this condition, 
  *           its peripherals are not working and needs to be wake up by an external
  *           interrupt or MICO core's internal timer. So if you are using a peripherals,  
  *           you should disable this function temporarily.
  *           To make this function works, you should not disable the macro in MicoDefault.h: 
  *           MICO_DISABLE_MCU_POWERSAVE
  *
  * @param    enable : 1 = enable MCU powersave, 0 = disable MCU powersave
  * @return   none
  */
void MicoMcuPowerSaveConfig( int enable );

/**
  * @}
  */


/**
  * @}
  */
#ifdef __cplusplus
} /*extern "C" */
#endif


#endif

