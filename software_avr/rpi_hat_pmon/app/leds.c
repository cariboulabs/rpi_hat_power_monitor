/*
 * leds.c
 *
 * Created: 01/01/2022 16:01:11
 *  Author: David Michaeli / CaribouLabs LTD
 */ 

#include "leds.h"
#include <stdbool.h>

//==============================================================================================
// init_lo_power: the low power leds are connected to the DBG_SS1_SCL and DBG_SCK_SDA so they may
// be used as communication rather than LEDs (and then not initialized)
int leds_init(leds_st* dev, bool init_lo_power)
{
	LED1_set_dir(PORT_DIR_OUT);
	leds_set_state(dev, led0, false);
	LED2_set_dir(PORT_DIR_OUT);
	leds_set_state(dev, led1, false);
	
	dev->initialized = true;
	return 0;
}

//==============================================================================================
void leds_set_state(leds_st* dev, leds_en led, bool state)
{
	leds_set_blink(dev, led, state, 0, 0);
}

//==============================================================================================
void leds_set_blink(leds_st* dev, leds_en led, uint32_t on_time, uint32_t off_time, uint32_t offs)
{
	leds_state_st* l_state = (led == led0) ? (&dev->led0_state) : (&dev->led1_state);
	uint32_t total_time = on_time + off_time;
	
	if (l_state->cur_tick > total_time)
	{
		l_state->cur_tick = 0;
	}
	l_state->on_time = on_time;
	l_state->total_time = total_time;
	
	//if (led == led0) LED1_set_level(current_state);
	//else if (led == led1) LED2_set_level(current_state);
}

//==============================================================================================
void leds_tick(leds_st* dev)
{
	dev->global_tick ++;
	
	dev->led0_state.cur_tick ++;
	if (dev->led0_state.cur_tick >= dev->led0_state.total_time)
	{
		dev->led0_state.cur_tick = 0;
	}
	
	dev->led1_state.cur_tick ++;
	if (dev->led1_state.cur_tick >= dev->led1_state.total_time)
	{
		dev->led1_state.cur_tick = 0;
	}
	
	LED1_set_level(dev->led0_state.cur_tick < dev->led0_state.on_time);
	LED2_set_level(dev->led1_state.cur_tick < dev->led1_state.on_time);
}