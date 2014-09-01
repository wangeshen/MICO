
#include "MICORTOS.h"
#include "MICOPlatform.h"

#include "EMW3162/platform.h"
#include "EMW3162/platform_common_config.h"
#include "stm32f2xxMapping.h"
#include "stm32f2xx.h"
#include "gpio_irq.h"

/******************************************************
 *                    Constants
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
 *               Variables Definitions
 ******************************************************/


/******************************************************
 *               Function Declarations
 ******************************************************/



/******************************************************
 *               Function Definitions
 ******************************************************/



OSStatus wiced_gpio_init( wiced_gpio_t gpio, wiced_gpio_config_t configuration )
{
    GPIO_InitTypeDef gpio_init_structure;

    wiced_platform_mcu_disable_powersave();

    RCC_AHB1PeriphClockCmd( gpio_mapping[gpio].peripheral_clock, ENABLE );
    gpio_init_structure.GPIO_Speed = GPIO_Speed_50MHz;
    gpio_init_structure.GPIO_Mode  = ( (configuration == INPUT_PULL_UP ) || (configuration == INPUT_PULL_DOWN ) || (configuration == INPUT_HIGH_IMPEDANCE ) ) ? GPIO_Mode_IN : GPIO_Mode_OUT;
    gpio_init_structure.GPIO_OType = (configuration == OUTPUT_PUSH_PULL )? GPIO_OType_PP :  GPIO_OType_OD;

    if ( (configuration == INPUT_PULL_UP ) || (configuration == OUTPUT_OPEN_DRAIN_PULL_UP ) )
    {
        gpio_init_structure.GPIO_PuPd  =  GPIO_PuPd_UP;
    }
    else if (configuration == INPUT_PULL_DOWN )
    {
        gpio_init_structure.GPIO_PuPd  =  GPIO_PuPd_DOWN;
    }
    else
    {
        gpio_init_structure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
    }

    gpio_init_structure.GPIO_Pin = (uint16_t) ( 1 << gpio_mapping[gpio].number );
    GPIO_Init( gpio_mapping[gpio].bank, &gpio_init_structure );

    wiced_platform_mcu_enable_powersave();

    return kNoErr;
}

OSStatus wiced_gpio_output_high( wiced_gpio_t gpio )
{
    wiced_platform_mcu_disable_powersave();

    GPIO_SetBits( gpio_mapping[gpio].bank, (uint16_t) (1 << gpio_mapping[gpio].number) );

    wiced_platform_mcu_enable_powersave();

    return kNoErr;
}

OSStatus wiced_gpio_output_low( wiced_gpio_t gpio )
{
    wiced_platform_mcu_disable_powersave();

    GPIO_ResetBits( gpio_mapping[gpio].bank, (uint16_t) (1 << gpio_mapping[gpio].number) );

    wiced_platform_mcu_enable_powersave();

    return kNoErr;
}

bool wiced_gpio_input_get( wiced_gpio_t gpio )
{
    bool result;

    wiced_platform_mcu_disable_powersave();

    result =  (GPIO_ReadInputDataBit( gpio_mapping[gpio].bank, (uint16_t) ( 1 << gpio_mapping[gpio].number ) ) == 0 )? false : true;

    wiced_platform_mcu_enable_powersave();

    return result;
}

OSStatus wiced_gpio_input_irq_enable( wiced_gpio_t gpio, wiced_gpio_irq_trigger_t trigger, wiced_gpio_irq_handler_t handler, void* arg )
{
    return gpio_irq_enable( gpio_mapping[gpio].bank, gpio_mapping[gpio].number, trigger, handler, arg );
}

OSStatus wiced_gpio_input_irq_disable( wiced_gpio_t gpio )
{
    return gpio_irq_disable( gpio_mapping[gpio].bank, gpio_mapping[gpio].number );
}


