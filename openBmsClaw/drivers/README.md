# drivers (SoC 硬件抽象与转译层 / SAL)

本目录在 `openBattery` 最新双核协同架构下，作为跨 SoC 厂商（如英集芯 IP 系列、智融 SW 系列等）的统一接头人与抽象层 (SoC Abstraction Layer)。

## 1. 核心职能
- **抹平厂商差异 (SAL 统一接口)**：对上层应用与服务层（`app/` 和 `services/`）只暴露标准化接口，如 `soc_get_voltage()`、`soc_get_temperature()` 和 `soc_set_ocp()`。业务层无需触碰底层各异的寄存器表或闭源库。
- **总线复位与防呆守护**：当检测到从机无应答或总线处于挂死态时，通过调用底层的 `hal_i2c_bus_recovery()` 执行 GPIO 模拟时钟释放算法，并打印诊断日志，重置通信状态机。
- **兼容起步探测基线**：保留 `adc/` 和 `i2c/` 下的起步探针测试文件 (`*_probe.c`/`h`)，用于单片机 bring-up 初期的双向信道验证。

## 2. 文件组织
```text
drivers/
├── README.md       本说明文档
├── soc_sal.h       SoC 统一标准 API 定义及错误码/端口结构
├── soc_sal.c       SoC 抽象转译实现层 (集成 9 脉冲总线自愈协调与诊断日志输出)
├── adc/            起步 ADC 探针验证模块 (供 bring-up 服务调用)
└── i2c/            起步 I2C 扫描探针验证模块 (供 bring-up 服务调用)
```
