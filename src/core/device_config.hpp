
#pragma once

#include <Arduino.h>

struct DeviceConfig
{
    // Time
    int8_t utcOffset = 0;

    // Alarm
    bool alarmArmed = false;
    uint32_t alarmTargetEpoch = 0;

    // LoRaWAN
    uint8_t devEui[8];
    uint8_t appEui[8];
    uint8_t appKey[16];

    // Bluetooth
    String bleDeviceName;
    String serviceUUID;
    String characteristicUUID;
    String timeSyncUUID;
    String alarmTargetUUID;

    // WiFi AP
    String wifiApSSID;
    String wifiApPassword;

    // Sensors
    uint16_t enabledSensorsMask = 0xFFFF; 
};