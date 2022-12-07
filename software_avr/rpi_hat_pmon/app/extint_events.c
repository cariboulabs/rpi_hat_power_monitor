/*
 * extint_events.h
 *
 * Created: 03/07/2022 9:06:54
 *  Author: David Michaeli / CaribouLabs LTD
 */ 

#include "extint_events.h"
#include "timer.h"


extint_registry_st extint = 
{
	.load_sw_fault = {
		// PORT C
		.pin = 1,
		.pin_value = -1,
		.sens = extint_sens_both,
		.cb = NULL,
		.context = NULL,
		.event_time_tick = 0,
	},
};


#define EXTINT_CHECK_PIN(p,f,d,t)	if ((p).sens != extint_sens_none)											\
									{																						\
										bool last_pin_val = (p).pin_value;													\
										bool cur_pin_val = (f);																\
										(p).event_time_tick = (t);															\
										if (((p).sens == extint_sens_both && last_pin_val != cur_pin_val) ||				\
										((p).sens == extint_sens_rising && cur_pin_val > last_pin_val) ||					\
										((p).sens == extint_sens_falling && cur_pin_val < last_pin_val))					\
										{																					\
											if ((p).cb != NULL)																\
											{																				\
												(p).cb(&(p), cur_pin_val, d, (p).context);									\
											}																				\
										}																					\
										(p).pin_value = cur_pin_val;														\
									}


//=========================================================================================
void extint_event_port_c(void)
{
	uint32_t event_tick = timer_get_time_short();

	EXTINT_CHECK_PIN(extint.load_sw_fault,LOAD_SWITCH_FAULT_get_level(),0,event_tick);
}

//=========================================================================================
void extint_register(extint_pin_en pin_type, extint_callback_t cb, void* context)
{
	switch(pin_type)
	{
		case extint_pin_load_sw_fault:
			extint.load_sw_fault.cb = cb;
			extint.load_sw_fault.context = context;	
			break;
		default: break;
	}
}

