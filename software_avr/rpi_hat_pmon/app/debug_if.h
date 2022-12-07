/*
 * debug_if.h
 *
 * Created: 03/07/2022 9:06:54
 *  Author: David Michaeli / CaribouLabs LTD
 */ 


#ifndef DEBUG_IF_H_
#define DEBUG_IF_H_

#include <atmel_start.h>
#include "common_data_types.h"

#pragma pack(1)
#define DEBUG_IF_SYNCWORD_B0		((uint32_t)0xDE)
#define DEBUG_IF_SYNCWORD_B1		((uint32_t)0xAD)
#define DEBUG_IF_SYNCWORD_B2		((uint32_t)0xBE)
#define DEBUG_IF_SYNCWORD_B3		((uint32_t)0xEF)
#define DEBUG_IF_SYNCWORD			((uint32_t)(DEBUG_IF_SYNCWORD_B3<<24 | DEBUG_IF_SYNCWORD_B2<<16 | DEBUG_IF_SYNCWORD_B1<<8 | DEBUG_IF_SYNCWORD_B0))
#define DEBUG_IF_MAX_PAYLOAD_LEN	32

typedef enum
{
	OPCODE_PING = 0,
	OPCODE_RESERVED_CODES_MAX = 10,
	OPCODE_VERSION = 11,
	OPCODE_STATE_REPORT = 12,
	OPCODE_COMMAND = 13,
} OPCODE_en;

typedef struct  
{
	uint32_t sync_word;
	uint8_t opcode;
	uint8_t length;
} debug_if_message_st;
#pragma pack()

typedef void (*debug_if_callback_t)(OPCODE_en, void* data, uint32_t len, void* context);

typedef enum
{
	debug_if_msg_state_header_s1,
	debug_if_msg_state_header_s2,
	debug_if_msg_state_header_s3,
	debug_if_msg_state_header_s4,
	debug_if_msg_state_header_oc,
	debug_if_msg_state_header_len,
	debug_if_msg_state_payload,
	debug_if_msg_state_checksum,
} debug_if_msg_state_en;

typedef struct  
{
	debug_if_callback_t cb;
	void* context;
	
	debug_if_message_st rx_hdr;
	uint8_t rx_payload[DEBUG_IF_MAX_PAYLOAD_LEN];
	uint8_t rx_payload_cnt;
	uint8_t rx_chs;
	debug_if_msg_state_en rx_state;

	bool initialized;
} debug_if_st;

void debug_if_init(debug_if_callback_t cb_func, void* context);
void debug_if_send_message(uint8_t opcode, uint8_t* buff, uint32_t len);

// common internal commands
#define debug_if_send_ping			debug_if_send_message(OPCODE_PING, NULL, 0);

#endif /* DEBUG_IF_H_ */