# Repository Guidelines

## 1. 当前项目名称与目标
当前项目主名称统一为：

> `bmsClaw`

本仓库当前服务于一个明确目标：

> 构建一套面向中小移动电源厂商的开源电源管理 / BMS 平台方案，并以 `bmsClaw` 作为当前实施主线，逐步形成可扩展、可复用、可量产迁移的 STM32 平台。

当前系统目标可总结为：

1. **平台目标**
   面向中小厂商提供一套 **模块化、可裁剪、可二次开发** 的开源方案，而不是只交付一套固定板卡或单一 SKU。

2. **技术目标**
   平台架构默认围绕 **STM32 + 模块化硬件 + HAL + 可扩展软件框架** 展开，并明确区分：
   - `BMS`
   - `电源转换`
   - `协议管理`
   - `BLE / 通信`
   - `显示 / 交互`
   - `底座 / 托盘扩展`

3. **工程目标**
   采用“**先最小可运行验证，再逐步平台化**”的路线，先完成主控 bring-up、基础外设、单链路验证，再逐步接入 BMS、协议和功率协同。

4. **参考目标**
   `90.mini-Lite` 仅作为参考产品样本，用于帮助理解多口移动电源、底座/托盘、LED 状态机等需求，不是当前项目的直接实现目标。

---

## 2. 当前仓库结构
本仓库当前至少包含以下几层内容：

### 2.1 当前主项目层
- `[0_System/00.tech_archture.md](/Users/huoward/Project/101.bms/bmsClaw/0_System/00.tech_archture.md)`
- `[0_System/10.hardware_softare_frame.md](/Users/huoward/Project/101.bms/bmsClaw/0_System/10.hardware_softare_frame.md)`
- `[0_System/11.prepare_dev.md](/Users/huoward/Project/101.bms/bmsClaw/0_System/11.prepare_dev.md)`
- `[0_System/12.procurement_list.md](/Users/huoward/Project/101.bms/bmsClaw/0_System/12.procurement_list.md)`
- `[0_System/13.software_installation.md](/Users/huoward/Project/101.bms/bmsClaw/0_System/13.software_installation.md)`
- `[1_Plan/Plan-Week-0325.md](/Users/huoward/Project/101.bms/bmsClaw/1_Plan/Plan-Week-0325.md)`

这些文档描述的是 **`bmsClaw` 当前主线**：

- 当前项目名和工作边界
- 平台架构和模块边界
- 开发准备与软件安装
- 当前执行计划

凡涉及当前项目目标、工作顺序、文档口径和路径边界，优先以这些文档为准。

### 2.2 当前代码实现层
- `[openBmsClaw/CMakeLists.txt](/Users/huoward/Project/101.bms/bmsClaw/openBmsClaw/CMakeLists.txt)`
- `[openBmsClaw/Src/main.c](/Users/huoward/Project/101.bms/bmsClaw/openBmsClaw/Src/main.c)`

这是当前仓库内已经存在的 **STM32 生成工程**。目前可确认的事实包括：

- 工程名为 `openBmsClaw`
- 由 `STM32CubeIDE for Visual Studio Code` 自动生成
- 当前器件事实为 `STM32F103C8T6`
- 当前代码已经具备最小目录骨架和薄入口 `main.c`
- 当前学习实物统一按 `野火小智 STM32F103C8T6 核心板（双 USB 款）` 理解
- 该学习板带 `5-pin SWD`、`CH340C USB-UART`、`USB Device`、`4 LEDs` 和 `3 buttons`

它的价值在于：

- 提供当前可落地的 F103 学习与 bring-up 工程骨架
- 作为后续 bring-up 和分层重构的当前起点

它**不是**完整平台实现，也**不等于**最终芯片路线已完全定型。

这里需要明确区分三种角色：

- **当前学习/起步板**：`野火小智 STM32F103C8T6 核心板（双 USB 款）`
- **长期平台参考开发板**：`STM32G0` Nucleo 板，例如 `NUCLEO-G071RB`、`NUCLEO-G0B1RE`
- **量产 MCU 选型**：最终应落到具体 MCU，例如 `STM32G0` 系列器件，而不是 `NUCLEO-*` 开发板

例如：

- `NUCLEO-G0B1RE` 是开发板型号
- `STM32G0B1RE` 是板上 MCU 型号
- `STM32G030C8T6` 则更接近后续可评估的低成本量产 MCU 候选

### 2.3 自动生成物层
- `build/`

这是当前工程的自动生成构建目录。默认规则：

- 视为生成物
- 不作为架构事实来源
- 非必要不手工编辑

### 2.4 参考项目层
- `[90.mini-Lite/miniLite_system_design.md](/Users/huoward/Project/101.bms/bmsClaw/90.mini-Lite/miniLite_system_design.md)`

这是一个 **参考项目**，用于帮助理解：

- 多口输出能力
- 底座 / 托盘充电逻辑
- LED 与按键行为
- 温度保护
- 多口功率限制

它的作用是 **参考样本**，不是当前 `bmsClaw` 的主规格源头。

### 2.5 外部参考资料层
- `91.reference/`

这里存放的是外部安装包、原始资料、压缩包和第三方参考件。默认规则：

- 作为原始资料归档
- 不视为当前项目实现
- 除非用户明确要求，不主动改写其内容

---

## 3. 文档优先级
涉及需求、设计和当前工作判断时，按以下顺序理解：

1. `[0_System/00.tech_archture.md](/Users/huoward/Project/101.bms/bmsClaw/0_System/00.tech_archture.md)`  
   当前项目名称、目标、仓库边界和工作路径说明。

2. `[0_System/10.hardware_softare_frame.md](/Users/huoward/Project/101.bms/bmsClaw/0_System/10.hardware_softare_frame.md)`  
   当前最核心的模块化硬件 / 软件框架说明。

3. `[1_Plan/Plan-Week-0325.md](/Users/huoward/Project/101.bms/bmsClaw/1_Plan/Plan-Week-0325.md)` 及其 `item` 文档  
   当前执行计划与本周动作。

4. `[0_System/11.prepare_dev.md](/Users/huoward/Project/101.bms/bmsClaw/0_System/11.prepare_dev.md)`、`12`、`13` 系列  
   当前开发准备、采购和工具链路线。

5. `[openBmsClaw/](/Users/huoward/Project/101.bms/bmsClaw/openBmsClaw)`  
   当前代码事实来源。

6. `[90.mini-Lite/miniLite_system_design.md](/Users/huoward/Project/101.bms/bmsClaw/90.mini-Lite/miniLite_system_design.md)`  
   仅作为参考项目样本。

7. `91.reference/`  
   仅作为外部资料归档。

如果这些信息之间冲突：

- **当前项目主线** 以 `0_System/*.md` 和 `1_Plan/*.md` 为准
- **当前代码事实** 以 `openBmsClaw/` 为准
- **参考样本行为** 仅在明确标注“参考”时，才从 `90.mini-Lite/` 借鉴

回复中必须明确区分：

- 已实现
- 当前代码事实
- 规划中
- 参考项目
- 推荐路线

---

## 4. 当前工作路径
当前更合理的工作路径如下：

### 4.1 先澄清边界
先明确：

- `bmsClaw` 是当前主项目
- `90.mini-Lite` 是参考项目
- `build/` 是生成物
- `91.reference/` 是外部参考资料

不要把参考项目误写成当前实现，不要把生成物误写成源码结构。

### 4.2 先稳定当前 STM32 工程
当前最现实的起点是先围绕现有生成工程推进：

1. 主控 bring-up
2. GPIO / LED
3. UART / 日志
4. ADC / I2C
5. 单链路验证

这里的口径需要固定为：

- 当前**学习与最小验证**先围绕 `野火小智 STM32F103C8T6 核心板（双 USB 款）` + 外部 `SWD` 调试器推进
- 当前**长期平台参考路线**围绕 `STM32G0` 展开
- 当前**企业量产选型**应最终落到具体 MCU，而不是 Nucleo 开发板

### 4.3 先做结构，再做复杂功能
在当前工程基础上，优先规划并逐步引入：

- `app/`
- `services/`
- `hal/`
- `drivers/`
- `board/`
- `config/`

不要一开始把产品逻辑、板级细节和协议控制全部堆进 `main.c`。

### 4.4 参考项目只提炼能力，不直接照搬
从 `90.mini-Lite` 中主要提炼：

- 端口角色
- 底座 / 托盘模式
- LED / 按键状态机
- 温度与保护规则

但不要默认：

- 沿用它的芯片选型
- 沿用它的目录结构
- 沿用它的产品规格作为当前项目主规格

---

## 5. Codex 工作准则
Codex 在本仓库中工作时，应遵守以下原则：

### 5.1 先判断任务类型
先判断请求属于哪一类：

- 文档梳理 / 重写
- 架构规划
- 当前 STM32 工程分析
- 当前 STM32 工程修改
- 开发准备与采购
- 参考项目抽取

### 5.2 默认做小步、可验证改动
严格选择规则：

- 最小范围重构
- 对应结构化文档更新
- 单一模块边界澄清
- 单一功能点验证建议

禁止一次性做大而散的改动。

### 5.3 不伪造完成状态
若没有实际完成：

- 编译
- 下载
- 板测
- 协议联调
- 温升验证

则必须明确写出“未执行”及原因。

### 5.4 文档必须能落地
对于系统文档，Codex 不应只写概念性表达。  
优先写出：

- 模块职责
- 接口边界
- 当前工作路径
- 验证项
- 代码与参考目录边界

### 5.5 输出要对新手友好
如果内容面向新手，应默认做到：

- 首次出现的缩写要解释
- 结论先说，原理后说
- 步骤按顺序编号
- 每项建议说明“为什么”
- 避免把多个新概念堆在一段话里

---

## 6. 嵌入式开发新手友好规则
本仓库的文档和说明应尽量适合嵌入式开发新手阅读，因此默认遵守以下规则：

### 6.1 先解释角色，再解释细节
例如：

- `BMS`：负责电池监测、保护、均衡
- `电源转换`：负责升降压和功率路径
- `协议模块`：负责 PD / QC / UFCS 等协商
- `board`：负责当前板子的引脚和外设绑定

### 6.2 先给“做事顺序”
说明一个复杂主题时，优先给出：

1. 先准备什么
2. 再验证什么
3. 最后扩展什么

### 6.3 不默认读者懂硬件调试
如果提到：

- ST-LINK
- I2C
- UART
- ADC
- DRP
- Sink
- Power Path

应尽量给一句用途说明。

### 6.4 用“现象-原因-建议”表达
尤其在 review、排错或架构建议中，优先使用：

- 现象是什么
- 为什么会这样
- 建议如何做

---

## 7. 当前工程现实约束
从仓库现状可确认：

- 当前有一个已生成的 STM32 起步工程，路径为 `openBmsClaw/`
- 当前该工程处于最小模板状态，代码量很少
- `build/` 是自动生成目录
- `90.mini-Lite` 是参考项目，不是当前实现
- `91.reference/` 中存在外部资料和安装包

因此：

- 不要把 `90.mini-Lite` 写成当前主工程
- 不要把 `build/` 里的内容写成需要长期维护的源码
- 不要承诺当前代码已经具备完整模块化框架
- 不要在没有验证的情况下宣称当前 MCU 路线已经完全定型

---

## 8. 修改代码时的规则
若任务落到当前 STM32 工程，遵守以下约束：

- 优先修改 `openBmsClaw/Src/`、`openBmsClaw/Inc/` 和必要的工程配置文件
- 谨慎修改自动生成的 `.settings/`、`cmake/vscode_generated.cmake`
- 非必要不直接编辑 `build/`
- 不为“修格式”整体重排 STM32 生成文件
- 任何涉及芯片型号、启动文件、链接脚本的修改，都要先确认依据

代码风格保持当前 STM32 工程基础风格：

- C 代码以简洁、明确为主
- 不在最小 bring-up 阶段引入无必要的复杂抽象
- 新增抽象时优先围绕模块能力命名

---

## 9. 文档更新规则
下列变更通常应同步更新文档：

- 当前项目名称、边界变化：更新 `0_System/00.tech_archture.md`
- 模块边界变化：更新 `0_System/10.hardware_softare_frame.md`
- 开发路线、工具链、采购变化：更新 `0_System/11-13` 系列文档
- 当前执行计划变化：更新 `1_Plan/*.md`
- 协作方式、约束变化：更新 `AGENTS.md`

更新文档时，优先保持：

- 术语统一
- 主线一致
- 当前项目和参考项目分离
- 不重复定义冲突内容

---

## 10. 构建、验证与交付
当前仓库中的实际构建路径是当前 STM32 生成工程：

- 工程目录：`openBmsClaw/`
- 工程名：`openBmsClaw`
- 构建输出目录：`openBmsClaw/build/`

但对当前仓库更重要的是：

- 架构文档是否一致
- 当前工作路径是否清晰
- 当前 STM32 起步工程是否稳定
- 参考项目是否被正确隔离

如果任务涉及硬件逻辑，回复中应明确建议验证项，例如：

- LED 状态机
- 按键短按 / 长按
- ADC / NTC 采样
- BMS 采样和保护
- 电源转换链路
- 协议协商
- 底座识别
- 托盘模式限制

---

## 11. 工作树说明
不要假设这里已经有成熟的研发基础设施。除非实际看到，否则不要默认存在：

- 完整 CI
- 稳定测试流水线
- 已完成板级联调
- 已验证量产配置

更合适的交付方式是：

- 说明改了哪些文件
- 说明改动目的
- 说明影响范围
- 说明已验证和未验证部分
- 说明仍然受哪些条件限制

---

## 12. 给 Codex 的最终要求
在这个仓库里，Codex 的最佳行为不是“快速写很多内容”，而是：

1. 先对齐当前项目主线 `bmsClaw`
2. 再识别当前代码事实、参考项目和生成物边界
3. 用最小、最清晰、最可验证的方式推进
4. 对新手保持解释性
5. 对架构保持边界感
6. 对未验证内容保持诚实

只要遵守这六点，输出就会同时符合：

- 当前开发路线
- Codex 的协作最佳实践
- 嵌入式开发新手的阅读需要
