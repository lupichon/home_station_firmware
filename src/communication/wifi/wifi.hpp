/**
 * @file    wifi.hpp
 * @brief   WiFi communication interface for the HomeStation device.
 * @author  Lucas Pichon
 * @date    2026-07-25
 */

#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <WebServer.h>

#include "../../core/device_config.hpp"
#include "../../core/configuration_manager.hpp"
#include "../communication.hpp"
#include "wifi_config_page.hpp"

// ============================================================
// WiFiCommunication class definition
// ============================================================

class WiFiCommunication : public Communication
{
    // ── Private members ───────────────────────────────────────────────────
    private:
        String wifiApSSID;      // WiFi Access Point SSID
        String wifiApPassword;  // WiFi Access Point Password

        WebServer server; // HTTP server instance (port 80)

        DeviceConfig* deviceConfig; // Pointer to the device configuration structure

        std::function<void(const DeviceConfig&)> onConfigSaved; // Callback function to be called when configuration is saved
        std::function<String()> sensorInfoProvider;             // Callback function to provide sensor information in JSON format

        // HTTP route handlers
        void handleRoot();
        void handleGetConfig();
        void handlePostConfig();
        void handleNotFound();
        void handleGetSensors();

    // ── Public interface ──────────────────────────────────────────────────
    public:
        /**
         * @brief Constructor for WiFiCommunication.
         */
        WiFiCommunication();

        /**
         * @brief Configure the WiFi Access Point parameters before initialization.
         * @param ssid     Access Point SSID.
         * @param password Access Point password.
         */
        void configure(const String& ssid, const String& password);

        /**
         * @brief Initialize the WiFi communication.
         * @return true if initialization was successful, false otherwise.
         */
        bool begin() override;

        /**
         * @brief Main loop to handle incoming HTTP requests.
         */
        void loop();

        /**
         * @brief Not used in AP/config mode. Required by base class.
         * @return Always returns false.
         */
        bool send(uint8_t* data, size_t size) override;

        /**
         * @brief Check if there is at least one connected client to the WiFi Access Point.
         * @return true if there is at least one connected client, false otherwise.
         */
        bool hasConnectedClient() const; 

        /**
         * @brief Set the target DeviceConfig structure to be used for configuration.
         * @param config Pointer to the DeviceConfig structure.
         */
        void setConfigTarget(DeviceConfig* config);

        /**
         * @brief Set a callback function to be called when the configuration is saved.
         * @param cb Callback function that takes a const reference to DeviceConfig.
         */
        void setOnConfigSaved(std::function<void(const DeviceConfig&)> cb);

        /**
         * @brief Set a callback function to provide sensor information in JSON format.
         * @param cb Callback function that returns a String containing sensor information in JSON format.
         */
        void setSensorInfoProvider(std::function<String()> cb);
};


// ============================================================
// Implementation of WiFiCommunication methods
// ============================================================


// ─────────────────────────────────────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────────────────────────────────────

inline WiFiCommunication::WiFiCommunication()
    : server(80), 
      deviceConfig(nullptr),
      Communication()
{
}

// ─────────────────────────────────────────────────────────────────────────────
// Configuration
// ─────────────────────────────────────────────────────────────────────────────

void WiFiCommunication::configure(const String& ssid, const String& password)
{
    // Store the provided SSID and password for the WiFi Access Point
    wifiApSSID = ssid;
    wifiApPassword = password;
}

// ─────────────────────────────────────────────────────────────────────────────
// Initialization
// ─────────────────────────────────────────────────────────────────────────────

inline bool WiFiCommunication::begin()
{
    // Set the WiFi mode to Access Point
    WiFi.mode(WIFI_AP);

    // Start the WiFi Access Point with the configured SSID and password
    bool ok = WiFi.softAP(wifiApSSID.c_str(), wifiApPassword.c_str());
    if (!ok)
    {
        initialized = false;
        return false;
    }

    // Route registration
    server.on("/",           HTTP_GET,  [this]() { handleRoot();      });
    server.on("/api/config", HTTP_GET,  [this]() { handleGetConfig(); });
    server.on("/api/config", HTTP_POST, [this]() { handlePostConfig();});
    server.on("/api/sensors", HTTP_GET, [this]() { handleGetSensors(); });
    server.onNotFound(       [this]() { handleNotFound(); });

    // Start the HTTP server
    server.begin();

    initialized = true;
    return true;
}


// ─────────────────────────────────────────────────────────────────────────────
// Main loop
// ─────────────────────────────────────────────────────────────────────────────

inline void WiFiCommunication::loop()
{
    // Handle incoming HTTP requests
    server.handleClient();
}

// ─────────────────────────────────────────────────────────────────────────────
// Data Transmission
// ─────────────────────────────────────────────────────────────────────────────

inline bool WiFiCommunication::send(uint8_t* data, size_t size)
{
    return false; // Not used in AP/config mode
}

// ─────────────────────────────────────────────────────────────────────────────
// HTTP Route Handlers
// ─────────────────────────────────────────────────────────────────────────────

inline void WiFiCommunication::handleRoot()
{
    // Serve the WiFi configuration page (HTML) to the client
    server.send_P(200, "text/html", WIFI_CONFIG_PAGE);
}

inline void WiFiCommunication::handleGetConfig()
{
    // Check if the device configuration pointer is valid
    if (deviceConfig == nullptr)
    {
        server.send(
            500,
            "application/json",
            "{\"ok\":false,\"error\":\"configuration unavailable\"}"
        );
        return;
    }

    // Construct a JSON response with the current device configuration
    String json = "{";

    json += "\"utcOffset\":";
    json += String((int)deviceConfig->utcOffset);

    json += ",\"enabledSensorsMask\":";
    json += String(deviceConfig->enabledSensorsMask);

    json += ",\"enabledCommsMask\":";
    json += String(deviceConfig->enabledCommsMask);

    json += ",\"devEui\":\"";
    json += bytesToHex(deviceConfig->devEui, sizeof(deviceConfig->devEui));
    json += "\"";

    json += ",\"appEui\":\"";
    json += bytesToHex(deviceConfig->appEui, sizeof(deviceConfig->appEui));
    json += "\"";

    json += ",\"appKey\":\"";
    json += bytesToHex(deviceConfig->appKey, sizeof(deviceConfig->appKey));
    json += "\"";

    json += ",\"bleDeviceName\":\"";
    json += deviceConfig->bleDeviceName;
    json += "\"";

    json += ",\"serviceUUID\":\"";
    json += deviceConfig->serviceUUID;
    json += "\"";

    json += ",\"characteristicUUID\":\"";
    json += deviceConfig->characteristicUUID;
    json += "\"";

    json += ",\"timeSyncUUID\":\"";
    json += deviceConfig->timeSyncUUID;
    json += "\"";

    json += ",\"alarmTargetUUID\":\"";
    json += deviceConfig->alarmTargetUUID;
    json += "\"";

    json += ",\"wifiApSSID\":\"";
    json += deviceConfig->wifiApSSID;
    json += "\"";

    json += ",\"wifiApPassword\":\"";
    json += deviceConfig->wifiApPassword;
    json += "\"";

    json += "}";

    // Send the JSON response to the client
    server.send(200, "application/json", json);
}

inline void WiFiCommunication::handlePostConfig()
{
    // Check if the device configuration pointer is valid
    if (deviceConfig == nullptr)
    {
        server.send(500, "application/json", "{\"ok\":false,\"error\":\"no config target\"}");
        return;
    }

    // Read the request body containing the new configuration in JSON format
    String body = server.arg("plain");

    // Parse the JSON body using ArduinoJson
    StaticJsonDocument<1024> doc;
    if (deserializeJson(doc, body))
    {
        server.send(400, "application/json", "{\"ok\":false,\"error\":\"invalid JSON\"}");
        return;
    }

    // Extract configuration parameters from the JSON document with default values
    int8_t utcOffset               = doc["utcOffset"] | -128; 
    uint16_t enabledSensorsMask    = doc["enabledSensorsMask"] | 0xFFFF;
    uint8_t enabledCommsMask       = doc["enabledCommsMask"]   | 0x07;
    String devEui                  = doc["devEui"]             | "";
    String appEui                  = doc["appEui"]             | "";
    String appKey                  = doc["appKey"]             | "";
    String bleDeviceName           = doc["bleDeviceName"]      | "";
    String serviceUUID             = doc["serviceUUID"]        | "";
    String characteristicUUID      = doc["characteristicUUID"] | "";
    String timeSyncUUID            = doc["timeSyncUUID"]       | "";
    String alarmTargetUUID         = doc["alarmTargetUUID"]    | "";
    String wifiApSSID              = doc["wifiApSSID"]         | "";
    String wifiApPassword          = doc["wifiApPassword"]     | "";

    // Validate the extracted configuration parameters
    if (utcOffset < -12 || utcOffset > 14)
    {
        server.send(400, "application/json", "{\"ok\":false,\"error\":\"utcOffset out of range\"}");
        return;
    }

    if (!isHexString(devEui, 16))
    {
        server.send(400, "application/json", "{\"ok\":false,\"error\":\"devEui must be 16 hex characters\"}");
        return;
    }

    if (!isHexString(appEui, 16))
    {
        server.send(400, "application/json", "{\"ok\":false,\"error\":\"appEui must be 16 hex characters\"}");
        return;
    }

    if (!isHexString(appKey, 32))
    {
        server.send(400, "application/json", "{\"ok\":false,\"error\":\"appKey must be 32 hex characters\"}");
        return;
    }

    if (bleDeviceName.length() == 0)
    {
        server.send(400, "application/json", "{\"ok\":false,\"error\":\"bleDeviceName cannot be empty\"}");
        return;
    }

    if (serviceUUID.length() == 0 || characteristicUUID.length() == 0 ||
        timeSyncUUID.length() == 0 || alarmTargetUUID.length() == 0)
    {
        server.send(400, "application/json", "{\"ok\":false,\"error\":\"Bluetooth UUID fields cannot be empty\"}");
        return;
    }

    if (wifiApSSID.length() == 0)
    {
        server.send(400, "application/json", "{\"ok\":false,\"error\":\"wifiApSSID cannot be empty\"}");
        return;
    }

    if (wifiApPassword.length() == 0)
    {
        server.send(400, "application/json", "{\"ok\":false,\"error\":\"wifiApPassword cannot be empty\"}");
        return;
    }

    // Create a new DeviceConfig instance with the updated values
    DeviceConfig newDeviceConfig = *deviceConfig; 

    newDeviceConfig.utcOffset          = utcOffset;
    newDeviceConfig.bleDeviceName      = bleDeviceName;
    newDeviceConfig.serviceUUID        = serviceUUID;
    newDeviceConfig.characteristicUUID = characteristicUUID;
    newDeviceConfig.timeSyncUUID       = timeSyncUUID;
    newDeviceConfig.alarmTargetUUID    = alarmTargetUUID;
    newDeviceConfig.wifiApSSID         = wifiApSSID;
    newDeviceConfig.wifiApPassword     = wifiApPassword;
    newDeviceConfig.enabledSensorsMask = enabledSensorsMask;
    newDeviceConfig.enabledCommsMask   = enabledCommsMask;
    hexToBytes(devEui.c_str(), newDeviceConfig.devEui, sizeof(newDeviceConfig.devEui));
    hexToBytes(appEui.c_str(), newDeviceConfig.appEui, sizeof(newDeviceConfig.appEui));
    hexToBytes(appKey.c_str(), newDeviceConfig.appKey, sizeof(newDeviceConfig.appKey));

    // If a callback for configuration saving is set, call it with the new configuration
    if (onConfigSaved) onConfigSaved(newDeviceConfig);

    // Send a success response to the client and indicate that a reboot is required
    server.send(200, "application/json", "{\"ok\":true,\"rebootRequired\":true}");
    delay(300);
    ESP.restart();
}

inline void WiFiCommunication::handleNotFound()
{
    // Handle requests to unknown routes by sending a 404 Not Found response
    server.send(404, "text/plain", "Not found");
}

inline void WiFiCommunication::handleGetSensors()
{
    // Check if a sensor information provider callback is set
    if (!sensorInfoProvider)
    {
        server.send(200, "application/json", "[]");
        return;
    }
    server.send(200, "application/json", sensorInfoProvider());
}

inline bool WiFiCommunication::hasConnectedClient() const 
{ 
    // Check if the WiFi communication is initialized and if there is at least one connected client to the WiFi Access Point
    if (!initialized)
    {
        return false;
    }
    return WiFi.softAPgetStationNum() > 0;
}

inline void WiFiCommunication::setSensorInfoProvider(std::function<String()> cb)
{
    // Set the callback function to provide sensor information in JSON format
    sensorInfoProvider = cb;
}

inline void WiFiCommunication::setConfigTarget(DeviceConfig* target)
{
    // Set the pointer to the target DeviceConfig structure for configuration
    deviceConfig = target;
}

inline void WiFiCommunication::setOnConfigSaved(std::function<void(const DeviceConfig&)> cb)
{
    // Set the callback function to be called when the configuration is saved
    onConfigSaved = cb;
}