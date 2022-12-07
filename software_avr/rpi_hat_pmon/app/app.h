/*
 * app.h
 *
 * Created: 02/07/2022 21:59:30
 *  Author: David Michaeli / CaribouLabs LTD
 */ 

#ifndef APP_H_
#define APP_H_

#include "analog_sense.h"
#include "leds.h"
#include "debug_if.h"
#include "recorder.h"
#include "timer.h"
#include "sigrow.h"
#include "extint_events.h"
#include "i2c_if.h"

// =============================================================
// PARAMETERS
#define APP_I2C_ADDRESS		(0x25)

typedef void (*timed_task_callback_t)(void* context);
typedef struct  
{
	uint32_t period;
	uint32_t cur_counter;
	bool trigger;
	timed_task_callback_t cb;
	void* context;
} system_timed_task_st;

typedef struct  
{
	versions_st versions;
	analog_sense_st sensors;
	leds_st leds;
	i2c_if_st i2c;
		
	// state
	bool load_switch_on;
	state_st state_data;
	
	// timed tasks
	system_timed_task_st send_report_task;
} app_st;

// MAIN FUNCTIONALITY
void app(void);
void sys_main_loop(void);

// TIMED TASK REGIRTSATION
void timed_task_register(system_timed_task_st* task, uint32_t period, timed_task_callback_t cb, void* context);
void timed_task_tick(system_timed_task_st* task);
void timed_task_fetch(system_timed_task_st* task);

#endif /* APP_H_ */