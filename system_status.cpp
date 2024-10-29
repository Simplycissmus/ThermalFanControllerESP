#include "system_status.h"
#include "config.h"

SystemStatus::SystemStatus() :
    temperature(0.0f),
    minTemperature(100.0f),
    maxTemperature(-40.0f),
    humidity(0.0f),
    autoMode(true),
    fanOn(false),
    manualFanSpeed(0.5f),
    currentFanSpeed(0.0f),
    targetFanSpeed(0.0f),
    fanRPM(0.0f),
    autoModeStatus("System started"),
    lastSensorUpdate(0),
    lastRPMUpdate(0),
    lastHeatCalc(0),
    errorState(ErrorState::NONE),
    manualOverride(false),
    rpmErrorCount(0),
    totalOperatingTime(0),
    fanOperatingTime(0),
    energyUsage(0.0f),
    referenceTemp(0.0f),
    totalHeatEnergy(0.0f),
    currentHeatPower(0.0f),
    airVolumeMoved(0.0f),
    heatCalcInitialized(false)
{
    initializeSystem();
}

void SystemStatus::initializeSystem() {
    heatCalcInitialized = true;
    referenceTemp = temperature;
    lastHeatCalc = millis();
    
    Serial.println("Heat calculation initialized:");
    Serial.print("Reference Temperature: ");
    Serial.println(referenceTemp);
}

bool SystemStatus::setAutoMode(bool enable) {
    autoMode = enable;
    if (!enable) {
        manualFanSpeed = currentFanSpeed > 0.0f ? currentFanSpeed : 0.5f;
    }
    Serial.print("Mode changed to: ");
    Serial.println(enable ? "Automatic" : "Manual");
    return true;
}

void SystemStatus::updateHeatCalculation() {
    if (!heatCalcInitialized) {
        initializeSystem();
    }
    
    if (!fanOn) return;
    
    unsigned long now = millis();
    float deltaTime = (now - lastHeatCalc) / 1000.0f;
    lastHeatCalc = now;
    
    float tempDiff = temperature - referenceTemp;
    if (tempDiff > Config::Heat::MIN_TEMP_DIFF) {
        float airflow = (Config::Heat::MAX_AIRFLOW / 3600.0f) * currentFanSpeed;
        float airDensity = Config::Heat::AIR_DENSITY * (293.15f / (temperature + 273.15f));
        
        currentHeatPower = airDensity * airflow * 
                         Config::Heat::AIR_SPECIFIC_HEAT * 
                         tempDiff * 
                         Config::Heat::SYSTEM_EFFICIENCY;
        
        totalHeatEnergy += (currentHeatPower * deltaTime) / 3600000.0f;
        airVolumeMoved += airflow * deltaTime;
    }
}

String SystemStatus::toJson() const {
    String json = "{";
    
    // Basic sensor data
    json += "\"temperature\":" + String(temperature, 1);
    json += ",\"min_temperature\":" + String(minTemperature, 1);
    json += ",\"max_temperature\":" + String(maxTemperature, 1);
    json += ",\"humidity\":" + String(humidity, 1);
    
    // Operation mode
    json += ",\"auto_mode\":" + String(autoMode ? "true" : "false");
    json += ",\"fan_on\":" + String(fanOn ? "true" : "false");
    
    // Fan control
    json += ",\"manual_fan_speed\":" + String(manualFanSpeed, 3);
    json += ",\"current_fan_speed\":" + String(currentFanSpeed, 3);
    json += ",\"target_fan_speed\":" + String(targetFanSpeed, 3);
    json += ",\"fan_rpm\":" + String(fanRPM);
    
    // Heat calculation data
    json += ",\"heat_calc_active\":true";
    json += ",\"reference_temp\":" + String(referenceTemp, 1);
    json += ",\"total_heat_energy\":" + String(totalHeatEnergy, 3);
    json += ",\"current_heat_power\":" + String(currentHeatPower, 1);
    json += ",\"air_volume_moved\":" + String(airVolumeMoved, 2);
    
    // Calculate average power
    float runningHours = fanOperatingTime / 3600.0f;
    float avgPower = runningHours > 0 ? (totalHeatEnergy * 1000.0f) / runningHours : 0;
    json += ",\"avg_heat_power\":" + String(avgPower, 1);
    
    // Operating statistics
    json += ",\"total_operating_time\":" + String(totalOperatingTime);
    json += ",\"fan_operating_time\":" + String(fanOperatingTime);
    json += ",\"energy_usage\":" + String(energyUsage, 3);
    json += ",\"auto_mode_status\":\"" + autoModeStatus + "\"";
    json += ",\"error_state\":\"" + getErrorString() + "\"";
    json += ",\"last_sensor_update\":" + String(lastSensorUpdate);
    json += ",\"last_rpm_update\":" + String(lastRPMUpdate);
    json += ",\"last_heat_calc\":" + String(lastHeatCalc);
    
    json += "}";
    return json;
}

String SystemStatus::getErrorString() const {
    switch(errorState) {
        case ErrorState::NONE:
            return "OK";
        case ErrorState::SENSOR_ERROR:
            return "Sensor Error";
        case ErrorState::WIFI_ERROR:
            return "WiFi Error";
        case ErrorState::FAN_ERROR:
            return "Fan Error";
        default:
            return "Unknown Error";
    }
}