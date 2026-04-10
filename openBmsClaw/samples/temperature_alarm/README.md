# Temperature Alarm Sample

这个 sample 独立放在 `samples/temperature_alarm`，目的是学习 STM32 的最小温度采样、阈值判断、LED 告警和蜂鸣器告警，不影响后续主线方案结构。

## 接线

Wildfire F103C8 核心板接线如下：

- 热敏模块 `VDD` -> `3V3`
- 热敏模块 `GND` -> `GND`
- 热敏模块 `AO` -> `PA4` (`A4`, `ADC1_CH4`)
- 热敏模块 `DO` -> `PA5` (`A5`)
- 有源蜂鸣器 `3V3` -> `3V3`
- 有源蜂鸣器 `GND` -> `GND`
- 有源蜂鸣器 `SIG` -> `PB9` (`B9`)
- 外接红色 LED：`PB1` (`B1`) -> `限流电阻` -> `LED 正极`
- 外接红色 LED：`LED 负极` -> `GND`
- 板载 `LED1` -> `PA1` (`A1`)
- 板载 `LED2` -> `PA2` (`A2`)
- 板载 `LED3` -> `PA3` (`A3`)

注意：

- 如果你用的是“裸 LED 两个脚”，必须串一个限流电阻，常见可先用 `330R` 到 `1k`
- 不能把裸 LED 直接硬接到 `PB1` 和 `GND`
- 如果你用的是自带电阻的小模块，则按模块说明接

## 原理

这个 sample 能区分 `30C` 和 `40C` 的原因：

- 热敏模块手册说明 `AO` 是给 `ADC` 采样的模拟输出
- 热敏模块手册说明 `DO` 只是板上比较器给出的单阈值数字输出
- 配套 F103 例程给了 `AO -> PA4`、`DO -> PA5`，并给出热敏电阻 `B=3950`

因此本 sample 的判断方式是：

- `AO + ADC` 作为主判断输入
- `DO` 只作为辅助观察量打印到串口，不用于区分 `30C/40C`

当前策略：

- 上电自检：`LED1/LED2/LED3` 同步闪烁约 3 秒，蜂鸣器响 2 下
- `LED2`：上电后持续闪烁，作为心跳灯
- `< 20.0C`：只有 `LED2` 心跳，`LED3` 和外接红色 LED 熄灭，蜂鸣器关闭
- `>= 20.0C` 且 `< 30.0C`：进入 `warning`，`LED3` 和外接红色 LED 同步闪烁，蜂鸣器关闭
- `>= 30.0C`：进入 `alarm`，`LED3` 和外接红色 LED 更快闪烁，同时有源蜂鸣器鸣叫
- 温度降回阈值以下后，对应告警会关闭：
  - 低于 `30.0C`：蜂鸣器停止
  - 低于 `20.0C`：`LED3` 和外接红色 LED 停止闪烁

## 编译

当前工程不是 `make`，而是 `CMake + Ninja`。

### 终端方式

在工程根目录执行：

```bash
pwd
cmake --preset Debug
cmake --build --preset Debug
```

如果输出是：

```bash
ninja: no work to do.
```

表示当前源码没有变化，产物已经是最新状态，不是报错。

### VS Code 方式

1. 用 VS Code 打开 `openBmsClaw/`
2. 按 `Cmd+Shift+P`
3. 执行 `CMake: Select Configure Preset`
4. 选择 `Debug`
5. 执行 `CMake: Configure`
6. 执行 `CMake: Build`

## 确认产物

编译完成后，可在终端检查：

```bash
ls build/Debug/openBmsClaw.*
```

正常应至少看到：

- `build/Debug/openBmsClaw.elf`
- `build/Debug/openBmsClaw.hex`
- `build/Debug/openBmsClaw.bin`

含义：

- `openBmsClaw.elf`：调试和下载最常用
- `openBmsClaw.hex`：常见烧录文件
- `openBmsClaw.bin`：裸二进制文件

## 下载到板子

### 方式 1：外部 SWD 调试器

这是当前推荐路径，也是后续调试主路径。

硬件条件：

- 核心板 `SWD` 接口
- 外部 `ST-LINK` / `J-LINK` / `DAP`

基本步骤：

1. 给核心板接好 `SWD`
2. 保持 sample 接线已经接好
3. 在 VS Code 里先完成一次 `Configure + Build`
4. 用下载工具把 `build/Debug/openBmsClaw.elf` 或 `build/Debug/openBmsClaw.hex` 烧到板子

如果你使用 STM32CubeIDE for VS Code 的下载按钮，通常选 `Debug` 产物对应的 `elf` 即可。

注意：

- 我这里已经验证了 `Configure + Build`
- 还没有实际验证你手上的 `SWD` 下载链路

### 方式 2：只有 USB-UART

这块板的 `USB TO UART` 主要用于串口日志，不等于 `SWD` 下载。

如果你手头没有外部 `ST-LINK/J-LINK/DAP`：

- 可以先完成编译
- 可以先查看串口日志
- 串口下载只有在 `BOOT0`、复位、串口烧录工具都配置正确时才可行

当前 README 不把 UART 下载作为已验证路径。

## 查看串口输出

这块板有独立 `USB TO UART` 接口，接电脑后通常会枚举为 `CH340` 串口。

### 1. 连接方式

- 用数据线连接板子的 `USB TO UART` 口到电脑
- 安装好 `CH340` 驱动

### 2. 查串口号

macOS 终端可执行：

```bash
ls /dev/tty.*
ls /dev/cu.*
```

通常会看到类似：

- `/dev/cu.wchusbserial...`
- `/dev/tty.wchusbserial...`

### 3. 串口参数

当前 sample 使用：

- 波特率：`115200`
- 数据位：`8`
- 停止位：`1`
- 校验位：`None`

也就是常说的 `115200 8N1`。

### 4. 查看工具

可以使用：

- 串口调试助手
- `screen`
- `minicom`
- `CoolTerm`

例如 macOS 下用 `screen`：

```bash
screen /dev/cu.wchusbserialxxxx 115200
```

退出 `screen`：

1. 按 `Ctrl+A`
2. 再按 `K`
3. 输入 `y`

### 5. 新手推荐操作顺序

1. 先确认你接的是板子的 `USB TO UART` 口，不是 `USB DEVICE` 口
2. 把串口工具先打开，再给板子上电或按一次 `RESET`
3. 如果串口里什么都没有，先检查：
   - 串口号是不是选错了
   - 波特率是不是 `115200`
   - 数据线是不是只支持充电不支持数据
4. 如果串口工具一打开就报“端口被占用”，先把别的串口工具或旧的 `screen` 会话关掉

### 6. 正常日志

上电或复位后，正常应能看到类似：

```text
boot ok
sample: temperature alarm
temp sample wiring: AO->PA4 DO->PA5 SIG->PB9 LED1->PA1 LED2->PA2 LED3->PA3
temp sample selftest: LED1/LED2/LED3 blink ~3s, buzzer beeps twice
temp sample policy: LED2 heartbeat, >=warn LED3+AUXLED blink, >=alarm LED3+AUXLED+buzzer
```

运行中日志改成了固定周期输出。

说明：

- 主循环一直在跑，每圈都会重新采样温度
- 但串口日志不是每圈都打，而是按当前 sample 的循环节拍，约每 `1000ms` 打一条
- 这里的 `1000ms` 是 sample 里的近似值，因为当前用的是 `busy_wait`，不是硬件定时器

所以如果当前一直稳定在 `25.3C`，你会周期性看到类似：

```text
temp sample warn temp=25.3C adc=2035 do=1
```

如果温度继续变化，日志内容也会跟着变化。例如：

```text
temp sample warn temp=25.2C adc=xxxx do=0
temp sample warn temp=25.3C adc=xxxx do=0
temp sample alarm temp=35.1C adc=xxxx do=1
temp sample warn temp=34.8C adc=xxxx do=1
temp sample normal temp=24.7C adc=xxxx do=0
```

## 相关代码

- 开关入口：[config/app_config.h](config/app_config.h)
- 主接入点：[app/app.c](app/app.c)
- sample 逻辑：[samples/temperature_alarm/temperature_alarm_sample.c](samples/temperature_alarm/temperature_alarm_sample.c)
- sample 硬件访问：[samples/temperature_alarm/temperature_alarm_sample_port.c](samples/temperature_alarm/temperature_alarm_sample_port.c)
