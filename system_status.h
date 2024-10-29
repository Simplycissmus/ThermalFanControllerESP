#ifndef SYSTEM_STATUS_H
#define SYSTEM_STATUS_H

#include <Arduino.h>
#include "config.h"

class SystemStatus {
public:
    // Constructor with default values
    SystemStatus();

    // Sensor data
    float temperature;
    float minTemperature;
    float maxTemperature;
    float humidity;

    // Operation mode
    bool autoMode;
    bool fanOn;

    // Fan control
    float manualFanSpeed;
    float currentFanSpeed;
    float targetFanSpeed;
    float fanRPM;
    bool manualOverride;

    // Status messages
    String autoModeStatus;

    // Timestamps
    unsigned long lastSensorUpdate;
    unsigned long lastRPMUpdate;
    unsigned long lastHeatCalc;

    // Error states
    enum class ErrorState {
        NONE,
        SENSOR_ERROR,
        WIFI_ERROR,
        FAN_ERROR
    };
    ErrorState errorState;

    // Error counting
    int rpmErrorCount;

    // Operating statistics
    unsigned long totalOperatingTime;
    unsigned long fanOperatingTime;
    float energyUsage;

    // Heat calculation
    float referenceTemp;
    float totalHeatEnergy;
    float currentHeatPower;
    float airVolumeMoved;
    bool heatCalcInitialized;

    // Methods
    String toJson() const;
    void initializeSystem();
    bool setAutoMode(bool enable);
    void updateHeatCalculation();
    
    bool needsSensorUpdate() const {
        return (millis() - lastSensorUpdate) >= Config::Sensor::UPDATE_INTERVAL;
    }

    bool needsRPMUpdate() const {
        return (millis() - lastRPMUpdate) >= Config::Tacho::RPM_UPDATE_INTERVAL;
    }

    bool needsHeatCalc() const {
        return (millis() - lastHeatCalc) >= Config::Heat::CALC_INTERVAL;
    }

    void updateMinMaxTemperature(float newTemp) {
        if (newTemp < minTemperature) minTemperature = newTemp;
        if (newTemp > maxTemperature) maxTemperature = newTemp;
    }

    void resetMinMaxTemperature() {
        minTemperature = temperature;
        maxTemperature = temperature;
    }

    void updateOperatingStats() {
        totalOperatingTime++;
        if (fanOn) {
            fanOperatingTime++;
            energyUsage += (1.68f * currentFanSpeed) / 3600.0f;
        }
    }

    bool isHeatCalcInitialized() const {
        return heatCalcInitialized;
    }

private:
    String getErrorString() const;
};

#endif // SYSTEM_STATUS_H