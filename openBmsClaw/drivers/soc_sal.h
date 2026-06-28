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
 * 注：board_soc_int_sim_trigger() 是 bring-up 自测 hook，按 ISSUE-003 后续应
 * 迁入 bringup/self-test 路径并改名，本轮 (ISSUE-001) 不动其位置。
 */

/**
 * @brief 往 STM32 EXTI SWIER 软件中断寄存器写位，人为模拟触发一次硬件 INT 低电平脉冲
 * @note 用于在无实物硬件环境下执行 100% 软件闭环中断自测
 */
void board_soc_int_sim_trigger(void);

#endif /* OPEN_BMS_CLAW_DRIVERS_SOC_SAL_H */
