/*
 * app.c
 *
 * Created: 02/07/2022 21:59:44
 *  Author: David Michaeli / CaribouLabs LTD
 */ 

#include "app.h"
#include <atomic.h>

app_st sys = 
{
	.versions = {.version = SOFTWARE_VERSION, .subversion = SOFTWARE_SUBVERSION,},
};	

//=========================================================================================
void register_timed_task(system_timed_task_st* task, uint32_t period, timed_task_callback_t cb, void* context)
{
	task->cb = cb;
	task->context = context;
	task->trigger = false;
	task->cur_counter = 0;
	task->period = period;
}

//=========================================================================================
void timed_task_tick(system_timed_task_st* task)
{
	task->cur_counter ++;
	if (task->cur_counter >= task->period)
	{
		task->cur_counter = 0;
		task->trigger = true;
	}
}

//=========================================================================================
void timed_task_fetch(system_timed_task_st* task)
{
	if (task->trigger)
	{
		task->trigger = false;
		if (task->cb) task->cb(task->context);
	}
}

//=========================================================================================
void debug_if_callback(OPCODE_en opcode, void* data, uint32_t len, void* context)
{
	app_st* s = (app_st*)context;
	
	switch(opcode)
	{
		//---------------------------------------------------------------------
		case OPCODE_VERSION:
		{
			debug_if_send_message(OPCODE_VERSION, (uint8_t*)&s->versions, sizeof(versions_st));
		}
		break;
		
		// commands
		//---------------------------------------------------------------------
		case OPCODE_COMMAND:
		{
			command_st *msg = (command_st*)data;
			
			// led states	
			if (msg->led0_state != 255) leds_set_blink(&s->leds, led0, msg->led0_state*10, 1, 0);
			if (msg->led1_state != 255) leds_set_blink(&s->leds, led0, msg->led1_state*10, 1, 0);
			
			// load switch with negative logic
			s->load_switch_on = (bool)(msg->load_switch_state == 1);
			LOAD_SWITCH_EN_set_level(!s->load_switch_on);
		}
		break;

		//---------------------------------------------------------------------
		default: break;
	}
}

//=========================================================================================
void load_switch_fault_event(void *pin, bool new_val, uint8_t details, void* context)
{
	app_st* s = (app_st*)context;
	
	// negative logic on the button pressing
	s->state_data.load_sw_fault_state = (new_val == 0);
	i2c_if_update_reg(&s->i2c, I2C_REG_FAULT_STATE, (int32_t)(s->state_data.load_sw_fault_state), I2C_IF_UPDATE_SRC_APP);
}

//=========================================================================================
void send_sensor_report_handler(void* context)
{
	app_st* s = (app_st*)context;

	// send the report
	debug_if_send_message(OPCODE_STATE_REPORT, (uint8_t*)&s->state_data, sizeof(state_st));
}

//=========================================================================================
void timer_callback(uint32_t time, void* context)
{
	app_st* s = (app_st*)context;
	
	// tick the leds
	leds_tick(&s->leds);
	
	// scheduling
	timed_task_tick(&s->send_report_task);
}

//=========================================================================================
void temperature_callback(void* p, void* context)
{
	analog_value_st* point = (analog_value_st*)p;
	app_st* s = (app_st*)context;
	s->state_data.sys_temp = point->value;
}

//=========================================================================================
void voltage_callback(void* p, void* context)
{
	analog_value_st* point = (analog_value_st*)p;
	app_st* s = (app_st*)context;
	s->state_data.voltage = point->value;
	s->state_data.power = s->state_data.voltage * s->state_data.current;
	i2c_if_update_reg(&s->i2c, I2C_REG_VOLTAGE, (int32_t)(s->state_data.voltage), I2C_IF_UPDATE_SRC_APP);
	i2c_if_update_reg(&s->i2c, I2C_REG_POWER, (int32_t)(s->state_data.power), I2C_IF_UPDATE_SRC_APP);
}

//=========================================================================================
void current_callback(void* p, void* context)
{
	analog_value_st* point = (analog_value_st*)p;
	app_st* s = (app_st*)context;
	s->state_data.current = point->value;
	s->state_data.power = s->state_data.voltage * s->state_data.current;
	i2c_if_update_reg(&s->i2c, I2C_REG_CURRENT, (int32_t)(s->state_data.current), I2C_IF_UPDATE_SRC_APP);
	i2c_if_update_reg(&s->i2c, I2C_REG_POWER, (int32_t)(s->state_data.power), I2C_IF_UPDATE_SRC_APP);
}

//=========================================================================================
void i2c_if_error_callback(I2C_IF_ERROR_en err, void* context)
{
	//app_st* s = (app_st*)context;
}

//=========================================================================================
void i2c_if_data_callback(I2C_IF_REG_en reg, uint8_t val, void* context)
{
	app_st* s = (app_st*)context;
	switch(reg)
	{
		case I2C_REG_LOAD_SW_STATE:
			// negative logic
			LOAD_SWITCH_EN_set_level(val == 0);
			break;
		
		case I2C_REG_LED1_STATE:
			leds_set_state(&s->leds, led0, val != 0);
			break;
		
		case I2C_REG_LED2_STATE:
			leds_set_state(&s->leds, led1, val != 0);
		break;
		
		default: break;
	}
}

//=========================================================================================
void app(void)
{
	// Get Sigrow
	init_system_data(&sys.versions.mcu_type, sys.versions.uuid);
	
	// initialize the LEDs
	leds_init(&sys.leds, true);

	// debug interface
	debug_if_init(debug_if_callback, &sys);
	debug_if_send_ping;

	// initialize the periodic timer
	timer_init(10, timer_callback, &sys);
	
	// external interrupt low level events
	extint_register(extint_pin_load_sw_fault, load_switch_fault_event, &sys);

	// initialize analog sensors
	analog_sense_init();
	analog_sense_points_st* sens_points = analog_sense_get_points();
	analog_read_register(&sens_points->temperature, temperature_callback, &sys);
	analog_read_register(&sens_points->voltage, voltage_callback, &sys);
	analog_read_register(&sens_points->current, current_callback, &sys);
	
	// timed tasks registration
	register_timed_task(&sys.send_report_task, timer_get_ticks_from_millisec(250), send_sensor_report_handler, &sys);
	
	// i2c slave
	i2c_if_init(&sys.i2c, APP_I2C_ADDRESS, i2c_if_error_callback, &sys);
	i2c_if_set_reg_cb(&sys.i2c, I2C_REG_LOAD_SW_STATE, i2c_if_data_callback);
	i2c_if_set_reg_cb(&sys.i2c, I2C_REG_LED1_STATE, i2c_if_data_callback);
	i2c_if_set_reg_cb(&sys.i2c, I2C_REG_LED2_STATE, i2c_if_data_callback);
	
	// initial states of IOs
	sys.state_data.load_sw_state = !LOAD_SWITCH_EN_get_level();
	sys.state_data.load_sw_fault_state = !LOAD_SWITCH_FAULT_get_level();
	
	// main loop
	sys_main_loop();
}

//=========================================================================================
void sys_main_loop(void)
{
	while (true)
	{
		timed_task_fetch(&sys.send_report_task);
	}
}