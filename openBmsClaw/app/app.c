#include "app/app.h"

#include "board/board.h"
#include "config/sys_config.h"
#include "samples/temperature_alarm/temperature_alarm_sample.h"
#include "services/bms/bms_service.h"
#include "services/bringup/bringup_service.h"
#include "services/power/power_service.h"
#include "services/protocol/protocol_service.h"
#include "services/ui/ui_service.h"

void app_init(void)
{
#if APP_ENABLE_TEMPERATURE_ALARM_SAMPLE
    temperature_alarm_sample_init();
#else
#if APP_ENABLE_BMS_SERVICE
    bms_service_init();
#endif
#if APP_ENABLE_POWER_SERVICE
    power_service_init();
#endif
#if APP_ENABLE_PROTOCOL_SERVICE
    protocol_service_init();
#endif
#if APP_ENABLE_UI_SERVICE
    ui_service_init();
#endif
#if APP_ENABLE_BRINGUP_SERVICE
    bringup_service_init();
#endif
#endif

    if (board_has_uart_log())
    {
        board_uart_write_string("boot ok\r\n");
#if APP_ENABLE_TEMPERATURE_ALARM_SAMPLE
        board_uart_write_string("sample: temperature alarm\r\n");
#endif
    }
}

void app_run(void)
{
#if APP_ENABLE_TEMPERATURE_ALARM_SAMPLE
    temperature_alarm_sample_process();
#else
#if APP_ENABLE_BRINGUP_SERVICE
    bringup_service_process();
#endif
#if APP_ENABLE_BMS_SERVICE
    bms_service_process();
#endif
#if APP_ENABLE_POWER_SERVICE
    power_service_process();
#endif
#if APP_ENABLE_PROTOCOL_SERVICE
    protocol_service_process();
#endif
#if APP_ENABLE_UI_SERVICE
    ui_service_process();
#endif
#endif
}
