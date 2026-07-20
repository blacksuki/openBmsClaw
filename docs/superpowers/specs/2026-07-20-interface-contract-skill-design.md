# 设计规格：interface-contract skill（SoC 供商能力评估）

> 日期：2026-07-20
> 状态：Draft（设计规格，待落地）
> 性质：brainstorming 产出的设计规格，非最终产物
> 最终产物：`.claude/skills/interface-contract/SKILL.md`（+ 每次运行生成 `0_System/skills/soc_capability_assessment_injoinic.md`）
> 关联：三件套第一个 skill；接 `0_System/04.team_and_ownership.md`（卡1 主评、卡4 记录）

---

## 1. 目标与背景

三件套（`project-charter` / `interface-contract` / `stage-gate`）中先做 `interface-contract`，因为它直接护住 `soc_api.h` 抽象边界、风险最高。

经澄清，本 skill 的**评审对象是"供商能力评估（英集芯）"**：拿英集芯（或任一候选 SoC）的 datasheet / API / 事件能力，对照 openBattery **当前 SoC 契约所需**，产出**能力差距表 + 待确认 RFI**，回答 master §1.6 待验证项"英集芯可提供的 API、文档、事件、调试支持是否达到平台化合作要求"。

它**不是**代码边界审计（as-is），也**不是**变更评审门；这两种是另外的评审对象，本次不做。

### 1.1 已确认输入（本次澄清锁定）

- 形态：**标准 Claude Code skill**，`.claude/skills/interface-contract/SKILL.md`，frontmatter `name: interface-contract` + `description`，可 `/interface-contract` 调用。
- 评审对象：**供商能力评估（英集芯）**。
- 输入不全时：**评估 + 自动生成 RFI**（未知不猜，标"待向英集芯确认"并汇成问卷）。
- 契约基线来源：**方案 A —— 运行时实读** `soc_api.h`/`soc_types.h`/`soc_driver.h`（单一真相源，与代码同步）。
- 评估维度：**8 维**（遥测 / 保护设置 / 事件模型 / 错误恢复 / 端口 / 传输 / 单位 / 文档调试），不增不减。
- 报告落地：`0_System/skills/soc_capability_assessment_injoinic.md`（卡4 owner 维护）。

### 1.2 契约事实来源（Explore 已核实，firmware 目录 `openBmsClaw/`）

- 公开 API `drivers/soc/soc_api.h`：10 个函数——`soc_sal_init(uint8_t)`、`soc_sal_is_initialized`、`soc_sal_bus_recovery`、`soc_get_voltage(uint32_t* mv)`、`soc_get_current(int32_t* ma)`、`soc_get_temperature(int16_t* °C)`、`soc_set_ocp(soc_port_t, uint32_t ma)`、`soc_set_port_power(soc_port_t, uint16_t w)`、`soc_poll_events(uint32_t* mask)`、`soc_sal_register_emergency_callback(cb)`。头文件严禁含 `hal_*.h`（ISSUE-001）。
- 类型 `drivers/soc/soc_types.h`：告警位掩码 4 类（`SOC_ALARM_SHORT_CIRCUIT`/`OVER_TEMP`/`OVER_CURRENT`/`SYS_ERROR`）；错误码枚举 `SOC_SAL_OK/ERR_OFFLINE/ERR_BUSY/ERR_PARAM/ERR_HW_LOCKED`；端口 `SOC_PORT_TYPE_C1/C2/A1`；紧急回调 `void(*)(uint32_t alarm_mask)`，µs 级、禁慢速 I2C。
- 适配器契约 `drivers/soc/soc_driver.h`：`soc_driver_ops_t` 7 个函数指针，寄存器表只准在 `vendor/<chip>.c`。
- 已知债（评估时需标注为契约不稳定点）：`soc_api.h` watt 与 vtable milliwatt 不一致（arch §5.3）；ISR 恒发 `SOC_ALARM_SYS_ERROR`；`ops` 无 NULL 守卫。

---

## 2. skill 形态与结构

### 2.1 frontmatter

```yaml
---
name: interface-contract
description: 评估候选电源 SoC（默认英集芯）的能力是否满足 openBattery 当前 SoC 契约。运行时实读 soc_api.h/soc_types.h/soc_driver.h 作为需求基线，按 8 维度逐项判定（满足/部分/缺失/未知），未知不猜、自动汇成 RFI 问卷。输出差距表 + RFI 到 0_System/skills/。当需要评审 SoC 供商（英集芯）API/datasheet/事件能力、或回答"该芯片是否达平台化合作要求"时使用。
---
```

### 2.2 SKILL.md 正文结构

1. **何时用 / 不用** —— 用于供商能力评估；不做代码边界审计、不做变更门、不改契约头。
2. **8 维度定义表** —— 每维度写清"我们契约所需"从哪读、判定标准。
3. **Workflow（5 步）** —— 见 §3。
4. **差距表模板** —— 见 §4。
5. **RFI 模板** —— 见 §4。
6. **判定口径** —— 满足/部分/缺失/未知 的明确定义 + "未知不猜"规则。
7. **归属指向** —— 卡1 主评、卡4 记录（引 `0_System/04.team_and_ownership.md`）。

---

## 3. Workflow（写入 SKILL.md）

1. **读契约基线** —— 实读 `openBmsClaw/drivers/soc/soc_api.h`、`soc_types.h`、`soc_driver.h`，抽出 8 维度的"我们所需"。若文件缺失/路径变动，停并提示，不臆造基线。
2. **收集供商资料** —— 提示用户提供英集芯 datasheet/API/事件表节选，或指向 `90_documents/` / `91.reference/` 下的文件路径。
3. **逐维度判定** —— 8 维每项给判定：`满足 / 部分满足 / 缺失 / 未知`。**未知不猜**：资料未覆盖即标"未知—待向英集芯确认"，不据经验推测。
4. **汇 RFI** —— 所有"部分/未知"项自动汇成结构化问卷（该问英集芯什么、为什么需要）。
5. **出报告** —— 写差距表 + RFI + 一句话结论（是否达平台化合作要求：达标/有条件达标/不达标），落地到 `0_System/skills/soc_capability_assessment_injoinic.md`。

---

## 4. 产出物模板

### 4.1 差距表（8 行，每维度一行）

| 维度 | 我们所需（引自代码） | 供商现状 | 判定 | 建议行动 |
|------|--------------------|---------|------|---------|
| 遥测读取 | 电压 mV / 电流 mA(有符号) / 温度 °C，I2C 可读 | …… | 满足/部分/缺失/未知 | 直接用 / 降配 / 进 RFI |
| 保护设置 | 每端口 OCP(ma)、端口功率上限，可设 + 粒度 | | | |
| 事件模型 | 4 类告警 + 硬件 INT 线 + µs 级、禁慢速 I2C | | | |
| 错误恢复 | offline/busy/param/hw-locked→总线恢复 语义 | | | |
| 端口模型 | C1/C2/A1 | | | |
| 传输 | I2C 地址/协议（或 UART） | | | |
| 单位/量纲 | mV/mA/mW/°C + 转换公式可得 | | | |
| 文档/调试 | 寄存器图完整度、API 文档、调试支持是否平台级 | | | |

### 4.2 RFI 问卷（自动从"部分/未知"项生成）

每条：`编号 | 关联维度 | 问题 | 为什么需要（关联契约点） | 期望回复形式`。

### 4.3 报告头（`0_System/skills/soc_capability_assessment_injoinic.md`）

```markdown
# 英集芯 SoC 能力评估（对照 openBattery SoC 契约）

> 生成日期：<运行时填>
> 契约基线版本：<soc_api.h 当次 commit 或摘要>
> 评审：卡1 owner 主评 / 卡4 owner 记录
> 状态：Draft
```

---

## 5. 范围与非目标

**范围内**：
- `.claude/skills/interface-contract/SKILL.md`（方法 + 8 维度 + workflow + 差距表/RFI 模板 + 判定口径）。

**非目标（本次不做）**：
- 不做代码边界审计（as-is）与变更评审门。
- 不改 `soc_api.h`/`soc_types.h`/`soc_driver.h`。
- 不替英集芯填未知项、不据经验臆造供商能力。
- 不建 `project-charter` / `stage-gate`（后续单独做）。
- 不在 SKILL.md 内嵌固化契约清单（方案 B 已排除）。

---

## 6. 验证方式

无测试运行器，验证为人工复审 + 试跑：
- **verify**：SKILL.md 有合法 frontmatter（`name: interface-contract` + `description`），可在 `.claude/skills/` 下被 `/interface-contract` 识别。
- **verify**：workflow 5 步齐全，步骤 1 明确"实读三个头文件"、步骤 3 明确"未知不猜"、步骤 5 明确落地路径。
- **verify**：8 维度与 §4.1 表一致；差距表列 = 维度/我们所需/供商现状/判定/建议行动。
- **verify**：与 `soc_api.h` 事实一致——10 函数、4 告警、5 错误码、C1/C2/A1 端口，且标注 watt/mW 已知债。
- **verify**：不含"内嵌固化契约清单"（避免方案 B 漂移）；不越界去审计代码或改头文件。
- **verify（试跑，可选）**：用占位供商资料跑一遍，能产出差距表 + 至少一条 RFI。
