#ifndef _HAL_OLED_H
#define _HAL_OLED_H

//#include <stdio.h>
//#include <stm32f10x.h>
//#include <stdio.h>
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_spi.h"
#include "stm32f4xx_rcc.h"
#include "MICOPlatform.h"

#define OLED_CMD    0
#define OLED_DAT    1

extern const uint8_t OLED_Asc2_1206[95][12];
extern const uint8_t OLED_Asc2_1608[95][16];

/*************************************************************
*1 > VCC    -----  3.3V
*2 > GBD    -----  GND
*3 > NC     -----  NC
*4 > DIN    -----  PA10
*5 > SCLK   -----  PB0
*6 > CS     -----  PC13
*7 > D/C    -----  PA12
*8 > RES    -----  PA9
***************************************************************/
#define OLED_SPIx                		SPI5   
#define OLED_SPIx_CLK          			RCC_APB2Periph_SPI5
#define OLED_SPIx_CLK_Cmd		       	RCC_APB2PeriphClockCmd

#define OLED_SPIx_MOSI_PIN         GPIO_Pin_10
#define OLED_SPIx_MOSI_PORT        GPIOA
#define OLED_SPIx_MOSI_CLK         RCC_AHB1Periph_GPIOA

#define OLED_SPIx_MISO_PIN         GPIO_Pin_12
#define OLED_SPIx_MISO_PORT        GPIOA
#define OLED_SPIx_MISO_CLK         RCC_AHB1Periph_GPIOA

#define OLED_SPIx_SCK_PIN          GPIO_Pin_0
#define OLED_SPIx_SCK_PORT         GPIOB
#define OLED_SPIx_SCK_CLK          RCC_AHB1Periph_GPIOB


//#ifdef STUNO 
//#define OLED_CS_PORT              GPIOA
//#define OLED_CS_CLK          			RCC_APB2Periph_GPIOA
//#define OLED_CS_PIN               GPIO_Pin_15
//
//#define OLED_DC_PORT              GPIOB
//#define OLED_DC_CLK          			RCC_APB2Periph_GPIOB
//#define OLED_DC_PIN               GPIO_Pin_11
//
//#define OLED_RES_PORT             GPIOA
//#define OLED_RES_CLK          		RCC_APB2Periph_GPIOA
//#define OLED_RES_PIN              GPIO_Pin_9
//#endif
//
//#ifdef Gokit 
//#define OLED_CS_PORT              GPIOA
//#define OLED_CS_CLK          			RCC_APB2Periph_GPIOA
//#define OLED_CS_PIN               GPIO_Pin_15
//
//#define OLED_DC_PORT              GPIOB
//#define OLED_DC_CLK          			RCC_APB2Periph_GPIOB
//#define OLED_DC_PIN               GPIO_Pin_11
//
//#define OLED_RES_PORT             GPIOB
//#define OLED_RES_CLK          		RCC_APB2Periph_GPIOB
//#define OLED_RES_PIN              GPIO_Pin_6
//#endif
//
//#ifdef Xnucleo
//#define OLED_CS_PORT              GPIOB
//#define OLED_CS_CLK          			RCC_APB2Periph_GPIOB
//#define OLED_CS_PIN               GPIO_Pin_6
//
//#define OLED_DC_PORT              GPIOC
//#define OLED_DC_CLK          			RCC_APB2Periph_GPIOC
//#define OLED_DC_PIN               GPIO_Pin_7
//
//#define OLED_RES_PORT             GPIOA
//#define OLED_RES_CLK          		RCC_APB2Periph_GPIOA
//#define OLED_RES_PIN              GPIO_Pin_9
//#endif

#define OLED_CS_PORT              GPIOC
#define OLED_CS_CLK          	  RCC_AHB1Periph_GPIOC
#define OLED_CS_PIN               GPIO_Pin_13

#define OLED_DC_PORT              GPIOA
#define OLED_DC_CLK          	  RCC_AHB1Periph_GPIOA
#define OLED_DC_PIN               GPIO_Pin_12

#define OLED_RES_PORT             OLED_DC_PORT   // RESET connect system RESET pin, set DC pin no use.
#define OLED_RES_CLK          	  OLED_DC_CLK
#define OLED_RES_PIN              OLED_DC_PIN


#define OLED_CS_SET()     	  GPIO_WriteBit(OLED_CS_PORT, OLED_CS_PIN, Bit_SET)    
#define OLED_CS_RESET()		  GPIO_WriteBit(OLED_CS_PORT, OLED_CS_PIN, Bit_RESET)		

#define OLED_DC_SET()     	  GPIO_WriteBit(OLED_DC_PORT, OLED_DC_PIN, Bit_SET)    
#define OLED_DC_RESET()		  GPIO_WriteBit(OLED_DC_PORT, OLED_DC_PIN, Bit_RESET)		

#define OLED_RES_SET()
#define OLED_RES_RESET()


void OLED_Init(void);
void OLED_DisplayOn(void);
void OLED_DisplayOff(void);
void OLED_Refresh_Gram(void);
void LCD_Clear(uint8_t Color);
void OLED_DrawPoint(uint8_t Xpos,uint8_t Ypos,uint8_t Fill);
void OLED_ShowChar(uint8_t X, uint8_t Y, uint8_t Chr, uint8_t Size, uint8_t Mode);
void OLED_ShowString(uint8_t X, uint8_t Y, const uint8_t *Str);

#endif /*_HAL_OLED_H*/


