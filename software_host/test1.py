from hat_powermon.hat_powermon import HatPowerMonitor

mon = HatPowerMonitor(0, 0x25)
mon.read_all_registers()

print(mon)

mon.set_load_switch_state(True)
mon.read_all_registers()

print(mon)

