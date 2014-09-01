
#include "Debug.h"
#include "MICORTOS.h"
#include "MICOPlatform.h"

#include "rtc.h"
#include "platform.h"
#include "platform_common_config.h"
#include "stm32f2xx.h"

/******************************************************
 *                      Macros
 ******************************************************/

#ifndef WICED_DISABLE_MCU_POWERSAVE
#define MCU_CLOCKS_NEEDED()       stm32f2xx_clocks_needed()
#define MCU_CLOCKS_NOT_NEEDED()   stm32f2xx_clocks_not_needed()
#define MCU_RTC_INIT()            RTC_Wakeup_init()
#else
#define MCU_CLOCKS_NEEDED()
#define MCU_CLOCKS_NOT_NEEDED()
#ifdef WICED_ENABLE_MCU_RTC
#define MCU_RTC_INIT() platform_rtc_init()
#else /* #ifdef WICED_ENABLE_MCU_RTC */
#define MCU_RTC_INIT()
#endif /* #ifdef WICED_ENABLE_MCU_RTC */
#endif /* ifndef WICED_DISABLE_MCU_POWERSAVE */

#define USE_RTC_BKP 0x00BB32F2// yhb defined, use RTC BKP to initilize system time.

#define Platform_log(M, ...) custom_log("Platform", M, ##__VA_ARGS__)
#define Platform_log_trace() custom_log_trace("Platform")

/******************************************************
 *                    Constants
 ******************************************************/

#ifndef STDIO_BUFFER_SIZE
#define STDIO_BUFFER_SIZE   64
#endif

#define NUMBER_OF_LSE_TICKS_PER_MILLISECOND(scale_factor) ( 32768 / 1000 / scale_factor )
#define CONVERT_FROM_TICKS_TO_MS(n,s) ( n / NUMBER_OF_LSE_TICKS_PER_MILLISECOND(s) )
#define CK_SPRE_CLOCK_SOURCE_SELECTED 0xFFFF

/******************************************************
 *               Variables Definitions
 ******************************************************/

#ifndef MICO_DISABLE_STDIO
static const wiced_uart_config_t stdio_uart_config =
{
    .baud_rate    = 115200,
    .data_width   = DATA_WIDTH_8BIT,
    .parity       = NO_PARITY,
    .stop_bits    = STOP_BITS_1,
    .flow_control = FLOW_CONTROL_DISABLED,
};

static volatile ring_buffer_t   stdio_rx_buffer;
static volatile uint8_t         stdio_rx_data[STDIO_BUFFER_SIZE];
static mico_mutex_t             stdio_rx_mutex;
static mico_mutex_t             stdio_tx_mutex;
#endif /* #ifndef MICO_DISABLE_STDIO */

wiced_rtc_time_t wiced_default_time =
{
     /* set it to 12:20:30 08/04/2013 monday */
     .sec   = 30,
     .min   = 20,
     .hr    = 12,
     .weekday  = 1,
     .date  = 8,
     .month = 4,
     .year  = 13
};

static char stm32_platform_inited = 0;

#ifndef WICED_DISABLE_MCU_POWERSAVE
static bool wake_up_interrupt_triggered  = false;
static unsigned long rtc_timeout_start_time           = 0;
#endif /* #ifndef WICED_DISABLE_MCU_POWERSAVE */

/******************************************************
 *               Function Declarations
 ******************************************************/

#if defined(WICED_ENABLE_MCU_RTC) && defined(WICED_DISABLE_MCU_POWERSAVE)
void platform_rtc_init( void );
#endif /* #if defined(WICED_ENABLE_MCU_RTC) && defined(WICED_DISABLE_MCU_POWERSAVE) */
void wake_up_interrupt_notify( void );

/******************************************************
 *               Function Definitions
 ******************************************************/

 /* STM32F2 common clock initialisation function
 * This brings up enough clocks to allow the processor to run quickly while initialising memory.
 * Other platform specific clock init can be done in init_platform() or init_architecture()
 */
__weak void init_clocks( void )
{
    //RCC_DeInit( ); /* if not commented then the LSE PA8 output will be disabled and never comes up again */

    /* Configure Clocks */

    RCC_HSEConfig( HSE_SOURCE );
    RCC_WaitForHSEStartUp( );

    RCC_HCLKConfig( AHB_CLOCK_DIVIDER );
    RCC_PCLK2Config( APB2_CLOCK_DIVIDER );
    RCC_PCLK1Config( APB1_CLOCK_DIVIDER );

    /* Enable the PLL */
    FLASH_SetLatency( INT_FLASH_WAIT_STATE );
    FLASH_PrefetchBufferCmd( ENABLE );

    /* Use the clock configuration utility from ST to calculate these values
     * http://www.st.com/st-web-ui/static/active/en/st_prod_software_internet/resource/technical/software/utility/stsw-stm32090.zip
     */
    RCC_PLLConfig( PLL_SOURCE, PLL_M_CONSTANT, PLL_N_CONSTANT, PLL_P_CONSTANT, PPL_Q_CONSTANT ); /* NOTE: The CPU Clock Frequency is independently defined in <WICED-SDK>/Wiced/Platform/<platform>/<platform>.mk */
    RCC_PLLCmd( ENABLE );

    while ( RCC_GetFlagStatus( RCC_FLAG_PLLRDY ) == RESET )
    {
    }
    RCC_SYSCLKConfig( SYSTEM_CLOCK_SOURCE );

    while ( RCC_GetSYSCLKSource( ) != 0x08 )
    {
    }

    /* Configure HCLK clock as SysTick clock source. */
    SysTick_CLKSourceConfig( SYSTICK_CLOCK_SOURCE );

#ifndef MICO_DISABLE_STDIO
    {
        USART_ClockInitTypeDef usart_clock_init_structure;
        STDIO_CLOCK_CMD( STDIO_PERIPH_CLOCK, ENABLE );

        usart_clock_init_structure.USART_Clock   = USART_Clock_Disable;
        usart_clock_init_structure.USART_CPOL    = USART_CPOL_Low;
        usart_clock_init_structure.USART_CPHA    = USART_CPHA_2Edge;
        usart_clock_init_structure.USART_LastBit = USART_LastBit_Disable;

        USART_ClockInit( STDIO_USART, &usart_clock_init_structure );
    }
#endif /* ifdef MICO_DISABLE_STDIO */
}

__weak void init_memory( void )
{

}

void init_architecture( void )
{
    uint8_t i;

#ifdef INTERRUPT_VECTORS_IN_RAM
    SCB->VTOR = 0x20000000; /* Change the vector table to point to start of SRAM */
#endif /* ifdef INTERRUPT_VECTORS_IN_RAM */

    if ( stm32_platform_inited == 1 )
        return;

    watchdog_init( );

    /* Initialise the interrupt priorities to a priority lower than 0 so that the BASEPRI register can mask them */
    for ( i = 0; i < 81; i++ )
    {
        NVIC ->IP[i] = 0xff;
    }

    NVIC_PriorityGroupConfig( NVIC_PriorityGroup_4 );

#ifndef MICO_DISABLE_STDIO
    mico_rtos_init_mutex( &stdio_tx_mutex );
    mico_rtos_unlock_mutex ( &stdio_tx_mutex );
    mico_rtos_init_mutex( &stdio_rx_mutex );
    mico_rtos_unlock_mutex ( &stdio_rx_mutex );
    ring_buffer_init  ( (ring_buffer_t*)&stdio_rx_buffer, (uint8_t*)stdio_rx_data, STDIO_BUFFER_SIZE );
    PlatformStdioUartInitialize( &stdio_uart_config, (ring_buffer_t*)&stdio_rx_buffer );
#endif

    /* Ensure 802.11 device is in reset. */
    host_platform_init( );

    MCU_RTC_INIT();

    /* Disable MCU powersave at start-up. Application must explicitly enable MCU powersave if desired */
    MCU_CLOCKS_NEEDED();

    stm32_platform_inited = 1;
}

void PlatformSoftReboot(void)
{
  NVIC_SystemReset();
}

#define RTC_INTERRUPT_EXTI_LINE EXTI_Line22
#define WUT_COUNTER_MAX  0xffff

#define ENABLE_INTERRUPTS   __asm("CPSIE i")
#define DISABLE_INTERRUPTS  __asm("CPSID i")

/**
 * This function will set MCU RTC time to a new value. Time value must be given in the format of
 * the structure wiced_rtc_time_t
 *
 * @return    WICED_SUCCESS : on success.
 * @return    WICED_ERROR   : if an error occurred with any step
 */
OSStatus wiced_platform_set_rtc_time(wiced_rtc_time_t* time)
{
#ifdef WICED_ENABLE_MCU_RTC
    RTC_TimeTypeDef rtc_write_time;
    RTC_DateTypeDef rtc_write_date;
    wiced_bool_t    valid = WICED_FALSE;

    WICED_VERIFY_TIME(time, valid);
    if( valid == WICED_FALSE )
    {
        return WICED_BADARG;
    }
    rtc_write_time.RTC_Seconds = time->sec;
    rtc_write_time.RTC_Minutes = time->min;
    rtc_write_time.RTC_Hours   = time->hr;
    rtc_write_date.RTC_WeekDay = time->weekday;
    rtc_write_date.RTC_Date    = time->date;
    rtc_write_date.RTC_Month   = time->month;
    rtc_write_date.RTC_Year    = time->year;


    RTC_SetTime( RTC_Format_BIN, &rtc_write_time );
    RTC_SetDate( RTC_Format_BIN, &rtc_write_date );

    return WICED_SUCCESS;
#else /* #ifdef WICED_ENABLE_MCU_RTC */
    UNUSED_PARAMETER(time);
    return kUnsupportedErr;
#endif /* #ifdef WICED_ENABLE_MCU_RTC */
}

#ifndef WICED_DISABLE_MCU_POWERSAVE

static int stm32f2_clock_needed_counter = 0;

void stm32f2xx_clocks_needed( void )
{
    DISABLE_INTERRUPTS;
    if ( stm32f2_clock_needed_counter <= 0 )
    {
        SCB->SCR &= (~((unsigned long)SCB_SCR_SLEEPDEEP_Msk));
        stm32f2_clock_needed_counter = 0;
    }
    stm32f2_clock_needed_counter++;
    ENABLE_INTERRUPTS;
}

void stm32f2xx_clocks_not_needed( void )
{
    DISABLE_INTERRUPTS;
    stm32f2_clock_needed_counter--;
    if ( stm32f2_clock_needed_counter <= 0 )
    {
        SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
        stm32f2_clock_needed_counter = 0;
    }
    ENABLE_INTERRUPTS;
}


#ifndef WICED_DISABLE_MCU_POWERSAVE
void RTC_Wakeup_init(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;
    EXTI_InitTypeDef EXTI_InitStructure;
    RTC_InitTypeDef RTC_InitStruct;

    RTC_DeInit( );

    RTC_InitStruct.RTC_HourFormat = RTC_HourFormat_24;

    /* RTC ticks every second */
    RTC_InitStruct.RTC_AsynchPrediv = 0x7F;
    RTC_InitStruct.RTC_SynchPrediv = 0xFF;

    RTC_Init( &RTC_InitStruct );

    /* Enable the PWR clock */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);

    /* RTC clock source configuration ------------------------------------------*/
    /* Allow access to BKP Domain */
    PWR_BackupAccessCmd(ENABLE);
#ifdef USE_RTC_BKP
    PWR_BackupRegulatorCmd(ENABLE);
#endif

    /* Enable the LSE OSC */
    RCC_LSEConfig(RCC_LSE_ON);
    /* Wait till LSE is ready */
    while(RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET)
    {
    }

    /* Select the RTC Clock Source */
    RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);

    /* Enable the RTC Clock */
    RCC_RTCCLKCmd(ENABLE);

    /* RTC configuration -------------------------------------------------------*/
    /* Wait for RTC APB registers synchronisation */
    RTC_WaitForSynchro();

    RTC_WakeUpCmd( DISABLE );
    EXTI_ClearITPendingBit( RTC_INTERRUPT_EXTI_LINE );
    PWR_ClearFlag(PWR_FLAG_WU);
    RTC_ClearFlag(RTC_FLAG_WUTF);

    RTC_WakeUpClockConfig(RTC_WakeUpClock_RTCCLK_Div2);

    EXTI_ClearITPendingBit( RTC_INTERRUPT_EXTI_LINE );
    EXTI_InitStructure.EXTI_Line = RTC_INTERRUPT_EXTI_LINE;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel = RTC_WKUP_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    RTC_ITConfig(RTC_IT_WUT, DISABLE);

    /* Prepare Stop-Mode but leave disabled */
//    PWR_ClearFlag(PWR_FLAG_WU);
    PWR->CR |= PWR_CR_LPDS;
    PWR->CR &= (unsigned long)(~(PWR_CR_PDDS));
    SCB->SCR |= ((unsigned long)SCB_SCR_SLEEPDEEP_Msk);

#ifdef USE_RTC_BKP
    if (RTC_ReadBackupRegister(RTC_BKP_DR0) != USE_RTC_BKP) {
        /* set it to 12:20:30 08/04/2013 monday */
        wiced_default_time.sec   = 0,
        wiced_default_time.min   = 0,
        wiced_default_time.hr    = 0,
        wiced_default_time.weekday  = 1,
        wiced_default_time.date  = 8,
        wiced_default_time.month = 4,
        wiced_default_time.year  = 13;
        platform_set_rtc_time(&wiced_default_time);
        RTC_WriteBackupRegister(RTC_BKP_DR0, USE_RTC_BKP);
    }
#else
//#ifdef RTC_ENABLED
    /* application must have wiced_application_default_time structure declared somewhere, otherwise it wont compile */
    /* write default application time inside rtc */
    platform_set_rtc_time(&wiced_default_time);
//#endif /* RTC_ENABLED */
#endif
}
#endif /* #ifndef WICED_DISABLE_MCU_POWERSAVE */


static OSStatus select_wut_prescaler_calculate_wakeup_time( unsigned long* wakeup_time, unsigned long delay_ms, unsigned long* scale_factor )
{
    unsigned long temp;
    bool scale_factor_is_found = false;
    int i                              = 0;

    static unsigned long int available_wut_prescalers[] =
    {
        RTC_WakeUpClock_RTCCLK_Div2,
        RTC_WakeUpClock_RTCCLK_Div4,
        RTC_WakeUpClock_RTCCLK_Div8,
        RTC_WakeUpClock_RTCCLK_Div16
    };
    static unsigned long scale_factor_values[] = { 2, 4, 8, 16 };

    if ( delay_ms == 0xFFFFFFFF )
    {
        /* wake up in a 100ms, since currently there may be no tasks to run, but after a few milliseconds */
        /* some of them can get unblocked( for example a task is blocked on mutex with unspecified ) */
        *scale_factor = 2;
        RTC_WakeUpClockConfig( RTC_WakeUpClock_RTCCLK_Div2 );
        *wakeup_time = NUMBER_OF_LSE_TICKS_PER_MILLISECOND( scale_factor_values[0] ) * 100;
    }
    else
    {
        for ( i = 0; i < 4; i++ )
        {
            temp = NUMBER_OF_LSE_TICKS_PER_MILLISECOND( scale_factor_values[i] ) * delay_ms;
            if ( temp < WUT_COUNTER_MAX )
            {
                scale_factor_is_found = true;
                *wakeup_time = temp;
                *scale_factor = scale_factor_values[i];
                break;
            }
        }
        if ( scale_factor_is_found )
        {
            /* set new prescaler for wakeup timer */
            RTC_WakeUpClockConfig( available_wut_prescalers[i] );
        }
        else
        {
            /* scale factor can not be picked up for delays more that 32 seconds when RTCLK is selected as a clock source for the wakeup timer
             * for delays more than 32 seconds change from RTCCLK to 1Hz ck_spre clock source( used to update calendar registers ) */
            RTC_WakeUpClockConfig( RTC_WakeUpClock_CK_SPRE_16bits );

            /* with 1Hz ck_spre clock source the resolution changes to seconds  */
            *wakeup_time = ( delay_ms / 1000 ) + 1;
            *scale_factor = CK_SPRE_CLOCK_SOURCE_SELECTED;

            return kGeneralErr;
        }
    }

    return kNoErr;
}

void wake_up_interrupt_notify( void )
{
    wake_up_interrupt_triggered = true;
}

static unsigned long stop_mode_power_down_hook( unsigned long delay_ms )
{
    unsigned long retval;
    unsigned long wut_ticks_passed;
    unsigned long scale_factor = 0;
    UNUSED_PARAMETER(delay_ms);
    UNUSED_PARAMETER(rtc_timeout_start_time);
    UNUSED_PARAMETER(scale_factor);

   /* pick up the appropriate prescaler for a requested delay */
    select_wut_prescaler_calculate_wakeup_time(&rtc_timeout_start_time, delay_ms, &scale_factor );

    if ( ( ( SCB->SCR & (unsigned long)SCB_SCR_SLEEPDEEP_Msk) ) != 0 )
    {
        DISABLE_INTERRUPTS;

        SysTick->CTRL &= (~(SysTick_CTRL_TICKINT_Msk|SysTick_CTRL_ENABLE_Msk)); /* systick IRQ off */
        RTC_ITConfig(RTC_IT_WUT, ENABLE);

        EXTI_ClearITPendingBit( RTC_INTERRUPT_EXTI_LINE );
        PWR_ClearFlag(PWR_FLAG_WU);
        RTC_ClearFlag(RTC_FLAG_WUTF);

        RTC_SetWakeUpCounter( rtc_timeout_start_time );
        RTC_WakeUpCmd( ENABLE );
        rtc_sleep_entry();

        DBGMCU->CR |= 0x03; /* Enable debug in stop mode */

        /* This code will be running with BASEPRI register value set to 0, the main intention behind that is that */
        /* all interrupts must be allowed to wake the CPU from the power-down mode */
        /* the PRIMASK is set to 1( see DISABLE_INTERRUPTS), thus we disable all interrupts before entering the power-down mode */
        /* This may sound contradictory, however according to the ARM CM3 documentation power-management unit */
        /* takes into account only the contents of the BASEPRI register and it is an external from the CPU core unit */
        /* PRIMASK register value doesn't affect its operation. */
        /* So, if the interrupt has been triggered just before the wfi instruction */
        /* it remains pending and wfi instruction will be treated as a nop  */
        __asm("wfi");

        /* After CPU exits powerdown mode, the processer will not execute the interrupt handler(PRIMASK is set to 1) */
        /* Disable rtc for now */
        RTC_WakeUpCmd( DISABLE );
        RTC_ITConfig(RTC_IT_WUT, DISABLE);

        /* Initialise the clocks again */
        init_clocks( );

        /* Enable CPU ticks */
        SysTick->CTRL |= (SysTick_CTRL_TICKINT_Msk|SysTick_CTRL_ENABLE_Msk);

        /* Get the time of how long the sleep lasted */
        wut_ticks_passed = rtc_timeout_start_time - RTC_GetWakeUpCounter();
        UNUSED_VARIABLE(wut_ticks_passed);
        rtc_sleep_exit( delay_ms, &retval );
        /* as soon as interrupts are enabled, we will go and execute the interrupt handler */
        /* which triggered a wake up event */
        ENABLE_INTERRUPTS;
        wake_up_interrupt_triggered = false;
        return retval;
    }
    else
    {
        UNUSED_PARAMETER(wut_ticks_passed);
        ENABLE_INTERRUPTS;
        __asm("wfi");

        /* Note: We return 0 ticks passed because system tick is still going when wfi instruction gets executed */
        return 0;
    }
}

#else /* WICED_DISABLE_MCU_POWERSAVE */

static unsigned long idle_power_down_hook( unsigned long delay_ms  )
{
    UNUSED_PARAMETER( delay_ms );
    ENABLE_INTERRUPTS;
    __asm("wfi");
    return 0;
}

#endif /* WICED_DISABLE_MCU_POWERSAVE */


unsigned long platform_power_down_hook( unsigned long delay_ms )
{
#ifndef WICED_DISABLE_MCU_POWERSAVE
    return stop_mode_power_down_hook( delay_ms );
#else
    return idle_power_down_hook( delay_ms );
#endif
}

void RTC_WKUP_irq( void )
{
    EXTI_ClearITPendingBit( RTC_INTERRUPT_EXTI_LINE );
}


void wiced_platform_mcu_enable_powersave( void )
{
    MCU_CLOCKS_NOT_NEEDED();
}

void wiced_platform_mcu_disable_powersave( void )
{
    MCU_CLOCKS_NEEDED();
}

void platform_idle_hook( void )
{
    __asm("wfi");
}


void host_platform_reset_wifi( bool reset_asserted )
{
    if ( reset_asserted == true )
    {
        GPIO_ResetBits( WL_RESET_BANK, WL_RESET_PIN );
    }
    else
    {
        GPIO_SetBits( WL_RESET_BANK, WL_RESET_PIN );
    }
}

void host_platform_power_wifi( bool power_enabled )
{
    if ( power_enabled == true )
    {
        GPIO_ResetBits( WL_REG_ON_BANK, WL_REG_ON_PIN );
    }
    else
    {
        GPIO_SetBits( WL_REG_ON_BANK, WL_REG_ON_PIN );
    }
}


void platform_stdio_write( const char* str, uint32_t len )
{
#ifndef MICO_DISABLE_STDIO
    mico_rtos_lock_mutex( &stdio_tx_mutex );
    PlatformUartSend( STDIO_UART, (void*)str, len );
    mico_rtos_unlock_mutex( &stdio_tx_mutex );
#else
    UNUSED_PARAMETER( str );
    UNUSED_PARAMETER( len );
#endif
}

void platform_stdio_read( char* str, uint32_t len )
{
#ifndef MICO_DISABLE_STDIO
    mico_rtos_lock_mutex( &stdio_tx_mutex );
    PlatformUartRecv( STDIO_UART, (void*)str, len, MICO_NEVER_TIMEOUT );
    mico_rtos_unlock_mutex( &stdio_tx_mutex );
#else
    UNUSED_PARAMETER( str );
    UNUSED_PARAMETER( len );
#endif
}

