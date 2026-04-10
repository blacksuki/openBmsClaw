#ifndef OPEN_BMS_CLAW_BOARD_BOARD_H
#define OPEN_BMS_CLAW_BOARD_BOARD_H

#include <stdbool.h>
#include <stdint.h>

void board_init(void);
void board_status_led_set(bool on);
void board_status_led_toggle(void);
void board_busy_wait(volatile uint32_t cycles);
void board_uart_write_string(const char *text);
void board_adc_probe_init(void);
uint16_t board_adc_probe_read_once(void);
uint16_t board_adc_probe_read_average(uint32_t sample_count);
bool board_i2c_probe_init(void);
int board_i2c_probe_scan(uint8_t *first_found_address);

const char *board_name(void);
bool board_has_status_led(void);
bool board_has_uart_log(void);
bool board_has_i2c_bus(void);
bool board_has_adc_channel(void);

#endif
