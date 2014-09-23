/*----------------------------------------------------------------------------
 * Name:    Serial.c
 * Purpose: Low level serial routines for STM32
 * Version: V1.00
 *----------------------------------------------------------------------------
 * This file is part of the uVision/ARM development tools.
 * This software may only be used under the terms of a valid, current,
 * end user licence from KEIL for a compatible version of KEIL software
 * development tools. Nothing else gives you the right to use this software.
 *
 * Copyright (c) 2005-2007 Keil Software. All rights reserved.
 *----------------------------------------------------------------------------*/

#include "Common.h"
#include "Mico.h"
#include "platform_common_config.h"
#include "MicoPlatform.h"

/*----------------------------------------------------------------------------
  Read character from Serial Port   (blocking read)
 *----------------------------------------------------------------------------*/
uint8_t GetKey(void)  {

  uint8_t key;
  MicoUartRecv( STDIO_UART, &key, 1, MICO_NEVER_TIMEOUT );
  return key;
}

/*----------------------------------------------------------------------------
  Read character from Serial Port   (blocking read)-----used by ymodem.c
 *----------------------------------------------------------------------------*/

/**
  * @brief  Test to see if a key has been pressed on the HyperTerminal
  * @param  key: The key pressed
  * @retval 1: Correct
  *         0: Error
  */
uint32_t SerialKeyPressed(uint8_t *key)
{
  if ( MicoUartGetLengthInBuffer(STDIO_UART) )
  {
    MicoUartRecv( STDIO_UART, key, 1, MICO_NEVER_TIMEOUT );
    return 1;
  }
  else
  {
    return 0;
  }
}

/**
  * @brief  Print a string on the HyperTerminal
  * @param  s: The string to be printed
  * @retval None
  */
void Serial_PutString(uint8_t *s)
{
  MicoUartSend(STDIO_UART, s, strlen((char *)s));
}
