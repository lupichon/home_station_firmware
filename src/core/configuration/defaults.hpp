/**
 * @file    defaults.hpp
 * @brief   Values used for configuration entries that are not stored yet.
 * @author  Lucas Pichon
 * @date    2026-11-16
 */

#pragma once

#include <stddef.h>
#include <stdint.h>

namespace ConfigDefaults
{
    // ── Time ─────────────────────────────────────────────────────────────
    inline constexpr int8_t   UTC_OFFSET = 0;

    // ── Alarm ────────────────────────────────────────────────────────────
    inline constexpr bool     ALARM_ARMED        = false;
    inline constexpr uint32_t ALARM_TARGET_EPOCH = 0;

    // ── LoRaWAN ──────────────────────────────────────────────────────────
    inline constexpr uint8_t DEV_EUI[8]  = {0};
    inline constexpr uint8_t APP_EUI[8]  = {0};
    inline constexpr uint8_t APP_KEY[16] = {0};

    // ── Bluetooth ────────────────────────────────────────────────────────
    inline constexpr const char* BLE_DEVICE_NAME      = "HomeStation";
    inline constexpr const char* SERVICE_UUID         = "00000000-0000-0000-0000-000000000000";
    inline constexpr const char* CHARACTERISTIC_UUID  = "00000000-0000-0000-0000-000000000001";
    inline constexpr const char* TIME_SYNC_UUID       = "00000000-0000-0000-0000-000000000002";
    inline constexpr const char* ALARM_TARGET_UUID    = "00000000-0000-0000-0000-000000000003";
    inline constexpr const char* WIFI_CONTROL_UUID    = "00000000-0000-0000-0000-000000000004";
    inline constexpr const char* DOOR_STATE_UUID      = "00000000-0000-0000-0000-000000000005";

    // ── WiFi AP ──────────────────────────────────────────────────────────
    inline constexpr const char* WIFI_AP_SSID         = "HomeStation";
    inline constexpr const char* WIFI_AP_PASSWORD     = "homestation";

    // ── Features ─────────────────────────────────────────────────────────
    inline constexpr uint16_t ENABLED_SENSORS_MASK = 0x0000;
    inline constexpr uint8_t  ENABLED_COMMS_MASK   = 0x04;    //Only Wifi AP

    // ── Location ─────────────────────────────────────────────────────────
    inline constexpr int16_t  ALTITUDE_M = 0;

    // ── Crypto ───────────────────────────────────────────────────────────
    inline constexpr uint8_t  AES_KEY[16] = {0};
}
