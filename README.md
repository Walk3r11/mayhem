# MAYHEM
<p align="center">
  <b>DIY Multi-Microcontroller PC Prototype — A Custom “CPU” System with Thermal & I/O Control</b>
</p>

---

## Table of Contents
- [Project Concept](#project-concept)
- [Hardware Breakdown](#hardware-breakdown)
- [Key Features](#key-features)
- [Why This Project?](#why-this-project)
- [Potential Use Cases](#potential-use-cases)
- [Project Status](#project-status)

---

## Project Concept

Inspired by classic PC architecture, this project pieces together several microcontrollers into a functional system that demonstrates:

- **CPU-like control and coordination** across multiple microcontrollers
- **Dynamic thermal management** using onboard temperature sensors (Raspberry Pi Pico’s internal sensor) to control cooling fans
- **LED arrays** as system status indicators, boot-up animations, and diagnostics (ESP32 and Arduino controlling multiple LEDs with flicker and sequence effects)
- **Parallel and serial shift registers** (SN74HC165 & SN74HC595) for efficient expansion of digital inputs/outputs, emulating real CPU buses and registers
- **Mass storage integration** with a microSD card module for data logging or system state saving
- **Visual feedback** through a 2.8” SPI TFT display showing system status and debug info
- **User inputs** via DIP switches and buttons for configuration and control signals
- **Modular power management** supporting USB and battery sources

---

## Hardware Breakdown

| Component                | Purpose/Role                                                                 |
|--------------------------|------------------------------------------------------------------------------|
| **ESP32 Boards (x2)**    | Main “processing units” running complex LED control and managing communication |
| **Raspberry Pi Pico**    | “Thermal control unit,” reading temperature sensors and triggering fans        |
| **Arduino Nano/Uno**     | Handles auxiliary LED arrays and additional I/O expansion                      |
| **SN74HC165/595**        | Shift registers for scalable I/O, mimicking CPU buses                          |
| **MicroSD Card Module**  | Persistent storage for logs or data                                            |
| **5x 5V Fans**           | Controlled based on temperature thresholds for dynamic cooling                 |
| **2.8” SPI TFT Display** | Live visual feedback on system operation                                       |
| **DIP Switch & Buttons** | Manual user input, configuration, and debugging                                |

---

## Key Features

- 🚦 **Boot-up LED animations** that light LEDs sequentially to mimic system initialization
- 💡 **Continuous LED status display** for live monitoring
- 🌡️ **Fan speed control** based on real-time temperature, automatically cooling hardware under load
- 🔗 **Multi-core communication** between microcontrollers simulating CPU and peripheral interactions
- 🔋 **Flexible power input options** (USB or battery)
- 🛠️ **Comprehensive debugging output** and troubleshooting support via serial console
- ⬆️ **Expandable I/O** using shift registers, allowing future additions without major rewiring

---

## Why This Project?

This is not just a microcontroller project but an exploration of computer architecture fundamentals implemented in hardware:

- Understanding how CPUs and peripherals communicate via buses
- Practical experience with thermal management in embedded systems
- Integrating multiple microcontrollers as “cores” for distributed control
- Learning efficient use of limited GPIO pins via shift registers
- Building a visually interactive embedded system with LEDs and displays
- Developing firmware that coordinates complex hardware interactions

---

## Potential Use Cases

- 🏫 **Educational tool** for computer architecture and embedded systems
- 🧪 **Prototype platform** for testing multi-core communication
- 🖥️ **Custom DIY PC-like controller** for small devices
- 💡 **Advanced LED and fan control system** for electronics projects
- 🤖 **Base for building more complex IoT or robotics systems**

---

## Project Status

- ✅ Hardware wired and tested across multiple breadboards
- ✅ Core LED and fan control firmware running on ESP32, Pico, and Arduino
- 🔄 Ongoing work on enhanced communication protocols and storage integration
- 📝 Documentation and wiring guides in progress

---
