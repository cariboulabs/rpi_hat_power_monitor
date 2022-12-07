/*
 * i2c_if.h
 *
 * Created: 16/08/2022 13:36:11
 *  Author: David Michaeli / CaribouLabs LTD
 */ 


#ifndef I2C_IF_H_
#define I2C_IF_H_

#include "common_data_types.h"

typedef enum
{
	I2C_REG_VERSION = 0,		// read only,		bits[3:0] = Version, bits[7:0] = Sub-Version
	I2C_REG_LOAD_SW_STATE = 1,	// read / write,	0x00 = Load switch is off, 0x01 = Load switch is on
	I2C_REG_LED1_STATE = 2,		// read / write,	0x00 = off, 0x01 = on
	I2C_REG_LED2_STATE = 3,		// read / write,	0x00 = off, 0x01 = on
	I2C_REG_FAULT_STATE = 4,	// read only,		0x00 = OK, 0x01 = Fault
	I2C_REG_CURRENT = 5,		// read only,		Val = real_current_mA / 5.0, possible currents [0.0 mA .. 1,275 mA] truncated if above 1.275 A, 5 mA resolution
	I2C_REG_VOLTAGE = 6,		// read only		Val = real_voltage_mV / 25.0, possible voltages [0.0 mV .. 6375 mV] truncated if above 6.375 V, 25 mV resolution
	I2C_REG_POWER = 7,			// read only		Val = real_power_mW / 125.0, possible powers [0.0 mW .. 8,128.125 mW] truncated if above 8.128125 Watt, 125 mW resolution
	I2C_REGS_MAX = 8,
} I2C_IF_REG_en;

typedef enum
{
	I2C_ERROR_NO_ERROR = 0,
	I2C_ERROR_COLLISION = 1,
	I2C_ERROR_BUS = 2,
} I2C_IF_ERROR_en;

typedef enum
{
	I2C_STATE_IDLE = 0,
	I2C_STATE_SLAVE_ADDR = 1,
	I2C_STATE_DATA_WRITE = 3,
} I2C_IF_BUS_STATE_en;

typedef void (*i2c_if_command_callback_t)(I2C_IF_REG_en reg, uint8_t val, void* context);
typedef void (*i2c_if_error_callback_t)(I2C_IF_ERROR_en err, void* context);

typedef enum
{
	I2C_IF_UPDATE_SRC_APP = 0,
	I2C_IF_UPDATE_SRC_NET = 1,
} I2C_IF_UPDATE_SRC_en;

typedef enum
{
	I2C_IF_ACCESS_READ_ONLY = 0,
	I2C_IF_ACCESS_WRITE_ONLY = 1,
	I2C_IF_ACCESS_READ_WRITE = 2,
} I2C_IF_ACCESS_en;

typedef struct
{
	uint8_t val;
	uint8_t access;
	i2c_if_command_callback_t cb;
} i2c_if_reg_st;

typedef struct  
{
	uint8_t addr;
	i2c_if_reg_st registers[I2C_REGS_MAX];
	i2c_if_error_callback_t err_cb;
	void* context;
	
	// transaction
	I2C_IF_BUS_STATE_en bus_state;
	uint8_t reg_addr;
} i2c_if_st;

void i2c_if_init(i2c_if_st* i2c, uint8_t addr, i2c_if_error_callback_t err_cb, void* context);
void i2c_if_set_reg_cb(i2c_if_st* i2c, I2C_IF_REG_en reg, i2c_if_command_callback_t cb);
void i2c_if_update_reg(i2c_if_st* i2c, I2C_IF_REG_en reg, int32_t val, I2C_IF_UPDATE_SRC_en src);

#endif /* I2C_IF_H_ */