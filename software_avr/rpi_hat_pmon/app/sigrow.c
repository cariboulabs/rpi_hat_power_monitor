/*
 * sigrow.c
 *
 * Created: 03/07/2022 10:46:14
 *  Author: David Michaeli / CaribouLabs LTD
 */ 

#include <atmel_start.h>
#include "sigrow.h"

void init_system_data(uint32_t* mcu_type, uint8_t *uuid)
{
	uint32_t mcu_t = 0;
	mcu_t = SIGROW.DEVICEID0; mcu_t <<= 8;
	mcu_t |= SIGROW.DEVICEID1;  mcu_t <<= 8;
	mcu_t |= SIGROW.DEVICEID2;
	if (mcu_type) *mcu_type = mcu_t;
	
	if (uuid)
	{
		uuid[0] = SIGROW.SERNUM0;
		uuid[1] = SIGROW.SERNUM1;
		uuid[2] = SIGROW.SERNUM2;
		uuid[3] = SIGROW.SERNUM3;
		uuid[4] = SIGROW.SERNUM4;
		uuid[5] = SIGROW.SERNUM5;
		uuid[6] = SIGROW.SERNUM6;
		uuid[7] = SIGROW.SERNUM7;
		uuid[8] = SIGROW.SERNUM8;
		uuid[9] = SIGROW.SERNUM9;
		uuid[10] = SIGROW.SERNUM10;
		uuid[11] = SIGROW.SERNUM11;
		uuid[12] = SIGROW.SERNUM12;
		uuid[13] = SIGROW.SERNUM13;
		uuid[14] = SIGROW.SERNUM14;
		uuid[15] = SIGROW.SERNUM15;
	}
}
