/*
 * Copyright 2013, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */
#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

/************************************************************************
 * Default WICED networking timeouts in milliseconds */
#define WICED_ALLOCATE_PACKET_TIMEOUT             (2000)
#define WICED_TCP_DISCONNECT_TIMEOUT              (3000)
#define WICED_TCP_BIND_TIMEOUT                    (3000)
#define WICED_TCP_SEND_TIMEOUT                    (3000)
#define WICED_TCP_ACCEPT_TIMEOUT                  (3000)
#define WICED_UDP_BIND_TIMEOUT                    (3000)
#define WICED_NTP_REPLY_TIMEOUT                   (5000)
#define WICED_TLS_RECEIVE_TIMEOUT                 (5000)
#define WICED_TLS_TRANSMIT_TIMEOUT                (5000)
#define WICED_DHCP_IP_ADDRESS_RESOLUTION_TIMEOUT (15000)

/************************************************************************
 * WICED TCP Options */
#define WICED_TCP_WINDOW_SIZE           (14*1024)
#define WICED_DEFAULT_TCP_LISTEN_QUEUE_SIZE   (5)

/************************************************************************
 * WICED Join Options */
#define WICED_JOIN_RETRY_ATTEMPTS       3

/************************************************************************
 * WICED TLS Options */
#define WICED_TLS_MAX_RESUMABLE_SESSIONS   4
#define WICED_TLS_DEFAULT_VERIFICATION     (TLS_VERIFICATION_REQUIRED)

/************************************************************************
 * Country code */
#define WICED_DEFAULT_COUNTRY_CODE    WICED_COUNTRY_AUSTRALIA

/************************************************************************
 * Application thread stack size */
#define MICO_DEFAULT_APPLICATION_STACK_SIZE         (6144)

/************************************************************************
 * Soft AP Options */
#define WICED_DEFAULT_SOFT_AP_DTIM_PERIOD       (1)

/************************************************************************
 * Uncomment to disable watchdog. For debugging only */
//#define MICO_DISABLE_WATCHDOG

/************************************************************************
 * Uncomment to disable standard IO, i.e. printf(), etc. */
//#define MICO_DISABLE_STDIO

/************************************************************************
 * Uncomment to disable MCU powersave API functions */
//#define MICO_DISABLE_MCU_POWERSAVE

/************************************************************************
 * Uncomment to enable MCU real time clock */
#define MICO_ENABLE_MCU_RTC

#ifdef __cplusplus
} /*extern "C" */
#endif
