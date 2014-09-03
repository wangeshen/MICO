/**
  ******************************************************************************
  * @file    MICORTOS.h 
  * @author  William Xu
  * @version V1.0.0
  * @date    05-May-2014
  * @brief   This file provides all the headers of UART operation functions.
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

#ifndef __MICODRIVERSPI_H__
#define __MICODRIVERSPI_H__

#pragma once
#include "Common.h"
#include "platform.h"

/******************************************************
 *                   Enumerations
 ******************************************************/


/******************************************************
 *                    Structures
 ******************************************************/


typedef struct
{
    mico_spi_t  port;
    mico_gpio_t chip_select;
    uint32_t     speed;
    uint8_t      mode;
    uint8_t      bits;
} mico_spi_device_t;

typedef struct
{
    const void* tx_buffer;
    void*       rx_buffer;
    uint32_t    length;
} mico_spi_message_segment_t;

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
OSStatus wiced_spi_init( const mico_spi_device_t* spi );


/** Transmits and/or receives data from a SPI device
 *
 * @param  spi      : the SPI device to be initialised
 * @param  segments : a pointer to an array of segments
 * @param  number_of_segments : the number of segments to transfer
 *
 * @return    WICED_SUCCESS : on success.
 * @return    WICED_ERROR   : if an error occurred
 */
OSStatus wiced_spi_transfer( const mico_spi_device_t* spi, mico_spi_message_segment_t* segments, uint16_t number_of_segments );


/** De-initialises a SPI interface
 *
 * Turns off a SPI hardware interface
 *
 * @param  spi : the SPI device to be de-initialised
 *
 * @return    WICED_SUCCESS : on success.
 * @return    WICED_ERROR   : if an error occurred
 */
OSStatus wiced_spi_deinit( const mico_spi_device_t* spi );


/** @} */

#endif