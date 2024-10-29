#ifndef HEAT_CALCULATOR_H
#define HEAT_CALCULATOR_H

#include <Arduino.h>
#include "config.h"
#include "system_status.h"

class HeatCalculator {
private:
    SystemStatus& status;

    /**
     * @brief Calculates air density based on temperature and humidity
     * @return Corrected air density in kg/m³
     */
    float calculateAirDensity() const {
        // Basic temperature correction for air density
        // ρ = ρ0 * (T0 / T), where T is in Kelvin
        float tempKelvin = status.temperature + 273.15f;
        float density = Config::Heat::AIR_DENSITY * (293.15f / tempKelvin);
        
        // Humidity correction (simplified)
        // Reduce density slightly with increasing humidity
        density *= (1.0f - (status.humidity * 0.0002f));  // Max 2% reduction at 100% humidity
        
        return density;
    }

    /**
     * @brief Calculates current airflow based on fan speed
     * @return Current airflow in m³/s
     */
    float calculateCurrentAirflow() const {
        // Convert max airflow from m³/h to m³/s
        const float maxAirflowPerSecond = Config::Heat::MAX_AIRFLOW / 3600.0f;
        
        // Calculate current airflow based on fan speed
        // Using cubic relationship for better accuracy
        return maxAirflowPerSecond * pow(status.currentFanSpeed, 3);
    }

    /**
     * @brief Calculates useful temperature difference
     * @return Temperature difference in Kelvin
     */
    float calculateTempDifference() const {
        float tempDiff = status.temperature - status.referenceTemp;
        
        // Only consider positive temperature differences above minimum threshold
        if (tempDiff < Config::Heat::MIN_TEMP_DIFF) {
            return 0.0f;
        }
        
        return tempDiff;
    }

public:
    /**
     * @brief Constructor that initializes with a reference to SystemStatus
     * @param systemStatus Reference to the system status object
     */
    explicit HeatCalculator(SystemStatus& systemStatus) : status(systemStatus) {}

    /**
     * @brief Initializes heat calculation with the current temperature
     */
    void initialize() {
        if (!status.isHeatCalcInitialized()) {
            status.initHeatCalculation(status.temperature);
            Serial.println("DEBUG: Heat calculation initialized.");
        }
    }

    /**
     * @brief Updates heat transfer statistics based on current sensor data and fan status
     */
    void update() {
        if (!status.isHeatCalcInitialized() || !status.fanOn || !status.needsHeatCalc()) {
            return;
        }

        // Calculate time delta in seconds
        float deltaTime = (millis() - status.lastHeatCalc) / 1000.0f;
        status.lastHeatCalc = millis();

        // Get current air properties
        float airDensity = calculateAirDensity();
        float airflow = calculateCurrentAirflow();
        float tempDiff = calculateTempDifference();

        // Update air volume moved
        status.airVolumeMoved += airflow * deltaTime;

        // Calculate current heat power
        // P = ṁ * c * ΔT * η
        // where ṁ = ρ * V̇ (mass flow rate = density * volume flow rate)
        float massFlowRate = airDensity * airflow;
        status.currentHeatPower = massFlowRate * 
                                 Config::Heat::AIR_SPECIFIC_HEAT * 
                                 tempDiff * 
                                 Config::Heat::SYSTEM_EFFICIENCY;

        // Calculate energy moved in this interval
        float energyKWh = (status.currentHeatPower * deltaTime) / 3600000.0f; // Convert Ws to kWh
        status.totalHeatEnergy += energyKWh;
        status.heatCalcInitialized = true;
    }

    /**
     * @brief Gets the estimated heating cost savings
     * @param pricePerKWh Price per kWh in local currency
     * @return Estimated cost savings
     */
    float calculateCostSaving(float pricePerKWh) const {
        return status.totalHeatEnergy * pricePerKWh;
    }

    /**
     * @brief Calculates the system's current efficiency
     * @return Current efficiency percentage
     */
    float calculateCurrentEfficiency() const {
        if (!status.fanOn || status.currentHeatPower <= 0) {
            return 0.0f;
        }

        // Compare actual heat transfer with theoretical maximum
        float maxPossiblePower = calculateMaxPossiblePower();
        if (maxPossiblePower <= 0) {
            return 0.0f;
        }

        return (status.currentHeatPower / maxPossiblePower) * 100.0f;
    }

    /**
     * @brief Calculates maximum possible heat power under current conditions
     * @return Maximum theoretical power in Watts
     */
    float calculateMaxPossiblePower() const {
        float airDensity = calculateAirDensity();
        float maxAirflow = Config::Heat::MAX_AIRFLOW / 3600.0f; // Convert to m³/s
        float tempDiff = calculateTempDifference();

        return airDensity * maxAirflow * Config::Heat::AIR_SPECIFIC_HEAT * tempDiff;
    }
};

#endif // HEAT_CALCULATOR_H
