# openBattery 执行记录与状态追踪 (Week-0518)

> 创建与更新时间：2026-05-18

本文档用于完整记录本周（Week-0518）围绕“打通 MCU 与高集成度 SoC 双向通信基线”总目标的具体行动落实情况。本周严格遵循“脑体协同、资源受限（F103 64KB）裁剪、防 I2C 死锁”的开发纪律。

---

## 1. 总体进度摘要 (Summary)
- **当期进展**：已顺利完成 **Day 1 至 Day 3** 核心战役，建立了 `config/` 系统配置基线，拉起底层带防死锁自愈的 `hal_i2c` 驱动，并在 `drivers/` 下成功构建了跨 SoC 厂商的标准抽象转译层 (SAL) 与诊断协调机制。
- **硬件联调状态**：评估板与逻辑分析仪正持续跟进采购到货状态，实际 I2C 上板通信测试处于等待前置硬件就位状态。

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
- **当前状态**：✅ **已完成 (Completed)**
- **执行详情与输出物**：
  1. 在 `drivers/` 下构建了跨 SoC 厂商的标准抽象转译层 (SAL)：[soc_sal.h](file:///Users/huoward/Project/11.openBattery/openBmsClaw/openBmsClaw/drivers/soc_sal.h) 与 [soc_sal.c](file:///Users/huoward/Project/11.openBattery/openBmsClaw/openBmsClaw/drivers/soc_sal.c)。
  2. **统一标准 API 抹平差异**：暴露了一致的 `soc_get_voltage()`, `soc_get_current()`, `soc_get_temperature()`, `soc_set_ocp()`, `soc_poll_events()` 接口，业务层完全脱离底层具体芯片的寄存器依赖。
  3. **SAL 层自愈协调与诊断日志**：实现了 `soc_sal_bus_recovery` 机制。当底层读写返回总线错误或超时锁死时，SAL 层自动接管并触发 GPIO 模拟 9 个时钟脉冲释放总线，同时向 UART 终端打印出带计数追踪的实时警告日志。
  4. 移除了空置冗余的 `gpio/`, `tim/`, `uart/` 辅助目录，更新了 [drivers/README.md](file:///Users/huoward/Project/11.openBattery/openBmsClaw/openBmsClaw/drivers/README.md)，保留起步探针。
  5. 已将 `soc_sal.c` 添加至 `CMakeLists.txt`。

### Action Item 06: 集成 SoC 基线通信实板联调
- **计划定位**：评估板到货即开始
- **当前状态**：⏸️ **未开始 (等待硬件就位)**
- **真实度声明**：未执行编译下载与上板实测，需等待专用 SoC 评估板与逻辑分析仪就位后方可展开总线波形抓包与终端数据跳动校验。

### Action Item 07: 设计“紧急告警高速通道”
- **计划定位**：Day 4
- **当前状态**：⏸️ **未开始 (Not Started)**
- **后续规划**：定义硬件中断引脚直达 `services` 限流回调的快速响应链路。

### Action Item 08: 周末复盘与文档回写
- **计划定位**：周末
- **当前状态**：⏸️ **未开始 (Not Started)**

---

## 3. 已验证与未验证说明 (Verification Summary)
- **已验证**：工程目录规范、配置宏的语法层面对齐、头文件层级依赖关系（已处理相对路径与同级引用机制）。
- **未验证**：单片机实物交叉编译二进制文件、I2C 总线真实电气波形捕获、SoC 寄存器握手通信（均受限于当期硬件物流到货进度）。
