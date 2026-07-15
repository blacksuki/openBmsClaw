#ifndef OPEN_BMS_CLAW_DRIVERS_SOC_SOC_API_H
#define OPEN_BMS_CLAW_DRIVERS_SOC_SOC_API_H

#include <stdbool.h>
#include <stdint.h>

#include "soc_types.h"

/* ============================================================================
 * SoC 抽象层公共 API (面向 app/ 与 services/ 的唯一入口)
 * ============================================================================
 * 只暴露平台领域能力，屏蔽各家芯片寄存器差异。
 * 本头文件只允许包含标准 C 头与 soc_types.h，严禁包含任何 hal_*.h，
 * 以保证上层包含本头时不会间接看到 MCU 外设实现细节 (见 ISSUE-001)。
 */

/**
 * @brief 初始化 SoC 硬件抽象层及底层总线句柄
 * @param i2c_addr SoC 从机通信地址
 * @return 是否初始化成功
 */
bool soc_sal_init(uint8_t i2c_addr);

/**
 * @brief 查询 SAL 当前是否已经完成运行时初始化
 * @return true 表示所有已启用的初始化步骤均已完成
 */
bool soc_sal_is_initialized(void);

/**
 * @brief 在 SAL 层触发总线死锁自愈机制 (Bus Recovery)
 * @note 调用底层 GPIO 模拟翻转逻辑连续发出 9 个时钟脉冲，恢复 I2C 信道，并重置
 * SAL 状态机
 */
void soc_sal_bus_recovery(void);

/**
 * @brief 获取当前电池实时总电压
 * @param voltage_mv 接收电压缓冲 (毫伏)
 * @return soc_sal_status_t
 */
soc_sal_status_t soc_get_voltage(uint32_t *voltage_mv);

/**
 * @brief 获取当前电池实时电流
 * @param current_ma 接收电流缓冲 (毫安，正数表示充电，负数表示放电)
 * @return soc_sal_status_t
 */
soc_sal_status_t soc_get_current(int32_t *current_ma);

/**
 * @brief 获取 SoC 内部实时核心温度
 * @param temp_celsius 接收温度缓冲 (摄氏度)
 * @return soc_sal_status_t
 */
soc_sal_status_t soc_get_temperature(int16_t *temp_celsius);

/**
 * @brief 动态下发/修改过流保护阈值 (OCP)
 * @param port 目标端口
 * @param limit_ma 设定最大过流阈值 (毫安)
 * @return soc_sal_status_t
 */
soc_sal_status_t soc_set_ocp(soc_port_t port, uint32_t limit_ma);

/**
 * @brief 动态分配/限制指定端口的最大允许输出功率
 * @param port 目标端口
 * @param max_power_w 功率限额 (瓦特)
 * @return soc_sal_status_t
 */
soc_sal_status_t soc_set_port_power(soc_port_t port, uint16_t max_power_w);

/**
 * @brief 轮询/捕获 SoC 异步事件与告警通知
 * @param event_mask 接收返回的事件掩码 (如快充握手成功、短路告警)
 * @return soc_sal_status_t
 */
soc_sal_status_t soc_poll_events(uint32_t *event_mask);

/**
 * @brief 注册上层服务层紧急告警回调函数
 * @param callback 回调函数指针 (将在底层 EXTI 中断上下文直接调用)
 */
void soc_sal_register_emergency_callback(soc_sal_emergency_callback_t callback);

#endif /* OPEN_BMS_CLAW_DRIVERS_SOC_SOC_API_H */
