
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

OSStatus wiced_pwm_init( wiced_pwm_t pwm_peripheral, uint32_t frequency, float duty_cycle )
{
    TIM_TimeBaseInitTypeDef tim_time_base_structure;
    TIM_OCInitTypeDef       tim_oc_init_structure;
    GPIO_InitTypeDef        gpio_init_structure;
    RCC_ClocksTypeDef       rcc_clock_frequencies;
    const platform_pwm_mapping_t* pwm                 = &pwm_mappings[pwm_peripheral];
    uint16_t                      period              = 0;
    float                         adjusted_duty_cycle = ( ( duty_cycle > 100.0f ) ? 100.0f : duty_cycle );

    wiced_platform_mcu_enable_powersave();

    RCC_GetClocksFreq( &rcc_clock_frequencies );

    if ( pwm->tim == TIM1 || pwm->tim == TIM8 || pwm->tim == TIM9 || pwm->tim == TIM10 || pwm->tim == TIM11 )
    {
        RCC_APB2PeriphClockCmd( pwm->tim_peripheral_clock, ENABLE );
        period = (uint16_t)( rcc_clock_frequencies.PCLK2_Frequency / frequency - 1 ); /* Auto-reload value counts from 0; hence the minus 1 */
    }
    else
    {
        RCC_APB1PeriphClockCmd( pwm->tim_peripheral_clock, ENABLE );
        period = (uint16_t)( rcc_clock_frequencies.PCLK1_Frequency / frequency - 1 ); /* Auto-reload value counts from 0; hence the minus 1 */
    }

    RCC_AHB1PeriphClockCmd( pwm->pin->peripheral_clock, ENABLE );

    gpio_init_structure.GPIO_Pin   = (uint32_t) ( 1 << pwm->pin->number );
    gpio_init_structure.GPIO_Mode  = GPIO_Mode_AF;
    gpio_init_structure.GPIO_Speed = GPIO_Speed_100MHz;
    gpio_init_structure.GPIO_OType = GPIO_OType_PP;
    gpio_init_structure.GPIO_PuPd  = GPIO_PuPd_UP;
    GPIO_Init( pwm->pin->bank, &gpio_init_structure );
    GPIO_PinAFConfig( pwm->pin->bank, pwm->pin->number, pwm->gpio_af );

    /* Time base configuration */
    tim_time_base_structure.TIM_Period            = (uint32_t) period;
    tim_time_base_structure.TIM_Prescaler         = (uint16_t) 1;  /* Divide clock by 1+1 to enable a count of high cycle + low cycle = 1 PWM cycle */
    tim_time_base_structure.TIM_ClockDivision     = 0;
    tim_time_base_structure.TIM_CounterMode       = TIM_CounterMode_Up;
    tim_time_base_structure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit( pwm->tim, &tim_time_base_structure );

    /* PWM1 Mode configuration */
    tim_oc_init_structure.TIM_OCMode       = TIM_OCMode_PWM1;
    tim_oc_init_structure.TIM_OutputState  = TIM_OutputState_Enable;
    tim_oc_init_structure.TIM_OutputNState = TIM_OutputNState_Enable;
    tim_oc_init_structure.TIM_Pulse        = (uint16_t) ( adjusted_duty_cycle * (float) period / 100.0f );
    tim_oc_init_structure.TIM_OCPolarity   = TIM_OCPolarity_High;
    tim_oc_init_structure.TIM_OCNPolarity  = TIM_OCNPolarity_High;
    tim_oc_init_structure.TIM_OCIdleState  = TIM_OCIdleState_Reset;
    tim_oc_init_structure.TIM_OCNIdleState = TIM_OCIdleState_Set;

    switch ( pwm->channel )
    {
        case 1:
        {
            TIM_OC1Init( pwm->tim, &tim_oc_init_structure );
            TIM_OC1PreloadConfig( pwm->tim, TIM_OCPreload_Enable );
            break;
        }
        case 2:
        {
            TIM_OC2Init( pwm->tim, &tim_oc_init_structure );
            TIM_OC2PreloadConfig( pwm->tim, TIM_OCPreload_Enable );
            break;
        }
        case 3:
        {
            TIM_OC3Init( pwm->tim, &tim_oc_init_structure );
            TIM_OC3PreloadConfig( pwm->tim, TIM_OCPreload_Enable );
            break;
        }
        case 4:
        {
            TIM_OC4Init( pwm->tim, &tim_oc_init_structure );
            TIM_OC4PreloadConfig( pwm->tim, TIM_OCPreload_Enable );
            break;
        }
        default:
        {
            break;
        }
    }

    wiced_platform_mcu_disable_powersave();

    return kNoErr;
}

OSStatus wiced_pwm_start( wiced_pwm_t pwm )
{
    wiced_platform_mcu_enable_powersave();

    TIM_Cmd(pwm_mappings[pwm].tim, ENABLE);
    TIM_CtrlPWMOutputs( pwm_mappings[pwm].tim, ENABLE );

    wiced_platform_mcu_disable_powersave();

    return kNoErr;
}

OSStatus wiced_pwm_stop( wiced_pwm_t pwm )
{
    wiced_platform_mcu_enable_powersave();

    TIM_CtrlPWMOutputs( pwm_mappings[pwm].tim, DISABLE );
    TIM_Cmd(pwm_mappings[pwm].tim, DISABLE);

    wiced_platform_mcu_disable_powersave();

    return kNoErr;
}


