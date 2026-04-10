#ifndef OPEN_BMS_CLAW_SERVICES_BRINGUP_BRINGUP_SERVICE_H
#define OPEN_BMS_CLAW_SERVICES_BRINGUP_BRINGUP_SERVICE_H

#include <stdbool.h>

typedef enum
{
    BRINGUP_STAGE_BOOT = 0,
    BRINGUP_STAGE_LED,
    BRINGUP_STAGE_UART,
    BRINGUP_STAGE_ADC,
    BRINGUP_STAGE_I2C,
    BRINGUP_STAGE_DONE
} bringup_stage_t;

void bringup_service_init(void);
void bringup_service_process(void);
bringup_stage_t bringup_service_get_stage(void);
bool bringup_service_is_complete(void);

#endif
