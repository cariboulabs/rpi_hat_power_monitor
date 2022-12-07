/*
 * i2c_if.c
 *
 * Created: 07/12/2022 10:52:12
 *  Author: David Michaeli / CaribouLabs LTD
 */ 

#include <atmel_start.h>
#include <atomic.h>
#include "i2c_if.h"

#define I2C_IF_VERSION_SHORT		(((uint8_t)(SOFTWARE_VERSION) & 0xF << 4) | ((uint8_t)(SOFTWARE_SUBVERSION)) & 0xF)
#define I2C_IF_SHORT_SCALING(v,s)	{(v)/=(s); if ((v)<0) (v)=0; if ((v)>255) (v)=255;}
	
static i2c_if_st* this_i2c = NULL;


void i2c_if_address_handler();
void i2c_if_read_handler();
void i2c_if_write_handler();
void i2c_if_stop_handler();
void i2c_if_collision_error_handler();
void i2c_if_bus_error_handler();

//==============================================================================================
void i2c_if_update_reg(i2c_if_st* i2c, I2C_IF_REG_en reg, int32_t val, I2C_IF_UPDATE_SRC_en src)
{
	i2c_if_reg_st *reg_s = &i2c->registers[I2C_REG_LOAD_SW_STATE];
	switch (reg)
	{
		// --------------------------------------------
		case I2C_REG_VERSION: break;
		
		// --------------------------------------------
		case I2C_REG_LOAD_SW_STATE:
		case I2C_REG_LED1_STATE:
		case I2C_REG_LED2_STATE:
			reg_s->val = (val == 0?0:1);
			
			if (src == I2C_IF_UPDATE_SRC_NET && reg_s->cb != NULL)
			{
				reg_s->cb(reg, reg_s->val, i2c->context);
			}
			break;

		// --------------------------------------------
		case I2C_REG_FAULT_STATE:
			reg_s->val = (val == 0?0:1);
			break;
		
		// --------------------------------------------
		case I2C_REG_CURRENT:
			{
				I2C_IF_SHORT_SCALING(val,5);
				reg_s->val = val;
			}			
			break;
		
		// --------------------------------------------
		case I2C_REG_VOLTAGE: 
			{
				I2C_IF_SHORT_SCALING(val,25);
				reg_s->val = val;
			}
			break;
		
		// --------------------------------------------
		case I2C_REG_POWER: 
			{
				I2C_IF_SHORT_SCALING(val,125);
				reg_s->val = val;
			}
			break;
			
		// --------------------------------------------	
		default: break;
	}
	
}

//==============================================================================================
void i2c_if_init(i2c_if_st* i2c, uint8_t addr, i2c_if_error_callback_t err_cb, void* context)
{
	this_i2c = i2c;
	i2c->addr = addr & 0x7f;
	
	// adapt the address
	TWI0.SADDR = (i2c->addr) << TWI_ADDRMASK_gp	| 0 << TWI_ADDREN_bp;	// Slave Address: addr
				 														// General Call Recognition Enable: disabled
	
	i2c->err_cb = err_cb;
	i2c->context = context;
	
	// create the register file
	i2c->registers[I2C_REG_VERSION].val =		I2C_IF_VERSION_SHORT;		i2c->registers[I2C_REG_VERSION].access = I2C_IF_ACCESS_READ_ONLY;			i2c->registers[I2C_REG_VERSION].cb = NULL;
	i2c->registers[I2C_REG_LOAD_SW_STATE].val = 0x00;						i2c->registers[I2C_REG_LOAD_SW_STATE].access = I2C_IF_ACCESS_READ_WRITE;	i2c->registers[I2C_REG_LOAD_SW_STATE].cb = NULL;
	i2c->registers[I2C_REG_LED1_STATE].val =	0x00;						i2c->registers[I2C_REG_LED1_STATE].access = I2C_IF_ACCESS_READ_WRITE;		i2c->registers[I2C_REG_LED1_STATE].cb = NULL;
	i2c->registers[I2C_REG_LED2_STATE].val =	0x00;						i2c->registers[I2C_REG_LED2_STATE].access = I2C_IF_ACCESS_READ_WRITE;		i2c->registers[I2C_REG_LED2_STATE].cb = NULL;
	i2c->registers[I2C_REG_FAULT_STATE].val =	0x00;						i2c->registers[I2C_REG_FAULT_STATE].access = I2C_IF_ACCESS_READ_ONLY;		i2c->registers[I2C_REG_FAULT_STATE].cb = NULL;
	i2c->registers[I2C_REG_CURRENT].val =		0x00;						i2c->registers[I2C_REG_CURRENT].access = I2C_IF_ACCESS_READ_ONLY;			i2c->registers[I2C_REG_CURRENT].cb = NULL;
	i2c->registers[I2C_REG_VOLTAGE].val =		0x00;						i2c->registers[I2C_REG_VOLTAGE].access = I2C_IF_ACCESS_READ_ONLY;			i2c->registers[I2C_REG_VOLTAGE].cb = NULL;
	i2c->registers[I2C_REG_POWER].val =			0x00;						i2c->registers[I2C_REG_POWER].access = I2C_IF_ACCESS_READ_ONLY;				i2c->registers[I2C_REG_POWER].cb = NULL;

	i2c->bus_state = I2C_STATE_IDLE;
	i2c->reg_addr = I2C_REG_VERSION;
	
	// setup I2C
	I2C_0_set_address_callback(i2c_if_address_handler);
	I2C_0_set_read_callback(i2c_if_read_handler);
	I2C_0_set_write_callback(i2c_if_write_handler);
	I2C_0_set_stop_callback(i2c_if_stop_handler);
	I2C_0_set_collision_callback(i2c_if_collision_error_handler);
	I2C_0_set_bus_error_callback(i2c_if_bus_error_handler);
	I2C_0_open();
}

//==============================================================================================
void i2c_if_set_reg_cb(i2c_if_st* i2c, I2C_IF_REG_en reg, i2c_if_command_callback_t cb)
{
	i2c_if_reg_st *reg_s = &i2c->registers[reg];
	reg_s->cb = cb;
}

//==============================================================================================
void i2c_if_address_handler()
{
	uint8_t slave_addr = I2C_0_read();
	
	if (slave_addr == this_i2c->addr)
	{
		this_i2c->bus_state = I2C_STATE_SLAVE_ADDR;
		I2C_0_send_ack();
	}
}

//==============================================================================================
void i2c_if_read_handler()
{	
	if (this_i2c->reg_addr < I2C_REGS_MAX)
	{
		I2C_0_write(this_i2c->registers[this_i2c->reg_addr].val);
		this_i2c->reg_addr ++;
		if (this_i2c->reg_addr >= I2C_REGS_MAX)
		{
			this_i2c->reg_addr = 0;
		}
	}
}

//==============================================================================================
void i2c_if_write_handler()
{
	if (this_i2c->bus_state == I2C_STATE_SLAVE_ADDR)
	{
		this_i2c->reg_addr = I2C_0_read();
		I2C_0_send_ack();
		this_i2c->bus_state = I2C_STATE_DATA_WRITE;
	}
	else if (this_i2c->bus_state == I2C_STATE_DATA_WRITE)
	{
		if (this_i2c->reg_addr < I2C_REGS_MAX)
		{			
			i2c_if_update_reg(this_i2c, this_i2c->reg_addr, I2C_0_read(), I2C_IF_UPDATE_SRC_NET);
			
			this_i2c->reg_addr ++;
			if (this_i2c->reg_addr == I2C_REGS_MAX)
			{
				this_i2c->reg_addr = 0;
				I2C_0_send_nack();
			}
			else I2C_0_send_ack();
		}
	}
}

//==============================================================================================
void i2c_if_stop_handler()
{
	this_i2c->bus_state = I2C_STATE_IDLE;
}

//==============================================================================================
void i2c_if_collision_error_handler()
{
	if (this_i2c->err_cb)
	{
		this_i2c->err_cb(I2C_ERROR_COLLISION, this_i2c->context);		
	}
	this_i2c->bus_state = I2C_STATE_IDLE;
}

//==============================================================================================
void i2c_if_bus_error_handler()
{
	if (this_i2c->err_cb)
	{
		this_i2c->err_cb(I2C_ERROR_BUS, this_i2c->context);
	}
	this_i2c->bus_state = I2C_STATE_IDLE;
}