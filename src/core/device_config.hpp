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
};