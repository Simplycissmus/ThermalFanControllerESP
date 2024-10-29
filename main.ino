#include <Arduino.h>
#include <WiFi.h>
#include <Wire.h>
#include <driver/ledc.h>  // For PWM control
#include <Adafruit_SHT4x.h>
#include "config.h"
#include "sensor_manager.h"
#include "web_server.h"
#include "system_status.h"
#include "fan_controller.h"

// Global objects
Adafruit_SHT4x sht4;                       // Temperature sensor
SystemStatus systemStatus;                 // System status
FanController fanController(systemStatus); // Fan controller
SensorManager sensorManager(sht4, systemStatus, fanController); // Sensor manager
WebServerManager webServer(systemStatus, fanController); // Web server
unsigned long tachoLastPulse = 0;          // Timestamp of last tachometer pulse
volatile unsigned long tachoPulses = 0;    // Number of tachometer pulses
unsigned long lastStatsUpdate = 0;         // Timestamp of last statistics update

// Interrupt handler for tachometer sensor
void IRAM_ATTR handleTachoInterrupt() {
    unsigned long now = micros();  // Microseconds for precise time measurement
    if (now - tachoLastPulse >= 1000) { // Set debouncing to 1 ms
        tachoPulses++;
        tachoLastPulse = now;
    }
}

// Initialize hardware
void initializeHardware() {
    // Initialize I2C
    Wire.begin(Config::Pins::I2C_SDA, Config::Pins::I2C_SCL);
    
    // Initialize Serial
    Serial.begin(Config::System::SERIAL_BAUD);

    // Configure MOSFET pin
    pinMode(Config::Pins::MOSFET_PIN, OUTPUT);
    digitalWrite(Config::Pins::MOSFET_PIN, LOW);  // Default to off

    // LEDC timer configuration for PWM
    ledc_timer_config_t ledcTimer = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = static_cast<ledc_timer_bit_t>(Config::PWM::RESOLUTION),
        .timer_num = LEDC_TIMER_0,
        .freq_hz = Config::PWM::FREQUENCY
    };
    ledc_timer_config(&ledcTimer);

    // LEDC channel configuration for PWM
    ledc_channel_config_t ledcChannel = {
        .gpio_num = Config::Pins::PWM_PIN,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = static_cast<ledc_channel_t>(Config::PWM::CHANNEL),
        .timer_sel = LEDC_TIMER_0,
        .duty = 0,
        .hpoint = 0
    };
    ledc_channel_config(&ledcChannel);

    // Configure tachometer pin and interrupt
    pinMode(Config::Pins::TACHO_PIN, INPUT_PULLUP);  // Pull-up for hall sensor
    attachInterrupt(digitalPinToInterrupt(Config::Pins::TACHO_PIN), handleTachoInterrupt, RISING);
}

// Initialize WiFi
void initializeWiFi() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(Config::WIFI_SSID, Config::WIFI_PASSWORD);
    
    unsigned long startAttemptTime = millis();
    
    // Wait for WiFi connection
    while (WiFi.status() != WL_CONNECTED && 
           millis() - startAttemptTime < Config::WIFI_CONNECT_TIMEOUT) {
        Serial.print(".");
        delay(Config::WIFI_RETRY_DELAY);
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nConnected to WiFi");
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("\nUnable to connect to WiFi!");
        systemStatus.errorState = SystemStatus::ErrorState::WIFI_ERROR;
    }
}

// Calculate RPM
void updateRPM() {
    if (systemStatus.needsRPMUpdate()) {
        unsigned long pulses = tachoPulses;
        tachoPulses = 0; // Reset counter
        
        float rpm = (pulses * 60000.0f) / 
                   ((millis() - systemStatus.lastRPMUpdate) * Config::Tacho::PULSES_PER_REVOLUTION);
                   
        // Update status
        systemStatus.fanRPM = rpm;
        systemStatus.lastRPMUpdate = millis();
        
        // Check if fan is blocked
        if (systemStatus.fanOn && rpm < Config::Tacho::MIN_RPM_THRESHOLD) {
            systemStatus.errorState = SystemStatus::ErrorState::FAN_ERROR;
        } else if (systemStatus.errorState == SystemStatus::ErrorState::FAN_ERROR) {
            systemStatus.errorState = SystemStatus::ErrorState::NONE;
        }
    }
}

void setup() {
    initializeHardware();
    Serial.println("\nHardware initialized");
    
    initializeWiFi();
    
    if (!sensorManager.initialize()) {
        Serial.println("Sensor initialization failed!");
    } else {
        Serial.println("Sensor initialized");
    }
    
    webServer.begin();
}

void loop() {
    // Feed the watchdog
    delay(Config::System::WATCHDOG_DELAY);
    
    // Update sensor data
    sensorManager.update();
    
    // Update RPM
    updateRPM();
    
    // Update web server
    webServer.handle();

    // Update operating statistics every second
    if (millis() - lastStatsUpdate >= 1000) {  // Interval of 1 second
        systemStatus.updateOperatingStats();
        lastStatsUpdate = millis();
    }
}
