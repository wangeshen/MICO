/**
  ******************************************************************************
  * @file    MICOPlatform.h 
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

/*
 * Copyright 2013, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */

/** @file
 *  Defines functions that access platform specific peripherals
 *
 */

#pragma once

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

#define ENABLE_INTERRUPTS   __asm("CPSIE i")
#define DISABLE_INTERRUPTS  __asm("CPSID i")

/*****************************************************************************/
/** @defgroup platform       Platform functions
 *
 *  MICO hardware platform functions
 */
/*****************************************************************************/

void MicoSystemReboot(void);

void MicoSystemStandBy(void);


#ifdef __cplusplus
} /*extern "C" */
#endif


#endif

