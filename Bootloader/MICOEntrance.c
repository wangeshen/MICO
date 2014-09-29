/**
******************************************************************************
* @file    MICOEntrance.c 
* @author  William Xu
* @version V1.0.0
* @date    05-May-2014
* @brief   MICO system main entrance.
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


#include "Mico.h"
#include "MicoPlatform.h"
#include "platform.h"
#include "platform_common_config.h"
#include "stm32f2xx.h"
#include "menu.h"

#define boot_log(M, ...) custom_log("BOOT", M, ##__VA_ARGS__)
#define boot_log_trace() custom_log_trace("BOOT")

const char menu[] =
"\r\n"
"+***************(C) COPYRIGHT 2014 MXCHIP corporation************+\r\n"
"|               MICO Common Bootloader                           |\r\n"
"+ command ----------------+ function ----------------------------+\r\n"
"| 1:FWUPDATE <-a>    | update the firmware from UART using Ymodem|\r\n"
"| 3:BOOT             | excute the current firmware               |\r\n"
"| 4:REBOOT           | Reboot                                    |\r\n"
"| ?:HELP             | displays this help                        |\r\n"
"+--------------------+-------------------------------------------+\r\n"
"|                           By William Xu from MXCHIP M2M Team   |\r\n"
"+----------------------------------------------------------------+\r\n";

#if defined ( __ICCARM__ )

static inline void __jump_to( uint32_t addr )
{
  __asm( "MOV R1, #0x00000001" );
  __asm( "ORR R0, R0, R1" );  /* Last bit of jump address indicates whether destination is Thumb or ARM code */
  __asm( "BLX R0" );
}

#elif defined ( __GNUC__ )

__attribute__( ( always_inline ) ) static __INLINE void __jump_to( uint32_t addr )
{
  addr |= 0x00000001;  /* Last bit of jump address indicates whether destination is Thumb or ARM code */
  __ASM volatile ("BX %0" : : "r" (addr) );
}

#endif

void startApplication(void)
{
  uint32_t text_addr = APPLICATION_START_ADDRESS;
  
  /* Test if user code is programmed starting from address "ApplicationAddress" */
  if (((*(volatile uint32_t*)text_addr) & 0x2FFE0000 ) == 0x20000000)
  { 
    uint32_t* stack_ptr;
    uint32_t* start_ptr;
    
    stack_ptr = (uint32_t*) text_addr;  /* Initial stack pointer is first 4 bytes of vector table */
    start_ptr = ( stack_ptr + 1 );  /* Reset vector is second 4 bytes of vector table */
    
    __asm( "MOV LR,        #0xFFFFFFFF" );
    __asm( "MOV R1,        #0x01000000" );
    __asm( "MSR APSR_nzcvq,     R1" );
    __asm( "MOV R1,        #0x00000000" );
    __asm( "MSR PRIMASK,   R1" );
    __asm( "MSR FAULTMASK, R1" );
    __asm( "MSR BASEPRI,   R1" );
    __asm( "MSR CONTROL,   R1" );
    
    SCB->VTOR = text_addr; /* Change the vector table to point to app vector table */
    __set_MSP( *stack_ptr );
    __jump_to( *start_ptr );
  }  
}

void PlatformEasyLinkButtonClickedCallback(void)
{
  startApplication();
  return;
}

void PlatformEasyLinkButtonLongPressedCallback(void)
{
  return;
}

void PlatformStandbyButtonClickedCallback(void)
{
  return;
}

int main(void)
{
  OSStatus err = kNoErr;  
  
  init_clocks();
  init_memory();
  init_architecture();
  
  MicoGpioInitialize((mico_gpio_t)BOOT_SEL, INPUT_PULL_UP);
  MicoGpioInitialize((mico_gpio_t)MFG_SEL, INPUT_HIGH_IMPEDANCE);
  
  /* BOOT_SEL = 1 => Normal start*/
  if(MicoGpioInputGet(BOOT_SEL)==true)
    startApplication();
  /* BOOT_SEL = 0, MFG_SEL = 0 => Normal start, MICO will enter MFG mode when "MicoInit" is called*/
  else if(MicoGpioInputGet(MFG_SEL)==false)
    startApplication();
  
  boot_log("Starting Bootloader");
  printf ( menu );
  while(1){                             
    Main_Menu ();
  }
  
  return err;
}

int application_start(void)
{
  boot_log("Bootloader should nerver come here! Rebootint......");
  MicoSystemReboot();
  return 0;
}


