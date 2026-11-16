/**
 * @file    config_manager.hpp
 * @brief   Loads and saves the device configuration in persistent storage.
 * @author  Lucas Pichon
 * @date    2026-11-16
 */

#pragma once

#include <Arduino.h>
#include <string.h>

#include "device_config.hpp"
#include "defaults.hpp"
#include "../storage/storage.hpp"
#include "../storage/keys.hpp"


// ============================================================
// ConfigManager class definition
// ============================================================

/**
 * @brief Keeps a DeviceConfig in sync with the NVS storage.
 *
 * Entries that are not stored yet take the values of ConfigDefaults, without
 * being written back.
 */
class ConfigManager
{
    // ── Private members ───────────────────────────────────────────────────
    private:
        Storage&      storage;  // Persistent storage
        DeviceConfig& config;   // Live configuration kept in sync with the storage

        String loadString(const char* key, const String& defaultValue);
        void   loadBytes(const char* key, uint8_t* out, size_t size, const uint8_t* defaultValue);

        // Write a value to the storage and to the live config, only if it changed
        void update(const char* key, String& current, const String& incoming);
        void update(const char* key, uint8_t* current, const uint8_t* incoming, size_t size);
        void update(const char* key, bool& current, bool incoming);
        void update(const char* key, int8_t& current, int8_t incoming);
        void update(const char* key, int16_t& current, int16_t incoming);
        void update(const char* key, uint8_t& current, uint8_t incoming);
        void update(const char* key, uint16_t& current, uint16_t incoming);
        void update(const char* key, uint32_t& current, uint32_t incoming);

    // ── Public interface ────────────────────────────────────────────────────
    public:

        /**
         * @param storage Persistent storage.
         * @param config  Live configuration that load() fills and save() updates.
         */
        ConfigManager(Storage& storage, DeviceConfig& config);

        /**
         * @brief Fill the configuration from the storage, using defaults for missing entries.
         */
        void load();

        /**
         * @brief Persist every field of incoming that differs from the live configuration.
         * @param incoming New configuration, typically submitted from the WiFi page.
         */
        void save(const DeviceConfig& incoming);

        /**
         * @brief Persist the alarm state.
         */
        void saveAlarm(bool armed, uint32_t targetEpoch);

        /**
         * @brief Persist the mask of enabled communication interfaces.
         */
        void saveCommsMask(uint8_t mask);
};


// ============================================================
// Implementations of ConfigManager methods
// ============================================================

inline ConfigManager::ConfigManager(Storage& storage, DeviceConfig& config)
    : storage(storage),
      config(config)
{
}


// ─────────────────────────────────────────────────────────────────────────────
// Loading
// ─────────────────────────────────────────────────────────────────────────────

inline void ConfigManager::load()
{
    storage.begin(StorageKeys::STORAGE_NAMESPACE, false);

    // LoRaWAN
    loadBytes(StorageKeys::devEUIKey, config.devEui, sizeof(config.devEui), ConfigDefaults::DEV_EUI);
    loadBytes(StorageKeys::appEUIKey, config.appEui, sizeof(config.appEui), ConfigDefaults::APP_EUI);
    loadBytes(StorageKeys::appKeyKey, config.appKey, sizeof(config.appKey), ConfigDefaults::APP_KEY);

    // Bluetooth
    config.bleDeviceName      = loadString(StorageKeys::bleNameKey,            ConfigDefaults::BLE_DEVICE_NAME);
    config.serviceUUID        = loadString(StorageKeys::serviceUUIDKey,        ConfigDefaults::SERVICE_UUID);
    config.characteristicUUID = loadString(StorageKeys::characteristicUUIDKey, ConfigDefaults::CHARACTERISTIC_UUID);
    config.timeSyncUUID       = loadString(StorageKeys::timeSyncUUIDKey,       ConfigDefaults::TIME_SYNC_UUID);
    config.alarmTargetUUID    = loadString(StorageKeys::alarmTargetUUIDKey,    ConfigDefaults::ALARM_TARGET_UUID);
    config.wifiControlUUID    = loadString(StorageKeys::wifiControlUUIDKey,    ConfigDefaults::WIFI_CONTROL_UUID);
    config.doorStateUUID      = loadString(StorageKeys::doorStateUUIDKey,      ConfigDefaults::DOOR_STATE_UUID);

    // WiFi AP
    config.wifiApSSID         = loadString(StorageKeys::wifiApSSIDKey,     ConfigDefaults::WIFI_AP_SSID);
    config.wifiApPassword     = loadString(StorageKeys::wifiApPasswordKey, ConfigDefaults::WIFI_AP_PASSWORD);

    // Other settings
    config.utcOffset          = storage.getChar(StorageKeys::utcOffsetKey,   ConfigDefaults::UTC_OFFSET);
    config.alarmArmed         = storage.getBool(StorageKeys::alarmArmedKey,  ConfigDefaults::ALARM_ARMED);
    config.alarmTargetEpoch   = storage.getUInt(StorageKeys::alarmTargetKey, ConfigDefaults::ALARM_TARGET_EPOCH);
    config.altitudeM          = storage.getShort(StorageKeys::altitudeKey,   ConfigDefaults::ALTITUDE_M);
    config.enabledSensorsMask = static_cast<uint16_t>(storage.getUInt(StorageKeys::enabledSensorsMaskKey, ConfigDefaults::ENABLED_SENSORS_MASK));
    config.enabledCommsMask   = static_cast<uint8_t>(storage.getUInt(StorageKeys::enabledCommsMaskKey, ConfigDefaults::ENABLED_COMMS_MASK));

    loadBytes(StorageKeys::aesKeyKey, config.aesKey, sizeof(config.aesKey), ConfigDefaults::AES_KEY);

    storage.end();
}

inline String ConfigManager::loadString(const char* key, const String& defaultValue)
{
    return storage.exists(key) ? storage.getString(key) : defaultValue;
}

inline void ConfigManager::loadBytes(const char* key, uint8_t* out, size_t size, const uint8_t* defaultValue)
{
    if (storage.exists(key) && storage.getBytesLength(key) == size)
    {
        storage.getBytes(key, out, size);
    }
    else
    {
        memcpy(out, defaultValue, size);
    }
}


// ─────────────────────────────────────────────────────────────────────────────
// Saving
// ─────────────────────────────────────────────────────────────────────────────

inline void ConfigManager::save(const DeviceConfig& incoming)
{
    storage.begin(StorageKeys::STORAGE_NAMESPACE, false);

    update(StorageKeys::devEUIKey, config.devEui, incoming.devEui, sizeof(config.devEui));
    update(StorageKeys::appEUIKey, config.appEui, incoming.appEui, sizeof(config.appEui));
    update(StorageKeys::appKeyKey, config.appKey, incoming.appKey, sizeof(config.appKey));

    update(StorageKeys::bleNameKey,            config.bleDeviceName,      incoming.bleDeviceName);
    update(StorageKeys::serviceUUIDKey,        config.serviceUUID,        incoming.serviceUUID);
    update(StorageKeys::characteristicUUIDKey, config.characteristicUUID, incoming.characteristicUUID);
    update(StorageKeys::timeSyncUUIDKey,       config.timeSyncUUID,       incoming.timeSyncUUID);
    update(StorageKeys::alarmTargetUUIDKey,    config.alarmTargetUUID,    incoming.alarmTargetUUID);
    update(StorageKeys::wifiControlUUIDKey,    config.wifiControlUUID,    incoming.wifiControlUUID);
    update(StorageKeys::doorStateUUIDKey,      config.doorStateUUID,      incoming.doorStateUUID);

    update(StorageKeys::wifiApSSIDKey,         config.wifiApSSID,         incoming.wifiApSSID);
    update(StorageKeys::wifiApPasswordKey,     config.wifiApPassword,     incoming.wifiApPassword);

    update(StorageKeys::utcOffsetKey,          config.utcOffset,          incoming.utcOffset);
    update(StorageKeys::altitudeKey,           config.altitudeM,          incoming.altitudeM);
    update(StorageKeys::enabledSensorsMaskKey, config.enabledSensorsMask, incoming.enabledSensorsMask);
    update(StorageKeys::enabledCommsMaskKey,   config.enabledCommsMask,   incoming.enabledCommsMask);

    update(StorageKeys::aesKeyKey, config.aesKey, incoming.aesKey, sizeof(config.aesKey));

    storage.end();
}

inline void ConfigManager::saveAlarm(bool armed, uint32_t targetEpoch)
{
    storage.begin(StorageKeys::STORAGE_NAMESPACE, false);

    update(StorageKeys::alarmArmedKey,  config.alarmArmed,       armed);
    update(StorageKeys::alarmTargetKey, config.alarmTargetEpoch, targetEpoch);

    storage.end();
}

inline void ConfigManager::saveCommsMask(uint8_t mask)
{
    storage.begin(StorageKeys::STORAGE_NAMESPACE, false);

    update(StorageKeys::enabledCommsMaskKey, config.enabledCommsMask, mask);

    storage.end();
}

inline void ConfigManager::update(const char* key, String& current, const String& incoming)
{
    if (current == incoming) return;

    storage.putString(key, incoming);
    current = incoming;
}

inline void ConfigManager::update(const char* key, uint8_t* current, const uint8_t* incoming, size_t size)
{
    if (memcmp(current, incoming, size) == 0) return;

    storage.putBytes(key, incoming, size);
    memcpy(current, incoming, size);
}

inline void ConfigManager::update(const char* key, bool& current, bool incoming)
{
    if (current == incoming) return;

    storage.putBool(key, incoming);
    current = incoming;
}

inline void ConfigManager::update(const char* key, int8_t& current, int8_t incoming)
{
    if (current == incoming) return;

    storage.putChar(key, incoming);
    current = incoming;
}

inline void ConfigManager::update(const char* key, int16_t& current, int16_t incoming)
{
    if (current == incoming) return;

    storage.putShort(key, incoming);
    current = incoming;
}

inline void ConfigManager::update(const char* key, uint8_t& current, uint8_t incoming)
{
    if (current == incoming) return;

    storage.putUInt(key, incoming);
    current = incoming;
}

inline void ConfigManager::update(const char* key, uint16_t& current, uint16_t incoming)
{
    if (current == incoming) return;

    storage.putUInt(key, incoming);
    current = incoming;
}

inline void ConfigManager::update(const char* key, uint32_t& current, uint32_t incoming)
{
    if (current == incoming) return;

    storage.putUInt(key, incoming);
    current = incoming;
}
