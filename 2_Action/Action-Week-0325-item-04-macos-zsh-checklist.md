# openBmsClaw macOS + zsh 工程稳定化清单

## 1. 文档目的与适用范围
本文件对应 `Plan-Week-0325 item-04`，用于整理当前 `openBmsClaw` 的工程稳定化状态。

适用范围固定为：

- 当前系统：`macOS + zsh`
- 当前工程路径：`/Users/huoward/Project/101.bms/bmsClaw/openBmsClaw`
- 当前主工具：`STM32CubeIDE for Visual Studio Code`
- 当前器件：`STM32F103C8T6`
- 当前学习板：`野火小智 STM32F103C8T6 核心板（双 USB 款）`

## 2. 当前已知工程事实
当前已知事实如下：

- `Configure` 已完成
- `Build` 已完成
- 在终端侧，重新 `Build` 时若未把 ST 工具链加入 `PATH`，会在 post-build 阶段报：
  - `arm-none-eabi-size: command not found`
- 在补上下面这个工具链目录后，终端侧 `Build` 已再次通过：
  - `$HOME/Library/Application Support/stm32cube/bundles/gnu-tools-for-stm32/14.3.1+st.2/bin`
- 当前构建产物已经存在：
  - `build/Debug/openBmsClaw.elf`
  - `build/Debug/openBmsClaw.hex`
  - `build/Debug/openBmsClaw.bin`
  - `build/Debug/openBmsClaw.map`

当前生成层关键文件如下：

- `Src/startup_stm32f103xx.S`
- `stm32f103x8_flash.ld`
- `cmake/vscode_generated.cmake`

当前学习板还应补充记住这些板级事实：

- 板上有 `5-pin` 标准 `SWD`
- 板上有 `CH340C USB-UART`
- 板上有独立 `USB Device`
- 板上资源有 `4 LEDs` 和 `3 buttons`

## 3. 当前完成状态
当前 `item-04` 应按下面的状态理解：

| 检查项 | 当前状态 | 说明 |
| --- | --- | --- |
| `Configure` | 已完成 | 用户已确认 |
| `Build` | 已完成 | 用户已确认，且产物已存在 |
| 下载 | 已完成 | 已通过 `STM32CubeProgrammer + ST-LINK` 完成程序下载 |
| 调试 | 未验证 | 当前 `ST-LINK` 下载链路已通，但 VS Code 断点调试仍待验证 |

这表示：

- 当前软件工程已经具备“可继续维护”的基础
- 当前板级下载已经具备事实依据
- VS Code 断点调试还不能写成“已跑通”
- 终端侧构建依赖 ST 工具链 PATH，VS Code 插件成功不自动代表 shell 环境已经配置完整

## 4. 当前最重要的概念区分

### 4.1 `USB-UART` 不是调试器，但这块板并不只有 `USB-UART`
当前你手头的学习板是 `野火小智 STM32F103C8T6 核心板（双 USB 款）`。它板上同时提供：

- `CH340C USB-UART`
- `USB Device`
- `5-pin` 标准 `SWD`

其中 `USB-UART` 通常只解决：

- 供电
- 串口日志
- 某些场景下的串口 Bootloader 下载

它默认不等于：

- 外部 `SWD` 调试器
- 断点调试

真正的断点下载 / 调试，仍应走板上的 `SWD` 接口。

### 4.2 item-04 的真实通过标准
当前 item-04 的四步顺序仍然是：

1. `Configure`
2. `Build`
3. 下载
4. 调试

其中前两步已经具备事实依据；后两步还需要硬件链路确认。

### 4.3 什么是 `SWD`
`SWD` 是 `Serial Wire Debug`，中文可理解为“串行线调试接口”。

它的作用是：

- 把程序下载进 `STM32`
- 让调试器读取和控制芯片运行状态
- 支持断点、单步、查看变量、查看寄存器

对当前这块 `STM32F103` 学习板来说：

- `USB-UART` 主要用于串口日志和某些串口下载场景
- `SWD` 才是标准的下载 / 断点调试链路

所以：

- `CubeProgrammer + ST-LINK` 下载成功，说明当前 `SWD` 下载链路已经跑通
- 下一步要补的是 `VS Code` 中的断点调试链路

## 5. 现在该怎么验证“下载”
当前有两条可能路径：

### 5.1 路径 A：外部 `SWD` 调试器
这块板的规格书明确支持通过标准 `SWD` 接口外接：

- `DAP`
- `JLINK`
- `ST-LINK`

其中项目标准推荐路径仍是 `ST-LINK`。

当前可直接按下面的 `SWD` 对应关系接线：

| 调试器侧 | F103 目标板侧 | 说明 |
| --- | --- | --- |
| `SWDIO` | `SWDIO` / `PA13` | 必接 |
| `SWCLK` | `SWCLK` / `PA14` | 必接 |
| `GND` | `GND` | 必接，共地 |
| `NRST` | `NRST` / `RST` | 强烈建议接 |
| `3V3` / `VREF` | `3V3` | 可选，常用于电平参考 |

如果调试器丝印使用老 `JTAG` 风格，则按下面理解：

- `TMS -> SWDIO`
- `TCK -> SWCLK`
- `RST -> NRST`
- `GND -> GND`
- `VREF -> 3V3`

接线建议固定为两档：

- 最小可用接线：`SWDIO + SWCLK + GND`
- 推荐稳定接线：`SWDIO + SWCLK + GND + NRST`

执行顺序：

1. 按板卡丝印把 `SWDIO / SWCLK / GND / NRST / 3.3V` 接到外部调试器
2. 使用与当前调试器匹配的主机工具
3. 如果使用 `ST-LINK`，在 `STM32CubeProgrammer` 中选择 `ST-LINK + SWD`
4. 若连接成功，再下载 `openBmsClaw.elf` 或 `openBmsClaw.hex`

通过标准：

- 目标芯片能被识别
- 可以下载程序

当前已知事实补充：

- 你已经通过 `STM32CubeProgrammer + ST-LINK` 完成过一次真实下载
- 因此这条 `SWD` 下载路径当前可记为已验证

### 5.2 路径 B：串口 Bootloader
如果当前没有 `ST-LINK`，可以把它视为“下载应急路径”，但它不等于调试路径。

需要额外确认的板级事实包括：

- 板上是否引出了 `BOOT0`
- 如何把板子切到系统 Bootloader 模式
- 当前 `USB-UART` 对应的是 `CH340C`
- 串口下载默认相关引脚是 `PA10 / PA9`

当前文档只把这条路径记为“可能路径”，不把它写成“已验证完成”。

## 6. 现在该怎么验证“调试”
当前 `SWD` 下载链路已经跑通，下一步就是在 `VS Code` 中验证断点调试。

### 6.1 在 `VS Code` 中什么叫“断点调试”
这里的“断点调试”指的是：

- 把程序下载到板子
- 启动调试会话
- 让程序停在 `main()` 或你设置的断点
- 可以执行“继续运行 / 单步 / 查看变量”

如果只完成了下载，但不能停断点、不能单步，那还不算“调试已完成”。

### 6.2 `VS Code` 中的具体操作步骤
建议按下面顺序操作：

1. 用 `STM32CubeIDE for Visual Studio Code` 打开 `openBmsClaw` 工程目录。
2. 确认当前工程已经完成一次 `Configure` 和 `Build`。
3. 确认 `ST-LINK` 已按 `SWD` 正确接线，并且目标板已经上电。
4. 在左侧活动栏进入 `Run and Debug`。
5. 如果界面里已有 STM32 相关调试配置，直接选择当前工程对应的调试项。
6. 如果界面里没有现成调试项：
   - 打开命令面板
   - 查找 STM32 / Cortex 调试相关入口
   - 选择当前工程对应的 `ST-LINK + SWD` 调试配置
7. 先在 [main.c](/Users/huoward/Project/101.bms/bmsClaw/openBmsClaw/Src/main.c) 的 `main()` 入口点一个断点。
8. 点击 `Start Debugging` 或 `F5`。
9. 观察是否出现以下现象：
   - 调试会话启动
   - 程序停在 `main()`
   - 可以执行 `Continue`
   - 可以执行 `Step Over`
10. 如果能停在断点，再继续验证第二个断点，例如放在 `board_init()` 或 `app_init()`。

### 6.3 调试通过标准
调试通过标准：

- VS Code 能启动调试会话
- 程序能停在 `main()`
- 可以单步

### 6.4 调试失败时先看什么
如果在 VS Code 中无法进入断点调试，优先检查：

- `ST-LINK` 是否仍能在 `STM32CubeProgrammer` 中正常连接
- 当前是否真的是 `ST-LINK + SWD` 调试配置，而不是串口配置
- 工程是否重新 `Build` 成功
- 板子是否掉电或 `NRST` 没接稳
- 是否真的给 `main.c` 设置了可命中的断点

### 6.5 当前状态记录口径
基于当前事实，更合理的记录应为：

- `Configure`：通过
- `Build`：通过
- `SWD` 下载：通过
- `SWD` 断点调试：未验证

## 7. 当前推荐的一次性检查清单
在终端中先检查当前工程与产物：

```bash
cd /Users/huoward/Project/101.bms/bmsClaw/openBmsClaw
pwd
ls build/Debug
```

应至少看到：

- `openBmsClaw.elf`
- `openBmsClaw.hex`
- `openBmsClaw.bin`

然后继续记录以下信息：

- 当前开发板：`野火小智 STM32F103C8T6 核心板（双 USB 款）`
- 当前是否有外部 `SWD` 调试器
- 当前是否已确认 `BOOT0`
- 当前串口号
- 当前下载尝试结果

## 8. 当前 item-04 的结论
基于当前事实，`item-04` 应写成：

- 工程已能 `Configure + Build`
- 当前代码起步工程已稳定到可持续扩展
- 已通过 `STM32CubeProgrammer + ST-LINK` 完成下载
- VS Code 断点调试仍待验证

这比“直接宣称整项完成”更准确，也更符合当前真实状态。
