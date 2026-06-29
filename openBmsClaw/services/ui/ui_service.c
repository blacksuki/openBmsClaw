#include "services/ui/ui_service.h"

#include "board/board.h"
#include "config/sys_config.h"

#if APP_ENABLE_POWER_EMERGENCY_TEST && FEATURE_ENABLE_POWER
#include "services/power/power_service.h"
#endif

/* 状态灯闪烁周期 (毫秒)：正常心跳慢闪，紧急锁定快闪 */
#define UI_HEARTBEAT_PERIOD_MS 500u
#define UI_EMERGENCY_PERIOD_MS 100u

static uint32_t s_last_toggle_ms = 0u;

void ui_service_init(void)
{
    s_last_toggle_ms = 0u;
}

void ui_service_process(void)
{
    uint32_t period_ms = UI_HEARTBEAT_PERIOD_MS;
    uint32_t now_ms;

    if (!board_has_status_led())
    {
        return;
    }

#if APP_ENABLE_POWER_EMERGENCY_TEST && FEATURE_ENABLE_POWER
    if (power_service_get_state() == POWER_STATE_EMERGENCY_LOCK)
    {
        period_ms = UI_EMERGENCY_PERIOD_MS;
    }
#endif

    now_ms = board_get_tick_ms();
    if ((now_ms - s_last_toggle_ms) >= period_ms)
    {
        s_last_toggle_ms = now_ms;
        board_status_led_toggle();
    }
}
