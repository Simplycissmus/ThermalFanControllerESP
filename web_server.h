#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <WebServer.h>
#include "config.h"
#include "system_status.h"
#include "fan_controller.h"
#include "html_content.h"

class WebServerManager {
private:
    WebServer server;
    SystemStatus& status;
    FanController& controller;

    void setupRoutes() {
        // Root and API routes with debug output
        server.on("/", HTTP_GET, [this]() { 
            Serial.println("DEBUG: Serving root page");
            handleRoot(); 
        });

        server.on("/api/v1/status", HTTP_GET, [this]() { 
            Serial.println("DEBUG: Status request received");
            handleGetData(); 
        });

        server.on("/api/v1/fan/toggle", HTTP_POST, [this]() { 
            Serial.println("DEBUG: Fan toggle request received");
            handleToggleFan(); 
        });

        server.on("/api/v1/fan/mode", HTTP_POST, [this]() { 
            Serial.println("DEBUG: Mode change request received");
            handleSetAutoMode(); 
        });

        server.on("/api/v1/fan/speed", HTTP_POST, [this]() { 
            Serial.println("DEBUG: Speed change request received");
            handleSetFanSpeed(); 
        });

        server.on("/api/v1/temperature/reset", HTTP_POST, [this]() { 
            Serial.println("DEBUG: Temperature reset request received");
            handleResetTemperature(); 
        });

        // CORS Options handling
        server.on("/api/v1/status", HTTP_OPTIONS, [this]() { handleCORS(); });
        server.on("/api/v1/fan/toggle", HTTP_OPTIONS, [this]() { handleCORS(); });
        server.on("/api/v1/fan/mode", HTTP_OPTIONS, [this]() { handleCORS(); });
        server.on("/api/v1/fan/speed", HTTP_OPTIONS, [this]() { handleCORS(); });
        server.on("/api/v1/temperature/reset", HTTP_OPTIONS, [this]() { handleCORS(); });

        // 404 Handler
        server.onNotFound([this]() {
            Serial.println("DEBUG: 404 - Not Found");
            handleNotFound();
        });
    }

    void handleCORS() {
        server.sendHeader("Access-Control-Allow-Origin", "*");
        server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
        server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
        server.send(204);
    }

    void handleRoot() {
        Serial.println("DEBUG: Building HTML content");
        server.send(200, "text/html", buildHtmlContent());
        Serial.println("DEBUG: Root page sent");
    }

    void handleGetData() {
        Serial.println("DEBUG: Preparing status data");
        
        // Add CORS and cache control headers
        server.sendHeader("Access-Control-Allow-Origin", "*");
        server.sendHeader("Access-Control-Allow-Methods", "GET");
        server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
        server.sendHeader("Pragma", "no-cache");
        server.sendHeader("Expires", "-1");

        // Generate and send JSON
        String jsonData = status.toJson();
        Serial.print("DEBUG: Sending JSON data: ");
        Serial.println(jsonData);
        server.send(200, "application/json", jsonData);
    }

    void handleToggleFan() {
        Serial.println("DEBUG: Processing fan toggle request");
        if (!validatePostRequest()) return;

        if (status.autoMode) {
            sendError(400, "Cannot toggle fan in automatic mode");
            return;
        }

        status.fanOn = !status.fanOn;
        controller.toggleFan(status.fanOn);

        if (status.fanOn) {
            controller.setFanSpeed(status.manualFanSpeed);
        } else {
            controller.setFanSpeed(0.0f);
        }

        sendSuccess("Fan state toggled successfully");
    }

    void handleSetAutoMode() {
        Serial.println("DEBUG: Processing auto mode change request");
        if (!validatePostRequest()) return;

        if (!server.hasArg("mode")) {
            Serial.println("DEBUG: Missing 'mode' parameter");
            sendError(400, "Missing 'mode' parameter");
            return;
        }

        String mode = server.arg("mode");
        Serial.print("DEBUG: Requested mode: ");
        Serial.println(mode);

        bool newMode = (mode == "1" || mode.equalsIgnoreCase("true"));
        
        if (status.setAutoMode(newMode)) {
            if (newMode) {
                controller.updateAutomaticMode();
            } else if (status.fanOn) {
                controller.setFanSpeed(status.manualFanSpeed);
            }
            sendSuccess("Mode updated successfully");
        } else {
            sendError(400, "Failed to update mode");
        }
    }

    void handleSetFanSpeed() {
        Serial.println("DEBUG: Processing fan speed change request");
        if (!validatePostRequest()) return;

        if (status.autoMode) {
            Serial.println("DEBUG: Cannot set fan speed in automatic mode");
            sendError(400, "Cannot set fan speed in automatic mode");
            return;
        }

        if (!server.hasArg("speed")) {
            Serial.println("DEBUG: Missing 'speed' parameter");
            sendError(400, "Missing 'speed' parameter");
            return;
        }

        String speedStr = server.arg("speed");
        Serial.print("DEBUG: Requested speed: ");
        Serial.println(speedStr);

        float speed = speedStr.toFloat();
        if (isnan(speed) || speed < 0.0f || speed > 1.0f) {
            Serial.println("DEBUG: Invalid speed value");
            sendError(400, "Invalid speed value");
            return;
        }

        status.manualFanSpeed = speed;
        if (status.fanOn) {
            controller.setFanSpeed(speed);
        }

        sendSuccess("Speed updated successfully");
    }

    void handleResetTemperature() {
        Serial.println("DEBUG: Processing temperature reset request");
        if (!validatePostRequest()) return;
        
        status.resetMinMaxTemperature();
        sendSuccess("Temperature ranges reset successfully");
    }

    void handleNotFound() {
        Serial.println("DEBUG: Handling 404 Not Found");
        String message = "File Not Found\n\n";
        message += "URI: ";
        message += server.uri();
        message += "\nMethod: ";
        message += (server.method() == HTTP_GET) ? "GET" : "POST";
        message += "\nArguments: ";
        message += server.args();
        message += "\n";
        for (uint8_t i = 0; i < server.args(); i++) {
            message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
        }
        server.send(404, "text/plain", message);
    }

    bool validatePostRequest() {
        if (server.method() != HTTP_POST) {
            Serial.println("DEBUG: Invalid method - expecting POST");
            sendError(405, "Method Not Allowed");
            return false;
        }
        return true;
    }

    void sendError(int code, const String& message) {
        Serial.print("DEBUG: Sending error response: ");
        Serial.println(message);
        
        server.sendHeader("Access-Control-Allow-Origin", "*");
        String json = "{\"error\":\"" + message + "\"}";
        server.send(code, "application/json", json);
    }

    void sendSuccess(const String& message) {
        Serial.print("DEBUG: Sending success response: ");
        Serial.println(message);
        
        server.sendHeader("Access-Control-Allow-Origin", "*");
        String json = "{\"success\":\"" + message + "\"}";
        server.send(200, "application/json", json);
    }

public:
    WebServerManager(SystemStatus& systemStatus, FanController& fanController)
        : server(Config::WebServer::PORT), status(systemStatus), controller(fanController)
    {
        setupRoutes();
    }

    void begin() {
        server.begin();
        Serial.println("DEBUG: Web server initialized on port " + String(Config::WebServer::PORT));
        Serial.println("DEBUG: Server IP address: " + WiFi.localIP().toString());
    }

    void handle() {
        server.handleClient();
    }
};

#endif // WEB_SERVER_H