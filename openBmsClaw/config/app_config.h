#ifndef OPEN_BMS_CLAW_CONFIG_APP_CONFIG_H
#define OPEN_BMS_CLAW_CONFIG_APP_CONFIG_H

/* ============================================================================
 * 应用层与系统服务启动配置 (Day 1 基线服务裁剪)
 * ============================================================================
 */
#define APP_ENABLE_BRINGUP_SERVICE     1    /* 启动底层 Bring-up 服务，依次执行 UART/ADC/I2C 外设自检 */
#define APP_ENABLE_UI_SERVICE          1    /* 启动极简 UI 服务 (通过状态灯闪烁展示系统存活状态) */

#define APP_ENABLE_BMS_SERVICE         0    /* BMS 电芯保护与均衡服务，当前处于裁剪等待态 */
#define APP_ENABLE_POWER_SERVICE       1    /* 多口智能功率动态分配服务，当前处于裁剪等待态 */
#define APP_ENABLE_PROTOCOL_SERVICE    0    /* 快充协议通知服务，当前处于裁剪等待态 */

/* 示例工程独立测试开关 */
#define APP_ENABLE_TEMPERATURE_ALARM_SAMPLE 0 /* 温度超限告警演示工程开关，主线联调时关闭 */

#endif /* OPEN_BMS_CLAW_CONFIG_APP_CONFIG_H */
