/*
 * buttons.c
 *
 * Created: 01/01/2022 16:01:43
 *  Author: David Michaeli / CaribouLabs LTD
 */

#include "buttons.h"
#include "extint_events.h"

//=========================================================================================
void button_extint_callback_t(void *pin_obj, bool new_val, uint8_t details, void* context)
{
	button_st *button = (button_st*)context;
	extint_pin_details_st *pin = (extint_pin_details_st*)pin_obj;
	uint32_t last_time = button->button_last_event_time;
	uint32_t cur_time = pin->event_time_tick;	
	
	button->state = new_val;
	if (button->button_ev != NULL)
	{
		if (new_val == false)
		{
			// release determine type of release. if long press, first trigger the long press event
			// and only then the regular release trigger
			if ( (cur_time - last_time) > 20000)
			{
				button->button_ev(button->type, button_event_long_pressed, button->button_context);
			}

			button->button_ev(button->type, button_event_released, button->button_context);
		}
		else 
		{
			button->button_ev(button->type, button_event_pressed, button->button_context);
		}
	}
	
	button->button_last_event_time = cur_time;
}

//=========================================================================================
int buttons_init(buttons_st* dev)
{
	extint_register(extint_pin_button, button_extint_callback_t, &dev->button1);	
	dev->global_ticks = 0;
	dev->initialized = true;
	return 0;
}

//=========================================================================================
void buttons_setup(buttons_st* dev, buttons_en b, button_callback_t c, void* context)
{
	button_st* btn = NULL;
	if (b == button_1)
	{
		btn = &dev->button1;
	}
	
	btn->button_ev = c;
	btn->button_context = context;
	btn->type = b;
	btn->state = false;
	btn->button_last_event_time = 0;
}

//=========================================================================================
bool buttons_get_current_state(buttons_st* dev, buttons_en b)
{
	return dev->button1.state;
}

//=========================================================================================
void buttons_tick_time(buttons_st* dev)
{
	dev->global_ticks ++;
}