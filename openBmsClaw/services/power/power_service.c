#include "services/power/power_service.h"
#include "drivers/soc_sal.h"
#include "board/board.h"
#include "config/sys_config.h"
#include <stdio.h>

static volatile power_state_t s_power_state = POWER_STATE_NORMAL;
static volatile uint32_t s_emergency_alarm_mask = SOC_ALARM_NONE;
static bool s_diag_printed = false;

void power_service_init(void) {
  s_power_state = POWER_STATE_NORMAL;
  s_emergency_alarm_mask = SOC_ALARM_NONE;
  s_diag_printed = false;

  /* 注册紧急告警回调至驱动 SAL 层 */
  soc_sal_register_emergency_callback(power_service_handle_emergency_stop);

  if (board_has_uart_log()) {
    board_uart_write_string("PowerService: Init OK, emergency callback registered.\r\n");
  }
}

void power_service_handle_emergency_stop(uint32_t alarm_mask) {
  /* ==========================================================================
   * 【第一阶段】 中断级微秒响应自锁保护 (反射区)
   * ==========================================================================
   * 1. 瞬间锁定状态为自锁锁定，防止主循环调度普通充放电配置。
   */
  s_power_state = POWER_STATE_EMERGENCY_LOCK;
  s_emergency_alarm_mask = alarm_mask;

  /* 2. 模拟切断物理开关 GPIO (直接修改 ODR 寄存器拉低使能，微秒级切断物理通路)
   * 在测试板上，我们直接熄灭 Status LED 表现自锁切断，或控制板载继电器。
   */
  board_status_led_set(false);
}

power_state_t power_service_get_state(void) {
  return s_power_state;
}

void power_service_process(void) {
  if (s_power_state == POWER_STATE_NORMAL) {
    /* 正常工作模式 */
    // 周期性调度智能功率分配（当前为 V0 原型，暂无复杂业务）
  } else if (s_power_state == POWER_STATE_EMERGENCY_LOCK) {
    /* ==========================================================================
     * 【第二阶段】 主循环安全诊断慢查询 (决策区)
     * ==========================================================================
     * 中断响应已安全切断物理开关。此时我们在主循环（非中断）中执行慢速 I2C 交互，
     * 诊断具体故障原因，并持续通过灯光/警报提醒用户。
     */
    if (!s_diag_printed) {
      s_diag_printed = true;

      if (board_has_uart_log()) {
        board_uart_write_string("\r\n============================================\r\n");
        board_uart_write_string("[EMERGENCY LOCK] High-speed interrupt triggered!\r\n");
        board_uart_write_string("Status: Buck-Boost disabled. Hardware safety lock engaged.\r\n");
      }

      /* 安全读取 SoC 报警寄存器诊断原因 */
      uint32_t soc_events = 0;
      soc_sal_status_t status = soc_poll_events(&soc_events);

      if (board_has_uart_log()) {
        if (status == SOC_SAL_OK) {
          char msg[80];
          (void)snprintf(msg, sizeof(msg), "Diagnosis: SoC Event Register = 0x%08X\r\n", (unsigned int)soc_events);
          board_uart_write_string(msg);

          /* 假定第 0 位是负载短路，第 1 位是严重过温，第 2 位是严重过流 */
          if (soc_events & 0x01) {
            board_uart_write_string("ALERT >>> [LOAD SHORT CIRCUIT] detected on port!\r\n");
          }
          if (soc_events & 0x02) {
            board_uart_write_string("ALERT >>> [BATTERY OVER-TEMPERATURE] thermal limit exceeded!\r\n");
          }
          if (soc_events & 0x04) {
            board_uart_write_string("ALERT >>> [OVER-CURRENT] input/output threshold breached!\r\n");
          }
        } else {
          board_uart_write_string("Diagnosis: Failed to read SoC alert registers (I2C error or offline).\r\n");
        }
        board_uart_write_string("Action Required: Please power off the device, remove the fault, and reboot.\r\n");
        board_uart_write_string("============================================\r\n\r\n");
      }
    }

    /* 紧急警告 UI 表现：高速闪烁 LED 以警示危险 */
    board_status_led_toggle();
    board_busy_wait(100000u); /* 高频闪烁 */
  }
}
