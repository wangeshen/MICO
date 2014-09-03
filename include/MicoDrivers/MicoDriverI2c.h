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

#ifndef __MICODRIVERI2C_H__
#define __MICODRIVERI2C_H__

#pragma once
#include "Common.h"
#include "platform.h"

/******************************************************
 *                   Macros
 ******************************************************/  

#define MicoGpioInputGet wiced_gpio_input_get

/******************************************************
 *                   Enumerations
 ******************************************************/

typedef enum
{
    I2C_ADDRESS_WIDTH_7BIT,
    I2C_ADDRESS_WIDTH_10BIT,
    I2C_ADDRESS_WIDTH_16BIT,
} mico_i2c_bus_address_width_t;

typedef enum
{
    I2C_LOW_SPEED_MODE,         /* 10Khz devices */
    I2C_STANDARD_SPEED_MODE,    /* 100Khz devices */
    I2C_HIGH_SPEED_MODE         /* 400Khz devices */
} mico_i2c_speed_mode_t;

/******************************************************
 *                    Structures
 ******************************************************/

typedef struct
{
    wiced_i2c_t                   port;
    uint16_t                      address;       /* the address of the device on the i2c bus */
    mico_i2c_bus_address_width_t address_width;
    uint8_t                       flags;
    mico_i2c_speed_mode_t        speed_mode;    /* speed mode the device operates in */
} mico_i2c_device_t;

typedef struct
{
    const void*  tx_buffer;  /* A pointer to the data to be transmitted. If NULL, the message is an RX message when 'combined' is FALSE */
    void*        rx_buffer;  /* A pointer to the data to be transmitted. If NULL, the message is an TX message when 'combined' is FALSE */
    uint16_t     tx_length;
    uint16_t     rx_length;
    uint16_t     retries;    /* Number of times to retry the message */
    bool combined;
    uint8_t      flags;      /* MESSAGE_DISABLE_DMA : if set, this flag disables use of DMA for the message */
} mico_i2c_message_t;

/******************************************************
 *                 Type Definitions
 ******************************************************/

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
OSStatus wiced_i2c_init( mico_i2c_device_t* device );


/** Checks whether the device is available on a bus or not
 *
 *
 * @param  device : the i2c device to be probed
 * @param  retries    : the number of times to attempt to probe the device
 *
 * @return    WICED_TRUE : device is found.
 * @return    WICED_FALSE: device is not found
 */
OSStatus wiced_i2c_probe_device( mico_i2c_device_t* device, int retries );


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
OSStatus wiced_i2c_init_tx_message(mico_i2c_message_t* message, const void* tx_buffer, uint16_t  tx_buffer_length, uint16_t retries , bool disable_dma);

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
OSStatus wiced_i2c_init_rx_message(mico_i2c_message_t* message, void* rx_buffer, uint16_t rx_buffer_length, uint16_t retries , bool disable_dma);


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
OSStatus wiced_i2c_init_combined_message(mico_i2c_message_t* message, const void* tx_buffer, void* rx_buffer, uint16_t tx_buffer_length, uint16_t rx_buffer_length, uint16_t retries , bool disable_dma);


/** Transmits and/or receives data over an I2C interface
 *
 * @param  device             : the i2c device to communicate with
 * @param  message            : a pointer to a message (or an array of messages) to be transmitted/received
 * @param  number_of_messages : the number of messages to transfer. [1 .. N] messages
 *
 * @return    WICED_SUCCESS : on success.
 * @return    WICED_ERROR   : if an error occurred during message transfer
 */
OSStatus wiced_i2c_transfer( mico_i2c_device_t* device, mico_i2c_message_t* message, uint16_t number_of_messages );


/** Deinitialises an I2C device
 *
 * @param  device : the device for which the i2c port should be deinitialised
 *
 * @return    WICED_SUCCESS : on success.
 * @return    WICED_ERROR   : if an error occurred during deinitialisation
 */
OSStatus wiced_i2c_deinit( mico_i2c_device_t* device );



/** @} */

#endif


