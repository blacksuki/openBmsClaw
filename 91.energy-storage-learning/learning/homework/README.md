# Homework

这里保存导师布置的作业、测验和批改记录。

## 作业规则

1. 每份作业必须有明确问题。
2. 输出必须能被 reviewer 审查。
3. 不确定内容要写入 `待验证项`。
4. 被批改后，不覆盖原稿；新增 `reviewed` 或 `revision` 文件。

## 统一归档方式

`homework/` 不是所有 Day 作业最终成品的统一落点，而是：

1. Day 作业入口索引
2. 作业过程稿
3. 导师批改稿 / revision 稿

最终归档按内容属性决定：

- 场景、法规、用户原声、供应链证据类 -> `research/`
- 学习笔记、技术卡片、导师讲解稿 -> `learning/notes/`
- 可直接支撑业务决策的市场进入方案、PRD、ROI、GTM -> `project/`

## 推荐命名

```text
Day01_homework_scenario-map.md
Day01_homework_scenario-map_reviewed.md
Day01_homework_scenario-map_revision.md
```

## 已完成作业索引

已完成作业按内容性质归档到 `research/`，这里保留入口索引：

| Day | 作业 | 归档路径 |
|---|---|---|
| Day 1 | 日本低压储能四大场景地图 | [../../research/japan-market/Day01_japan-low-voltage-storage-scenario-map.md](../../research/japan-market/Day01_japan-low-voltage-storage-scenario-map.md) |
| Day 1 | 公寓/集合住宅防灾停电用户原声 | [../../research/japan-market/Day01_japan-apartment-disaster-power-x-voice.md](../../research/japan-market/Day01_japan-apartment-disaster-power-x-voice.md) |
| Day 1 | 一户建家庭备电用户原声 | [../../research/japan-market/Day01_japan-detached-house-backup-power-x-voice.md](../../research/japan-market/Day01_japan-detached-house-backup-power-x-voice.md) |
| Day 1 | 车中泊/露营/户外备电用户原声 | [../../research/japan-market/Day01_japan-car-camping-outdoor-power-x-voice.md](../../research/japan-market/Day01_japan-car-camping-outdoor-power-x-voice.md) |
| Day 1 | 小型商用备电用户原声 | [../../research/japan-market/Day01_japan-small-business-backup-power-x-voice.md](../../research/japan-market/Day01_japan-small-business-backup-power-x-voice.md) |
| Day 2 | 合规路线图 v0 | [../../research/regulations/Day02_japan-compliance-route-v0.md](../../research/regulations/Day02_japan-compliance-route-v0.md) |
| Day 3 | 九州 60Hz 50kW/100kWh 低压储能规格关键条目 | [../../research/regulations/Day03_kyushu-60hz-50kw-100kwh-low-voltage-storage-spec.md](../../research/regulations/Day03_kyushu-60hz-50kw-100kwh-low-voltage-storage-spec.md) |
| Day 3 | 低压储能核心技术卡片 | [../notes/Day03_low-voltage-storage-core-knowledge-cards.md](../notes/Day03_low-voltage-storage-core-knowledge-cards.md) |

## 作业模板

```md
# Homework：主题

日期：
对应路线：

## 题目

## 我的回答

## 证据

## 我不确定的地方

## 希望 reviewer 检查的点
```
