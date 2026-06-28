#ifndef OPEN_BMS_CLAW_DRIVERS_SOC_SAL_H
#define OPEN_BMS_CLAW_DRIVERS_SOC_SAL_H

#include "soc/soc_api.h"

/* ============================================================================
 * SoC SAL bring-up 入口
 * ============================================================================
 * 平台标准类型见 soc/soc_types.h，公共领域 API 见 soc/soc_api.h。
 * 本头文件只额外暴露 startup 阶段的自测 hook；它不依赖任何 HAL 头，
 * 因此 app/ 与 services/ 包含本头时不会间接看到 MCU 外设实现细节。
 *
 * 注：soc_sal_int_selftest_trigger() 是 bring-up 自测 hook，仅由 bringup_service
 * 在 CONFIG_ENABLE_BRINGUP_SELFTEST 开启时调用，app/ 不再直接触发 (见 ISSUE-003)。
 */

/**
 * @brief SAL 层自测：往 STM32 EXTI SWIER 写位，软件触发一次 SoC INT 高速通道
 * @note 仅用于 bring-up/self-test 路径，在无实物硬件时执行 100% 软件闭环中断自测；
 *       受 CONFIG_ENABLE_SOC_INT_HIGHWAY 与调用方 CONFIG_ENABLE_BRINGUP_SELFTEST 约束
 */
void soc_sal_int_selftest_trigger(void);

#endif /* OPEN_BMS_CLAW_DRIVERS_SOC_SAL_H */
