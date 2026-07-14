# Stage A SoC Runtime Closure Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Wire `soc_sal_init()` into the F103 bring-up runtime path, complete the `EXTI -> callback -> service` software闭环, and produce the first real-SoC + logic-analyzer evidence for Stage A.

**Architecture:** `bringup_service` becomes the only Stage A orchestration point: it captures the probed I2C address, calls `soc_sal_init()`, runs one online poll, and then triggers the SoC INT selftest. `soc_manager` remains the sole SAL owner for I2C/EXTI setup and exposes only readiness/logging helpers, while bench evidence is written into a dedicated `2_Action` record instead of being mixed back into code comments.

**Tech Stack:** C11, STM32F103 bare-metal startup code, custom `hal_i2c`/`hal_exti`, CMake + Ninja (`cmake --preset Debug`, `cmake --build --preset Debug`), UART logs, logic analyzer

---

## File Map

- `openBmsClaw/services/bringup/bringup_service.h`
  - Expand the bring-up state machine with a dedicated `SOC_SAL_INIT` stage.
- `openBmsClaw/services/bringup/bringup_service.c`
  - Capture the probed SoC address, call `soc_sal_init()`, log online/offline status, and verify selftest pass/fail.
- `openBmsClaw/drivers/soc/soc_api.h`
  - Add a minimal SAL readiness query for service-side observability without leaking HAL details.
- `openBmsClaw/drivers/soc/soc_manager.c`
  - Add init/EXTI logs and implement the readiness query.
- `2_Action/Action-StageA-SoC-Validation.md`
  - Record the first real-board ChipID/data readback and logic-analyzer evidence.

## Verification Rules

- Code verification uses:
  - `cd openBmsClaw && cmake --preset Debug && cmake --build --preset Debug`
- Dependency-boundary verification uses:
  - `cd openBmsClaw/build/Debug && ninja -t deps CMakeFiles/openBmsClaw.dir/services/bringup/bringup_service.c.obj`
- Bench verification requires:
  - UART log capture
  - at least one real I2C waveform screenshot or analyzer export

### Task 1: Wire `soc_sal_init()` Into the Bring-up State Machine

**Files:**
- Modify: `openBmsClaw/services/bringup/bringup_service.h:6-18`
- Modify: `openBmsClaw/services/bringup/bringup_service.c:9-184`
- Test: `rg -n "soc_sal_init\\(" openBmsClaw/app openBmsClaw/services`
- Test: `cd openBmsClaw && cmake --preset Debug && cmake --build --preset Debug`

- [ ] **Step 1: Prove the current gap**

Run: `rg -n "soc_sal_init\\(" openBmsClaw/app openBmsClaw/services`
Expected: no hit in `openBmsClaw/app/` or `openBmsClaw/services/`; runtime code does not call `soc_sal_init()`.

- [ ] **Step 2: Add a dedicated bring-up stage for SAL init**

```c
typedef enum
{
    BRINGUP_STAGE_BOOT = 0,
    BRINGUP_STAGE_LED,
    BRINGUP_STAGE_UART,
    BRINGUP_STAGE_ADC,
    BRINGUP_STAGE_I2C,
    BRINGUP_STAGE_SOC_SAL_INIT,
    BRINGUP_STAGE_SOC_INT_SELFTEST,
    BRINGUP_STAGE_DONE
} bringup_stage_t;
```

- [ ] **Step 3: Store the probed address and SAL state inside `bringup_service.c`**

```c
static bringup_stage_t s_current_stage = BRINGUP_STAGE_BOOT;
static uint8_t s_detected_soc_address = 0x75u;
static bool s_detected_soc_address_valid = false;
static bool s_soc_sal_ready = false;
```

- [ ] **Step 4: Update the I2C probe handler to save the first detected SoC address**

```c
static void bringup_service_run_i2c_probe(void)
{
    char message[96];
    uint8_t first_found_address = 0u;
    int found_count;

    s_detected_soc_address_valid = false;

    if (board_has_uart_log())
    {
        board_uart_write_string("i2c init start\r\n");
    }

    if (!board_i2c_probe_init())
    {
        if (board_has_uart_log())
        {
            board_uart_write_string("i2c init fail\r\n");
        }

        return;
    }

    if (board_has_uart_log())
    {
        board_uart_write_string("i2c init ok\r\n");
    }

    found_count = board_i2c_probe_scan(&first_found_address);

    if (found_count > 0)
    {
        s_detected_soc_address = first_found_address;
        s_detected_soc_address_valid = true;
    }

    if (!board_has_uart_log())
    {
        return;
    }

    if (found_count > 0)
    {
        (void)snprintf(
            message,
            sizeof(message),
            "i2c found: 0x%02X count=%d\r\n",
            (unsigned int)first_found_address,
            found_count);
        board_uart_write_string(message);
        return;
    }

    if (found_count == 0)
    {
        board_uart_write_string("i2c scan done, no device found\r\n");
        return;
    }

    board_uart_write_string("i2c scan fail\r\n");
}
```

- [ ] **Step 5: Add a `SOC_SAL_INIT` handler and route the state machine through it**

```c
static void bringup_service_run_soc_sal_init(void)
{
    if (!s_detected_soc_address_valid)
    {
        s_soc_sal_ready = false;

        if (board_has_uart_log())
        {
            board_uart_write_string("bringup: soc sal init skipped (no i2c device)\r\n");
        }

        return;
    }

    if (board_has_uart_log())
    {
        char message[80];
        (void)snprintf(
            message,
            sizeof(message),
            "bringup: soc sal init start addr=0x%02X\r\n",
            (unsigned int)s_detected_soc_address);
        board_uart_write_string(message);
    }

    s_soc_sal_ready = soc_sal_init(s_detected_soc_address);
}

static bringup_stage_t bringup_service_stage_after_probes(void)
{
    return BRINGUP_STAGE_SOC_SAL_INIT;
}

static bringup_stage_t bringup_service_next_stage(bringup_stage_t stage)
{
    switch (stage)
    {
    case BRINGUP_STAGE_BOOT:
        return board_has_status_led() ? BRINGUP_STAGE_LED : BRINGUP_STAGE_UART;
    case BRINGUP_STAGE_LED:
        return BRINGUP_STAGE_UART;
    case BRINGUP_STAGE_UART:
        if (bringup_service_adc_enabled())
        {
            return BRINGUP_STAGE_ADC;
        }

        return bringup_service_i2c_enabled() ? BRINGUP_STAGE_I2C
                                             : bringup_service_stage_after_probes();
    case BRINGUP_STAGE_ADC:
        return bringup_service_i2c_enabled() ? BRINGUP_STAGE_I2C
                                             : bringup_service_stage_after_probes();
    case BRINGUP_STAGE_I2C:
        return bringup_service_stage_after_probes();
    case BRINGUP_STAGE_SOC_SAL_INIT:
        return bringup_service_soc_int_selftest_enabled() ? BRINGUP_STAGE_SOC_INT_SELFTEST
                                                          : BRINGUP_STAGE_DONE;
    case BRINGUP_STAGE_SOC_INT_SELFTEST:
        return BRINGUP_STAGE_DONE;
    case BRINGUP_STAGE_DONE:
    default:
        return BRINGUP_STAGE_DONE;
    }
}

void bringup_service_process(void)
{
    if (s_current_stage == BRINGUP_STAGE_DONE)
    {
        return;
    }

    if (s_current_stage == BRINGUP_STAGE_ADC)
    {
        bringup_service_run_adc_probe();
    }
    else if (s_current_stage == BRINGUP_STAGE_I2C)
    {
        bringup_service_run_i2c_probe();
    }
    else if (s_current_stage == BRINGUP_STAGE_SOC_SAL_INIT)
    {
        bringup_service_run_soc_sal_init();
    }
    else if (s_current_stage == BRINGUP_STAGE_SOC_INT_SELFTEST)
    {
        bringup_service_run_soc_int_selftest();
    }

    s_current_stage = bringup_service_next_stage(s_current_stage);
}
```

- [ ] **Step 6: Build and verify the new stage compiles cleanly**

Run: `cd openBmsClaw && cmake --preset Debug && cmake --build --preset Debug`
Expected: exit code `0`, no new warnings under `-Wall -Wextra -Wpedantic`.

- [ ] **Step 7: Commit**

```bash
git add openBmsClaw/services/bringup/bringup_service.h openBmsClaw/services/bringup/bringup_service.c
git commit -m "feat: wire soc sal init into bringup stages"
```

### Task 2: Add SAL Readiness and EXTI Init Observability

**Files:**
- Modify: `openBmsClaw/drivers/soc/soc_api.h:17-79`
- Modify: `openBmsClaw/drivers/soc/soc_manager.c:19-183`
- Test: `rg -n "soc_sal_is_initialized|SAL: init start|SAL: EXTI init" openBmsClaw`
- Test: `cd openBmsClaw && cmake --preset Debug && cmake --build --preset Debug`

- [ ] **Step 1: Prove the observability gap**

Run: `rg -n "soc_sal_is_initialized|SAL: init start|SAL: EXTI init ok|SAL: EXTI init fail" openBmsClaw`
Expected: no hits; SAL has no public readiness query and no explicit EXTI init log.

- [ ] **Step 2: Add a public readiness query to the SAL API**

```c
/**
 * @brief 查询 SAL 当前是否已经完成运行时初始化
 * @return true 表示 I2C / vendor init / EXTI init 已完成
 */
bool soc_sal_is_initialized(void);
```

- [ ] **Step 3: Add init start / fail / EXTI logs inside `soc_manager.c`**

```c
bool soc_sal_init(uint8_t i2c_addr) {
  bool exti_ready = true;

  s_soc_i2c_addr = i2c_addr;
  s_recovery_count = 0u;
  s_ops = demo_soc_get_ops();

  if (board_has_uart_log()) {
    char msg[64];
    (void)snprintf(msg, sizeof(msg), "SAL: init start addr=0x%02X\r\n",
                   (unsigned int)s_soc_i2c_addr);
    board_uart_write_string(msg);
  }

  if (!hal_i2c_init()) {
    if (board_has_uart_log()) {
      board_uart_write_string("SAL: I2C hardware init failed!\r\n");
    }
    s_sal_initialized = false;
    return false;
  }

  if (s_ops->init(s_soc_i2c_addr) != SOC_SAL_OK) {
    if (board_has_uart_log()) {
      board_uart_write_string("SAL: vendor init failed!\r\n");
    }
    s_sal_initialized = false;
    return false;
  }

#if CONFIG_ENABLE_SOC_INT_HIGHWAY
  exti_ready = hal_exti_init(HAL_EXTI_PORT_A, CONFIG_SOC_INT_PIN_INDEX,
                             HAL_EXTI_TRIGGER_FALLING, soc_sal_exti_hardware_handler);
  if (board_has_uart_log()) {
    board_uart_write_string(exti_ready ? "SAL: EXTI init ok\r\n"
                                       : "SAL: EXTI init fail\r\n");
  }
  if (!exti_ready) {
    s_sal_initialized = false;
    return false;
  }
#endif

  s_sal_initialized = true;
  return true;
}
```

- [ ] **Step 4: Implement the readiness query**

```c
bool soc_sal_is_initialized(void) { return s_sal_initialized; }
```

- [ ] **Step 5: Rebuild after the API addition**

Run: `cd openBmsClaw && cmake --preset Debug && cmake --build --preset Debug`
Expected: exit code `0`, no warnings, `soc_api.h` public surface compiles cleanly.

- [ ] **Step 6: Commit**

```bash
git add openBmsClaw/drivers/soc/soc_api.h openBmsClaw/drivers/soc/soc_manager.c
git commit -m "feat: add soc sal readiness and init logs"
```

### Task 3: Close the Software Selftest Loop and Prove the Layer Boundary

**Files:**
- Modify: `openBmsClaw/services/bringup/bringup_service.c:1-184`
- Test: `rg -n "selftest PASS|selftest FAIL|soc poll online" openBmsClaw/services/bringup`
- Test: `cd openBmsClaw && cmake --preset Debug && cmake --build --preset Debug`
- Test: `cd openBmsClaw/build/Debug && ninja -t deps CMakeFiles/openBmsClaw.dir/services/bringup/bringup_service.c.obj`

- [ ] **Step 1: Prove the selftest result is not observable yet**

Run: `rg -n "selftest PASS|selftest FAIL|soc poll online|soc poll offline" openBmsClaw/services/bringup`
Expected: no hits; bring-up only emits the trigger log and cannot tell whether the callback chain actually fired.

- [ ] **Step 2: Import `power_service` state and SAL readiness into bring-up**

```c
#include "services/power/power_service.h"

static void bringup_service_run_soc_sal_init(void)
{
    if (!s_detected_soc_address_valid)
    {
        s_soc_sal_ready = false;

        if (board_has_uart_log())
        {
            board_uart_write_string("bringup: soc sal init skipped (no i2c device)\r\n");
        }

        return;
    }

    s_soc_sal_ready = soc_sal_init(s_detected_soc_address);

    if (board_has_uart_log())
    {
        board_uart_write_string(s_soc_sal_ready ? "bringup: soc sal init ok\r\n"
                                                : "bringup: soc sal init fail\r\n");
    }

    if (s_soc_sal_ready)
    {
        uint32_t event_mask = 0u;
        soc_sal_status_t status = soc_poll_events(&event_mask);

        if (board_has_uart_log())
        {
            if (status == SOC_SAL_OK)
            {
                char message[80];
                (void)snprintf(
                    message,
                    sizeof(message),
                    "bringup: soc poll online mask=0x%08X\r\n",
                    (unsigned int)event_mask);
                board_uart_write_string(message);
            }
            else
            {
                board_uart_write_string("bringup: soc poll offline\r\n");
            }
        }
    }
}
```

- [ ] **Step 3: Turn the selftest into a pass/fail check**

```c
static void bringup_service_run_soc_int_selftest(void)
{
    if (!soc_sal_is_initialized())
    {
        if (board_has_uart_log())
        {
            board_uart_write_string("bringup: soc int selftest skipped (sal offline)\r\n");
        }
        return;
    }

    if (board_has_uart_log())
    {
        board_uart_write_string("bringup: soc int selftest (sw EXTI trigger)\r\n");
    }

    soc_sal_int_selftest_trigger();

    if (board_has_uart_log())
    {
        board_uart_write_string(
            power_service_get_state() == POWER_STATE_EMERGENCY_LOCK
                ? "bringup: soc int selftest PASS\r\n"
                : "bringup: soc int selftest FAIL\r\n");
    }
}
```

- [ ] **Step 4: Rebuild and then inspect the dependency boundary**

Run: `cd openBmsClaw && cmake --preset Debug && cmake --build --preset Debug`
Expected: exit code `0`, no warnings.

Run: `cd openBmsClaw/build/Debug && ninja -t deps CMakeFiles/openBmsClaw.dir/services/bringup/bringup_service.c.obj | rg "hal/i2c/hal_i2c.h|hal/exti/hal_exti.h|drivers/soc/vendor/demo_soc.h"`
Expected: no output; `bringup_service.c` still depends only on service/SAL public headers, not HAL or vendor-private headers.

- [ ] **Step 5: Commit**

```bash
git add openBmsClaw/services/bringup/bringup_service.c
git commit -m "feat: add stage a selftest and online poll logs"
```

### Task 4: Create the Stage A Bench Validation Record and Capture First Hardware Evidence

**Files:**
- Create: `2_Action/Action-StageA-SoC-Validation.md`
- Test: `rg -n "Stage A SoC 实板联调记录|ChipID|Logic Analyzer" 2_Action`
- Test: `cd openBmsClaw && cmake --build --preset Debug`

- [ ] **Step 1: Prove there is no dedicated Stage A hardware record yet**

Run: `rg -n "Stage A SoC 实板联调记录|ChipID|Logic Analyzer" 2_Action`
Expected: no hit for a dedicated Stage A validation note.

- [ ] **Step 2: Create the validation note template**

~~~markdown
# Stage A SoC 实板联调记录

## 1. 构建与烧录基线
- 固件路径：
- 构建命令：
- 构建结果：
- 烧录板卡：

## 2. 硬件连接
- MCU 板：
- SoC 评估板：
- I2C 连接：
- INT 连接：
- 供电方式：

## 3. UART 运行日志
```text

```

## 4. 真实 SoC 读值
- I2C 地址：
- ChipID：
- Voltage：
- Temperature：
- Event Mask：

## 5. Logic Analyzer
- 通道定义：
- 触发条件：
- 截图文件：
- 观察到的 ACK/NACK：

## 6. 异常与恢复
- 现象：
- 原因判断：
- 是否触发 9-pulse recovery：
- 修复动作：

## 7. 结论
- Stage A 当前状态：
- 下一步动作：
~~~

- [ ] **Step 3: Build the firmware that will be flashed during the bench session**

Run: `cd openBmsClaw && cmake --build --preset Debug`
Expected: `openBmsClaw/build/Debug/openBmsClaw.elf`, `.hex`, `.bin` are regenerated successfully.

- [ ] **Step 4: Flash and capture the first real-board evidence**

Action:
- 烧录 `openBmsClaw/build/Debug/openBmsClaw.bin` 到当前 F103 学习板。
- 连接逻辑分析仪：
  - `CH0 -> PB6 / SCL`
  - `CH1 -> PB7 / SDA`
  - `GND -> GND`
- 上电后记录 UART 日志，至少捕获以下之一：
  - `SAL: SoC(demo) init OK, ChipID=...`
  - `bringup: soc poll online mask=...`
  - `bringup: soc int selftest PASS`
- 把实际地址、寄存器、ACK/NACK 现象填入 `2_Action/Action-StageA-SoC-Validation.md`。

Expected:
- 至少拿到一个真实值：`ChipID` / `Voltage` / `Temperature`
- 至少保存一份逻辑分析仪截图或导出文件路径

- [ ] **Step 5: Commit**

```bash
git add 2_Action/Action-StageA-SoC-Validation.md
git commit -m "docs: record stage a soc bench validation"
```

## Self-Review

- Spec coverage:
  - `soc_sal_init()` 进入真实运行路径：Task 1
  - `hal_exti_init()` 完成运行时 SoC INT 初始化：Task 2
  - `SWIER -> EXTI -> callback -> power_service` 软件闭环：Task 3
  - 真实 SoC 与逻辑分析仪记录：Task 4
- Placeholder scan:
  - No `TBD` / `TODO` / “implement later” text remains.
  - Bench note template uses concrete section headings, not placeholder tags.
- Type consistency:
  - New enum stage uses `BRINGUP_STAGE_SOC_SAL_INIT` consistently.
  - Public readiness query is named `soc_sal_is_initialized()` consistently in API and caller.
