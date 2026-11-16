/**
 * @file    config_json.hpp
 * @brief   Conversion between DeviceConfig and its JSON representation, with validation.
 * @author  Lucas Pichon
 * @date    2026-11-16
 */

#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>

#include "device_config.hpp"
#include "../utils/hex.hpp"

/**
 * @brief Write every field of a configuration into a JSON document.
 */
inline void configToJson(const DeviceConfig& config, JsonDocument& doc)
{
    doc["utcOffset"]          = static_cast<int>(config.utcOffset);
    doc["altitudeM"]          = static_cast<int>(config.altitudeM);
    doc["enabledSensorsMask"] = static_cast<unsigned int>(config.enabledSensorsMask);
    doc["enabledCommsMask"]   = static_cast<unsigned int>(config.enabledCommsMask);

    doc["devEui"] = bytesToHex(config.devEui, sizeof(config.devEui));
    doc["appEui"] = bytesToHex(config.appEui, sizeof(config.appEui));
    doc["appKey"] = bytesToHex(config.appKey, sizeof(config.appKey));

    doc["bleDeviceName"]      = config.bleDeviceName;
    doc["serviceUUID"]        = config.serviceUUID;
    doc["characteristicUUID"] = config.characteristicUUID;
    doc["timeSyncUUID"]       = config.timeSyncUUID;
    doc["alarmTargetUUID"]    = config.alarmTargetUUID;
    doc["wifiControlUUID"]    = config.wifiControlUUID;
    doc["doorStateUUID"]      = config.doorStateUUID;

    doc["wifiApSSID"]     = config.wifiApSSID;
    doc["wifiApPassword"] = config.wifiApPassword;

    doc["aesKey"] = bytesToHex(config.aesKey, sizeof(config.aesKey));
}

/**
 * @brief Validate a JSON configuration and apply it to a DeviceConfig.
 *
 * The altitude and the two masks keep their current value when absent.
 * The configuration is left untouched if validation fails.
 *
 * @param json   JSON object submitted by the client.
 * @param config Configuration to update.
 * @return An empty string on success, otherwise the description of the first error.
 */
inline String configFromJson(JsonVariantConst json, DeviceConfig& config)
{
    int          utcOffset          = json["utcOffset"]          | -128;
    int          altitudeM          = json["altitudeM"]          | static_cast<int>(config.altitudeM);
    unsigned int enabledSensorsMask = json["enabledSensorsMask"] | static_cast<unsigned int>(config.enabledSensorsMask);
    unsigned int enabledCommsMask   = json["enabledCommsMask"]   | static_cast<unsigned int>(config.enabledCommsMask);

    String devEui             = json["devEui"]             | "";
    String appEui             = json["appEui"]             | "";
    String appKey             = json["appKey"]             | "";
    String bleDeviceName      = json["bleDeviceName"]      | "";
    String serviceUUID        = json["serviceUUID"]        | "";
    String characteristicUUID = json["characteristicUUID"] | "";
    String timeSyncUUID       = json["timeSyncUUID"]       | "";
    String alarmTargetUUID    = json["alarmTargetUUID"]    | "";
    String wifiControlUUID    = json["wifiControlUUID"]    | "";
    String doorStateUUID      = json["doorStateUUID"]      | "";
    String wifiApSSID         = json["wifiApSSID"]         | "";
    String wifiApPassword     = json["wifiApPassword"]     | "";
    String aesKey             = json["aesKey"]             | "";

    if (utcOffset < -12 || utcOffset > 14)       return "utcOffset out of range";
    if (altitudeM < -500 || altitudeM > 9000)    return "altitudeM out of range";
    if (!isHexString(devEui, 16))                return "devEui must be 16 hex characters";
    if (!isHexString(appEui, 16))                return "appEui must be 16 hex characters";
    if (!isHexString(appKey, 32))                return "appKey must be 32 hex characters";
    if (bleDeviceName.length() == 0)             return "bleDeviceName cannot be empty";

    if (serviceUUID.length() == 0 || characteristicUUID.length() == 0 ||
        timeSyncUUID.length() == 0 || alarmTargetUUID.length() == 0 ||
        wifiControlUUID.length() == 0 || doorStateUUID.length() == 0)
    {
        return "Bluetooth UUID fields cannot be empty";
    }

    if (wifiApSSID.length() == 0)                return "wifiApSSID cannot be empty";
    if (wifiApPassword.length() == 0)            return "wifiApPassword cannot be empty";
    if (!isHexString(aesKey, 32))                return "aesKey must be 32 hex characters";

    config.utcOffset          = static_cast<int8_t>(utcOffset);
    config.altitudeM          = static_cast<int16_t>(altitudeM);
    config.enabledSensorsMask = static_cast<uint16_t>(enabledSensorsMask);
    config.enabledCommsMask   = static_cast<uint8_t>(enabledCommsMask);

    hexToBytes(devEui.c_str(), config.devEui, sizeof(config.devEui));
    hexToBytes(appEui.c_str(), config.appEui, sizeof(config.appEui));
    hexToBytes(appKey.c_str(), config.appKey, sizeof(config.appKey));
    hexToBytes(aesKey.c_str(), config.aesKey, sizeof(config.aesKey));

    config.bleDeviceName      = bleDeviceName;
    config.serviceUUID        = serviceUUID;
    config.characteristicUUID = characteristicUUID;
    config.timeSyncUUID       = timeSyncUUID;
    config.alarmTargetUUID    = alarmTargetUUID;
    config.wifiControlUUID    = wifiControlUUID;
    config.doorStateUUID      = doorStateUUID;
    config.wifiApSSID         = wifiApSSID;
    config.wifiApPassword     = wifiApPassword;

    return String();
}
