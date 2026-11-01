# Mini DOOM Game Console 🕹️💀

A portable, custom-designed handheld gaming console built to run a port of **DOOM**, powered by an **ESP32-S3** microcontroller and an 8-button custom mechanical keypad.

---

## 🚀 Project Overview

This repository contains the hardware design files, schematics, and Bill of Materials (BOM) for a custom-fabricated circuit board designed for a mini retro gaming console. 

> **Note on Firmware:** The firmware for this project (running the DOOM port and handling input/display rendering) is **not** hosted or developed in this specific repository. This repo focuses strictly on the hardware engineering, PCB layout, and physical components.

---

## 🛠️ Hardware Specifications

* **Microcontroller:** ESP32-S3-WROOM-2 (Dual-core Xtensa 32-bit LX7, Wi-Fi & BLE)
* **Input Array:** 8× Mechanical Push Switches / Tactile Buttons
* **Power Management:** USB-C powered via a 16-pin receptacle, stepped down via an LD1117S33TR 3.3V LDO regulator with proper 5.1kΩ CC pull-down resistors.
* **Display Interface:** High-resolution display module support.
* **Board Fabrication:** Custom 2-layer PCB designed in KiCad, fabricated via PCBWay.

---
<img width="1064" height="905" alt="Screenshot 2026-11-01 014820" src="https://github.com/user-attachments/assets/20873ace-6a0b-4b9c-92f5-ef2c84dd0879" />
<img width="733" height="521" alt="image" src="https://github.com/user-attachments/assets/9b8ab677-f3a1-48a3-af4c-a3067c790061" />


## 📂 Repository Structure

```text
├── Pdb design/              # KiCad project files, schematics, and PCB layouts
│   └── game boy fake/       # Main hardware project files & 3D models (.step)
├── Images and videos/       # Prototype photos, build logs, and testing clips
├── BOM.csv                  # Complete Bill of Materials with supplier links and costs
└── README.md                # Project documentation



