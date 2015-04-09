/**
******************************************************************************
* @file    rgb_led.h 
* @author  Eshen Wang
* @version V1.0.0
* @date    17-Mar-2015
* @brief   converts HSB color values to RGB colors. 
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

#ifndef __RGB_LED_H_
#define __RGB_LED_H_

#define H2R_MAX_RGB_val 255.0

void H2R_HSBtoRGB(float hue, float sat, float bright, float *color);
void OpenLED_RGB(float *color);
void CloseLED_RGB();
//void OpenLED_W(float Size);
//void CloseLED_W();
//
//void OpenLED_C(float Size);
//void CloseLED_C();

#endif