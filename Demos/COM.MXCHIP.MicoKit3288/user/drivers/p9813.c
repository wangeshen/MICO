/**
******************************************************************************
* @file    p9813.c
* @author  Eshen Wang
* @version V1.0.0
* @date    17-Mar-2015
* @brief   rgb led controller. 
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

#include "MICO.h"
#include "p9813.h"

#define rgb_led_log(M, ...) custom_log("rgb_led", M, ##__VA_ARGS__)
#define rgb_led_log_trace() custom_log_trace("rgb_led")


#ifndef RGB_LED_USE_I2C

/* use gpio */
void RGB_LED_write_frame(uint32_t data)
{	
  uint8_t i;			  
  uint32_t f_data = data;
  //rgb_led_log("frame_data = %x", f_data);
    
  RGB_LED_CIN_Clr();
  RGB_LED_DIN_Clr();
  
  for(i=0; i<32; i++){
    RGB_LED_CIN_Clr();
    if(f_data & 0x80000000){
      RGB_LED_DIN_Set();
    }
    else{
      RGB_LED_DIN_Clr();
    }
    RGB_LED_CIN_Set();  // raise edge to set data
    f_data = f_data << 1;
    //rgb_led_log("frame_data<<1 =%x", f_data);
  }	
  
  RGB_LED_CIN_Clr();
  RGB_LED_DIN_Clr();   	  
}

void RGB_LED_write_start_frame(void)
{	
  uint32_t start_frame = 0x00000000;
  RGB_LED_write_frame(start_frame); 	  
}

void RGB_LED_write_data(uint8_t blue, uint8_t green, uint8_t red)
{
  uint8_t check_byte = 0xC0;  // starting flag "11"
  uint32_t send_data = 0;
  
  // calc check data
  check_byte |= (((~blue) >> 2) & 0x30);  // B7, B6
  check_byte |= (((~green) >> 4) & 0x0C);  // G7,G6
  check_byte |= (((~red) >> 6) & 0x03);   // R7,R6
  
  // create send data 32bit
  send_data |= (check_byte << 24) | (blue << 16) | (green << 8) | (red);
  
  //send_data = 0xFC0000FF;
  RGB_LED_write_frame(send_data);
}
 
#else  // use i2c interface

// I2C device
mico_i2c_device_t p9813_i2c_device = {
  MICO_I2C_1, 0x76, I2C_ADDRESS_WIDTH_7BIT, I2C_STANDARD_SPEED_MODE
};

void RGB_LED_write_frame_with_i2c(uint8_t *data, uint32_t cnt)
{
   mico_i2c_message_t _i2c_msg = {NULL, NULL, 0, 0, 0, false};
   
   MicoI2cBuildTxMessage(&_i2c_msg, data, cnt, 10);
   MicoI2cTransfer(&p9813_i2c_device, &_i2c_msg, 1);
}

void RGB_LED_write_data_with_i2c(uint8_t blue, uint8_t green, uint8_t red)
{
  uint8_t check_byte = 0xC0;  // starting flag "11"
  uint8_t send_data[8] = {0};
  
  // starting frame
  send_data[0] = 0x00;
  send_data[1] = 0x00;
  send_data[2] = 0x00;
  send_data[3] = 0x00;
  
  // calc check byte
  check_byte |= (((~blue) >> 2) & 0x30);  // B7, B6
  check_byte |= (((~green) >> 4) & 0x0C);  // G7,G6
  check_byte |= (((~red) >> 6) & 0x03);   // R7,R6
  
  // create send data 32bit (check_byte|blue|green|red)
  //send_data[1] |= (check_byte << 24) | (blue << 16) | (green << 8) | (red);
  send_data[4] = check_byte;
  send_data[5] =  (blue << 16);
  send_data[6] =  (green << 8);
  send_data[7] =  (red);
  
  // send data use i2c
  RGB_LED_write_frame_with_i2c(send_data, 8);
}

#endif

/*------------------------------- user interfaces ----------------------------*/
void rgb_led_init(void)
{
#ifndef RGB_LED_USE_I2C
  
  MicoGpioInitialize( (mico_gpio_t)RGB_LED_CIN, OUTPUT_OPEN_DRAIN_NO_PULL );
  MicoGpioInitialize( (mico_gpio_t)RGB_LED_DIN, OUTPUT_OPEN_DRAIN_NO_PULL );
  
#else
  
  // just use i2c sck && sda to send data to control p9813
  OSStatus err = kUnknownErr;
  err = MicoI2cInitialize(&p9813_i2c_device);
  //require_noerr_action( err, exit, bme280_log("ERROR: MicoI2cInitialize err = %d.", err) );
  
#endif
}

void rgb_led_open(uint16_t blue, uint16_t green, uint16_t red)
{
#ifndef RGB_LED_USE_I2C
  RGB_LED_write_start_frame();
  RGB_LED_write_data(blue, green, red);
#else
  RGB_LED_write_data_with_i2c(blue, green, red);
#endif
}

void rgb_led_close(void)
{
  rgb_led_open(0, 0, 0);
}
