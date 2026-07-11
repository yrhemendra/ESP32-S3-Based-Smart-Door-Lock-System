# 🔐 ESP32-S3 Based Smart Door Lock System

> A secure PIN-based electronic door lock system built using the ESP32-S3 microcontroller, a 4×4 matrix keypad, relay module, and 12V solenoid lock.

![Arduino](https://img.shields.io/badge/Arduino-Framework-blue)
![ESP32](https://img.shields.io/badge/ESP32-S3-green)
![Embedded](https://img.shields.io/badge/Embedded-C++-orange)
![License](https://img.shields.io/badge/License-MIT-red)

---

## 📌 Overview

This project implements a secure embedded access control system using an ESP32-S3 microcontroller. Users authenticate themselves through a 4×4 matrix keypad, and upon successful PIN verification, the ESP32 activates a relay module to unlock a 12V DC solenoid door lock.

The firmware is designed using a **non-blocking state machine**, enabling responsive keypad input while handling automatic relocking without using blocking delays.

This project demonstrates practical concepts in:

- Embedded Systems
- Firmware Development
- Cyber-Physical Security
- GPIO Programming
- Relay Control
- Electrical Safety
- Secure Access Control

---

## ✨ Features

- PIN-based user authentication
- Non-blocking firmware architecture
- Automatic door relocking
- Matrix keypad scanning
- Relay-controlled solenoid lock
- Electrical isolation using relay module
- Flyback diode protection
- UART serial logging
- Modular firmware structure
- Extendable design for IoT applications

---

# Hardware Used

| Component | Description |
|-----------|-------------|
| ESP32-S3 | Main Controller |
| 4×4 Matrix Keypad | PIN Input |
| Relay Module | Solenoid Driver |
| 12V Solenoid Lock | Door Lock |
| 1N4007 Diode | Flyback Protection |
| 12V Adapter | Solenoid Supply |

---

# System Architecture

```
        User
          │
          ▼
+--------------------+
| 4×4 Matrix Keypad  |
+--------------------+
          │
          ▼
+--------------------+
|     ESP32-S3       |
| PIN Verification   |
| State Machine      |
+--------------------+
          │
          ▼
+--------------------+
| Relay Module       |
+--------------------+
          │
          ▼
+--------------------+
| 12V Solenoid Lock  |
+--------------------+
```

---

# Firmware Workflow

```
Power ON

↓

Enter PIN

↓

Verify PIN

↓

Correct ?

YES → Unlock Door → Auto Relock after Timeout

NO → Access Denied
```

---

# Firmware Design

The firmware follows a **millis()-based non-blocking architecture**.

Main modules:

- Keypad Driver
- Input Handler
- Lock Controller
- State Machine
- UART Logger

This design ensures the ESP32 remains responsive while continuously monitoring keypad input and managing the relay without blocking execution.

---

# Security Features

- PIN Authentication
- Input Buffer Protection
- PIN Masking on UART
- Auto Relock
- Relay Isolation
- Flyback Diode Protection

Future security improvements:

- Brute-force Lockout
- AES Flash Encryption
- Multi-user PINs
- Audit Logging
- Wi-Fi Authentication

---

# GPIO Mapping

| GPIO | Function |
|------|----------|
| 13 | Row 1 |
| 12 | Row 2 |
| 11 | Row 3 |
| 10 | Row 4 |
| 9 | Column 1 |
| 8 | Column 2 |
| 7 | Column 3 |
| 6 | Column 4 |
| 4 | Relay |

---

# Project Structure

```
ESP32-Smart-Door-Lock/

│
├── code/
│   └── DoorLock.ino
│
├── circuit/
│   ├── wiring.png
│   ├── schematic.png
│
├── images/
│   ├── setup.jpg
│   ├── keypad.jpg
│   ├── relay.jpg
│
├── report/
│   └── Project_Report.pdf
│
├── README.md
└── LICENSE
```

---

# Results

✅ PIN Authentication

✅ Automatic Unlock

✅ Automatic Relock

✅ Stable Relay Operation

✅ Safe Solenoid Switching

✅ Reliable Keypad Detection

---

# Future Enhancements

- Fingerprint Authentication
- RFID Card Support
- BLE Mobile Unlock
- Wi-Fi Remote Access
- MQTT Integration
- Home Assistant Integration
- OTA Firmware Updates
- Access Logs
- Cloud Dashboard

---

# Skills Demonstrated

- Embedded C++
- ESP32 Programming
- Firmware Development
- State Machine Design
- GPIO Programming
- Relay Interfacing
- Matrix Keypad Scanning
- Cyber-Physical Security
- Hardware Debugging
- Electrical Safety

---

# Author

**Hemendra Solanki**

M.Tech Computer Science & Engineering

National Institute of Technology Patna

GitHub: https://github.com/yrhemendra

LinkedIn: https://linkedin.com/in/hemendra-solanki

---

# License

This project is released under the MIT License.
