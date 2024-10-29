#ifndef HTML_CONTENT_H
#define HTML_CONTENT_H

#include "html_styles.h"
#include "html_script.h"

const char HTML_CONTENT[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Fireplace Fan Control</title>
)rawliteral";

const char HTML_BODY[] PROGMEM = R"rawliteral(
</head>
<body>
    <div class="container" id="app">
        <h1>Fireplace Fan Control</h1>
        <div class="card">
            <!-- Controls Section -->
            <div class="controls">
                <div class="mode-switch">
                    <label class="switch">
                        <input type="checkbox" id="mode-toggle" onchange="setAutoMode(this.checked)">
                        <span class="slider round"></span>
                    </label>
                    <span id="mode-label">Manual</span>
                </div>
                
                <div class="fan-status">
                    <div class="status-indicator" id="fan-status"></div>
                    <span>Fan</span>
                </div>

                <button class="btn" id="fan-toggle" onclick="toggleFan()">
                    <span id="fan-button-text">Turn Fan On</span>
                </button>
            </div>

            <!-- Manual Speed Control -->
            <div id="manual-speed-control" class="slider-container">
                <div class="slider-label">
                    <span>Manual Speed</span>
                    <span id="manual-speed">0%</span>
                </div>
                <input type="range" min="0" max="100" value="0" 
                    class="speed-slider" id="speed-slider"
                    oninput="updateManualSpeed(this.value)">
            </div>

            <!-- Temperature and Humidity -->
            <div class="metrics">
                <div class="metric-item">
                    <div class="metric-header">
                        <div class="metric-label">Temperature</div>
                        <div id="temp-trend" class="trend-indicator"></div>
                    </div>
                    <div class="metric-value" id="temp-value">-°C</div>
                    <div class="metric-minmax">
                        <span class="min" id="temp-min">Min: -°C</span>
                        <span class="max" id="temp-max">Max: -°C</span>
                    </div>
                    <button class="reset-btn" onclick="resetTemperatureRanges()">Reset Ranges</button>
                </div>
                <div class="metric-item">
                    <div class="metric-label">Humidity</div>
                    <div class="metric-value" id="humidity-value">-%</div>
                </div>
            </div>

            <!-- Mode Status -->
            <div class="mode-status">
                <div class="mode-label">Current Mode:</div>
                <div id="fan-mode" class="mode-value">Initializing...</div>
            </div>

            <!-- Fan Power Display -->
            <div class="fan-info">
                <div>
                    <div class="slider-label">
                        <span>Fan Power</span>
                        <span id="auto-speed">0%</span>
                    </div>
                    <div class="progress-bar">
                        <div class="progress-fill" style="width: 0%"></div>
                    </div>
                </div>
                <div class="rpm-display">
                    <span>Speed:</span>
                    <span id="fan-rpm">0 RPM</span>
                </div>
            </div>

            <!-- Heat Recovery Stats -->
            <div class="heat-stats">
                <h3>Heat Recovery Statistics</h3>
                <div class="stats-grid">
                    <div class="stat-item">
                        <div class="stat-label">Current Power</div>
                        <div class="stat-value" id="current-heat-power">0 W</div>
                    </div>
                    <div class="stat-item">
                        <div class="stat-label">Total Energy Recovered</div>
                        <div class="stat-value" id="total-heat-energy">0 kWh</div>
                    </div>
                    <div class="stat-item">
                        <div class="stat-label">Average Power</div>
                        <div class="stat-value" id="avg-heat-power">0 W</div>
                    </div>
                    <div class="stat-item">
                        <div class="stat-label">Air Moved</div>
                        <div class="stat-value" id="air-volume">0 m³</div>
                    </div>
                </div>
            </div>

            <!-- System Statistics -->
            <div class="statistics">
                <h3>System Statistics</h3>
                <div class="stats-grid">
                    <div class="stat-item">
                        <div class="stat-label">Total Runtime</div>
                        <div class="stat-value" id="total-runtime">0 Hours</div>
                    </div>
                    <div class="stat-item">
                        <div class="stat-label">Fan Runtime</div>
                        <div class="stat-value" id="fan-runtime">0 Hours</div>
                    </div>
                    <div class="stat-item">
                        <div class="stat-label">Energy Usage</div>
                        <div class="stat-value" id="energy-usage">0 Wh</div>
                    </div>
                </div>
            </div>

            <!-- Status Message -->
            <div class="status-message" id="auto-status">System Ready</div>
        </div>
    </div>
</body>
</html>
)rawliteral";

String buildHtmlContent() {
    String content = FPSTR(HTML_CONTENT);
    content += FPSTR(HTML_STYLES);
    content += FPSTR(HTML_SCRIPT);
    content += FPSTR(HTML_BODY);
    return content;
}

#endif // HTML_CONTENT_H