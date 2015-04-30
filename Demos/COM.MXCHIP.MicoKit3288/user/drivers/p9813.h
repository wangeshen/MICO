/**
******************************************************************************
* @file    p9813.h 
* @author  Eshen Wang
* @version V1.0.0
* @date    17-Mar-2015
* @brief   converts rgb led controller. 
  operation
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

#ifndef __P9813_H_
#define __P9813_H_

#ifndef RGB_LED_USE_I2C

// pin defines
#define RGB_LED_CIN    MICO_GPIO_31    //CLK
#define RGB_LED_DIN    MICO_GPIO_18    //DIN

#define RGB_LED_CIN_Clr() MicoGpioOutputLow(RGB_LED_CIN)  
#define RGB_LED_CIN_Set() MicoGpioOutputHigh(RGB_LED_CIN)

#define RGB_LED_DIN_Clr() MicoGpioOutputLow(RGB_LED_DIN) 
#define RGB_LED_DIN_Set() MicoGpioOutputHigh(RGB_LED_DIN)

#endif

/* high level interfaces
 */
void rgb_led_init(void);
void rgb_led_open(uint16_t blue, uint16_t green, uint16_t red);
void rgb_led_close(void);

/* low level interfaces
 */


#endif
