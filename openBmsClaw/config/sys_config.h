#ifndef OPEN_BMS_CLAW_CONFIG_SYS_CONFIG_H
#define OPEN_BMS_CLAW_CONFIG_SYS_CONFIG_H

/* ============================================================================
 * openBattery V0 原型机系统总体配置与裁剪规范 (针对 STM32F103 64KB Flash 限制)
 * ============================================================================
 * 针对首发测试硬件建立严格的资源与功能开关。在最小可行性联调阶段，关闭所有
 * 高内存、高Flash消耗的非核心模块，保障底层双向 I2C/UART 通信链路的验证。
 */

/* 1. UI 与字体资源裁剪宏 (面向 64KB Flash 平台严格裁剪) */
#define CONFIG_ENABLE_OLED_DISPLAY     0    /* 屏蔽复杂 OLED 屏幕图形驱动引擎，防止 Flash 溢出 */
#define CONFIG_ENABLE_LARGE_FONT       0    /* 严格屏蔽中日韩多语言大字库点阵数据 (汉字/假名) */
#define CONFIG_ENABLE_COMPLEX_UI_ANIM  0    /* 屏蔽实时动态功率曲线与复杂波形渲染交互 */

/* 2. 通信与外部链路开关 */
#define CONFIG_ENABLE_BLE_COMM         0    /* 早期联调阶段屏蔽 BLE 手机端 APP 连接 */

/* 3. 总线与安全配置 */
#define CONFIG_I2C_BUS_RECOVERY_ENABLE 1    /* 开启 I2C 总线死锁自愈机制 (连续发送 9 个时钟脉冲释放总线) */
#define CONFIG_I2C_TIMEOUT_MS          10   /* 严格禁止无超时等待，设置标准外设 Timeout 为 10ms */

/* 4. 紧急告警中断高速通道配置 */
#define CONFIG_ENABLE_SOC_INT_HIGHWAY  1    /* 开启 SoC 硬件中断告警高速通道 */
#define CONFIG_SOC_INT_PIN_INDEX       4    /* SoC 中断引脚对应为引脚 4 (PA4 -> EXTI4) */
#define CONFIG_ENABLE_BRINGUP_SELFTEST 1    /* 开启 bring-up 自测：由 bringup_service 软件触发一次 SoC INT 高速通道验证中断链路；生产 profile 应置 0 */

#include "board_config.h"
#include "feature_config.h"
#include "app_config.h"

#endif /* OPEN_BMS_CLAW_CONFIG_SYS_CONFIG_H */
