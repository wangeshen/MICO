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
#include "platformInternal.h"
#include "platform_common_config.h"
#include "Update_for_OTA.h"

#define boot_log(M, ...) custom_log("BOOT", M, ##__VA_ARGS__)
#define boot_log_trace() custom_log_trace("BOOT")

extern void Main_Menu(void);

const char menu[] =
"\r\n"
"+************** (C) COPYRIGHT 2014 MXCHIP Corporation ***********+\r\n"
"|               MICO Common Bootloader                           |\r\n"
"+ command --------------------------------+ function ------------+\r\n"
"| 0:BOOTUPDATE  <-r>                      | Update bootloader    |\r\n"
"| 1:FWUPDATE    <-r>                      | Update application   |\r\n"
"| 2:DRIVERUPDATE     <-r>                 | Update RF driver     |\r\n"
"| 3:PARAUPDATE <-e>                       | Update MICO settings |\r\n"
"| 4:FWUPDATE  <-i><-s><-e><-r>            |                      |\r\n"
"|  <-start hex or dec><-end hex or dec>   | Update flash content |\r\n"
"| 5:BOOT                                  | Excute application   |\r\n"
"| 6:REBOOT                                | Reboot               |\r\n"
"| ?:HELP                                  | displays this help   |\r\n"
"+--------------------+-------------------------------------------+\r\n"
" -e Erase only  -r Read from flash\r\n"
" -i internal flash  -s SPI flash -start flash start address\r\n"
" -end flash start address\r\n"
" Example: Input \"4 -i -start 0x400 -end 0x800\": Update internal\r\n"
"          flash from address 0x400 to 0x 800\r\n"
" By William Xu from MXCHIP M2M Team\r\n";

int main(void)
{
  init_clocks();
  init_memory();
  init_architecture();
  init_platform_bootloader();
  
  update();
  
  /* BOOT_SEL = 1 => Normal start*/
  if(MicoGpioInputGet((mico_gpio_t)BOOT_SEL)==true)
    startApplication();
  /* BOOT_SEL = 0, MFG_SEL = 0 => Normal start, MICO will enter MFG mode when "MicoInit" is called*/
  else if(MicoGpioInputGet((mico_gpio_t)MFG_SEL)==false)
    startApplication();
  
  printf ( menu );

  while(1){                             
    Main_Menu ();
  }
}

