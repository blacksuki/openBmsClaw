#include "services/bringup/bringup_service.h"

#include <stdio.h>

#include "board/board.h"
#include "config/sys_config.h"
#include "drivers/soc_sal.h"
#include "services/power/power_service.h"

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

/* 探针阶段结束后先进入 SoC SAL 初始化，再决定是否进入后续自测 */
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

    s_soc_sal_ready = soc_sal_init(s_detected_soc_address);

    if (board_has_uart_log())
    {
        board_uart_write_string(s_soc_sal_ready ? "bringup: soc sal init ok\r\n"
                                                : "bringup: soc sal init fail\r\n");
    }

    if (s_soc_sal_ready)
    {
        uint32_t event_mask = 0u;
        soc_sal_status_t status = soc_poll_events(&event_mask);

        if (board_has_uart_log())
        {
            if (status == SOC_SAL_OK)
            {
                char message[80];
                (void)snprintf(
                    message,
                    sizeof(message),
                    "bringup: soc poll online mask=0x%08X\r\n",
                    (unsigned int)event_mask);
                board_uart_write_string(message);
            }
            else
            {
                board_uart_write_string("bringup: soc poll offline\r\n");
            }
        }
    }
}

static void bringup_service_run_soc_int_selftest(void)
{
    if (!soc_sal_is_initialized())
    {
        if (board_has_uart_log())
        {
            board_uart_write_string("bringup: soc int selftest skipped (sal offline)\r\n");
        }
        return;
    }

    if (board_has_uart_log())
    {
        board_uart_write_string("bringup: soc int selftest (sw EXTI trigger)\r\n");
    }

    soc_sal_int_selftest_trigger();

    if (board_has_uart_log())
    {
        board_uart_write_string(
            power_service_get_state() == POWER_STATE_EMERGENCY_LOCK
                ? "bringup: soc int selftest PASS\r\n"
                : "bringup: soc int selftest FAIL\r\n");
    }
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
        return (s_soc_sal_ready && bringup_service_soc_int_selftest_enabled())
                   ? BRINGUP_STAGE_SOC_INT_SELFTEST
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
