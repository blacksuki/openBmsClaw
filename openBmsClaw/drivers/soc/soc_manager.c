#include "drivers/soc_sal.h"

#include <stdio.h>

#include "board/board.h"
#include "config/sys_config.h"
#include "drivers/soc/soc_driver.h"
#include "drivers/soc/vendor/demo_soc.h"
#include "hal/exti/hal_exti.h"
#include "hal/i2c/hal_i2c.h"

/* ============================================================================
 * SoC 管理层：实现对 app/services 暴露的公共 API，转发给当前 vendor 适配器。
 * ============================================================================
 * 本文件只负责：当前适配器选择、传输初始化、总线自愈、中断高速通道与紧急回调、
 * 单位口径适配 (公共 API 的瓦特 -> 适配器的毫瓦)。不持有任何 vendor 寄存器表。
 */

static const soc_driver_ops_t *s_ops = 0;
static uint8_t s_soc_i2c_addr = 0x75u;
static bool s_sal_initialized = false;
static uint32_t s_recovery_count = 0u;

/* 紧急中断上层回调句柄 */
static soc_sal_emergency_callback_t s_emergency_callback = 0;

static void soc_sal_exti_hardware_handler(void) {
  /* 硬件 EXTI 中断响应入口 (在 EXTI4 ISR 中执行)
   * 严禁执行任何慢速、带 while 等待的 I2C 动作！
   * 直接向服务层投递一个紧急报警信号
   */
  if (s_emergency_callback != 0) {
    s_emergency_callback(SOC_ALARM_SYS_ERROR);
  }
}

bool soc_sal_init(uint8_t i2c_addr) {
  s_soc_i2c_addr = i2c_addr;
  s_recovery_count = 0u;
  s_ops = demo_soc_get_ops(); /* 当前选择 demo/基线适配器 */

  if (!hal_i2c_init()) {
    if (board_has_uart_log()) {
      board_uart_write_string("SAL: I2C hardware init failed!\r\n");
    }
    s_sal_initialized = false;
    return false;
  }

  if (s_ops->init(s_soc_i2c_addr) != SOC_SAL_OK) {
    s_sal_initialized = false;
    return false;
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

soc_sal_status_t soc_get_voltage(uint32_t *voltage_mv) {
  if (voltage_mv == 0) {
    return SOC_SAL_ERR_PARAM;
  }
  if (!s_sal_initialized) {
    return SOC_SAL_ERR_OFFLINE;
  }

  soc_sal_status_t status = s_ops->get_voltage_mv(voltage_mv);
  if (status != SOC_SAL_OK) {
    soc_sal_bus_recovery();
  }
  return status;
}

soc_sal_status_t soc_get_current(int32_t *current_ma) {
  if (current_ma == 0) {
    return SOC_SAL_ERR_PARAM;
  }
  if (!s_sal_initialized) {
    return SOC_SAL_ERR_OFFLINE;
  }

  soc_sal_status_t status = s_ops->get_current_ma(current_ma);
  if (status != SOC_SAL_OK) {
    soc_sal_bus_recovery();
  }
  return status;
}

soc_sal_status_t soc_get_temperature(int16_t *temp_celsius) {
  if (temp_celsius == 0) {
    return SOC_SAL_ERR_PARAM;
  }
  if (!s_sal_initialized) {
    return SOC_SAL_ERR_OFFLINE;
  }

  soc_sal_status_t status = s_ops->get_temperature_c(temp_celsius);
  if (status != SOC_SAL_OK) {
    soc_sal_bus_recovery();
  }
  return status;
}

soc_sal_status_t soc_set_ocp(soc_port_t port, uint32_t limit_ma) {
  if (!s_sal_initialized) {
    return SOC_SAL_ERR_OFFLINE;
  }

  soc_sal_status_t status = s_ops->set_ocp_ma(port, limit_ma);
  if (status == SOC_SAL_ERR_HW_LOCKED) {
    soc_sal_bus_recovery();
  }
  return status;
}

soc_sal_status_t soc_set_port_power(soc_port_t port, uint16_t max_power_w) {
  if (!s_sal_initialized) {
    return SOC_SAL_ERR_OFFLINE;
  }

  /* 公共 API 以瓦特表达，向适配器统一口径转换为毫瓦 (见 §5.3) */
  soc_sal_status_t status =
      s_ops->set_port_power_mw(port, (uint32_t)max_power_w * 1000u);
  if (status == SOC_SAL_ERR_HW_LOCKED) {
    soc_sal_bus_recovery();
  }
  return status;
}

soc_sal_status_t soc_poll_events(uint32_t *event_mask) {
  if (event_mask == 0) {
    return SOC_SAL_ERR_PARAM;
  }
  if (!s_sal_initialized) {
    return SOC_SAL_ERR_OFFLINE;
  }

  soc_sal_status_t status = s_ops->poll_events(event_mask);
  if (status != SOC_SAL_OK) {
    soc_sal_bus_recovery();
  }
  return status;
}

void soc_sal_register_emergency_callback(soc_sal_emergency_callback_t callback) {
  s_emergency_callback = callback;
}

void soc_sal_int_selftest_trigger(void) {
#if CONFIG_ENABLE_SOC_INT_HIGHWAY
  if (board_has_uart_log()) {
    board_uart_write_string(
        "SAL: Triggering SoC INT software pulse via EXTI SWIER...\r\n");
  }
  /* 调用 HAL EXTI 软件触发，实现 100% 软件闭环中断链路自测 */
  hal_exti_software_trigger(CONFIG_SOC_INT_PIN_INDEX);
#endif
}
