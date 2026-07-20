# interface-contract skill Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 建 `.claude/skills/interface-contract/SKILL.md` —— 一个标准 Claude Code skill，做 SoC 供商（英集芯）能力评估，产出差距表 + RFI。

**Architecture:** 单文件交付（SKILL.md）。无代码、无测试运行器；验证 = frontmatter 合法性 + 内容一致性核对（对照 spec §6 与 `soc_api.h` 事实）。产物与设计 spec、计划一起提交到已检出的 `docs/team-ownership` 分支。

**Tech Stack:** Markdown + YAML frontmatter（Claude Code skill 格式）；正文中文，与项目文档风格一致。

## Global Constraints

- 文件路径：`.claude/skills/interface-contract/SKILL.md`（skill 目录名 = frontmatter name）。
- frontmatter 必含 `name: interface-contract` 与 `description:`（触发描述，见 spec §2.1）。
- 契约基线来源：**方案 A** —— workflow 明确"运行时实读 `openBmsClaw/drivers/soc/soc_api.h`、`soc_types.h`、`soc_driver.h`"，**不在 SKILL.md 内嵌固化契约清单**。
- 评估维度：固定 **8 维**（遥测 / 保护设置 / 事件模型 / 错误恢复 / 端口 / 传输 / 单位 / 文档调试），不增不减，顺序与 spec §4.1 一致。
- 判定口径固定四档：`满足 / 部分满足 / 缺失 / 未知`；**未知不猜**规则必须显式写出。
- 报告落地路径：`0_System/skills/soc_capability_assessment_injoinic.md`。
- 归属：卡1 owner 主评、卡4 owner 记录，引 `0_System/04.team_and_ownership.md`。
- 非目标：不做代码边界审计 / 变更门；不改契约头文件；不替供商填未知。
- 提交：本次不单独 commit；产物 + spec + 计划一起提交到 `docs/team-ownership`（`docs:` 前缀）。

---

### Task 1: 建 `.claude/skills/interface-contract/SKILL.md`

**Files:**
- Create: `.claude/skills/interface-contract/SKILL.md`
- Reference: `docs/superpowers/specs/2026-07-20-interface-contract-skill-design.md`（内容来源）
- Reference（skill 运行时实读，非本次修改）: `openBmsClaw/drivers/soc/soc_api.h`、`soc_types.h`、`soc_driver.h`

**Interfaces:**
- Consumes: spec §2.1（frontmatter）、§2.2（正文结构）、§3（workflow）、§4（模板）、§1.2（契约事实）。
- Produces: 可 `/interface-contract` 调用的 skill；每次运行生成 `0_System/skills/soc_capability_assessment_injoinic.md`。

- [ ] **Step 1: 写 frontmatter + "何时用/不用"**

写入文件头：

```markdown
---
name: interface-contract
description: 评估候选电源 SoC（默认英集芯）的能力是否满足 openBattery 当前 SoC 契约。运行时实读 soc_api.h/soc_types.h/soc_driver.h 作为需求基线，按 8 维度逐项判定（满足/部分/缺失/未知），未知不猜、自动汇成 RFI 问卷，输出差距表 + RFI 到 0_System/skills/。当需要评审 SoC 供商（英集芯）API/datasheet/事件能力、或回答"该芯片是否达平台化合作要求"时使用。
---

# interface-contract：SoC 供商能力评估

## 何时用
- 评审英集芯（或任一候选电源 SoC）的 datasheet / API / 事件能力。
- 回答"该芯片是否达到平台化合作要求"（master §1.6 待验证项）。

## 何时不用
- 代码边界审计（as-is，查现有代码是否越界）——另一种评审对象，不在此。
- 变更评审门（新适配器 / 改 API）——不在此。
- 需要修改 `soc_api.h`/`soc_types.h`/`soc_driver.h`——本 skill 只读契约、不改契约。
```

- [ ] **Step 2: 写 8 维度定义表**

写入维度表（"我们所需"列引自代码事实，spec §1.2 / §4.1）：

```markdown
## 8 个评估维度

| # | 维度 | 我们契约所需（运行时以代码为准） |
|---|------|-------------------------------|
| 1 | 遥测读取 | 电压 mV(uint32)、电流 mA(int32 有符号)、温度 °C(int16)，经 I2C 可读 |
| 2 | 保护设置 | 每端口 OCP(uint32 ma)、端口功率上限，可设 + 设置粒度 |
| 3 | 事件模型 | 4 类告警(短路/过温/过流/系统错误) + 硬件 INT 线 + µs 级回调、禁慢速 I2C |
| 4 | 错误恢复 | offline / busy / param / hw-locked→总线恢复 语义 |
| 5 | 端口模型 | Type-C1 / Type-C2 / USB-A1 |
| 6 | 传输 | I2C 从地址/协议（或 UART） |
| 7 | 单位/量纲 | mV / mA / mW / °C 整数 + 转换公式可得 |
| 8 | 文档/调试 | 寄存器图完整度、API 文档、调试支持是否平台级 |

> 契约不稳定点（评估时须标注）：`soc_api.h` watt 与 vtable milliwatt 不一致（arch §5.3）；ISR 当前恒发系统错误告警；`soc_driver_ops_t` 无 NULL 守卫。这些是我方契约待收敛项，不计入供商缺失。
```

- [ ] **Step 3: 写 Workflow（5 步）+ 判定口径**

写入 workflow 与判定口径（spec §3、§2.2 point 6）：

```markdown
## Workflow

1. **读契约基线**：实读 `openBmsClaw/drivers/soc/soc_api.h`、`soc_types.h`、`soc_driver.h`，抽出 8 维度的"我们所需"。若文件缺失或路径变动，停并提示用户，**不臆造基线**。
2. **收集供商资料**：请用户提供英集芯 datasheet / API / 事件表节选，或指向 `90_documents/` 或 `91.reference/` 下的文件路径。
3. **逐维度判定**：8 维每项给一档判定。资料未覆盖即标"未知"，**不据经验推测**。
4. **汇 RFI**：所有"部分满足 / 未知"项汇成结构化问卷。
5. **出报告**：写差距表 + RFI + 一句话结论，落地到 `0_System/skills/soc_capability_assessment_injoinic.md`。

## 判定口径（四档）

- **满足**：供商资料明确覆盖且达到我方所需。
- **部分满足**：覆盖但有限制（粒度不足 / 需降配 / 条件性），须说明限制并进 RFI。
- **缺失**：供商资料明确表明不具备。
- **未知**：现有资料未覆盖。**未知不猜** —— 标"待向英集芯确认"并进 RFI，不填推测值。
```

- [ ] **Step 4: 写差距表模板 + RFI 模板 + 报告头 + 归属**

写入模板与归属（spec §4、§2.2 point 7）：

```markdown
## 产出模板

### 差距表（8 行，每维度一行）

| 维度 | 我们所需（引自代码） | 供商现状 | 判定 | 建议行动 |
|------|--------------------|---------|------|---------|
| 遥测读取 | … | … | 满足/部分/缺失/未知 | 直接用 / 降配 / 进 RFI |
| （其余 7 维同格式） | | | | |

### RFI 问卷（自动从"部分/未知"项生成）

每条：`编号 | 关联维度 | 问题 | 为什么需要(关联契约点) | 期望回复形式`。

### 报告头（写入 0_System/skills/soc_capability_assessment_injoinic.md）

​```markdown
# 英集芯 SoC 能力评估（对照 openBattery SoC 契约）

> 生成日期：<运行时填>
> 契约基线版本：<soc_api.h 当次 commit 或摘要>
> 评审：卡1 owner 主评 / 卡4 owner 记录
> 状态：Draft
​```

## 评审归属

主评：卡1（固件 + SoC 集成）owner；记录：卡4（项目协调 / 供应链）owner。角色定义见 `0_System/04.team_and_ownership.md`。
```

> 注：报告头示例块在 SKILL.md 中用围栏嵌套呈现（外层四反引号或用零宽字符规避）；实现时确保嵌套代码块渲染正确。

- [ ] **Step 5: 一致性核对（verify）**

对照检查，逐条确认（spec §6）：
- frontmatter 合法：`name: interface-contract` + 非空 `description`；目录名 = `interface-contract`。
- workflow 5 步齐全；步骤 1 明确"实读三个头文件"、步骤 3 明确"未知不猜"、步骤 5 明确落地路径 `0_System/skills/soc_capability_assessment_injoinic.md`。
- 8 维度与 spec §4.1 表一致、顺序一致；差距表列 = 维度/我们所需/供商现状/判定/建议行动。
- 与 `soc_api.h` 事实一致：遥测 mV/mA(有符号)/°C、每端口 OCP、4 告警、5 错误码含 hw-locked→恢复、C1/C2/A1；watt/mW 已知债有标注且归为"我方契约待收敛"。
- 不含内嵌固化契约清单（方案 B 已排除）；不越界审计代码或改头文件。
- 嵌套代码块（报告头示例）渲染正确，无破坏 SKILL.md 结构。

发现不符则就地修正。

- [ ] **Step 6: 试跑（可选 verify）**

在本会话用一段占位供商资料（如"英集芯 IP53xx：I2C 从地址 0x75，可读电压/电流/温度；OCP 可设；事件通过 INT 引脚 + 状态寄存器；UFCS 支持；调试文档中文"）跑一遍 `/interface-contract`，确认能产出：8 行差距表 + 至少 1 条 RFI + 一句话结论。若不能，回到 Step 3/4 修 workflow 措辞。

- [ ] **Step 7: 提交（skill + spec + 计划一起）**

已在 `docs/team-ownership` 分支（无需再开分支）：

```bash
git add .claude/skills/interface-contract/SKILL.md docs/superpowers/specs/2026-07-20-interface-contract-skill-design.md docs/superpowers/plans/2026-07-20-interface-contract-skill.md
git commit -m "skill: add interface-contract (SoC supplier capability eval) + design spec

Co-Authored-By: Claude Opus 4.8 <noreply@anthropic.com>"
```

---

## Self-Review

**1. Spec coverage:**
- spec §2.1（frontmatter）→ Step 1。✅
- spec §2.2（正文结构：何时用/维度/workflow/模板/判定/归属）→ Step 1–4。✅
- spec §3（workflow 5 步）→ Step 3。✅
- spec §4.1/4.2/4.3（差距表/RFI/报告头模板）→ Step 4。✅
- spec §1.2（契约事实）→ Step 2 维度表 + Step 5 一致性核对。✅
- spec §6（验证）→ Step 5 一致性核对 + Step 6 试跑。✅
- spec §1.1 锁定输入（Claude Code skill / 供商评估 / 评估+RFI / 方案 A / 8 维 / 报告路径）→ Global Constraints + 各 Step。✅

**2. Placeholder scan:** 无 TBD/TODO。模板中的 `…`、`<运行时填>`、`(其余 7 维同格式)` 是**模板内容本身**（供 skill 运行时填充），非计划占位。✅

**3. Type consistency:** 8 维度名称与顺序在 Global Constraints、Step 2、Step 4、Step 5 一致；四档判定（满足/部分满足/缺失/未知）在 Step 3、Step 4、Step 5 一致；frontmatter name（interface-contract）与目录名、commit 路径一致。✅
