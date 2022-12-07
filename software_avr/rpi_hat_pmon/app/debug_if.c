/*
 * debug_if.c
 *
 * Created: 03/07/2022 9:06:54
 *  Author: David Michaeli / CaribouLabs LTD
 */ 

#include "debug_if.h"

static debug_if_st debug_if = 
{
	.cb = NULL,
	.context = NULL,
	.initialized = false,
	.rx_payload_cnt = 0,
	.rx_chs = 0,
	.rx_state = debug_if_msg_state_header_s1,
};

//=========================================================================================
void debug_if_internal_handler(void)
{
	switch (debug_if.rx_hdr.opcode)
	{
		case OPCODE_PING: 
			debug_if_send_ping;
			break;
				
		default: break;
	}
}

//=========================================================================================
void debug_if_rx_isr_cb(void)
{
	uint8_t data = USART_0_get_data();

	switch(debug_if.rx_state)
	{
		case debug_if_msg_state_header_s1:
			if (data == DEBUG_IF_SYNCWORD_B0 ) debug_if.rx_state = debug_if_msg_state_header_s2; 
			else debug_if.rx_state = debug_if_msg_state_header_s1; 
			break;
		case debug_if_msg_state_header_s2:
			if (data == DEBUG_IF_SYNCWORD_B1 ) debug_if.rx_state = debug_if_msg_state_header_s3;
			else debug_if.rx_state = debug_if_msg_state_header_s1;
			break;
		case debug_if_msg_state_header_s3:
			if (data == DEBUG_IF_SYNCWORD_B2 ) debug_if.rx_state = debug_if_msg_state_header_s4;
			else debug_if.rx_state = debug_if_msg_state_header_s1;
			break;
		case debug_if_msg_state_header_s4:
			if (data == DEBUG_IF_SYNCWORD_B3 ) debug_if.rx_state = debug_if_msg_state_header_oc;
			else debug_if.rx_state = debug_if_msg_state_header_s1;
			break;
		case debug_if_msg_state_header_oc: 
			debug_if.rx_hdr.opcode = data;
			debug_if.rx_state = debug_if_msg_state_header_len;
			break;
		case debug_if_msg_state_header_len:
			if (data > DEBUG_IF_MAX_PAYLOAD_LEN)
			{
				debug_if.rx_state = debug_if_msg_state_header_s1;
			}
			else
			{
				debug_if.rx_hdr.length = data;
				debug_if.rx_payload_cnt = 0;
				if (debug_if.rx_hdr.length == 0)
				{
					debug_if.rx_state = debug_if_msg_state_checksum;
				}
				else debug_if.rx_state = debug_if_msg_state_payload;	
			}
			break;
		
		case debug_if_msg_state_payload:
			if (debug_if.rx_payload_cnt >= (debug_if.rx_hdr.length-1))
			{
				debug_if.rx_state = debug_if_msg_state_checksum;	
			}
			debug_if.rx_payload[debug_if.rx_payload_cnt++] = data;
			
			break;
			
		case debug_if_msg_state_checksum:
			debug_if.rx_state = debug_if_msg_state_header_s1;
			
			if (debug_if.rx_chs != data)
			{
			}
			else
			{
				if (debug_if.rx_hdr.opcode < OPCODE_RESERVED_CODES_MAX)
				{
					debug_if_internal_handler();
				}
				else if (debug_if.cb != NULL)
				{
					debug_if.cb(debug_if.rx_hdr.opcode, debug_if.rx_payload, debug_if.rx_payload_cnt, debug_if.context);
				}
			}
			
			break;
			
		default: break;
	}
	
	if (debug_if.rx_state == debug_if_msg_state_header_s1) debug_if.rx_chs = 0;
	else debug_if.rx_chs += data;
}

//=========================================================================================
void debug_if_init(debug_if_callback_t cb_func, void* context)
{
	debug_if.cb = cb_func;
	debug_if.context = context;
	USART_0_set_ISR_cb(debug_if_rx_isr_cb, RX_CB);
	debug_if.initialized = true;
}

//=========================================================================================
void debug_if_send_message(uint8_t opcode, uint8_t* buff, uint32_t len)
{
	debug_if_message_st hdr = 
	{
		.sync_word = DEBUG_IF_SYNCWORD,
		.opcode = opcode,
		.length = len,
	};
	uint8_t chs = 0;
	uint8_t *hdr_buf = (uint8_t*)&hdr;
	
	// header
	for (int i = 0; i < (int)sizeof(hdr); i++)
	{
		chs += hdr_buf[i];
		USART_0_write(hdr_buf[i]);
	}
	
	// payload
	for (int i = 0; i < (int)len; i++)
	{
		chs += buff[i];
		USART_0_write(buff[i]);
	}
	
	// checksum
	USART_0_write(chs);
}

