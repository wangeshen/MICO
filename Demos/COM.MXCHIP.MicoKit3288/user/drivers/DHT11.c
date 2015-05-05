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


static void dht11_delay_ms(uint32_t nTime_ms)
{
  if(nTime_ms > 0){
    mico_thread_msleep(nTime_ms);
  }
}

/*
*  return: 0:  从机无响应
*         1:  从机有相应
*        -1:  error
*/
int Send_Request(void)
{
  uint32_t TimingDelay1 = 0;
  int ret = 0;  
  OSStatus err = kUnknownErr;
  
  // init data pin output
  err = MicoGpioInitialize( (mico_gpio_t)DHT11_DATA, OUTPUT_PUSH_PULL );
  require_noerr_action( err, exit, dht11_log("ERROR: MicoGpioInitialize err=%d.", err) );
  
  // host set data low 20ms
  err = DHT11_DATA_Clr();
  require_noerr_action( err, exit, dht11_log("ERROR: MicoGpioOutputLow err=%d.", err) );
  dht11_delay_ms(20);	 
  
  // host set high 30us
  err = DHT11_DATA_Set();
  require_noerr_action( err, exit, dht11_log("ERROR: MicoGpioOutputLow err=%d.", err) );
  //Delay_10us(3);
  
  // host pin set input, wait for slave response
  err = MicoGpioInitialize( (mico_gpio_t)DHT11_DATA, INPUT_PULL_UP );
  require_noerr_action( err, exit, dht11_log("ERROR: MicoGpioInitialize err=%d.", err) );
  
  // wait for slave set data pin low, 80us timeout
  TimingDelay1 = 0;  //超时计时
  while((DHT11_DATA_IN) && ((TimingDelay1++)<800));
  
  err = kNoErr;
  if(TimingDelay1 >= 800)
  {
    return 0;  // timeout, slave no response
  }
  else
  {
    return 1;  // slave response ok
  }
  
exit:
  if(kNoErr != err){
    ret = -1;
  }
  return ret;
}

//static void delay_us(uint32_t nUs)
//{
//  uint32_t delay = 30*nUs;
//  while(delay--);
//}
//
//uint8_t com(void)
//{
//  uint8_t i = 0;
//  int tt = 0;
//  uint8_t sbuf = 0;
//  
//  for(i = 0;i<8;i++)
//  {
//    sbuf <<= 1;
//    tt = 1;
//    delay_us(20);//接收到响应后会出现50us的低电平表示发送数据的开始，所以这里小延时一下
//    while(!DHT11_DATA_IN);//等到高电平的出现，高电平的时间表示的是数据位的0和1
//    
//    //delay_us(25);//数据为0的信号时间为26-28us，1则为70us，这里超时检测
//    //while( (DHT11_DATA_IN) && (tt++)<700);
//    while( (DHT11_DATA_IN) && (tt++));
//    dht11_log("<<<<<<<<<<<<<<<<<DHT11 high=%d", tt);
//    
//    if(tt >=700)//如果还为高
//    {
//      sbuf |= 0x01;
//      //delay_us(30);//这里的延时足够了，40+10+30>70了
//    }
//    else //如果变低
//    {
//      sbuf &= 0xfe;
//    }
//  }
//  
//  return sbuf;
//}


/*------------------------------ USER INTERFACES -----------------------------*/

OSStatus DHT11_init(void)
{
  OSStatus err = kUnknownErr;
  err = MicoGpioInitialize( (mico_gpio_t)DHT11_DATA, OUTPUT_PUSH_PULL );
  require_noerr_action( err, exit, dht11_log("ERROR: DHT11_init [MicoGpioInitialize]err=%d.", err) );
  
  //  err = mico_init_timer(&_user_key1_timer, user_key1_long_press_timeout, _user_key1_timeout_handler, NULL);
  //  require_noerr_action( err, exit, dht11_log("ERROR: DHT11_init [mico_init_timer]err=%d.", err) );
  
exit:
  return err;
}

OSStatus DHT11_read(int8_t *temp, uint8_t *hum)
{
  OSStatus err = kUnknownErr;
  
  err = kNoErr;
  return err;
  
}

///*
//* return data[4] = data[0] | data[1] | data[2] | data[3] ( data[4])
//*                    hum       hum.     temp      temp.  check_sum
//*
//* return status:   0: success
//*                 -1: error
//*/
//int DHT11_read(uint8_t *data)
//{
//  int ret = 0;
//  uint8_t i = 0;
//  uint8_t sum = 0;
//  uint8_t check = 0;
//  
//  if(NULL == data){
//    dht11_log("ERROR: DHT11_read param error!");
//    return -1;
//  }
//  
//  if(Send_Request()){
//    dht11_log("DHT11 response OK!");
//    for(i=0; i<4; i++){
//      data[i] = com();
//      sum += data[i];
//    }
//    check = com();
//    if(sum == check){
//      ret = 0;
//    }
//    else{
//      ret = -2;
//    }
//  }
//  else{
//    dht11_log("ERROR: Send_Request error!");
//    ret = -1;   // request error
//  }
//  
//  return ret;
//}

