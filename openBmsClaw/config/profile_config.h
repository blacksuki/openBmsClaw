#ifndef OPEN_BMS_CLAW_CONFIG_PROFILE_CONFIG_H
#define OPEN_BMS_CLAW_CONFIG_PROFILE_CONFIG_H

/* ============================================================================
 * 构建 Profile 选择 (配置的单一权威来源)
 * ============================================================================
 * FEATURE_* (能力是否编译) 与 APP_ENABLE_* (app 是否调度) 全部由当前 PROFILE 派生，
 * 二者由同一来源生成，因此不可能再出现 feature 与 app 开关含义不一致的"配置双真相"
 * (见 ISSUE-004 与 00.tech_architecure.md §7)。
 *
 * 规则：以下 PROFILE_* 有且仅有一个被定义。切换量产候补时改这一处即可。
 */

#define PROFILE_F103_BRINGUP            /* 当前学习板与基础外设验证 */
/* #define PROFILE_F030_MIN_PRODUCT */  /* F030 最小量产候补 (需 Phase 3 样片验证) */

#if (defined(PROFILE_F103_BRINGUP) + defined(PROFILE_F030_MIN_PRODUCT)) != 1
#error "必须且只能定义一个 PROFILE_* (PROFILE_F103_BRINGUP 或 PROFILE_F030_MIN_PRODUCT)"
#endif

#endif /* OPEN_BMS_CLAW_CONFIG_PROFILE_CONFIG_H */
