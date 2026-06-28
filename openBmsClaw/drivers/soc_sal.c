#include "soc_sal.h"

#include "../board/board.h"
#include "../config/sys_config.h"
#include "../hal/exti/hal_exti.h"
#include "../hal/i2c/hal_i2c.h"
#include <stdio.h>

/* 基线 SoC (例如英集芯系列) 通用演示寄存器地址 */
#define REG_SOC_CHIP_ID 0x00u
#define REG_SOC_BAT_VOLT_H 0x50u
#define REG_SOC_BAT_VOLT_L 0x51u
#define REG_SOC_BAT_CURR_H 0x52u
#define REG_SOC_BAT_CURR_L 0x53u
#define REG_SOC_CHIP_TEMP 0x58u
#define REG_SOC_PORT_C1_OCP 0x60u
#define REG_SOC_PORT_C2_OCP 0x61u
#define REG_SOC_PORT_A1_OCP 0x62u
#define REG_SOC_EVENT_FLAGS 0x70u

static uint8_t s_soc_i2c_addr = 0x75u; /* 默认基线 I2C 地址 (英集芯常为 0x75) */
static bool s_sal_initialized = false;
static uint32_t s_recovery_count = 0u;

/* 紧急中断上层回调句柄 */
static soc_sal_emergency_callback_t s_emergency_callback = 0;

static void soc_sal_exti_hardware_handler(void) {
  /* 硬件 EXTI 中断响应入口 (在 EXTI4 ISR 中执行)
   * 严禁执行任何慢速、带while等待的 I2C 动作！
   * 直接向服务层投递一个紧急报警信号
   */
  if (s_emergency_callback != 0) {
    s_emergency_callback(SOC_ALARM_SYS_ERROR);
  }
}

bool soc_sal_init(uint8_t i2c_addr) {
  s_soc_i2c_addr = i2c_addr;
  s_recovery_count = 0u;

  if (!hal_i2c_init()) {
    if (board_has_uart_log()) {
      board_uart_write_string("SAL: I2C hardware init failed!\r\n");
    }
    s_sal_initialized = false;
    return false;
  }

  /* 读取芯片 ID 验证通信 */
  uint8_t chip_id = 0u;
  hal_i2c_status_t status = hal_i2c_read(s_soc_i2c_addr, REG_SOC_CHIP_ID,
                                         &chip_id, 1u, CONFIG_I2C_TIMEOUT_MS);

  if (status != HAL_I2C_OK) {
    if (board_has_uart_log()) {
      board_uart_write_string(
          "SAL: SoC chip probe failed, offline or locked.\r\n");
    }
    s_sal_initialized = false;
    return false;
  }

  if (board_has_uart_log()) {
    char msg[64];
    (void)snprintf(msg, sizeof(msg), "SAL: SoC init OK, ChipID=0x%02X\r\n",
                   (unsigned int)chip_id);
    board_uart_write_string(msg);
  }

  s_sal_initialized = true;

#if CONFIG_ENABLE_SOC_INT_HIGHWAY
  /* 初始化中断高速告警信道：PA4 (PORT_A, Pin 4)，下降沿触发，绑定本地处理器 */
  (void)hal_exti_init(HAL_EXTI_PORT_A, CONFIG_SOC_INT_PIN_INDEX,
                      HAL_EXTI_TRIGGER_FALLING, soc_sal_exti_hardware_handler);
#endif

  return true;
}

void soc_sal_bus_recovery(void) {
  s_recovery_count++;
  if (board_has_uart_log()) {
    char msg[80];
    (void)snprintf(msg, sizeof(msg),
                   "SAL: I2C bus lock detected (#%u)! Executing GPIO 9-pulse "
                   "recovery...\r\n",
                   (unsigned int)s_recovery_count);
    board_uart_write_string(msg);
  }

  /* 调用底层硬件外设的 9 时钟自愈算法 */
  hal_i2c_bus_recovery();

  /* 重新建立 SAL 层连接状态 */
  (void)soc_sal_init(s_soc_i2c_addr);
}

static soc_sal_status_t convert_hal_status(hal_i2c_status_t hal_status) {
  switch (hal_status) {
  case HAL_I2C_OK:
    return SOC_SAL_OK;
  case HAL_I2C_ERR_BUSY:
  case HAL_I2C_ERR_TIMEOUT:
  case HAL_I2C_ERR_BUS_ERROR:
    return SOC_SAL_ERR_HW_LOCKED;
  case HAL_I2C_ERR_NACK:
  default:
    return SOC_SAL_ERR_OFFLINE;
  }
}

soc_sal_status_t soc_get_voltage(uint32_t *voltage_mv) {
  uint8_t buf[2];

  if (voltage_mv == 0) {
    return SOC_SAL_ERR_PARAM;
  }
  if (!s_sal_initialized) {
    return SOC_SAL_ERR_OFFLINE;
  }

  hal_i2c_status_t status = hal_i2c_read(s_soc_i2c_addr, REG_SOC_BAT_VOLT_H,
                                         buf, 2u, CONFIG_I2C_TIMEOUT_MS);

  if (status != HAL_I2C_OK) {
    soc_sal_bus_recovery();
    return convert_hal_status(status);
  }

  /* 假定读取到的是 12 位 ADC 值，基准转换单位为 1.25mV */
  uint16_t raw_val = (uint16_t)(((uint16_t)buf[0] << 8u) | buf[1]);
  *voltage_mv = (uint32_t)raw_val * 5u / 4u; /* raw_val * 1.25 */

  return SOC_SAL_OK;
}

soc_sal_status_t soc_get_current(int32_t *current_ma) {
  uint8_t buf[2];

  if (current_ma == 0) {
    return SOC_SAL_ERR_PARAM;
  }
  if (!s_sal_initialized) {
    return SOC_SAL_ERR_OFFLINE;
  }

  hal_i2c_status_t status = hal_i2c_read(s_soc_i2c_addr, REG_SOC_BAT_CURR_H,
                                         buf, 2u, CONFIG_I2C_TIMEOUT_MS);

  if (status != HAL_I2C_OK) {
    soc_sal_bus_recovery();
    return convert_hal_status(status);
  }

  int16_t raw_val = (int16_t)(((uint16_t)buf[0] << 8u) | buf[1]);
  *current_ma = (int32_t)raw_val * 10; /* 假定每 LSB 代表 10mA */

  return SOC_SAL_OK;
}

soc_sal_status_t soc_get_temperature(int16_t *temp_celsius) {
  uint8_t raw_temp;

  if (temp_celsius == 0) {
    return SOC_SAL_ERR_PARAM;
  }
  if (!s_sal_initialized) {
    return SOC_SAL_ERR_OFFLINE;
  }

  hal_i2c_status_t status = hal_i2c_read(s_soc_i2c_addr, REG_SOC_CHIP_TEMP,
                                         &raw_temp, 1u, CONFIG_I2C_TIMEOUT_MS);

  if (status != HAL_I2C_OK) {
    soc_sal_bus_recovery();
    return convert_hal_status(status);
  }

  /* 假定公式：摄氏度 = raw - 50 */
  *temp_celsius = (int16_t)raw_temp - 50;

  return SOC_SAL_OK;
}

soc_sal_status_t soc_set_ocp(soc_port_t port, uint32_t limit_ma) {
  uint8_t reg;
  uint8_t val;

  if (!s_sal_initialized) {
    return SOC_SAL_ERR_OFFLINE;
  }

  switch (port) {
  case SOC_PORT_TYPE_C1:
    reg = REG_SOC_PORT_C1_OCP;
    break;
  case SOC_PORT_TYPE_C2:
    reg = REG_SOC_PORT_C2_OCP;
    break;
  case SOC_PORT_TYPE_A1:
    reg = REG_SOC_PORT_A1_OCP;
    break;
  default:
    return SOC_SAL_ERR_PARAM;
  }

  /* 简易编码：每 500mA 对应 1 阶 */
  val = (uint8_t)(limit_ma / 500u);

  hal_i2c_status_t status =
      hal_i2c_write(s_soc_i2c_addr, reg, &val, 1u, CONFIG_I2C_TIMEOUT_MS);

  if (status != HAL_I2C_OK) {
    soc_sal_bus_recovery();
    return convert_hal_status(status);
  }

  if (board_has_uart_log()) {
    char msg[64];
    (void)snprintf(msg, sizeof(msg), "SAL: Set Port #%d OCP limit to %u mA\r\n",
                   (int)port, (unsigned int)limit_ma);
    board_uart_write_string(msg);
  }

  return SOC_SAL_OK;
}

soc_sal_status_t soc_set_port_power(soc_port_t port, uint16_t max_power_w) {
  /* 假定通过转换公式调整对应输出限额 */
  uint32_t ocp_limit =
      (uint32_t)max_power_w * 1000u / 5u; /* 假定 5V 下的等效电流 */
  return soc_set_ocp(port, ocp_limit);
}

soc_sal_status_t soc_poll_events(uint32_t *event_mask) {
  uint8_t raw_flags;

  if (event_mask == 0) {
    return SOC_SAL_ERR_PARAM;
  }
  if (!s_sal_initialized) {
    return SOC_SAL_ERR_OFFLINE;
  }

  hal_i2c_status_t status = hal_i2c_read(s_soc_i2c_addr, REG_SOC_EVENT_FLAGS,
                                         &raw_flags, 1u, CONFIG_I2C_TIMEOUT_MS);

  if (status != HAL_I2C_OK) {
    soc_sal_bus_recovery();
    return convert_hal_status(status);
  }

  *event_mask = (uint32_t)raw_flags;
  return SOC_SAL_OK;
}

void soc_sal_register_emergency_callback(soc_sal_emergency_callback_t callback) {
  s_emergency_callback = callback;
}

void board_soc_int_sim_trigger(void) {
#if CONFIG_ENABLE_SOC_INT_HIGHWAY
  if (board_has_uart_log()) {
    board_uart_write_string("SAL: Triggering SoC INT software pulse via EXTI SWIER...\r\n");
  }
  /* 调用 HAL EXTI 软件触发，实现 100% 软件闭环中断链路自测 */
  hal_exti_software_trigger(CONFIG_SOC_INT_PIN_INDEX);
#endif
}
