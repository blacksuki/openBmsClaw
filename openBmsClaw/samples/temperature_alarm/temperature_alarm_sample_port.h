#ifndef OPEN_BMS_CLAW_SAMPLES_TEMPERATURE_ALARM_SAMPLE_PORT_H
#define OPEN_BMS_CLAW_SAMPLES_TEMPERATURE_ALARM_SAMPLE_PORT_H

#include <stdbool.h>
#include <stdint.h>

void temperature_alarm_sample_port_init(void);
uint16_t temperature_alarm_sample_port_read_adc_average(uint32_t sample_count);
bool temperature_alarm_sample_port_read_digital_threshold(void);
void temperature_alarm_sample_port_set_buzzer(bool on);
void temperature_alarm_sample_port_set_heartbeat_led(bool on);
void temperature_alarm_sample_port_set_alarm_led(bool on);
void temperature_alarm_sample_port_set_warning_aux_led(bool on);

#endif
