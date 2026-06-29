#ifndef OPEN_BMS_CLAW_DRIVERS_SOC_SOC_DRIVER_H
#define OPEN_BMS_CLAW_DRIVERS_SOC_SOC_DRIVER_H

#include <stdint.h>

#include "soc_types.h"

/* ============================================================================
 * SoC vendor adapter 接口 (内部，仅 soc_manager 与各 vendor 适配器使用)
 * ============================================================================
 * 每颗 SoC 提供一份 soc_driver_ops_t 实现，封装自己的寄存器表与转换公式。
 * soc_manager 只持有当前 adapter 指针并转发调用——新增第二颗 SoC 时，
 * 只需新增一个 vendor 适配器并在 manager 中选择，不需改动 app/ 与 services/。
 *
 * 单位口径统一为 mV / mA / mW (见 00.tech_architecure.md §5.3)。
 */
typedef struct {
  soc_sal_status_t (*init)(uint8_t i2c_addr);
  soc_sal_status_t (*get_voltage_mv)(uint32_t *voltage_mv);
  soc_sal_status_t (*get_current_ma)(int32_t *current_ma);
  soc_sal_status_t (*get_temperature_c)(int16_t *temp_celsius);
  soc_sal_status_t (*set_ocp_ma)(soc_port_t port, uint32_t limit_ma);
  soc_sal_status_t (*set_port_power_mw)(soc_port_t port, uint32_t power_mw);
  soc_sal_status_t (*poll_events)(uint32_t *event_mask);
} soc_driver_ops_t;

#endif /* OPEN_BMS_CLAW_DRIVERS_SOC_SOC_DRIVER_H */
