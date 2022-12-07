/*
 * buttons.h
 *
 * Created: 01/01/2022 16:01:33
 *  Author: David Michaeli / CaribouLabs LTD
 */ 


#ifndef BUTTONS_H_
#define BUTTONS_H_

#include <atmel_start.h>

typedef enum
{
	button_1 = 0,
} buttons_en;

typedef enum
{
	button_event_pressed = 0,
	button_event_released = 1,
	button_event_long_pressed = 2,
	button_event_double_press = 3,
} button_event_en;

typedef void (*button_callback_t)(buttons_en b, button_event_en ev, void* context);

typedef struct
{
	button_callback_t button_ev;
	void* button_context;
	uint32_t button_last_event_time;
	buttons_en type;
	bool state;
} button_st;

typedef struct  
{
	button_st button1;
	uint32_t global_ticks;
	bool initialized;
} buttons_st;

int buttons_init(buttons_st* dev);
void buttons_setup(buttons_st* dev, buttons_en b, button_callback_t c, void* context);
bool buttons_get_current_state(buttons_st* dev, buttons_en b);
void buttons_tick_time(buttons_st* dev);

#endif /* BUTTONS_H_ */