/**
 * @file    wifi.hpp
 * @brief   WiFi access point and web server of the HomeStation device.
 * @author  Lucas Pichon
 * @date    2026-07-25
 */

#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <functional>
#include <vector>

#include "../communication.hpp"
#include "./pages/wifi_home_page.hpp"
#include "./pages/wifi_config_page.hpp"
#include "./pages/wifi_monitoring_page.hpp"
#include "./pages/wifi_status_page.hpp"

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

        std::vector<std::function<void(WebServer&)>> routeInstallers;  // Register extra routes on every begin()

        std::function<String()> sensorInfoProvider;             // Callback function to provide sensor information in JSON format
        std::function<String()> measurementProvider;            // Callback to provide current measurement as JSON
        std::function<String()> statusProvider;                 // Callback to provide current status as JSON

        unsigned long lastClientSeenMillis; // Timestamp of the last connected client
        static constexpr unsigned long WIFI_TIMEOUT_MS = 300000; // WIFI timeout (5 minutes)
        bool sleep; // Flag indicating if the WiFi is currently in sleep mode (off after timeout)

        // HTTP route handlers
        void handleNotFound();
        void handleGetSensors();
        void handleGetMeasurement();
        void handleHome();
        void handleConfigPage();
        void handleMonitoringPage();
        void handleGetStatus();
        void handleStatusPage();

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
        void loop() override;

        /**
         * @brief Check if there is at least one connected client to the WiFi Access Point.
         * @return true if there is at least one connected client, false otherwise.
         */
        bool hasConnectedClient() const override;

        /**
         * @brief Check if the WiFi communication is currently in sleep mode (off after a timeout).
         * @return true if the WiFi is sleeping, false otherwise.
         */
        bool isSleeping() const;

        /**
         * @brief Stop the WiFi Access Point and disconnect all clients.
         */
        void stop() override;

        /**
         * @brief Add routes that are registered on the server each time the access point starts.
         * @param installer Function that registers the routes on the given server.
         */
        void addRoutes(std::function<void(WebServer&)> installer);

        /**
         * @brief Set a callback function to provide sensor information in JSON format.
         * @param cb Callback function that returns a String containing sensor information in JSON format.
         */
        void setSensorInfoProvider(std::function<String()> cb);

        /**
         * @brief Set a callback function to provide the current measurement in JSON format.
         * @param cb Callback function that returns a String containing the current measurement in JSON format.
         */
        void setMeasurementProvider(std::function<String()> cb);

        /**
         * @brief Set a callback function to provide the current status in JSON format.
         * @param cb Callback function that returns a String containing the current status in JSON format.
         */
        void setStatusProvider(std::function<String()> cb);
};


// ============================================================
// Implementation of WiFiCommunication methods
// ============================================================


// ─────────────────────────────────────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────────────────────────────────────

inline WiFiCommunication::WiFiCommunication()
    : server(80),
      lastClientSeenMillis(0),
      sleep(false)
{
}

// ─────────────────────────────────────────────────────────────────────────────
// Configuration
// ─────────────────────────────────────────────────────────────────────────────

inline void WiFiCommunication::configure(const String& ssid, const String& password)
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
    server.on("/",                  HTTP_GET,  [this]() { handleHome();              });
    server.on("/api/sensors",       HTTP_GET,  [this]() { handleGetSensors();        });
    server.on("/api/measurement",   HTTP_GET,  [this]() { handleGetMeasurement();    });
    server.on("/monitoring",        HTTP_GET,  [this]() { handleMonitoringPage();    });
    server.on("/config",            HTTP_GET,  [this]() { handleConfigPage();        });
    server.on("/api/status",        HTTP_GET,  [this]() { handleGetStatus();         });
    server.on("/status",            HTTP_GET,  [this]() { handleStatusPage();        });
    server.onNotFound(                         [this]() { handleNotFound();          });

    for (auto& installer : routeInstallers)
    {
        installer(server);
    }

    // Start the HTTP server
    server.begin();

    initialized = true;
    sleep = false;
    lastClientSeenMillis = millis(); // start the inactivity timer even if no client ever connects
    return true;
}


// ─────────────────────────────────────────────────────────────────────────────
// Main loop
// ─────────────────────────────────────────────────────────────────────────────

inline void WiFiCommunication::loop()
{
    if (!isInitialized() || isSleeping())
        return;

    // Handle incoming HTTP requests
    server.handleClient();

    if (hasConnectedClient())
    {
        lastClientSeenMillis = millis();
    }
    else if (millis() - lastClientSeenMillis > WIFI_TIMEOUT_MS)
    {
        stop();
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// HTTP Route Handlers
// ─────────────────────────────────────────────────────────────────────────────

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

inline void WiFiCommunication::handleGetMeasurement()
{
    if (!measurementProvider)
    {
        server.send(200, "application/json", "{}");
        return;
    }
    server.send(200, "application/json", measurementProvider());
}

inline void WiFiCommunication::handleHome()
{
    server.send_P(200, "text/html", WIFI_HOME_PAGE);
}

inline void WiFiCommunication::handleConfigPage()
{
    server.send_P(200, "text/html", WIFI_CONFIG_PAGE);
}

inline void WiFiCommunication::handleMonitoringPage()
{
    server.send_P(200, "text/html", WIFI_MONITORING_PAGE);
}

inline void WiFiCommunication::handleGetStatus()
{
    if (!statusProvider)
    {
        server.send(200, "application/json", "{}");
        return;
    }
    server.send(200, "application/json", statusProvider());
}

inline void WiFiCommunication::handleStatusPage()
{
    server.send_P(200, "text/html", WIFI_STATUS_PAGE);
}

// ─────────────────────────────────────────────────────────────────────────────
// State
// ─────────────────────────────────────────────────────────────────────────────

inline bool WiFiCommunication::hasConnectedClient() const
{
    // Check if the WiFi communication is initialized and if there is at least one connected client to the WiFi Access Point
    if (!initialized || sleep)
    {
        return false;
    }
    return WiFi.softAPgetStationNum() > 0;
}

inline bool WiFiCommunication::isSleeping() const
{
    // Check if the WiFi communication is currently in sleep mode (off after a timeout)
    return sleep;
}

inline void WiFiCommunication::stop()
{
    // Stop the WiFi Access Point and disconnect all clients
    WiFi.softAPdisconnect(true);
    WiFi.mode(WIFI_OFF);
    server.stop();
    sleep = true;
    lastClientSeenMillis = 0;
}

// ─────────────────────────────────────────────────────────────────────────────
// Setters
// ─────────────────────────────────────────────────────────────────────────────

inline void WiFiCommunication::addRoutes(std::function<void(WebServer&)> installer)
{
    routeInstallers.push_back(installer);
}

inline void WiFiCommunication::setSensorInfoProvider(std::function<String()> cb)
{
    // Set the callback function to provide sensor information in JSON format
    sensorInfoProvider = cb;
}

inline void WiFiCommunication::setMeasurementProvider(std::function<String()> cb)
{
    // Set the callback function to provide the current measurement in JSON format
    measurementProvider = cb;
}

inline void WiFiCommunication::setStatusProvider(std::function<String()> cb)
{
    // Set the callback function to provide the current status in JSON format
    statusProvider = cb;
}
