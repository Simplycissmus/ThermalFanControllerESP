#ifndef FAN_CONTROLLER_H
#define FAN_CONTROLLER_H

#include <Arduino.h>
#include <driver/ledc.h>
#include "config.h"
#include "system_status.h"

class FanController {
private:
    SystemStatus& status;
    float lastTemperature = 0.0f;
    unsigned long lastTempUpdate = 0;
    unsigned long lastCheckTime = 0;
    bool inSleepMode = true;
    int errorCount = 0;

    // Temperature history for trend analysis
    static constexpr int TEMP_HISTORY_SIZE = 6;
    float tempHistory[TEMP_HISTORY_SIZE] = {0};
    int tempHistoryIndex = 0;
    
    // Constants
    static constexpr float SLEEP_TEMP_THRESHOLD = 22.0f;    // Below this temperature -> sleep mode
    static constexpr float TEMP_RISE_THRESHOLD = 0.2f;      // Temperature rise indicating activity
    static constexpr float MIN_SPEED = 0.2f;                // Minimum fan speed when active
    static constexpr int MAX_ERRORS = 3;
    
    // LEDC configuration
    ledc_timer_config_t ledcTimer;
    ledc_channel_config_t ledcChannel;

    void initPWM() {
        Serial.println("DEBUG: Initializing PWM");
        
        // LEDC Timer configuration
        ledcTimer.speed_mode = LEDC_LOW_SPEED_MODE;
        ledcTimer.duty_resolution = static_cast<ledc_timer_bit_t>(Config::PWM::RESOLUTION);
        ledcTimer.timer_num = LEDC_TIMER_0;
        ledcTimer.freq_hz = Config::PWM::FREQUENCY;
        ledc_timer_config(&ledcTimer);

        // LEDC Channel configuration
        ledcChannel.gpio_num = Config::Pins::PWM_PIN;
        ledcChannel.speed_mode = LEDC_LOW_SPEED_MODE;
        ledcChannel.channel = static_cast<ledc_channel_t>(Config::PWM::CHANNEL);
        ledcChannel.timer_sel = LEDC_TIMER_0;
        ledcChannel.duty = 0;
        ledcChannel.hpoint = 0;
        ledc_channel_config(&ledcChannel);

        pinMode(Config::Pins::MOSFET_PIN, OUTPUT);
        digitalWrite(Config::Pins::MOSFET_PIN, LOW);
        
        Serial.println("DEBUG: PWM initialized");
    }

    unsigned long getCheckInterval() {
        time_t now;
        time(&now);
        struct tm* timeinfo = localtime(&now);
        
        // Night time (22:00 - 06:00)
        if (timeinfo->tm_hour >= 22 || timeinfo->tm_hour < 6) {
            return 30 * 60000; // 30 minutes
        }
        // Peak usage time (17:00 - 22:00)
        else if (timeinfo->tm_hour >= 17 && timeinfo->tm_hour < 22) {
            return 3 * 60000;  // 3 minutes
        }
        // Default interval
        return 5 * 60000;     // 5 minutes
    }

    void updateTempHistory(float temp) {
        tempHistory[tempHistoryIndex] = temp;
        tempHistoryIndex = (tempHistoryIndex + 1) % TEMP_HISTORY_SIZE;
    }

    float getTempTrend() {
        float sum = 0;
        for (int i = 1; i < TEMP_HISTORY_SIZE; i++) {
            int prev = (tempHistoryIndex - i - 1 + TEMP_HISTORY_SIZE) % TEMP_HISTORY_SIZE;
            int curr = (tempHistoryIndex - i + TEMP_HISTORY_SIZE) % TEMP_HISTORY_SIZE;
            sum += tempHistory[curr] - tempHistory[prev];
        }
        return sum / (TEMP_HISTORY_SIZE - 1);
    }

    void handleError(const String& errorType) {
        errorCount++;
        Serial.print("DEBUG: Fan error: ");
        Serial.println(errorType);
        
        if (errorCount >= MAX_ERRORS) {
            status.autoModeStatus = "Critical Error - System Restart Required";
            Serial.println("DEBUG: Maximum errors reached, restarting system");
            ESP.restart();
        } else {
            status.autoModeStatus = "Error - Recovery Attempt " + String(errorCount);
            Serial.println("DEBUG: Attempting error recovery");
            initPWM();
            toggleFan(false);
            delay(1000);
            if (status.autoMode) {
                toggleFan(true);
            }
        }
    }

    void clearErrors() {
        if (errorCount > 0) {
            errorCount = 0;
            status.autoModeStatus = "System Recovered";
            Serial.println("DEBUG: System recovered from errors");
        }
    }

    bool shouldActivateCheck() {
        if (!inSleepMode) return true;
        
        unsigned long now = millis();
        unsigned long checkInterval = getCheckInterval();
        
        if (now - lastCheckTime >= checkInterval) {
            lastCheckTime = now;
            status.autoModeStatus = "Checking for Activity...";
            Serial.println("DEBUG: Starting activity check");
            return true;
        }
        
        if (now - lastCheckTime < Config::CHECK_DURATION) {
            return true;
        }
        
        status.autoModeStatus = "Sleep Mode - Next Check in " + 
            String((checkInterval - (now - lastCheckTime)) / 60000) + " Minutes";
        return false;
    }

    float calculateTargetSpeed() {
        float temp = status.temperature;
        float targetSpeed = 0.0f;
        String statusMsg;
        
        updateTempHistory(temp);
        float tempTrend = getTempTrend();
        
        float tempChangeRate = 0.0f;
        unsigned long timeDiff = millis() - lastTempUpdate;
        if (timeDiff > 0) {
            tempChangeRate = (temp - lastTemperature) / (timeDiff / 60000.0f);
        }

        // Sleep mode logic
        if (temp < SLEEP_TEMP_THRESHOLD) {
            if (!shouldActivateCheck()) {
                lastTemperature = temp;
                lastTempUpdate = millis();
                return 0.0f;
            }
            targetSpeed = MIN_SPEED;
            
            if (tempChangeRate > TEMP_RISE_THRESHOLD || tempTrend > TEMP_RISE_THRESHOLD) {
                inSleepMode = false;
                statusMsg = "Activity Detected - Starting Normal Operation";
                Serial.println("DEBUG: Activity detected, exiting sleep mode");
            } else {
                statusMsg = "Checking for Activity (" + 
                    String((Config::CHECK_DURATION - (millis() - lastCheckTime)) / 1000) + "s)";
            }
        } else {
            inSleepMode = false;
            
            if (tempChangeRate > 0.5f || tempTrend > 0.3f) {
                float normalizedTemp = (temp - Config::TEMP_THRESHOLD) / 
                                     (Config::MAX_TEMP - Config::TEMP_THRESHOLD);
                targetSpeed = 0.6f + (normalizedTemp * 0.4f);
                statusMsg = "Warm-up Phase: Optimizing Heat Distribution (" + 
                    String(targetSpeed * 100, 0) + "%)";
                Serial.println("DEBUG: Warm-up phase active");
            }
            else if (temp >= Config::TEMP_THRESHOLD + Config::HYSTERESIS) {
                float normalizedTemp = (temp - Config::TEMP_THRESHOLD) / 
                                     (Config::MAX_TEMP - Config::TEMP_THRESHOLD);
                targetSpeed = 0.3f + (pow(normalizedTemp, 2) * 0.7f);
                statusMsg = "Operating Phase: " + String(targetSpeed * 100, 0) + "% Power";
            }
            else if (temp > Config::TEMP_THRESHOLD - Config::HYSTERESIS) {
                if (status.currentFanSpeed < 0.1f) {
                    targetSpeed = 0.0f;
                    statusMsg = "Cooling Phase: Fan Off";
                    if (tempChangeRate < 0 && temp < SLEEP_TEMP_THRESHOLD) {
                        inSleepMode = true;
                        statusMsg = "Entering Sleep Mode";
                        Serial.println("DEBUG: Entering sleep mode");
                    }
                } else {
                    targetSpeed = 0.3f;
                    statusMsg = "Cooling Phase: Using Residual Heat";
                }
            }
        }

        lastTemperature = temp;
        lastTempUpdate = millis();
        status.autoModeStatus = statusMsg;

        Serial.print("DEBUG: Target speed calculated: ");
        Serial.println(targetSpeed);
        
        return targetSpeed;
    }

public:
    explicit FanController(SystemStatus& systemStatus) : status(systemStatus) {
        initPWM();
        lastCheckTime = millis() - getCheckInterval(); // Allow immediate first check
        Serial.println("DEBUG: Fan controller initialized");
    }

    bool isInSleepMode() const {
        return inSleepMode;
    }

    void updateAutomaticMode() {
        if (!status.autoMode) return;

        try {
            float targetSpeed = calculateTargetSpeed();
            status.targetFanSpeed = targetSpeed;

            if (targetSpeed > 0.0f) {
                if (!status.fanOn) {
                    Serial.println("DEBUG: Auto mode activating fan");
                    toggleFan(true);
                }
                setFanSpeed(targetSpeed);
            } else {
                if (status.fanOn) {
                    Serial.println("DEBUG: Auto mode deactivating fan");
                    toggleFan(false);
                }
            }
            clearErrors();
        } catch (...) {
            handleError("Auto Mode Update Error");
        }
    }

    void setFanSpeed(float speed) {
        try {
            speed = constrain(speed, 0.0f, 1.0f);
            uint32_t duty = map(speed * 100, 0, 100, Config::PWM::MIN_DUTY, Config::PWM::MAX_DUTY);
            
            Serial.print("DEBUG: Setting fan speed to ");
            Serial.print(speed);
            Serial.print(" (duty: ");
            Serial.print(duty);
            Serial.println(")");
            
            ledc_set_duty(LEDC_LOW_SPEED_MODE, static_cast<ledc_channel_t>(Config::PWM::CHANNEL), duty);
            ledc_update_duty(LEDC_LOW_SPEED_MODE, static_cast<ledc_channel_t>(Config::PWM::CHANNEL));
            status.currentFanSpeed = speed;
            clearErrors();
        } catch (...) {
            handleError("Fan Speed Control Error");
        }
    }

    void toggleFan(bool on) {
        try {
            Serial.print("DEBUG: Toggling fan ");
            Serial.println(on ? "ON" : "OFF");
            
            digitalWrite(Config::Pins::MOSFET_PIN, on ? HIGH : LOW);
            status.fanOn = on;
            if (!on) {
                setFanSpeed(0.0f);
            }
            clearErrors();
        } catch (...) {
            handleError("Fan Toggle Error");
        }
    }
};

#endif // FAN_CONTROLLER_H