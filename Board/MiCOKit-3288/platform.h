/**
******************************************************************************
* @file    platform.h
* @author  William Xu
* @version V1.0.0
* @date    05-May-2014
* @brief   This file provides all MICO Peripherals defined for current platform.
******************************************************************************
*
*  The MIT License
*  Copyright (c) 2014 MXCHIP Inc.
*
*  Permission is hereby granted, free of charge, to any person obtaining a copy 
*  of this software and associated documentation files (the "Software"), to deal
*  in the Software without restriction, including without limitation the rights 
*  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*  copies of the Software, and to permit persons to whom the Software is furnished
*  to do so, subject to the following conditions:
*
*  The above copyright notice and this permission notice shall be included in
*  all copies or substantial portions of the Software.
*
*  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
*  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
*  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
*  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
*  WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR 
*  IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
******************************************************************************
*/ 

#include "platform_common_config.h"

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/
  
#define HARDWARE_REVISION   "3288"
#define DEFAULT_NAME        "EMW3288 Module"
#define MODEL               "EMW3288"
#define Bootloader_VISION   "V 0.1"
   
/******************************************************
 *                   Enumerations
 ******************************************************/

/*
EMW3288 on EMB-3288V1.0 platform pin definitions ...
+-------------------------------------------------------------------------------+
| Enum ID       |Pin | STM32      | Peripheral  |    Board     |   Peripheral   |
|               | #  | Port       | Available   |  Connection  |     Alias      |
|---------------+----+------------+-------------+--------------+----------------|
| MICO_GPIO_1   | 1  | -          |             |              | GND            |
|---------------+----+------------+-------------+--------------+----------------|
| MICO_GPIO_2   | 2  | A 13       |             |              | SWDIO          |
|---------------+----+------------+-------------+--------------+----------------|
| MICO_GPIO_3   | 3  | A 14       |             |              | SWDCLK         |
|---------------+----+------------+-------------+--------------+----------------|
| MICO_GPIO_4   | 4  | A 15       |             |              | UART1_TX_DEBUG |
|---------------+----+------------+-------------+--------------+----------------|
| MICO_GPIO_5   | 5  | B  3       |             |              | UART1_RX_DEBUG |      
|---------------+----+------------+-------------+--------------+----------------|
| MICO_GPIO_6   | 6  | A 11       |             |              | SPI4_MISO      |
|---------------+----+------------+-------------+--------------+----------------|
| MICO_GPIO_7   | 7  | B  4       |             |              | GPIO           |
|---------------+----+------------+-------------+--------------+----------------|
| MICO_GPIO_8   | 8  | BOOT0      |             |              | MCU_BOOT       |
|---------------+----+------------+-------------+--------------+----------------|
| MICO_GPIO_9   | 9  | A  4       |             |              | GPIO           |
|---------------+----+------------+-------------+--------------+----------------|
| MICO_GPIO_10  | 10 | B  8       |             |              | GPIO           |
|---------------+----+------------+-------------+--------------+----------------|
| MICO_GPIO_11  | 11 | B  9       |             |              | GPIO           |
|---------------+----+------------+-------------+--------------+----------------|
| MICO_GPIO_12  | 12 | VDD        |             |              | VDD_MCU        |
|---------------+----+------------+-------------+--------------+----------------|
| MICO_GPIO_13  | 13 | VBAT       |             |              | VBAT           |
|---------------+----+------------+-------------+--------------+----------------|
| MICO_GPIO_14  | 14 | C 13       |             |              | GPIO           |
|---------------+----+------------+-------------+--------------+----------------|
| MICO_GPIO_15  | 15 | OSC32IN    |             |              | OSC32IN        |
|---------------+----+------------+-------------+--------------+----------------|
| MICO_GPIO_16  | 16 | OSC32OUT   |             |              | OSC32OUT       |
|---------------+----+------------+-------------+--------------+----------------|
| MICO_GPIO_17  | 17 | PHO-OSC_IN |             |              | OSCIN          |
|---------------+----+------------+-------------+--------------+----------------|
| MICO_GPIO_18  | 18 | PH1OSC_OUT |             |              | OSCOUT         |
|---------------+----+------------+-------------+--------------+----------------|
| MICO_GPIO_19  | 19 | NRST       |             |              | nRESET         |
|---------------+----+------------+-------------+--------------+----------------|
| MICO_GPIO_20  | 20 | -          |             |              | GND            |
|---------------+----+------------+-------------+--------------+----------------|
| MICO_GPIO_21  | 21 | A  2       |             |              | USART2_TX      |
|---------------+----+------------+-------------+--------------+----------------|
| MICO_GPIO_22  | 22 | A  3       |             |              | USART2_RX      |
|---------------+----+------------+-------------+--------------+----------------|
| MICO_GPIO_23  | 23 | A  0       |             |              | USART2_CTS     |
|---------------+----+------------+-------------+--------------+----------------|
| MICO_GPIO_24  | 24 | A  1       |             |              | USART2_RTS     |
|               |    |            |             |              | SPI4_MOSI      |
|---------------+----+------------+-------------+--------------+----------------|
| MICO_GPIO_25  | 25 | B  0       |             |              | FLASH_SPI_CLK  | 
|---------------+----+------------+-------------+--------------+----------------|
| MICO_GPIO_26  | 26 | B  1       |             |              | FLASH_SPI_CS   |    
|---------------+----+------------+-------------+--------------+----------------|
| MICO_GPIO_27  | 27 | B 12       |             |              | FLASH_SPI_MISO |  
|---------------+----+------------+-------------+--------------+----------------|
| MICO_GPIO_28  | 28 | A 10       |             |              | FLASH_SPI_MOSI |
|---------------+----+------------+-------------+--------------+----------------|
| MICO_GPIO_29  | 29 | A  5       |             |              | STATUS         |
|---------------+----+------------+-------------+--------------+----------------|
| MICO_GPIO_30  | 30 | B  2       |             |              | Module_BOOT    |
|---------------+----+------------+-------------+--------------+----------------|
| MICO_GPIO_31  | 31 | A  7       |             |              | GPIO           | 
|---------------+----+------------+-------------+--------------+----------------|
| MICO_GPIO_32  | 32 | B 12       |             |              | SPI4_NSS       | 
|---------------+----+------------+-------------+--------------+----------------|  
| MICO_GPIO_33  | 33 | B 13       |             |              | SPI4_SCK       |   
+-------------------------------------------------------------------------------+  
 
Notes
1. These mappings are defined in <MICO-SDK>/Platform/EMW3288/platform.c
2. STM32F2xx Datasheet  -> http://www.st.com/web/en/resource/technical/document/datasheet/CD00237391.pdf
3. STM32F2xx Ref Manual -> http://www.st.com/web/en/resource/technical/document/reference_manual/CD00225773.pdf
*/


typedef enum
{
    MICO_GPIO_0 = MICO_COMMON_GPIO_MAX,
    MICO_GPIO_1,
    MICO_GPIO_2,
    MICO_GPIO_3,
    MICO_GPIO_4,
    MICO_GPIO_5,
    MICO_GPIO_6,
    MICO_GPIO_7,
    MICO_GPIO_8,
    MICO_GPIO_9,
    MICO_GPIO_10,
    MICO_GPIO_11,
    MICO_GPIO_12,
    MICO_GPIO_13,
    MICO_GPIO_14,
    MICO_GPIO_15,
    MICO_GPIO_16,
    MICO_GPIO_17,
    MICO_GPIO_18,
    MICO_GPIO_19,
    MICO_GPIO_20,
    MICO_GPIO_21,
    MICO_GPIO_22,
    MICO_GPIO_23,
    MICO_GPIO_24,
    MICO_GPIO_25,
    MICO_GPIO_26,
    MICO_GPIO_27,
    MICO_GPIO_28,
    MICO_GPIO_29,
    MICO_GPIO_30,
    MICO_GPIO_31,
    MICO_GPIO_32,
    MICO_GPIO_33,

    MICO_GPIO_MAX, /* Denotes the total number of GPIO port aliases. Not a valid GPIO alias */
} mico_gpio_t;

typedef enum
{
    MICO_SPI_1,
    
    MICO_SPI_MAX, /* Denotes the total number of SPI port aliases. Not a valid SPI alias */
} mico_spi_t;

typedef enum
{
    MICO_I2C_UNUSED = -1,
    MICO_I2C_1,
    
    MICO_I2C_MAX, /* Denotes the total number of I2C port aliases. Not a valid I2C alias */
} mico_i2c_t;

typedef enum
{
  MICO_PWM_1,
  
  MICO_PWM_MAX, /* Denotes the total number of PWM port aliases. Not a valid PWM alias */
} mico_pwm_t;

typedef enum
{
    MICO_ADC_1,
    MICO_ADC_2,
    MICO_ADC_3,
    
    MICO_ADC_MAX, /* Denotes the total number of ADC port aliases. Not a valid ADC alias */
} mico_adc_t;

typedef enum
{
    MICO_UART_UNUSED = -1,
    MICO_UART_1,
    MICO_UART_2,
    
    MICO_UART_MAX, /* Denotes the total number of UART port aliases. Not a valid UART alias */
} mico_uart_t;

typedef enum
{
  MICO_SPI_FLASH,
  MICO_INTERNAL_FLASH,
  
  MICO_FLASH_MAX,
} mico_flash_t;

#define USE_MICO_SPI_FLASH
//#define SFLASH_SUPPORT_MACRONIX_PART 
//#define SFLASH_SUPPORT_SST_PARTS
#define SFLASH_SUPPORT_WINBOND_PARTS

#define STM32_UART_1 NULL  /*Not used here, define to avoid warning*/
#define STM32_UART_2 MICO_UART_1
#define STM32_UART_6 NULL

/* Components connected to external I/Os*/


/* I/O connection <-> Peripheral Connections */
#define MICO_I2C_CP         (MICO_I2C_1)


#define RestoreDefault_TimeOut          3000  /**< Restore default and start easylink after 
                                                   press down EasyLink button for 3 seconds. */

#ifdef __cplusplus
} /*extern "C" */
#endif

