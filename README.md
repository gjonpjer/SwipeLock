# SwipeLock 🔐  
**Embedded Smart Locker System (EECS 373 Final Project)**

SwipeLock is a fully integrated embedded locker system that authenticates users via magnetic card swipe, provides an interactive touchscreen UI, and controls a physical solenoid-based locking mechanism. The system was designed, built, and demonstrated as a hardware–software co-design project.

---

## 📌 Overview

SwipeLock combines:
- **Embedded firmware (STM32)**
- **Real-time hardware interfaces**
- **Custom graphics/UI**
- **Physical system integration**

The final system supports secure locker usage with clear user feedback, administrative controls, and robust handling of erroneous input.

---

## 🧠 System Architecture

**Core components:**
- STM32 microcontroller (bare-metal + HAL)
- Magnetic stripe card reader (MCard)
- ILI9488 TFT display (480×320)
- XPT2046 resistive touchscreen
- Electromagnetic solenoid lock
- Piezo buzzer for audio feedback

**Key subsystems:**
- Card authentication pipeline
- UI rendering & touch input
- State machine (vacant / occupied / abandoned / admin)
- Lock control & safety logic
- Physical enclosure & wiring

---

## 👥 Contributors

This project was developed by **two primary contributors**:

- **Gjonpjer (gjonpjer@umich.edu)** — display subsystem, UI/UX, touch integration, solenoid lock system, card reader debugging, system architecture, physical design
- **Thomas Rollmann (tver@umich.edu)** — card reader protocol implementation, sound system, system integration support

---

## 🚀 Takeaways

SwipeLock was a deeply hands-on embedded systems project involving:
- Hardware debugging at the signal level
- Performance optimization using DMA
- Full-stack embedded UI development
- Real-world system integration and failure recovery
