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

- **状态**：Open
- **优先级**：High
- **类型**：分层边界 / 可迁移性
- **代码位置**：
  - `openBmsClaw/drivers/soc_sal.h`
  - 当前现象：`soc_sal.h` include 了 `../hal/i2c/hal_i2c.h` 和 `../hal/exti/hal_exti.h`

### 现象

`soc_sal.h` 是面向 `app/` 和 `services/` 的 SoC 公共 API，但它直接暴露了 HAL 层头文件。这样上层 include `soc_sal.h` 时，会间接看到 MCU 外设实现细节。

### 影响

- 上层容易依赖 `hal_i2c` / `hal_exti` 类型或函数。
- 后续从 F103 迁移到 F030、G0 或国产兼容 MCU 时，SoC API 会被底层 HAL 改动牵连。
- 不符合新架构文档中的依赖矩阵：公共 SoC API 不应暴露 HAL 类型。

### 当前为何暂时接受

当前处于 F103 bring-up 阶段，SAL、I2C、EXTI 都在同一轮验证中推进，短期直接 include 能降低接线成本。

### 建议整改

1. 新增 `drivers/soc/soc_types.h`，只放平台标准状态、端口、事件和错误码。
2. 新增 `drivers/soc/soc_api.h`，作为 `app/` / `services/` 只能 include 的公共入口。
3. 将 HAL 相关 include 移入 `soc_sal.c` 或 `soc_transport_i2c.c`。
4. 若需要 transport 抽象，单独建立 `soc_transport.h`，不对 `app/` 暴露。

### 退出条件

- `app/` 和 `services/` include SoC API 时，不会间接 include `hal_i2c.h` 或 `hal_exti.h`。
- `soc_api.h` 只包含标准 C 头、`soc_types.h` 和平台领域 API。

---

## ISSUE-002：SoC SAL 仍是单文件 demo 实现，尚未 vendor adapter 化

- **状态**：Open
- **优先级**：High
- **类型**：SoC 解耦 / 多厂商适配
- **代码位置**：
  - `openBmsClaw/drivers/soc_sal.c`
  - 当前现象：demo 寄存器地址、I2C 地址、转换公式和平台 API 混在同一个文件中

### 现象

`soc_sal.c` 中同时包含：

- demo register：`REG_SOC_BAT_VOLT_H`、`REG_SOC_CHIP_TEMP` 等。
- 默认 I2C 地址：`0x75`。
- 电压、电流、温度、OCP 的假定转换公式。
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
3. 新增 adapter 接口，例如：

```c
typedef struct {
    soc_status_t (*init)(void);
    soc_status_t (*get_voltage_mv)(uint16_t port, uint16_t *voltage_mv);
    soc_status_t (*get_temperature_c)(int16_t *temperature_c);
    soc_status_t (*set_ocp_limit)(uint16_t port, uint16_t current_ma);
    soc_status_t (*poll_events)(soc_event_t *events, uint8_t max_count);
} soc_driver_ops_t;
```

4. `soc_manager.c` 只负责选择当前 adapter 和转发调用。

### 退出条件

- vendor 寄存器表不再出现在平台公共 SAL 文件中。
- 新增第二颗 SoC 时，不需要修改 `app/` 和 `services/`。

---

## ISSUE-003：`app/` 直接触发底层中断自测 hook

- **状态**：Open
- **优先级**：Medium
- **类型**：测试代码隔离 / 应用层边界
- **代码位置**：
  - `openBmsClaw/app/app.c`
  - `openBmsClaw/drivers/soc_sal.h`
  - `openBmsClaw/drivers/soc_sal.c`

### 现象

`app_init()` 在 `APP_ENABLE_POWER_SERVICE` 开启时，会打印测试日志并直接调用 `board_soc_int_sim_trigger()`。该函数声明在 `soc_sal.h`，实现位于 `soc_sal.c`，实际调用 `hal_exti_software_trigger()`。

### 影响

- `app/` 层混入 bring-up 自测动作，后续可能误触发 emergency lock。
- `board_soc_int_sim_trigger()` 命名属于 board/self-test 语义，却暴露在 SoC SAL 公共头文件中。
- 正式产品路径和测试路径没有隔离。

### 当前为何暂时接受

当前没有真实 SoC INT 引脚和故障注入硬件，软件触发 EXTI 可以验证高速告警链路是否接通。

### 建议整改

1. 新增 `services/bringup/` 或 `tests/selftest/` 下的自测入口。
2. 将 `board_soc_int_sim_trigger()` 改名为更明确的 `soc_int_selftest_trigger()` 或 `bringup_soc_int_selftest_run()`。
3. 使用独立宏控制，例如 `CONFIG_ENABLE_BRINGUP_SELFTEST`，默认生产 profile 关闭。
4. `app/` 只调度 bring-up service，不直接触发底层自测 hook。

### 退出条件

- `app/` 中不再出现 `*_sim_trigger()` 或 `*_selftest_*()` 调用。
- 软件触发 EXTI 只存在于 bring-up/self-test 路径。

---

## ISSUE-004：配置存在双真相，feature 与 app 开关含义未收敛

- **状态**：Open
- **优先级**：Medium
- **类型**：配置治理 / 可裁剪性
- **代码位置**：
  - `openBmsClaw/config/app_config.h`
  - `openBmsClaw/config/feature_config.h`
  - `openBmsClaw/config/sys_config.h`

### 现象

当前 `APP_ENABLE_POWER_SERVICE` 为 `1`，但 `FEATURE_ENABLE_POWER` 为 `0`。这会造成新成员难以判断 power 模块到底是启用、裁剪、还是仅启用测试路径。

### 影响

- 后续 F030 profile 裁剪时，可能出现编译启用和功能声明不一致。
- 构建资源评估会失真。
- 文档、计划和代码状态难以对应。

### 当前为何暂时接受

当前 power service 只承担 emergency callback 和自测链路，不等同于完整“智能功率分配”业务启用。

### 建议整改

1. 明确三类开关层级：
   - `PROFILE_*`：选择构建目标，如 `PROFILE_F103_BRINGUP`、`PROFILE_F030_MIN_PRODUCT`。
   - `FEATURE_*`：产品能力是否存在。
   - `APP_ENABLE_*`：当前 app 是否调度该服务。
2. 对 startup 阶段增加注释或宏名，例如 `APP_ENABLE_POWER_EMERGENCY_TEST`。
3. 避免 `FEATURE_ENABLE_POWER=0` 时仍默认运行完整 power service。

### 退出条件

- 每个功能只有一个权威启用来源。
- `F103_BRINGUP` 与 `F030_MIN_PRODUCT` profile 能清楚说明哪些模块会编译、哪些模块会运行。

---

## ISSUE-005：`services/power` 直接操作 board LED 和 busy wait

- **状态**：Open
- **优先级**：Medium
- **类型**：服务层与板级耦合 / 响应性
- **代码位置**：
  - `openBmsClaw/services/power/power_service.c`

### 现象

`power_service_handle_emergency_stop()` 直接调用 `board_status_led_set(false)`；`power_service_process()` 在 emergency lock 状态下调用 `board_status_led_toggle()` 和 `board_busy_wait(100000u)`。

### 影响

- power service 和当前 F103 学习板 LED 绑定，后续换板或换 UI 形式时需要改服务层。
- `board_busy_wait()` 会阻塞主循环，可能影响日志、watchdog、按键处理或其他服务。
- 现在用于表现 emergency 状态可以接受，但不适合作为长期策略。

### 当前为何暂时接受

当前阶段没有完整 UI service 和事件总线，LED 是最直接的可观测手段。

### 建议整改

1. 将 emergency 状态发布为系统事件，由 UI service 或 indicator service 负责显示。
2. 使用 tick / timer 驱动 LED 闪烁，不在 service 中 busy wait。
3. power service 只维护安全状态和策略，不直接绑定某个板载 LED。

### 退出条件

- `services/power` 不直接调用 `board_status_led_*()`。
- emergency 闪烁不阻塞主循环。

---

## ISSUE-006：安全快路径语义已建立，但物理切断职责仍是模拟

- **状态**：Open
- **优先级**：High
- **类型**：安全链路 / 硬件验证
- **代码位置**：
  - `openBmsClaw/services/power/power_service.c`
  - `openBmsClaw/drivers/soc_sal.c`

### 现象

当前 emergency callback 会锁定状态并关闭状态 LED，用注释表达“模拟切断物理开关”。真实 SoC INT、功率路径使能脚、限流动作和故障寄存器尚未上板验证。

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

建议按以下顺序清理，避免一次性重构过大：

1. **先收敛配置双真相**：明确 startup profile 与 power emergency test 的关系。
2. **隔离 self-test hook**：把 `app/` 中的软件 EXTI 触发移到 bring-up/self-test。
3. **清理 SAL 公共头文件**：不再暴露 HAL 头。
4. **拆分 SoC adapter**：真实 SoC 接入前先准备 `soc_api.h`、`soc_types.h` 和 demo adapter。
5. **替换 busy wait UI**：用 UI/indicator service 承担 emergency 闪烁。
6. **完成真实硬件验证**：SoC INT、故障寄存器、限流/关断动作、I2C 恢复。

## 3. 当前验证状态

- 本文为技术债记录，不代表已经完成整改。
- 本轮未运行新的编译或上板测试。
- 之前已知构建结果为：`RAM: 4184 B / 20 KB = 20.43%`，`FLASH: 31348 B / 64 KB = 47.83%`。
- 真实 SoC 通信、真实 INT 引脚、功率限流和故障注入仍未完成硬件验证。
