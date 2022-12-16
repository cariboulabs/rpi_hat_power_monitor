/*
 * analog_sense.c
 *
 * Created: 01/01/2022 15:59:17
 *  Author: David Michaeli / CaribouLabs LTD
 */ 

#include "analog_sense.h"
analog_sense_points_st analog_sensors = 
{
	.temperature = {
		.type = analog_sens_type_temperature,
		.value = 0.0f,
		.bias = 0.0f,
		.conversion_coeff = 1.0f,
		.cb = NULL,
		.context = NULL,
		.adc_channel = 0x42,
		.alpha_filter = 0.2f,
	},
	
	.voltage = {
		.type = analog_sens_type_voltage,
		.value = 0.0f,
		.bias = 0.0f,
		.conversion_coeff = 3.0f,
		.cb = NULL,
		.context = NULL,
		.adc_channel = 0x3,
		.alpha_filter = 0.2f,
	},
	
	.current = {
		// INA181A4IDBVR, 10mOhm, 200 V/V
		// Vin=I*0.01Ohms => Vout = 200*I*0.01=2*I => I = Vout/2
		.type = analog_sens_type_current,
		.value = 0.0f,
		.bias = 0.0f,
		.conversion_coeff = 0.5f,
		.cb = NULL,
		.context = NULL,
		.adc_channel = 0x4,
		.alpha_filter = 0.01f,
	},
};

analog_sense_st analog_sense_dev = 
{
	.sens_points = &analog_sensors,
	.sens_points_values = &analog_sensors.temperature,
	.current_sensor = 0,
};

#define NUM_SENS_POINTS		(sizeof(analog_sensors) / sizeof(analog_value_st))

//=========================================================================================
void analog_sense_adc_handler(void)
{
	analog_value_st* value = &analog_sense_dev.sens_points_values[analog_sense_dev.current_sensor];
	
	float interm_val = 0.0f;
	float raw_val = ADC_0_get_conversion_result();
	
	switch(value->type)
	{
		case analog_sens_type_current:
		case analog_sens_type_voltage:
			raw_val *= 2500.0f / 4096.0f / 4.0f;
			interm_val = (raw_val + value->bias) * value->conversion_coeff;
			break;
		
		case analog_sens_type_temperature:
			{
				uint16_t sigrow_offset = SIGROW.TEMPSENSE1;
				uint16_t sigrow_slope = SIGROW.TEMPSENSE0; // Read unsigned value from signature row
				uint16_t adc_reading = ADC0.RES >> 2;
				uint32_t temp = sigrow_offset - adc_reading;
				temp *= sigrow_slope;
				temp += 0x0800; // Add 4096/2 to get correct rounding on division below
				temp >>= 12; // Round off to nearest degree in Kelvin, by dividing with 2^12 (4096)
				uint16_t temperature_in_K = temp;
		
				interm_val = -273.15f + (float)(temperature_in_K);
			}
			break;
		
		default:
			return;
			break;
	}
	
	value->value = value->value * (1-value->alpha_filter) + interm_val * value->alpha_filter;
	
	analog_sense_dev.current_sensor ++;
	if (analog_sense_dev.current_sensor >= NUM_SENS_POINTS) analog_sense_dev.current_sensor = 0;
	
	ADC_0_start_conversion(analog_sense_dev.sens_points_values[analog_sense_dev.current_sensor].adc_channel);
	if (value->cb != NULL) value->cb(value, value->context);
}

//=========================================================================================
int analog_sense_init(void)
{
	ADC_0_register_callback(analog_sense_adc_handler);
	analog_sense_dev.sens_points = &analog_sensors;
	analog_sense_dev.current_sensor = 0;
	
	VREF.ADC0REF = 0x00 | VREF_REFSEL_2V500_gc;
	
	ADC_0_start_conversion(analog_sense_dev.sens_points_values[analog_sense_dev.current_sensor].adc_channel);
	return 0;
}

//=========================================================================================
void analog_read_register(analog_value_st* sens_point, analog_sens_callback_t cb, void* context)
{
	if (sens_point)
	{
		sens_point->cb = cb;
		sens_point->context = context;
	}
}

//=========================================================================================
float analog_sense_read(analog_value_st* sens_point)
{
	if (sens_point) return sens_point->value;
	return -999999.0;
}

//=========================================================================================
analog_sense_points_st* analog_sense_get_points(void)
{
	return &analog_sensors;
}

