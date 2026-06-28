我认为，这个需求已经不是一个 Prompt 设计问题，而是一个 **Agent 产品设计** 问题。

你设计的对象不是"储能导师"，而是一个**Industry Mentor Agent Framework（行业导师框架）**。

它应该能够支持：

* 储能
* 半导体
* 医疗
* 金融
* AI
* 制造业
* SaaS
* ...

换句话说，**行业只是数据，不应该写进 Agent。**

---

# 一、重新定义需求

我建议先把需求抽象。

目标不是：

> 做一个储能导师。

而是：

> **做一个能够快速帮助任何学习者建立某个行业整体认知的导师 Agent。**

因此：

```
                行业知识
                    ↑
                Knowledge Layer
                    ↑
                Mentor Agent
                    ↑
                  Student
```

Agent 本身不关心行业。

Agent 关心：

> 如何教。

---

# 二、核心能力分析

我通常会拆成五层。

```
User

↓

Learning Goal

↓

Learning Diagnosis

↓

Teaching Strategy

↓

Knowledge Delivery

↓

Assessment

↓

Adaptive Planning
```

这六层实际上构成一个完整的 Tutor System。

---

## 第一层：Learning Goal（学习目标）

Agent 第一件事情，不是开始讲课。

而是回答：

> **为什么学习？**

例如：

同样学习储能。

有人是：

```
找工作
```

有人：

```
创业
```

有人：

```
投资
```

有人：

```
销售
```

有人：

```
CTO
```

学习路线完全不同。

所以：

第一步必须确定：

```
Learning Goal
```

---

## 第二层：Learning Diagnosis（学习诊断）

这是很多 Prompt 没有的。

Agent 应该先了解：

### 当前水平

例如：

```
完全不会

↓

知道一点

↓

行业从业者

↓

专家
```

---

### 背景

例如：

```
软件

AI

咨询

制造

销售

财务

电气

化学
```

---

### 学习方式

例如：

喜欢：

```
视频

案例

画图

读论文

项目

问答
```

---

### 时间

例如：

```
3天

2周

1个月

半年
```

---

### 输出目标

例如：

```
能聊天

能做方案

能卖产品

能做研发

能创业
```

---

这些都会影响课程。

---

# 第三层：Teaching Strategy

不是所有行业都一样。

Agent 要决定：

采用什么方法。

例如：

```
Executive Overview
```

适合：

经理。

---

```
Project Based Learning
```

适合：

创业。

---

```
Case Study
```

适合：

咨询。

---

```
Reverse Engineering
```

适合：

产品经理。

---

```
First Principle
```

适合：

技术人员。

---

所以：

Teaching Strategy

也是动态决定。

---

# 第四层：Knowledge Map

这里行业开始进入。

例如：

储能。

Agent 自动生成：

```
Industry

↓

Market

↓

Supply Chain

↓

Product

↓

Technology

↓

Competition

↓

Business

↓

Future
```

如果：

AI。

可能就是：

```
History

↓

Foundation Model

↓

Agent

↓

Application

↓

Infrastructure
```

Agent 不应该固定。

而应该：

动态生成：

Knowledge Graph。

---

# 第五层：Assessment

很多学习 Agent 完全没有。

实际上：

应该持续：

```
学习

↓

提问

↓

测试

↓

修正

↓

继续
```

而不是：

一直输出。

---

# 三、需要主动询问哪些问题？

我建议不要一次问二十个问题。

用户会放弃。

建议采用**渐进式诊断**。

---

## 第一轮（3 个问题）

这是最重要的。

### Q1

为什么想学习这个行业？

例如：

```
找工作

创业

投资

研发

管理
```

---

### Q2

希望多久达到目标？

```
3天

2周

1个月

半年
```

---

### Q3

最终希望做到什么？

例如：

```
能理解行业

能分析企业

能制定战略

能开发产品

能领导团队
```

---

有了这三个。

课程已经能定。

---

## 第二轮

如果需要。

Agent 再问：

```
目前水平？

专业背景？

每天时间？

```

不用全部问。

按需。

---

# 四、Agent 内部应该如何组织？

我建议内部维护一个 Learning State。

例如：

```yaml
student:

  goal:

  industry:

  duration:

  background:

  current_level:

  target_level:

  preferred_style:

learning:

  roadmap:

  completed:

  weaknesses:

  strengths:

  quizzes:

  homework:

mentor:

  strategy:

  pace:

  depth:
```

这样。

Agent 每次都知道：

学生在哪里。

---

# 五、我建议增加一个「诊断阶段」

很多 Mentor Prompt：

直接开始：

```
第一课
```

其实是不合理的。

应该：

```
Diagnosis

↓

Generate Roadmap

↓

Student Review

↓

Adjust

↓

Start Learning
```

这一步非常重要。

---

# 六、一个更完整的通用框架

如果把这个做成真正可复用的 Industry Mentor Agent，我建议拆成四个独立模块，而不是一个超长 Prompt：

| 模块                     | 职责                            | 是否行业相关         |
| ---------------------- | ----------------------------- | -------------- |
| **Learning Diagnoser** | 识别用户目标、背景、时间、已有知识和学习偏好，生成学习画像 | 否              |
| **Curriculum Planner** | 根据学习画像和目标自动生成学习路径、阶段目标和里程碑    | 基本无关（行业知识作为输入） |
| **Mentor**             | 执行每日教学、提问、监督、调整节奏             | 否              |
| **Reviewer/Examiner**  | 测验、复盘、指出薄弱点、决定是否进入下一阶段        | 否              |

**行业知识（储能、AI、金融等）应作为独立的 Knowledge Pack 插入，而不是写死在 Agent 中。**

这样，你最终得到的就不是一个"储能导师 Prompt"，而是一个真正可扩展的**Industry Mentor Framework**：只需替换行业知识包，就可以快速生成面向不同领域的短期入门培训 Agent。
