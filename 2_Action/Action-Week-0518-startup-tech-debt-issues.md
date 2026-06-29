# openBattery Startup 阶段技术债 Issues

> 创建时间：2026-06-28
> 适用范围：`openBmsClaw/` 当前 F103 bring-up 工程
> 关联架构：[0_System/00.tech_architecure.md](../0_System/00.tech_architecure.md)

本文记录当前代码中已经识别、但在 startup / bring-up 阶段暂时接受的架构偏差。它们不是当前路线错误，而是后续从 demo 骨架进入真实 SoC 联调、F030 候补验证和最小量产链路前必须逐步清理的技术债。

## 1. 总体判断

当前代码已经具备基础分层雏形：

- `app/` 负责应用入口和服务调度。
- `services/` 开始承接 power emergency state。
- `drivers/` 已有 SoC SAL API。
- `hal/` 已有 I2C timeout 与 bus recovery 思路。
- `board/` 承接当前 F103 学习板的 UART / LED / probe 封装。

但当前实现仍带有 startup 阶段的验证代码和边界穿透。进入真实 SoC 联调和 F030 量产候补验证前，应按本文 issue 逐项收敛。

---

## ISSUE-001：SoC SAL 公共头文件泄露 HAL 依赖

- **状态**：Resolved（2026-06-28 整改并通过构建验证，见文末「整改记录」）
- **优先级**：High
- **类型**：分层边界 / 可迁移性
- **代码位置**：
  - `openBmsClaw/drivers/soc_sal.h`
  - 当前现象：`soc_sal.h` include 了 `../hal/i2c/hal_i2c.h` 和 `../hal/exti/hal_exti.h`

### 现象

`soc_sal.h` 是面向 `app/` 和 `services/` 的 SoC 公共 API，但它直接暴露了 HAL 层头文件。这样上层 include `soc_sal.h` 时，会间接看到 MCU 外设实现细节。

代码依据：`soc_sal.h:9-10` 直接 `#include "../hal/i2c/hal_i2c.h"` 与 `"../hal/exti/hal_exti.h"`；而 `app/app.c:4`、`services/power/power_service.c:2` 都 include 了 `soc_sal.h`，已经**传递性**看到 HAL 类型——泄露是当前事实，不是理论风险。

### 影响

- 上层容易依赖 `hal_i2c` / `hal_exti` 类型或函数。
- 后续从 F103 迁移到 F030、G0 或国产兼容 MCU 时，SoC API 会被底层 HAL 改动牵连。
- 不符合新架构文档中的依赖矩阵：公共 SoC API 不应暴露 HAL 类型。

### 当前为何暂时接受

当前处于 F103 bring-up 阶段，SAL、I2C、EXTI 都在同一轮验证中推进，短期直接 include 能降低接线成本。

### 建议整改

1. 新增 `drivers/soc/soc_types.h`，把 `soc_sal_status_t`、`soc_port_t`、`SOC_ALARM_*` 掩码、`soc_sal_emergency_callback_t`（均不依赖 HAL）搬入。
2. 新增 `drivers/soc/soc_api.h`，作为 `app/` / `services/` 只能 include 的公共入口。
3. 将 HAL 相关 include 移入 `soc_sal.c` 或 `soc_transport_i2c.c`。
4. 若需要 transport 抽象，单独建立 `soc_transport.h`，不对 `app/` 暴露。

### 退出条件

- `app/` 和 `services/` include SoC API 时，不会间接 include `hal_i2c.h` 或 `hal_exti.h`。
- `soc_api.h` 只包含标准 C 头、`soc_types.h` 和平台领域 API。

> 依赖：本 issue 是 ISSUE-002 的前置——必须先有不泄露 HAL 的 `soc_types.h`，才能定义干净的 `soc_driver_ops_t`。

### 整改记录（2026-06-28）

实际改动（分支 `fix/issue-001-soc-sal-hal-leak`）：

- 新增 `drivers/soc/soc_types.h`：仅 `<stdint.h>` + 平台类型（`soc_sal_status_t`、`soc_port_t`、`SOC_ALARM_*`、`soc_sal_emergency_callback_t`），不含任何 HAL。
- 新增 `drivers/soc/soc_api.h`：仅 `<stdbool.h>`/`<stdint.h>` + `soc_types.h` + 领域 API 声明，不含 HAL。
- `drivers/soc_sal.h` 改为薄入口：`#include "soc/soc_api.h"`，移除 `hal_i2c.h`/`hal_exti.h` 与类型/API 定义；仅保留自测 hook `board_soc_int_sim_trigger()` 声明（其迁移归 ISSUE-003，本轮不动）。
- `drivers/soc_sal.c`：补回直接 include `../config/sys_config.h`、`../hal/i2c/hal_i2c.h`、`../hal/exti/hal_exti.h`（HAL 在 `.c` 内使用，符合 `00.tech §4.2`）。
- `services/power/power_service.c`：include 由 `drivers/soc_sal.h` 改为 `drivers/soc/soc_api.h`。

退出条件验证：

- 构建：`cmake --preset Debug && cmake --build --preset Debug` → 退出码 0，`-Wall -Wextra -Wpedantic` 下 0 warning；`RAM 4184 B / FLASH 31348 B` 与整改前一致（纯重构，无体积变化）。
- 依赖树：`ninja -t deps` 证明 `app/app.c.obj`、`services/power/power_service.c.obj` 的依赖中**不含** `hal_i2c.h`/`hal_exti.h`；`drivers/soc_sal.c.obj` 仍含 HAL（driver 层允许）。
- 两条退出条件均满足。未上板测试（本 issue 为编译期分层边界问题，不涉及硬件行为）。

---

## ISSUE-002：SoC SAL 仍是单文件 demo 实现，尚未 vendor adapter 化

- **状态**：Resolved（2026-06-29 整改并通过构建验证，见文末「整改记录」）
- **优先级**：High
- **类型**：SoC 解耦 / 多厂商适配
- **代码位置**：
  - `openBmsClaw/drivers/soc_sal.c`
  - 当前现象：demo 寄存器地址、I2C 地址、转换公式和平台 API 混在同一个文件中

### 现象

`soc_sal.c` 中同时包含：

- demo register：`REG_SOC_BAT_VOLT_H`、`REG_SOC_CHIP_TEMP` 等（`soc_sal.c:6-15`）。
- 默认 I2C 地址：`0x75`（`soc_sal.c:17`）。
- 电压、电流、温度、OCP 的假定转换公式（`soc_sal.c:130` `*5/4`、`:154` `*10`、`:178` `raw-50`、`:206` `/500`、`:229` `*1000/5`）。
- SAL 初始化、bus recovery、事件轮询和上层 API。

### 影响

- 第一颗 SoC 接入后，若继续往 `soc_sal.c` 里堆寄存器，会变成不可维护的多厂商混合文件。
- 英集芯、智融或其他 SoC 的寄存器差异会向平台公共 API 侵蚀。
- 后续“要接口不要源码”的接口边界会变弱。

### 当前为何暂时接受

当前还没有真实 SoC 评估板完成联调，demo register 有助于先验证 I2C read/write、错误处理和服务层调用链。

### 建议整改

1. 保留公共能力 API：`soc_get_voltage`、`soc_get_temperature`、`soc_set_ocp`、`soc_poll_events`。
2. 将 demo register 移入 `drivers/soc/vendor/demo_soc.c` 或 `vendor/injoinic_ip53xx.c`。
3. 新增 adapter 接口。签名按当前 `soc_sal.h` 实际 API + `00.tech_architecure.md §5.3` 单位口径（mV/mA/mW、功率 `uint32`）统一：

```c
typedef struct {
    soc_sal_status_t (*init)(uint8_t i2c_addr);
    soc_sal_status_t (*get_voltage_mv)(uint32_t *voltage_mv);
    soc_sal_status_t (*get_current_ma)(int32_t *current_ma);
    soc_sal_status_t (*get_temperature_c)(int16_t *temp_celsius);
    soc_sal_status_t (*set_ocp_ma)(soc_port_t port, uint32_t limit_ma);
    soc_sal_status_t (*set_port_power_mw)(soc_port_t port, uint32_t power_mw);
    soc_sal_status_t (*poll_events)(uint32_t *event_mask);
} soc_driver_ops_t;
```

> 注意：当前 `soc_set_port_power` 实参仍是 `uint16_t max_power_w`（`soc_sal.c:226`），adapter 化时一并收敛为 `power_mw`（uint32），与 §5.3 口径一致；否则又是一处单位双真相。

4. `soc_manager.c` 只负责选择当前 adapter 和转发调用。

### 退出条件

- vendor 寄存器表不再出现在平台公共 SAL 文件中。
- 新增第二颗 SoC 时，不需要修改 `app/` 和 `services/`。

### 整改记录（2026-06-29）

实际改动（分支 `fix/issue-002-soc-adapter`）：

- 新增 `drivers/soc/soc_driver.h`：`soc_driver_ops_t` 适配器函数表（mV/mA/mW 口径，§5.3）。
- 新增 `drivers/soc/vendor/demo_soc.{h,c}`：demo/基线适配器，**唯一**持有 `REG_SOC_*`、I2C 地址 `0x75` 与转换公式，实现 ops 并暴露 `demo_soc_get_ops()`。
- 新增 `drivers/soc/soc_manager.c`：实现公共 `soc_api.h`，转发给当前适配器；负责适配器选择、`hal_i2c` 传输初始化、总线自愈、EXTI 高速通道与紧急回调、`soc_sal_int_selftest_trigger`，以及公共 API 瓦特 → 适配器毫瓦的口径转换。
- 删除 `drivers/soc_sal.c`（内容拆入 manager + 适配器）；`CMakeLists.txt` 用 `soc/soc_manager.c` + `soc/vendor/demo_soc.c` 替换原 `soc_sal.c`。
- 公共头 `soc_api.h`、`soc_sal.h` 保持不变。

退出条件验证：

- `REG_SOC_*` 仅出现在 `vendor/demo_soc.c`，平台公共/manager 文件中已无 vendor 寄存器表。
- `app/`、`services/`、`soc_api.h` 本次零改动（`git diff` 确认）→ 新增第二颗 SoC 只需加一个 vendor 适配器并在 manager 选择，不动上层。
- 构建：`cmake --build --preset Debug` 退出码 0、0 warning；`RAM 4200 B / FLASH 32708 B`（较前 +8 B / +1160 B，来自适配器函数表与去内联开销）。
- 两条退出条件均满足。未上板（`soc_sal_init` 仍未被调用，与整改前一致，属 ISSUE-006 范围）。

---

## ISSUE-003：`app/` 直接触发底层中断自测 hook

- **状态**：Resolved（2026-06-28 整改并通过构建验证，见文末「整改记录」）
- **优先级**：Medium
- **类型**：测试代码隔离 / 应用层边界
- **代码位置**：
  - `openBmsClaw/app/app.c`
  - `openBmsClaw/drivers/soc_sal.h`
  - `openBmsClaw/drivers/soc_sal.c`

### 现象

`app_init()` 在 `APP_ENABLE_POWER_SERVICE` 开启时，会打印测试日志并直接调用 `board_soc_int_sim_trigger()`（`app/app.c:43-48`）。该函数声明在 `soc_sal.h:114`，实现位于 `soc_sal.c:259`，实际调用 `hal_exti_software_trigger()`。

### 影响

- `app/` 层混入 bring-up 自测动作，后续可能误触发 emergency lock。
- `board_soc_int_sim_trigger()` 命名属于 board/self-test 语义，却暴露在 SoC SAL 公共头文件中。
- 正式产品路径和测试路径没有隔离。

### 当前为何暂时接受

当前没有真实 SoC INT 引脚和故障注入硬件，软件触发 EXTI 可以验证高速告警链路是否接通。

### 建议整改

1. 复用现成的 `services/bringup/bringup_service.c`：它已是带 `BRINGUP_STAGE_ADC/I2C` 的阶段机（`bringup_service.c:96-119`），新增一个 `BRINGUP_STAGE_SOC_INT_SELFTEST` 即可承接，无需另起 `tests/`。
2. 将 `board_soc_int_sim_trigger()` 改名为 `bringup_soc_int_selftest()`，并把声明从 `soc_sal.h` 移除。
3. 使用独立宏控制，例如 `CONFIG_ENABLE_BRINGUP_SELFTEST`，默认生产 profile 关闭。
4. `app/` 只调度 bring-up service，不直接触发底层自测 hook。

### 退出条件

- `app/` 中不再出现 `*_sim_trigger()` 或 `*_selftest_*()` 调用。
- 软件触发 EXTI 只存在于 bring-up/self-test 路径。

### 整改记录（2026-06-28）

实际改动（分支 `fix/issue-003-isolate-selftest-hook`）：

- 新增单一来源宏 `CONFIG_ENABLE_BRINGUP_SELFTEST`（`config/sys_config.h`，F103 bring-up 默认 1，生产 profile 置 0）。
- `board_soc_int_sim_trigger()` 重命名为 `soc_sal_int_selftest_trigger()`：实现仍在 `soc_sal.c`（SAL 层拥有 INT 高速通道，服务层不直接碰 HAL），声明仍在 `soc_sal.h`，不再以 `board_` 自测语义混入。
- `bringup_service` 新增 `BRINGUP_STAGE_SOC_INT_SELFTEST` 阶段：在 `CONFIG_ENABLE_BRINGUP_SELFTEST && CONFIG_ENABLE_SOC_INT_HIGHWAY` 时，于探针阶段之后软件触发一次 INT 自测（`bringup_service.c` 调用 `soc_sal_int_selftest_trigger()`）。
- `app/app.c` 删除 `[Test]` 自测块与不再需要的 `#include "drivers/soc_sal.h"`，app 层只调度 `bringup_service`。

退出条件验证：

- 构建：`cmake --build --preset Debug` → 退出码 0，`-Wall -Wextra -Wpedantic` 下 0 warning；`RAM 4184 B / FLASH 31420 B`（较前 +72 B，新增阶段与日志字符串）。
- grep：`app/` 无 `*_sim_trigger`/`*_selftest_*` 调用；`hal_exti_software_trigger` 仅 `soc_sal.c` 调用，`soc_sal_int_selftest_trigger` 仅 `bringup_service.c` 调用 → 软件 EXTI 触发只在 bring-up/self-test 路径。
- 两条退出条件均满足。

> 旁注（不在本 issue 范围）：`soc_sal_init()` 目前未被 app/services 调用，故 `hal_exti_init()` 尚未把 EXTI4 回调接上；当前自测验证的是 SWIER 写入路径，而非完整中断回调链路。真实 INT 接线与初始化归 ISSUE-006。

---

## ISSUE-004：配置存在双真相，feature 与 app 开关含义未收敛

- **状态**：Resolved（2026-06-28 整改并通过构建验证，见文末「整改记录」）
- **优先级**：Medium
- **类型**：配置治理 / 可裁剪性
- **代码位置**：
  - `openBmsClaw/config/app_config.h`
  - `openBmsClaw/config/feature_config.h`
  - `openBmsClaw/config/sys_config.h`

### 现象

当前 `APP_ENABLE_POWER_SERVICE` 为 `1`，但 `FEATURE_ENABLE_POWER` 为 `0`。这会造成新成员难以判断 power 模块到底是启用、裁剪、还是仅启用测试路径。

代码依据：grep 确认 `FEATURE_ENABLE_POWER/BMS/PROTOCOL` 在 `services/`、`app/` 中**零引用**，构建路径只认 `app.c` 里的 `APP_ENABLE_*`（`app.c:21-32`、`:43-48`）。所以这不只是"两处状态不一致"，而是 `FEATURE_*` 当前是**纯装饰性死宏**、完全无效。

### 影响

- 后续 F030 profile 裁剪时，可能出现编译启用和功能声明不一致。
- 构建资源评估会失真。
- 文档、计划和代码状态难以对应。

### 当前为何暂时接受

当前 power service 只承担 emergency callback 和自测链路，不等同于完整“智能功率分配”业务启用。

### 建议整改

1. 先对死宏二选一：要么把 `FEATURE_ENABLE_POWER` 真正接进 `app.c`（`#if APP_ENABLE_POWER_SERVICE && FEATURE_ENABLE_POWER`），要么直接删除这些零引用宏，避免装饰性配置继续误导。
2. 明确三类开关层级：
   - `PROFILE_*`：选择构建目标，如 `PROFILE_F103_BRINGUP`、`PROFILE_F030_MIN_PRODUCT`。
   - `FEATURE_*`：产品能力是否存在。
   - `APP_ENABLE_*`：当前 app 是否调度该服务。
3. 将 `APP_ENABLE_POWER_SERVICE` 改名 `APP_ENABLE_POWER_EMERGENCY_TEST`，避免被当成"功率分配业务已启用"。
4. 避免 `FEATURE_ENABLE_POWER=0` 时仍默认运行完整 power service。

### 退出条件

- 每个功能只有一个权威启用来源。
- `F103_BRINGUP` 与 `F030_MIN_PRODUCT` profile 能清楚说明哪些模块会编译、哪些模块会运行。

### 整改记录（2026-06-28）

实际改动（分支 `fix/issue-004-config-profiles`）：

- 新增 `config/profile_config.h`：以单个 `PROFILE_*`（默认 `PROFILE_F103_BRINGUP`，备 `PROFILE_F030_MIN_PRODUCT`）作为配置唯一权威来源，并加 `#error` 保证有且仅有一个被定义。
- `config/feature_config.h`、`config/app_config.h` 改为按 PROFILE 派生 `FEATURE_*` / `APP_ENABLE_*`：二者同源，不可能再出现含义不一致的双真相。
- 死宏处理：`FEATURE_ENABLE_POWER/BMS/PROTOCOL` 由 `app.c` 真正接入（`#if APP_ENABLE_* && FEATURE_ENABLE_*`），不再零引用；`FEATURE_ENABLE_STATUS_LED/UART_LOG`（与 `board_has_*()` 能力查询重复的死宏）删除。
- 重命名 `APP_ENABLE_POWER_SERVICE` → `APP_ENABLE_POWER_EMERGENCY_TEST`，反映其当前仅承担紧急回调 + ISSUE-003 自测链路。
- `sys_config.h` 在 board/feature/app 之前先 include `profile_config.h`。

退出条件验证：

- 单一来源：`app/app.c` 现读取 `FEATURE_ENABLE_POWER/BMS/PROTOCOL`（行 17/20/23/51/54/57），死宏消除；`APP_ENABLE_POWER_SERVICE` 全仓无残留；`FEATURE_ENABLE_STATUS_LED/UART_LOG` 无引用。
- profile 可切换且都编译通过：F103（默认）`cmake --build --preset Debug` 退出码 0、0 warning、`RAM 4184 B / FLASH 31420 B`（与整改前一致，power 仍链接运行，ISSUE-003 自测链路保留）；临时切到 `PROFILE_F030_MIN_PRODUCT` 亦构建通过（退出码 0、0 warning，未调度服务被链接器回收，体积显著下降），验证后已切回 F103。
- 两条退出条件均满足。

---

## ISSUE-005：`services/power` 直接操作 board LED 和 busy wait

- **状态**：Resolved（2026-06-28 整改并通过构建验证，见文末「整改记录」）
- **优先级**：Medium
- **类型**：服务层与板级耦合 / 响应性
- **代码位置**：
  - `openBmsClaw/services/power/power_service.c`

### 现象

`power_service_handle_emergency_stop()` 直接调用 `board_status_led_set(false)`（`power_service.c:36`）；`power_service_process()` 在 emergency lock 状态下调用 `board_status_led_toggle()` 和 `board_busy_wait(100000u)`（`power_service.c:92-93`）。

同类债务还在 `ui_service.c:17`（`board_busy_wait(1600000u)`）。两者同处一个 `app_run` 循环（`app.c:62,68`），emergency 时每圈被 ui 的 1.6M 阻塞拖累，"高速闪烁"实际快不起来——所以本 issue 的整改要 power 和 ui **一起做**。

### 影响

- power service 和当前 F103 学习板 LED 绑定，后续换板或换 UI 形式时需要改服务层。
- `board_busy_wait()` 会阻塞主循环，可能影响日志、watchdog、按键处理或其他服务。
- 现在用于表现 emergency 状态可以接受，但不适合作为长期策略。

### 当前为何暂时接受

当前阶段没有完整 UI service 和事件总线，LED 是最直接的可观测手段。

### 建议整改

0. **前置**：当前 `board.h` 只有 `board_busy_wait()`，**没有任何 tick/时间基准**（全仓 grep 无 `get_tick`/SysTick）。必须先新增非阻塞时间基准，例如 `uint32_t board_get_tick_ms(void)`（SysTick），否则下面第 2 步无从落地。
1. 将 emergency 状态发布为系统事件，由 UI service 或 indicator service 负责显示。
2. 使用 `board_get_tick_ms()` 做差值翻转 LED，不在 service 中 busy wait；`power_service.c:93` 与 `ui_service.c:17` 两处一并替换。
3. power service 只维护安全状态和策略，不直接绑定某个板载 LED。

### 退出条件

- `services/power` 不直接调用 `board_status_led_*()`。
- emergency 闪烁不阻塞主循环（`board_busy_wait` 在 `power` 和 `ui` 中均被移除）。

### 整改记录（2026-06-28）

实际改动（分支 `fix/issue-005-tickbase-ui`）：

- 前置时间基准：`board.c` 新增 1ms SysTick（HSI 8 MHz，reload=7999，CLKSOURCE=处理器时钟、TICKINT 使能），强定义 `SysTick_Handler` 覆盖 startup 弱向量，`board_init()` 调用 `board_systick_init()`；`board.h` 暴露 `uint32_t board_get_tick_ms(void)`。
- `power_service`：快路径回调只自锁状态，删除 `board_status_led_set(false)`；`process()` 紧急分支保留一次性诊断，删除 `board_status_led_toggle()` 与 `board_busy_wait(100000)`，power 不再驱动 LED。
- `ui_service`：改为 tick 驱动非阻塞翻转——正常 500ms 心跳、紧急 100ms 快闪（在 `APP_ENABLE_POWER_EMERGENCY_TEST && FEATURE_ENABLE_POWER` 下读取 `power_service_get_state()`），删除 `board_busy_wait(1600000)`。

退出条件验证：

- 构建：`cmake --build --preset Debug` 退出码 0、0 warning；`RAM 4192 B / FLASH 31548 B`（较前 +8 B / +128 B，来自时基计数与 SysTick 逻辑）。
- grep：`services/power` 无 `board_status_led_*` 调用；`board_busy_wait` 在 `power` 与 `ui` 中均已移除。
- 符号：`SysTick_Handler`（强定义，地址独立于 `Default_Handler`）与 `board_get_tick_ms` 均在 elf 中。
- 两条退出条件均满足。

> 未上板：1ms 时基由 reload = HCLK/1000 - 1 计算保证、闪烁为非阻塞 tick 差值；实际时序与 LED 行为待上板确认。`ui→power` 的状态查询是当前最小耦合，后续可演进为事件/indicator 总线。

---

## ISSUE-006：安全快路径语义已建立，但物理切断职责仍是模拟

- **状态**：Open
- **优先级**：High
- **类型**：安全链路 / 硬件验证
- **代码位置**：
  - `openBmsClaw/services/power/power_service.c`
  - `openBmsClaw/drivers/soc_sal.c`

### 现象

当前 emergency callback 会锁定状态并关闭状态 LED，用注释表达“模拟切断物理开关”（`power_service.c:33-36`）。且中断入口 `soc_sal_exti_hardware_handler()` **恒投递 `SOC_ALARM_SYS_ERROR`**（`soc_sal.c:30`，与软件自测 `board_soc_int_sim_trigger` 对应），真实故障类型要等慢路径 `soc_poll_events()` 再解析（`power_service.c:65`）。真实 SoC INT、功率路径使能脚、限流动作和故障寄存器尚未上板验证。

### 影响

- 当前代码只能证明软件链路可走通，不能证明产品安全链路有效。
- 若文档或计划误写成“已具备保护能力”，会造成验证状态失真。
- PSE、温升、过流、短路等产品级验证不能基于模拟 LED 行为判断。

### 当前为何暂时接受

硬件评估板和故障注入条件尚未就位，当前只能先验证中断回调、状态锁定和慢路径诊断结构。

### 建议整改

1. 明确硬件安全职责：SoC / 保护芯片负责快速物理保护，MCU 负责锁存、降额、提示和恢复流程。
2. 等 SoC 评估板到货后，增加真实 INT 引脚、限流寄存器、输出关闭动作验证。
3. 将实测波形、串口日志和故障注入步骤记录到 `2_Action/`。

### 退出条件

- 至少完成一次真实 SoC INT 触发测试。
- 能证明 MCU 读取到真实故障码。
- 能证明功率路径由 SoC 或硬件保护机制安全降额或切断。

---

## 2. 后续清理顺序建议

根据代码核实，原顺序需两处修正：① ISSUE-005 缺少时间基准前置（当前无 tick）；② ISSUE-001（拆头）应排在 ISSUE-002（adapter）之前。修正后顺序：

| 序 | 动作 | 对应 issue | 依赖 |
| --- | --- | --- | --- |
| 1 | 收敛配置：删除或接入 `FEATURE_*` 死宏 + 建立 profile | ISSUE-004 ✅ 已完成（2026-06-28） | 无 |
| 2 | 隔离 self-test hook：软件 EXTI 触发移入 `bringup_service` | ISSUE-003 ✅ 已完成（2026-06-28） | 无 |
| 3 | **新增 `board_get_tick_ms()` 时间基准**（前置，原文档缺） | ISSUE-005 前置 ✅ 已完成（2026-06-28） | 无 |
| 4 | 替换 busy-wait UI：`power` 与 `ui` 两处一并改 tick 驱动 | ISSUE-005 ✅ 已完成（2026-06-28） | 依赖 3 |
| 5 | 拆 SAL 公共头：`soc_types.h` / `soc_api.h`，HAL include 下沉 | ISSUE-001 ✅ 已完成（2026-06-28） | 无 |
| 6 | SoC adapter 化：签名对齐 `00.tech_architecure.md §5.3` 口径 | ISSUE-002 ✅ 已完成（2026-06-29） | 依赖 5 |
| 7 | 真实硬件验证：SoC INT、故障寄存器、限流/关断、I2C 恢复 | ISSUE-006 | 依赖评估板 |

## 3. 当前验证状态

- 本文为技术债记录，ISSUE-001~005 已整改，仅剩 ISSUE-006（真实硬件验证，依赖评估板）Open。
- 2026-06-28 已对 6 个 issue 做**静态代码核实**（按 `file:line` 比对当前源码）。
- 2026-06-28 ISSUE-001 已整改并通过 `arm-none-eabi-gcc` 构建验证（Debug，退出码 0，0 warning），`ninja -t deps` 确认 `app/`、`services/` 不再间接包含 HAL；未上板（编译期边界问题）。
- 2026-06-28 ISSUE-003 已整改并通过构建验证（Debug，退出码 0，0 warning，FLASH 31420 B），grep 确认 `app/` 无自测 hook、软件 EXTI 触发仅在 bring-up 路径；未上板。
- 2026-06-28 ISSUE-004 已整改并通过构建验证：新增 `profile_config.h` 单一来源，F103 与 F030 两 profile 均 `cmake Debug` 退出码 0、0 warning；死宏消除、`APP_ENABLE_POWER_SERVICE` 改名完成。
- 2026-06-28 ISSUE-005 已整改并通过构建验证：新增 1ms SysTick 时基（`board_get_tick_ms`），`power` 不再驱动 LED、`power`/`ui` 的 `board_busy_wait` 移除，UI 改 tick 驱动（退出码 0、0 warning，RAM 4192 B / FLASH 31548 B）；未上板。
- 2026-06-29 ISSUE-002 已整改并通过构建验证：拆出 `soc_driver.h` + `vendor/demo_soc.c`（唯一持寄存器表）+ `soc_manager.c`，删除 `soc_sal.c`；`REG_SOC_*` 仅在 vendor，`app/`/`services/`/`soc_api.h` 零改动（退出码 0、0 warning，RAM 4200 B / FLASH 32708 B）；未上板。
- 本轮未运行新的编译或上板测试。
- 之前已知构建结果为：`RAM: 4184 B / 20 KB = 20.43%`，`FLASH: 31348 B / 64 KB = 47.83%`。
- 真实 SoC 通信、真实 INT 引脚、功率限流和故障注入仍未完成硬件验证。
