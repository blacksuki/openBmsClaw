#include "services/bringup/bringup_service.h"

#include <stdio.h>

#include "board/board.h"
#include "config/feature_config.h"

static bringup_stage_t s_current_stage = BRINGUP_STAGE_BOOT;

static bool bringup_service_adc_enabled(void)
{
    return (FEATURE_ENABLE_ADC_PROBE != 0) && board_has_adc_channel();
}

static bool bringup_service_i2c_enabled(void)
{
    return (FEATURE_ENABLE_I2C_PROBE != 0) && board_has_i2c_bus();
}

static void bringup_service_run_adc_probe(void)
{
    char message[96];
    uint16_t raw_value;
    uint16_t average_value;

    board_adc_probe_init();
    raw_value = board_adc_probe_read_once();
    average_value = board_adc_probe_read_average(8u);

    if (board_has_uart_log())
    {
        (void)snprintf(
            message,
            sizeof(message),
            "adc probe ch6 raw=%u avg=%u\r\n",
            (unsigned int)raw_value,
            (unsigned int)average_value);
        board_uart_write_string(message);
    }
}

static void bringup_service_run_i2c_probe(void)
{
    char message[96];
    uint8_t first_found_address = 0u;
    int found_count;

    if (board_has_uart_log())
    {
        board_uart_write_string("i2c init start\r\n");
    }

    if (!board_i2c_probe_init())
    {
        if (board_has_uart_log())
        {
            board_uart_write_string("i2c init fail\r\n");
        }

        return;
    }

    if (board_has_uart_log())
    {
        board_uart_write_string("i2c init ok\r\n");
    }

    found_count = board_i2c_probe_scan(&first_found_address);

    if (!board_has_uart_log())
    {
        return;
    }

    if (found_count > 0)
    {
        (void)snprintf(
            message,
            sizeof(message),
            "i2c found: 0x%02X count=%d\r\n",
            (unsigned int)first_found_address,
            found_count);
        board_uart_write_string(message);
        return;
    }

    if (found_count == 0)
    {
        board_uart_write_string("i2c scan done, no device found\r\n");
        return;
    }

    board_uart_write_string("i2c scan fail\r\n");
}

static bringup_stage_t bringup_service_next_stage(bringup_stage_t stage)
{
    switch (stage)
    {
    case BRINGUP_STAGE_BOOT:
        return board_has_status_led() ? BRINGUP_STAGE_LED : BRINGUP_STAGE_UART;
    case BRINGUP_STAGE_LED:
        return BRINGUP_STAGE_UART;
    case BRINGUP_STAGE_UART:
        if (bringup_service_adc_enabled())
        {
            return BRINGUP_STAGE_ADC;
        }

        return bringup_service_i2c_enabled() ? BRINGUP_STAGE_I2C : BRINGUP_STAGE_DONE;
    case BRINGUP_STAGE_ADC:
        return bringup_service_i2c_enabled() ? BRINGUP_STAGE_I2C : BRINGUP_STAGE_DONE;
    case BRINGUP_STAGE_I2C:
        return BRINGUP_STAGE_DONE;
    case BRINGUP_STAGE_DONE:
    default:
        return BRINGUP_STAGE_DONE;
    }
}

void bringup_service_init(void)
{
    s_current_stage = BRINGUP_STAGE_BOOT;
}

void bringup_service_process(void)
{
    if (s_current_stage == BRINGUP_STAGE_DONE)
    {
        return;
    }

    if (s_current_stage == BRINGUP_STAGE_ADC)
    {
        bringup_service_run_adc_probe();
    }
    else if (s_current_stage == BRINGUP_STAGE_I2C)
    {
        bringup_service_run_i2c_probe();
    }

    s_current_stage = bringup_service_next_stage(s_current_stage);
}

bringup_stage_t bringup_service_get_stage(void)
{
    return s_current_stage;
}

bool bringup_service_is_complete(void)
{
    return s_current_stage == BRINGUP_STAGE_DONE;
}
