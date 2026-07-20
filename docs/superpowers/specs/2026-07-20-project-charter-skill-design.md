# 设计规格：project-charter skill（charter 完整性体检）

> 日期：2026-07-20
> 状态：Draft（设计规格，待落地）
> 性质：brainstorming 产出的设计规格，非最终产物
> 最终产物：`.claude/skills/project-charter/SKILL.md`（+ 每次运行生成 `0_System/skills/charter_completeness_check.md`）
> 关联：三件套第三个（最后一个）skill；接 `0_System/04.team_and_ownership.md`（卡4 owner 主持）

---

## 1. 目标与背景

三件套最后一个做 `project-charter`。关键约束：**`00.project_master.md` 已是事实上的 charter**，几乎所有经典 charter 要素都已存在，只是散落在 4 份文档。因此本 skill **绝不能再抄一份 charter**（重复造轮子），其价值落在**完整性体检**：对照经典 charter 清单，体检现有文档，报"哪些已在哪 / 哪些缺"，不抄内容、只出差距表 + 指向。

核心定位：**完整性体检**（非内容整合、非新起草）。

### 1.1 已确认输入（本次澄清锁定）

- 形态：**标准 Claude Code skill**，`.claude/skills/project-charter/SKILL.md`，frontmatter `name: project-charter` + `description`，可 `/project-charter` 调用。
- 核心定位：charter 完整性体检。
- 内容来源：**运行时实读** `00.project_master.md` / `02.product_requirements.md` / `03.tech_architecture.md` / `04.team_and_ownership.md`；**不内嵌固化项目内容**（只嵌通用 charter 清单这一方法本身）。
- 报告落地：`0_System/skills/charter_completeness_check.md`。
- 归属：卡4 owner 主持体检。
- 缺口不强推创建：每个缺失项标"需补 / 现阶段可不补（注明理由）"，尊重最小化原则。

### 1.2 现有 charter 素材事实（Explore 已核实）

`00.project_master.md` 为 charter 主脊（自称"统一主线文档 / 项目总文档"，§12.4 "唯一收敛入口"）。完整覆盖需并读另外三份。已存在要素与出处：

| charter 要素 | 现有出处 |
|-------------|---------|
| 项目目标 / vision | `00` §1、§3.1；`01` §一；`02` §1；`03` §1 |
| 范围 in-scope | `00` §3.2；`02` §7.3；`03` §3.2 |
| 非目标 out-of-scope | `00` §3.3；`02` §7.3；`03` ADR-002 |
| 里程碑 / 阶段 | `03` §8（Phase 0–4 + 阶段门）；`00` §11 |
| 成功标准 / 验收 | `02` §8；`03` §8 各阶段门；`00` §11.3 |
| 约束（PSE/技適/≤100Wh/64KB/团队/供应链） | `02` §4§6；`00` §9§10；`03` §1 ADR-002 |
| 关键假设 | `00` §8（A–D 具名 + 回退）；`02` §9 |
| 待验证项 / 风险 | `00` 各章 §x.待验证项；`02` §9 |
| 干系人 / 决策权 / 角色 | `04` §2–§3 |
| 已确认输入 | `00` §1.1；`02` 内联标记 |

### 1.3 缺口（哪份文档都没有的经典 charter 要素）

1. 项目预算 / 花费授权
2. 日历里程碑日期（Phase 0–4 有退出标准，无承诺日期）
3. 外部干系人登记（日本客户、英集芯的联系 / 影响 / 沟通要求）
4. 沟通计划
5. 变更控制流程（除隐性"假设→证据→确认"纪律外无正式流程）
6. 资源分配（承认"一人多岗"，无逐阶段资源计划）
7. 正式项目 sponsor / 业务授权

### 1.4 项目四态词汇（体检时须沿用，不另造）

`已确认输入`（给定事实）/ `当前假设`（无证据判断）/ `待验证`（需证据）/ `规划中`（计划中未落代码）。定义见 `02.product_requirements.md` §0。

---

## 2. skill 形态与结构

### 2.1 frontmatter

```yaml
---
name: project-charter
description: 对 openBattery 的项目章程（charter）做完整性体检。运行时实读 00.project_master / 02.product_requirements / 03.tech_architecture / 04.team_and_ownership，按经典 charter 清单（17 要素）逐项判定齐备/部分/缺失并指向现有出处，缺口不强推创建（标"需补/现阶段可不补+理由"）。输出体检表到 0_System/skills/。当需要检查 charter 是否完整、有无遗漏预算/里程碑日期/干系人/沟通/变更控制等要素时使用。不重写或合并 charter 内容。
---
```

### 2.2 SKILL.md 正文结构

1. **何时用 / 不用** —— 用于 charter 完整性体检；不重写 / 合并 charter、不改源文档、不强制补齐缺口。
2. **charter 体检清单（17 要素）** —— 见 §3.1。
3. **Workflow（5 步）** —— 见 §3.2。
4. **判定口径** —— 齐备 / 部分 / 缺失 + "需补 / 可不补" 分级规则。
5. **体检表模板** —— 见 §4。
6. **归属指向** —— 卡4 owner 主持（引 `0_System/04.team_and_ownership.md`）。

---

## 3. 清单与 Workflow

### 3.1 charter 体检清单（17 要素，写入 SKILL.md）

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
13. 外部干系人登记
14. 沟通计划
15. 变更控制流程
16. 资源分配
17. 正式项目 sponsor / 业务授权

### 3.2 Workflow（写入 SKILL.md）

1. **实读 4 文档** —— 实读 `0_System/00.project_master.md`、`02.product_requirements.md`、`03.tech_architecture.md`、`04.team_and_ownership.md`。若文件缺失或路径变动，停并提示，不臆造。
2. **逐要素定位** —— 对 17 要素逐一在 4 文档中定位现有出处（文件 §节）。
3. **逐要素判定** —— 每要素给 `齐备 / 部分 / 缺失`，附现有出处；沿用四态词汇。
4. **缺口分级** —— 每个"缺失 / 部分"项标 `需补` 或 `现阶段可不补（注明理由）`；**不强推创建**（小团队未必需要正式预算 / sponsor / 沟通计划）。
5. **出报告** —— 体检表 + 缺口小结 + 一句话结论（charter 完整度 + 建议优先补的项），写入 `0_System/skills/charter_completeness_check.md`。

---

## 4. 体检表模板（`0_System/skills/charter_completeness_check.md`）

### 4.1 报告头

```markdown
# openBattery Charter 完整性体检

> 体检日期：<运行时填>
> 来源文档：00.project_master / 02.product_requirements / 03.tech_architecture / 04.team_and_ownership
> 主持：卡4 owner（见 0_System/04.team_and_ownership.md）
> 结论：<charter 完整度一句话 + 优先补齐项>
```

### 4.2 体检表（17 行）

| charter 要素 | 是否齐备 | 现有出处（文件§节） | 判定 | 现阶段是否需补 |
|-------------|---------|-------------------|------|--------------|
| 项目目标 | 齐备 | 00 §1 / 03 §1 | 齐备 | — |
| （其余 16 要素同格式） | | | 齐备/部分/缺失 | 需补 / 可不补（理由） |

### 4.3 缺口小结

- 逐条列"缺失 / 部分"要素 + 需补/可不补判断 + 理由。
- 一句话结论：当前 charter 完整度与优先补齐建议。

---

## 5. 范围与非目标

**范围内**：
- `.claude/skills/project-charter/SKILL.md`（方法 + 17 要素清单 + workflow + 判定口径 + 体检表模板 + 归属）。

**非目标（本次不做）**：
- 不重写、合并或"整合成一页"charter 内容（那会与 master 职能重叠）。
- 不改 4 份源文档。
- 不强制补齐所有缺口（缺口分级由卡4 owner 决定，尊重最小化）。
- 不在 SKILL.md 内嵌固化项目内容（只嵌通用 charter 清单）。
- 不新起草某子项目的 charter（另一种定位，未采纳）。

---

## 6. 验证方式

无测试运行器，验证为人工复审 + 试跑：
- **verify**：SKILL.md 有合法 frontmatter（`name: project-charter` + `description`），可在 `.claude/skills/` 下被 `/project-charter` 识别。
- **verify**：workflow 5 步齐全；步骤 1 明确"实读 4 文档"、步骤 4 明确"缺口不强推、标需补/可不补+理由"、步骤 5 明确落地路径 `0_System/skills/charter_completeness_check.md`。
- **verify**：17 要素清单与 §3.1 一致（10 常规 + 7 缺口）；体检表列 = 要素/是否齐备/现有出处/判定/现阶段是否需补。
- **verify**：沿用四态词汇（已确认输入/当前假设/待验证/规划中），不另造术语。
- **verify**：明确"不重写/合并 charter、不改源文档、不强制补齐"三条非目标；不内嵌固化项目内容。
- **verify（试跑，可选）**：跑一遍，能定位到 §1.2 已有要素的出处、把 §1.3 的 7 项标为"缺失"，并对其中如预算/sponsor 给出"现阶段可不补 + 理由"。
