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

#include "wwd_constants.h"

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

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************
 * @cond                Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/
/* SPI mode constants */
#define SPI_CLOCK_RISING_EDGE  ( 1 << 0 )
#define SPI_CLOCK_FALLING_EDGE ( 0 << 0 )
#define SPI_CLOCK_IDLE_HIGH    ( 1 << 1 )
#define SPI_CLOCK_IDLE_LOW     ( 0 << 1 )
#define SPI_USE_DMA            ( 1 << 2 )
#define SPI_NO_DMA             ( 0 << 2 )
#define SPI_MSB_FIRST          ( 1 << 3 )
#define SPI_LSB_FIRST          ( 0 << 3 )

#define I2C_DEVICE_DMA_MASK_POSN 0
#define I2C_DEVICE_NO_DMA    (0 << I2C_DEVICE_DMA_MASK_POSN)
#define I2C_DEVICE_USE_DMA   (1 << I2C_DEVICE_DMA_MASK_POSN)


/******************************************************
 *                   Enumerations
 ******************************************************/



/******************************************************
 *                 Type Definitions
 ******************************************************/

typedef void (*wiced_gpio_irq_handler_t)( void* arg );

/******************************************************
 *                    Structures
 ******************************************************/



typedef struct
{
    uint8_t sec;
    uint8_t min;
    uint8_t hr;
    uint8_t weekday;/* 1-sunday... 7-saturday */
    uint8_t date;
    uint8_t month;
    uint8_t year;
}wiced_rtc_time_t;

/******************************************************
 *                     Variables
 ******************************************************/

#ifdef WICED_PLATFORM_INCLUDES_SPI_FLASH
extern wiced_spi_device_t wiced_spi_flash;
#endif

/******************************************************
 * @endcond           Function Declarations
 ******************************************************/

/*****************************************************************************/
/** @defgroup platform       Platform functions
 *
 *  WICED hardware platform functions
 */
/*****************************************************************************/

/*****************************************************************************/
/** @addtogroup adc       ADC
 *  @ingroup platform
 *
 * Analog to Digital Converter (ADC) Functions
 *
 *
 *  @{
 */
/*****************************************************************************/


/** Initialises an ADC interface
 *
 * Prepares an ADC hardware interface for sampling
 *
 * @param adc            : the interface which should be initialised
 * @param sampling_cycle : sampling period in number of ADC clock cycles. If the
 *                         MCU does not support the value provided, the closest
 *                         supported value is used.
 *
 * @return    WICED_SUCCESS : on success.
 * @return    WICED_ERROR   : if an error occurred with any step
 */
wiced_result_t wiced_adc_init( wiced_adc_t adc, uint32_t sampling_cycle );



/** Takes a single sample from an ADC interface
 *
 * Takes a single sample from an ADC interface
 *
 * @param adc    : the interface which should be sampled
 * @param output : pointer to a variable which will receive the sample
 *
 * @return    WICED_SUCCESS : on success.
 * @return    WICED_ERROR   : if an error occurred with any step
 */
wiced_result_t wiced_adc_take_sample( wiced_adc_t adc, uint16_t* output );


/** Takes multiple samples from an ADC interface
 *
 * Takes multiple samples from an ADC interface and stores them in
 * a memory buffer
 *
 * @param adc           : the interface which should be sampled
 * @param buffer        : a memory buffer which will receive the samples
 *                        Each sample will be uint16_t little endian.
 * @param buffer_length : length in bytes of the memory buffer.
 *
 *
 * @return    WICED_SUCCESS : on success.
 * @return    WICED_ERROR   : if an error occurred with any step
 */
wiced_result_t wiced_adc_take_sample_stream( wiced_adc_t adc, void* buffer, uint16_t buffer_length );


/** De-initialises an ADC interface
 *
 * Turns off an ADC hardware interface
 *
 * @param  adc : the interface which should be de-initialised
 *
 * @return    WICED_SUCCESS : on success.
 * @return    WICED_ERROR   : if an error occurred with any step
 */
wiced_result_t wiced_adc_deinit( wiced_adc_t adc );

/** @} */


/*****************************************************************************/
/** @addtogroup watchdog       Watchdog
 *  @ingroup platform
 *
 * Watchdog Timer Functions
 *
 *
 *  @{
 */
/*****************************************************************************/


/** Kick the system watchdog.
 *
 * Resets (kicks) the timer of the system watchdog.
 * This MUST be done done by the App regularly.
 *
 * @return    WICED_SUCCESS : on success.
 * @return    WICED_ERROR   : if an error occurred with any step
 */
wiced_result_t wiced_watchdog_kick( void );


/** @} */
/*****************************************************************************/
/** @addtogroup mcupowersave       Powersave
 *  @ingroup platform
 *
 * MCU Powersave Functions
 *
 *
 *  @{
 */
/*****************************************************************************/

/** Enables the MCU to enter powersave mode.
 *
 * @warning   If the MCU drives the sleep clock input pin of the WLAN chip,   \n
 *            ensure the 32kHz clock output from the MCU is maintained while  \n
 *            the MCU is in powersave mode. The WLAN sleep clock reference is \n
 *            typically configured in the file:                               \n
 *            <WICED-SDK>/include/platforms/<PLATFORM_NAME>/platform.h
 * @return    void
 */
void wiced_platform_mcu_enable_powersave( void );


/** Stops the MCU entering powersave mode.
 *
 * @return    void
 */
void wiced_platform_mcu_disable_powersave( void );

/** @} */

/**
 * This function will return the value of time read from the on board CPU real time clock. Time value must be given in the format of
 * the structure wiced_rtc_time_t
 *
 * @param time        : pointer to a time structure
 *
 * @return    WICED_SUCCESS : on success.
 * @return    WICED_ERROR   : if an error occurred with any step
 */
wiced_result_t wiced_platform_get_rtc_time(wiced_rtc_time_t* time);

/**
 * This function will set MCU RTC time to a new value. Time value must be given in the format of
 * the structure wiced_rtc_time_t
 *
 * @param time        : pointer to a time structure
 *
 * @return    WICED_SUCCESS : on success.
 * @return    WICED_ERROR   : if an error occurred with any step
 */
wiced_result_t wiced_platform_set_rtc_time(wiced_rtc_time_t* time);


void MicoSystemReboot(void);

void MicoSystemStandBy(void);


#ifdef __cplusplus
} /*extern "C" */
#endif


#endif

