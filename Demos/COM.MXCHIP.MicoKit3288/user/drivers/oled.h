//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//�о�԰����
//���̵�ַ��http://shop73023976.taobao.com/?spm=2013.1.0.0.M4PqC2
//
//  �� �� ��   : main.c
//  �� �� ��   : v2.0
//  ��    ��   : HuangKai
//  ��������   : 2014-0101
//  ����޸�   : 
//  ��������   : OLED 4�ӿ���ʾ����(51ϵ��)
//              ˵��: 
//              ----------------------------------------------------------------
//              GND    ��Դ��
//              VCC  ��5V��3.3v��Դ
//              D0   ��PD6��SCL��
//              D1   ��PD7��SDA��
//              RES  ��PD4
//              DC   ��PD5
//              CS   ��PD3               
//              ----------------------------------------------------------------
// �޸���ʷ   :
// ��    ��   : 
// ��    ��   : HuangKai
// �޸�����   : �����ļ�
//��Ȩ���У�����ؾ���
//Copyright(C) �о�԰����2014/3/16
//All rights reserved
//******************************************************************************/
#ifndef __OLED_H
#define __OLED_H			  	 
//#include "sys.h"
#include "MICOPlatform.h"

#include "stdlib.h"	    	
//OLEDģʽ����
//0:4�ߴ���ģʽ
//1:����8080ģʽ
#define OLED_MODE 0
#define SIZE 16
#define XLevelL		0x00
#define XLevelH		0x10
#define Max_Column	128
#define Max_Row		64
#define	Brightness	0xFF 
#define X_WIDTH 	128
#define Y_WIDTH 	64	

// typedef
#define u8     uint8_t
#define u16    uint16_t
#define u32    uint32_t

// SPI pin
#define USER_SPI_SCK     MICO_GPIO_37
#define USER_SPI_DIN     MICO_GPIO_35
#define USER_SPI_DC      MICO_GPIO_27
#define USER_SPI_CS      MICO_GPIO_16

//-----------------OLED�˿ڶ���----------------  					   
//#define OLED_CS_Clr()  MicoGpioOutputLow(GPIOD,GPIO_Pin_3)//CS
//#define OLED_CS_Set()  MicoGpioOutputHigh(GPIOD,GPIO_Pin_3)
//
//#define OLED_RST_Clr() MicoGpioOutputLow(GPIOD,GPIO_Pin_4)//RES
//#define OLED_RST_Set() MicoGpioOutputHigh(GPIOD,GPIO_Pin_4)
//
//#define OLED_DC_Clr() MicoGpioOutputLow(GPIOD,GPIO_Pin_5)//DC
//#define OLED_DC_Set() MicoGpioOutputHigh(GPIOD,GPIO_Pin_5)
//
//#define OLED_WR_Clr() MicoGpioOutputLow(GPIOG,GPIO_Pin_14)
//#define OLED_WR_Set() MicoGpioOutputHigh(GPIOG,GPIO_Pin_14)
//
//#define OLED_RD_Clr() MicoGpioOutputLow(GPIOG,GPIO_Pin_13)
//#define OLED_RD_Set() MicoGpioOutputHigh(GPIOG,GPIO_Pin_13)

#define OLED_CS_Clr()  MicoGpioOutputLow(USER_SPI_CS)//CS
#define OLED_CS_Set()  MicoGpioOutputHigh(USER_SPI_CS)

#define OLED_RST_Clr() 
#define OLED_RST_Set()

#define OLED_DC_Clr() MicoGpioOutputLow(USER_SPI_DC)//DC
#define OLED_DC_Set() MicoGpioOutputHigh(USER_SPI_DC)


//PC0~7,��Ϊ������
#define DATAOUT(x) GPIO_Write(GPIOC,x);//���  
//ʹ��4�ߴ��нӿ�ʱʹ�� 

#define OLED_SCLK_Clr() MicoGpioOutputLow(USER_SPI_SCK)//CLK
#define OLED_SCLK_Set() MicoGpioOutputHigh(USER_SPI_SCK)

#define OLED_SDIN_Clr() MicoGpioOutputLow(USER_SPI_DIN)//DIN
#define OLED_SDIN_Set() MicoGpioOutputHigh(USER_SPI_DIN)

 		     
#define OLED_CMD  0	//д����
#define OLED_DATA 1	//д����


//OLED�����ú���
void OLED_WR_Byte(u8 dat,u8 cmd);	    
void OLED_Display_On(void);
void OLED_Display_Off(void);	   							   		    
void OLED_Init(void);
void OLED_Clear(void);
void OLED_DrawPoint(u8 x,u8 y,u8 t);
void OLED_Fill(u8 x1,u8 y1,u8 x2,u8 y2,u8 dot);
void OLED_ShowChar(u8 x,u8 y,u8 chr);
void OLED_ShowNum(u8 x,u8 y,u32 num,u8 len,u8 size);
void OLED_ShowString(u8 x,u8 y, u8 *p);	 
void OLED_Set_Pos(unsigned char x, unsigned char y);
void OLED_ShowCHinese(u8 x,u8 y,u8 no);
void OLED_DrawBMP(unsigned char x0, unsigned char y0,unsigned char x1, unsigned char y1,unsigned char BMP[]);

void delay_init(void);
void delay_ms(u16 nms);
void delay_us(u32 nus);

#endif  
	 



