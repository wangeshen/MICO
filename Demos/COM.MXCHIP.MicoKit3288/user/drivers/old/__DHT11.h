/**
******************************************************************************
* @file    DHT11.h 
* @author  Eshen Wang
* @version V1.0.0
* @date    1-May-2015
* @brief   DHT11 operation. 
  operation
******************************************************************************
* @attention
*
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDATAG CUSTOMERS
* WITH CODATAG INFORMATION REGARDATAG THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
* TIME. AS A RESULT, MXCHIP Inc. SHALL NOT BE HELD LIABLE FOR ANY
* DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
* FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
* CODATAG INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*
* <h2><center>&copy; COPYRIGHT 2014 MXCHIP Inc.</center></h2>
******************************************************************************
*/ 

#ifndef __DHT11_H_
#define __DHT11_H_

#include "MICO.h"

//--------------------------------  pin defines --------------------------------
#define DHT11_DATA             MICO_GPIO_6  // PA11

#define DHT11_DATA_Clr()       MicoGpioOutputLow(DHT11_DATA) 
#define DHT11_DATA_Set()       MicoGpioOutputHigh(DHT11_DATA)

#define DHT11_DATA_IN          MicoGpioInputGet(DHT11_DATA)

#define DHT11_DATA_BITS        40         // 5 BYTES DATA
#define DHT11_DATA_RESPONSE_START_TIME  160        // 80+80us
#define DHT11_DATA_RESPONSE_BIT0_TIME   78         // 50+28us
#define DHT11_DATA_RESPONSE_BIT1_TIME  120         // 50+70us

#define SYSTEM_CORE_CLOCK      168000000

//------------------------------ user interfaces -------------------------------
OSStatus DHT11_init(void);
OSStatus DHT11_read(int8_t *temp, uint8_t *hum);

#endif  // __DHT11_H_
