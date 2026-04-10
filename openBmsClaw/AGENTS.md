# Repository Guidelines

## 1. 适用范围
本文件仅适用于当前 STM32 子工程：

> `openBmsClaw/`

它是当前子工程的补充规则，不是整个 `bmsClaw` 开源项目的总说明。

如果本文件与根目录 [AGENTS.md](/Users/huoward/Project/101.bms/bmsClaw/AGENTS.md) 冲突：

- 以根目录 `AGENTS.md` 为准

本文件只补充三类内容：

1. 当前 `STM32F103C8T6` 起步工程的本地事实
2. 当前子工程的目录边界和编码约束
3. 当前子工程的构建、下载、调试与新手注意事项

## 2. 当前子工程事实
当前可确认的本地事实如下：

- 当前工程名：`openBmsClaw`
- 当前工程路径：`/Users/huoward/Project/101.bms/bmsClaw/openBmsClaw`
- 当前工程由 `STM32CubeIDE for Visual Studio Code` 生成
- 当前 `CMakePresets.json` 指向的器件事实为 `STM32F103C8T6`
- 当前启动文件为 [Src/startup_stm32f103xx.S](/Users/huoward/Project/101.bms/bmsClaw/openBmsClaw/Src/startup_stm32f103xx.S)
- 当前链接脚本为 [stm32f103x8_flash.ld](/Users/huoward/Project/101.bms/bmsClaw/openBmsClaw/stm32f103x8_flash.ld)
- 当前构建产物名为：
  - `openBmsClaw.elf`
  - `openBmsClaw.hex`
  - `openBmsClaw.bin`

这些事实说明：

- 这是当前实际使用的 F103 bring-up 基线
- 它只代表当前起步工程，不代表长期平台芯片路线已经定型
- 长期平台路线仍可保持 `STM32G0`，但当前代码落地先围绕手头 `STM32F103C8T6` 硬件推进

当前默认对应的实物假设是：

- `野火小智 STM32F103C8T6 核心板（双 USB 款）`
- 板上带 `5-pin` 标准 `SWD`
- 板上带 `CH340C USB-UART` 与独立 `USB Device`
- 板上资源含 `4 LEDs` 和 `3 buttons`

如果用户提供了更具体的板卡、调试器、下载器、跳帽状态或芯片版本信息：

- 以用户提供的信息为准

注意：

- `USB-UART` 不等于 `ST-LINK`
- 但这块板并不是“只有 USB-UART”，它还提供可接外部 `DAP / JLINK / ST-LINK` 的 `SWD`
- 如果手头没有可用外部 `SWD` 调试器，则可以先完成 `Configure / Build / UART` 路径，并在 `BOOT` 条件满足时尝试串口下载；但不能写成“已完成 SWD 下载和断点调试”

## 3. 本地目录职责

### 3.1 生成层
以下目录和文件优先视为 ST 生成层或启动层：

- `Src/`
- `Inc/`
- `cmake/`
- `CMakeLists.txt`
- `CMakePresets.json`
- `stm32f103x8_flash.ld`

其中：

- [Src/main.c](/Users/huoward/Project/101.bms/bmsClaw/openBmsClaw/Src/main.c) 是启动入口
- 启动文件、链接脚本、生成工具链文件默认不要随意大改

### 3.2 自定义代码层
后续自定义代码优先进入以下目录：

- `app/`
- `services/`
- `hal/`
- `drivers/`
- `board/`
- `config/`

建议边界如下：

- `app/`：产品行为、运行主流程、状态编排
- `services/`：`bms / power / protocol / ui / bringup` 协调
- `hal/`：隐藏芯片差异，预留后续 BMS / 协议 / 电源适配层
- `drivers/`：`GPIO / UART / I2C / ADC / TIM` 等 MCU 外设访问
- `board/`：当前板级能力、引脚和下载方式假设
- `config/`：功能开关、板型选择、默认能力配置

### 3.3 `main.c` 约束
[Src/main.c](/Users/huoward/Project/101.bms/bmsClaw/openBmsClaw/Src/main.c) 必须保持薄入口。

当前子工程中，`main.c` 只应长期承担：

- `board_init()`
- `app_init()`
- 主循环中调用 `app_run()`

不要把以下内容长期堆进 `main.c`：

- 产品逻辑
- 外设寄存器读写
- 协议状态机
- BMS 策略
- 板级分支判断

### 3.4 生成物规则
- `build/` 是生成物目录
- 不把 `build/` 当成架构事实来源
- 非必要不手工编辑 `build/` 内文件

## 4. 构建、下载与调试规则
当任务目标涉及固件运行验证、下载或断点调试时，当前子工程默认验证顺序为：

1. `Configure`
2. `Build`
3. 下载
4. 调试

此类任务不要跳步。

### 4.1 Configure
先确认工程能被 `STM32CubeIDE for Visual Studio Code` 正常识别，并完成一次 `CMake Configure`。

常用命令：

- `cmake --preset Debug`
- `cmake --preset Release`

通过标准：

- 没有 `CMake Error`
- 能生成 `build/<Preset>/`

### 4.2 Build
在 `Configure` 成功后，再执行编译。

常用命令：

- `cmake --build --preset Debug`
- `cmake --build --preset Release`

通过标准：

- 编译和链接成功
- 至少生成：
  - `openBmsClaw.elf`
  - `openBmsClaw.hex`
  - `openBmsClaw.bin`

### 4.3 下载
当前下载路径要区分两种情况：

1. **有外部 `SWD` 调试器**
   - 走 `SWD` 下载
   - 这是后续断点调试的主路径
2. **只有 `USB-UART`**
   - 可作为串口日志路径
   - 若板子和跳帽条件满足，也可走 STM32 串口 Bootloader 下载
   - 但它不等于 `SWD` 断点调试

如果没有实际完成下载，不得写成“下载已验证完成”。

### 4.4 Debug
只有在 `Build` 成功且 `SWD / ST-LINK` 路径可用后，才进入断点调试。

通过标准：

- 调试会话能启动
- 程序能停在 `main()` 或断点
- 可执行继续运行和单步

如果当前只有 `USB-UART`，则应写成：

- 已完成串口连接验证
- `SWD` 断点调试未验证

### 4.5 完成状态约束
如果没有实际完成以下动作：

- `Configure`
- `Build`
- 下载
- 调试连接
- 板级现象确认

则不得写成“已验证完成”。

应明确写出：

- 未执行
- 执行失败
- 被什么条件阻塞

## 5. 编码与分层规则

### 5.1 基本风格
- 使用 4 空格缩进
- 花括号独占一行
- 函数尽量单一职责
- 命名优先使用模块前缀

例如：

- `app_init`
- `app_run`
- `board_init`
- `bringup_service_init`
- `protocol_service_process`

宏保持全大写，例如：

- `APP_ENABLE_UI_SERVICE`
- `FEATURE_ENABLE_UART_LOG`
- `BOARD_CURRENT_STM32F103C8T6_USB_UART`

### 5.2 分层纪律
严格遵守以下规则：

- `app/` 不直接访问寄存器
- `services/` 不直接访问寄存器
- 芯片差异优先下沉到 `hal/`
- MCU 外设访问优先下沉到 `drivers/`
- 板级差异优先下沉到 `board/`

### 5.3 配置规则
优先把可切换行为放进：

- [config/app_config.h](/Users/huoward/Project/101.bms/bmsClaw/openBmsClaw/config/app_config.h)
- [config/feature_config.h](/Users/huoward/Project/101.bms/bmsClaw/openBmsClaw/config/feature_config.h)
- [config/board_config.h](/Users/huoward/Project/101.bms/bmsClaw/openBmsClaw/config/board_config.h)

不要把功能开关散落成到处都是的条件编译。

## 6. 新手执行注意事项
- 优先小步修改，不一次性大改生成工程
- 先稳定 `Configure / Build / 下载路径`，再追加复杂模块
- 先做 `LED / UART / ADC / I2C` 的最小可观测闭环，再接 `BMS / PD`
- 如果当前没有外部 `ST-LINK`，先把“下载方式”和“串口日志方式”区分清楚
- 出现问题时，优先记录：
  - 工具链路径
  - 当前工程路径
  - 下载方式是 `SWD` 还是 `UART`
  - `BOOT0` 状态
  - 串口号
  - 失败现象
