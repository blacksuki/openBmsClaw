#ifndef OPEN_BMS_CLAW_DRIVERS_SOC_SAL_H
#define OPEN_BMS_CLAW_DRIVERS_SOC_SAL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "../config/sys_config.h"
#include "../hal/i2c/hal_i2c.h"
#include "../hal/exti/hal_exti.h"

/* 紧急中断告警事件类型掩码 */
#define SOC_ALARM_NONE           0u
#define SOC_ALARM_SHORT_CIRCUIT  (1u << 0) /* 负载短路告警 */
#define SOC_ALARM_OVER_TEMP      (1u << 1) /* 电池或芯片过温 */
#define SOC_ALARM_OVER_CURRENT   (1u << 2) /* 严重充电/放电过流 */
#define SOC_ALARM_SYS_ERROR      (1u << 3) /* 底层系统或通信死锁故障 */

/* SoC 紧急中断非阻塞回调函数类型定义 (微秒级执行，严禁含慢速I2C) */
typedef void (*soc_sal_emergency_callback_t)(uint32_t alarm_mask);

/* ============================================================================
 * SoC 抽象层数据结构与错误码定义
 * ============================================================================
 */
typedef enum {
  SOC_SAL_OK = 0,            /* 通信与解析成功 */
  SOC_SAL_ERR_OFFLINE = -1,  /* 从机无响应/离线 */
  SOC_SAL_ERR_BUSY = -2,     /* 总线繁忙或被拉低 */
  SOC_SAL_ERR_PARAM = -3,    /* 参数范围错误 */
  SOC_SAL_ERR_HW_LOCKED = -4 /* 硬件被锁死，需执行总线自愈 */
} soc_sal_status_t;

/* 统一电源端口类型定义 */
typedef enum {
  SOC_PORT_TYPE_C1 = 0, /* Type-C 1 主充放接口 */
  SOC_PORT_TYPE_C2 = 1, /* Type-C 2 充放接口 */
  SOC_PORT_TYPE_A1 = 2  /* USB-A 放电接口 */
} soc_port_t;

/* ============================================================================
 * 统一标准 API 接口声明 (拒绝黑盒，通过一致的函数屏蔽各家芯片寄存器差异)
 * ============================================================================
 */

/**
 * @brief 初始化 SoC 硬件抽象层及底层总线句柄
 * @param i2c_addr SoC 从机通信地址
 * @return 是否初始化成功
 */
bool soc_sal_init(uint8_t i2c_addr);

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

/**
 * @brief 往 STM32 EXTI SWIER 软件中断寄存器写位，人为模拟触发一次硬件 INT 低电平脉冲
 * @note 用于在无实物硬件环境下执行 100% 软件闭环中断自测
 */
void board_soc_int_sim_trigger(void);

#endif /* OPEN_BMS_CLAW_DRIVERS_SOC_SAL_H */
