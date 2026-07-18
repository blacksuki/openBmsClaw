# openBattery 执行记录与状态追踪 (Week-0518)

> 创建时间：2026-05-18
> 最新更新时间：2026-07-01

本文档用于完整记录本周（Week-0518）围绕“打通 MCU 与高集成度 SoC 双向通信基线”总目标的具体行动落实情况。本周严格遵循“脑体协同、资源受限（F103 64KB）裁剪、防 I2C 死锁”的开发纪律。

---

## 1. 总体进度摘要 (Summary)
- **当期进展**：已完成 F103 bring-up 阶段的基础分层、`hal_i2c` 超时与 bus recovery、`drivers/soc/` adapter 骨架、配置三层模型以及部分 startup 技术债整改。代码已从单文件 `soc_sal.c` 演进为 `soc_api.h / soc_types.h / soc_manager.c / vendor/demo_soc.c` 结构。
- **本地工具链状态**：`STM32CubeCLT` 已完成安装，`arm-none-eabi-gcc`、`cmake` 与 `ninja` 可用；当前 `openBmsClaw` 工程已完成 `cmake --preset Debug` 配置与 `cmake --build --preset Debug` 构建，成功生成 `.elf`、`.hex` 与 `.bin` 产物。
- **当前构建资源占用**：最新已知构建结果为 `RAM: 4200 B / 20 KB = 20.51%`，`FLASH: 32708 B / 64 KB = 49.91%`。较早期版本略有增长，主要来自 `soc_driver_ops_t` 适配器结构、`soc_manager.c` 与 `vendor/demo_soc.c` 拆分后的函数表与独立函数开销。
- **硬件联调状态**：仍未形成真实 SoC 上板记录。当前已有 I2C probe、自测 hook 和 demo adapter，但 `soc_sal_init()` 尚未进入运行路径，因此 SoC 初始化和真实 INT 初始化都还未在运行时发生。

---

## 2. 本周行动项清单与完成详情 (Action Items Status)

### Action Item 01: 确立 F103 资源裁剪方案
- **计划定位**：Day 1
- **当前状态**：✅ **已完成 (Completed)**
- **执行详情与输出物**：
  1. 在 `openBmsClaw/config/` 下建立了全系统统一配置入口 `sys_config.h`。
  2. 针对 STM32F103 64KB Flash 平台，设立并默认关闭了 `CONFIG_ENABLE_OLED_DISPLAY`（OLED渲染引擎）、`CONFIG_ENABLE_LARGE_FONT`（中日韩大字库）以及 `CONFIG_ENABLE_COMPLEX_UI_ANIM`（复杂动画）等裁剪开关。
  3. 在 `feature_config.h` 和 `app_config.h` 中，明确启用了状态指示灯、UART 日志输出以及底层外设探针，显式关闭当期未联调的 BMS、Power、Protocol 业务模块。
  4. 对 `app.c`、`board.c` 及 `bringup_service.c` 进行了头文件包含重构，全部切换为直接引用同级或归一化包含。
- **新手与架构提示**：通过这种集中式宏定义隔离，团队成员及新进开发者可以清晰识别 V0 阶段的核心关注点为底层双向数据信道，有效防范早期 Flash 与内存越界。

### Action Item 02: 跟踪 SoC 评估板与逻辑分析仪到货
- **计划定位**：持续跟踪
- **当前状态**：⏳ **进行中 (In Progress)**
- **执行详情**：开发团队正积极跟进英集芯等集成 SoC 评估板及逻辑分析仪的物流情况，为后续实测做前置准备。

### Action Item 03: 搭建全新的双核分层工程骨架
- **计划定位**：Day 1-2
- **当前状态**：✅ **已完成 (Completed)**
- **执行详情与输出物**：
  1. 彻底清除了早期基于散件拆分思维遗留在 `hal/` 下的冗余目录 (`bms/`, `display/`, `power/`, `protocol/`, `sense/`)。
  2. 按照最新双核分工与抽象层职责，重构并明确了 `hal/` 作为 MCU 内部基础硬件驱动封装层的地位。
  3. 更新了 `hal/README.md`，对齐防死锁规范与通用外设句柄定义。

### Action Item 04: 编写强鲁棒性的 `hal_i2c` 驱动
- **计划定位**：Day 2
- **当前状态**：✅ **已完成 (Completed)**
- **执行详情与输出物**：
  1. 在 `hal/i2c/` 下设计并实现了 [hal_i2c.h](file:///Users/huoward/Project/11.openBattery/openBmsClaw/openBmsClaw/hal/i2c/hal_i2c.h) 和 [hal_i2c.c](file:///Users/huoward/Project/11.openBattery/openBmsClaw/openBmsClaw/hal/i2c/hal_i2c.c)。
  2. **带防死锁特性**：在所有读写轮询（SB, ADDR, TXE, RXNE, BTF）中均引入了基于时钟周期的 Timeout 超时退出与总线错误位（BERR/AF）自检，杜绝了无超时 `while` 死循环挂死系统的问题。
  3. **总线自愈 (Bus Recovery)**：实现了 `hal_i2c_bus_recovery` 函数。当检测到总线忙碌或锁死时，自动切换 SCL/SDA 为通用开漏输出模式，模拟发送 9 个时钟脉冲释放总线，随后发出 STOP 信号并自动软复位重新初始化 I2C 控制器。
  4. 已将 `hal/i2c/hal_i2c.c` 成功添加至 [CMakeLists.txt](file:///Users/huoward/Project/11.openBattery/openBmsClaw/openBmsClaw/CMakeLists.txt) 编译源文件列表中。

### Action Item 05: 在 `drivers/` 抽象 SAL 层与防呆自愈机制
- **计划定位**：Day 3
- **当前状态**：🟡 **部分完成 (Partially Completed)**
- **执行详情与输出物**：
  1. 公共 API 已拆为 `drivers/soc/soc_api.h` 与 `drivers/soc/soc_types.h`，上层不再通过 `soc_sal.h` 传递性看到 HAL 头文件。
  2. `soc_sal.c` 已拆分为 [soc_manager.c](file:///Users/huoward/Project/11.openBattery/openBmsClaw/openBmsClaw/drivers/soc/soc_manager.c) 与 [vendor/demo_soc.c](file:///Users/huoward/Project/11.openBattery/openBmsClaw/openBmsClaw/drivers/soc/vendor/demo_soc.c)，demo 寄存器表已移出公共 SAL。
  3. **SAL 层自愈协调与诊断日志**：`soc_sal_bus_recovery` 仍保留，manager 会在 adapter 返回错误后触发 bus recovery。
  4. 已建立 `soc_driver_ops_t` 函数表机制，形成真正的 adapter 雏形。
  5. 但 `soc_sal_init()` 仍未进入运行路径，因此当前只能说明“结构与构建完成”，不能说明“初始化已打通”。

### Action Item 06: 集成 SoC 基线通信实板联调
- **计划定位**：评估板到货即开始
- **当前状态**：⏸️ **未开始 (等待硬件就位)**
- **真实度声明**：本地交叉编译已经完成，当前已具备 `.elf`、`.hex` 与 `.bin` 固件产物；但仍未执行真实 SoC 的下载、上板和实测。当前甚至还未在运行路径中调用 `soc_sal_init()`，因此不应将 demo adapter 视为 SoC 联调已开始。

### Action Item 07: 设计“紧急告警高速通道”
- **计划定位**：Day 4
- **当前状态**：🟡 **结构已完成，运行时未完全接通 (Structurally Done, Runtime Not Fully Wired)**
- **执行详情与输出物**：
  1. 在 `config/sys_config.h` 中启用了 `CONFIG_ENABLE_SOC_INT_HIGHWAY`，并定义了当前中断引脚索引。
  2. 在 `hal/exti/` 下实现了 EXTI 初始化、软件触发与 `EXTI4_IRQHandler` 中断服务链路。
  3. `drivers/soc/soc_manager.c` 已包含 SoC 中断入口与上层紧急回调注册逻辑。
  4. `services/power/power_service.c` 已完成“快路径只锁存状态、慢路径再诊断”的结构，并移除了 LED 直控和 busy wait。
  5. 但由于 `soc_sal_init()` 尚未被实际调用，`hal_exti_init()` 当前并未在运行时接通该链路；现阶段仅能说明软件结构存在，不能说明中断初始化已真正执行。

### Action Item 08: 周末复盘与文档回写
- **计划定位**：周末
- **当前状态**：✅ **已完成 (Completed)**
- **执行详情**：
  1. `0_System/03.tech_architecure.md` 已升级为可执行的顶层架构约束文档。
  2. `2_Action/Action-Week-0518-startup-tech-debt-issues.md` 已记录 startup 技术债并跟踪部分 issue 的整改状态。
  3. `0_System/Course/` 已回写中断快/慢路径、配置三层模型、验证分层、接口泄露等教学内容。

---

## 3. 已验证与未验证说明 (Verification Summary)
- **已验证**：
  1. 工程目录规范、配置宏的语法层面对齐、头文件层级依赖关系（已处理相对路径与同级引用机制）。
  2. 本地 STM32 交叉编译工具链可用，`arm-none-eabi-gcc`、`cmake`、`ninja` 均已通过命令行检查。
  3. `openBmsClaw` 已完成 `Debug` 配置与构建，成功生成 `openBmsClaw.elf`、`openBmsClaw.hex` 和 `openBmsClaw.bin`。
  4. `drivers/soc/` adapter 结构、配置三层模型和部分技术债整改已通过构建层验证。
- **未验证**：
  1. 单片机实物下载与板测启动。
  2. I2C 总线真实电气波形捕获。
  3. SoC 寄存器握手通信与真实电压/温度读取。
  4. SoC 紧急中断引脚、限流动作和故障注入场景。
  5. `soc_sal_init()` 接入运行路径后的完整 `SWIER -> EXTI -> callback -> power lock` 闭环。
