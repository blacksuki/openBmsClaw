#include "services/ui/ui_service.h"

#include "board/board.h"

void ui_service_init(void)
{
}

void ui_service_process(void)
{
    if (!board_has_status_led())
    {
        return;
    }

    board_status_led_toggle();
    board_busy_wait(1600000u);
}
