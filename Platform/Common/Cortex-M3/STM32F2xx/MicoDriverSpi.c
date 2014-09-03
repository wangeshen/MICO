
#include "MICORTOS.h"
#include "MICOPlatform.h"

#include "EMW3162/platform.h"
#include "EMW3162/platform_common_config.h"
#include "stm32f2xx_platform.h"
#include "stm32f2xx.h"

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

static wiced_result_t wiced_spi_configure_baudrate( uint32_t speed, uint16_t* prescaler );
static wiced_result_t spi_dma_transfer            ( const wiced_spi_device_t* spi );
static void           spi_dma_config              ( const wiced_spi_device_t* spi, wiced_spi_message_segment_t* message );

/******************************************************
 *               Function Definitions
 ******************************************************/

static OSStatus wiced_spi_configure_baudrate( uint32_t speed, uint16_t* prescaler )
{
    uint8_t i;

    wiced_assert("Bad args", prescaler != NULL);

    for( i = 0 ; i < MAX_NUM_SPI_PRESCALERS ; i++ )
    {
        if( ( 60000000 / spi_baudrate_prescalers[i].factor ) <= speed )
        {
            *prescaler = spi_baudrate_prescalers[i].prescaler_value;
            return WICED_SUCCESS;
        }
    }

    return WICED_ERROR;
}

static OSStatus spi_dma_transfer( const wiced_spi_device_t* spi )
{
    uint32_t loop_count;

    /* Enable dma channels that have just been configured */
    DMA_Cmd(spi_mapping[spi->port].rx_dma_stream, ENABLE);
    DMA_Cmd(spi_mapping[spi->port].tx_dma_stream, ENABLE);

    /* Wait for DMA to complete */
    /* TODO: This should wait on a semaphore that is triggered from an IRQ */
    loop_count = 0;
    while ( ( DMA_GetFlagStatus( spi_mapping[spi->port].rx_dma_stream, spi_transfer_complete_flags[ spi_mapping[spi->port].rx_dma_stream_number ] ) == RESET ) )
    {
        loop_count++;
        /* Check if we've run out of time */
        if ( loop_count >= (uint32_t) SPI_DMA_TIMEOUT_LOOPS )
        {
            MicoGpioOutputHigh(spi->chip_select);
            return WICED_TIMEOUT;
        }
    }

    MicoGpioOutputHigh(spi->chip_select);
    return WICED_SUCCESS;
}

static void spi_dma_config( const wiced_spi_device_t* spi, wiced_spi_message_segment_t* message )
{
    DMA_InitTypeDef dma_init;
    uint8_t         dummy = 0xFF;

    wiced_assert("Bad args", (spi != NULL) && (message != NULL))

    /* Setup DMA for SPI TX if it is enabled */
    DMA_DeInit( spi_mapping[spi->port].tx_dma_stream );

    /* Setup DMA stream for TX */
    dma_init.DMA_Channel            = spi_mapping[spi->port].tx_dma_channel;
    dma_init.DMA_PeripheralBaseAddr = ( uint32_t )&spi_mapping[spi->port].spi_regs->DR;
    dma_init.DMA_DIR                = DMA_DIR_MemoryToPeripheral;
    dma_init.DMA_PeripheralInc      = DMA_PeripheralInc_Disable;
    dma_init.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    dma_init.DMA_BufferSize         = message->length;
    dma_init.DMA_MemoryDataSize     = DMA_MemoryDataSize_Byte;
    dma_init.DMA_Mode               = DMA_Mode_Normal;
    dma_init.DMA_Priority           = DMA_Priority_VeryHigh;
    dma_init.DMA_FIFOMode           = DMA_FIFOMode_Disable;
    dma_init.DMA_FIFOThreshold      = DMA_FIFOThreshold_Full;
    dma_init.DMA_MemoryBurst        = DMA_MemoryBurst_Single;
    dma_init.DMA_PeripheralBurst    = DMA_PeripheralBurst_Single;

    if ( message->tx_buffer != NULL )
    {
       dma_init.DMA_Memory0BaseAddr = ( uint32_t )message->tx_buffer;
       dma_init.DMA_MemoryInc       = DMA_MemoryInc_Enable;
    }
    else
    {
       dma_init.DMA_Memory0BaseAddr = ( uint32_t )(&dummy);
       dma_init.DMA_MemoryInc       = DMA_MemoryInc_Disable;
    }

    DMA_Init( spi_mapping[spi->port].tx_dma_stream, &dma_init );

    /* Activate SPI DMA mode for transmission */
    SPI_I2S_DMACmd( spi_mapping[spi->port].spi_regs, SPI_I2S_DMAReq_Tx, ENABLE );

    /* TODO: Init TX DMA finished semaphore  */

    /* Setup DMA for SPI RX stream */
    DMA_DeInit( spi_mapping[spi->port].rx_dma_stream );
    dma_init.DMA_Channel            = spi_mapping[spi->port].rx_dma_channel;
    dma_init.DMA_PeripheralBaseAddr = ( uint32_t )&spi_mapping[spi->port].spi_regs->DR;
    dma_init.DMA_DIR                = DMA_DIR_PeripheralToMemory;
    dma_init.DMA_PeripheralInc      = DMA_PeripheralInc_Disable;
    dma_init.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    dma_init.DMA_BufferSize         = message->length;
    dma_init.DMA_MemoryDataSize     = DMA_MemoryDataSize_Byte;
    dma_init.DMA_Mode               = DMA_Mode_Normal;
    dma_init.DMA_Priority           = DMA_Priority_VeryHigh;
    dma_init.DMA_FIFOMode           = DMA_FIFOMode_Disable;
    dma_init.DMA_FIFOThreshold      = DMA_FIFOThreshold_Full;
    dma_init.DMA_MemoryBurst        = DMA_MemoryBurst_Single;
    dma_init.DMA_PeripheralBurst    = DMA_PeripheralBurst_Single;
    if ( message->rx_buffer != NULL )
    {
        dma_init.DMA_Memory0BaseAddr = (uint32_t)message->rx_buffer;
        dma_init.DMA_MemoryInc = DMA_MemoryInc_Enable;
    }
    else
    {
        dma_init.DMA_Memory0BaseAddr = (uint32_t)&dummy;
        dma_init.DMA_MemoryInc = DMA_MemoryInc_Disable;
    }

    /* Init and activate RX DMA channel */
    DMA_Init( spi_mapping[spi->port].rx_dma_stream, &dma_init );
    SPI_I2S_DMACmd( spi_mapping[spi->port].spi_regs, SPI_I2S_DMAReq_Rx, ENABLE );

    /* TODO: Init RX DMA finish semaphore */
}

wiced_result_t wiced_spi_init( const wiced_spi_device_t* spi )
{
    GPIO_InitTypeDef gpio_init_structure;
    wiced_result_t   result;
    SPI_InitTypeDef  spi_init;

    wiced_assert("Bad args", spi != NULL);

    MCU_CLOCKS_NEEDED();

    /* Init SPI GPIOs */
    gpio_init_structure.GPIO_Mode  = GPIO_Mode_AF;
    gpio_init_structure.GPIO_OType = GPIO_OType_PP;
    gpio_init_structure.GPIO_Speed = GPIO_Speed_100MHz;
    gpio_init_structure.GPIO_Pin   = ((uint32_t) (1 << spi_mapping[spi->port].pin_clock->number)) |
                                     ((uint32_t) (1 << spi_mapping[spi->port].pin_miso->number )) |
                                     ((uint32_t) (1 << spi_mapping[spi->port].pin_mosi->number ));
    GPIO_Init( spi_mapping[spi->port].pin_clock->bank, &gpio_init_structure );

    /* Init the chip select GPIO */
    MicoGpioInitialize(spi->chip_select, OUTPUT_PUSH_PULL);
    MicoGpioOutputHigh(spi->chip_select);

    GPIO_PinAFConfig( spi_mapping[spi->port].pin_clock->bank, spi_mapping[spi->port].pin_clock->number,  spi_mapping[spi->port].gpio_af );
    GPIO_PinAFConfig( spi_mapping[spi->port].pin_miso->bank,  spi_mapping[spi->port].pin_miso->number,   spi_mapping[spi->port].gpio_af );
    GPIO_PinAFConfig( spi_mapping[spi->port].pin_mosi->bank,  spi_mapping[spi->port].pin_mosi->number,   spi_mapping[spi->port].gpio_af );

    /* Configure baudrate */
    result = wiced_spi_configure_baudrate( spi->speed, &spi_init.SPI_BaudRatePrescaler );
    if ( result != WICED_SUCCESS )
    {
        return result;
    }

    /* Configure data-width */
    if ( spi->bits == 8 )
    {
        spi_init.SPI_DataSize = SPI_DataSize_8b;
    }
    else if ( spi->bits == 16 )
    {
        if ( spi->mode & SPI_USE_DMA )
        {
            /* 16 bit mode is not supported for a DMA */
            return WICED_ERROR;
        }
        spi_init.SPI_DataSize = SPI_DataSize_16b;
    }
    else
    {
        /* Requested mode is not supported */
        return WICED_BADOPTION;
    }

    /* Configure MSB or LSB */
    if ( spi->mode & SPI_MSB_FIRST )
    {
        spi_init.SPI_FirstBit = SPI_FirstBit_MSB;
    }
    else
    {
        spi_init.SPI_FirstBit = SPI_FirstBit_LSB;
    }

    /* Configure mode CPHA and CPOL */
    if ( spi->mode & SPI_CLOCK_IDLE_HIGH )
    {
        spi_init.SPI_CPOL = SPI_CPOL_High;
    }
    else
    {
        spi_init.SPI_CPOL = SPI_CPOL_Low;
    }

    if ( spi->mode & SPI_CLOCK_RISING_EDGE )
    {
        spi_init.SPI_CPHA = ( spi->mode & SPI_CLOCK_IDLE_HIGH )? SPI_CPHA_2Edge : SPI_CPHA_1Edge;
    }
    else
    {
        spi_init.SPI_CPHA = ( spi->mode & SPI_CLOCK_IDLE_HIGH )? SPI_CPHA_1Edge : SPI_CPHA_2Edge;
    }

    spi_init.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    spi_init.SPI_Mode      = SPI_Mode_Master;
    spi_init.SPI_NSS       = SPI_NSS_Soft;
    SPI_CalculateCRC( spi_mapping[spi->port].spi_regs, DISABLE );

    /* Enable SPI peripheral clock */
    spi_mapping[spi->port].peripheral_clock_func( spi_mapping[spi->port].peripheral_clock_reg,  ENABLE );

    /* Init and enable SPI */
    SPI_Init( spi_mapping[spi->port].spi_regs, &spi_init );
    SPI_Cmd ( spi_mapping[spi->port].spi_regs, ENABLE );

    MCU_CLOCKS_NOT_NEEDED();

    current_spi_device = (wiced_spi_device_t*)spi;

    return WICED_SUCCESS;
}

wiced_result_t wiced_spi_transfer( const wiced_spi_device_t* spi, wiced_spi_message_segment_t* segments, uint16_t number_of_segments )
{
    wiced_result_t result = WICED_SUCCESS;
    uint16_t       i;
    uint32_t       count = 0;

    wiced_assert("Bad args", (spi != NULL) && (segments != NULL) && (number_of_segments != 0));

    MCU_CLOCKS_NEEDED();

    /* If the given SPI device is not the current SPI device, initialise */
    if ( spi != current_spi_device )
    {
        wiced_spi_init( spi );
    }

    /* Activate chip select */
    MicoGpioOutputLow(spi->chip_select);

    for ( i = 0; i < number_of_segments; i++ )
    {
        /* Check if we are using DMA */
        if ( spi->mode & SPI_USE_DMA )
        {
            spi_dma_config( spi, &segments[i] );
            result = spi_dma_transfer( spi );
            if ( result != WICED_SUCCESS )
            {
                goto cleanup_transfer;
            }
        }
        else
        {
            /* in interrupt-less mode */
            if ( spi->bits == 8 )
            {
                const uint8_t* send_ptr = ( const uint8_t* )segments[i].tx_buffer;
                uint8_t*       rcv_ptr  = ( uint8_t* )segments[i].rx_buffer;
                count = segments[i].length;
                while ( count-- )
                {
                    uint16_t data;
                    if ( send_ptr != NULL )
                    {
                        data = *send_ptr;
                        send_ptr++;
                    }
                    else
                    {
                        data = 0xFF;
                    }

                    /* Wait until the transmit buffer is empty */
                    while ( SPI_I2S_GetFlagStatus( spi_mapping[spi->port].spi_regs, SPI_I2S_FLAG_TXE ) == RESET )
                    {}

                    /* Send the byte */
                    SPI_I2S_SendData( spi_mapping[spi->port].spi_regs, data );

                    /* Wait until a data is received */
                    while ( SPI_I2S_GetFlagStatus( spi_mapping[spi->port].spi_regs, SPI_I2S_FLAG_RXNE ) == RESET )
                    {}

                    /* Get the received data */
                    data = SPI_I2S_ReceiveData( spi_mapping[spi->port].spi_regs );

                    if ( rcv_ptr != NULL )
                    {
                        *rcv_ptr++ = (uint8_t)data;
                    }
                }
            }
            else if ( spi->bits == 16 )
            {
                const uint16_t* send_ptr = (const uint16_t *) segments[i].tx_buffer;
                uint16_t*       rcv_ptr  = (uint16_t *) segments[i].rx_buffer;

                /* Check that the message length is a multiple of 2 */
                if ( ( count % 2 ) == 0 )
                {
                    result = WICED_ERROR;
                    goto cleanup_transfer;
                }

                while ( count != 0)
                {
                    uint16_t data = 0xFFFF;
                    count -= 2;

                    if ( send_ptr != NULL )
                    {
                        data = *send_ptr++;
                    }

                    /* Wait until the transmit buffer is empty */
                    while ( SPI_I2S_GetFlagStatus( spi_mapping[spi->port].spi_regs, SPI_I2S_FLAG_TXE ) == RESET )
                    {}

                    /* Send the byte */
                    SPI_I2S_SendData( spi_mapping[spi->port].spi_regs, data );

                    /* Wait until a data is received */
                    while ( SPI_I2S_GetFlagStatus( spi_mapping[spi->port].spi_regs, SPI_I2S_FLAG_RXNE ) == RESET )
                    {}

                    /* Get the received data */
                    data = SPI_I2S_ReceiveData( spi_mapping[spi->port].spi_regs );

                    if ( rcv_ptr != NULL )
                    {
                        *rcv_ptr++ = data;
                    }
                }
            }
        }
    }
cleanup_transfer:
    MicoGpioOutputHigh(spi->chip_select);

    MCU_CLOCKS_NOT_NEEDED();

    return result;
}

wiced_result_t wiced_spi_deinit( const wiced_spi_device_t* spi )
{
    GPIO_InitTypeDef gpio_init_structure;

    MCU_CLOCKS_NEEDED();

    /* De-init and disable SPI */
    SPI_Cmd( spi_mapping[ spi->port ].spi_regs, DISABLE );
    SPI_I2S_DeInit( spi_mapping[ spi->port ].spi_regs );

    /* Disable SPI peripheral clock */
    spi_mapping[spi->port].peripheral_clock_func( spi_mapping[spi->port].peripheral_clock_reg,  DISABLE );

    /* Reset all pins to input floating state */
    gpio_init_structure.GPIO_Mode  = GPIO_Mode_IN;      // Input
    gpio_init_structure.GPIO_PuPd  = GPIO_PuPd_NOPULL;  // Floating (No-pull)
    gpio_init_structure.GPIO_OType = GPIO_OType_PP;     // Arbitrary. Only applicable for output
    gpio_init_structure.GPIO_Speed = GPIO_Speed_100MHz;
    gpio_init_structure.GPIO_Pin   = ((uint32_t) (1 << spi_mapping[spi->port].pin_clock->number)) |
                                     ((uint32_t) (1 << spi_mapping[spi->port].pin_miso->number )) |
                                     ((uint32_t) (1 << spi_mapping[spi->port].pin_mosi->number ));

    GPIO_Init( spi_mapping[spi->port].pin_clock->bank, &gpio_init_structure );

    /* Reset CS pin to input floating state */
    MicoGpioInitialize( spi->chip_select, INPUT_HIGH_IMPEDANCE );

    if ( spi == current_spi_device )
    {
        current_spi_device = NULL;
    }

    MCU_CLOCKS_NOT_NEEDED();

    return WICED_SUCCESS;
}




