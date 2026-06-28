#ifndef OPEN_BMS_CLAW_DRIVERS_SOC_SOC_TYPES_H
#define OPEN_BMS_CLAW_DRIVERS_SOC_SOC_TYPES_H

#include <stdint.h>

/* ============================================================================
 * SoC 抽象层平台标准类型 (不依赖任何 HAL / MCU 寄存器细节)
 * ============================================================================
 * 本文件只放平台领域类型：状态码、端口、告警事件掩码和回调签名。
 * 严禁在此 include 任何 hal_*.h 或 MCU 寄存器头，以保证 app/services 可安全包含。
 */

/* 紧急中断告警事件类型掩码 */
#define SOC_ALARM_NONE           0u
#define SOC_ALARM_SHORT_CIRCUIT  (1u << 0) /* 负载短路告警 */
#define SOC_ALARM_OVER_TEMP      (1u << 1) /* 电池或芯片过温 */
#define SOC_ALARM_OVER_CURRENT   (1u << 2) /* 严重充电/放电过流 */
#define SOC_ALARM_SYS_ERROR      (1u << 3) /* 底层系统或通信死锁故障 */

/* SoC 紧急中断非阻塞回调函数类型定义 (微秒级执行，严禁含慢速I2C) */
typedef void (*soc_sal_emergency_callback_t)(uint32_t alarm_mask);

/* SoC 抽象层错误码 */
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

#endif /* OPEN_BMS_CLAW_DRIVERS_SOC_SOC_TYPES_H */
