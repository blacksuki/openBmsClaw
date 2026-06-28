#ifndef OPEN_BMS_CLAW_CONFIG_FEATURE_CONFIG_H
#define OPEN_BMS_CLAW_CONFIG_FEATURE_CONFIG_H

#include "profile_config.h"

/* ============================================================================
 * FEATURE_* : 该能力是否编译进固件 (编译轴)，由当前 PROFILE 派生
 * ============================================================================
 * 注：板载 LED / UART 的"是否存在"由 board 能力查询 board_has_status_led() /
 * board_has_uart_log() 决定，是唯一权威来源；此处不再保留与之重复的
 * FEATURE_ENABLE_STATUS_LED / FEATURE_ENABLE_UART_LOG (原为零引用死宏)。
 */

#if defined(PROFILE_F103_BRINGUP)

#define FEATURE_ENABLE_ADC_PROBE   1    /* ADC 采样探针 (基础电压与 NTC 温度) */
#define FEATURE_ENABLE_I2C_PROBE   1    /* I2C 总线探针 (扫描并打通 SoC 地址/寄存器) */
#define FEATURE_ENABLE_BMS         0    /* BMS 电芯采样与保护，bring-up 暂不编译 */
#define FEATURE_ENABLE_POWER       1    /* 功率/紧急保护能力 (承载 emergency 回调与自测链路) */
#define FEATURE_ENABLE_PROTOCOL    0    /* 快充协议握手，bring-up 暂不编译 */

#elif defined(PROFILE_F030_MIN_PRODUCT)

#define FEATURE_ENABLE_ADC_PROBE   1    /* NTC/ADC 温度与电压采样 */
#define FEATURE_ENABLE_I2C_PROBE   1    /* 与专用 SoC 的 I2C 通信 */
#define FEATURE_ENABLE_BMS         0    /* 待 Phase 3 样片验证后确定 */
#define FEATURE_ENABLE_POWER       1    /* 轻量功率分配与温度降额 (arch §7.2) */
#define FEATURE_ENABLE_PROTOCOL    0    /* 协议握手由 SoC 承担，MCU 侧通知服务待定 */

#endif

#endif /* OPEN_BMS_CLAW_CONFIG_FEATURE_CONFIG_H */
