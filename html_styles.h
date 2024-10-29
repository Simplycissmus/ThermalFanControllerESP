#ifndef HTML_STYLES_H
#define HTML_STYLES_H

#include <Arduino.h>

const char HTML_STYLES[] PROGMEM = R"rawliteral(
<style>
    /* Base Styles */
    body { 
        font-family: Arial, sans-serif; 
        background-color: #f5f5f5; 
        margin: 0; 
        padding: 20px; 
        color: #333;
    }
    .container { 
        max-width: 800px; 
        margin: auto; 
        background: #ffffff; 
        border-radius: 8px; 
        box-shadow: 0 0 10px rgba(0,0,0,0.15); 
        overflow: hidden; 
    }
    h1 { 
        text-align: center; 
        background: #007BFF; 
        color: white; 
        margin: 0; 
        padding: 20px; 
    }
    h3 {
        color: #007BFF;
        margin: 15px 0;
        font-size: 1.2em;
    }

    /* Card Styles */
    .card { 
        margin: 20px; 
        padding: 20px; 
        background: #f9f9f9; 
        border-radius: 8px; 
        box-shadow: 0 2px 4px rgba(0,0,0,0.1); 
    }

    /* Controls Section */
    .controls { 
        display: flex; 
        justify-content: space-between; 
        align-items: center; 
        gap: 20px;
        margin-bottom: 20px;
        padding: 15px;
        background: #ffffff;
        border-radius: 8px;
    }

    /* Mode Switch */
    .mode-switch {
        display: flex;
        align-items: center;
        gap: 10px;
    }
    .switch {
        position: relative;
        display: inline-block;
        width: 60px;
        height: 34px;
        margin-right: 10px;
    }
    .switch input {
        opacity: 0;
        width: 0;
        height: 0;
    }
    .slider {
        position: absolute;
        cursor: pointer;
        top: 0;
        left: 0;
        right: 0;
        bottom: 0;
        background-color: #ccc;
        transition: .4s;
    }
    .slider.round {
        border-radius: 34px;
    }
    .slider.round:before {
        border-radius: 50%;
    }
    .slider:before {
        position: absolute;
        content: "";
        height: 26px;
        width: 26px;
        left: 4px;
        bottom: 4px;
        background-color: white;
        transition: .4s;
    }
    input:checked + .slider {
        background-color: #007BFF;
    }
    input:checked + .slider:before {
        transform: translateX(26px);
    }

    /* Fan Status */
    .fan-status {
        display: flex;
        align-items: center;
        gap: 8px;
        padding: 5px 15px;
        background: #f8f9fa;
        border-radius: 20px;
    }
    .status-indicator { 
        display: inline-block; 
        width: 12px; 
        height: 12px; 
        border-radius: 50%; 
        transition: all 0.3s ease; 
    }
    .status-on { 
        background: #28a745; 
        box-shadow: 0 0 8px #28a745; 
    }
    .status-off { 
        background: #dc3545; 
        box-shadow: 0 0 8px #dc3545; 
    }

    /* Manual Speed Control */
    .manual-speed-control {
        background: #ffffff;
        padding: 15px;
        border-radius: 8px;
        margin: 20px 0;
        display: none;
    }
    .manual-speed-control.visible {
        display: block;
    }

    /* Speed Slider */
    .speed-slider {
        -webkit-appearance: none;
        width: 100%;
        height: 8px;
        border-radius: 4px;
        background: #ddd;
        outline: none;
        margin: 10px 0;
    }
    .speed-slider::-webkit-slider-thumb {
        -webkit-appearance: none;
        width: 20px;
        height: 20px;
        border-radius: 50%;
        background: #007BFF;
        cursor: pointer;
        border: 2px solid #fff;
        box-shadow: 0 0 4px rgba(0,0,0,0.2);
    }
    .speed-slider::-moz-range-thumb {
        width: 20px;
        height: 20px;
        border-radius: 50%;
        background: #007BFF;
        cursor: pointer;
        border: 2px solid #fff;
        box-shadow: 0 0 4px rgba(0,0,0,0.2);
    }

    /* Buttons */
    .btn { 
        padding: 10px 20px; 
        background: #007BFF; 
        color: #fff; 
        border: none; 
        border-radius: 4px; 
        cursor: pointer; 
        transition: background 0.3s;
        font-size: 0.95em;
        min-width: 120px;
        text-align: center;
    }
    .btn:hover { 
        background: #0056b3; 
    }
    .reset-btn {
        background: #6c757d;
        color: white;
        border: none;
        border-radius: 4px;
        padding: 6px 12px;
        font-size: 0.9em;
        cursor: pointer;
        margin-top: 8px;
        transition: background 0.3s;
    }
    .reset-btn:hover {
        background: #5a6268;
    }

    /* Metrics Section */
    .metrics { 
        display: grid; 
        grid-template-columns: repeat(auto-fit, minmax(200px, 1fr)); 
        gap: 15px; 
        margin: 20px 0; 
    }
    .metric-item { 
        background: #fff; 
        padding: 15px; 
        border-radius: 8px; 
        text-align: center;
        transition: transform 0.2s ease;
    }
    .metric-item:hover {
        transform: translateY(-2px);
    }
    .metric-header {
        display: flex;
        justify-content: center;
        align-items: center;
        gap: 10px;
        margin-bottom: 5px;
    }
    .metric-value { 
        font-size: 1.8em; 
        font-weight: bold; 
        color: #007BFF; 
        margin: 10px 0; 
    }
    .metric-label { 
        color: #666; 
        font-size: 0.9em; 
    }

    /* Temperature Trend */
    .trend-indicator {
        width: 0;
        height: 0;
        border-left: 6px solid transparent;
        border-right: 6px solid transparent;
        transition: all 0.3s ease;
    }
    .trend-indicator.rising {
        border-bottom: 8px solid #28a745;
        transform: translateY(-2px);
    }
    .trend-indicator.falling {
        border-top: 8px solid #dc3545;
        transform: translateY(2px);
    }
    .trend-indicator.stable {
        border: 4px solid #ffc107;
        border-radius: 50%;
    }

    /* Fan Power Display */
    .fan-info {
        background: #ffffff;
        padding: 15px;
        border-radius: 8px;
        margin: 20px 0;
    }
    .progress-bar { 
        width: 100%; 
        height: 12px; 
        background: #ddd; 
        border-radius: 6px; 
        overflow: hidden; 
        margin: 10px 0; 
    }
    .progress-fill { 
        height: 100%; 
        background: #007BFF; 
        transition: width 0.3s ease; 
    }
    .rpm-display {
        text-align: right;
        color: #666;
        margin-top: 10px;
        font-size: 0.9em;
    }

    /* Heat Recovery Stats */
    .heat-stats {
        background: #ffffff;
        padding: 20px;
        border-radius: 8px;
        margin: 20px 0;
    }
    .stats-grid {
        display: grid;
        grid-template-columns: repeat(auto-fit, minmax(150px, 1fr));
        gap: 15px;
        margin-top: 15px;
    }
    .stat-item {
        background: #f8f9fa;
        padding: 15px;
        border-radius: 6px;
        text-align: center;
        transition: transform 0.2s ease;
    }
    .stat-item:hover {
        transform: translateY(-2px);
    }
    .stat-label {
        color: #666;
        font-size: 0.9em;
        margin-bottom: 8px;
    }
    .stat-value {
        font-size: 1.2em;
        font-weight: bold;
        color: #007BFF;
    }

    /* System Statistics */
    .statistics {
        background: #ffffff;
        padding: 20px;
        border-radius: 8px;
        margin-top: 20px;
    }

    /* Status Messages */
    .status-message {
        margin-top: 20px;
        padding: 12px;
        background: #fff;
        border-radius: 8px;
        text-align: center;
        color: #666;
        font-size: 0.95em;
    }

    /* Responsive Design */
    @media (max-width: 600px) {
        body {
            padding: 10px;
        }
        .container {
            margin: 0;
            border-radius: 0;
        }
        .card {
            margin: 10px;
            padding: 15px;
        }
        .controls {
            flex-direction: column;
            align-items: stretch;
        }
        .stats-grid {
            grid-template-columns: repeat(auto-fit, minmax(120px, 1fr));
        }
        .metric-value {
            font-size: 1.5em;
        }
        .stat-value {
            font-size: 1.1em;
        }
        .btn {
            width: 100%;
        }
    }

    /* Dark Mode Support */
    @media (prefers-color-scheme: dark) {
        body {
            background-color: #1a1a1a;
            color: #f5f5f5;
        }
        .container {
            background: #2d2d2d;
        }
        .card {
            background: #333;
        }
        .controls,
        .fan-info,
        .heat-stats,
        .statistics,
        .status-message,
        .metric-item {
            background: #2d2d2d;
        }
        .stat-item {
            background: #333;
        }
        .metric-value,
        .stat-value {
            color: #5c9eff;
        }
        .metric-label,
        .stat-label {
            color: #aaa;
        }
    }
</style>
)rawliteral";

#endif // HTML_STYLES_H