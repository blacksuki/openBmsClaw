#ifndef OPEN_BMS_CLAW_HAL_I2C_HAL_I2C_H
#define OPEN_BMS_CLAW_HAL_I2C_HAL_I2C_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "../../config/sys_config.h"

/* ============================================================================
 * 错误码定义 (返回值判断)
 * ============================================================================
 */
typedef enum {
  HAL_I2C_OK = 0,            /* 通信成功 */
  HAL_I2C_ERR_BUSY = -1,     /* 总线忙碌或锁死 */
  HAL_I2C_ERR_TIMEOUT = -2,  /* 操作超时 (防止死循环等待) */
  HAL_I2C_ERR_NACK = -3,     /* 从机无应答 (地址错误或设备离线) */
  HAL_I2C_ERR_BUS_ERROR = -4 /* 总线异常拉低 */
} hal_i2c_status_t;

/* ============================================================================
 * 接口函数声明
 * ============================================================================
 */

/**
 * @brief 初始化 MCU 底层 I2C1 外设 (包含 GPIO 引脚及时钟配置)
 * @return 是否初始化成功
 */
bool hal_i2c_init(void);

/**
 * @brief 执行总线死锁恢复 (Bus Recovery)
 * @note 通过连续模拟发送 9 个时钟脉冲，强制释放被异常从机锁死 (SDA拉低) 的总线
 */
void hal_i2c_bus_recovery(void);

/**
 * @brief 带超时与防死锁保护的 I2C 主机读取寄存器函数
 * @param dev_addr   从机 7 位 I2C 设备地址 (未移位)
 * @param reg_addr   目标内部寄存器地址
 * @param data       接收数据缓冲区指针
 * @param len        读取字节数
 * @param timeout_ms 操作最大超时时间 (毫秒)
 * @return hal_i2c_status_t 运行结果
 */
hal_i2c_status_t hal_i2c_read(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data,
                              uint16_t len, uint32_t timeout_ms);

/**
 * @brief 带超时与防死锁保护的 I2C 主机写入寄存器函数
 * @param dev_addr   从机 7 位 I2C 设备地址 (未移位)
 * @param reg_addr   目标内部寄存器地址
 * @param data       待发送数据缓冲区指针
 * @param len        写入字节数
 * @param timeout_ms 操作最大超时时间 (毫秒)
 * @return hal_i2c_status_t 运行结果
 */
hal_i2c_status_t hal_i2c_write(uint8_t dev_addr, uint8_t reg_addr,
                               const uint8_t *data, uint16_t len,
                               uint32_t timeout_ms);

#endif /* OPEN_BMS_CLAW_HAL_I2C_HAL_I2C_H */
