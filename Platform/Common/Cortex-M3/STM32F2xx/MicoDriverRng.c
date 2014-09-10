
#include "MICOPlatform.h"
#include "MICORTOS.h"
#include "gpio_irq.h"

#include "platform.h"
#include "platform_common_config.h"
#include "stm32f2xx_platform.h"
#include "stm32f2xx.h"

/******************************************************
 *                   Macros
 ******************************************************/


/******************************************************
 *                   Enumerations
 ******************************************************/

/******************************************************
 *                 Type Definitions
 ******************************************************/

 /******************************************************
 *                    Structures
 ******************************************************/


/******************************************************
 *                     Variables
 ******************************************************/

/******************************************************
 *               Function Declarations
 ******************************************************/

OSStatus MicoRandomNumberRead( void *inBuffer, int inByteCount )
{
    // PLATFORM_TO_DO
    int idx;
    uint32_t *pWord = inBuffer;
    uint32_t tempRDM;
    uint8_t *pByte = NULL;
    int inWordCount;
    int remainByteCount;

    inWordCount = inByteCount/4;
    remainByteCount = inByteCount%4;
    pByte = (uint8_t *)pWord+inWordCount*4;

    RNG_DeInit();
    RCC_AHB2PeriphClockCmd(RCC_AHB2Periph_RNG, ENABLE);
    RNG_Cmd(ENABLE);

    for(idx = 0; idx<inWordCount; idx++, pWord++){
        while(RNG_GetFlagStatus(RNG_FLAG_DRDY)!=SET);
        *pWord = RNG_GetRandomNumber();
    }

    if(remainByteCount){
        while(RNG_GetFlagStatus(RNG_FLAG_DRDY)!=SET);
        tempRDM = RNG_GetRandomNumber();
        memcpy(pByte, &tempRDM, (size_t)remainByteCount);
    }
    
    RNG_DeInit();
    return kNoErr;
}
