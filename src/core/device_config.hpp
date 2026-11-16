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
    String wifiControlUUID;      // BLE characteristic UUID for WiFi control
    String doorStateUUID;        // BLE characteristic UUID for door state

    // ── WiFi AP ──────────────────────────────────────────────────────────
    String wifiApSSID;       // SSID of the WiFi access point
    String wifiApPassword;   // Password of the WiFi access point

    // ── Sensors ──────────────────────────────────────────────────────────
    uint16_t enabledSensorsMask = 0x0000;   // Bitmask of enabled sensors (all enabled by default)

    // ── Communication ────────────────────────────────────────────────────
    uint8_t enabledCommsMask = 0x00; // Bitmask of enabled communication interfaces (bit 0: LoRaWAN, bit 1: Bluetooth, bit 2: WiFi AP)

    // ── Location ─────────────────────────────────────────────────────────
    int16_t altitudeM = 0;   // Altitude in meters, used to convert pressure to sea level

    // ── Crypto ─────────────────────────────────────────────────────────────
    uint8_t aesKey[16];   // AES-128 key for encrypting/decrypting data
};

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