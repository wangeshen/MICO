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
#include "wwd_constants.h"
#include "platform.h" /* This file is unique for each platform */

#include "MicoDrivers/MICODriverUART.h"
#include "MicoDrivers/MICODriverGpio.h"
//#include "MICOPlatform/MICOPlatformPWM.h"

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

typedef enum
{
    I2C_ADDRESS_WIDTH_7BIT,
    I2C_ADDRESS_WIDTH_10BIT,
    I2C_ADDRESS_WIDTH_16BIT,
} wiced_i2c_bus_address_width_t;

typedef enum
{
    I2C_LOW_SPEED_MODE,         /* 10Khz devices */
    I2C_STANDARD_SPEED_MODE,    /* 100Khz devices */
    I2C_HIGH_SPEED_MODE         /* 400Khz devices */
} wiced_i2c_speed_mode_t;

/******************************************************
 *                 Type Definitions
 ******************************************************/

typedef void (*wiced_gpio_irq_handler_t)( void* arg );

/******************************************************
 *                    Structures
 ******************************************************/

typedef struct
{
    wiced_spi_t  port;
    mico_gpio_t chip_select;
    uint32_t     speed;
    uint8_t      mode;
    uint8_t      bits;
} wiced_spi_device_t;

typedef struct
{
    const void* tx_buffer;
    void*       rx_buffer;
    uint32_t    length;
} wiced_spi_message_segment_t;

typedef struct
{
    wiced_i2c_t                   port;
    uint16_t                      address;       /* the address of the device on the i2c bus */
    wiced_i2c_bus_address_width_t address_width;
    uint8_t                       flags;
    wiced_i2c_speed_mode_t        speed_mode;    /* speed mode the device operates in */
} wiced_i2c_device_t;

typedef struct
{
    const void*  tx_buffer;  /* A pointer to the data to be transmitted. If NULL, the message is an RX message when 'combined' is FALSE */
    void*        rx_buffer;  /* A pointer to the data to be transmitted. If NULL, the message is an TX message when 'combined' is FALSE */
    uint16_t     tx_length;
    uint16_t     rx_length;
    uint16_t     retries;    /* Number of times to retry the message */
    wiced_bool_t combined;
    uint8_t      flags;      /* MESSAGE_DISABLE_DMA : if set, this flag disables use of DMA for the message */
} wiced_i2c_message_t;

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
/** @addtogroup spi       SPI
 *  @ingroup platform
 *
 * Serial Peripheral Interface (SPI) Functions
 *
 *
 *  @{
 */
/*****************************************************************************/


/** Initialises the SPI interface for a given SPI device
 *
 * Prepares a SPI hardware interface for communication as a master
 *
 * @param  spi : the SPI device to be initialised
 *
 * @return    WICED_SUCCESS : on success.
 * @return    WICED_ERROR   : if the SPI device could not be initialised
 */
wiced_result_t wiced_spi_init( const wiced_spi_device_t* spi );


/** Transmits and/or receives data from a SPI device
 *
 * @param  spi      : the SPI device to be initialised
 * @param  segments : a pointer to an array of segments
 * @param  number_of_segments : the number of segments to transfer
 *
 * @return    WICED_SUCCESS : on success.
 * @return    WICED_ERROR   : if an error occurred
 */
wiced_result_t wiced_spi_transfer( const wiced_spi_device_t* spi, wiced_spi_message_segment_t* segments, uint16_t number_of_segments );


/** De-initialises a SPI interface
 *
 * Turns off a SPI hardware interface
 *
 * @param  spi : the SPI device to be de-initialised
 *
 * @return    WICED_SUCCESS : on success.
 * @return    WICED_ERROR   : if an error occurred
 */
wiced_result_t wiced_spi_deinit( const wiced_spi_device_t* spi );


/** @} */
/*****************************************************************************/
/** @addtogroup i2c       I2C
 *  @ingroup platform
 *
 * Inter-IC bus (I2C) Functions
 *
 *
 *  @{
 */
/*****************************************************************************/


/** Initialises an I2C interface
 *
 * Prepares an I2C hardware interface for communication as a master
 *
 * @param  device : the device for which the i2c port should be initialised
 *
 * @return    WICED_SUCCESS : on success.
 * @return    WICED_ERROR   : if an error occurred during initialisation
 */
wiced_result_t wiced_i2c_init( wiced_i2c_device_t* device );


/** Checks whether the device is available on a bus or not
 *
 *
 * @param  device : the i2c device to be probed
 * @param  retries    : the number of times to attempt to probe the device
 *
 * @return    WICED_TRUE : device is found.
 * @return    WICED_FALSE: device is not found
 */
wiced_bool_t wiced_i2c_probe_device( wiced_i2c_device_t* device, int retries );


/** Initialize the wiced_i2c_message_t structure for i2c tx transaction
 *
 * @param message : pointer to a message structure, this should be a valid pointer
 * @param tx_buffer : pointer to a tx buffer that is already allocated
 * @param tx_buffer_length : number of bytes to transmit
 * @param retries    : the number of times to attempt send a message in case it can't not be sent
 * @param disable_dma : if true, disables the dma for current tx transaction. You may find it useful to switch off dma for short tx messages.
 *                     if you set this flag to 0, then you should make sure that the device flags was configured with I2C_DEVICE_USE_DMA. If the
 *                     device doesnt support DMA, the message will be transmitted with no DMA.
 *
 * @return    WICED_SUCCESS : message structure was initialised properly.
 * @return    WICED_BADARG: one of the arguments is given incorrectly
 */
wiced_result_t wiced_i2c_init_tx_message(wiced_i2c_message_t* message, const void* tx_buffer, uint16_t  tx_buffer_length, uint16_t retries , wiced_bool_t disable_dma);

/** Initialize the wiced_i2c_message_t structure for i2c rx transaction
 *
 * @param message : pointer to a message structure, this should be a valid pointer
 * @param rx_buffer : pointer to an rx buffer that is already allocated
 * @param rx_buffer_length : number of bytes to receive
 * @param retries    : the number of times to attempt receive a message in case device doesnt respond
 * @param disable_dma : if true, disables the dma for current rx transaction. You may find it useful to switch off dma for short rx messages.
 *                     if you set this flag to 0, then you should make sure that the device flags was configured with I2C_DEVICE_USE_DMA. If the
 *                     device doesnt support DMA, the message will be received not using DMA.
 *
 * @return    WICED_SUCCESS : message structure was initialised properly.
 * @return    WICED_BADARG: one of the arguments is given incorrectly
 */
wiced_result_t wiced_i2c_init_rx_message(wiced_i2c_message_t* message, void* rx_buffer, uint16_t rx_buffer_length, uint16_t retries , wiced_bool_t disable_dma);


/** Initialize the wiced_i2c_message_t structure for i2c combined transaction
 *
 * @param  message : pointer to a message structure, this should be a valid pointer
 * @param tx_buffer: pointer to a tx buffer that is already allocated
 * @param rx_buffer: pointer to an rx buffer that is already allocated
 * @param tx_buffer_length: number of bytes to transmit
 * @param rx_buffer_length: number of bytes to receive
 * @param  retries    : the number of times to attempt receive a message in case device doesnt respond
 * @param disable_dma: if true, disables the dma for current rx transaction. You may find it useful to switch off dma for short rx messages.
 *                     if you set this flag to 0, then you should make sure that the device flags was configured with I2C_DEVICE_USE_DMA. If the
 *                     device doesnt support DMA, the message will be received not using DMA.
 *
 * @return    WICED_SUCCESS : message structure was initialised properly.
 * @return    WICED_BADARG: one of the arguments is given incorrectly
 */
wiced_result_t wiced_i2c_init_combined_message(wiced_i2c_message_t* message, const void* tx_buffer, void* rx_buffer, uint16_t tx_buffer_length, uint16_t rx_buffer_length, uint16_t retries , wiced_bool_t disable_dma);


/** Transmits and/or receives data over an I2C interface
 *
 * @param  device             : the i2c device to communicate with
 * @param  message            : a pointer to a message (or an array of messages) to be transmitted/received
 * @param  number_of_messages : the number of messages to transfer. [1 .. N] messages
 *
 * @return    WICED_SUCCESS : on success.
 * @return    WICED_ERROR   : if an error occurred during message transfer
 */
wiced_result_t wiced_i2c_transfer( wiced_i2c_device_t* device, wiced_i2c_message_t* message, uint16_t number_of_messages );


/** Deinitialises an I2C device
 *
 * @param  device : the device for which the i2c port should be deinitialised
 *
 * @return    WICED_SUCCESS : on success.
 * @return    WICED_ERROR   : if an error occurred during deinitialisation
 */
wiced_result_t wiced_i2c_deinit( wiced_i2c_device_t* device );



/** @} */
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
/** @addtogroup pwm       PWM
 *  @ingroup platform
 *
 * Pulse-Width Modulation (PWM) Functions
 *
 *
 *  @{
 */
/*****************************************************************************/


/** Initialises a PWM pin
 *
 * Prepares a Pulse-Width Modulation pin for use.
 * Does not start the PWM output (use @ref wiced_pwm_start).
 *
 * @param pwm        : the PWM interface which should be initialised
 * @param frequency  : Output signal frequency in Hertz
 * @param duty_cycle : Duty cycle of signal as a floating-point percentage (0.0 to 100.0)
 *
 * @return    WICED_SUCCESS : on success.
 * @return    WICED_ERROR   : if an error occurred with any step
 */
wiced_result_t wiced_pwm_init(wiced_pwm_t pwm, uint32_t frequency, float duty_cycle);


/** Starts PWM output on a PWM interface
 *
 * Starts Pulse-Width Modulation signal output on a PWM pin
 *
 * @param pwm        : the PWM interface which should be started
 *
 * @return    WICED_SUCCESS : on success.
 * @return    WICED_ERROR   : if an error occurred with any step
 */
wiced_result_t wiced_pwm_start(wiced_pwm_t pwm);


/** Stops output on a PWM pin
 *
 * Stops Pulse-Width Modulation signal output on a PWM pin
 *
 * @param pwm        : the PWM interface which should be stopped
 *
 * @return    WICED_SUCCESS : on success.
 * @return    WICED_ERROR   : if an error occurred with any step
 */
wiced_result_t wiced_pwm_stop(wiced_pwm_t pwm);

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

