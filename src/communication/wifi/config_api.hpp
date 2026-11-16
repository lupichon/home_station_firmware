/**
 * @file    config_api.hpp
 * @brief   HTTP routes to read and write the device configuration.
 * @author  Lucas Pichon
 * @date    2026-11-16
 */

#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <WebServer.h>
#include <functional>

#include "../../core/configuration/device_config.hpp"
#include "../../core/configuration/config_json.hpp"
#include "../../core/firmware_version.hpp"


// ============================================================
// ConfigApi class definition
// ============================================================

class ConfigApi
{
    // ── Private members ───────────────────────────────────────────────────
    private:
        DeviceConfig& config;  // Live configuration, read by GET and used as the base of POST

        std::function<void(const DeviceConfig&)> onSaved;  // Called with the validated configuration

        void handleGet(WebServer& server);
        void handlePost(WebServer& server);

    // ── Public interface ────────────────────────────────────────────────────
    public:

        explicit ConfigApi(DeviceConfig& config);

        /**
         * @brief Set the callback that persists and applies a validated configuration.
         */
        void setOnSaved(std::function<void(const DeviceConfig&)> cb);

        /**
         * @brief Register GET and POST /api/config on the server.
         */
        void install(WebServer& server);
};


// ============================================================
// Implementations of ConfigApi methods
// ============================================================

inline ConfigApi::ConfigApi(DeviceConfig& config)
    : config(config)
{
}

inline void ConfigApi::setOnSaved(std::function<void(const DeviceConfig&)> cb)
{
    onSaved = cb;
}

inline void ConfigApi::install(WebServer& server)
{
    server.on("/api/config", HTTP_GET,  [this, &server]() { handleGet(server);  });
    server.on("/api/config", HTTP_POST, [this, &server]() { handlePost(server); });
}

inline void ConfigApi::handleGet(WebServer& server)
{
    JsonDocument doc;
    configToJson(config, doc);
    doc["firmwareVersion"] = FIRMWARE_VERSION_STRING;

    String json;
    serializeJson(doc, json);

    server.send(200, "application/json", json);
}

inline void ConfigApi::handlePost(WebServer& server)
{
    JsonDocument doc;
    if (deserializeJson(doc, server.arg("plain")))
    {
        server.send(400, "application/json", "{\"ok\":false,\"error\":\"invalid JSON\"}");
        return;
    }

    DeviceConfig updated = config;
    String error = configFromJson(doc, updated);

    if (error.length() > 0)
    {
        server.send(400, "application/json", "{\"ok\":false,\"error\":\"" + error + "\"}");
        return;
    }

    if (onSaved) onSaved(updated);

    server.send(200, "application/json", "{\"ok\":true,\"rebootRequired\":true}");
}
