#include "drivers/soc/vendor/demo_soc.h"

#include <stdio.h>

#include "board/board.h"
#include "config/sys_config.h"
#include "hal/i2c/hal_i2c.h"

/* ============================================================================
 * demo / 基线 SoC 适配器 (例如英集芯系列通用演示寄存器)
 * ============================================================================
 * 本文件是唯一持有具体寄存器地址与转换公式的地方。平台公共 SAL/manager 不再
 * 出现任何 vendor 寄存器表 (见 ISSUE-002)。
 */

/* 通用演示寄存器地址 */
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

static uint8_t s_addr = 0x75u; /* 默认基线 I2C 地址 (英集芯常为 0x75) */

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

static soc_sal_status_t demo_init(uint8_t i2c_addr) {
  s_addr = i2c_addr;

  uint8_t chip_id = 0u;
  hal_i2c_status_t status = hal_i2c_read(s_addr, REG_SOC_CHIP_ID, &chip_id, 1u,
                                         CONFIG_I2C_TIMEOUT_MS);
  if (status != HAL_I2C_OK) {
    if (board_has_uart_log()) {
      board_uart_write_string(
          "SAL: SoC chip probe failed, offline or locked.\r\n");
    }
    return convert_hal_status(status);
  }

  if (board_has_uart_log()) {
    char msg[64];
    (void)snprintf(msg, sizeof(msg), "SAL: SoC(demo) init OK, ChipID=0x%02X\r\n",
                   (unsigned int)chip_id);
    board_uart_write_string(msg);
  }
  return SOC_SAL_OK;
}

static soc_sal_status_t demo_get_voltage_mv(uint32_t *voltage_mv) {
  uint8_t buf[2];
  hal_i2c_status_t status = hal_i2c_read(s_addr, REG_SOC_BAT_VOLT_H, buf, 2u,
                                         CONFIG_I2C_TIMEOUT_MS);
  if (status != HAL_I2C_OK) {
    return convert_hal_status(status);
  }

  /* 假定读取到的是 12 位 ADC 值，基准转换单位为 1.25mV */
  uint16_t raw_val = (uint16_t)(((uint16_t)buf[0] << 8u) | buf[1]);
  *voltage_mv = (uint32_t)raw_val * 5u / 4u; /* raw_val * 1.25 */
  return SOC_SAL_OK;
}

static soc_sal_status_t demo_get_current_ma(int32_t *current_ma) {
  uint8_t buf[2];
  hal_i2c_status_t status = hal_i2c_read(s_addr, REG_SOC_BAT_CURR_H, buf, 2u,
                                         CONFIG_I2C_TIMEOUT_MS);
  if (status != HAL_I2C_OK) {
    return convert_hal_status(status);
  }

  int16_t raw_val = (int16_t)(((uint16_t)buf[0] << 8u) | buf[1]);
  *current_ma = (int32_t)raw_val * 10; /* 假定每 LSB 代表 10mA */
  return SOC_SAL_OK;
}

static soc_sal_status_t demo_get_temperature_c(int16_t *temp_celsius) {
  uint8_t raw_temp;
  hal_i2c_status_t status = hal_i2c_read(s_addr, REG_SOC_CHIP_TEMP, &raw_temp,
                                         1u, CONFIG_I2C_TIMEOUT_MS);
  if (status != HAL_I2C_OK) {
    return convert_hal_status(status);
  }

  /* 假定公式：摄氏度 = raw - 50 */
  *temp_celsius = (int16_t)raw_temp - 50;
  return SOC_SAL_OK;
}

static soc_sal_status_t demo_set_ocp_ma(soc_port_t port, uint32_t limit_ma) {
  uint8_t reg;
  uint8_t val;

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
      hal_i2c_write(s_addr, reg, &val, 1u, CONFIG_I2C_TIMEOUT_MS);
  if (status != HAL_I2C_OK) {
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

static soc_sal_status_t demo_set_port_power_mw(soc_port_t port,
                                               uint32_t power_mw) {
  /* 假定 5V 下的等效电流：limit_ma = power_mw / 5 */
  return demo_set_ocp_ma(port, power_mw / 5u);
}

static soc_sal_status_t demo_poll_events(uint32_t *event_mask) {
  uint8_t raw_flags;
  hal_i2c_status_t status = hal_i2c_read(s_addr, REG_SOC_EVENT_FLAGS, &raw_flags,
                                         1u, CONFIG_I2C_TIMEOUT_MS);
  if (status != HAL_I2C_OK) {
    return convert_hal_status(status);
  }

  *event_mask = (uint32_t)raw_flags;
  return SOC_SAL_OK;
}

static const soc_driver_ops_t s_demo_soc_ops = {
    .init = demo_init,
    .get_voltage_mv = demo_get_voltage_mv,
    .get_current_ma = demo_get_current_ma,
    .get_temperature_c = demo_get_temperature_c,
    .set_ocp_ma = demo_set_ocp_ma,
    .set_port_power_mw = demo_set_port_power_mw,
    .poll_events = demo_poll_events,
};

const soc_driver_ops_t *demo_soc_get_ops(void) { return &s_demo_soc_ops; }
