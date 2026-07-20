# 硬件架构 / Power Tree 模板（卡2 · 启动阶段交付物）

> 用法：Chief Architect 硬件侧产出，由卡2（外包硬件团队）填写、卡1 owner review 后方可进阶段门。与 `03.tech_architecture.md` ADR-001（双芯架构）一致；本模板补 03 缺的硬件框图与 power tree。不含 PCB 布局（PCB 属卡2 开发中交付物）。

## 1. 系统框图

> 用文字或框图描述 MCU（Brain）↔ SoC（Body，英集芯）↔ 外设的连接关系。

- MCU：<型号，如 F103 bring-up / F030 候补>
- SoC：<英集芯型号>
- 关键总线：I2C（MCU↔SoC，SCL/SDA/INT）、<其它>

## 2. Power Tree

| 电源节点 | 来源 | 电压 / 电流 | 供给 | 备注 |
|---------|------|-----------|------|------|
| （示例）VBUS | Type-C 输入 | 5–20V | SoC buck-boost | PD 协商 |
| （示例）VSYS | SoC | <V> | MCU / 外设 | — |
| （示例）VDD_MCU | LDO/DCDC | 3.3V | MCU | — |

## 3. 关键器件

| 器件 | 型号 / 类型 | 角色 | 关联能力 |
|------|-----------|------|---------|
| （示例）电源 SoC | 英集芯 <型号> | buck-boost + PD/QC/UFCS + 硬件保护 | Power / PD |
| （示例）NFC | ST25DV | 数据读出窗口 | NFC |
| （示例）电芯 | <规格> | 储能 | Battery |

## 4. 接口划分（硬件 → 软件边界）

> 明确硬件交给软件的接口，呼应"要接口不要源码"。GPIO/Pin 定义全部落 board 层，不在 app 硬编码。

| 接口 | 硬件侧 | 软件侧（board 层引脚） | 说明 |
|------|-------|---------------------|------|
| （示例）SoC INT | SoC INT 引脚 | PA<x>（EXTI） | 硬件安全中断高速通道 |
| （示例）I2C | SoC SCL/SDA | PB6/PB7 | MCU↔SoC 通信 |

> 三态标注：<已实现 / 当前代码事实 / 已验证>，逐项注明是否已实测。
