#!/usr/bin/env python
from smbus import SMBus
import time

class HatPowerMonitor:
    REG_VERSION = 0             # read only,	bits[3:0] = Version, bits[7:0] = Sub-Version
	REG_LOAD_SW_STATE = 1		# read / write,	0x00 = Load switch is off, 0x01 = Load switch is on
	REG_LED1_STATE = 2		    # read / write,	0x00 = off, 0x01 = on
	REG_LED2_STATE = 3		    # read / write,	0x00 = off, 0x01 = on
	REG_FAULT_STATE = 4		    # read only,	0x00 = OK, 0x01 = Fault
	REG_CURRENT = 5			    # read only,	Val = real_current_mA / 5.0, possible currents [0.0 mA .. 1,275 mA]
                                #                     truncated if above 1.275 A, 5 mA resolution
	REG_VOLTAGE = 6			    # read only		Val = real_voltage_mV / 25.0, possible voltages [0.0 mV .. 6375 mV]
                                #                     truncated if above 6.375 V, 25 mV resolution
	REG_POWER = 7				# read only		Val = real_power_mW / 125.0, possible powers [0.0 mW .. 8,128.125 mW]
                                #                     truncated if above 8.128125 Watt, 125 mW resolution

    def __init__(self, bus, address):
        self.bus = bus
        self.address = address
        self.i2cbus = SMBus(self.bus)

        self.version = 0
        self.sub_version = 0
        self.load_switch_state = false
        self.led1_state = false
        self.led2_state = false
        self.fault_state = false
        self.current = 0.0
        self.voltage = 0.0
        self.power = 0.0
        self.read_all_registers()

    def read_all_registers(self):
        v = self.i2cbus.read_byte_data(self.address, REG_VERSION)
        self.version = (v >> 4) & 0xF
        self.sub_version = (v & 0xF)

        self.load_switch_state = self.i2cbus.read_byte_data(self.address, REG_LOAD_SW_STATE) == 1
        self.led1_state = self.i2cbus.read_byte_data(self.address, REG_LED1_STATE) == 1
        self.led2_state = self.i2cbus.read_byte_data(self.address, REG_LED2_STATE) == 1
        self.fault_state = self.i2cbus.read_byte_data(self.address, REG_FAULT_STATE) == 1
        self.current = self.i2cbus.read_byte_data(self.address, REG_CURRENT) * 5.0
        self.voltage = self.i2cbus.read_byte_data(self.address, REG_VOLTAGE) * 25.0
        self.power = self.i2cbus.read_byte_data(self.address, REG_POWER) * 125.0

    def set_load_switch_state(self, state = false):
        self.i2cbus.write_byte_data(self.address, REG_LOAD_SW_STATE, int(state))

    def set_leds_state(self, led1 = false, led2 = false):
        self.i2cbus.write_byte_data(self.address, REG_LED1_STATE, int(led1))
        self.i2cbus.write_byte_data(self.address, REG_LED2_STATE, int(led2))

    def __repr__(self):
        return str(self)

    def __str__(self):
        return "HatPowerMon V{}.{} (SW Ld1 Ld2 Flt)=({}, {}, {}, {}) (I_mA, V_mV, P_mW)=({:.2f}, {:.2f}, {:.2f})".format(
                                                                            self.version, self.subversion,
                                                                            self.load_switch_state,
                                                                            self.led1_state, self.led2_state,
                                                                            self.fault_state,
                                                                            self.current, self.voltage, self.power)

