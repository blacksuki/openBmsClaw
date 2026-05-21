#ifndef OPEN_BMS_CLAW_SERVICES_POWER_POWER_SERVICE_H
#define OPEN_BMS_CLAW_SERVICES_POWER_POWER_SERVICE_H

#include <stdint.h>

typedef enum {
  POWER_STATE_NORMAL = 0,
  POWER_STATE_EMERGENCY_LOCK = 1
} power_state_t;

void power_service_init(void);
void power_service_process(void);

/**
 * @brief 智能功率分配层紧急中断回调函数
 * @param alarm_mask 报警类型掩码
 * @note 此函数在底层硬件 EXTI 中断服务程序（ISR）中直接调用。
 *       必须保证微秒级极速运行，严禁包含任何延时、慢速 I2C 读写或打印！
 */
void power_service_handle_emergency_stop(uint32_t alarm_mask);

/**
 * @brief 获取当前功率系统状态机状态
 */
power_state_t power_service_get_state(void);

#endif
