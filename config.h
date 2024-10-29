#ifndef CONFIG_H
#define CONFIG_H

namespace Config {
    // Temperature Configuration
    constexpr float TEMP_THRESHOLD = 25.0f;    // Start temperature in °C
    constexpr float MAX_TEMP = 100.0f;         // Maximum temperature in °C
    constexpr float HYSTERESIS = 2.0f;         // Temperature hysteresis in °C
    
    // Check Configuration
    constexpr unsigned long CHECK_DURATION = 30000;  // Duration of activity check in ms

    // Heat Configuration
    namespace Heat {
        // Fan specifications (Noctua NF-A12x25 PWM)
        constexpr float MAX_AIRFLOW = 102.1f;        // Maximum airflow in m³/h
        constexpr float FAN_DIAMETER = 0.120f;       // Fan diameter in meters
        constexpr float FAN_AREA = 0.0113f;          // Fan area in m² (π * r²)
        
        // Physical constants
        constexpr float AIR_SPECIFIC_HEAT = 1.005f;  // Specific heat capacity of air in kJ/(kg·K)
        constexpr float AIR_DENSITY = 1.204f;        // Air density at 20°C in kg/m³
        
        // Calculation intervals
        constexpr unsigned long CALC_INTERVAL = 10000; // Heat calculation interval in ms
        
        // System efficiency
        constexpr float SYSTEM_EFFICIENCY = 0.85f;   // Estimated system efficiency (heat transfer)
        
        // Temperature reference
        constexpr float MIN_TEMP_DIFF = 0.5f;        // Minimum temperature difference to consider
    }
    
    // WiFi Configuration
    constexpr const char* WIFI_SSID = "yourSSID";
    constexpr const char* WIFI_PASSWORD = "yourPassword";
    constexpr int WIFI_CONNECT_TIMEOUT = 10000; // WiFi connection timeout in ms
    constexpr int WIFI_RETRY_DELAY = 500;       // Delay between connection attempts in ms
    
    // Pin Configuration
    namespace Pins {
        // I²C Pins
        constexpr int I2C_SDA = 22;              // GPIO22 for I2C Data
        constexpr int I2C_SCL = 23;              // GPIO23 for I2C Clock
        
        // Fan Control
        constexpr int PWM_PIN = 20;              // GPIO20 for Fan PWM input
        constexpr int MOSFET_PIN = 17;           // GPIO17 for MOSFET control
        constexpr int TACHO_PIN = 19;            // GPIO19 for tachometer input
    }

    // PWM Configuration
    namespace PWM {
        constexpr int FREQUENCY = 25000;         // PWM frequency in Hz
        constexpr int CHANNEL = 0;               // PWM channel (0-15)
        constexpr int RESOLUTION = 8;            // Resolution in bits (8 = values 0-255)
        constexpr int MIN_DUTY = 20;             // Minimum duty cycle
        constexpr int MAX_DUTY = 255;            // Maximum duty cycle at 8 bit
    }
    
    // Sensor Configuration
    namespace Sensor {
        constexpr unsigned long UPDATE_INTERVAL = 2000; // Sensor update interval in ms
        constexpr unsigned long WARMUP_TIME = 100;      // Sensor warmup time in ms
    }
    
    // Tachometer Configuration
    namespace Tacho {
        constexpr unsigned long RPM_UPDATE_INTERVAL = 1000; // RPM calculation interval in ms
        constexpr int PULSES_PER_REVOLUTION = 2;           // Pulses per revolution
        constexpr int MIN_RPM_THRESHOLD = 450;             // Minimum valid RPM
        constexpr int MAX_NO_RPM_COUNT = 5;                // Max error count
    }
    
    // Webserver Configuration
    namespace WebServer {
        constexpr int PORT = 80;                           // HTTP port
        constexpr unsigned long UPDATE_INTERVAL = 2000;    // Client update interval in ms
    }
    
    // System Configuration
    namespace System {
        constexpr int SERIAL_BAUD = 115200;               // Baud rate for serial communication
        constexpr int WATCHDOG_DELAY = 10;                // Delay for ESP32 watchdog in ms
    }
}

#endif // CONFIG_H
