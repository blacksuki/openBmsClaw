#ifndef OPEN_BMS_CLAW_CONFIG_APP_CONFIG_H
#define OPEN_BMS_CLAW_CONFIG_APP_CONFIG_H

#include "profile_config.h"

/* ============================================================================
 * APP_ENABLE_* : 当前 app 是否调度该服务 (运行轴)，由当前 PROFILE 派生
 * ============================================================================
 * 服务实际是否运行 = APP_ENABLE_*  &&  对应 FEATURE_ENABLE_* (见 app.c)。
 * 二者同源于 PROFILE，因此始终一致。
 */

#if defined(PROFILE_F103_BRINGUP)

#define APP_ENABLE_BRINGUP_SERVICE       1    /* 底层 Bring-up 自检 (UART/ADC/I2C/INT 自测) */
#define APP_ENABLE_UI_SERVICE            1    /* 极简 UI (状态灯心跳) */
#define APP_ENABLE_BMS_SERVICE           0
#define APP_ENABLE_POWER_EMERGENCY_TEST  1    /* 仅紧急回调 + ISSUE-003 自测链路，非完整功率分配 */
#define APP_ENABLE_PROTOCOL_SERVICE      0

#elif defined(PROFILE_F030_MIN_PRODUCT)

#define APP_ENABLE_BRINGUP_SERVICE       0    /* 量产不跑探针自测 */
#define APP_ENABLE_UI_SERVICE            1
#define APP_ENABLE_BMS_SERVICE           0    /* 待样片验证 */
#define APP_ENABLE_POWER_EMERGENCY_TEST  0    /* 量产关闭自测；真实功率分配为后续独立开关 */
#define APP_ENABLE_PROTOCOL_SERVICE      0

#endif

/* 示例工程独立测试开关 (与 PROFILE 无关，主线恒关) */
#define APP_ENABLE_TEMPERATURE_ALARM_SAMPLE 0 /* 温度超限告警演示工程开关，主线联调时关闭 */

#endif /* OPEN_BMS_CLAW_CONFIG_APP_CONFIG_H */
