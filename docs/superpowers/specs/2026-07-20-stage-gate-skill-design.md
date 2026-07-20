# 设计规格：stage-gate skill（阶段门评审 / 决策方法）

> 日期：2026-07-20
> 状态：Draft（设计规格，待落地）
> 性质：brainstorming 产出的设计规格，非最终产物
> 最终产物：`.claude/skills/stage-gate/SKILL.md`（+ 每次运行生成 `2_Action/Gate-Stage<X>-<YYYYMMDD>.md`）
> 关联：三件套第二个 skill；接 `0_System/04.team_and_ownership.md`（卡4 owner 拍板）

---

## 1. 目标与背景

三件套第二个做 `stage-gate`。项目**已有成熟阶段体系**（Stage A–E，对应 Phase 1–4 + 量产前验证），每阶段在 `1_Plan/Plan-Stage-Post-0518.md` 有明确验收标准。本 skill 的活**不是造阶段/标准**，而是把一次阶段门评审跑成标准动作：拉出该阶段验收标准 → 逐条对证据判定 → 强制"没证据不算过" → 出通过/打回记录。

核心定位：**门评审 / 决策方法**（非验收标准作者方法）。

### 1.1 已确认输入（本次澄清锁定）

- 形态：**标准 Claude Code skill**，`.claude/skills/stage-gate/SKILL.md`，frontmatter `name: stage-gate` + `description`，可 `/stage-gate` 调用。
- 核心定位：门评审 / 决策方法。
- 标准来源：**运行时实读** `1_Plan/Plan-Stage-Post-0518.md`（必要时并 `Plan-Execution-0714.md` 增补项）；**不在 SKILL.md 内嵌固化标准**（避免漂移，与 interface-contract 方案 A 一致）。
- 记录落地：`2_Action/Gate-Stage<X>-<YYYYMMDD>.md`。
- 归属：卡4 owner 拍板通过/打回。

### 1.2 阶段与验收标准事实（Explore 已核实）

- 阶段：Stage A（Phase 1 SoC SAL 真实基线闭环）、B（服务层最小策略闭环）、C（F030 样片验证）、D（最小量产功能集闭环）、E（量产前验证与资料沉淀）。来源 `Plan-Stage-Post-0518.md` §3–§8。
- 每阶段验收标准 4–5 条，逐条可引（如 Stage A：串口日志见 SoC init 成败 / `EXTI→callback→service` 至少一次真实或自测闭环 / 逻辑分析仪抓到真实应答波形 / 终端打印至少一个真实 SoC 值而非 demo 常量 / 至少一份抓包记录）。
- 边界原则（`Plan-Execution-0714.md` §4）：A 未完成，B/C 建立在不可靠前提上；A 分"软件闭合半"与"硬件证据半"，仅软件半完成不得宣告 A 完成（§5.6 / `Plan-Stage-Post-0518.md` §4.5）。

### 1.3 项目既有验证词汇（skill 须复用，不另造）

- **build 门**：`cd openBmsClaw && cmake --preset Debug && cmake --build --preset Debug` → exit 0、`-Wall -Wextra -Wpedantic` 无新增告警。
- **层边界门**：`ninja -t deps CMakeFiles/<target>.dir/<file>.c.obj` → 期望无越界头文件输出。
- **UART 日志串**：如 `bringup: soc sal init ok`、`bringup: soc poll online mask=0x...`、`bringup: soc int selftest PASS` 等，作为可观测完成信号。
- **逻辑分析仪抓包**：`CH0→PB6/SCL`、`CH1→PB7/SDA`，地址阶段 ACK/NACK。
- **三态诚实**：`已实现`（结构/接口存在）/ `当前代码事实`（是否真进运行路径）/ `已验证`（编译·串口·实板·抓包均完成）。
- **RAM/FLASH 余量**：Stage C/D 输出要求真实资源占用数据。
- **诚实底线（全局）**：没有实际执行的构建/下载/板测/抓包，一律不写"已打通"（`Plan-Execution-0714.md` §3 item 5）；demo adapter 固定值 ≠ 真实 SoC 已打通（`Plan-Stage-Post-0518.md` §4.5）。

---

## 2. skill 形态与结构

### 2.1 frontmatter

```yaml
---
name: stage-gate
description: 对 openBattery 某个阶段（Stage A–E）跑一次阶段门评审。运行时实读 1_Plan/Plan-Stage-Post-0518.md 中该阶段的验收标准，逐条对证据判定（通过/未达/证据不足），强制"没证据不算过、demo 常量≠真实 SoC、物理证据不能靠软件状态过"，输出通过/打回记录到 2_Action/。当需要决定某阶段能否收口/进入下一阶段、或做里程碑签核时使用。
---
```

### 2.2 SKILL.md 正文结构

1. **何时用 / 不用** —— 用于阶段门通过/打回决策；不发明或修改验收标准、不替执行者跑板测抓包、不写代码。
2. **Workflow（5 步）** —— 见 §3。
3. **判定口径** —— 通过/未达/证据不足 定义 + 三态叠加 + 四条核心规则。
4. **门记录模板** —— 见 §4。
5. **归属指向** —— 卡4 owner 拍板（引 `0_System/04.team_and_ownership.md`）。

---

## 3. Workflow（写入 SKILL.md）

1. **定阶段 + 读标准** —— 用户报阶段（Stage A–E）。实读 `1_Plan/Plan-Stage-Post-0518.md` 该阶段验收标准（必要时并 `Plan-Execution-0714.md` 增补项）。若阶段名或标准找不到，**停并提示，不臆造标准**。
2. **收证据** —— 逐条确定所需证据类型并收集：`build 零告警`、`ninja -t deps` 层边界、UART 日志串、逻辑分析仪抓包、RAM/FLASH 余量。**物理证据（板测 / 抓包 / 资源占用）向用户索取实测结果，不自造、不假设。**
3. **逐条判定** —— 每条给 `通过 / 未达 / 证据不足`，并标三态（`已验证` / `已实现` / `当前代码事实`）。规则：物理证据项若只有软件状态 = 证据不足；demo 常量当作真实值 = 未达；无实际执行记录 = 证据不足（不得写"已打通"）。
4. **汇阻塞项** —— 列阻塞项（如硬件未到货、卡2 硬件方案未评审）+ scope-freeze 提醒（BLE / 复杂 OLED / 大字库 / 预测算法 / 多 SoC / 双镜像 IAP-OTA 在 A–C 完成前不启动）。
5. **出门记录 + 决策** —— 总判 `通过 / 有条件通过 / 打回`；"有条件通过"须列明放行的前置条件。写入 `2_Action/Gate-Stage<X>-<YYYYMMDD>.md`。

---

## 4. 门记录模板（`2_Action/Gate-Stage<X>-<YYYYMMDD>.md`）

### 4.1 记录头

```markdown
# 阶段门评审：Stage <X>（<阶段名>）

> 评审日期：<运行时填>
> 标准来源：1_Plan/Plan-Stage-Post-0518.md §<节>（+ Plan-Execution-0714.md §<节> 如有）
> 拍板：卡4 owner（见 0_System/04.team_and_ownership.md）
> 结论：<通过 / 有条件通过 / 打回>
```

### 4.2 逐条判定表

| 验收标准条目 | 所需证据 | 实际证据 | 判定 | 三态 |
|-------------|---------|---------|------|------|
| （引自 Plan-Stage-Post-0518.md） | build/dep/日志/抓包/资源 | … | 通过/未达/证据不足 | 已验证/已实现/当前代码事实 |

### 4.3 阻塞项与结论

- **阻塞项**：逐条列（硬件到货、卡2 评审、scope-freeze 违规等）。
- **结论**：通过 / 有条件通过（列前置条件）/ 打回（列必须补齐的证据条目）。

---

## 5. 范围与非目标

**范围内**：
- `.claude/skills/stage-gate/SKILL.md`（方法 + workflow + 判定口径 + 门记录模板 + 归属）。

**非目标（本次不做）**：
- 不发明或修改验收标准（改标准去改 `Plan-Stage-Post-0518.md`）。
- 不替执行者跑构建/下载/板测/抓包，不臆造物理证据。
- 不写固件代码实现。
- 不在 SKILL.md 内嵌固化验收标准（避免与计划文档漂移）。
- 不建 `project-charter`（三件套最后一个，另行做）。

---

## 6. 验证方式

无测试运行器，验证为人工复审 + 试跑：
- **verify**：SKILL.md 有合法 frontmatter（`name: stage-gate` + `description`），可在 `.claude/skills/` 下被 `/stage-gate` 识别。
- **verify**：workflow 5 步齐全；步骤 1 明确"实读 `Plan-Stage-Post-0518.md`"、步骤 2 明确"物理证据向用户索取"、步骤 3 含四条核心规则、步骤 5 明确落地路径 `2_Action/Gate-Stage<X>-<YYYYMMDD>.md`。
- **verify**：判定口径 = 通过/未达/证据不足 + 三态；四条核心规则（没证据不算过 / demo≠真实 / 物理项不靠软件过 / 卡2 硬件评审前置）全部写出。
- **verify**：复用项目既有验证词汇（build 零告警、`ninja -t deps`、UART 日志串、逻辑分析仪、RAM/FLASH、三态、诚实底线），不另造术语。
- **verify**：不含内嵌固化标准；不越界改标准 / 跑板测 / 写代码。
- **verify（试跑，可选）**：以 Stage A 为例跑一遍，能实读其验收标准、生成逐条判定表、对物理证据项正确标"证据不足"（因当前硬件未完成）。
