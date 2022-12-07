/*
 * leds.h
 *
 * Created: 01/01/2022 16:01:20
 *  Author: David Michaeli / CaribouLabs LTD
 */ 


#ifndef LEDS_H_
#define LEDS_H_

#include <atmel_start.h>

typedef enum
{
	led0 = 0,
	led1 = 1,
} leds_en;

typedef struct
{
	uint32_t on_time;
	uint32_t total_time;
	uint32_t cur_tick;
} leds_state_st;

typedef struct
{
	leds_state_st led0_state;
	leds_state_st led1_state;
	uint32_t global_tick;
	bool initialized;
} leds_st;

int leds_init(leds_st* dev, bool init_lo_power);
void leds_set_state(leds_st* dev, leds_en led, bool state);
void leds_set_blink(leds_st* dev, leds_en led, uint32_t on_time, uint32_t off_time, uint32_t offs);
void leds_tick(leds_st* dev);

#endif /* LEDS_H_ */