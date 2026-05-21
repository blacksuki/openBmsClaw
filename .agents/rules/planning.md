# planning.md - openBattery Project Planning Phase Rules

This file provides guidance to Antigravity during the **Research & Planning** phase. Use these rules when drafting `implementation_plan.md` for any `openBattery` task.

---

## 1. Core Architectural Constraints
All implementation plans must align with the `openBattery` target platform architecture:
- **Double-Chip Architecture**: Dedicate specialized SoCs (e.g., Chipown/IP, Injoinic/SW) for power conversion (buck-boost) and fast charge handshake protocols ("physical labor"). Dedicate the STM32 MCU for intelligent strategies, power management, HMI/display, and safety protection ("brain labor").
- **Modular Software Design**: Do NOT pile complex logic into `main.c`. The system must be structured into the following layers:
  - `app/`: User interaction, OLED/LED remaining run-time and SOH (State of Health) prediction, button handling, and IAP (In-Application Programming) firmware updates.
  - `services/`: Smart power routing/allocation, dynamic current/voltage configuration, thermal monitoring and over-temperature protection.
  - `hal/`: STM32 peripheral driver wrappers (ADC, I2C, SPI, GPIO, UART).
  - `drivers/`: Dedicated SoC API/register wrappers.
  - `board/`: Specific board pin mappings and device initialization bindings.
  - `config/`: Configuration parameters (safety thresholds, system timings).

---

## 2. Dedicated SoC Integration Strategy
Follow the **"要接口不要源码" (Require Interfaces, Not Source Code)** standard:
- Do not request or attempt to reverse engineer/crack private vendor source code.
- Focus on standard API wrapper designs that expose three essential interface groups:
  1. **Data Reading**: Voltage, current, battery temperature, protocol status.
  2. **Parameter Configuration**: Dynamic OCP (Overcurrent Protection) thresholds, charging currents, target voltage limits.
  3. **Event Callbacks**: Interrupts, quick charge handshake notifications, fault alerts.

---

## 3. Hardware Roadmap Rules
Ensure proper distinction between hardware targets in plans:
- **Starter/Bring-up Board**: `野火小智 STM32F103C8T6 核心板 (双 USB 款)` with standard SWD debug interfaces.
- **Platform Reference Board**: `STM32G0` Nucleo boards (e.g., `NUCLEO-G071RB`, `NUCLEO-G0B1RE`).
- **Production MCU Targets**: STM32G0 series microcontrollers (e.g., `STM32G030C8T6`) for cost-effective mass production.

---

## 4. Isolation of Reference Materials
- `90.mini-Lite/` is strictly a reference case for learning behaviors (e.g., multi-port power limits, button state machines, NTC protection curves).
- NEVER adopt its MCU choice, direct directory structure, or specific implementation code directly into the active `openBattery` system.

---

## 5. Planning Phase Outputs
When formulating a plan:
- Highlight critical design decisions, safety thresholds, and register mapping strategies.
- Formulate a clear verification plan listing automated tests (if any) and physical/simulated hardware validation scenarios.
