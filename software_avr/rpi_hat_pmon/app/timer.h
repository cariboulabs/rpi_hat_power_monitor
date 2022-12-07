/*
 * timer.h
 *
 * Created: 03/07/2022 9:06:54
 *  Author: David Michaeli / CaribouLabs LTD
 */ 

#ifndef TIMER_H_
#define TIMER_H_
#include <atmel_start.h>

typedef void (*timer_callback_t)(uint32_t time, void* context);

typedef struct  
{
	uint32_t ovf_val;
	uint32_t timer_ovf_till_callback;
	float timer_millis_till_callback;
	uint32_t timer_current_ovf;
	timer_callback_t cb;
	void* cb_context;
	bool initialized;
} timer_st;

void timer_init(uint32_t callback_time, timer_callback_t cb, void* context);
uint32_t timer_get_ticks_from_millisec(uint32_t millisec);
uint64_t timer_get_time_long(void);
uint32_t timer_get_time_short(void);
void timer_event(void);


#endif /* TIMER_H_ */