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

#ifndef __MICODRIVERUART_H__
#define __MICODRIVERUART_H__

#pragma once
#include "Common.h"
#include "RingBufferUtils.h"
#include "platform.h"

/******************************************************
 *                   Enumerations
 ******************************************************/

typedef enum
{
    DATA_WIDTH_5BIT,
    DATA_WIDTH_6BIT,
    DATA_WIDTH_7BIT,
    DATA_WIDTH_8BIT,
    DATA_WIDTH_9BIT
} wiced_uart_data_width_t;

typedef enum
{
    STOP_BITS_1,
    STOP_BITS_2,
} wiced_uart_stop_bits_t;

typedef enum
{
    FLOW_CONTROL_DISABLED,
    FLOW_CONTROL_CTS,
    FLOW_CONTROL_RTS,
    FLOW_CONTROL_CTS_RTS
} wiced_uart_flow_control_t;

typedef enum
{
    NO_PARITY,
    ODD_PARITY,
    EVEN_PARITY,
} wiced_uart_parity_t;

/******************************************************
 *                    Structures
 ******************************************************/

 typedef struct
{
    uint32_t                  baud_rate;
    wiced_uart_data_width_t   data_width;
    wiced_uart_parity_t       parity;
    wiced_uart_stop_bits_t    stop_bits;
    wiced_uart_flow_control_t flow_control;
} mico_uart_config_t;


/*****************************************************************************/
/** @addtogroup uart       UART
 *  @ingroup platform
 *
 * Universal Asynchronous Receiver Transmitter (UART) Functions
 *
 *
 *  @{
 */
/*****************************************************************************/

/** Initialises a UART interface
 *
 * Prepares an UART hardware interface for communications
 *
 * @param  uart     : the interface which should be initialised
 * @param  config   : UART configuration structure
 * @param  optional_rx_buffer : Pointer to an optional RX ring buffer
 *
 * @return    WICED_SUCCESS : on success.
 * @return    WICED_ERROR   : if an error occurred with any step
 */
OSStatus MicoUartInitialize( mico_uart_t uart, const mico_uart_config_t* config, ring_buffer_t* optional_rx_buffer );


/** Initialises a STDIO UART interface, internal use only
 *
 * Prepares an UART hardware interface for stdio communications
 *
 * @param  config   : UART configuration structure
 * @param  optional_rx_buffer : Pointer to an optional RX ring buffer
 *
 * @return    WICED_SUCCESS : on success.
 * @return    WICED_ERROR   : if an error occurred with any step
 */
OSStatus MicoStdioUartInitialize( const mico_uart_config_t* config, ring_buffer_t* optional_rx_buffer );


/** Deinitialises a UART interface
 *
 * @param  uart : the interface which should be deinitialised
 *
 * @return    WICED_SUCCESS : on success.
 * @return    WICED_ERROR   : if an error occurred with any step
 */
OSStatus MicoUartFinalize( mico_uart_t uart );


/** Transmit data on a UART interface
 *
 * @param  uart     : the UART interface
 * @param  data     : pointer to the start of data
 * @param  size     : number of bytes to transmit
 *
 * @return    WICED_SUCCESS : on success.
 * @return    WICED_ERROR   : if an error occurred with any step
 */
OSStatus MicoUartSend( mico_uart_t uart, const void* data, uint32_t size );


/** Receive data on a UART interface
 *
 * @param  uart     : the UART interface
 * @param  data     : pointer to the buffer which will store incoming data
 * @param  size     : number of bytes to receive
 * @param  timeout  : timeout in milisecond
 *
 * @return    WICED_SUCCESS : on success.
 * @return    WICED_ERROR   : if an error occurred with any step
 */
OSStatus MicoUartRecv( mico_uart_t uart, void* data, uint32_t size, uint32_t timeout );

/** Read the length of the data that is already recived by uart driver and stored in buffer
 *
 * @param  uart     : the UART interface
 *
 * @return    Data length
 */
uint32_t MicoUartGetLengthInBuffer( mico_uart_t uart ); 

/** @} */

#endif