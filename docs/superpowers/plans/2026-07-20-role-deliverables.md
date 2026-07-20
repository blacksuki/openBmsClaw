# 角色交付物层 Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 在 `04.team_and_ownership.md` 增加"阶段×交付物"节（6→4 映射 + 4×3 矩阵），并建 4 个缺失交付物模板到 `0_System/templates/`（含审阅中追加的硬件架构模板）。

**Architecture:** 纯文档交付（1 处扩展 + 3 新文件）。无代码、无测试运行器；验证 = 人工一致性核对（对照 spec §6）。产物与 spec、计划一起提交到已检出的 `docs/team-ownership` 分支。

**Tech Stack:** Markdown（中文散文，风格对齐 `0_System/*.md`）。

## Global Constraints

- 角色模型：**保留 4-owner，只加交付物层**；6 角色作参考映射进 4 卡，不重写、不加第 5 卡。
- 阶段轴：3 通用生命周期阶段 —— 启动 / 开发中 / 验收测试。
- 04 只**追加** §5，现有 §1–§4 与 4 张卡五字段**不改动**。
- 模板范围：只建 4 个缺失模板（Feature List / 干系人登记 / 测试计划 / 硬件架构）；已有交付物（PRD/架构/验收标准/编码规范/阶段门）只指向出处、不重建。Chief Architect ①技术路线/②软件架构/④API 规范 指向 03，不建模板。
- 模板为**通用空骨架**，不预填项目内容；每模板含"如何用"一行说明；证据词汇复用 stage-gate（build/deps/UART/逻辑分析仪/资源/三态）。
- 模板落地：`0_System/templates/`（新建目录，4 文件）。
- **不做任何新 skill**。
- 风格：中文、四态标注习惯、无绝对路径。
- 提交：本次不单独 commit；产物 + spec + 计划一起提交到 `docs/team-ownership`（`docs:` 前缀）。

---

### Task 1: 在 `04.team_and_ownership.md` 追加 §5 阶段×交付物

**Files:**
- Modify: `0_System/04.team_and_ownership.md`（在现有 §4 之后追加 §5）
- Reference: `docs/superpowers/specs/2026-07-20-role-deliverables-design.md`（§1.2 映射、§2 矩阵、§4 改动说明）

**Interfaces:**
- Consumes: spec §1.2（6→4 映射）、§2（矩阵）、§4（改动范围）。
- Produces: 04 §5，供后续引用；指向 `0_System/templates/` 3 模板。

- [ ] **Step 1: 读取 04 当前结尾锚点**

Run: 打开 `0_System/04.team_and_ownership.md`，确认末尾为 §4"与 skill 的接口"，其最后一行为 `project-charter：引用本框架 owner 定义，不重复定义角色。`
Expected: §4 是当前最后一节，无 §5。

- [ ] **Step 2: 在 §4 之后追加 §5**

在文件末尾追加以下内容：

```markdown

---

## 5. 阶段×交付物

本节在 4-owner 框架上加一层"每个 owner 在启动 / 开发中 / 验收测试三阶段该交付什么"。组织理念为 **MVP-first**：组织为最小可验证产品样板服务，平台能力仅作架构约束（分层 / 接口 / Board 抽象 / Config 四项），不作组织目标，不提前做多平台 / 多 SoC。

### 5.1 参考 6 角色 → 4 卡 映射

外部提案的 6 角色（Product Owner / Chief Architect / Hardware Lead / Software Lead / Embedded Engineer / Validation）映射进本框架 4 卡：

| 6 角色 | 归入 4 卡 |
|--------|-----------|
| Product Owner | 卡3（产品 / 数据 / 合规） |
| Chief Architect | 跨卡：软件 / SoC 架构·API 契约·code review·技术债 → 卡1；硬件架构·power tree → 卡2 |
| Software Lead + Embedded Engineer（Feature Ownership） | 卡1（固件 + SoC） |
| Hardware Lead | 卡2（硬件 / 电源 / 热） |
| Validation | 不设第 5 卡：各 owner 在"验收测试"列自验其交付物；整体测试计划 + gate 记录归卡4 |

### 5.2 交付物矩阵

行 = 4 owner，列 = 3 阶段。标注：`[已有→出处]` / `[缺失→建模板]` / `[缺失→可后补]`。

| owner | 启动 | 开发中 | 验收测试 |
|-------|------|--------|---------|
| 卡1 固件+SoC | 软件分层[已有 03 §4]·API 契约[已有 soc_api.h + interface-contract]·仓库/CI/编码规范[已有 AGENTS.md/CLAUDE.md] | 各 Feature 实现(drv→logic→api)·Feature 设计评审记录[缺失→可后补] | build 零告警 + `ninja -t deps` 证据·集成报告[缺失→可后补] |
| 卡2 硬件 | 硬件架构 / power tree / 选型[外包·部分] | 原理图 / PCB / BOM / bring-up[外包] | EMC / 热 / bring-up 报告[外包·需内评] |
| 卡3 产品 | PRD[已有 02]·MVP Scope[已有 00 §3]·验收标准[已有 02 §8]·Feature List[缺失→建模板] | 范围 / 优先级变更记录·数据模型 v0[规划中] | 产品验收(对 PRD 验收标准)·PSE 合规证据 |
| 卡4 协调 | 阶段门原则[已有 00 §11.3]·外部干系人登记[缺失→建模板]·测试计划 / Validation Plan[缺失→建模板] | 阶段门记录[stage-gate 产出]·英集芯 RFI[interface-contract 产出] | gate 记录[stage-gate]·资料沉淀[Stage E] |

已有交付物只指向出处、不重建。标"缺失→建模板"的 3 项模板见 `0_System/templates/`：`feature_list_template.md`、`stakeholder_register_template.md`、`validation_plan_template.md`。
```

- [ ] **Step 3: 核对 §1–§4 未被改动**

Run: 通读 04，确认 §1 说明段、§2 四张角色卡（五字段）、§3 兼岗映射、§4 与 skill 接口 均保持原样，仅新增 §5。
Expected: 无原有内容被改写；§5 为唯一新增。

---

### Task 2: 建 `0_System/templates/` 下 4 个模板

**Files:**
- Create: `0_System/templates/feature_list_template.md`
- Create: `0_System/templates/stakeholder_register_template.md`
- Create: `0_System/templates/validation_plan_template.md`
- Create: `0_System/templates/hardware_architecture_template.md`
- Reference: `docs/superpowers/specs/2026-07-20-role-deliverables-design.md`（§3 模板骨架）

**Interfaces:**
- Consumes: spec §3.1/§3.2/§3.3。
- Produces: 3 个通用模板骨架，被 04 §5.2 矩阵指向。

- [ ] **Step 1: 建 Feature List 模板（卡3）**

Create `0_System/templates/feature_list_template.md`：

```markdown
# Feature List 模板（卡3 · 启动阶段交付物）

> 用法：复制本骨架，逐 Feature 填写；每个 Feature 由一人端到端负责（driver→logic→api→test）；延后项对齐 scope-freeze（见 03 ADR-002 / Plan-Stage-Post-0518 §9）。

| Feature | 描述 | 所属能力 | Owner（端到端） | MVP 优先级 | 依赖 | 验收关联 |
|---------|------|---------|----------------|-----------|------|---------|
| （示例）电量读取 | 经 SoC 读电压/电流/温度 | Battery | <人名/卡号> | P0 | soc_api | 02 §8 第 x 条 |

> 优先级：P0（MVP 必须）/ P1（MVP 内次要）/ 延后（下一阶段）。所属能力：Battery / PD / NFC / 温度保护 / …。
```

- [ ] **Step 2: 建 外部干系人登记 模板（卡4，闭合 charter 体检 #13）**

Create `0_System/templates/stakeholder_register_template.md`：

```markdown
# 外部干系人登记 模板（卡4 · 启动阶段交付物）

> 用法：登记项目外部干系人；至少登记英集芯（SoC 合作主线）与日本目标客户。英集芯行与 interface-contract skill 产出的 RFI 对接。闭合 charter 完整性体检 #13。

| 对象 | 类型 | 角色与依赖 | 接口 / 联系与响应支持 | 影响度 | 沟通要求 | 关联 |
|------|------|-----------|---------------------|-------|---------|------|
| （示例）英集芯 | 供应商(SoC) | 提供 SoC API/事件/调试支持 | <联系人/响应时效> | 高 | <频率/形式> | interface-contract RFI |

> 类型：客户 / 供应商 / 认证机构 / …。影响度：高 / 中 / 低。
```

- [ ] **Step 3: 建 测试计划 / Validation Plan 模板（卡4）**

Create `0_System/templates/validation_plan_template.md`：

```markdown
# 测试计划 / Validation Plan 模板（卡4 · 启动阶段交付物）

> 用法：从第一周开始的持续验证计划（非最后测试）。证据形式复用 stage-gate 词汇；与 Stage A–E 验收标准挂钩，但本模板按能力组织，不替代 stage-gate 的门评审。

| 验证目标 | 测试类别 | 用例概要 | 证据形式 | 关联 Stage 验收标准 | 状态（三态） |
|---------|---------|---------|---------|-------------------|------------|
| （示例）真实 SoC 数据链 | Battery | 读电压并与万用表比对 | UART 日志 + 逻辑分析仪抓包 | Stage A 第 4 条 | 当前代码事实 |

> 测试类别：Power / Battery / NFC / 温度保护 / 集成 / …。证据形式：build 零告警 / `ninja -t deps` / UART 日志 / 逻辑分析仪抓包 / 资源占用。三态：已实现 / 当前代码事实 / 已验证。
```

- [ ] **Step 4: 建 硬件架构 / Power Tree 模板（卡2 · Chief Architect 硬件侧，审阅追加）**

Create `0_System/templates/hardware_architecture_template.md`：骨架含 系统框图（MCU↔SoC↔外设）/ Power Tree 表 / 关键器件表 / 接口划分（硬件→软件 board 层边界）；用法说明"卡2 外包填、卡1 review 后进阶段门；与 03 ADR-001 一致；不含 PCB 布局"。

- [ ] **Step 5: 核对 4 模板一致性（verify）**

对照检查：
- 4 文件均在 `0_System/templates/`，均为通用空骨架（仅"（示例）"行，无真实项目内容预填）。
- 每模板含"用法"一行说明。
- 证据词汇（build/deps/UART/逻辑分析仪/资源/三态）与 stage-gate 一致。
- 干系人模板含英集芯行并指向 interface-contract RFI。
- 硬件架构模板含 power tree + 接口划分，指向 03 ADR-001、由卡1 review。
- 文件名与 04 §5.2 指向的 4 个文件名完全一致。

---

### Task 3: 提交（产物 + spec + 计划一起）

- [ ] **Step 1: 一致性总核对（对照 spec §6）**

- 04 §5 含 6→4 映射表（Validation 不设第 5 卡、Architect 跨卡）+ 4×3 矩阵；§1–§4 未改动。
- 矩阵每格有三种标注之一；3 个"缺失→建模板"项与 templates 下 3 文件一一对应、文件名一致。
- 3 模板为通用骨架、含用法说明、证据词汇复用 stage-gate。
- 无新 skill；风格与 `0_System/*.md` 一致（中文、四态、无绝对路径）。

- [ ] **Step 2: 提交**

已在 `docs/team-ownership` 分支：

```bash
git add 0_System/04.team_and_ownership.md 0_System/templates/feature_list_template.md 0_System/templates/stakeholder_register_template.md 0_System/templates/validation_plan_template.md 0_System/templates/hardware_architecture_template.md docs/superpowers/specs/2026-07-20-role-deliverables-design.md docs/superpowers/plans/2026-07-20-role-deliverables.md
git commit -m "docs: add role deliverable matrix (04 §5) + 4 missing-deliverable templates

Co-Authored-By: Claude Opus 4.8 <noreply@anthropic.com>"
```

---

## Self-Review

**1. Spec coverage:**
- spec §1.2（6→4 映射）→ Task 1 Step 2 §5.1。✅
- spec §2（矩阵）→ Task 1 Step 2 §5.2。✅
- spec §3.1/3.2/3.3（3 模板骨架）→ Task 2 Step 1–3。✅
- spec §4（04 只追加 §5，§1–§4 不改）→ Task 1 Step 2 + Step 3 核对。✅
- spec §5（范围/非目标）→ Global Constraints。✅
- spec §6（验证）→ Task 2 Step 4 + Task 3 Step 1。✅

**2. Placeholder scan:** 无 TBD/TODO。模板中的"（示例）…"行是**模板示例内容**（教用户怎么填），非计划占位；spec §3 明确要求给出可用骨架。✅

**3. Type consistency:** 3 个模板文件名（feature_list_template / stakeholder_register_template / validation_plan_template）在 Task 1 §5.2 指向、Task 2 创建、Task 3 commit 三处完全一致；映射表（Validation 不设第 5 卡、Architect 跨卡）在 spec 与 Task 1 一致；证据词汇在 3 模板与 stage-gate 一致。✅
