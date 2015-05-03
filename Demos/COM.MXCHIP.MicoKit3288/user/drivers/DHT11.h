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


#include "MICOPlatform.h"

//--------------------------------  pin defines --------------------------------
#define DHT11_DATA             MICO_GPIO_7  // PB4

#define DHT11_DATA_Clr()       MicoGpioOutputLow(DHT11_DATA) 
#define DHT11_DATA_Set()       MicoGpioOutputHigh(DHT11_DATA)

#define DHT11_In               MicoGpioInputGet(DHT11_DATA)

//------------------------------ user interfaces -------------------------------
int DHT11_init(void);
int DHT11_read(uint8_t *data);   // data format data[5]


#endif  // __DHT11_H_
