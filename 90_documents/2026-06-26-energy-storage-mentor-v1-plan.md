# Energy-Storage Mentor (v1 wedge) Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 搭出储能行业入门导师 v1 的可运行骨架——目录 + 薄 mentor 提示 + 三份带 schema 的模板 + 验证手册——让专家只需填内容签收即可跑通一次真实入门。

**Architecture:** 纯 markdown 文件,无代码无编译。Agent 只产出"怎么教 + 空模板"(通用品);专家产出并签收"教什么 + 怎么问"(知识地图 + 诊断脚本,核心资产)。两者通过固定的字段/章节名约定对接。GIGO 闸门:mentor 只引用 `status: signed-off` 的内容。

**Tech Stack:** Markdown only. 验证用 `grep`/`ls` 做存在性与内容检查(本仓库无测试框架,以行为/检视为质量门)。

## Global Constraints

- 落地目录:`90_documents/energy-storage-mentor/`(与本设计材料同处)。
- 受众/周期:行业小白、入门级、1 天–2 周。措辞避免行业黑话。
- **提交策略**:本仓库**仅在用户明确要求时提交**(CLAUDE.md)。每个 Task 末尾"暂存(stage)"即可,实际 `git commit` 等用户指示。
- **GIGO 硬规则**:Agent 不得为 `knowledge_map.md` / `diagnostic.md` 填写权威储能事实;Agent 只产出模板。真实内容由专家填写并把 `status` 置为 `signed-off`。
- **跨文件字段约定(所有 Task 共享,改名即破坏对接)**:
  - frontmatter 键:`status`(`draft|signed-off`)、`signed_off_by`、`signed_off_date`、`sources`
  - 节点字段:`适合谁`、`识别信号`、`核心概念`、`一句话事实`、`经典案例`、`来源`
  - 诊断脚本:recognition 式、`Q1/Q2/...`、每选项 `→ 侧重节点 [..]`
  - 教学策略仅两种字面值:`Executive Overview`、`Case Study`

---

## File Structure

| 文件 | 责任 | 谁写 |
|------|------|------|
| `90_documents/energy-storage-mentor/README.md` | 项目导航 + 运行入口 | Agent |
| `.../mentor.md` | 薄提示:诊断方法论 + GIGO 闸门 + 教学 + 自测 | Agent |
| `.../knowledge/knowledge_map.md` | 知识地图**模板**(schema,待专家填) | Agent(模板)/ 专家(内容) |
| `.../knowledge/diagnostic.md` | 诊断脚本**模板**(待专家填) | Agent(模板)/ 专家(内容) |
| `.../roadmap.md` | 2 周路径**模板**(按诊断圈选填) | Agent(模板)/ 专家(内容) |
| `.../research/.gitkeep` | 未校验原料区(占位) | Agent |
| `.../HOWTO-run-v1.md` | 验证手册:作业步骤 + 成功判据 | Agent |

---

### Task 1: 目录骨架 + README

**Files:**
- Create: `90_documents/energy-storage-mentor/README.md`
- Create: `90_documents/energy-storage-mentor/research/.gitkeep`

**Interfaces:**
- Produces: 目录 `90_documents/energy-storage-mentor/`(后续所有 Task 在此之下)。

- [ ] **Step 1: 建目录与占位文件**

```bash
mkdir -p 90_documents/energy-storage-mentor/knowledge
mkdir -p 90_documents/energy-storage-mentor/research
touch 90_documents/energy-storage-mentor/research/.gitkeep
```

- [ ] **Step 2: 写 README.md**

```markdown
# 储能行业入门导师 (v1 wedge)

面向**行业小白**的快速入门引导。1 天–2 周。首个行业:日本储能。

## 这是什么
- 价值不在 agent,在两份**专家签收**的资产:`knowledge/knowledge_map.md`(教什么)、`knowledge/diagnostic.md`(怎么问出范围)。
- mentor 只引用签收过的内容,不自由发挥(GIGO)。

## 怎么用
1. 专家填好并签收 `knowledge/knowledge_map.md` 和 `knowledge/diagnostic.md`(`status: signed-off`)。
2. 把 `mentor.md` 贴进 Claude Code 会话(或作为 skill)。
3. mentor 先跑诊断、回放确认范围,再按 `roadmap.md` 开课。

## 目录
- `mentor.md` — 导师提示(稳定不变)
- `knowledge/` — 签收过的权威资产
- `research/` — 未校验原料,**不可当事实**
- `roadmap.md` — 2 周路径
- `HOWTO-run-v1.md` — 怎么验证它有没有价值

详见设计 spec:`90_documents/2026-06-26-industry-mentor-framework-design.md`
```

- [ ] **Step 3: 验证结构**

Run: `ls -R 90_documents/energy-storage-mentor/`
Expected: 列出 `README.md`、`knowledge/`、`research/.gitkeep`

- [ ] **Step 4: 暂存(commit 待用户指示)**

```bash
git add 90_documents/energy-storage-mentor/README.md 90_documents/energy-storage-mentor/research/.gitkeep
```

---

### Task 2: `mentor.md` — 薄导师提示

**Files:**
- Create: `90_documents/energy-storage-mentor/mentor.md`

**Interfaces:**
- Consumes: 跨文件字段约定(frontmatter `status`、节点 `适合谁/识别信号`、诊断 `Q1...`、`Executive Overview`/`Case Study`)。
- Produces: 一份可直接贴入会话的导师提示。

- [ ] **Step 1: 写 mentor.md**

```markdown
# 储能行业入门导师 (mentor)

你是一位**懂日本储能行业的导师**,面向**完全零基础或略懂一点**的小白,带他在 1 天–2 周内入门。

## 铁律 (GIGO)
1. 行业事实**只能引用** `knowledge/knowledge_map.md` 中 `status: signed-off` 的内容,并可点明出处。
2. 若该文件 `status != signed-off`,进入"未校验研究模式":只用 `research/` 原料,每条**显式标注"未校验、待证实"**,绝不当事实讲。
3. 知识地图没覆盖的问题:直说"这超出已校验范围,我不编",不臆测。

## 第一步:专家式诊断(不要直接开讲)
小白说不清自己要学什么。**不要问"你想学什么"**。执行 `knowledge/diagnostic.md` 的脚本,遵守:
1. **认得 > 想得起**:给具体选项/场景让他认。
2. **具体场景代替抽象**:用他真实会遇到的处境,不用黑话。
3. **一次一问,按答案分叉**,可临场追问,封顶约 7 问。
4. **回放确认**:问完用大白话把范围还给他——"所以你要学这 N 块、跳过那 M 块,对吗?"——他确认/微调后才开课。

诊断产出 = 在知识地图节点上的一份**圈选**(学哪些/跳过哪些/什么深度),写进/更新 `roadmap.md`。

## 第二步:教学
- 按确认后的范围 + `roadmap.md` 推进。
- 教学策略只用两种,按诊断出的目的选:**Executive Overview**(快速全貌)或 **Case Study**(案例落地)。
- 每个节点讲完做**轻量自测**(用知识地图题库,"懂没懂"),错了就补讲,不做严格晋级闸门。

## 风格
- 结论先行、编号步骤、每个判断说"为什么"。
- 不承诺小白能做研发/创业;目标是看懂全貌、能与从业者对话、能做初步判断。
```

- [ ] **Step 2: 验证关键约束在位**

Run:
```bash
grep -c "signed-off" 90_documents/energy-storage-mentor/mentor.md
grep -E "Executive Overview|Case Study|回放确认|认得 > 想得起" 90_documents/energy-storage-mentor/mentor.md
```
Expected: `signed-off` 出现 ≥2 次;四个关键词均命中。

- [ ] **Step 3: 暂存**

```bash
git add 90_documents/energy-storage-mentor/mentor.md
```

---

### Task 3: 两份签收资产的**模板** (`knowledge_map.md` + `diagnostic.md`)

**Files:**
- Create: `90_documents/energy-storage-mentor/knowledge/knowledge_map.md`
- Create: `90_documents/energy-storage-mentor/knowledge/diagnostic.md`

**Interfaces:**
- Produces: 字段名与 mentor.md / roadmap.md 引用的约定一致(`status`、`适合谁`、`识别信号`、`Q1...→ 侧重节点`)。
- 注意:内容为**空模板**,`status: draft`。专家填完置 `signed-off` 前,mentor 不得当事实使用。

- [ ] **Step 1: 写 knowledge_map.md 模板**

```markdown
---
industry: 储能 (energy storage, Japan)
status: draft                 # 专家填完内容并复核后改成 signed-off
signed_off_by:
signed_off_date:
sources: []                   # URL/文献/内部文档清单
---

# 行业骨架(节点) — 专家填,建议 5–8 个

1. <节点名>
   - 适合谁: [采购/销售/研发/投资/...]
   - 识别信号: <小白说出什么话/有什么处境,说明他需要它>
   - 核心概念: [<词1>, <词2>]
   - 一句话事实: <可被引用的权威陈述>
   - 经典案例: <1 个>
   - 来源: <sources 里的引用编号>

2. <节点名>
   - ...

# 入门必答题库(供轻量自测) — 专家填
- Q: <问题> / A: <答案> / 关联节点: <节点号>
```

- [ ] **Step 2: 写 diagnostic.md 模板**

```markdown
---
status: draft                 # 与 knowledge_map 同步签收
signed_off_by:
signed_off_date:
---

# 诊断脚本 (recognition 式,一次一问,按答案分叉,封顶约 7 问)

Q1(定位用): 你接触储能,更接近哪种处境?
  - 要采购/选型      → 侧重节点 [..]
  - 要卖给客户       → 侧重节点 [..]
  - 要做产品/研发    → 侧重节点 [..]
  - 只想看懂全貌     → 全节点浅讲

Q2(场景化探测): 假设下周你要 <具体场景>,下面哪个你"完全没概念"?
  - <选项A>  → 必学节点 [..]
  - <选项B>  → 必学节点 [..]
  - <选项C>  → 必学节点 [..]

# Q3.. 按需追问(专家补)

# 回放模板(mentor 用): "所以你要学 <节点列表>、跳过 <节点列表>,对吗?"
```

- [ ] **Step 3: 验证 schema 字段齐全**

Run:
```bash
grep -E "status:|适合谁|识别信号|一句话事实|来源" 90_documents/energy-storage-mentor/knowledge/knowledge_map.md
grep -E "status:|侧重节点|回放模板" 90_documents/energy-storage-mentor/knowledge/diagnostic.md
```
Expected: 两条命令各字段均命中;两文件 `status: draft`。

- [ ] **Step 4: 暂存**

```bash
git add 90_documents/energy-storage-mentor/knowledge/knowledge_map.md 90_documents/energy-storage-mentor/knowledge/diagnostic.md
```

---

### Task 4: `roadmap.md` — 2 周路径模板

**Files:**
- Create: `90_documents/energy-storage-mentor/roadmap.md`

**Interfaces:**
- Consumes: 诊断圈选的节点(来自 mentor 执行 diagnostic 的产出)、`knowledge_map.md` 节点号。
- Produces: 逐日/逐段计划,每段标注用 `Executive Overview` 还是 `Case Study`。

- [ ] **Step 1: 写 roadmap.md 模板**

```markdown
# 2 周入门路径 (按诊断圈选填,不是全量节点)

> 圈选范围: <mentor 诊断后填入的节点号清单>
> 学习目的: <采购/销售/研发/投资/全貌>

## Day 1
- 节点: <号>  策略: Executive Overview
- 产出: 能用三句话说清这个节点是什么

## Day 2–3
- 节点: <号>  策略: <Executive Overview | Case Study>
- ...

## (按周期 1 天–2 周裁剪;略懂者可压缩)

## 自测点
- 每个节点讲完用 knowledge_map 题库自测一次
```

- [ ] **Step 2: 验证策略字面值在位**

Run: `grep -E "Executive Overview|Case Study|圈选" 90_documents/energy-storage-mentor/roadmap.md`
Expected: 均命中。

- [ ] **Step 3: 暂存**

```bash
git add 90_documents/energy-storage-mentor/roadmap.md
```

---

### Task 5: `HOWTO-run-v1.md` — 验证手册(作业 + 成功判据)

**Files:**
- Create: `90_documents/energy-storage-mentor/HOWTO-run-v1.md`

**Interfaces:**
- Consumes: spec §6 成功判据、§7 作业。
- Produces: 一份让人能独立跑完并判断"有没有价值"的手册。这是 v1 的验证 harness。

- [ ] **Step 1: 写 HOWTO-run-v1.md**

```markdown
# 怎么验证 v1 有没有价值

## 你要做的(本周作业)
1. **专家填内容**:在 `knowledge/knowledge_map.md` 填 5–8 个储能核心节点,每节点带 `适合谁/识别信号` + 来源;在 `knowledge/diagnostic.md` 填 5–7 个 recognition 问题。
2. **签收**:复核无误后把两文件 `status` 改成 `signed-off`,填 `signed_off_by/date`。
   - ⚠️ 签收前 mentor 只能进"未校验模式",不会当事实讲。
3. **找一个真实零基础的人**(可以是同事)。
4. **先跑诊断**:把 `mentor.md` 贴进会话,让它执行诊断脚本、回放确认范围。
5. **再跑 Day 1**:按确认范围讲第一节。
6. **对照**:同一个人,同样问题直接问 raw Claude,比差异。

## 成功判据(逐条回答)
1. 诊断是否摸清了**正确范围**? 学的东西是不是他真正需要的?
2. 行业事实是否**都可追溯到签收来源**(零幻觉)?
3. 学完能否看懂全貌 / 与从业者对话 / 做初步判断?
4. **是否明显优于直接问 raw AI**? 若不优于 → 价值不成立,回炉,别急着写更多编排。

## 不要做
- 不要先建 v2 的四模块 / 状态机 / 通用换包(见 spec §8)。
- 不要让 agent 代笔知识地图内容(GIGO)。
```

- [ ] **Step 2: 验证四条成功判据在位**

Run: `grep -E "正确范围|零幻觉|优于直接问|GIGO" 90_documents/energy-storage-mentor/HOWTO-run-v1.md`
Expected: 均命中。

- [ ] **Step 3: 暂存**

```bash
git add 90_documents/energy-storage-mentor/HOWTO-run-v1.md
```

---

## Self-Review

**1. Spec coverage:**
- §1 目标/受众 → README + mentor.md 风格段。✓
- §2 非目标 → HOWTO "不要做" + 未建任何 v2 件。✓
- §3 护城河(知识地图 + 诊断两资产)→ Task 3 两模板。✓
- §4.1 knowledge_map schema(含 `适合谁/识别信号`)→ Task 3 Step 1。✓
- §4.2 diagnostic(recognition/分叉/回放)→ Task 3 Step 2 + mentor.md 诊断段。✓
- §4.3 roadmap(按圈选、两策略)→ Task 4。✓
- §4.4 mentor(诊断/只引用签收/自测/不编)→ Task 2。✓
- §5 目录 + GIGO 闸门 → Task 1 + Task 2 铁律。✓
- §6 成功判据 → Task 5。✓
- §7 作业 → Task 5。✓

**2. Placeholder scan:** 模板里的 `<...>` 是**给专家填的字段占位**,属交付物的一部分(空模板),非计划占位。Agent 步骤本身无 TODO/TBD。✓

**3. Type/命名一致性:** `status: signed-off`、节点 `适合谁/识别信号`、诊断 `→ 侧重节点`、`Executive Overview/Case Study` 在 mentor.md / knowledge_map.md / diagnostic.md / roadmap.md 间用词一致。✓

无遗漏,无需返工。
