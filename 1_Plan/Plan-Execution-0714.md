# 开发执行计划：未完成项收口路线 (2026-07-14)

> 日期：2026-07-14
> 适用范围：承接 `Plan-Week-0518.md` 与 `Plan-Stage-Post-0518.md`
> 目标：把当前 6 项未完成工作，收敛为可执行、可验收、可持续跟踪的开发计划

## 1. 本文档目的

当前仓库已经完成了 `board / hal / drivers / services / app / config` 的基础结构，也完成了 `hal_i2c` timeout、bus recovery、`drivers/soc/` 公共 API 拆分等代码层准备。

但截至 `2026-07-14`，项目仍停留在“结构已成形、真实硬件链路未闭环”的状态。后续工作如果继续按零散 issue 或临时口头安排推进，容易出现以下问题：

1. `soc_sal_init()`、`hal_exti_init()`、真实 SoC 联调、逻辑分析仪抓包被混在一起，缺少前后依赖顺序。
2. `srv_power_alloc`、`srv_thermal`、故障状态机可能在没有真实 SoC 输入的前提下空转。
3. `PROFILE_F030_MIN_PRODUCT` 容易过早并行推进，打断当前 F103 + SoC 基线收口。

因此，本文档的目的不是再写一轮“方向性规划”，而是把未完成项拆成三条主线，并明确：

- 先做什么
- 后做什么
- 每一阶段改哪些文件
- 每一阶段的验收标准是什么
- 哪些事项因为硬件未到位而只能先做到一半

## 2. 当前未完成项

截至 `2026-07-14`，当前明确未完成的事项如下：

1. `soc_sal_init()` 还没有进入真实运行路径。
2. `hal_exti_init()` 尚未在实际运行中完成 SoC INT 初始化。
3. 没有真实 SoC 芯片联调记录。
4. 没有逻辑分析仪抓包记录。
5. `srv_power_alloc`、`srv_thermal`、故障状态机尚未形成最小闭环。
6. `PROFILE_F030_MIN_PRODUCT` 只有配置入口，没有真实验证结果。

## 3. 收敛原则

本阶段执行统一遵守以下原则：

1. 先收口真实 SoC 基线，再做服务层策略，再做 F030 样片验证。
2. 不把 demo adapter 当成真实 SoC 打通结果。
3. 不在 ISR 快路径加入 `printf`、I2C、delay。
4. 任何“已完成”都必须区分：
   - 已实现：代码结构或接口已存在
   - 当前代码事实：是否真的进入运行路径
   - 已验证：是否编译、串口、实板、抓包都已完成
5. 没有实际执行的构建、下载、板测、抓包，一律不能写成“已打通”。

## 4. 主线拆分与执行顺序

本轮开发计划不按 6 条待办平铺展开，而按 3 条主线推进：

```text
主线 A  SoC SAL 真实运行闭环
   -> 主线 B  服务层最小策略闭环
   -> 主线 C  F030 样片验证入口
```

推荐顺序固定为：

1. 先完成主线 A
2. 再进入主线 B
3. 最后进入主线 C

原因如下：

- 主线 A 解决的是“系统有没有真实 SoC 输入”的问题。
- 主线 B 解决的是“拿到真实输入后，服务层能不能做最小决策闭环”的问题。
- 主线 C 解决的是“当前方案能不能迁移到 F030 候选量产 MCU”的问题。

如果主线 A 没完成，主线 B 和主线 C 都会建立在不可靠前提上。

## 5. 主线 A：SoC SAL 真实运行闭环

> 逐任务执行清单：`1_Plan/Plan-StageA-Implementation-0714.md`

> 当前状态补充（2026-07-15）：
> - Task 1、Task 2、Task 3 已完成代码与构建层收口；
> - Task 4 仍未执行；
> - 当前尚无实板烧录记录、UART 实机日志记录和逻辑分析仪抓包记录，因此 `Stage A` 还不能写成“已完成”。

### 5.1 目标

把当前仅存在于代码结构中的 `SoC SAL`，推进到真实运行时闭环，至少形成一次完整的：

`I2C probe -> soc_sal_init -> hal_exti_init -> selftest / real SoC read -> 串口日志 / 抓包记录`

### 5.2 对应未完成项

- 第 1 项：`soc_sal_init()` 还没有进入真实运行路径
- 第 2 项：`hal_exti_init()` 尚未在实际运行中完成 SoC INT 初始化
- 第 3 项：没有真实 SoC 芯片联调记录
- 第 4 项：没有逻辑分析仪抓包记录

### 5.3 建议涉及文件

- `openBmsClaw/app/app.c`
- `openBmsClaw/services/bringup/bringup_service.c`
- `openBmsClaw/services/bringup/bringup_service.h`
- `openBmsClaw/services/power/power_service.c`
- `openBmsClaw/drivers/soc/soc_manager.c`
- `openBmsClaw/drivers/soc/vendor/demo_soc.c`
- `openBmsClaw/hal/exti/hal_exti.c`
- `openBmsClaw/config/sys_config.h`

### 5.4 执行步骤

1. 接通 `soc_sal_init()` 运行入口
   - 选择 `bringup_service` 作为当前 F103 bring-up 阶段的接线点。
   - 保持 `main.c` 只做 `board_init()` 与 `app_init()`，不把 SoC 细节硬塞回入口文件。
2. 调整 bring-up 顺序
   - 先做 `I2C probe`
   - 再调用 `soc_sal_init()`
   - 再执行 SoC INT 自测
   - 避免当前“只写 SWIER，但 EXTI 运行时未初始化”的假闭环
3. 补齐运行时日志
   - `SAL init start`
   - `SAL init ok / fail`
   - `EXTI init ok / fail`
   - `chip probe ok / fail`
   - `soc_poll_events offline / online`
4. 验证软件闭环
   - 至少确认一次 `SWIER -> EXTI -> callback -> power_service lock`
   - 自测通过后，再进入真实 SoC 联调
5. 完成真实 SoC 联调
   - 接入 SoC 评估板
   - 优先读取 `chip id`
   - 再扩展到 `voltage`、`temperature` 或 `event`
6. 完成逻辑分析仪抓包
   - 抓到一条真实 I2C 应答波形
   - 记录从机地址、寄存器地址、ACK/NACK、异常与恢复现象

### 5.5 验收标准

满足以下条件，才算主线 A 完成：

1. 串口能清晰显示 `soc_sal_init()` 成功或失败信息。
2. `hal_exti_init()` 已在运行时真正执行，而不是只存在于代码中。
3. `SWIER -> EXTI -> callback -> power_service` 至少完成一次闭环。
4. 至少读出一个真实 SoC 数据值：
   - `chip id`
   - 或 `voltage`
   - 或 `temperature`
5. 至少留下一份逻辑分析仪抓包记录。

### 5.6 风险与阻塞项

- 如果 SoC 评估板未到位，主线 A 只能完成“初始化接线 + 软件闭环自测”，不能写成“真实 SoC 已打通”。
- 如果逻辑分析仪未到位，真实读值也不能算完全闭环，只能写成“实板初步联调已开始”。

## 6. 主线 B：服务层最小策略闭环

### 6.1 目标

在真实 SoC 输入链路可用后，建立最小版的：

- `srv_thermal`
- `srv_power_alloc`
- 故障状态机

本阶段只追求“最小能工作”，不追求复杂算法或完整产品体验。

### 6.2 对应未完成项

- 第 5 项：`srv_power_alloc`、`srv_thermal`、故障状态机尚未形成最小闭环

### 6.3 建议涉及文件

- `openBmsClaw/services/power/power_service.c`
- `openBmsClaw/services/power/power_service.h`
- `openBmsClaw/services/bms/bms_service.c`
- `openBmsClaw/services/bms/bms_service.h`
- `openBmsClaw/drivers/soc/soc_api.h`
- `openBmsClaw/drivers/soc/soc_manager.c`
- `openBmsClaw/config/feature_config.h`
- `openBmsClaw/config/app_config.h`

### 6.4 执行步骤

1. 建立最小热输入来源
   - 当前优先复用 SoC 温度读取
   - 后续再扩展 NTC / ADC，不在本阶段并入复杂采样策略
2. 建立最小故障状态机
   - `NORMAL`
   - `DERATING`
   - `EMERGENCY_LOCK`
   - `RECOVER_WAIT` 或同等恢复态
3. 建立最小功率下发逻辑
   - 高温时执行降额
   - 严重事件时执行锁存
   - 通过 `soc_set_port_power()` 或 `soc_set_ocp()` 下发
4. 收口快路径与慢路径边界
   - ISR 只锁存
   - 主循环负责诊断、日志、恢复判断、参数下发
5. 收口单位口径
   - 公共 API 与内部 adapter 的 `W / mW` 口径统一清楚
   - 避免后续服务层按错误单位下发配置

### 6.5 验收标准

满足以下条件，才算主线 B 完成：

1. 温度或故障事件输入能触发一次真实降额或锁存。
2. 至少一条 SoC 参数下发动作可以观察到。
3. `services/` 层不直接包含 `hal_i2c.h` 或其他底层外设头文件。
4. 故障状态机状态迁移能通过串口日志或观察行为确认。

### 6.6 风险与边界

- 当前阶段不要做复杂功率分配数学模型。
- 当前阶段不要引入 BLE、复杂 UI、预测算法。
- 如果温度输入仍只能软件模拟，必须明确标记“流程验证已做，未板测”。

## 7. 主线 C：F030 样片验证入口

### 7.1 目标

把 `PROFILE_F030_MIN_PRODUCT` 从“只有配置入口”推进到“有构建记录、有差异清单、有基础样片验证”。

### 7.2 对应未完成项

- 第 6 项：`PROFILE_F030_MIN_PRODUCT` 只有配置入口，没有真实验证结果

### 7.3 建议涉及文件

- `openBmsClaw/config/profile_config.h`
- `openBmsClaw/config/board_config.h`
- `openBmsClaw/board/board.c`
- `openBmsClaw/board/board.h`
- `openBmsClaw/CMakeLists.txt`
- 后续新增的 F030 启动文件、链接脚本、板级适配文件

### 7.4 执行步骤

1. 先做构建差异审计
   - 不急着迁移业务层
   - 先确认 F103 与 F030 的启动文件、时钟、外设差异
2. 切换 `PROFILE_F030_MIN_PRODUCT`
   - 完整记录首次构建失败点
   - 明确哪些问题是配置、哪些问题是外设差异、哪些问题是板级文件缺失
3. 建立最小 F030 板级入口
   - 先保证下载、启动、串口日志
4. 再验证基础外设
   - I2C
   - ADC
   - EXTI
   - watchdog
5. 最后记录资源占用
   - FLASH
   - RAM

### 7.5 验收标准

满足以下条件，才算主线 C 完成：

1. `PROFILE_F030_MIN_PRODUCT` 至少完成一次构建验证。
2. 至少有一份 F103 / F030 差异清单。
3. 至少完成一次 F030 样片下载与串口输出。
4. 至少验证一条 F030 上的 I2C 通信链路。

### 7.6 风险与边界

- 不要默认 F030 与 F103 pin-to-pin 兼容。
- F030 验证失败，不代表架构失败，只代表量产 MCU 型号需要重新上移。

## 8. 推荐执行节奏

建议执行节奏如下：

| 顺序 | 主线 | 建议时间 | 说明 |
| --- | --- | --- | --- |
| 1 | 主线 A | 当前立即开始 | 不完成则后续都缺少真实输入 |
| 2 | 主线 B | 主线 A 完成后 | 建立最小服务层闭环 |
| 3 | 主线 C | 主线 B 收口后 | 把量产候补从文档推到工程事实 |

如果硬件到位延迟，允许按以下方式分裂推进：

1. 主线 A 前半段
   - 接通 `soc_sal_init()`
   - 跑通 EXTI 软件闭环
   - 补齐日志
2. 等 SoC 评估板与逻辑分析仪就位后，再完成主线 A 后半段
   - 真实 SoC 读值
   - 逻辑分析仪抓包

## 9. 与现有文档关系

本文档与现有文档的关系如下：

- `Plan-Week-0518.md`
  - 负责说明周计划、已完成项、未完成项与阶段切换背景
- `Plan-Stage-Post-0518.md`
  - 负责说明 Stage A 到 Stage E 的总体收敛路线
- `Plan-Execution-0714.md`
  - 负责把当前 6 条未完成项整理成真正可执行的开发任务顺序与验收标准

因此，后续如果需要继续追踪本轮开发进度，应优先回写本文档，而不是把执行细节继续塞回阶段总览文档。
