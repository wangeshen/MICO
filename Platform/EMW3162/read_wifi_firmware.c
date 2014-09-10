#include "Common.h"

#ifndef MIN
#define MIN(a,b) (((a) < (b))?(a):(b))
#endif

const uint8_t *wifi_firmware_image2 = (uint8_t *)0x080C0000;

static uint32_t image_size = 0x40000;

uint32_t platform_get_wifi_image_size(void)
{

    uint32_t *p = (uint32_t *)0x08100000;

    p--;
    while(*p == 0xFFFFFFFF) {
        image_size-= 4;
        p--;
    }
    
    return image_size;
}

uint32_t platform_get_wifi_image(unsigned char*buffer, uint32_t size, uint32_t offset)
{
    uint32_t buffer_size;
    
    buffer_size = MIN(size, (image_size - offset));
    
    memcpy( buffer, &wifi_firmware_image2[offset], buffer_size );
    return buffer_size;
}

