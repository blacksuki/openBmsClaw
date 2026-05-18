#ifndef OPEN_BMS_CLAW_CONFIG_FEATURE_CONFIG_H
#define OPEN_BMS_CLAW_CONFIG_FEATURE_CONFIG_H

/* ============================================================================
 * 基础外设与硬件探针配置 (Bring-up 基线)
 * ============================================================================
 */
#define FEATURE_ENABLE_STATUS_LED      1    /* 开启状态指示灯使能，用于观察心跳节拍与基础状态机 */
#define FEATURE_ENABLE_UART_LOG        1    /* 开启 UART 日志终端输出，作为 V0 阶段最主要的数据观察窗口 */
#define FEATURE_ENABLE_ADC_PROBE       1    /* 开启 ADC 采样探针验证 (用于基础电压与 NTC 温度监测) */
#define FEATURE_ENABLE_I2C_PROBE       1    /* 开启 I2C 总线探针验证 (用于扫描与打通集成 SoC 地址及寄存器) */

/* ============================================================================
 * 后续高级业务模块使能宏 (Day 1 处于关闭状态，待单点打通后按次序开启)
 * ============================================================================
 */
#define FEATURE_ENABLE_BMS             0    /* 真实 BMS 电芯采样与安全防护开关，当前暂不启用 */
#define FEATURE_ENABLE_POWER           0    /* 智能功率分配与多端口调度器开关，当前暂不启用 */
#define FEATURE_ENABLE_PROTOCOL        0    /* 快充协议握手管理与事件分发开关，当前暂不启用 */

#endif /* OPEN_BMS_CLAW_CONFIG_FEATURE_CONFIG_H */
