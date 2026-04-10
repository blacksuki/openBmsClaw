#ifndef OPEN_BMS_CLAW_DRIVERS_ADC_ADC_PROBE_H
#define OPEN_BMS_CLAW_DRIVERS_ADC_ADC_PROBE_H

#include <stdint.h>

void adc_probe_init(void);
uint16_t adc_probe_read_once(void);
uint16_t adc_probe_read_average(uint32_t sample_count);

#endif
