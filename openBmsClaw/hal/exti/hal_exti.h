#ifndef OPEN_BMS_CLAW_HAL_EXTI_HAL_EXTI_H
#define OPEN_BMS_CLAW_HAL_EXTI_HAL_EXTI_H

#include <stdbool.h>
#include <stdint.h>

#include "../../config/sys_config.h"

/* EXTI 回调函数类型定义 */
typedef void (*hal_exti_callback_t)(void);

/* EXTI 物理端口定义 */
typedef enum {
  HAL_EXTI_PORT_A = 0,
  HAL_EXTI_PORT_B = 1,
  HAL_EXTI_PORT_C = 2
} hal_exti_port_t;

/* 中断边沿触发模式定义 */
typedef enum {
  HAL_EXTI_TRIGGER_RISING = 0,  /* 上升沿触发 */
  HAL_EXTI_TRIGGER_FALLING = 1, /* 下降沿触发 */
  HAL_EXTI_TRIGGER_BOTH = 2     /* 双边沿触发 */
} hal_exti_trigger_t;

/**
 * @brief 初始化指定 GPIO 引脚为外部中断模式 (EXTI)
 * @param port 目标 GPIO 端口 (PA / PB / PC)
 * @param pin_index 目标引脚索引 (0 - 15)
 * @param trigger 边沿触发类型 (上升/下降/双边沿)
 * @param callback 硬件中断触发时的 C 语言非阻塞回调函数
 * @return 是否配置成功
 */
bool hal_exti_init(hal_exti_port_t port, uint8_t pin_index,
                   hal_exti_trigger_t trigger, hal_exti_callback_t callback);

/**
 * @brief 开启指定引脚的中断屏蔽线 (Unmask)
 * @param pin_index 目标引脚索引 (0 - 15)
 */
void hal_exti_enable(uint8_t pin_index);

/**
 * @brief 屏蔽指定引脚的中断线 (Mask)
 * @param pin_index 目标引脚索引 (0 - 15)
 */
void hal_exti_disable(uint8_t pin_index);

/**
 * @brief 人为触发指定引脚的软件中断事件 (写入 SWIER 寄存器)
 * @param pin_index 目标引脚索引 (0 - 15)
 */
void hal_exti_software_trigger(uint8_t pin_index);

#endif /* OPEN_BMS_CLAW_HAL_EXTI_HAL_EXTI_H */
