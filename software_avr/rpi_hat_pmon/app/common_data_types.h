/*
 * common_data_types.h
 *
 * Created: 01/01/2022 16:01:33
 *  Author: David Michaeli / CaribouLabs LTD
 */ 

#ifndef COMMON_DATA_TYPES_H_
#define COMMON_DATA_TYPES_H_

//----------------------------------------------------
// DEFINITIONS
//----------------------------------------------------
#define SOFTWARE_VERSION		0x01
#define SOFTWARE_SUBVERSION		0x00

//----------------------------------------------------
// INTERFACE MESSAGES
//----------------------------------------------------
#pragma pack(1)

//----------------------------------------------------
// STATUS REPORTS
typedef struct
{
	// analog inputs
	float sys_temp;
	float voltage;
	float current;
	float power;
	uint8_t load_sw_state;
	uint8_t load_sw_fault_state;
} state_st;

typedef struct
{
	uint8_t version;
	uint8_t subversion;
	uint32_t mcu_type;
	uint8_t uuid[16];
} versions_st;

//----------------------------------------------------
// CONTROLS
typedef struct  
{
	uint8_t load_switch_state;	// 1 = on, 0 = off
	uint8_t led0_state;
	uint8_t led1_state;
} command_st;

#pragma pack()

#endif /* COMMON_DATA_TYPES_H_ */