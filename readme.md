# Smart Heat Recovery Fan Control System

![Project Status](https://img.shields.io/badge/status-active-success.svg)
![Version](https://img.shields.io/badge/version-2024.1-blue.svg)
![License](https://img.shields.io/badge/license-MIT-green.svg)

## Project Metadata

- **Project Name**: Smart Heat Recovery Fan Control System
- **Version**: 2024.1
- **Last Updated**: October 29, 2024
- **Author**: Patric Aeberhard
- **License**: MIT
- **Repository**: https://github.com/Simplycissmus/ThermalFanControllerESP_private
- **Development Status**: Active
- **Target Platform**: ESP32 (Seeed Studio XIAO ESP32C6)

## Description

An intelligent fan control system for optimizing heat recovery from heating cassettes (Cheminée), featuring advanced temperature monitoring, adaptive control algorithms, and comprehensive heat transfer calculations. Built on ESP32 architecture with a high-precision Noctua fan, the system offers both automatic and manual control modes through an intuitive web interface.

The system includes support for a custom 3D-printed adapter for the warm air tube, which will be published separately.

## Table of Contents
1. [Features](#features)
2. [Project Structure](#project-structure)
3. [Hardware](#hardware)
4. [Configuration](#configuration)
5. [Usage](#usage)
6. [API Reference](#api-reference)
7. [Heat Transfer Calculations](#heat-transfer-calculations)
8. [Safety Features](#safety-features)

## Features

### Core Functionality
- **Advanced Temperature Control**: Intelligent fan speed regulation based on temperature differentials
- **Heat Recovery Calculation**: Real-time computation of recovered energy and system efficiency
- **Adaptive Control**: Dynamic adjustment based on time of day and usage patterns
- **Sleep Mode**: Energy-efficient operation during low-activity periods
- **Precise Environmental Monitoring**: High-accuracy temperature and humidity tracking
- **Manual Override**: Granular speed control with percentage-based adjustment
- **Non-volatile Settings**: Configuration persistence across power cycles

### Performance Monitoring
- **Real-time Heat Transfer Analysis**:
  - Current heat power calculation (Watts)
  - Total energy recovered (kWh)
  - System efficiency metrics
  - Air volume movement tracking

- **Environmental Metrics**:
  - Temperature trending with spike detection
  - Humidity impact analysis
  - Min/max temperature recording

- **Advanced Performance Metrics**:
  - Air density compensation based on temperature and humidity
  - Real-time mass flow rate calculations
  - Temperature differential efficiency tracking
  - Cubic relationship airflow modeling based on fan speed
  - Dynamic system efficiency calculations
  - Heat power calculation using physical formulas:
    - P = ṁ * c * ΔT * η
    - Where: ṁ = ρ * V̇ (mass flow rate = density * volume flow rate)
  - Automatic humidity correction for density calculations
  - Energy savings estimation with cost calculations
  - Comprehensive performance statistics including:
    - Running averages
    - Peak performance tracking
    - Efficiency trending
    - Total air volume processed
    - Cumulative heat energy recovered

### Web Interface
- **Responsive Design**: Mobile-first interface with dark mode support
- **Real-time Updates**: Live data streaming with automatic refresh
- **Interactive Controls**:
  - Mode switching (Auto/Manual)
  - Visual speed adjustment
  - Temperature range reset
- **Visual Feedback**:
  - Dynamic progress indicators
  - Status notifications
  - Temperature trend visualization
  - Error state display

### Safety Features
- **Comprehensive Error Detection**:
  - Sensor malfunction monitoring
  - Fan failure detection
  - Temperature spike identification
  - Network connectivity verification
- **Automatic Recovery**:
  - Error state management
  - System reinitialization
  - Failsafe mode operation
- **Protection Mechanisms**:
  - Over-temperature safeguards
  - Fan blockage detection
  - Power surge protection
  - Sensor plausibility checks

## Project Structure
```
project/
├── main.ino          # Main application entry point
├── config.h                 # System configuration and constants
├── system_status.h         # System state definitions
├── system_status.cpp      # State management implementation
├── heat_calculator.h      # Heat transfer calculations
├── fan_controller.h       # Fan control algorithms
├── sensor_manager.h       # Sensor interface and validation
├── web_server.h          # Web server and API handler
├── html_content.h        # Web interface HTML structure
├── html_styles.h         # CSS styling definitions
└── html_script.h         # JavaScript client functionality
```

## Hardware

### Core Components
- **Microcontroller**: Seeed Studio XIAO ESP32C6
  - Ultra-compact form factor (21×17.5mm)
  - Built-in WiFi and Bluetooth
  - Low power consumption
  - 3.3V logic level

- **Fan**: Noctua NF-A12x25 PWM
  - 120mm premium cooling fan
  - PWM speed control (450-2000 RPM)
  - Advanced acoustic optimization
  - Efficiency-focused design
  - Maximum airflow: 102.1 m³/h
  - Peak power consumption: 1.68W

- **Temperature/Humidity Sensor**: Adafruit SHT45
  - High-precision measurements
  - Temperature accuracy: ±0.1°C
  - Humidity accuracy: ±1% RH
  - I²C interface
  - Built-in precision heater

### Power Management
- **Power Supply**: J5019 Dual Output
  - Input voltage: 12-24V DC
  - Primary output: 12V (fan)
  - Secondary output: 5V (microcontroller)
  - Integrated protection features

### Protection Circuit
- **Components**:
  - IRF530N MOSFET (N-Channel)
  - Schottky diode protection
  - Capacitive filtering
  - Pull-up/down resistors

### 3D-Printed Components
- Custom adapter for warm air tube (3D model to be published)

## Heat Transfer Calculations

The system implements sophisticated heat transfer calculations based on:

### Physical Parameters
- Air density compensation for temperature
- Humidity impact on heat capacity
- Fan characteristic curve modeling
- System efficiency factors

### Calculation Methods
```cpp
float calculateHeatTransfer() {
    float airDensity = calculateAirDensity();
    float airflow = calculateCurrentAirflow();
    float tempDiff = calculateTempDifference();
    
    return airDensity * airflow * AIR_SPECIFIC_HEAT * 
           tempDiff * SYSTEM_EFFICIENCY;
}
```

## API Reference

### Core Endpoints

#### Status Endpoint
```
GET /api/v1/status
```
Returns comprehensive system status including:
- Temperature and humidity readings
- Fan operational parameters
- Heat transfer metrics
- System state

#### Control Endpoints
```
POST /api/v1/fan/toggle
POST /api/v1/fan/mode
POST /api/v1/fan/speed
POST /api/v1/temperature/reset
```

### Response Format
```json
{
    "temperature": 25.5,
    "humidity": 45.2,
    "fan_on": true,
    "current_fan_speed": 0.75,
    "heat_calc_active": true,
    "current_heat_power": 150.5,
    "total_heat_energy": 2.35,
    "air_volume_moved": 250.5
}
```

## Configuration

### System Parameters
```cpp
namespace Config {
    // Temperature Control
    constexpr float TEMP_THRESHOLD = 25.0f;
    constexpr float MAX_TEMP = 100.0f;
    constexpr float HYSTERESIS = 2.0f;

    // Heat Calculation
    namespace Heat {
        constexpr float MAX_AIRFLOW = 102.1f;
        constexpr float AIR_SPECIFIC_HEAT = 1.005f;
        constexpr float SYSTEM_EFFICIENCY = 0.85f;
    }

    // System Timing
    namespace System {
        constexpr int WATCHDOG_DELAY = 10;
        constexpr int SERIAL_BAUD = 115200;
    }
}
```

## License
This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Safety Notes
- Follow proper electrical safety guidelines during installation
- Ensure adequate ventilation for components
- Monitor system performance regularly
- Maintain clearance around heat sources
- Follow local building and safety codes

## Change Log

### Version 2024.1 (2024-10-29)
- Initial release
- Implemented core heat recovery functionality
- Added web interface with real-time monitoring
- Integrated advanced heat transfer calculations
- Implemented adaptive control algorithms
- Added comprehensive error handling
- Developed responsive web interface

## Acknowledgments

Special thanks to:
- The ESP32 community for their extensive documentation
- The Noctua team for their detailed fan specifications
- Contributors to the various open-source libraries used in this project