# stage-gate skill Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 建 `.claude/skills/stage-gate/SKILL.md` —— 一个标准 Claude Code skill，对某阶段（Stage A–E）跑门评审，出通过/打回记录到 `2_Action/`。

**Architecture:** 单文件交付（SKILL.md）。无代码、无测试运行器；验证 = frontmatter 合法性 + 内容一致性核对（对照 spec §6 与阶段/验证词汇事实）。产物与设计 spec、计划一起提交到已检出的 `docs/team-ownership` 分支。

**Tech Stack:** Markdown + YAML frontmatter（Claude Code skill 格式）；正文中文，与项目文档风格一致。

## Global Constraints

- 文件路径：`.claude/skills/stage-gate/SKILL.md`（目录名 = frontmatter name）。
- frontmatter 必含 `name: stage-gate` 与 `description:`（触发描述，见 spec §2.1）。
- 标准来源：**运行时实读** `1_Plan/Plan-Stage-Post-0518.md`（必要时并 `Plan-Execution-0714.md`）；**不在 SKILL.md 内嵌固化验收标准**。
- 判定档固定：`通过 / 未达 / 证据不足`，叠加项目三态（`已验证` / `已实现` / `当前代码事实`）。
- 四条核心规则必须显式写出：①没证据不算过 ②demo 常量 ≠ 真实 SoC ③物理证据项不能靠软件状态过 ④卡2 硬件方案需评审后才进门。
- 复用项目既有验证词汇（build 零告警、`ninja -t deps`、UART 日志串、逻辑分析仪、RAM/FLASH 余量、诚实底线），不另造术语。
- 记录落地路径：`2_Action/Gate-Stage<X>-<YYYYMMDD>.md`。
- 归属：卡4 owner 拍板，引 `0_System/04.team_and_ownership.md`。
- 非目标：不发明/改验收标准；不替执行者跑板测抓包；不写代码；不内嵌固化标准。
- 提交：本次不单独 commit；产物 + spec + 计划一起提交到 `docs/team-ownership`（`skill:` 前缀）。

---

### Task 1: 建 `.claude/skills/stage-gate/SKILL.md`

**Files:**
- Create: `.claude/skills/stage-gate/SKILL.md`
- Reference: `docs/superpowers/specs/2026-07-20-stage-gate-skill-design.md`（内容来源）
- Reference（skill 运行时实读，非本次修改）: `1_Plan/Plan-Stage-Post-0518.md`、`1_Plan/Plan-Execution-0714.md`

**Interfaces:**
- Consumes: spec §2.1（frontmatter）、§2.2（正文结构）、§3（workflow）、§4（模板）、§1.2/§1.3（阶段与验证词汇事实）。
- Produces: 可 `/stage-gate` 调用的 skill；每次运行生成 `2_Action/Gate-Stage<X>-<YYYYMMDD>.md`。

- [ ] **Step 1: 写 frontmatter + "何时用/不用"**

写入文件头：

```markdown
---
name: stage-gate
description: 对 openBattery 某个阶段（Stage A–E）跑一次阶段门评审。运行时实读 1_Plan/Plan-Stage-Post-0518.md 中该阶段的验收标准，逐条对证据判定（通过/未达/证据不足），强制"没证据不算过、demo 常量≠真实 SoC、物理证据不能靠软件状态过"，输出通过/打回记录到 2_Action/。当需要决定某阶段能否收口/进入下一阶段、或做里程碑签核时使用。
---

# stage-gate：阶段门评审 / 决策

## 何时用
- 决定某阶段（Stage A–E）能否收口、进入下一阶段。
- 里程碑签核，需要一份可追溯的通过/打回记录。

## 何时不用
- 发明或修改验收标准——改标准请直接改 `1_Plan/Plan-Stage-Post-0518.md`，本 skill 只读不改。
- 代替执行者跑构建 / 下载 / 板测 / 抓包——本 skill 不产生物理证据，只核对。
- 写固件代码实现——不在此。
```

- [ ] **Step 2: 写 Workflow（5 步）**

写入 workflow（spec §3）：

```markdown
## Workflow

1. **定阶段 + 读标准**：请用户报阶段（Stage A–E）。实读 `1_Plan/Plan-Stage-Post-0518.md` 中该阶段的验收标准（必要时并 `1_Plan/Plan-Execution-0714.md` 的增补项）。若阶段名或标准找不到，**停并提示用户，不臆造标准**。
2. **收证据**：逐条确定所需证据类型并收集——`build 零告警`、`ninja -t deps` 层边界、UART 日志串、逻辑分析仪抓包、RAM/FLASH 余量。**物理证据（板测 / 抓包 / 资源占用）向用户索取实测结果，不自造、不假设。**
3. **逐条判定**：每条给 `通过 / 未达 / 证据不足`，并标三态（`已验证` / `已实现` / `当前代码事实`）。
4. **汇阻塞项**：列阻塞项（硬件未到货、卡2 硬件方案未评审等）+ scope-freeze 提醒（BLE / 复杂 OLED / 大字库 / 预测算法 / 多 SoC / 双镜像 IAP-OTA 在 A–C 完成前不启动）。
5. **出门记录 + 决策**：总判 `通过 / 有条件通过 / 打回`；"有条件通过"须列明放行前置条件。写入 `2_Action/Gate-Stage<X>-<YYYYMMDD>.md`。
```

- [ ] **Step 3: 写判定口径（四档规则）**

写入判定口径（spec §1.3、§3、§2.2 point 3）：

```markdown
## 判定口径

**四档判定：**
- **通过**：验收标准该条已被实际证据满足。
- **未达**：证据存在但不满足（如 demo 常量当作真实值、层边界越界）。
- **证据不足**：所需证据缺失或未实际执行。

**叠加项目三态**（`Plan-Stage-Post-0518.md` §4.2.1）：
- **已实现**：代码结构/接口已存在。
- **当前代码事实**：是否真的进入运行路径。
- **已验证**：编译 / 串口 / 实板 / 抓包均已完成。

**四条核心规则（不可放松）：**
1. **没证据不算过**：没有实际执行的构建 / 下载 / 板测 / 抓包，一律不写"已打通"，判"证据不足"。
2. **demo 常量 ≠ 真实 SoC**：demo adapter 固定值不得当作"真实 SoC 已打通"，判"未达"。
3. **物理证据项不靠软件过**：需板测 / 抓包 / 真实资源占用的条目，只有软件状态时判"证据不足"。
4. **卡2 硬件方案前置**：涉及硬件的条目，其方案须经卡2（硬件）内部评审确认后方可进门。
```

- [ ] **Step 4: 写门记录模板 + 归属**

写入模板与归属（spec §4、§2.2 point 5）。注意记录头示例为单层代码块：

```markdown
## 门记录模板（写入 2_Action/Gate-Stage<X>-<YYYYMMDD>.md）

### 记录头

​```markdown
# 阶段门评审：Stage <X>（<阶段名>）

> 评审日期：<运行时填>
> 标准来源：1_Plan/Plan-Stage-Post-0518.md §<节>（+ Plan-Execution-0714.md §<节> 如有）
> 拍板：卡4 owner（见 0_System/04.team_and_ownership.md）
> 结论：<通过 / 有条件通过 / 打回>
​```

### 逐条判定表

| 验收标准条目 | 所需证据 | 实际证据 | 判定 | 三态 |
|-------------|---------|---------|------|------|
| （引自 Plan-Stage-Post-0518.md） | build/dep/日志/抓包/资源 | … | 通过/未达/证据不足 | 已验证/已实现/当前代码事实 |

### 阻塞项与结论

- **阻塞项**：逐条列（硬件到货、卡2 评审、scope-freeze 违规等）。
- **结论**：通过 / 有条件通过（列前置条件）/ 打回（列必须补齐的证据条目）。

## 评审归属

拍板：卡4（项目协调 / 供应链）owner，持阶段门通过/打回权。角色定义见 `0_System/04.team_and_ownership.md`。
```

> 实现注意：SKILL.md 里"记录头示例"是代码块套代码块。写入时外层用内容分节呈现、记录头示例用三反引号 ```markdown 单层块即可（本 Step 引文中的 ​``` 是转义示意，实际文件中用正常三反引号）。确保渲染不破坏 SKILL.md 结构。

- [ ] **Step 5: 一致性核对（verify）**

对照检查，逐条确认（spec §6）：
- frontmatter 合法：`name: stage-gate` + 非空 `description`；目录名 = `stage-gate`。
- workflow 5 步齐全；步1"实读 Plan-Stage-Post-0518.md"、步2"物理证据向用户索取"、步3 含四条核心规则引用、步5 明确落地路径 `2_Action/Gate-Stage<X>-<YYYYMMDD>.md`。
- 判定口径 = 通过/未达/证据不足 + 三态；四条核心规则全部写出。
- 复用既有验证词汇（build 零告警、`ninja -t deps`、UART 日志串、逻辑分析仪、RAM/FLASH、诚实底线），无新造术语。
- 不含内嵌固化验收标准；不越界改标准 / 跑板测 / 写代码。
- 记录头示例代码块渲染正确，无破坏 SKILL.md 结构。

发现不符则就地修正。

- [ ] **Step 6: 试跑（可选 verify）**

在本会话以 Stage A 为例跑一遍 `/stage-gate`，确认能：实读 Stage A 验收标准（`Plan-Stage-Post-0518.md` §4.4）、生成逐条判定表、对物理证据项（逻辑分析仪抓包、真实 SoC 值）正确标"证据不足"（因当前硬件未完成）、总判"打回"或"有条件通过"。若不能，回到 Step 2/3 修 workflow 措辞。

- [ ] **Step 7: 提交（skill + spec + 计划一起）**

已在 `docs/team-ownership` 分支（无需再开分支）：

```bash
git add .claude/skills/stage-gate/SKILL.md docs/superpowers/specs/2026-07-20-stage-gate-skill-design.md docs/superpowers/plans/2026-07-20-stage-gate-skill.md
git commit -m "skill: add stage-gate (stage acceptance gate review) + design spec

Co-Authored-By: Claude Opus 4.8 <noreply@anthropic.com>"
```

---

## Self-Review

**1. Spec coverage:**
- spec §2.1（frontmatter）→ Step 1。✅
- spec §2.2（正文结构）→ Step 1–4。✅
- spec §3（workflow 5 步）→ Step 2。✅
- spec §4（记录头 / 判定表 / 阻塞结论模板）→ Step 4。✅
- spec §1.3（验证词汇 + 四条核心规则）→ Step 3。✅
- spec §1.2（阶段事实）→ Step 2 实读 + Step 6 试跑。✅
- spec §6（验证）→ Step 5 一致性核对 + Step 6 试跑。✅
- spec §1.1 锁定输入（Claude Code skill / 门评审 / 实读标准 / 2_Action 记录 / 卡4 归属）→ Global Constraints + 各 Step。✅

**2. Placeholder scan:** 无 TBD/TODO。模板中的 `<运行时填>`、`<X>`、`<节>`、`…`、`（引自 …）` 是**模板内容本身**（供 skill 运行时填充），非计划占位。✅

**3. Type consistency:** 四档判定（通过/未达/证据不足）与三态（已验证/已实现/当前代码事实）在 Step 3、Step 4、Step 5 一致；四条核心规则在 Global Constraints、Step 3、Step 5 一致；frontmatter name（stage-gate）与目录名、commit 路径、记录路径模板一致。✅
