/**
 * @file    keys.hpp
 * @brief   NVS namespace and keys under which the device configuration is persisted.
 * @author  Lucas Pichon
 * @date    2026-11-16
 */

#pragma once

namespace StorageKeys
{
    inline constexpr const char* STORAGE_NAMESPACE      = "HomeStation";
    inline constexpr const char* devEUIKey              = "devEui";
    inline constexpr const char* appEUIKey              = "appEui";
    inline constexpr const char* appKeyKey              = "appKey";
    inline constexpr const char* bleNameKey             = "bleName";
    inline constexpr const char* serviceUUIDKey         = "serUUID";
    inline constexpr const char* characteristicUUIDKey  = "charUUID";
    inline constexpr const char* timeSyncUUIDKey        = "tSynUUID";
    inline constexpr const char* alarmTargetUUIDKey     = "alTaUUID";
    inline constexpr const char* wifiControlUUIDKey     = "wiCoUUID";
    inline constexpr const char* doorStateUUIDKey       = "doStUUID";
    inline constexpr const char* wifiApSSIDKey          = "apSSID";
    inline constexpr const char* wifiApPasswordKey      = "apPass";
    inline constexpr const char* utcOffsetKey           = "utcOffset";
    inline constexpr const char* alarmArmedKey          = "alarmArmed";
    inline constexpr const char* alarmTargetKey         = "alarmTarget";
    inline constexpr const char* enabledSensorsMaskKey  = "enSensMask";
    inline constexpr const char* enabledCommsMaskKey    = "enCommMask";
    inline constexpr const char* aesKeyKey              = "aesKey";
    inline constexpr const char* altitudeKey            = "altitude";
}
