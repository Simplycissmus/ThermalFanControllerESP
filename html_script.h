#ifndef HTML_SCRIPT_H
#define HTML_SCRIPT_H

#include <Arduino.h>

const char HTML_SCRIPT[] PROGMEM = R"rawliteral(
<script>
    const globalState = {
        lastTemperature: null,
        lastHumidity: null,
        lastUpdateTime: null,
        errorCount: 0,
        maxErrors: 5,
        updateInterval: null,
        temperatureTrend: 'stable',
        manualSpeed: 0,
        retryDelay: 2000,
        maxRetries: 3,
        currentRetry: 0
    };

    // Helper Functions
    function safeFixed(value, decimals) {
        return (typeof value === 'number' && !isNaN(value)) ? value.toFixed(decimals) : 'N/A';
    }

    async function fetchWithRetry(url, options = {}) {
        let lastError;
        for (let i = 0; i <= globalState.maxRetries; i++) {
            try {
                const response = await fetch(url, options);
                if (!response.ok) throw new Error(`HTTP error! status: ${response.status}`);
                globalState.currentRetry = 0; // Reset on success
                return response;
            } catch (error) {
                lastError = error;
                console.warn(`Attempt ${i + 1} failed:`, error);
                if (i < globalState.maxRetries) {
                    await new Promise(resolve => setTimeout(resolve, globalState.retryDelay));
                }
            }
        }
        throw lastError;
    }

    // Mode Control Functions
    async function setAutoMode(isAuto) {
        try {
            const response = await fetchWithRetry('/api/v1/fan/mode', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/x-www-form-urlencoded',
                },
                body: `mode=${isAuto ? '1' : '0'}`
            });

            if (!response.ok) throw new Error('Failed to update mode');

            updateModeUI(isAuto);
            await fetchAndUpdateStatus();
        } catch (error) {
            console.error('Error setting mode:', error);
            handleUpdateError(error.message);
        }
    }

    function updateModeUI(isAuto) {
        document.getElementById('mode-label').textContent = isAuto ? 'Automatic' : 'Manual';
        document.getElementById('manual-speed-control').style.display = isAuto ? 'none' : 'block';
        document.getElementById('fan-toggle').style.display = isAuto ? 'none' : 'block';
    }

    // Fan Control Functions
    async function toggleFan() {
        try {
            const response = await fetchWithRetry('/api/v1/fan/toggle', {
                method: 'POST'
            });

            if (!response.ok) throw new Error('Failed to toggle fan');
            await fetchAndUpdateStatus();
        } catch (error) {
            console.error('Error toggling fan:', error);
            handleUpdateError(error.message);
        }
    }

    async function updateManualSpeed(value) {
        const speed = value / 100;
        document.getElementById('manual-speed').textContent = `${value}%`;

        try {
            await fetchWithRetry('/api/v1/fan/speed', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/x-www-form-urlencoded',
                },
                body: `speed=${speed}`
            });
        } catch (error) {
            console.error('Error updating speed:', error);
            handleUpdateError(error.message);
        }
    }

    async function resetTemperatureRanges() {
        try {
            const response = await fetchWithRetry('/api/v1/temperature/reset', {
                method: 'POST'
            });

            if (!response.ok) throw new Error('Failed to reset temperature ranges');
            await fetchAndUpdateStatus();
        } catch (error) {
            console.error('Error resetting temperature ranges:', error);
            handleUpdateError(error.message);
        }
    }

    // UI Update Functions
    function updateUI(data) {
        if (!data) return;

        try {
            updateSensorReadings(data);
            updateFanControls(data);
            updateOperationMode(data);
            updateHeatRecoveryStats(data);
            updateSystemStats(data);
            updateStatusMessages(data);

            globalState.lastUpdateTime = new Date();
            globalState.errorCount = 0;
        } catch (error) {
            console.error('Error in updateUI:', error);
            handleUpdateError('UI update failed: ' + error.message);
        }
    }

    function updateSensorReadings(data) {
        document.getElementById('temp-value').textContent = safeFixed(data.temperature, 1) + '°C';
        document.getElementById('temp-min').textContent = 'Min: ' + safeFixed(data.min_temperature, 1) + '°C';
        document.getElementById('temp-max').textContent = 'Max: ' + safeFixed(data.max_temperature, 1) + '°C';
        document.getElementById('humidity-value').textContent = safeFixed(data.humidity, 1) + '%';
        
        updateTemperatureTrend(data.temperature);
    }

    function updateTemperatureTrend(currentTemp) {
        const trendIndicator = document.getElementById('temp-trend');
        if (globalState.lastTemperature !== null) {
            if (currentTemp > globalState.lastTemperature + 0.1) {
                trendIndicator.className = 'trend-indicator rising';
            } else if (currentTemp < globalState.lastTemperature - 0.1) {
                trendIndicator.className = 'trend-indicator falling';
            } else {
                trendIndicator.className = 'trend-indicator stable';
            }
        }
        globalState.lastTemperature = currentTemp;
    }

    function updateFanControls(data) {
        // Fan status indicator
        const fanStatus = document.getElementById('fan-status');
        fanStatus.className = 'status-indicator ' + (data.fan_on ? 'status-on' : 'status-off');

        // Fan toggle button
        const fanToggle = document.getElementById('fan-toggle');
        const fanButtonText = document.getElementById('fan-button-text');
        fanToggle.style.display = data.auto_mode ? 'none' : 'block';
        fanButtonText.textContent = data.fan_on ? 'Turn Fan Off' : 'Turn Fan On';

        // Speed controls
        const speedControl = document.getElementById('manual-speed-control');
        speedControl.style.display = data.auto_mode ? 'none' : 'block';

        // Update speed displays
        updateSpeedDisplays(data);
    }

    function updateSpeedDisplays(data) {
        const progressFill = document.querySelector('.progress-fill');
        const autoSpeed = document.getElementById('auto-speed');
        const speed = data.current_fan_speed * 100;
        
        progressFill.style.width = `${speed}%`;
        autoSpeed.textContent = `${safeFixed(speed, 1)}%`;

        if (!data.auto_mode) {
            const speedSlider = document.getElementById('speed-slider');
            const manualSpeedDisplay = document.getElementById('manual-speed');
            speedSlider.value = data.manual_fan_speed * 100;
            manualSpeedDisplay.textContent = `${safeFixed(data.manual_fan_speed * 100, 1)}%`;
        }

        document.getElementById('fan-rpm').textContent = `${Math.round(data.fan_rpm)} RPM`;
    }

    function updateOperationMode(data) {
        const modeToggle = document.getElementById('mode-toggle');
        const modeLabel = document.getElementById('mode-label');
        const fanMode = document.getElementById('fan-mode');

        modeToggle.checked = data.auto_mode;
        modeLabel.textContent = data.auto_mode ? 'Automatic' : 'Manual';
        fanMode.textContent = data.auto_mode_status;
    }

    function updateHeatRecoveryStats(data) {
        if (data.heat_calc_active) {
            document.getElementById('current-heat-power').textContent = safeFixed(data.current_heat_power, 1) + ' W';
            document.getElementById('total-heat-energy').textContent = safeFixed(data.total_heat_energy, 3) + ' kWh';
            document.getElementById('avg-heat-power').textContent = safeFixed(data.avg_heat_power, 1) + ' W';
            document.getElementById('air-volume').textContent = safeFixed(data.air_volume_moved, 1) + ' m³';
        } else {
            document.querySelectorAll('.heat-stats .stat-value').forEach(el => {
                el.textContent = 'N/A';
            });
        }
    }

    function updateSystemStats(data) {
        document.getElementById('total-runtime').textContent = safeFixed(data.total_operating_time / 3600, 1) + ' Hours';
        document.getElementById('fan-runtime').textContent = safeFixed(data.fan_operating_time / 3600, 1) + ' Hours';
        document.getElementById('energy-usage').textContent = safeFixed(data.energy_usage, 1) + ' Wh';
    }

    function updateStatusMessages(data) {
        const autoStatus = document.getElementById('auto-status');
        autoStatus.textContent = data.auto_mode_status || 'System Ready';
        autoStatus.style.color = data.error_state === 'OK' ? '' : '#dc3545';
    }

    // Error Handling
    function handleUpdateError(message) {
        console.error('Update error:', message);
        globalState.errorCount++;
        const statusElement = document.getElementById('auto-status');
        
        if (globalState.errorCount >= globalState.maxErrors) {
            statusElement.textContent = 'Error: ' + message;
            statusElement.style.color = '#dc3545';
        }

        // Increase retry delay for subsequent attempts
        globalState.retryDelay = Math.min(globalState.retryDelay * 1.5, 10000);
    }

    async function fetchAndUpdateStatus() {
        try {
            const response = await fetchWithRetry('/api/v1/status');
            const data = await response.json();
            updateUI(data);
            globalState.retryDelay = 2000; // Reset retry delay on success
        } catch (error) {
            handleUpdateError('Failed to fetch status: ' + error.message);
        }
    }

    function initializeUpdates() {
        if (globalState.updateInterval) clearInterval(globalState.updateInterval);
        globalState.updateInterval = setInterval(fetchAndUpdateStatus, 2000);
        fetchAndUpdateStatus();
    }

    // Event Listeners
    window.addEventListener('load', initializeUpdates);
    window.addEventListener('focus', () => {
        fetchAndUpdateStatus(); // Immediate update when tab becomes active
    });
</script>
)rawliteral";

#endif // HTML_SCRIPT_H