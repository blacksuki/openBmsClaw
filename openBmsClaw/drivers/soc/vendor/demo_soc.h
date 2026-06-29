#ifndef OPEN_BMS_CLAW_DRIVERS_SOC_VENDOR_DEMO_SOC_H
#define OPEN_BMS_CLAW_DRIVERS_SOC_VENDOR_DEMO_SOC_H

#include "drivers/soc/soc_driver.h"

/**
 * @brief 获取 demo/基线 SoC 适配器的操作表
 * @note demo 适配器使用通用演示寄存器与假定转换公式，用于真实 SoC 接入前的
 *       bring-up 验证。接入英集芯/智融等真实 SoC 时新增对应 vendor 适配器即可。
 */
const soc_driver_ops_t *demo_soc_get_ops(void);

#endif /* OPEN_BMS_CLAW_DRIVERS_SOC_VENDOR_DEMO_SOC_H */
