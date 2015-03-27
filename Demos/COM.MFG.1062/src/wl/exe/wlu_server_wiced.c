/*
 * Copyright 2013, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */

/*
 * Wl server for generic RTOS
 */

#include <stdio.h>
#include "typedefs.h"
#include "bcmutils.h"
#include "bcmcdc.h"
#include "wlioctl.h"
#include "wlu.h"
#include "wlu_remote.h"
//#include "internal/SDPCM.h"
#include "wl_drv.h"
//#include "Network/wwd_buffer_interface.h"
//#include "RTOS/wwd_rtos_interface.h"
#include <string.h>

#include "MICO.h"

unsigned short defined_debug = 0; //DEBUG_ERR | DEBUG_INFO;

typedef void * mico_buf_t;
typedef enum sdpcm_command_type_enum
{
    SDPCM_GET = 0x00,
    SDPCM_SET = 0x02
} ;


extern int remote_server_exec(int argc, char **argv, void *ifr);

int
rwl_create_dir(void)
{
	/* not supported */
	return 0;
}

int
remote_shell_execute(char *buf_ptr)
{
	/* not supported */
	UNUSED_PARAMETER(buf_ptr);
	return 0;
}

int
remote_shell_get_resp(char* shell_fname, char* buf_ptr, int msg_len)
{
	/* not supported */
	UNUSED_PARAMETER(shell_fname);
	UNUSED_PARAMETER(buf_ptr);
	UNUSED_PARAMETER(msg_len);
	return 0;
}
int
rwl_write_serial_port(void* hndle, char* write_buf, unsigned long size, unsigned long *numwritten)
{
	/* not invoked for dongle transports */
	UNUSED_PARAMETER(hndle);

    *numwritten = size;
    if (size == 0)
        return SUCCESS;
    
    MicoUartSend( STDIO_UART, (const char*)write_buf, size );
	return SUCCESS;
}

void*
rwl_open_transport(int remote_type, char *port, int ReadTotalTimeout, int debug)
{
	/* not invoked for dongle transports */
	UNUSED_PARAMETER(remote_type);
	UNUSED_PARAMETER(port);
	UNUSED_PARAMETER(ReadTotalTimeout);
	UNUSED_PARAMETER(debug);
	return NULL;
}

int
rwl_close_transport(int remote_type, void* Des)
{
	/* not invoked for dongle transports */
	UNUSED_PARAMETER(remote_type);
	UNUSED_PARAMETER(Des);
	return FAIL;
}

int
rwl_read_serial_port(void* hndle, char* read_buf, uint data_size, uint *numread)
{
	/* not invoked for dongle transports */
	UNUSED_PARAMETER(hndle);
	*numread = data_size;
    MicoUartRecv( STDIO_UART, read_buf, data_size,  MICO_NEVER_TIMEOUT);
	return SUCCESS;
}

void
rwl_sleep(int delay)
{
	msleep( delay );
}

void
rwl_sync_delay(uint noframes)
{
	if (noframes > 1) {
		rwl_sleep(200);
	}
}

int
wl_ioctl(void *wl, int cmd, void *buf, int len, bool set)
{
	int ret = 0;
	mico_buf_t internalPacket;
	mico_buf_t response_buffer;
	UNUSED_PARAMETER(wl);

	// Set Wireless Security Type
	void* ioctl_data = (void*)wiced_get_ioctl_buffer( &internalPacket, len );
	memcpy( ioctl_data, buf, len );
	ret = wiced_send_ioctl( (set==TRUE)?SDPCM_SET:SDPCM_GET, cmd, internalPacket, &response_buffer, 0 );
	if (ret == 0)
	{
		if (set!=TRUE)
		{
			memcpy( buf, (void*)host_buffer_get_current_piece_data_pointer( response_buffer ), host_buffer_get_current_piece_size( response_buffer )  );
		}
		host_buffer_release(response_buffer, 1);
	}
	else if ( ret == -2 )
	{
	    ret = -1;
	}
	return ret; /*( ret == 0)?0:IOCTL_ERROR;*/
}

void app_main( void )
{
	// turn off buffers, so IO occurs immediately
	setvbuf(stdin, NULL, _IONBF, 0);
	setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stderr, NULL, _IONBF, 0);

	int argc = 2;
	char *argv[] = { "", "" };
	/* Main server process for all transport types */
	remote_server_exec(argc, argv, NULL);
}
