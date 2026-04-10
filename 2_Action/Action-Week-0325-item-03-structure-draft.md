# openBmsClaw 工程目录结构落地说明

## 1. 文档目的
本文件对应 `Plan-Week-0325 item-03`，用于把当前实际使用的 `STM32F103C8T6` 起步工程整理成一个可继续扩展的工程骨架。

这里的重点不是“把所有功能一次性写完”，而是先回答三件事：

1. 当前 `openBmsClaw` 工程里，哪些属于 ST 生成层。
2. 后续自定义代码应该放到哪一层。
3. 作为嵌入式新手，怎样扩展才不容易把工程搞乱。

## 2. 当前工程事实
当前实际活动工程位于：

- `/Users/huoward/Project/101.bms/bmsClaw/openBmsClaw`

当前工程事实如下：

- 工程名：`openBmsClaw`
- 生成方式：`STM32CubeIDE for Visual Studio Code`
- 当前器件：`STM32F103C8T6`
- 当前启动文件：`Src/startup_stm32f103xx.S`
- 当前链接脚本：`stm32f103x8_flash.ld`

这意味着：

- 当前真正的 startup project 已经从旧的 `F072` 基线切换到 `F103`
- 旧 `bmsClaw/bmsClaw/bmsClaw/` 可以只视为历史参考，不再作为当前主起步工程

## 3. 当前已落成的最小目录骨架
当前 `openBmsClaw` 已按职责扩展为下面这组目录：

```text
openBmsClaw/
├── Inc/                         # ST 生成层保留位
├── Src/                         # ST 生成层与启动入口
├── app/                         # 产品主流程
├── services/                    # 模块协调层
│   ├── bms/
│   ├── bringup/
│   ├── dock/
│   ├── power/
│   ├── protocol/
│   └── ui/
├── hal/                         # 芯片适配保留层
│   ├── bms/
│   ├── display/
│   ├── power/
│   ├── protocol/
│   └── sense/
├── drivers/                     # MCU 外设驱动保留层
│   ├── adc/
│   ├── gpio/
│   ├── i2c/
│   ├── tim/
│   └── uart/
├── board/                       # 当前板级能力与绑定
├── config/                      # 功能开关与板型配置
├── cmake/
├── CMakeLists.txt
├── CMakePresets.json
└── stm32f103x8_flash.ld
```

## 4. 当前每一层的职责

### 4.1 生成层
以下内容仍视为生成层或启动层：

- `Src/`
- `Inc/`
- `cmake/`
- `CMakeLists.txt`
- `CMakePresets.json`
- `stm32f103x8_flash.ld`

对新手来说，最重要的规则是：

> 不要一开始就大改启动文件、链接脚本和生成工具链文件。

### 4.2 `app/`
当前 `app/` 已用于承接主流程入口：

- `app_init()`
- `app_run()`

它只负责编排，不直接承载板级细节和寄存器操作。

### 4.3 `services/`
当前服务层已经预留了：

- `bms_service`
- `power_service`
- `protocol_service`
- `ui_service`
- `bringup_service`

这些服务当前大多还是空实现，但接口位置已经固定下来，后续接真实芯片时就不必再把逻辑塞回 `main.c`。

### 4.4 `board/`
当前板级层已经落了第一版事实：

- 默认板卡：`野火小智 STM32F103C8T6 核心板（双 USB 款）`
- 当前已确认的板级能力：
  - 板上带 `5-pin SWD`
  - 板上带 `CH340C USB-UART`
  - 板上带 `USB Device`
  - 板上有 `4 LEDs` 与 `3 buttons`
  - `UART log` 是当前最容易优先验证的观测路径
  - `LED / ADC / I2C` 先不宣称已验证

这一步的价值是把“当前板子已知事实”从上层逻辑中隔离出来。

### 4.5 `config/`
当前配置层已拆成：

- `app_config.h`
- `feature_config.h`
- `board_config.h`

这样后续切板、切 SKU、切 bring-up 阶段时，可以优先改配置，而不是到处改业务代码。

## 5. 当前已建立的最小调用关系
当前主流程已经改成：

```text
main()
  -> board_init()
  -> app_init()
  -> for (;;)
       -> app_run()
```

`app_init()` 当前按配置调用：

- `bms_service_init()`
- `power_service_init()`
- `protocol_service_init()`
- `ui_service_init()`
- `bringup_service_init()`

`app_run()` 当前按配置调用对应的 `process()`。

这说明当前工程已经从“空循环”迈到了“有固定主流程骨架”的状态。

## 6. 为什么这样处理更适合当前目标
当前主目标不是做一个只适配单块 F103 教学板的硬编码 demo，而是逐步走向：

- 可定制
- 可替换模块
- 可迁移到后续主路线 MCU

因此，这一版目录结构强调的是：

1. 先按职责拆，不按芯片型号拆。
2. 先把板级差异隔离到 `board/`。
3. 先把芯片适配位留在 `hal/`。
4. 先把主流程和服务层接口固定下来。

## 7. 当前还没有完成的部分
当前骨架已经落成，但下面这些事还不能写成“已完成”：

- `LED` 真实闪烁
- `UART` 真实上电日志
- `ADC` 真实采样
- `I2C` 真实探测
- `SWD` 下载和断点调试
- `USB-C/PD` 扩展板联调
- `BMS` 评估板联调

原因很直接：

- 当前新工程还是极简起步工程
- 还没有真实外设初始化代码
- 当前手头核心板的下载路径与调试路径还需继续确认

## 8. 对后续 item 的承接
- `item-04`：在这个骨架基础上确认 `Configure / Build / 下载 / Debug` 状态
- `item-05`：把 `bringup_service` 对应的 `LED / UART / ADC / I2C` 闭环逐步做实
- `item-06`：在 `services/*` 和 `hal/*` 预留位基础上，再接真实输入链路硬件

## 9. 当前结论
`item-03` 对应的核心目标已经落地：

- 已建立职责清晰的目录骨架
- 已把 `main.c` 改成薄入口
- 已把后续 `BMS / power / protocol / board / config` 的位置固定下来

这一步对当前项目很关键，因为它把 `openBmsClaw` 从“刚生成的 F103 空工程”推进成了“可以继续沉淀模块化结构的 F103 起步工程”。
