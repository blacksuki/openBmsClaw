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

---

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

---

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
- **需补**：该要素当前阶段确有价值且缺失 / 不足。
- **现阶段可不补**：缺失但当前阶段不必要（须写明理由，如"一人多岗，暂无独立预算授权需求"）。

---

## 体检表模板（写入 `0_System/skills/charter_completeness_check.md`）

### 报告头

```markdown
# openBattery Charter 完整性体检

> 体检日期：<运行时填>
> 来源文档：00.project_master / 02.product_requirements / 03.tech_architecture / 04.team_and_ownership
> 主持：卡4 owner（见 0_System/04.team_and_ownership.md）
> 结论：<charter 完整度一句话 + 优先补齐项>
```

### 体检表（17 行）

| charter 要素 | 是否齐备 | 现有出处（文件§节） | 判定 | 现阶段是否需补 |
|-------------|---------|-------------------|------|--------------|
| 项目目标 | 齐备 | 00 §1 / 03 §1 | 齐备 | — |
| （其余 16 要素同格式） | | | 齐备/部分/缺失 | 需补 / 可不补（理由） |

### 缺口小结

- 逐条列"缺失 / 部分"要素 + 需补 / 可不补判断 + 理由。
- 一句话结论：当前 charter 完整度与优先补齐建议。

---

## 主持归属

主持：卡4（项目协调 / 供应链）owner。角色定义见 `0_System/04.team_and_ownership.md`。
