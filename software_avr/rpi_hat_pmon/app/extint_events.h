/*
 * extint_events.h
 *
 * Created: 16/05/2022 13:36:11
 *  Author: David Michaeli / CaribouLabs LTD
 */ 


#ifndef EXTINT_EVENTS_H_
#define EXTINT_EVENTS_H_

#include <atmel_start.h>

// the void* pin is actually 'extint_pin_details_st'
typedef void (*extint_callback_t)(void *pin, bool new_val, uint8_t details, void* context);

typedef enum
{	
	extint_sens_none = 0,
	extint_sens_rising = 1,
	extint_sens_falling = 2,
	extint_sens_both = 3,
} extint_sens_en;

typedef struct
{
	uint8_t pin;
	bool pin_value;
	extint_sens_en sens;
	extint_callback_t cb;
	void* context;
	uint32_t event_time_tick;
} extint_pin_details_st;

typedef enum
{
	extint_pin_load_sw_fault = 0,
} extint_pin_en;

typedef struct  
{
	extint_pin_details_st load_sw_fault;
} extint_registry_st;

void extint_event_port_c(void);
void extint_register(extint_pin_en pin_type, extint_callback_t cb, void* context);


#endif /* EXTINT_EVENTS_H_ */
