/*
 * timer.c
 *
 * Created: 03/07/2022 9:06:54
 *  Author: David Michaeli / CaribouLabs LTD
 */ 

#include "timer.h"

#define TIMER_CLOCK			(float)(24e6)					// the basic clock source
#define CLOCK_OVF_CNT			(float)((1UL)<<16)			// an epoch is controlled by a number of overflow periods
#define MILLISEC_TO_OVFS(m)		(uint32_t)(((float)(m))*TIMER_CLOCK / 1000.0f / CLOCK_OVF_CNT)
#define OVFS_TO_MILLISEC(o)		(float)(((float)(o)*1000.0f*CLOCK_OVF_CNT) / TIMER_CLOCK)


timer_st timer = 
{
	.ovf_val = 0,
	.timer_ovf_till_callback = 10,
	.timer_current_ovf = 0,
	.cb = NULL,
	.cb_context = NULL,
	.initialized = false,
};

//=========================================================================================
void timer_init(uint32_t cb_timeout_millisec, timer_callback_t cb, void* context)
{
	timer.ovf_val = 0;
	timer.timer_ovf_till_callback = MILLISEC_TO_OVFS(cb_timeout_millisec);
	timer.timer_millis_till_callback = OVFS_TO_MILLISEC(timer.timer_ovf_till_callback);
	timer.timer_current_ovf = timer.timer_ovf_till_callback - 1;
	timer.cb = cb;
	timer.cb_context = context;
	timer.initialized = true;
}

//=========================================================================================
uint32_t timer_get_ticks_from_millisec(uint32_t millisec)
{
	float n = millisec;
	n /= timer.timer_millis_till_callback;
	if (n < 1.0) n = 1.0;
	return (uint32_t)n;
}

//=========================================================================================
uint64_t timer_get_time_long(void)
{
	uint64_t v = timer.ovf_val;
	v <<= 16;
	v |= TCB0_CNT;
	return v;
}

//=========================================================================================
uint32_t timer_get_time_short(void)
{
	uint32_t v = (timer.ovf_val & 0xFFFF) << 16;
	v |= TCB0_CNT;
	return v;
}

//=========================================================================================
void timer_event(void)
{
	if (!timer.initialized) return;
	timer.ovf_val ++;
	
	if (timer.timer_current_ovf <= 0)
	{
		timer.timer_current_ovf = timer.timer_ovf_till_callback;
		if (timer.cb != NULL)
		{
			timer.cb(timer_get_time_short(), timer.cb_context);
		}
	}
	
	timer.timer_current_ovf --;
}
