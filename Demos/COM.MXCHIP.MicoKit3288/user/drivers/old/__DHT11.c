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
#include "stm32f4xx_tim.h"
#include "stm32f4xx_rcc.h"

#define dht11_log(M, ...) custom_log("DHT11", M, ##__VA_ARGS__)
#define dht11_log_trace() custom_log_trace("DHT11")


/* Private variables ---------------------------------------------------------*/
 uint32_t uhIC4ReadCountBuffer[DHT11_DATA_BITS+1] = {0};
 uint16_t uhCaptureNumber = 0;

 volatile bool dht11_read_done = false;


static void dht11_delay_ms(uint32_t nTime_ms)
{
  if(nTime_ms > 0){
    mico_thread_msleep(nTime_ms);
  }
}


OSStatus Send_Request(void)
{
  OSStatus err = kUnknownErr;
  uint32_t TimingDelay1 = 0;
  
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
  TimingDelay1 = 0;  //³¬Ê±¼ÆÊ±
  while((DHT11_DATA_IN) && ((TimingDelay1++)<8000));
//  
//  err = kNoErr;
//  if(TimingDelay1 >= 800)
//  {
//    return 0;  // timeout, slave no response
//  }
//  else
//  {
//    return 1;  // slave response ok
//  }
  
exit:
  return err;
}

static uint32_t get_diff_cnt(uint32_t value1, uint32_t value2)
{
  uint32_t uwCapture = 0;
  
  /* Capture computation */
  if (value2 > value1)
  {
    uwCapture = (value2 - value1); 
  }
  else if (value2 < value1)
  {
    uwCapture = ((0xFFFFFFFF - value1) + value2); 
  }
  else
  {
    uwCapture = 0;
  }
  
  return uwCapture;
}

static OSStatus get_temp_hum_from_buffer(uint32_t *read_buffer, uint32_t read_buffer_len,
                                         int8_t *temp, uint8_t *hum)
{
    //OSStatus err = kUnknownErr;
    uint32_t start_cnt = 0;
    uint32_t bit_cnt = 0;
    uint32_t bit0_cnt = 0;
    int i = 0;
    uint8_t bytes_buffer[DHT11_DATA_BITS] = {0};
    int j = 0;
    uint8_t check_byte = 0;
    
    start_cnt = get_diff_cnt(read_buffer[0], read_buffer[1]);
    if(start_cnt > 0){
      //bit0_cnt = (start_cnt*DHT11_DATA_RESPONSE_BIT0_TIME)/DHT11_DATA_RESPONSE_START_TIME;
      bit0_cnt = 77;
      for(i = 0; i <= (DHT11_DATA_BITS); i++){
        bit_cnt = get_diff_cnt(read_buffer[i], read_buffer[i+1]);
        if((0 != i) && (0 == i%8)){
          j++;
        }
        bytes_buffer[j] <<= 1;
        
        if(bit_cnt > bit0_cnt){
          bytes_buffer[j] |= 0x01;  // bit 1
        }
        else{
          bytes_buffer[j] &= 0xfe;  // bit 0
        }
      }
      // check data
      check_byte = bytes_buffer[0] + bytes_buffer[1] + bytes_buffer[2] + bytes_buffer[3];
      if(check_byte == bytes_buffer[4]){
        *temp =  (int8_t)bytes_buffer[2];
        *hum = bytes_buffer[0];
      }
      else{
        return kChecksumErr;
      }
    }
    else{
      return kResponseErr;
    }
    
    return kNoErr;
}

static void _dht11_data_pin_irq_handler( void* arg )
{
  (void)(arg);
  
  if(uhCaptureNumber <= (DHT11_DATA_BITS)){
    uhIC4ReadCountBuffer[uhCaptureNumber] = TIM_GetCounter(TIM1);
    uhCaptureNumber++;
    // end
    if((DHT11_DATA_BITS+1) == uhCaptureNumber){
      dht11_read_done = true;
      uhCaptureNumber = 0;
    }
  }
}

static void TIM_Config(void)
{
//  //GPIO_InitTypeDef GPIO_InitStructure;
//  NVIC_InitTypeDef NVIC_InitStructure;
//  TIM_ICInitTypeDef  TIM_ICInitStructure;
//  
//  /* TIM1 clock enable */
//  RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM2, ENABLE);
//
//  /* GPIOA clock enable */
////  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
//  
////  /* TIM1 channel 2 pin (PE.11) configuration */
////  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_11;
////  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
////  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
////  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
////  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
////  GPIO_Init(GPIOE, &GPIO_InitStructure);
//
//  /* Connect TIM pins to AF2 */
////  GPIO_PinAFConfig(GPIOE, GPIO_PinSource11, GPIO_AF_TIM1);
//  
//  /* Enable the TIM1 global Interrupt */
//  NVIC_InitStructure.NVIC_IRQChannel = TIM1_CC_IRQn;
//  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
//  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
//  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//  NVIC_Init(&NVIC_InitStructure);
//  
//    /* TIM1 configuration: Input Capture mode ---------------------
//     The external signal is connected to TIM1 CH4 pin (PA11)  
//     The Falling edge is used as active edge,
//     The TIM1 CCR4 is used to compute the frequency value 
//  ------------------------------------------------------------ */
//
//  TIM_ICInitStructure.TIM_Channel = TIM_Channel_4;
//  TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Falling;
//  TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
//  TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
//  TIM_ICInitStructure.TIM_ICFilter = 0x0;
//
//  TIM_ICInit(TIM1, &TIM_ICInitStructure);
//  
//  /* TIM enable counter */
//  TIM_Cmd(TIM1, ENABLE);
//
//  /* Enable the CC4 Interrupt Request */
//  TIM_ITConfig(TIM1, TIM_IT_CC4, ENABLE);
  
  
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
  
  /* TIM1 clock enable */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
  
  // TIM2 configuration    10ms
  TIM_DeInit(TIM1);
  TIM_TimeBaseStructure.TIM_Period = 10000;          
  TIM_TimeBaseStructure.TIM_Prescaler = (100-1);       
  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  
  TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);
  
  // Clear TIM1 update pending flag 
  TIM_ClearFlag(TIM1, TIM_FLAG_Update);
  // Disable TIM1 Update interrupt 
  TIM_ITConfig(TIM1, TIM_IT_Update, DISABLE);
  // TIM2 enable counter 
  TIM_Cmd(TIM1, ENABLE);
  
  MicoGpioEnableIRQ( (mico_gpio_t)DHT11_DATA, IRQ_TRIGGER_FALLING_EDGE, _dht11_data_pin_irq_handler, NULL );
}

static void TIM_Stop(void){
  
  MicoGpioDisableIRQ((mico_gpio_t)DHT11_DATA);
  
  /* TIM enable counter */
  TIM_Cmd(TIM1, DISABLE);
  
  /* Enable the CC4 Interrupt Request */
  //TIM_ITConfig(TIM1, TIM_IT_CC4, DISABLE);
}

 
/******************************************************************************/
/*            STM32F4xx Peripherals Interrupt Handlers                        */
/******************************************************************************/
/**
  * @brief  This function handles TIM1 global interrupt request.
  * @param  None
  * @retval None
  */
//void TIM1_CC_IRQHandler(void)
//{ 
//  if(TIM_GetITStatus(TIM1, TIM_IT_CC4) == SET) 
//  {
//    /* Clear TIM1 Capture compare interrupt pending bit */
//    TIM_ClearITPendingBit(TIM1, TIM_IT_CC4);
//    
//    if(uhCaptureNumber <= (DHT11_DATA_BITS+1)){
//      uhIC4ReadCountBuffer[uhCaptureNumber] = TIM_GetCapture4(TIM1);
//      uhCaptureNumber++;
//      // end
//      if((DHT11_DATA_BITS+2) == uhCaptureNumber){
//        dht11_read_done = true;
//      }
//    }
//  }
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
  
  if((NULL == temp) || (NULL == hum)){
    return kParamErr;
  }
  
  if(kNoErr == Send_Request()){
    dht11_read_done = false;
    TIM_Config();
    mico_thread_msleep(5);
    TIM_Stop();
    
    if(dht11_read_done){
      // deal with read buffer
      err = get_temp_hum_from_buffer(uhIC4ReadCountBuffer, sizeof(uhIC4ReadCountBuffer),
                                     temp, hum);
      dht11_read_done = false;
    }
    else{ 
      err = kReadErr;  // read error
    }
  }
  else{
    err = kCommandErr;
    dht11_log("ERROR; Send_Request error!");
  }
  
  return err;
  
}


