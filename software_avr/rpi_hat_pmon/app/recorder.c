/*
 * recorder.c
 *
 * Created: 03/07/2022 9:12:48
 *  Author: David Michaeli / CaribouLabs LTD
 */ 

#include "recorder.h"

//===========================================================================
void recorder_init(recorder_st* rec, uint16_t max_len)
{
	memset(rec->vec, 0, RECORDER_MAX_LEN);
	rec->cur_pos = 0;
	
	if (max_len > RECORDER_MAX_LEN) max_len = RECORDER_MAX_LEN;
	rec->max_len = max_len;
}

//===========================================================================
void recorder_rewind(recorder_st* rec)
{
	rec->cur_pos = 0;	
}

//===========================================================================
uint16_t recorder_get_len(recorder_st* rec)
{
	return rec->cur_pos;	
}

//===========================================================================
recorder_status_en recorder_status(recorder_st* rec)
{
	if (rec->cur_pos == 0) return recorder_status_idle;
	if (rec->cur_pos == RECORDER_MAX_LEN) return recorder_status_finished;
	return recorder_status_running;
}

//===========================================================================
void recorder_rec_val(recorder_st* rec, uint8_t val)
{
	if (rec->cur_pos >= rec->max_len)
		return;
	
	rec->vec[rec->cur_pos++] = val;
}
