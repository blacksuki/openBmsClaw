# config (系统配置与资源裁剪入口层)

配置层是整个 `openBattery` 项目的硬件约束与特性统管中心。为了避免将当前开发板（如野火 STM32F103 核心板）的局限性硬编码到上层业务逻辑中，所有资源限制、特性开关及板级映射均在本目录下统一管理。

## 1. 核心头文件架构

```text
config/
├── sys_config.h       系统总体配置入口 (包含 V0 资源约束、UI 裁剪与总线防死锁规则)
├── feature_config.h   基础外设探针与功能特性使能开关
├── app_config.h       应用层及系统服务级运行开关
└── board_config.h     目标开发板硬件引脚及外设映射选择
```

## 2. 面向 Day 1 的 F103 资源裁剪方案 (64KB Flash 限制)

在当前 V0 双向通信基线打通阶段，受限于测试单片机的存储资源，我们在 `sys_config.h` 中实施了严格的资源纪律：
- **UI 裁剪宏 (`CONFIG_ENABLE_OLED_DISPLAY`)**：默认置 0，屏蔽复杂图形界面渲染。
- **大字库屏蔽 (`CONFIG_ENABLE_LARGE_FONT`)**：默认置 0，杜绝多语言中日韩大点阵位图对 Flash 空间的无效挤占。
- **动态动画裁剪 (`CONFIG_ENABLE_COMPLEX_UI_ANIM`)**：默认置 0，降低主频与计算负担，专注于 I2C/UART 底层报文。

## 3. 对新手的开发建议
1. 任何新增的硬件探针或应用层服务，都必须在 `feature_config.h` 或 `app_config.h` 中提供独立的使能开关。
2. 严禁在 `app/` 或 `services/` 业务代码中直接写死具体的硬件参数或时序，必须引用 `sys_config.h` 提供的规范宏。
