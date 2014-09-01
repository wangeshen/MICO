/*
 * Copyright 2014, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */

/** @file
 *
 */
#include "EMW3162/platform.h"
#include "MICOPlatform.h"
#include "debug.h"
#include "gpio_irq.h"
#include "MICODefine.h"
//#include "watchdog.h"
#include "stdio.h"
#include "string.h"
//#include "wwd_assert.h"
#include "stm32f2xx_platform.h"
#include "platform_common_config.h"
#include "platform_internal_gpio.h"
//#include "Platform/wwd_platform_interface.h"

/******************************************************
 *                      Macros
 ******************************************************/

#define EMW3162_Platform_log(M, ...) custom_log("Platform", M, ##__VA_ARGS__)
#define EMW3162_Platform_log_trace() custom_log_trace("Platform")

/******************************************************
 *                    Constants
 ******************************************************/

/******************************************************
 *                   Enumerations
 ******************************************************/

/******************************************************
 *                 Type Definitions
 ******************************************************/

/******************************************************
 *                    Structures
 ******************************************************/

/******************************************************
 *               Function Declarations
 ******************************************************/

/******************************************************
 *               Variables Definitions
 ******************************************************/

/* This table maps STM32 pins to GPIO definitions on the schematic
 * A full pin definition is provided in <WICED-SDK>/include/platforms/BCM943362WCD4/platform.h
 */

static uint32_t _default_start_time = 0;
static mico_timer_t _button_EL_timer;

const platform_pin_mapping_t gpio_mapping[] =
{
    [MICO_GPIO_1]  = {GPIOB,  6,  RCC_AHB1Periph_GPIOB},
    [MICO_GPIO_2]  = {GPIOB,  7,  RCC_AHB1Periph_GPIOB},
    [MICO_GPIO_4]  = {GPIOC,  7,  RCC_AHB1Periph_GPIOC},
    [MICO_GPIO_5]  = {GPIOA,  4,  RCC_AHB1Periph_GPIOA},
    [MICO_GPIO_6]  = {GPIOA,  4,  RCC_AHB1Periph_GPIOA},
    [MICO_GPIO_7]  = {GPIOB,  3,  RCC_AHB1Periph_GPIOB},
    [MICO_GPIO_8]  = {GPIOB , 4,  RCC_AHB1Periph_GPIOB},
    [MICO_GPIO_9]  = {GPIOB,  5,  RCC_AHB1Periph_GPIOB},
    [MICO_GPIO_10] = {GPIOB,  8,  RCC_AHB1Periph_GPIOB},
    [MICO_GPIO_11] = {GPIOA,  1,  RCC_AHB1Periph_GPIOA},
    [MICO_GPIO_12] = {GPIOC,  2,  RCC_AHB1Periph_GPIOC},
    [MICO_GPIO_13] = {GPIOB, 14,  RCC_AHB1Periph_GPIOB},
    [MICO_GPIO_14] = {GPIOC,  6,  RCC_AHB1Periph_GPIOC},
    [MICO_GPIO_16] = {GPIOB,  1,  RCC_AHB1Periph_GPIOB},
    [MICO_GPIO_18] = {GPIOA, 15,  RCC_AHB1Periph_GPIOA},
    [MICO_GPIO_19] = {GPIOB, 11,  RCC_AHB1Periph_GPIOB},
    [MICO_GPIO_20] = {GPIOA, 12,  RCC_AHB1Periph_GPIOA},
    [MICO_GPIO_21] = {GPIOA, 11,  RCC_AHB1Periph_GPIOA},
    [MICO_GPIO_22] = {GPIOA,  9,  RCC_AHB1Periph_GPIOA},
    [MICO_GPIO_23] = {GPIOA, 10,  RCC_AHB1Periph_GPIOA},
    [MICO_GPIO_29] = {GPIOA,  0,  RCC_AHB1Periph_GPIOA},
    [MICO_GPIO_30] = {GPIOB,  9,  RCC_AHB1Periph_GPIOB},
    [MICO_SYS_LED] = {GPIOB,  0,  RCC_AHB1Periph_GPIOB},


    /* Extended GPIOs for internal use */
    [WICED_GPIO_WLAN_POWERSAVE_CLOCK] = {WL_32K_OUT_BANK, WL_32K_OUT_PIN, WL_32K_OUT_BANK_CLK},
};

/*
 * Possible compile time inputs:
 * - Set which ADC peripheral to use for each ADC. All on one ADC allows sequential conversion on all inputs. All on separate ADCs allows concurrent conversion.
 */
/* TODO : These need fixing */
const platform_adc_mapping_t adc_mapping[] =
{
    [MICO_ADC_1] = {ADC1, ADC_Channel_1, RCC_APB2Periph_ADC1, 1, (platform_pin_mapping_t*)&gpio_mapping[MICO_GPIO_2]},
    [MICO_ADC_2] = {ADC1, ADC_Channel_2, RCC_APB2Periph_ADC1, 1, (platform_pin_mapping_t*)&gpio_mapping[MICO_GPIO_4]},
    [MICO_ADC_3] = {ADC1, ADC_Channel_3, RCC_APB2Periph_ADC1, 1, (platform_pin_mapping_t*)&gpio_mapping[MICO_GPIO_5]},
};


/* PWM mappings */
const platform_pwm_mapping_t pwm_mappings[] =
{
    [MICO_PWM_1]  = {TIM4, 1, RCC_APB1Periph_TIM4, GPIO_AF_TIM4, (platform_pin_mapping_t*)&gpio_mapping[MICO_GPIO_11]},
    [MICO_PWM_2]  = {TIM4, 2, RCC_APB1Periph_TIM4, GPIO_AF_TIM4, (platform_pin_mapping_t*)&gpio_mapping[MICO_GPIO_12]},
    [MICO_PWM_3]  = {TIM2, 2, RCC_APB1Periph_TIM2, GPIO_AF_TIM2, (platform_pin_mapping_t*)&gpio_mapping[MICO_GPIO_2]},  /* or TIM5/Ch2                       */
    [MICO_PWM_4]  = {TIM2, 3, RCC_APB1Periph_TIM2, GPIO_AF_TIM2, (platform_pin_mapping_t*)&gpio_mapping[MICO_GPIO_21]},  /* or TIM5/Ch3, TIM9/Ch1             */
    [MICO_PWM_5]  = {TIM2, 4, RCC_APB1Periph_TIM2, GPIO_AF_TIM2, (platform_pin_mapping_t*)&gpio_mapping[MICO_GPIO_4]},  /* or TIM5/Ch4, TIM9/Ch2             */
    [MICO_PWM_6]  = {TIM2, 1, RCC_APB1Periph_TIM2, GPIO_AF_TIM2, (platform_pin_mapping_t*)&gpio_mapping[MICO_GPIO_6]},  /* or TIM2_CH1_ETR, TIM8/Ch1N        */
    [MICO_PWM_7]  = {TIM3, 1, RCC_APB1Periph_TIM3, GPIO_AF_TIM3, (platform_pin_mapping_t*)&gpio_mapping[MICO_GPIO_7]},  /* or TIM1_BKIN, TIM8_BKIN, TIM13/Ch1*/
    [MICO_PWM_8]  = {TIM3, 2, RCC_APB1Periph_TIM3, GPIO_AF_TIM3, (platform_pin_mapping_t*)&gpio_mapping[MICO_GPIO_8]},  /* or TIM8/Ch1N, TIM14/Ch1           */
    [MICO_PWM_9]  = {TIM5, 2, RCC_APB1Periph_TIM5, GPIO_AF_TIM5, (platform_pin_mapping_t*)&gpio_mapping[MICO_GPIO_20]},  /* or TIM2/Ch2                       */

    /* Extended PWM for internal use */
    [WICED_PWM_WLAN_POWERSAVE_CLOCK] = {TIM1, 4, RCC_APB2Periph_TIM1, GPIO_AF_TIM1, (platform_pin_mapping_t*)&gpio_mapping[WICED_GPIO_WLAN_POWERSAVE_CLOCK] }, /* or TIM2/Ch2                       */

    /* TODO: fill in the other options here ... */
};

const platform_spi_mapping_t spi_mapping[] =
{
    [MICO_SPI_1]  =
    {
        .spi_regs              = SPI1,
        .gpio_af               = GPIO_AF_SPI1,
        .peripheral_clock_reg  = RCC_APB2Periph_SPI1,
        .peripheral_clock_func = RCC_APB2PeriphClockCmd,
        .pin_mosi              = &gpio_mapping[MICO_GPIO_8],
        .pin_miso              = &gpio_mapping[MICO_GPIO_7],
        .pin_clock             = &gpio_mapping[MICO_GPIO_6],
        .tx_dma_stream         = DMA2_Stream5,
        .rx_dma_stream         = DMA2_Stream0,
        .tx_dma_channel        = DMA_Channel_3,
        .rx_dma_channel        = DMA_Channel_3,
        .tx_dma_stream_number  = 5,
        .rx_dma_stream_number  = 0
    }
};

const platform_uart_mapping_t uart_mapping[] =
{
    [MICO_UART_1] =
    {
        .usart                        = USART1,
        .gpio_af                      = GPIO_AF_USART1,
        .pin_tx                       = &gpio_mapping[MICO_GPIO_22],
        .pin_rx                       = &gpio_mapping[MICO_GPIO_23],
        .pin_cts                      = NULL,
        .pin_rts                      = NULL,
        .usart_peripheral_clock       = RCC_APB2Periph_USART1,
        .usart_peripheral_clock_func  = RCC_APB2PeriphClockCmd,
        .usart_irq                    = USART1_IRQn,
        .tx_dma                       = DMA2,
        .tx_dma_stream                = DMA2_Stream7,
        .tx_dma_stream_number         = 7,
        .tx_dma_channel               = DMA_Channel_4,
        .tx_dma_peripheral_clock      = RCC_AHB1Periph_DMA2,
        .tx_dma_peripheral_clock_func = RCC_AHB1PeriphClockCmd,
        .tx_dma_irq                   = DMA2_Stream7_IRQn,
        .rx_dma                       = DMA2,
        .rx_dma_stream                = DMA2_Stream2,
        .rx_dma_stream_number         = 2,
        .rx_dma_channel               = DMA_Channel_4,
        .rx_dma_peripheral_clock      = RCC_AHB1Periph_DMA2,
        .rx_dma_peripheral_clock_func = RCC_AHB1PeriphClockCmd,
        .rx_dma_irq                   = DMA2_Stream2_IRQn
    },
    [MICO_UART_2] =
    {
        .usart                        = USART2,
        .gpio_af                      = GPIO_AF_USART2,
        .pin_tx                       = &gpio_mapping[MICO_GPIO_5],
        .pin_rx                       = &gpio_mapping[MICO_GPIO_4],
        .pin_cts                      = &gpio_mapping[MICO_GPIO_1],
        .pin_rts                      = &gpio_mapping[MICO_GPIO_2],
        .usart_peripheral_clock       = RCC_APB1Periph_USART2,
        .usart_peripheral_clock_func  = RCC_APB1PeriphClockCmd,
        .usart_irq                    = USART2_IRQn,
        .tx_dma                       = DMA1,
        .tx_dma_stream                = DMA1_Stream6,
        .tx_dma_channel               = DMA_Channel_4,
        .tx_dma_peripheral_clock      = RCC_AHB1Periph_DMA1,
        .tx_dma_peripheral_clock_func = RCC_AHB1PeriphClockCmd,
        .tx_dma_irq                   = DMA1_Stream6_IRQn,
        .rx_dma                       = DMA1,
        .rx_dma_stream                = DMA1_Stream5,
        .rx_dma_channel               = DMA_Channel_4,
        .rx_dma_peripheral_clock      = RCC_AHB1Periph_DMA1,
        .rx_dma_peripheral_clock_func = RCC_AHB1PeriphClockCmd,
        .rx_dma_irq                   = DMA1_Stream5_IRQn
    },
};

/******************************************************
 *               Function Definitions
 ******************************************************/

static void _button_EL_irq_handler( void* arg )
{
  (void)(arg);
  int interval = -1;

  if ( MicoGpioInputGet( EasyLink_BUTTON1 ) == 0 ) {
    _default_start_time = mico_get_time()+1;
    mico_start_timer(&_button_EL_timer);
  } else {
    interval = mico_get_time() + 1 - _default_start_time;
    if ( (_default_start_time != 0) && interval > 50 && interval < RestoreDefault_TimeOut){
      /* EasyLink button clicked once */
      PlatformEasyLinkButtonClickedCallback();
    }
    mico_stop_timer(&_button_EL_timer);
    _default_start_time = 0;
  }
}

static void _button_EL_Timeout_handler( void* arg )
{
  (void)(arg);
  _default_start_time = 0;
  PlatformEasyLinkButtonLongPressedCallback();
}

OSStatus wiced_platform_init( void )
{
    EMW3162_Platform_log( "Platform initialised" );

    if ( true == watchdog_check_last_reset() )
    {
        EMW3162_Platform_log( "WARNING: Watchdog reset occured previously. Please see watchdog.c for debugging instructions." );
    }

    return kNoErr;
}

void init_platform( void )
{
    // /* Initialise LEDs and turn off by default */
    MicoGpioInitialize( MICO_SYS_LED, OUTPUT_PUSH_PULL );
    // wiced_gpio_init( WICED_LED2, OUTPUT_PUSH_PULL );
    MicoGpioOutputLow( MICO_SYS_LED );
    // wiced_gpio_output_low( WICED_LED2 );

    //  Initialise buttons to input by default 
     MicoGpioInitialize( EasyLink_BUTTON1, INPUT_PULL_UP );
     mico_init_timer(&_button_EL_timer, RestoreDefault_TimeOut, _button_EL_Timeout_handler, NULL);
     MicoGpioEnableIRQ( EasyLink_BUTTON1, IRQ_TRIGGER_BOTH_EDGES, _button_EL_irq_handler, 0 );

    // wiced_gpio_init( WICED_BUTTON2, INPUT_PULL_UP );
}

void host_platform_reset_wifi( wiced_bool_t reset_asserted )
{
    if ( reset_asserted == WICED_TRUE )
    {
        GPIO_ResetBits( WL_RESET_BANK, WL_RESET_PIN );
    }
    else
    {
        GPIO_SetBits( WL_RESET_BANK, WL_RESET_PIN );
    }
}

void host_platform_power_wifi( wiced_bool_t power_enabled )
{
    if ( power_enabled == WICED_TRUE )
    {
        GPIO_ResetBits( WL_REG_ON_BANK, WL_REG_ON_PIN );
    }
    else
    {
        GPIO_SetBits( WL_REG_ON_BANK, WL_REG_ON_PIN );
    }
}
