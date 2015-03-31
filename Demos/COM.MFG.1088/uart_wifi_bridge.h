/** @file uart_wifi_bridge.h
 *
 *  Copyright 2008-2012, Marvell International Ltd.
 *  All Rights Reserved
 */

#ifndef _UART_WIFI_BRIDGE_H
#define _UART_WIFI_BRIDGE_H

#define BAUD_RATE 1500000

/** Re-define generic data types for MLAN/MOAL */
/** Signed char (1-byte) */
typedef char t_s8;
/** Unsigned char (1-byte) */
typedef unsigned char t_u8;
/** Signed short (2-bytes) */
typedef short t_s16;
/** Unsigned short (2-bytes) */
typedef unsigned short t_u16;
/** Signed long (4-bytes) */
typedef int t_s32;
/** Unsigned long (4-bytes) */
typedef unsigned int t_u32;
/** Signed long long 8-bytes) */
typedef long long t_s64;
/** Unsigned long long 8-bytes) */
typedef unsigned long long t_u64;
/** Void pointer (4-bytes) */
typedef void t_void;
/** Size type */
typedef t_u32 t_size;
/** Boolean type */
typedef t_u8 t_bool;
/* Configure BUF_LEN : length of the buffer used to send wifi packets */
#define BUF_LEN 1024

#define CRC32_POLY 0x04c11db7
/** Type definition:Protocol */

/** UART start pattern*/
typedef struct _uart_header {
    /** pattern */
	short pattern;
    /** Command length */
	short length;
} uart_header;

#define LABTOOL_PATTERN_HDR_LEN 4
#define CHECKSUM_LEN 4

#define  SDIO_OUTBUF_LEN	2048
#define UART_BUF_SIZE    2048

#define MLAN_TYPE_CMD   1

#define MLAN_PACK_START   __packed
/** Structure packing end */
#define MLAN_PACK_END


typedef struct _uart_cb {	/* uart control block */
	int uart_fd;
	unsigned int crc32_table[256];

	unsigned char uart_buf[UART_BUF_SIZE];	/* uart buffer */

} uart_cb, *puart_cb;

/** Labtool command header */
typedef struct _cmd_header {
    /** Command Type */
	short type;
    /** Command Sub-type */
	short sub_type;
    /** Command length (header+payload) */
	short length;
    /** Command status */
	short status;
    /** reserved */
	int reserved;
} cmd_header;

/** HostCmd_DS_COMMAND */
typedef MLAN_PACK_START struct _HostCmd_DS_COMMAND
{
	/** Command Header : Command */
	t_u16 command;
	/** Command Header : Size */
	t_u16 size;
	/** Command Header : Sequence number */
	t_u16 seq_num;
	/** Command Header : Result */
	t_u16 result;
	/** Command Body */
} MLAN_PACK_END HostCmd_DS_COMMAND;

typedef MLAN_PACK_START struct _SDIOPkt
{
	t_u16 size;
	t_u16 pkttype;
	HostCmd_DS_COMMAND hostcmd;
} MLAN_PACK_END SDIOPkt;



int task_init();
int sd_wifi_init(int type, void *fl);
#endif /** _UART_WIFI_BRIDGE_H */
