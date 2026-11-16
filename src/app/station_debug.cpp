/**
 * @file    station_debug.cpp
 * @brief   Debug information printed on Serial at startup (DEBUG_ENABLE only).
 * @author  Lucas Pichon
 * @date    2026-11-16
 */

#include "station.hpp"

#if DEBUG_ENABLE

#include <Wire.h>
#include <WiFi.h>

#include "firmware_version.hpp"

void Station::printDebugInfo()
{
    Serial.println("========  HomeStation Firmware  ========");

    Serial.println("--- Firmware Version ---");
    Serial.println(FIRMWARE_VERSION_STRING);

    Serial.println("--- Sensors ---");
    Serial.print("Number of sensors: ");
    Serial.println(sensorManager.count());

    Serial.println("--- Communication ---");
    Serial.println(bluetooth.isInitialized()
        ? "Bluetooth started. Device Name: " + String(deviceConfig.bleDeviceName)
        : "Bluetooth not started.");

    Serial.println(wifi.isInitialized()
        ? "WiFi AP started. IP: " + WiFi.softAPIP().toString()
        : "WiFi AP not started.");

    Serial.println(lorawan.isInitialized() ? "LoRaWAN used." : "LoRaWAN not used.");

    Serial.println("--- I2C Scan ---");
    for (uint8_t addr = 1; addr < 127; addr++)
    {
        Wire.beginTransmission(addr);
        if (Wire.endTransmission() == 0)
        {
            Serial.print("(I2C) Found peripheral at 0x");
            Serial.println(addr, HEX);
        }
    }

    Serial.println("--- Memory ---");
    Serial.print("devEui: ");
    for (int i = 0; i < 8; i++) { Serial.print(deviceConfig.devEui[i], HEX); if (i < 7) Serial.print(":"); }
    Serial.println();
    Serial.print("appEui: ");
    for (int i = 0; i < 8; i++) { Serial.print(deviceConfig.appEui[i], HEX); if (i < 7) Serial.print(":"); }
    Serial.println();
    Serial.print("appKey: ");
    for (int i = 0; i < 16; i++) { Serial.print(deviceConfig.appKey[i], HEX); if (i < 15) Serial.print(":"); }
    Serial.println();
    Serial.print("bleDeviceName: ");      Serial.println(deviceConfig.bleDeviceName);
    Serial.print("serviceUUID: ");        Serial.println(deviceConfig.serviceUUID);
    Serial.print("characteristicUUID: "); Serial.println(deviceConfig.characteristicUUID);
    Serial.print("timeSyncUUID: ");       Serial.println(deviceConfig.timeSyncUUID);
    Serial.print("alarmTargetUUID: ");    Serial.println(deviceConfig.alarmTargetUUID);
    Serial.print("doorStateUUID: ");      Serial.println(deviceConfig.doorStateUUID);
    Serial.print("alarmArmed: ");         Serial.println(deviceConfig.alarmArmed ? "true" : "false");
    Serial.print("alarmTargetEpoch: ");   Serial.println(deviceConfig.alarmTargetEpoch);
    Serial.print("utcOffset: ");          Serial.println(deviceConfig.utcOffset);
    Serial.print("wifiControlUUID: ");    Serial.println(deviceConfig.wifiControlUUID);
    Serial.print("enabledSensorsMask: "); Serial.println(deviceConfig.enabledSensorsMask, BIN);
    Serial.print("enabledCommsMask: ");   Serial.println(deviceConfig.enabledCommsMask, BIN);
    Serial.print("altitude: ");           Serial.println(deviceConfig.altitudeM);

    Serial.println("--- LoRaWAN Region ---");
    #if defined(CFG_eu868)
    Serial.println("Region: EU868");
    #elif defined(CFG_us915)
    Serial.println("Region: US915");
    #else
    Serial.println("Region: unknown");
    #endif

    Serial.println("--- AES ---");
    for (int i = 0; i < 16; i++) { Serial.print(deviceConfig.aesKey[i], HEX); if (i < 15) Serial.print(":"); }
    Serial.println("========================================");
}

#endif // DEBUG_ENABLE