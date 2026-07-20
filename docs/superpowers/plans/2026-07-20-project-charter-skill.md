# project-charter skill Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 建 `.claude/skills/project-charter/SKILL.md` —— 一个标准 Claude Code skill，对现有 charter 素材做完整性体检，出体检表到 `0_System/skills/`。

**Architecture:** 单文件交付（SKILL.md）。无代码、无测试运行器；验证 = frontmatter 合法性 + 内容一致性核对（对照 spec §6 与现有 charter 素材事实）。产物与设计 spec、计划一起提交到已检出的 `docs/team-ownership` 分支。

**Tech Stack:** Markdown + YAML frontmatter（Claude Code skill 格式）；正文中文，与项目文档风格一致。

## Global Constraints

- 文件路径：`.claude/skills/project-charter/SKILL.md`（目录名 = frontmatter name）。
- frontmatter 必含 `name: project-charter` 与 `description:`（触发描述，见 spec §2.1）。
- 内容来源：**运行时实读** `0_System/00.project_master.md` / `02.product_requirements.md` / `03.tech_architecture.md` / `04.team_and_ownership.md`；**不内嵌固化项目内容**（只嵌通用 charter 清单）。
- 体检清单固定 **17 要素**（10 常规 + 7 缺口，见 spec §3.1），顺序一致。
- 判定档固定：`齐备 / 部分 / 缺失`；缺口分级 `需补 / 现阶段可不补（注明理由）`，**不强推创建**。
- 沿用项目四态词汇（`已确认输入 / 当前假设 / 待验证 / 规划中`），不另造术语。
- 报告落地路径：`0_System/skills/charter_completeness_check.md`。
- 归属：卡4 owner 主持，引 `0_System/04.team_and_ownership.md`。
- 非目标：不重写 / 合并 charter 内容；不改 4 份源文档；不强制补齐缺口；不内嵌固化项目内容；不新起草子项目 charter。
- 提交：本次不单独 commit；产物 + spec + 计划一起提交到 `docs/team-ownership`（`skill:` 前缀）。

---

### Task 1: 建 `.claude/skills/project-charter/SKILL.md`

**Files:**
- Create: `.claude/skills/project-charter/SKILL.md`
- Reference: `docs/superpowers/specs/2026-07-20-project-charter-skill-design.md`（内容来源）
- Reference（skill 运行时实读，非本次修改）: `0_System/00.project_master.md`、`02.product_requirements.md`、`03.tech_architecture.md`、`04.team_and_ownership.md`

**Interfaces:**
- Consumes: spec §2.1（frontmatter）、§2.2（正文结构）、§3（清单 + workflow）、§4（模板）、§1.2/§1.3/§1.4（charter 素材与词汇事实）。
- Produces: 可 `/project-charter` 调用的 skill；每次运行生成 `0_System/skills/charter_completeness_check.md`。

- [ ] **Step 1: 写 frontmatter + "何时用/不用"**

写入文件头：

```markdown
---
name: project-charter
description: 对 openBattery 的项目章程（charter）做完整性体检。运行时实读 00.project_master / 02.product_requirements / 03.tech_architecture / 04.team_and_ownership，按经典 charter 清单（17 要素）逐项判定齐备/部分/缺失并指向现有出处，缺口不强推创建（标"需补/现阶段可不补+理由"）。输出体检表到 0_System/skills/。当需要检查 charter 是否完整、有无遗漏预算/里程碑日期/干系人/沟通/变更控制等要素时使用。不重写或合并 charter 内容。
---

# project-charter：charter 完整性体检

## 何时用
- 检查项目章程（charter）是否完整、有无遗漏。
- 阶段性回顾：预算 / 里程碑日期 / 干系人 / 沟通 / 变更控制等要素是否该补。

## 何时不用
- 重写、合并或"整合成一页"charter 内容——那会与 `00.project_master.md`（事实上的 charter）职能重叠，本 skill 只体检不重写。
- 修改 4 份源文档——只读不改。
- 强制补齐所有缺口——缺口是否补由卡4 owner 决定，本 skill 只报告与建议。
```

- [ ] **Step 2: 写 charter 体检清单（17 要素）**

写入清单（spec §3.1）：

```markdown
## charter 体检清单（17 要素）

10 类常规 + 7 缺口候选：

1. 项目目标 / vision
2. 范围 in-scope
3. 非目标 out-of-scope
4. 里程碑 / 阶段
5. 成功标准 / 验收
6. 约束（合规 / 资源 / 技术）
7. 关键假设
8. 待验证项 / 风险
9. 干系人 / 决策权 / 角色
10. 已确认输入
11. 项目预算 / 花费授权
12. 日历里程碑日期
13. 外部干系人登记（客户 / 供应商联系·影响·沟通要求）
14. 沟通计划
15. 变更控制流程
16. 资源分配
17. 正式项目 sponsor / 业务授权
```

- [ ] **Step 3: 写 Workflow（5 步）+ 判定口径**

写入 workflow 与判定口径（spec §3.2、§2.2 point 4）：

```markdown
## Workflow

1. **实读 4 文档**：实读 `0_System/00.project_master.md`、`0_System/02.product_requirements.md`、`0_System/03.tech_architecture.md`、`0_System/04.team_and_ownership.md`。若文件缺失或路径变动，**停并提示用户，不臆造**。
2. **逐要素定位**：对 17 要素逐一在 4 文档中定位现有出处（文件 §节）。
3. **逐要素判定**：每要素给 `齐备 / 部分 / 缺失`，附现有出处；沿用四态词汇（已确认输入 / 当前假设 / 待验证 / 规划中）。
4. **缺口分级**：每个"缺失 / 部分"项标 `需补` 或 `现阶段可不补（注明理由）`。**不强推创建**——小团队未必需要正式预算 / sponsor / 沟通计划，据实判断。
5. **出报告**：体检表 + 缺口小结 + 一句话结论（charter 完整度 + 优先补齐项），写入 `0_System/skills/charter_completeness_check.md`。

## 判定口径

- **齐备**：该要素在源文档有明确、可定位的内容。
- **部分**：有涉及但不完整（如假设无回退、里程碑无日期）。
- **缺失**：4 份文档均无。

缺口分级：
- **需补**：该要素当前阶段确有价值且缺失/不足。
- **现阶段可不补**：缺失但当前阶段不必要（须写明理由，如"一人多岗，暂无独立预算授权需求"）。
```

- [ ] **Step 4: 写体检表模板 + 归属**

写入模板与归属（spec §4、§2.2 point 6）。注意报告头示例为单层代码块：

```markdown
## 体检表模板（写入 0_System/skills/charter_completeness_check.md）

### 报告头

​```markdown
# openBattery Charter 完整性体检

> 体检日期：<运行时填>
> 来源文档：00.project_master / 02.product_requirements / 03.tech_architecture / 04.team_and_ownership
> 主持：卡4 owner（见 0_System/04.team_and_ownership.md）
> 结论：<charter 完整度一句话 + 优先补齐项>
​```

### 体检表（17 行）

| charter 要素 | 是否齐备 | 现有出处（文件§节） | 判定 | 现阶段是否需补 |
|-------------|---------|-------------------|------|--------------|
| 项目目标 | 齐备 | 00 §1 / 03 §1 | 齐备 | — |
| （其余 16 要素同格式） | | | 齐备/部分/缺失 | 需补 / 可不补（理由） |

### 缺口小结

- 逐条列"缺失 / 部分"要素 + 需补/可不补判断 + 理由。
- 一句话结论：当前 charter 完整度与优先补齐建议。

## 主持归属

主持：卡4（项目协调 / 供应链）owner。角色定义见 `0_System/04.team_and_ownership.md`。
```

> 实现注意：SKILL.md 里"报告头示例"是代码块套代码块。本 Step 引文中的 ​``` 是转义示意，实际写入文件时报告头用正常三反引号 ```markdown 单层块即可，确保渲染不破坏 SKILL.md 结构。

- [ ] **Step 5: 一致性核对（verify）**

对照检查，逐条确认（spec §6）：
- frontmatter 合法：`name: project-charter` + 非空 `description`；目录名 = `project-charter`。
- workflow 5 步齐全；步1"实读 4 文档"、步4"缺口不强推、标需补/可不补+理由"、步5 明确落地路径 `0_System/skills/charter_completeness_check.md`。
- 17 要素清单与 spec §3.1 一致（10 常规 + 7 缺口）、顺序一致；体检表列 = 要素/是否齐备/现有出处/判定/现阶段是否需补。
- 判定档 = 齐备/部分/缺失；缺口分级 = 需补/可不补（理由）。
- 沿用四态词汇（已确认输入/当前假设/待验证/规划中），无新造术语。
- 三条非目标（不重写/合并、不改源文档、不强制补齐）写出；不内嵌固化项目内容。
- 报告头示例代码块渲染正确，无破坏 SKILL.md 结构。

发现不符则就地修正。

- [ ] **Step 6: 试跑（可选 verify）**

在本会话跑一遍 `/project-charter`，确认能：实读 4 文档、定位到 spec §1.2 已有要素的出处、把 spec §1.3 的 7 项标为"缺失"、并对其中如预算/sponsor 给出"现阶段可不补 + 理由"、生成 17 行体检表。若不能，回到 Step 3/4 修 workflow 措辞。

- [ ] **Step 7: 提交（skill + spec + 计划一起，三件套收口）**

已在 `docs/team-ownership` 分支（无需再开分支）：

```bash
git add .claude/skills/project-charter/SKILL.md docs/superpowers/specs/2026-07-20-project-charter-skill-design.md docs/superpowers/plans/2026-07-20-project-charter-skill.md
git commit -m "skill: add project-charter (charter completeness check) + design spec

Co-Authored-By: Claude Opus 4.8 <noreply@anthropic.com>"
```

---

## Self-Review

**1. Spec coverage:**
- spec §2.1（frontmatter）→ Step 1。✅
- spec §2.2（正文结构）→ Step 1–4。✅
- spec §3.1（17 要素清单）→ Step 2。✅
- spec §3.2（workflow 5 步）→ Step 3。✅
- spec §4（报告头 / 体检表 / 缺口小结模板）→ Step 4。✅
- spec §1.2/§1.3（charter 素材与缺口事实）→ Step 2 清单 + Step 6 试跑。✅
- spec §1.4（四态词汇）→ Step 3。✅
- spec §6（验证）→ Step 5 一致性核对 + Step 6 试跑。✅
- spec §1.1 锁定输入（Claude Code skill / 完整性体检 / 实读 4 文档 / 报告落地 / 卡4 归属 / 缺口不强推）→ Global Constraints + 各 Step。✅

**2. Placeholder scan:** 无 TBD/TODO。模板中的 `<运行时填>`、`（其余 16 要素同格式）`、`…` 是**模板内容本身**（供 skill 运行时填充），非计划占位。✅

**3. Type consistency:** 17 要素清单与顺序在 Global Constraints、Step 2、Step 5 一致；判定档（齐备/部分/缺失）与缺口分级（需补/可不补）在 Step 3、Step 4、Step 5 一致；frontmatter name（project-charter）与目录名、commit 路径、报告路径一致。✅
