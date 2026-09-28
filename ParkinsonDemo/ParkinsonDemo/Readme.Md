# Parkinson's Disease Symptom Detector

A wearable device for detecting Parkinson's disease symptoms using the B-L475E-IOT01A development board.

## Overview

This project uses accelerometer data and FFT analysis to detect three key Parkinson's disease symptoms:

- **Tremor** (3-5 Hz): Rhythmic oscillations characteristic of the "Off" state
- **Dyskinesia** (5-7 Hz): Dance-like movements indicating excessive dopamine
- **Freezing of Gait (FOG)**: Sudden freezing after a period of walking

Detection results are transmitted via BLE to a mobile device for real-time monitoring.

## Hardware

- **Board**: ST B-L475E-IOT01A (IoT Discovery Kit)
- **Sensor**: LSM6DSL (onboard accelerometer/gyroscope)
- **Wireless**: SPBTLE-RF (onboard BLE module)
- **Power**: USB or portable power bank

## Features

| Feature | Description |
|---------|-------------|
| Real-time Detection | 3-second sliding window analysis at 52Hz sampling rate |
| FFT Analysis | Discrete Fourier Transform for frequency band energy calculation |
| BLE Transmission | Three characteristics for each symptom type |
| LED Indicators | Visual feedback for detected symptoms |
| Button Control | Pause/resume detection with USER button |

## Project Structure

```
parkinson_detector/
├── include/
│   ├── config.h          # Configuration parameters and thresholds
│   ├── sensor.h          # LSM6DSL sensor interface
│   ├── dft.h             # DFT analysis interface
│   ├── detection.h       # Detection algorithm interface
│   └── ble_service.h     # BLE service interface
├── src/
│   ├── main.cpp          # Main program and control loop
│   ├── sensor.cpp        # LSM6DSL driver implementation
│   ├── dft.cpp           # DFT spectral analysis implementation
│   ├── detection.cpp     # Symptom detection algorithms
│   └── ble_service.cpp   # BLE GATT service implementation
├── platformio.ini        # PlatformIO configuration
└── README.md             # This file
```

## Module Description

### config.h
Global configuration parameters including:
- Sampling rate (52 Hz)
- Window size (3 seconds, 156 samples)
- Frequency ranges for each symptom
- Detection thresholds

### sensor.cpp
LSM6DSL accelerometer driver:
- I2C communication at 400kHz
- 52Hz sampling rate, ±2G range
- Magnitude calculation from 3-axis data

### dft.cpp
Discrete Fourier Transform implementation:
- Optimized for 1-10 Hz frequency range
- Band energy calculation for symptom detection
- DC component removal

### detection.cpp
Symptom detection algorithms:
- Tremor: High energy in 3-5 Hz band
- Dyskinesia: High energy in 5-7 Hz band
- FOG: Walking detection followed by sudden stillness

### ble_service.cpp
BLE GATT service with:
- Custom service UUID: `12340000-1234-1234-1234-123456789abc`
- Three characteristics for Tremor, Dyskinesia, and FOG
- Each characteristic: [detected flag, intensity percentage]

## BLE Data Format

| Characteristic | UUID | Data Format |
|----------------|------|-------------|
| Tremor | `12340001-...` | [0/1, 0-100%] |
| Dyskinesia | `12340002-...` | [0/1, 0-100%] |
| FOG | `12340003-...` | [0/1, 100] |

Example:
- `0x01 0x4B` = Detected, 75% intensity
- `0x00 0x00` = Not detected

## LED Indicators

| State | LED1 (Green) | LED2 (Green) |
|-------|--------------|--------------|
| Normal | ON | OFF |
| Tremor | Fast blink | - |
| Dyskinesia | - | Slow blink |
| FOG | Both LEDs alternate blink |
| Walking | - | Toggle |
| BLE Connected | - | ON |

## Getting Started

### Prerequisites

- [PlatformIO](https://platformio.org/) IDE or CLI
- ST B-L475E-IOT01A board
- USB cable
- Mobile device with BLE (iOS/Android)
- LightBlue app (or similar BLE scanner)

### Build and Upload

1. Clone or download this project
2. Open in VSCode with PlatformIO extension
3. Build: Click ✓ or `Ctrl+Alt+B`
4. Upload: Click → or `Ctrl+Alt+U`
5. Monitor: Click 🔌 or `Ctrl+Alt+S`

### Mobile App Setup

1. Install **LightBlue** app on your phone
2. Power on the board
3. Scan for "ParkinsonDet" device
4. Connect and navigate to the custom service
5. Subscribe to characteristics for real-time updates

## Testing

| Action | Expected Result |
|--------|-----------------|
| Hold still | "Normal" state |
| Shake 3-5 times/sec | "TREMOR" detected |
| Shake 5-7 times/sec | "DYSKINESIA" detected |
| Simulate walking then stop | "FOG" detected |

## Configuration

Adjust detection thresholds in `include/config.h`:

```cpp
#define TREMOR_THRESHOLD     500
#define DYSKINESIA_THRESHOLD 500
#define WALKING_THRESHOLD    300
#define STILLNESS_THRESHOLD  1100
```

## Serial Output

```
================================================
    Parkinson's Disease Symptom Detector
    Board: B-L475E-IOT01A
================================================

[1/3] Initializing sensor...
LSM6DSL WHO_AM_I: 0x6A (expected 0x6A)
LSM6DSL initialized: 52Hz, +/-2G
[2/3] Initializing BLE...
BLE initialized.
BLE advertising: ParkinsonDet
[3/3] Initializing controls...

System ready. Starting detection...

T: 120(6%) D:  45(2%) W:  30 A:1080 | Normal
T: 850(42%) D: 200(10%) W: 150 A:1250 | >>> TREMOR <<<
T: 300(15%) D: 950(47%) W: 180 A:1300 | >>> DYSKINESIA <<<
```

## Framework

- **Platform**: ST STM32
- **Framework**: Mbed OS
- **Language**: C++
