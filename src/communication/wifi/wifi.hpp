#pragma once

#include <WiFi.h>
#include "../communication.hpp"

class WiFiCommunication : public Communication
{
    private:
        const char* ssid;
        const char* password;

    public:
        WiFiCommunication(const char* ssid, const char* password);

        bool begin() override;
        bool send(uint8_t* data, size_t size) override;
        bool isConnected() const;
};

inline WiFiCommunication::WiFiCommunication(const char* ssid, const char* password)
    : ssid(ssid), password(password), Communication()
{
}

inline bool WiFiCommunication::begin()
{
    // Connect to WiFi network
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    // Wait for connection with a timeout
    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED)
    {
        if (millis() - startTime > 10000)
        {
            initialized = false;
            return false;
        }

        delay(100);
    }

    initialized = true;

    return true;
}

inline bool WiFiCommunication::isConnected() const
{
    return initialized && WiFi.status() == WL_CONNECTED;
}

inline bool WiFiCommunication::send(uint8_t* data, size_t size)
{
    if (!isConnected())
    {
        return false;
    }

    
    return true;
}


