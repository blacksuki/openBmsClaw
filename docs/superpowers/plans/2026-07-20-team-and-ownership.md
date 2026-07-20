# 团队职责框架 Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 写出 `0_System/04.team_and_ownership.md` —— 一份概念性职责框架，按工作流分解为 4 个 owner 角色，每个五字段。

**Architecture:** 单文档交付。无代码、无测试运行器；验证 = 人工一致性检查（对照 spec §7 与 master 文档）。产物与设计 spec 一起在最后一步提交。

**Tech Stack:** Markdown（中文散文，风格对齐 `0_System/*.md`）。

## Global Constraints

- 语言：中文散文，与现有 `0_System/*.md` 一致；区分 **已确认 / 假设 / 待验证**。
- 不使用绝对路径（live 文档规范）。
- 文档头：`> 最新更新：2026-07-20`、`> 文档状态：Draft`、`> 文档性质：团队职责框架 / 概念性 owner 划分`。
- 角色卡固定五字段，顺序：**使命 / 边界 / 决策权 / 交付物 / 现阶段谁兼**。
- 概念性框架：不绑真人、不绑 AI agent；现实用"现阶段谁兼"标注。
- 卡2 现阶段谁兼 = **外包硬件开发团队（已在合作）**。
- 卡4 决策权 = **门禁 pass/打回，而非技术方案拍板**。
- 提交：本次不单独 commit spec；最终产物与设计 spec 一起提交（用户已确认）。

---

### Task 1: 写 `0_System/04.team_and_ownership.md`

**Files:**
- Create: `0_System/04.team_and_ownership.md`
- Reference: `docs/superpowers/specs/2026-07-20-team-and-ownership-design.md`（内容来源）
- Reference: `0_System/00.project_master.md`（§1.6 能力缺口、双芯架构、首发范围，用于一致性核对）

**Interfaces:**
- Consumes: 设计 spec §3（结构）、§4（4 张卡实质）、§5（skill 接口）、§1.1（锁定输入）。
- Produces: `0_System/04.team_and_ownership.md`，供后续 `stage-gate` / `interface-contract` / `project-charter` skill 引用 owner 定义。

- [ ] **Step 1: 写文档头 + 说明段**

写入文件头与第 1 节说明段：

```markdown
# openBattery 团队职责框架（概念性 owner 划分）

> 最新更新：2026-07-20
> 文档状态：Draft
> 文档性质：团队职责框架 / 概念性 owner 划分
> 适用范围：工作流分解、owner 角色边界、决策权、阶段门与接口评审归属

---

## 1. 说明

本文件定义 `openBattery` 当前设想下**应有哪些 owner 及其边界**，用于招人 / 外包 / 顾问时的对照。

- 这是**概念性占位框架**：**不绑定具体真人，也不绑定 AI agent 角色**。
- 现阶段一人多岗，各角色由"现阶段谁兼"字段标注现实。
- 框架主线为**工作流分解 + owner**：先按项目实际工作流切块，每块配一个 owner。
- 依据项目事实：MCU（Brain）+ 专用电源 SoC（Body，英集芯）双芯协同，MCU 仅经 `soc_api.h` 抽象层与 SoC 通信（"要接口不要源码"）；团队软件出身，硬件 / 电源 / 热 / 量产为已知能力缺口（见 `00.project_master.md` §1.6）。
```

- [ ] **Step 2: 写第 2 节 —— 4 张角色卡**

写入第 2 节，含工作流合并映射说明 + 4 张五字段卡（内容逐字采自 spec §4）：

```markdown
## 2. 角色卡（工作流分解 + owner）

工作流合并映射：卡1 = 固件 + SoC 集成；卡2 = 硬件 / 电源 / 热；卡3 = 产品 / 数据 / 合规（NFC + PSE + 市场）；卡4 = 项目协调 / 供应链。

### 卡1 · 固件 + SoC 集成 owner（Brain↔Body）
- **使命**：MCU 侧策略 / 日志 / UI + 英集芯 SoC 适配，守住 `soc_api.h` 抽象边界。
- **边界**：碰得了 `app`/`services`/`drivers/soc`/`hal`；碰不了在 app/service 里直接戳寄存器，碰不了硬件电路。
- **决策权**：固件架构、SoC 接口契约建议权；`vendor/<chip>.c` 寄存器表实现拍板权。
- **交付物**：固件架构、SAL 适配、编码规范、集成报告。
- **现阶段谁兼**：你（软件出身，主力）。

### 卡2 · 硬件 / 电源 / 热 owner
- **使命**：原理图、PCB、电源设计、热、EMC 及 PSE 相关硬件侧。
- **边界**：碰得了电路 / PCB / 选型；碰不了固件架构与业务策略。
- **决策权**：硬件方案拍板权（团队最大能力缺口，由外包团队承担，方案需内部评审确认）。
- **交付物**：原理图、PCB、BOM、热 / EMC 评审。
- **现阶段谁兼**：外包硬件开发团队（已在合作）。

### 卡3 · 产品 / 数据 / 合规 owner（NFC + PSE + 市场）
- **使命**：产品定义、竞品 / 价格带、NFC 数据模型 v0、PSE 认证路径。
- **边界**：碰得了需求 / 数据模型 / 合规策略；碰不了寄存器与电路实现。
- **决策权**：首发范围、数据模型 v0 拍板权；合规路径建议权（需认证机构确认）。
- **交付物**：产品定义、数据模型 v0、竞品 / 合规调研、待验证项清单。
- **现阶段谁兼**：你 + 待补市场 / 合规输入。

### 卡4 · 项目协调 / 供应链 owner
- **使命**：里程碑、stage gate 把关、决策记录、英集芯合作接口。
- **边界**：碰得了计划 / 门禁 / 对外接口；不替各 owner 做技术拍板。
- **决策权**：阶段门 通过 / 打回 拍板权；技术方案仅协调，不定技术方向。
- **交付物**：阶段计划、stage-gate 记录、供应链验收清单。
- **现阶段谁兼**：你。
```

- [ ] **Step 3: 写第 3 节兼岗映射 + 第 4 节 skill 接口**

写入第 3、4 节：

```markdown
## 3. 现阶段兼岗映射

现实提示（对照 §1.6 能力缺口）：

- 卡1（固件 + SoC）、卡3（产品 / 数据 / 合规）、卡4（协调）现由你一人兼。
- 卡2（硬件 / 电源 / 热）已由外包硬件开发团队承担；因团队硬件能力不足，其方案需内部评审确认后方可进入阶段门。
- 待验证 / 待补：市场与 PSE 合规输入尚缺专职来源（卡3），中长期为潜在补强点。

## 4. 与 skill 的接口

本框架是后续 skill 的 owner 锚点：

- `stage-gate`：阶段门由卡4 owner 把关（通过 / 打回）。
- `interface-contract`（SoC 契约评审）：由卡1 owner review，卡4 owner 记录。
- `project-charter`：引用本框架 owner 定义，不重复定义角色。
```

- [ ] **Step 4: 一致性核对（verify）**

对照检查，逐条确认：
- 文档能回答五问：每块工作流谁负责 / 边界在哪 / 谁拍板 / 交付什么 / 现在谁兼。
- 卡2 明确标注"外包硬件开发团队（已在合作）"；卡4 决策权明确限定为门禁而非技术拍板。
- 与 `00.project_master.md` §1.6（能力缺口）、双芯架构、`soc_api.h` 边界表述一致，无冲突。
- 风格与 `0_System/*.md` 一致：中文、状态标注齐全、无绝对路径。
- 五字段顺序在 4 张卡中一致；概念性框架声明存在。

发现不符则就地修正。

- [ ] **Step 5: 提交（产物 + 设计 spec + 计划一起）**

在默认分支 main 上，先开分支再提交（仓库规则）：

```bash
git checkout -b docs/team-ownership
git add 0_System/04.team_and_ownership.md docs/superpowers/specs/2026-07-20-team-and-ownership-design.md docs/superpowers/plans/2026-07-20-team-and-ownership.md
git commit -m "docs: add team & ownership framework (concept roles) + design spec

Co-Authored-By: Claude Opus 4.8 <noreply@anthropic.com>"
```

---

## Self-Review

**1. Spec coverage:**
- spec §2（方案 A 产物形态）→ Task 1 创建 `0_System/04.team_and_ownership.md`。✅
- spec §3（4 节结构）→ Step 1（说明）/ Step 2（卡）/ Step 3（兼岗 + skill 接口）。✅
- spec §4（4 张卡实质）→ Step 2 逐字写入。✅
- spec §5（skill 关系）→ Step 3 第 4 节。✅
- spec §7（验证）→ Step 4 一致性核对。✅
- spec §1.1 锁定输入（方案 A / 4 块 / 五字段 / 卡2 外包 / 卡4 门禁）→ Global Constraints + 各 Step。✅

**2. Placeholder scan:** 无 TBD/TODO；每步含实际 Markdown 内容。✅（文档正文中的"待补"是内容本身，非计划占位。）

**3. Type consistency:** 五字段顺序（使命/边界/决策权/交付物/现阶段谁兼）在 Global Constraints、Step 2 四张卡中一致；卡2/卡4 特例表述在 Constraints 与卡内容一致。✅
