# research.md - openBattery Project Research Phase Rules

This file provides guidance to Antigravity during the **Analysis & Exploration** phase. Use these rules when analyzing the codebase, answering hardware/software questions, or deep-diving into system components.

---

## 1. Primary Sources of Truth
When researching design specifications, current state, or upcoming steps:
- **Architecture & System Design**: Refer to `0_System/03.tech_architecture.md` (Strategy/Tech), `0_System/01.project_kickoff_proposal.md` (Project kickoff proposal / origin), and `0_System/10.hardware_softare_frame.md` (Core Software/Hardware Architecture).
- **MCU Roadmaps & Hardware**: Refer to `0_System/11.prepare_dev.md` through `0_System/16.mcu_hardware_selection.md`.
- **Plans & Tasks**: Check `1_Plan/Plan-Week-0325.md` (and other weekly/phase plan files) as well as the actions recorded under `2_Action/`.
- **Active Code Fact**: Always treat `openBmsClaw/` as the single source of truth for active code implementation.

---

## 2. Reference Materials
- The former reference project `90.mini-Lite/` has been retired and is no longer maintained in this repo.
- Treat `90_documents/` and `91.reference/` as external reference material only; never present them as the active `openBattery` implementation. The current architecture is defined by `0_System/*.md` and `openBmsClaw/`.

---

## 3. Beginner-Friendly Technical Explanations
For developers learning or working on this repository, all technical explanations should be highly accessible:
- **Jargon Explanations**: Explain standard embedded terms on first use (e.g., *SWD* for debugging, *ST-LINK* as debug probe, *I2C* for SoC communication, *ADC* for sensor readings, *DRP* for USB Type-C Dual Role Power, *SOH* for battery health, *OCP* for overcurrent protection, *PSE* for Japanese product safety compliance).
- **Structured Debugging**: When debugging or reviewing code errors/logs, format responses under:
  1. **Phenomenon** (What is happening)
  2. **Reason** (Why it is happening)
  3. **Suggestion** (How to fix/improve it)

---

## 4. Hardware Path Classification
In any study or research report regarding MCU selection, clearly classify the platform targets:
1. **Starter Phase**: `野火小智 STM32F103C8T6` (Dual USB learning board)
2. **Platform Development Phase**: `STM32G0` Nucleo Boards (e.g., `NUCLEO-G071RB`, `NUCLEO-G0B1RE`)
3. **Enterprise Mass Production**: Target cost-effective STM32G0 microcontrollers (e.g., `STM32G030C8T6`), never Nucleo evaluation boards themselves.
