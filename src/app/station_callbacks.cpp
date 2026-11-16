/**
 * @file    station_callbacks.cpp
 * @brief   Callbacks wiring the modules together (Alarm, WiFi, Bluetooth, LoRaWAN).
 * @author  Lucas Pichon
 * @date    2026-11-16
 */

#include "station.hpp"

#include "firmware_version.hpp"
#include "utils/helpers.hpp"
#include "utils/json_fields.hpp"

// Set up the callback for the alarm manager to persist changes
void Station::setAlarmManagerCallbacks()
{
    alarmManager.onAlarmChanged([this](bool armed, uint32_t targetEpoch)
    {
        configManager.saveAlarm(armed, targetEpoch);
        #if DEBUG_ENABLE
        Serial.println("Alarm changed: armed=" + String(armed) + ", targetEpoch=" + String(targetEpoch));
        #endif
    });
}

// Set up the callbacks for WiFi communication to handle configuration saving and sensor information requests
void Station::setWifiCallbacks()
{
    wifi.setSensorInfoProvider([this]() -> String
    {
        String json = "[";
        for (size_t i = 0; i < sensorManager.count(); i++)
        {
            if (i > 0) json += ",";
            json += "\"" + String(sensorManager.at(i).getName()) + "\"";
        }
        json += "]";
        return json;
    });


    configApi.setOnSaved([this](const DeviceConfig& newDeviceConfig)
    {
        configManager.save(newDeviceConfig);
        scheduleReboot(300);

        #if DEBUG_ENABLE
        Serial.println("Device configuration updated.");
        #endif
    });

    updateApi.setOnFinished([this]() { scheduleReboot(500); });

    wifi.addRoutes([this](WebServer& server) { configApi.install(server); });
    wifi.addRoutes([this](WebServer& server) { updateApi.install(server); });

    wifi.setMeasurementProvider([this]() -> String
    {
        String json = "{";
        json += "\"timestamp\":" + String(measurement.timestamp)   + ",";
        json += floatField("temperature", measurement.temperature) + ",";
        json += floatField("humidity",    measurement.humidity)    + ",";
        json += floatField("luminosity",  measurement.luminosity)  + ",";
        json += floatField("pressure",    measurement.pressure)    + ",";
        json += intField("co2",           measurement.co2)         + ",";
        json += intField("gasRaw",        measurement.gasRaw)      + ",";
        json += intField("vocIndex",      measurement.vocIndex)    + ",";
        json += intField("noxIndex",      measurement.noxIndex)    + ",";
        json += boolField("motion",       measurement.motion)      + ",";
        json += boolField("sound",        measurement.sound)       + ",";
        json += boolField("obstacle",     measurement.obstacle)    + ",";
        json += boolField("vibration",    measurement.vibration)   + ",";
        json += boolField("doorOpen",     measurement.doorOpen);
        json += "}";

        #if DEBUG_ENABLE
        Serial.println("Value sent over WiFi : " + json);
        #endif

        return json;
    });

    wifi.setStatusProvider([this]() -> String
    {
        String json = "{";

        // --- System ---
        json += "\"system\":{";
        json += "\"uptimeMs\":" + String(millis()) + ",";
        json += "\"firmwareVersion\":\"" + String(FIRMWARE_VERSION_STRING) + "\",";
        json += "\"resetReason\":\"" + getResetReasonString() + "\",";
        json += "\"chipModel\":\"" + String(ESP.getChipModel()) + "\",";
        json += "\"chipRevision\":" + String(ESP.getChipRevision()) + ",";
        json += "\"chipCores\":" + String(ESP.getChipCores()) + ",";
        json += "\"cpuFreqMHz\":" + String(ESP.getCpuFreqMHz()) + ",";
        json += "\"sdkVersion\":\"" + String(ESP.getSdkVersion()) + "\"";
        json += "},";

        // --- Memory ---
        json += "\"memory\":{";
        json += "\"freeHeap\":" + String(ESP.getFreeHeap()) + ",";
        json += "\"heapSize\":" + String(ESP.getHeapSize()) + ",";
        json += "\"minFreeHeap\":" + String(ESP.getMinFreeHeap()) + ",";
        json += "\"maxAllocHeap\":" + String(ESP.getMaxAllocHeap());
        json += "},";

        // --- Flash ---
        json += "\"flash\":{";
        json += "\"flashChipSize\":" + String(ESP.getFlashChipSize()) + ",";
        json += "\"sketchSize\":" + String(ESP.getSketchSize()) + ",";
        json += "\"freeSketchSpace\":" + String(ESP.getFreeSketchSpace());
        json += "},";

        // --- Clock ---
        json += "\"clock\":{";
        json += "\"synchronized\":" + String(systemClock.isSynchronized() ? 1 : 0) + ",";
        json += "\"synchronizedSinceS\":" + String(systemClock.isSynchronized() ? systemClock.isSynchronizedSince() : 0);
        json += "},";

        // --- Alarm ---
        json += "\"alarm\":{";
        json += "\"armed\":" + String(alarmManager.isArmed() ? 1 : 0) + ",";
        json += "\"ringing\":" + String(alarmManager.isRinging() ? 1 : 0) + ",";
        json += "\"targetEpoch\":" + String(deviceConfig.alarmTargetEpoch);
        json += "},";

        // --- Sensors ---
        json += "\"sensors\":[";
        for (size_t i = 0; i < sensorManager.count(); i++)
        {
            if (i > 0) json += ",";
            bool enabled = sensorManager.isEnabled(i, deviceConfig.enabledSensorsMask);
            json += "{\"name\":\"" + String(sensorManager.at(i).getName()) + "\",";
            json += "\"enabled\":" + String(enabled ? 1 : 0) + ",";
            json += "\"initialized\":" + String(sensorManager.at(i).isInitialized() ? 1 : 0) + ",";
            json += "\"lastReadOK\":" + String(sensorManager.lastReadSucceeded(i) ? 1 : 0) + "}";
        }
        json += "],";

        // --- Communications ---
        json += "\"comms\":{";

        json += "\"wifi\":{";
        json += "\"enabled\":" + String(isCommEnabled(deviceConfig.enabledCommsMask, WIFI_BIT) ? 1 : 0) + ",";
        json += "\"clients\":" + String(WiFi.softAPgetStationNum());
        json += "},";

        json += "\"bluetooth\":{";
        json += "\"enabled\":" + String(isCommEnabled(deviceConfig.enabledCommsMask, BLUETOOTH_BIT) ? 1 : 0) + ",";
        json += "\"initialized\":" + String(bluetooth.isInitialized() ? 1 : 0) + ",";
        json += "\"connected\":" + String(bluetooth.hasConnectedClient() ? 1 : 0) + ",";
        json += "\"lastOK\":" + String(status.bluetoothOK ? 1 : 0);
        json += "},";

        json += "\"lorawan\":{";
        json += "\"enabled\":" + String(isCommEnabled(deviceConfig.enabledCommsMask, LORAWAN_BIT) ? 1 : 0) + ",";
        json += "\"initialized\":" + String(lorawan.isInitialized() ? 1 : 0) + ",";
        json += "\"joined\":" + String(lorawan.isJoined() ? 1 : 0) + ",";
        json += "\"lastOK\":" + String(status.lorawanOK ? 1 : 0);
        json += "}";

        json += "}"; // comms
        json += "}"; // root

        return json;
    });
}

// Set up the callbacks for Bluetooth communication to handle time synchronization and alarm target updates
void Station::setBluetoothCallbacks()
{
    bluetooth.setTimeSyncCallback([this](uint32_t epoch)
    {
        systemClock.sync(epoch);

        #if DEBUG_ENABLE
        Serial.println("Time synchronized: epoch=" + String(epoch));
        #endif
    });

    bluetooth.setAlarmTargetCallback([this](uint32_t targetEpoch)
    {
        alarmManager.setAlarm(targetEpoch);

        #if DEBUG_ENABLE
        Serial.println("Alarm target updated: targetEpoch=" + String(targetEpoch));
        #endif
    });

    bluetooth.setWifiControlCallback([this](bool enable)
    {
        if (enable)
        {
            if (isCommEnabled(deviceConfig.enabledCommsMask, WIFI_BIT))
            {
                wifi.begin();

                #if DEBUG_ENABLE
                Serial.println("WiFi enabled.");
                #endif
            }
        }
        else
        {
            wifi.stop();

            #if DEBUG_ENABLE
            Serial.println("WiFi disabled.");
            #endif
        }
    });

    bluetooth.setDoorStateCallback([this](bool open)
    {
        doorSensor.onStateReceived(open);

        #if DEBUG_ENABLE
        Serial.println(open ? "Door open." : "Door closed.");
        #endif
    });
}

// Set up the callbacks for LoRaWAN communication to handle join events and transmission completion
void Station::setLoRaWANCallbacks()
{
    lorawan.onJoining([]()
    {
        #if DEBUG_ENABLE
        Serial.println("LoRaWAN : joining...");
        #endif
    });

    lorawan.onJoined([]()
    {
        #if DEBUG_ENABLE
        Serial.println("LoRaWAN : joined successfully!");
        #endif
    });

    lorawan.onJoinFailed([]()
    {
        #if DEBUG_ENABLE
        Serial.println("LoRaWAN : join failed.");
        #endif
    });

    lorawan.onRejoinFailed([]()
    {
        #if DEBUG_ENABLE
        Serial.println("LoRaWAN : rejoin failed.");
        #endif
    });

    lorawan.onTxStart([]()
    {
        #if DEBUG_ENABLE
        Serial.println("LoRaWAN : transmission started.");
        #endif
    });

    lorawan.onTxComplete([this](bool hasDownlink)
    {
        lorawan.setLastLoRaTwMs(millis());

        #if DEBUG_ENABLE
        Serial.println("LoRaWAN : transmission complete.");
        if (hasDownlink)
            Serial.println("LoRaWAN : downlink received.");
        #endif
    });

    lorawan.onAutoUplinkResult([this](bool acked)
    {
        // Keep the accumulated data until the network acknowledged it
        if (acked)
        {
            accumulator.reset();
        }

        #if DEBUG_ENABLE
        Serial.println(acked ? "LoRaWAN : uplink acknowledged." : "LoRaWAN : uplink not acknowledged, will retry.");
        #endif
    });

    lorawan.onDownlink([](uint8_t port, const uint8_t* data, uint8_t length)
    {
        #if DEBUG_ENABLE
        Serial.printf("LoRaWAN : downlink on port %d (%d bytes)\n", port, length);
        #endif
    });

    lorawan.onLinkDead([]()
    {
        #if DEBUG_ENABLE
        Serial.println("LoRaWAN : link dead.");
        #endif
    });

    lorawan.onLinkAlive([]()
    {
        #if DEBUG_ENABLE
        Serial.println("LoRaWAN : link alive.");
        #endif
    });
}