#include "services/bringup/bringup_service.h"

#include <stdio.h>

#include "board/board.h"
#include "config/sys_config.h"
#include "drivers/soc_sal.h"

static bringup_stage_t s_current_stage = BRINGUP_STAGE_BOOT;
static uint8_t s_detected_soc_address = 0x75u;
static bool s_detected_soc_address_valid = false;
static bool s_soc_sal_ready = false;

static bool bringup_service_adc_enabled(void)
{
    return (FEATURE_ENABLE_ADC_PROBE != 0) && board_has_adc_channel();
}

static bool bringup_service_i2c_enabled(void)
{
    return (FEATURE_ENABLE_I2C_PROBE != 0) && board_has_i2c_bus();
}

static bool bringup_service_soc_int_selftest_enabled(void)
{
    return (CONFIG_ENABLE_BRINGUP_SELFTEST != 0) && (CONFIG_ENABLE_SOC_INT_HIGHWAY != 0);
}

/* 探针阶段结束后的去向：开启自测则进入 SoC INT 自测，否则直接结束 */
static bringup_stage_t bringup_service_stage_after_probes(void)
{
    return BRINGUP_STAGE_SOC_SAL_INIT;
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

    s_detected_soc_address_valid = false;

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

    if (found_count > 0)
    {
        s_detected_soc_address = first_found_address;
        s_detected_soc_address_valid = true;
    }

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

static void bringup_service_run_soc_sal_init(void)
{
    if (!s_detected_soc_address_valid)
    {
        s_soc_sal_ready = false;

        if (board_has_uart_log())
        {
            board_uart_write_string("bringup: soc sal init skipped (no i2c device)\r\n");
        }

        return;
    }

    if (board_has_uart_log())
    {
        char message[80];
        (void)snprintf(
            message,
            sizeof(message),
            "bringup: soc sal init start addr=0x%02X\r\n",
            (unsigned int)s_detected_soc_address);
        board_uart_write_string(message);
    }

    s_soc_sal_ready = soc_sal_init(s_detected_soc_address);
}

static void bringup_service_run_soc_int_selftest(void)
{
    if (board_has_uart_log())
    {
        board_uart_write_string("bringup: soc int selftest (sw EXTI trigger)\r\n");
    }

    soc_sal_int_selftest_trigger();
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

        return bringup_service_i2c_enabled() ? BRINGUP_STAGE_I2C
                                             : bringup_service_stage_after_probes();
    case BRINGUP_STAGE_ADC:
        return bringup_service_i2c_enabled() ? BRINGUP_STAGE_I2C
                                             : bringup_service_stage_after_probes();
    case BRINGUP_STAGE_I2C:
        return bringup_service_stage_after_probes();
    case BRINGUP_STAGE_SOC_SAL_INIT:
        return bringup_service_soc_int_selftest_enabled() ? BRINGUP_STAGE_SOC_INT_SELFTEST
                                                          : BRINGUP_STAGE_DONE;
    case BRINGUP_STAGE_SOC_INT_SELFTEST:
        return BRINGUP_STAGE_DONE;
    case BRINGUP_STAGE_DONE:
    default:
        return BRINGUP_STAGE_DONE;
    }
}

void bringup_service_init(void)
{
    s_current_stage = BRINGUP_STAGE_BOOT;
    s_detected_soc_address = 0x75u;
    s_detected_soc_address_valid = false;
    s_soc_sal_ready = false;
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
    else if (s_current_stage == BRINGUP_STAGE_SOC_SAL_INIT)
    {
        bringup_service_run_soc_sal_init();
    }
    else if (s_current_stage == BRINGUP_STAGE_SOC_INT_SELFTEST)
    {
        bringup_service_run_soc_int_selftest();
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
