#include "samples/temperature_alarm/temperature_alarm_sample.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "board/board.h"
#include "samples/temperature_alarm/temperature_alarm_sample_config.h"
#include "samples/temperature_alarm/temperature_alarm_sample_port.h"

typedef enum
{
    TEMPERATURE_ALARM_SAMPLE_STATE_NORMAL = 0,
    TEMPERATURE_ALARM_SAMPLE_STATE_WARN,
    TEMPERATURE_ALARM_SAMPLE_STATE_ALARM,
} temperature_alarm_sample_state_t;

static temperature_alarm_sample_state_t s_current_state = TEMPERATURE_ALARM_SAMPLE_STATE_NORMAL;
static bool s_heartbeat_led_on = false;
static bool s_alarm_led_on = false;
static uint32_t s_heartbeat_tick_count = 0u;
static uint32_t s_alarm_tick_count = 0u;
static uint32_t s_log_tick_count = 0u;

static float temperature_alarm_sample_ln(float value)
{
    const int terms = 15;
    float x;
    float x_squared;
    float series;
    int denominator = (terms * 2) + 1;

    x = (value - 1.0f) / (value + 1.0f);
    x_squared = x * x;
    series = 1.0f / (float)denominator;

    for (int index = terms; index > 0; --index)
    {
        (void)index;
        denominator -= 2;
        series = (1.0f / (float)denominator) + (x_squared * series);
    }

    return 2.0f * x * series;
}

static void temperature_alarm_sample_uart_append_char(char *buffer, size_t buffer_size, size_t *length, char value)
{
    if ((*length + 1u) >= buffer_size)
    {
        return;
    }

    buffer[*length] = value;
    *length += 1u;
    buffer[*length] = '\0';
}

static void temperature_alarm_sample_uart_append_string(
    char *buffer,
    size_t buffer_size,
    size_t *length,
    const char *text)
{
    if (text == 0)
    {
        return;
    }

    while (*text != '\0')
    {
        temperature_alarm_sample_uart_append_char(buffer, buffer_size, length, *text);
        ++text;
    }
}

static void temperature_alarm_sample_uart_append_u32(
    char *buffer,
    size_t buffer_size,
    size_t *length,
    uint32_t value)
{
    char digits[10];
    size_t digit_count = 0u;

    if (value == 0u)
    {
        temperature_alarm_sample_uart_append_char(buffer, buffer_size, length, '0');
        return;
    }

    while (value > 0u)
    {
        digits[digit_count] = (char)('0' + (value % 10u));
        value /= 10u;
        ++digit_count;
    }

    while (digit_count > 0u)
    {
        --digit_count;
        temperature_alarm_sample_uart_append_char(buffer, buffer_size, length, digits[digit_count]);
    }
}

static void temperature_alarm_sample_uart_append_temperature_x10(
    char *buffer,
    size_t buffer_size,
    size_t *length,
    int32_t temperature_x10)
{
    uint32_t absolute_value;

    if (temperature_x10 < 0)
    {
        temperature_alarm_sample_uart_append_char(buffer, buffer_size, length, '-');
        absolute_value = (uint32_t)(-temperature_x10);
    }
    else
    {
        absolute_value = (uint32_t)temperature_x10;
    }

    temperature_alarm_sample_uart_append_u32(
        buffer,
        buffer_size,
        length,
        absolute_value / 10u);
    temperature_alarm_sample_uart_append_char(buffer, buffer_size, length, '.');
    temperature_alarm_sample_uart_append_char(
        buffer,
        buffer_size,
        length,
        (char)('0' + (absolute_value % 10u)));
}

static float temperature_alarm_sample_adc_to_celsius(uint16_t adc_value)
{
    float adc = (float)adc_value;
    float voltage_sensor;
    float voltage_pullup;
    float resistance;
    float inverse_kelvin;

    if (adc < 1.0f)
    {
        adc = 1.0f;
    }

    if (adc > (TEMPERATURE_ALARM_SAMPLE_ADC_MAX_COUNT - 1.0f))
    {
        adc = TEMPERATURE_ALARM_SAMPLE_ADC_MAX_COUNT - 1.0f;
    }

    voltage_sensor = (adc * TEMPERATURE_ALARM_SAMPLE_VREF_VOLTS) /
        TEMPERATURE_ALARM_SAMPLE_ADC_MAX_COUNT;
    voltage_pullup = TEMPERATURE_ALARM_SAMPLE_VREF_VOLTS - voltage_sensor;

    if (voltage_pullup < 0.001f)
    {
        voltage_pullup = 0.001f;
    }

    resistance = TEMPERATURE_ALARM_SAMPLE_PULLUP_RESISTANCE_OHM *
        (voltage_sensor / voltage_pullup);

    inverse_kelvin =
        (1.0f / TEMPERATURE_ALARM_SAMPLE_T25_KELVIN) +
        (temperature_alarm_sample_ln(resistance / TEMPERATURE_ALARM_SAMPLE_R25_OHM) /
         TEMPERATURE_ALARM_SAMPLE_BETA);

    return (1.0f / inverse_kelvin) - 273.15f;
}

static int32_t temperature_alarm_sample_celsius_to_x10(float temperature_celsius)
{
    if (temperature_celsius >= 0.0f)
    {
        return (int32_t)(temperature_celsius * 10.0f + 0.5f);
    }

    return (int32_t)(temperature_celsius * 10.0f - 0.5f);
}

static temperature_alarm_sample_state_t temperature_alarm_sample_resolve_state(int32_t temperature_x10)
{
    if (temperature_x10 >= TEMPERATURE_ALARM_SAMPLE_ALARM_THRESHOLD_C_X10)
    {
        return TEMPERATURE_ALARM_SAMPLE_STATE_ALARM;
    }

    if (temperature_x10 >= TEMPERATURE_ALARM_SAMPLE_WARN_THRESHOLD_C_X10)
    {
        return TEMPERATURE_ALARM_SAMPLE_STATE_WARN;
    }

    return TEMPERATURE_ALARM_SAMPLE_STATE_NORMAL;
}

static void temperature_alarm_sample_log_status(
    temperature_alarm_sample_state_t state,
    int32_t temperature_x10,
    uint16_t adc_value,
    bool digital_threshold_active)
{
    char buffer[128];
    size_t length = 0u;

    temperature_alarm_sample_uart_append_string(buffer, sizeof(buffer), &length, "temp sample ");

    switch (state)
    {
    case TEMPERATURE_ALARM_SAMPLE_STATE_WARN:
        temperature_alarm_sample_uart_append_string(buffer, sizeof(buffer), &length, "warn");
        break;
    case TEMPERATURE_ALARM_SAMPLE_STATE_ALARM:
        temperature_alarm_sample_uart_append_string(buffer, sizeof(buffer), &length, "alarm");
        break;
    case TEMPERATURE_ALARM_SAMPLE_STATE_NORMAL:
    default:
        temperature_alarm_sample_uart_append_string(buffer, sizeof(buffer), &length, "normal");
        break;
    }

    temperature_alarm_sample_uart_append_string(buffer, sizeof(buffer), &length, " temp=");
    temperature_alarm_sample_uart_append_temperature_x10(buffer, sizeof(buffer), &length, temperature_x10);
    temperature_alarm_sample_uart_append_string(buffer, sizeof(buffer), &length, "C adc=");
    temperature_alarm_sample_uart_append_u32(buffer, sizeof(buffer), &length, adc_value);
    temperature_alarm_sample_uart_append_string(buffer, sizeof(buffer), &length, " do=");
    temperature_alarm_sample_uart_append_char(
        buffer,
        sizeof(buffer),
        &length,
        digital_threshold_active ? '1' : '0');
    temperature_alarm_sample_uart_append_string(buffer, sizeof(buffer), &length, "\r\n");
    board_uart_write_string(buffer);
}

static void temperature_alarm_sample_set_heartbeat_led(bool on)
{
    s_heartbeat_led_on = on;
    temperature_alarm_sample_port_set_heartbeat_led(on);
}

static void temperature_alarm_sample_toggle_heartbeat_led(void)
{
    temperature_alarm_sample_set_heartbeat_led(!s_heartbeat_led_on);
}

static void temperature_alarm_sample_set_alarm_led(bool on)
{
    s_alarm_led_on = on;
    temperature_alarm_sample_port_set_alarm_led(on);
    temperature_alarm_sample_port_set_warning_aux_led(on);
}

static void temperature_alarm_sample_toggle_alarm_led(void)
{
    temperature_alarm_sample_set_alarm_led(!s_alarm_led_on);
}

static void temperature_alarm_sample_update_heartbeat_led(void)
{
    ++s_heartbeat_tick_count;

    if (s_heartbeat_tick_count >= TEMPERATURE_ALARM_SAMPLE_HEARTBEAT_TOGGLE_TICKS)
    {
        s_heartbeat_tick_count = 0u;
        temperature_alarm_sample_toggle_heartbeat_led();
    }
}

static void temperature_alarm_sample_update_alarm_outputs(void)
{
    uint32_t toggle_ticks;

    switch (s_current_state)
    {
    case TEMPERATURE_ALARM_SAMPLE_STATE_ALARM:
        temperature_alarm_sample_port_set_buzzer(true);
        toggle_ticks = TEMPERATURE_ALARM_SAMPLE_ALARM_TOGGLE_TICKS;
        break;

    case TEMPERATURE_ALARM_SAMPLE_STATE_WARN:
        temperature_alarm_sample_port_set_buzzer(false);
        toggle_ticks = TEMPERATURE_ALARM_SAMPLE_WARN_TOGGLE_TICKS;
        break;

    case TEMPERATURE_ALARM_SAMPLE_STATE_NORMAL:
    default:
        temperature_alarm_sample_port_set_buzzer(false);
        temperature_alarm_sample_set_alarm_led(false);
        s_alarm_tick_count = 0u;
        return;
    }

    ++s_alarm_tick_count;
    if (s_alarm_tick_count >= toggle_ticks)
    {
        s_alarm_tick_count = 0u;
        temperature_alarm_sample_toggle_alarm_led();
    }
}

static void temperature_alarm_sample_set_all_board_leds(bool on)
{
    board_status_led_set(on);
    temperature_alarm_sample_set_heartbeat_led(on);
    temperature_alarm_sample_set_alarm_led(on);
}

static void temperature_alarm_sample_run_startup_selftest(void)
{
    bool leds_on = false;
    uint32_t beep_count = 0u;

    for (uint32_t phase = 0u; phase < TEMPERATURE_ALARM_SAMPLE_SELFTEST_PHASES; ++phase)
    {
        leds_on = !leds_on;
        temperature_alarm_sample_set_all_board_leds(leds_on);

        if (leds_on && beep_count < 2u)
        {
            temperature_alarm_sample_port_set_buzzer(true);
            ++beep_count;
        }
        else
        {
            temperature_alarm_sample_port_set_buzzer(false);
        }

        board_busy_wait(TEMPERATURE_ALARM_SAMPLE_SELFTEST_PHASE_DELAY_CYCLES);
    }

    temperature_alarm_sample_port_set_buzzer(false);
    temperature_alarm_sample_set_all_board_leds(false);
}

void temperature_alarm_sample_init(void)
{
    temperature_alarm_sample_port_init();
    temperature_alarm_sample_port_set_buzzer(false);
    board_status_led_set(false);
    temperature_alarm_sample_set_heartbeat_led(false);
    temperature_alarm_sample_set_alarm_led(false);
    s_current_state = TEMPERATURE_ALARM_SAMPLE_STATE_NORMAL;
    s_heartbeat_tick_count = 0u;
    s_alarm_tick_count = 0u;
    s_log_tick_count = TEMPERATURE_ALARM_SAMPLE_LOG_PERIOD_TICKS;
    temperature_alarm_sample_run_startup_selftest();

    if (board_has_uart_log())
    {
        board_uart_write_string(
            "temp sample wiring: AO->PA4 DO->PA5 SIG->PB9 AUXLED->PB1 LED1->PA1 LED2->PA2 LED3->PA3\r\n");
        board_uart_write_string(
            "temp sample selftest: LED1/LED2/LED3 blink ~3s, buzzer beeps twice\r\n");
        board_uart_write_string(
            "temp sample policy: LED2 heartbeat, >=warn LED3+AUXLED blink, >=alarm LED3+AUXLED+buzzer\r\n");
    }
}

void temperature_alarm_sample_process(void)
{
    uint16_t adc_value = temperature_alarm_sample_port_read_adc_average(
        TEMPERATURE_ALARM_SAMPLE_ADC_AVERAGE_COUNT);
    bool digital_threshold_active = !temperature_alarm_sample_port_read_digital_threshold();
    int32_t temperature_x10 = temperature_alarm_sample_celsius_to_x10(
        temperature_alarm_sample_adc_to_celsius(adc_value));
    temperature_alarm_sample_state_t next_state = temperature_alarm_sample_resolve_state(temperature_x10);

    if (next_state != s_current_state)
    {
        s_current_state = next_state;
        s_alarm_tick_count = 0u;

        if (s_current_state == TEMPERATURE_ALARM_SAMPLE_STATE_NORMAL)
        {
            temperature_alarm_sample_set_alarm_led(false);
        }
        else
        {
            temperature_alarm_sample_set_alarm_led(true);
        }

    }

    if (board_has_uart_log())
    {
        ++s_log_tick_count;

        if (s_log_tick_count >= TEMPERATURE_ALARM_SAMPLE_LOG_PERIOD_TICKS)
        {
            s_log_tick_count = 0u;
            temperature_alarm_sample_log_status(
                s_current_state,
                temperature_x10,
                adc_value,
                digital_threshold_active);
        }
    }

    temperature_alarm_sample_update_heartbeat_led();
    temperature_alarm_sample_update_alarm_outputs();
    board_busy_wait(TEMPERATURE_ALARM_SAMPLE_LOOP_DELAY_CYCLES);
}
