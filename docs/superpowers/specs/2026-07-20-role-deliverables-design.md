# 设计规格：角色交付物层（4-owner × 3 阶段矩阵 + 3 模板）

> 日期：2026-07-20
> 状态：Draft（设计规格，待落地）
> 性质：brainstorming 产出的设计规格，非最终产物
> 最终产物：`0_System/04.team_and_ownership.md` 新增一节 + `0_System/templates/` 下 3 个模板文件
> 关联：接团队框架（4-owner）；参考"另一 AI 的 6 角色 + Feature Ownership"提案，映射进 4 卡

---

## 1. 目标与背景

项目负责人痛点：**每个角色在"启动 / 开发中 / 验收测试"三阶段该输出什么**，需要合理的矩阵 + 缺失交付物的模板。

外部提案给了一套 6 角色（Product Owner / Chief Architect / Hardware Lead / Software Lead / Embedded Engineer / Validation）+ Feature Ownership + MVP-first 组织理念。经决策：**保留已提交的 4-owner 框架，只在其上加一层"阶段×交付物"**；6 角色作为参考映射进 4 卡，不重写框架、不新增第 5 卡。

同时明确：**"角色交付物模板"是产物不是方法** —— 不做成 skill（类别错误）。本次交付纯产物：一张矩阵（写进 04）+ 3 个缺失模板文件。

### 1.1 已确认输入（本次澄清锁定）

- 角色模型：**保留 4-owner，只加交付物层**（不用 6 角色重写、不加第 5 卡）。
- 阶段轴：**3 通用生命周期阶段** —— 启动 / 开发中 / 验收测试；Stage A–E 仍归 `stage-gate` skill。
- 产物形式：**扩展 `04.team_and_ownership.md`** 新增"阶段×交付物"节。
- 模板范围：**矩阵 + 只补缺失的模板**；已有交付物（PRD/架构/验收标准/编码规范/阶段门）只指向出处、不重建。
- 缺失模板 = **4 个**：Feature List、外部干系人登记、测试计划 / Validation Plan、硬件架构 / Power Tree（后者为审阅中追加：Chief Architect 的③硬件架构在 03 仅有双芯 ADR、无 power tree 文档，属真缺口）。
- 模板落地：新建 `0_System/templates/` 目录。
- **不新做 skill**。

### 1.2 6 角色 → 4 卡 映射

| 外部提案 6 角色 | 归入 4 卡 |
|----------------|-----------|
| Product Owner | 卡3（产品 / 数据 / 合规） |
| Chief Architect | **跨卡**：软件 / SoC 架构·API 契约·code review·技术债 → 卡1；硬件架构·power tree → 卡2 |
| Software Lead + Embedded Engineer（Feature Ownership） | 卡1（固件 + SoC） |
| Hardware Lead | 卡2（硬件 / 电源 / 热） |
| Validation | **不设第 5 卡**：各 owner 在"验收测试"列自验其交付物；整体测试计划 + gate 记录归卡4 |

平台约束（提案强调、与项目一致）：分层 / 接口 / Board 抽象 / Config 四项作为**架构约束**存在，不作为组织目标；不提前做多平台 / 多 SoC / 插件 / DI 等（呼应 `03.tech_architecture.md` ADR-002 与 scope-freeze）。此为背景理念，写进节引言一句话，不展开。

---

## 2. 交付物矩阵（写进 04 新增节）

行 = 4 owner，列 = 3 阶段。每格标 `[已有→出处]` / `[缺失→建模板]` / `[缺失→可后补]`。

| owner | 启动 | 开发中 | 验收测试 |
|-------|------|--------|---------|
| **卡1 固件+SoC** | 软件分层[已有 03 §4]·API 契约[已有 `soc_api.h` + interface-contract]·仓库/CI/编码规范[已有 AGENTS.md/CLAUDE.md] | 各 Feature 实现(drv→logic→api)·Feature 设计评审记录[缺失→可后补] | build 零告警 + `ninja -t deps` 证据·集成报告[缺失→可后补] |
| **卡2 硬件** | 硬件架构 / power tree / 选型[外包·部分] | 原理图 / PCB / BOM / bring-up[外包] | EMC / 热 / bring-up 报告[外包·需内评] |
| **卡3 产品** | PRD[已有 02]·MVP Scope[已有 00 §3]·验收标准[已有 02 §8]·**Feature List[缺失→建模板]** | 范围 / 优先级变更记录·数据模型 v0[规划中] | 产品验收(对 PRD 验收标准)·PSE 合规证据 |
| **卡4 协调** | 阶段门原则[已有 00 §11.3]·**外部干系人登记[缺失→建模板]**·**测试计划 / Validation Plan[缺失→建模板]** | 阶段门记录[stage-gate 产出]·英集芯 RFI[interface-contract 产出] | gate 记录[stage-gate]·资料沉淀[Stage E] |

矩阵后附一句：已有交付物只指向出处不重建；标"缺失→建模板"的 3 项见 `0_System/templates/`。

---

## 3. 4 个缺失模板（`0_System/templates/`）

模板均为 Markdown、中文、含四态标注习惯、含"如何用"一行说明。仅给结构骨架，不预填项目内容（模板是通用骨架，实际内容运行时填）。

> Chief Architect 4 项产出覆盖核对：①技术路线 → 已有 03 §2 ADR；②软件架构 → 已有 03 §4；④API 规范 → 已有 03 §5.3 + soc_api.h + interface-contract；③硬件架构 / power tree → 缺，见 §3.4。前三项只指向出处，不建模板。

### 3.1 `0_System/templates/feature_list_template.md`（卡3）
- 目的：MVP Feature 清单 + Feature Ownership 分配。
- 骨架列：`Feature | 描述 | 所属能力(Battery/PD/NFC/…) | Owner(端到端) | MVP 优先级(P0/P1/延后) | 依赖 | 验收关联(PRD §)`。
- 说明：每个 Feature 由一人端到端负责（driver→logic→api→test）；延后项对齐 scope-freeze。

### 3.2 `0_System/templates/stakeholder_register_template.md`（卡4）
- 目的：外部干系人登记（闭合 charter 体检 #13）。
- 骨架列：`对象 | 类型(客户/供应商/认证) | 角色与依赖 | 接口/联系与响应支持 | 影响度 | 沟通要求 | 关联(interface-contract RFI 等)`。
- 说明：至少登记英集芯（SoC 合作主线）与日本目标客户；英集芯行与 interface-contract 产出的 RFI 对接。

### 3.3 `0_System/templates/validation_plan_template.md`（卡4）
- 目的：从第一周开始的持续验证计划（非最后测试）。
- 骨架结构：`验证目标 | 测试类别(Power/Battery/NFC/温度保护/集成) | 用例概要 | 证据形式(build/deps/UART日志/逻辑分析仪抓包/资源占用) | 关联 Stage 验收标准 | 状态(三态)`。
- 说明：证据形式复用 stage-gate 既有词汇；与 Stage A–E 验收标准挂钩，但本模板按能力组织、不替代 stage-gate 的门评审。

### 3.4 `0_System/templates/hardware_architecture_template.md`（卡2 · Chief Architect 硬件侧）
- 目的：补 Chief Architect ③硬件架构 —— 03 仅有双芯 ADR-001、无系统框图与 power tree。
- 骨架结构：系统框图（MCU↔SoC↔外设）/ Power Tree 表 / 关键器件表 / 接口划分（硬件→软件 board 层边界）。
- 说明：卡2（外包）填、卡1 owner review 后进阶段门；与 ADR-001 一致；不含 PCB 布局（PCB 属卡2 开发中交付物）。

---

## 4. 04.team_and_ownership.md 的改动

**新增节（追加在现有 §4"与 skill 的接口"之后，编号顺延）：**
- **§5 阶段×交付物** —— 引言一句话（MVP-first 理念 + 平台四约束）；6→4 映射表（§1.2）；交付物矩阵（§2）；指向 `0_System/templates/` 的 3 模板。

**不改动：** 现有 §1–§4（说明段、4 张卡、兼岗映射、skill 接口）保持原样；卡内五字段不变。

---

## 5. 范围与非目标

**范围内**：
- `04.team_and_ownership.md` 新增 §5（映射表 + 交付物矩阵）。
- `0_System/templates/` 下 4 个模板文件（Feature List / 干系人登记 / 测试计划 / 硬件架构）。

**非目标（本次不做）**：
- 不用 6 角色重写 04、不加第 5 卡。
- 不为已有交付物（PRD/架构/验收标准/编码规范/阶段门）建模板。
- 不做新 skill（含 deliverable-scaffold）。
- 不实际填充模板内容（模板是空骨架）。
- 不改 4 张卡的五字段、不动其它 `0_System` 文档。

---

## 6. 验证方式

无测试运行器，验证为人工复审：
- **verify**：04 §5 含 6→4 映射表（Validation 不设第 5 卡、Architect 跨卡）+ 4×3 交付物矩阵；§1–§4 未被改动。
- **verify**：矩阵每格有 `[已有→出处]` / `[缺失→建模板]` / `[缺失→可后补]` 标注；4 个"缺失→建模板"项与 `0_System/templates/` 4 文件一一对应。
- **verify**：4 模板文件存在、为通用骨架（无预填项目内容）、含"如何用"说明；证据词汇复用 stage-gate（build/deps/UART/逻辑分析仪/资源/三态）。
- **verify**：Chief Architect 4 项产出覆盖 —— ①技术路线/②软件架构/④API 规范 指向 03（不建模板），③硬件架构由 `hardware_architecture_template.md` 补齐。
- **verify**：干系人登记模板能容纳英集芯行并与 interface-contract RFI 对接（闭合 charter 体检 #13）。
- **verify**：无新 skill 产生；风格与 `0_System/*.md` 一致（中文、四态、无绝对路径）。
