# coding.md - openBattery Project Coding Phase Rules

This file provides coding standards and rules for Antigravity during the **Implementation** phase. Use these rules when writing code and updating `task.md`.

---

## 1. Directory & Code Organization
- **Development Target**: All current STM32 work must target `openBmsClaw/`.
- **Target Folder Structure**:
  - Store application/HMI logic in `openBmsClaw/Src/app/` and `openBmsClaw/Inc/app/`.
  - Store management services (power allocation, temperature protection) in `openBmsClaw/Src/services/` and `openBmsClaw/Inc/services/`.
  - Store peripheral driver wrappers in `openBmsClaw/Src/hal/` and `openBmsClaw/Inc/hal/`.
  - Store external SoC drivers in `openBmsClaw/Src/drivers/` and `openBmsClaw/Inc/drivers/`.
  - Store board-specific configurations/pin setups in `openBmsClaw/Src/board/` and `openBmsClaw/Inc/board/`.
- **Generated Folders**: `build/` contains compiled output. Do NOT manually edit files under the `build/` directory.

---

## 2. Embedded C Coding Standards
- **Code Simplicity**: C code should be simple, clear, and focused. Avoid overly complex dynamic memory allocations or abstract design patterns during early bring-up.
- **HAL Usage**: Use STM32 HAL libraries properly. Avoid bypassing HAL to directly write to registers unless specifically optimized for timing-critical tasks.
- **Safety Interlocks**: Any code handling power paths, charge/discharge switches, or heating must include hardware and software safety interlocks:
  - Default to a safe state (e.g., charge/discharge FETs OFF).
  - Explicitly check state boundaries and guard values before opening FETs.
- **Documentation**: Maintain code comments and do not strip out existing docstrings or copyright headers. Write clear, Doxygen-style headers for public API files.

---

## 3. Dedicated SoC Standard Interfaces
When writing driver code for external power control/protocol SoCs:
- Always define a standard driver structure:
  ```c
  typedef struct {
      HAL_StatusTypeDef (*init)(void);
      HAL_StatusTypeDef (*read_telemetry)(BatteryTelemetry_t *telemetry);
      HAL_StatusTypeDef (*set_limit)(PowerLimit_t *limits);
      void (*register_callback)(EventCallback_t cb);
  } SoC_DriverTypeDef;
  ```
- Reject heavy vendor SDKs; focus on lightweight, register-mapped standard API definitions.

---

## 4. Honest Progress and Task Management
- Update `task.md` continuously. Use standard markers: `[ ]` for pending, `[/]` for in-progress, and `[x]` for fully complete.
- **Zero Hallucination**: Never claim a feature is complete, compiled, or tested unless you have successfully compiled it and validated it against physical or simulated tests.
