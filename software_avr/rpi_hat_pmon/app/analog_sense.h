/*
 * analog_sense.h
 *
 * Created: 01/01/2022 15:59:24
 *  Author: David Michaeli / CaribouLabs LTD
 */ 


#ifndef ANALOG_SENSE_H_
#define ANALOG_SENSE_H_

#include <atmel_start.h>

typedef enum
{
	analog_sens_type_temperature = 0,
	analog_sens_type_voltage = 1,
	analog_sens_type_current = 2,
} analog_sens_type_en;


typedef void (*analog_sens_callback_t)(void *sens_point, void* context);

typedef struct
{
	analog_sens_type_en type;
	float value;
	float bias;
	float conversion_coeff;
	float alpha_filter;
	analog_sens_callback_t cb;
	void* context;
	uint8_t adc_channel;
} analog_value_st;

typedef struct  
{
	analog_value_st temperature;
	analog_value_st voltage;
	analog_value_st current;
} analog_sense_points_st;

typedef struct
{
	analog_sense_points_st *sens_points;
	analog_value_st* sens_points_values;
	uint8_t current_sensor;
} analog_sense_st;

int analog_sense_init(void);
float analog_sense_read(analog_value_st* sens_point);
void analog_read_register(analog_value_st* sens_point, analog_sens_callback_t cb, void* context);
analog_sense_points_st* analog_sense_get_points(void);

#endif /* ANALOG_SENSE_H_ */