#ifndef OPEN_BMS_CLAW_DRIVERS_I2C_I2C_PROBE_H
#define OPEN_BMS_CLAW_DRIVERS_I2C_I2C_PROBE_H

#include <stdbool.h>
#include <stdint.h>

bool i2c_probe_init(void);
int i2c_probe_scan(uint8_t *first_found_address);

#endif
