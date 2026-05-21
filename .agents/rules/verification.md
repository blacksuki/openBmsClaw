# verification.md - openBattery Project Verification Phase Rules

This file provides guidance to Antigravity during the **Testing & Verification** phase. Use these rules when validating code builds, running tests, or performing manual checkout.

---

## 1. Key Verification Categories
All updates to `openBattery` must undergo targeted verification focusing on key areas of the power management and BMS platform:
- **Digital HMI & Display**: Verify OLED/LED display telemetry (remaining run-time prediction for charge/discharge, SOH state-of-health estimation).
- **User Interaction**: Test button state machines (short-press to wake/show status, long-press for hard shutdown or mode switching).
- **Thermal Management**: Verify real-time ADC sampling of NTC temperature sensors, validation of temperature protection limits, and dynamic scaling of power outputs based on thermal state.
- **BMS Safety & Cell Balancing**: Test cell voltage/current telemetry, cell-balancing activation thresholds, and dynamic OCP (Overcurrent Protection) configs.
- **Dedicated SoC & Power Delivery**: Validate fast charge protocol handshakes (event callbacks) and intelligent power splitting algorithms when multiple USB ports are active.
- **Compliance & Product Safety**: Check safety margins for Japanese PSE standards, airline portable battery limits (≤100Wh threshold enforcement), Qi2 magnetic alignment, and dock/cradle charging limits.

---

## 2. Compilation and Build Instructions
- Run and check local builds using CMake/Ninja configured in the `openBmsClaw/` directory:
  - Ensure the build artifacts compile without errors or warnings.
  - Verify linking scripts (.ld) and startup files are correctly aligned to the target MCU (`STM32F103C8T6` or `STM32G0` depending on the build configuration).

---

## 3. Strict Verification & Documentation Rules
- **No Hallucinated Success**: Be transparent about verification bounds. If physical hardware is missing, state clearly that:
  - The code compiles successfully.
  - The logic was verified through simulator testing or unit tests.
  - Physical board-level integration remains unverified.
- **Walkthrough Generation**: Keep a detailed `walkthrough.md` record:
  - Outline exactly what files changed and the purpose of each change.
  - List the specific verification tests executed.
  - Include compilation output, terminal logs, or register capture data.
