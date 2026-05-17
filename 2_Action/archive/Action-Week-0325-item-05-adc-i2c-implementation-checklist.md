# openBmsClaw ADC / I2C 实施清单

## 1. 文档目的
本文件用于把 `ADC` 单点采样和 `I2C` 初始化 / 探测，明确映射到当前 `openBmsClaw` 工程里应该改哪些文件、先做哪些动作、每一步怎么验收。

本文档不是原理说明，而是当前仓库可直接执行的实施清单。

## 2. 当前代码事实
在开始前，先固定当前工程现状：

1. 当前工程入口仍是薄入口：
   - `Src/main.c`
   - 只做 `board_init()`、`app_init()`、`app_run()`
2. 当前板级初始化集中在：
   - `board/board.c`
   - 这里已经手写了 `LED + UART` 的最小寄存器实现
3. 当前板级能力声明集中在：
   - `board/board_stm32f103c8t6_usb_uart.h`
   - 其中 `BOARD_HAS_ADC_CHANNEL = 0`
   - 其中 `BOARD_HAS_I2C_BUS = 0`
4. 当前功能开关集中在：
   - `config/feature_config.h`
   - 其中 `FEATURE_ENABLE_ADC_PROBE = 0`
   - 其中 `FEATURE_ENABLE_I2C_PROBE = 0`
5. 当前 `bringup` 状态机集中在：
   - `services/bringup/bringup_service.c`
   - 目前只推进阶段，没有真正执行 `ADC / I2C` 动作
6. 当前 `drivers/adc/` 和 `drivers/i2c/` 还只是占位目录。
7. 当前 `CMakeLists.txt` 还没有纳入任何 `drivers/adc/*.c` 或 `drivers/i2c/*.c` 源文件。
8. 当前没有 `.ioc`、`adc.c`、`i2c.c`、`stm32f1xx_hal_msp.c`、`stm32f1xx_hal_conf.h` 这类 Cube/HAL 生成层文件。
9. 当前 `config/app_config.h` 里 `APP_ENABLE_TEMPERATURE_ALARM_SAMPLE = 1`，这意味着现在默认走的是 sample 分支，而不是 `bringup_service` 主线。

结论：

- 当前仓库更接近“最小寄存器基线 + 自定义分层骨架”
- 不是一个已经接入 `CubeMX/HAL ADC/I2C` 的工程

## 3. 推荐路线与不推荐路线

### 3.1 推荐路线
推荐先沿当前仓库已有风格继续推进：

- 保持 `main.c` 薄入口不变
- 不重建整个工程
- 先为 `ADC` 和 `I2C` 各补一个最小驱动
- 再由 `board/` 绑定当前 F103 学习板的通道 / 引脚
- 最后由 `bringup_service` 在 `UART` 日志里输出结果

为什么推荐这条路：

- 与当前 `board.c` 的最小实现风格一致
- 改动范围最小
- 最适合当前“先把最小可观测闭环做实”的目标

### 3.2 不推荐路线
当前阶段不建议直接切到“重新生成一套带 HAL 的完整工程再回迁骨架”。

原因：

- 当前仓库里还没有现成 `.ioc / HAL` 生成层
- 会一次性引入大量新文件和时钟 / MSP / 中断配置
- 对当前阶段的 `ADC` 单点采样和 `I2C` 探测来说，变更面过大

## 4. 总体实施顺序
建议固定顺序如下：

1. 先切回 `bringup` 主线
2. 再补 `ADC` 最小链路
3. 再补 `I2C` 最小链路
4. 每完成一项就立即回填手工测试表

不要并行乱改。

## 5. 第 0 步：先切回 bring-up 主线
这是开始 `ADC / I2C` 前必须先做的事。

### 5.1 需要修改的文件
- `openBmsClaw/config/app_config.h`

### 5.2 要做的动作
- 把 `APP_ENABLE_TEMPERATURE_ALARM_SAMPLE` 从 `1` 改成 `0`
- 保持 `APP_ENABLE_BRINGUP_SERVICE = 1`

### 5.3 原因
如果 sample 还开着，`app.c` 会优先走 sample 分支，`bringup_service` 不会成为当前主流程。

### 5.4 验收标准
- 上电后仍能看到 `boot ok`
- 工程行为确实走 `bringup_service`

## 6. ADC 单点采样实施清单

### 6.1 先确认板级输入点
先不要写代码，先确认一个最小可采样对象。

建议确认内容：

- 当前 F103 学习板上要用哪个 `ADC` 通道
- 这个通道对应哪个引脚
- 这个引脚是否已经引出，是否方便接一个稳定电压源

当前已确定第一版 `ADC probe` 改为：

- `PA6`
- `ADC1_CH6`

建议第一版目标：

- 不追求真实电池采样
- 只追求“采到一个会变化的电压值”

### 6.2 需要修改 / 新增的文件
- 修改：`openBmsClaw/config/feature_config.h`
- 修改：`openBmsClaw/board/board_stm32f103c8t6_usb_uart.h`
- 修改：`openBmsClaw/board/board.h`
- 修改：`openBmsClaw/board/board.c`
- 修改：`openBmsClaw/services/bringup/bringup_service.c`
- 修改：`openBmsClaw/CMakeLists.txt`
- 新增：`openBmsClaw/drivers/adc/adc_probe.h`
- 新增：`openBmsClaw/drivers/adc/adc_probe.c`

### 6.3 每个文件分别做什么

`config/feature_config.h`

- 把 `FEATURE_ENABLE_ADC_PROBE` 改成 `1`

`board/board_stm32f103c8t6_usb_uart.h`

- 在确认实测方案后，把 `BOARD_HAS_ADC_CHANNEL` 改成 `1`
- 只在“板级事实已确认”后再改，不要提前硬开

`drivers/adc/adc_probe.h`

- 定义最小接口，例如：
  - `adc_probe_init()`
  - `adc_probe_read_once()`

`drivers/adc/adc_probe.c`

- 放当前第一版 `ADC` 的最小驱动实现
- 目标仅是：
  - 初始化一个通道
  - 触发一次转换
  - 返回原始值

`board/board.h`

- 增加板级包装接口，例如：
  - `board_adc_probe_init()`
  - `board_adc_probe_read_once()`

`board/board.c`

- 只负责把“当前 F103 板的 ADC 通道选择”绑定到 `drivers/adc`
- 不把业务日志或 bring-up 状态机写进这里

`services/bringup/bringup_service.c`

- 在进入 `BRINGUP_STAGE_ADC` 时，真正执行一次 `ADC` 初始化和单点采样
- 通过 `board_uart_write_string()` 输出：
  - 通道号
  - 原始值
  - 可选的换算值

`CMakeLists.txt`

- 把 `drivers/adc/adc_probe.c` 加进 `sources_SRCS`

### 6.4 `ADC` 的最小实现目标
第一版只要求做到：

- 初始化成功
- 能读取一次原始值
- 能通过 `UART` 打印结果

先不要做：

- 多通道扫描
- DMA
- 连续采样
- 校准算法
- 复杂滤波

### 6.5 `ADC` 完成判定
- 工程能重新编译通过
- 上电后日志中出现 `ADC` 原始值
- 改变输入条件时，数值发生变化
- 手工测试表 `T07` 能回填为有效结果

## 7. I2C 初始化 / 探测实施清单

### 7.1 先确认总线目标
第一版 `I2C` 不要求马上接真实 BMS 或协议板。

建议先明确：

- 当前打算使用哪组 `I2C` 引脚
- 当前是否已经接了一个已知地址设备
- 如果没有设备，是否先只做初始化 + 空扫描

建议第一版目标：

- 没设备也能把 `I2C init ok` 和“扫描完成”跑出来

当前建议的默认起步口径：

- 优先尝试 `I2C1`
- 优先检查 `PB6 = SCL`、`PB7 = SDA`
- 如果当前学习板上这组脚被其他功能占用，再改选第二组

### 7.2 需要修改 / 新增的文件
- 修改：`openBmsClaw/config/feature_config.h`
- 修改：`openBmsClaw/board/board_stm32f103c8t6_usb_uart.h`
- 修改：`openBmsClaw/board/board.h`
- 修改：`openBmsClaw/board/board.c`
- 修改：`openBmsClaw/services/bringup/bringup_service.c`
- 修改：`openBmsClaw/CMakeLists.txt`
- 新增：`openBmsClaw/drivers/i2c/i2c_probe.h`
- 新增：`openBmsClaw/drivers/i2c/i2c_probe.c`

### 7.3 每个文件分别做什么

`config/feature_config.h`

- 把 `FEATURE_ENABLE_I2C_PROBE` 改成 `1`

`board/board_stm32f103c8t6_usb_uart.h`

- 在确认当前板级接线后，把 `BOARD_HAS_I2C_BUS` 改成 `1`

`drivers/i2c/i2c_probe.h`

- 定义最小接口，例如：
  - `i2c_probe_init()`
  - `i2c_probe_scan()`

`drivers/i2c/i2c_probe.c`

- 放当前第一版 `I2C` 最小驱动实现
- 第一版只要求：
  - 总线初始化
  - 遍历常见 7-bit 地址范围做探测

`board/board.h`

- 增加板级包装接口，例如：
  - `board_i2c_probe_init()`
  - `board_i2c_probe_scan_once()`

`board/board.c`

- 负责把当前板子的 `I2C` 引脚选择绑定到 `drivers/i2c`
- 不把扫描日志直接塞到驱动里

`services/bringup/bringup_service.c`

- 在进入 `BRINGUP_STAGE_I2C` 时：
  - 先初始化 `I2C`
  - 再执行一次扫描
  - 把结果通过 `UART` 打印出来

`CMakeLists.txt`

- 把 `drivers/i2c/i2c_probe.c` 加进 `sources_SRCS`

### 7.4 `I2C` 的最小实现目标
第一版只要求做到：

- 总线初始化成功
- 扫描过程可执行完成
- 有设备时打印地址
- 没设备时也打印“未发现设备”

先不要做：

- 中断模式
- DMA
- 多设备管理
- 重试策略
- 真正的 BMS / 协议寄存器读写

### 7.4.1 推荐的第一轮实施步骤
建议直接照这个顺序做：

1. 先查板卡资料，确认 `PB6 / PB7` 当前是否空闲可用。
2. 在 `board/board_stm32f103c8t6_usb_uart.h` 中把 `BOARD_HAS_I2C_BUS` 改成 `1`。
3. 在 `config/feature_config.h` 中把 `FEATURE_ENABLE_I2C_PROBE` 改成 `1`。
4. 新增：
   - `drivers/i2c/i2c_probe.h`
   - `drivers/i2c/i2c_probe.c`
5. 在 `board.h / board.c` 中增加板级包装接口。
6. 在 `CMakeLists.txt` 中把 `drivers/i2c/i2c_probe.c` 纳入构建。
7. 在 `bringup_service.c` 中：
   - 先打 `i2c init start`
   - 初始化成功后打 `i2c init ok`
   - 再做一次地址扫描
   - 最后打 `i2c scan done`
8. 第一轮即使没有外设，也要保证“初始化 + 空扫描 + 明确日志”三件事跑通。
9. 跑通后再回填手工测试表 `T08`。

### 7.4.2 第一轮日志建议
建议第一版至少输出下面几类日志：

- `i2c init start`
- `i2c init ok`
- `i2c found: 0x3C` 这类地址日志
- 或 `i2c scan done, no device found`

### 7.5 `I2C` 完成判定
- 工程能重新编译通过
- 上电后日志中出现 `I2C init ok` 或明确错误码
- 扫描过程有明确输出
- 手工测试表 `T08` 能回填为有效结果

## 8. 需要同时更新的测试与文档
每做完一项，都要同步更新：

- `2_Action/Action-Week-0325-item-07-manual-test-sheet.md`
  - `T07 ADC`
  - `T08 I2C`
- `2_Action/Action-Week-0325-item-05-openBmsClaw-bringup.md`
  - 回写哪些已完成，哪些仍待验证

## 9. 推荐的最小实施批次

### 批次 A：只做 `ADC`

1. 切回 `bringup` 主线
2. 开 `FEATURE_ENABLE_ADC_PROBE`
3. 新增 `drivers/adc/*`
4. 绑到 `board/*`
5. 在 `bringup_service` 打印一次 ADC 值
6. 回填 `T07`

### 批次 B：再做 `I2C`

1. 开 `FEATURE_ENABLE_I2C_PROBE`
2. 新增 `drivers/i2c/*`
3. 绑到 `board/*`
4. 在 `bringup_service` 打印 `I2C init` 和扫描结果
5. 回填 `T08`

不要把 `ADC` 和 `I2C` 第一次实现混成一个大改动。

## 10. 当前结论
对当前 `openBmsClaw` 来说，`ADC` 和 `I2C` 的实施关键，不是“先把外设理论讲全”，而是先把下面三件事做实：

1. 让 `bringup` 主线真正生效
2. 让 `drivers/adc` 和 `drivers/i2c` 从占位目录变成最小可运行实现
3. 让 `UART` 日志能够输出 `ADC / I2C` 的第一条真实结果

只要按这份清单推进，后面从 `F103` 学习板切到 `G0` 长期路线时，迁移成本也会更低。
