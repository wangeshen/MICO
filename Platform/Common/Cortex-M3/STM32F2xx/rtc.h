/*
 * Copyright 2014, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */

#include "MicoPlatform.h"
#pragma once

#define MICO_VERIFY_TIME(time, valid) \
    if( (time->sec > 60) || ( time->min > 60 ) || (time->hr > 24) || ( time->date > 31 ) || ( time->month > 12 )) \
    { \
        valid= false; \
    } \
    else \
    { \
        valid= true; \
    }



#ifndef MICO_DISABLE_MCU_POWERSAVE
OSStatus rtc_sleep_entry( void );
OSStatus rtc_sleep_abort( void );
OSStatus rtc_sleep_exit( unsigned long requested_sleep_time, unsigned long *cpu_sleep_time );
#endif /* #ifndef MICO_DISABLE_MCU_POWERSAVE */

OSStatus platform_set_rtc_time(mico_rtc_time_t* time);
