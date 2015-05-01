/**
******************************************************************************
* @file    DHT11.c
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

#include "DHT11.h"
#include "MICO.h"

#define dht11_log(M, ...) custom_log("DHT11", M, ##__VA_ARGS__)
#define dht11_log_trace() custom_log_trace("DHT11")

 
/*------------------------------ USER INTERFACES -----------------------------*/

int DHT11_init(void)
{
  MicoGpioInitialize( (mico_gpio_t)DHT11_DATA, OUTPUT_PUSH_PULL );
  return 0;
}

int DHT11_read(uint8_t *data)
{
  // test
  data[0] = 50;
  data[1] = 0;
  data[2] = 25;
  data[3] = 0;
  data[4] = 0;  // not check
  
  return 0;
}

