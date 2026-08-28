/**
 * @file    device_config.hpp
 * @brief   Aggregates all persistent device configuration (time, alarm, LoRaWAN,
 *          Bluetooth, WiFi AP and sensors) into a single struct.
 * @author  Lucas Pichon
 * @date    2026-07-30
 */

#pragma once

#include <Arduino.h>

// ============================================================
// DeviceConfig struct definition
// ============================================================

/**
 * @brief Holds the full persistent configuration of the device, as loaded
 *        from (or saved to) storage. Grouped by functional area.
 */
struct DeviceConfig
{
    // ── Time ─────────────────────────────────────────────────────────────
    int8_t utcOffset = 0;   // UTC offset (in hours) used for local time calculations

    // ── Alarm ────────────────────────────────────────────────────────────
    bool alarmArmed = false;          // Whether the alarm is currently armed
    uint32_t alarmTargetEpoch = 0;    // Target epoch time at which the alarm should ring

    // ── LoRaWAN ──────────────────────────────────────────────────────────
    uint8_t devEui[8];    // LoRaWAN device EUI (8 bytes)
    uint8_t appEui[8];    // LoRaWAN application EUI (8 bytes)
    uint8_t appKey[16];   // LoRaWAN application key (16 bytes)

    // ── Bluetooth ────────────────────────────────────────────────────────
    String bleDeviceName;        // BLE device name
    String serviceUUID;          // BLE service UUID
    String characteristicUUID;   // BLE characteristic UUID for measurements
    String timeSyncUUID;         // BLE characteristic UUID for time synchronization
    String alarmTargetUUID;      // BLE characteristic UUID for alarm target

    // ── WiFi AP ──────────────────────────────────────────────────────────
    String wifiApSSID;       // SSID of the WiFi access point
    String wifiApPassword;   // Password of the WiFi access point

    // ── Sensors ──────────────────────────────────────────────────────────
    uint16_t enabledSensorsMask = 0xFFFF;   // Bitmask of enabled sensors (all enabled by default)

    // ── Communication ────────────────────────────────────────────────────
    uint8_t enabledCommsMask = 0x07; // Bitmask of enabled communication interfaces (bit 0: LoRaWAN, bit 1: Bluetooth, bit 2: WiFi AP)
};

enum SensorsBit : uint16_t
{
    BH1750_BIT   = 1 << 0,  // Bit 0: BH1750 light sensor
    HCSR501_BIT  = 1 << 1,  // Bit 1: HC-SR501 motion sensor
    SCD41_BIT    = 1 << 2,  // Bit 2: SCD41 CO2 sensor
    MAX9814_BIT  = 1 << 3,  // Bit 3: MAX9814 microphone
    FC51_BIT     = 1 << 4,  // Bit 4: FC-51 rain sensor
    SW420_BIT    = 1 << 5,  // Bit 5: SW-420 vibration sensor
    MQ2_BIT      = 1 << 6,  // Bit 6: MQ-2 gas sensor
    BMP280_BIT   = 1 << 7,  // Bit 7: BMP280 pressure sensor
    SGP41_BIT    = 1 << 8   // Bit 8: SGP41 air quality sensor
};

inline bool isSensorEnabled(uint16_t enabledSensorsMask, SensorsBit sensor)
{
    return (enabledSensorsMask & sensor) != 0;
}

enum CommsBit : uint8_t
{
    BLUETOOTH_BIT = 1 << 0, // Bit 0: Bluetooth
    LORAWAN_BIT   = 1 << 1, // Bit 1: LoRaWAN
    WIFI_BIT      = 1 << 2  // Bit 2: WiFi AP
}; 

inline bool isCommEnabled(uint8_t enabledCommsMask, CommsBit comm)
{
    return (enabledCommsMask & comm) != 0;
}