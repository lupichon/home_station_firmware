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
// WiFiCommunication class
// ============================================================

class WiFiCommunication : public Communication
{
    private:
        String wifiApSSID;
        String wifiApPassword;
        WebServer server;

        DeviceConfig* deviceConfig;

        std::function<void(const DeviceConfig&)> onConfigSaved;
        std::function<String()> sensorInfoProvider;

        void handleRoot();
        void handleGetConfig();
        void handlePostConfig();
        void handleNotFound();
        void handleGetSensors();

    public:
        WiFiCommunication();
        void configure(const String& ssid, const String& password);
        bool begin() override;
        void loop();
        bool send(uint8_t* data, size_t size) override;
        bool hasConnectedClient() const; 
        void setConfigTarget(DeviceConfig* config);
        void setOnConfigSaved(std::function<void(const DeviceConfig&)> cb);
        void setSensorInfoProvider(std::function<String()> cb);
};


// ============================================================
// Constructor
// ============================================================

inline WiFiCommunication::WiFiCommunication()
    : server(80), 
      deviceConfig(nullptr),
      Communication()
{
}

void WiFiCommunication::configure(const String& ssid, const String& password)
{
    wifiApSSID = ssid;
    wifiApPassword = password;
}

// ============================================================
// Initialization
// ============================================================

inline bool WiFiCommunication::begin()
{
    WiFi.mode(WIFI_AP);

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

    server.begin();

    initialized = true;
    return true;
}


// ============================================================
// Loop (call every iteration)
// ============================================================

inline void WiFiCommunication::loop()
{
    if (!initialized) return;
    server.handleClient();
}


// ============================================================
// send() — not used for AP/config mode, required by base class
// ============================================================

inline bool WiFiCommunication::send(uint8_t* data, size_t size)
{
    return false;
}

inline void WiFiCommunication::setConfigTarget(DeviceConfig* target)
{
    deviceConfig = target;
}

inline void WiFiCommunication::setOnConfigSaved(std::function<void(const DeviceConfig&)> cb)
{
    onConfigSaved = cb;
}

// ============================================================
// HTTP handlers
// ============================================================

inline void WiFiCommunication::handleRoot()
{
    server.send_P(200, "text/html", WIFI_CONFIG_PAGE);
}

inline void WiFiCommunication::handleGetConfig()
{
    if (deviceConfig == nullptr)
    {
        server.send(
            500,
            "application/json",
            "{\"ok\":false,\"error\":\"configuration unavailable\"}"
        );
        return;
    }

    String json = "{";

    json += "\"utcOffset\":";
    json += String((int)deviceConfig->utcOffset);

    json += ",\"enabledSensorsMask\":";
    json += String(deviceConfig->enabledSensorsMask);

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

    server.send(200, "application/json", json);
}

inline void WiFiCommunication::handlePostConfig()
{
    if (deviceConfig == nullptr)
    {
        server.send(500, "application/json", "{\"ok\":false,\"error\":\"no config target\"}");
        return;
    }

    String body = server.arg("plain");

    StaticJsonDocument<1024> doc;
    if (deserializeJson(doc, body))
    {
        server.send(400, "application/json", "{\"ok\":false,\"error\":\"invalid JSON\"}");
        return;
    }

    int8_t utcOffset               = doc["utcOffset"] | -128; 
    uint16_t enabledSensorsMask    = doc["enabledSensorsMask"] | 0xFFFF;
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

    DeviceConfig newDeviceConfig = *deviceConfig; 

    // Écriture dans deviceConfig
    newDeviceConfig.utcOffset          = utcOffset;
    newDeviceConfig.bleDeviceName      = bleDeviceName;
    newDeviceConfig.serviceUUID        = serviceUUID;
    newDeviceConfig.characteristicUUID = characteristicUUID;
    newDeviceConfig.timeSyncUUID       = timeSyncUUID;
    newDeviceConfig.alarmTargetUUID    = alarmTargetUUID;
    newDeviceConfig.wifiApSSID         = wifiApSSID;
    newDeviceConfig.wifiApPassword     = wifiApPassword;
    newDeviceConfig.enabledSensorsMask = enabledSensorsMask;
    hexToBytes(devEui.c_str(), newDeviceConfig.devEui, sizeof(newDeviceConfig.devEui));
    hexToBytes(appEui.c_str(), newDeviceConfig.appEui, sizeof(newDeviceConfig.appEui));
    hexToBytes(appKey.c_str(), newDeviceConfig.appKey, sizeof(newDeviceConfig.appKey));

    // Callback → persist en NVS (dans main.cpp)
    if (onConfigSaved) onConfigSaved(newDeviceConfig);

    // Répondre AVANT de rebooter
    server.send(200, "application/json", "{\"ok\":true,\"rebootRequired\":true}");
    delay(300);
    ESP.restart();
}


inline void WiFiCommunication::handleNotFound()
{
    server.send(404, "text/plain", "Not found");
}

inline bool WiFiCommunication::hasConnectedClient() const 
{ 
    return WiFi.softAPgetStationNum() > 0;
}

inline void WiFiCommunication::setSensorInfoProvider(std::function<String()> cb)
{
    sensorInfoProvider = cb;
}

inline void WiFiCommunication::handleGetSensors()
{
    if (!sensorInfoProvider)
    {
        server.send(200, "application/json", "[]");
        return;
    }
    server.send(200, "application/json", sensorInfoProvider());
}