# SwipeLock 🔐  
**Embedded Smart Locker System**

SwipeLock is a fully integrated embedded locker system that authenticates users via magnetic card swipe, provides an interactive touchscreen UI, and controls a physical solenoid-based locking mechanism. The system was designed, built, and demonstrated as a hardware–software co-design project.

---
## Background

Shared locker systems in university gyms suffer from a combination of usability, reliability, and state-management failures. 
Traditional locker designs rely on users remembering numeric identifiers or PINs, while offering no system-level awareness of locker ownership, abandonment, or misuse.
In practice, this leads to several recurring issues:

1. Users must manually locate and remember a locker number among many visually identical lockers
2. Forgotten PINs or keys render lockers inaccessible without staff intervention
3. Lockers are frequently left in an “occupied” state despite no active user, reducing availability and creating ambiguity
4. Staff have no reliable, real-time mechanism to determine locker state or safely reclaim abandoned lockers until end-of-day cleaning

These problems stem from the absence of identity-bound access control and deterministic system state in conventional locker infrastructure.

SwipeLock is a solution to these problems, acting as a simple yet complex interface 
that handles both the usability and functionality of gym lockers. 

## Overview

At a high level, SwipeLock functions as follows:

1. A user swipes a magnetic card
2. The system validates the swipe and current locker state
3. The touchscreen UI updates accordingly
4. The solenoid lock is actuated when appropriate
5. Audio feedback confirms user actions

The system is designed to handle invalid input, conflicting actions, and hardware constraints deterministically.

---

## System Architecture

SwipeLock is composed of several interacting subsystems:
  
### Hardware Components
- STM32 microcontroller (bare-metal + HAL)
- Magnetic stripe card reader (MCard)
- ILI9488 TFT display (480×320)
- XPT2046 resistive touchscreen
- Electromagnetic solenoid lock
- Piezo buzzer

### Firmware Subsystems
- **Authentication pipeline** (card reader → parsing → validation)
- **Graphical UI and touch input** (LVGL-based)
- **System state machine** (vacant, occupied, abandoned, admin)
- **Lock control and safety logic**
- **Audio feedback**
- **Physical enclosure and wiring**

---

## 🖥️ Graphical UI & Touch Interface

The graphical interface is implemented using **LVGL** and rendered on an ILI9488 TFT display.

Key technical aspects:
- SPI-driven display with **DMA acceleration**
- Custom LVGL flush callback with **RGB666 → RGB565 conversion**
- **Double-buffered DMA** to improve refresh performance (~1.5–2× speedup)
- Touch input via XPT2046 on a separate SPI bus
- Touch calibration and ADC-to-screen coordinate mapping
- Fully event-driven UI tied into the system state machine

The UI provides clear feedback for:
- Locker availability
- Successful and failed card swipes
- Administrative actions (Admin Control Panel)
- Error states

---

## 💳 Magnetic Card Authentication

The magnetic card reader subsystem:
- Captures raw swipe data from the card reader
- Parses track data into structured information
- Validates swipes against system state
- Handles invalid cards and conflicting swipe events gracefully
  
---

## 🔒 Lock Control & Safety

The locking mechanism uses an electromagnetic solenoid with a mechanical wedge.

Design considerations include:
- Reliable actuation under load
- Preventing overheating during repeated use
- Synchronization with system state transitions

Safety logic enforces **time-limited activation** and integrates lock control tightly with authentication and UI feedback.

---

## 🔄 State Machine & System Logic

The system is driven by a deterministic state machine that coordinates all subsystems.

Primary states include:
- Vacant
- Occupied
- Abandoned
- Administrative modes

The state machine governs:
- Valid and invalid user actions
- UI transitions
- Lock actuation
- Audio feedback
- Error recovery

This design ensures predictable behavior even under conflicting or unexpected input.

---

## 🔊 Audio Feedback

A piezo buzzer provides audible feedback for:
- Button interactions
- Successful actions
- Error conditions

Audio cues are synchronized with UI and system events.

---

## 🧱 Physical Design & Assembly

SwipeLock is housed in a custom-built locker enclosure.  
The physical design accommodates:
- Secure mounting of electronics
- Solenoid placement and alignment
- Cable routing and serviceability

The final system was fully assembled and demonstrated as a working prototype.

---

## Contributors

This project was developed by **two primary contributors**:

- **Gjonpjer (gjonpjer@umich.edu)** — display subsystem, UI/UX, touch integration, solenoid lock system, card reader debugging, system architecture, physical design
- **Thomas Rollmann (tver@umich.edu)** — card reader protocol implementation, sound system, system integration support
