/*
 * recorder.h
 *
 * Created: 03/07/2022 9:06:54
 *  Author: David Michaeli / CaribouLabs LTD
 */ 


#ifndef RECORDER_H_
#define RECORDER_H_

#include <stdio.h>
#include <string.h>

#define RECORDER_MAX_LEN 1024

typedef enum
{
	recorder_status_idle = 0,
	recorder_status_running = 1,
	recorder_status_finished = 2,
} recorder_status_en;

typedef struct  
{
	uint8_t vec[RECORDER_MAX_LEN];
	uint16_t cur_pos;
	uint16_t max_len;
} recorder_st;

void recorder_init(recorder_st* rec, uint16_t max_len);
void recorder_rewind(recorder_st* rec);
recorder_status_en recorder_status(recorder_st* rec);
uint16_t recorder_get_len(recorder_st* rec);
void recorder_rec_val(recorder_st* rec, uint8_t val);

#endif /* RECORDER_H_ */