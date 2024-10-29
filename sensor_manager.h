#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include <Adafruit_SHT4x.h>
#include <Wire.h>
#include "config.h"
#include "system_status.h"
#include "fan_controller.h"

class SensorManager {
private:
    Adafruit_SHT4x& sht4;
    SystemStatus& status;
    FanController& controller;
    
    uint8_t errorCount = 0;
    static constexpr uint8_t MAX_ERRORS = 3;
    
    // Temperature history for spike detection
    static constexpr int TEMP_HISTORY_SIZE = 5;
    float tempHistory[TEMP_HISTORY_SIZE] = {0};
    int tempHistoryIndex = 0;
    bool historyInitialized = false;

    // Adaptive sampling intervals
    static constexpr unsigned long SLEEP_MODE_INTERVAL = 10000;   // 10 seconds in sleep mode
    static constexpr unsigned long ACTIVE_MODE_INTERVAL = 2000;   // 2 seconds in active mode
    static constexpr unsigned long NIGHT_MODE_INTERVAL = 15000;   // 15 seconds during night hours

    // Validity thresholds
    static constexpr float MIN_VALID_TEMP = -40.0f;
    static constexpr float MAX_VALID_TEMP = 125.0f;
    static constexpr float MIN_VALID_HUM = 0.0f;
    static constexpr float MAX_VALID_HUM = 100.0f;
    static constexpr float MAX_TEMP_CHANGE = 5.0f;  // Maximum plausible temperature change per second

    unsigned long getSensorInterval() {
        time_t now;
        time(&now);
        struct tm* timeinfo = localtime(&now);
        
        // Night mode (22:00 - 06:00)
        if (timeinfo->tm_hour >= 22 || timeinfo->tm_hour < 6) {
            return NIGHT_MODE_INTERVAL;
        }
        
        // Sleep/Active mode based on controller state
        return controller.isInSleepMode() ? SLEEP_MODE_INTERVAL : ACTIVE_MODE_INTERVAL;
    }

    void updateTempHistory(float temp) {
        tempHistory[tempHistoryIndex] = temp;
        tempHistoryIndex = (tempHistoryIndex + 1) % TEMP_HISTORY_SIZE;
        
        if (tempHistoryIndex == 0) {
            historyInitialized = true;
        }
    }

    bool isTemperatureSpike(float temp) {
        if (!historyInitialized) return false;

        float avgTemp = 0;
        for (int i = 0; i < TEMP_HISTORY_SIZE; i++) {
            avgTemp += tempHistory[i];
        }
        avgTemp /= TEMP_HISTORY_SIZE;

        // Check if new temperature deviates significantly from average
        return abs(temp - avgTemp) > MAX_TEMP_CHANGE;
    }
    
    bool checkSensorValues(float temperature, float humidity) {
        // Basic range checks
        if (temperature < MIN_VALID_TEMP || temperature > MAX_VALID_TEMP ||
            humidity < MIN_VALID_HUM || humidity > MAX_VALID_HUM) {
            Serial.println("DEBUG: Sensor values out of valid range");
            return false;
        }
        
        // Check for "stuck" values
        static float lastTemp = -300.0f;
        static float lastHum = -300.0f;
        static uint8_t sameValueCount = 0;
        
        if (temperature == lastTemp && humidity == lastHum) {
            sameValueCount++;
            if (sameValueCount >= 5) {
                Serial.println("DEBUG: Sensor values appear to be stuck");
                return false;
            }
        } else {
            sameValueCount = 0;
        }
        
        // Check for temperature spikes
        if (isTemperatureSpike(temperature)) {
            Serial.println("DEBUG: Temperature spike detected");
            return false;
        }
        
        lastTemp = temperature;
        lastHum = humidity;
        updateTempHistory(temperature);
        
        return true;
    }
    
    void updateErrorState(bool success) {
        if (!success) {
            errorCount++;
            Serial.print("DEBUG: Sensor error count: ");
            Serial.println(errorCount);
            
            if (errorCount >= MAX_ERRORS) {
                status.errorState = SystemStatus::ErrorState::SENSOR_ERROR;
                Serial.println("DEBUG: Maximum sensor errors reached");
            }
        } else {
            if (errorCount > 0) {
                Serial.println("DEBUG: Sensor recovered from errors");
            }
            errorCount = 0;
            if (status.errorState == SystemStatus::ErrorState::SENSOR_ERROR) {
                status.errorState = SystemStatus::ErrorState::NONE;
            }
        }
    }

public:
    SensorManager(Adafruit_SHT4x& sht4Sensor, 
                 SystemStatus& systemStatus,
                 FanController& fanController)
        : sht4(sht4Sensor)
        , status(systemStatus)
        , controller(fanController)
    {
        Serial.println("DEBUG: Sensor manager initialized");
    }

    bool initialize() {
        Serial.println("DEBUG: Initializing sensor");
        
        if (!sht4.begin(&Wire)) {
            Serial.println("DEBUG: Failed to find SHT4x sensor");
            status.errorState = SystemStatus::ErrorState::SENSOR_ERROR;
            return false;
        }

        sht4.setPrecision(SHT4X_HIGH_PRECISION);
        sht4.setHeater(SHT4X_NO_HEATER);
        
        delay(Config::Sensor::WARMUP_TIME);
        
        status.errorState = SystemStatus::ErrorState::NONE;
        errorCount = 0;
        
        Serial.println("DEBUG: Sensor initialization successful");
        return true;
    }

    bool update() {
        static unsigned long lastUpdate = 0;
        unsigned long now = millis();
        unsigned long interval = getSensorInterval();
        
        if (now - lastUpdate < interval) {
            return true;
        }
        lastUpdate = now;

        Serial.println("DEBUG: Reading sensor");
        
        sensors_event_t humidity_event, temp_event;
        bool success = sht4.getEvent(&humidity_event, &temp_event);
        
        if (success) {
            float newTemp = temp_event.temperature;
            float newHum = humidity_event.relative_humidity;
            
            Serial.print("DEBUG: Temperature reading: ");
            Serial.print(newTemp);
            Serial.print("°C, Humidity: ");
            Serial.print(newHum);
            Serial.println("%");
            
            if (checkSensorValues(newTemp, newHum)) {
                status.temperature = newTemp;
                status.humidity = newHum;
                status.updateMinMaxTemperature(newTemp);
                status.lastSensorUpdate = now;
                
                if (status.autoMode) {
                    controller.updateAutomaticMode();
                }
                
                status.updateOperatingStats();
                status.updateHeatCalculation();
                
                updateErrorState(true);
                return true;
            } else {
                Serial.println("DEBUG: Sensor values failed plausibility check");
            }
        } else {
            Serial.println("DEBUG: Failed to read sensor");
        }
        
        updateErrorState(false);
        return false;
    }
};

#endif // SENSOR_MANAGER_H