/**
 * @file    station_tasks.cpp
 * @brief   Periodic tasks (handleXxx) executed by Station::loop().
 * @author  Lucas Pichon
 * @date    2026-11-16
 */

#include "station.hpp"

#include <esp_task_wdt.h>

// Handle the alarm manager update
void Station::handleAlarmManager()
{
    alarmManager.update();
}

// Handle the screen update with the latest measurement data
void Station::handleScreen()
{
    screen.update(measurement);
}

// Handle the buzzer update
void Station::handleBuzzer()
{
    buzzer.update();
}

// Handle the background sampling of the sensors that need it (sound, VOC/NOx)
void Station::handleSensorsUpdate()
{
    sensorManager.update(deviceConfig.enabledSensorsMask);
}

// Handle the WiFi communication loop
void Station::handleWifi()
{
    if (isCommEnabled(deviceConfig.enabledCommsMask, WIFI_BIT))
    {
        wifi.loop();
    }
}

// Handle the LoRaWAN communication loop
void Station::handleLoRaWAN()
{
    if (isCommEnabled(deviceConfig.enabledCommsMask, LORAWAN_BIT))
    {
        lorawan.loop();
    }
}

// Handle the status LED update based on the current system status and indicators
void Station::handleLED()
{
    statusLED.update();

    status.lorawanOK = !isCommEnabled(deviceConfig.enabledCommsMask, LORAWAN_BIT) || (lorawan.isInitialized() && lorawan.isJoined());
    status.wifiOK = !isCommEnabled(deviceConfig.enabledCommsMask, WIFI_BIT) || wifi.isInitialized();

    if (!status.sensorOK && !status.lorawanOK)
    {
        statusLED.setState(StatusLED::State::ERROR);
    }
    else if (!status.sensorOK)
    {
        statusLED.setState(StatusLED::State::WARNING_SENSOR);
    }
    else if (!status.bluetoothOK || !status.lorawanOK || !status.wifiOK)
    {
        statusLED.setState(StatusLED::State::WARNING_COMMUNICATION);
    }
    else
    {
        statusLED.setState(StatusLED::State::OK);
    }

    statusLED.setIndicator(StatusLED::Indicator::BLUETOOTH_CONNECTED, bluetooth.hasConnectedClient());
    statusLED.setIndicator(StatusLED::Indicator::WIFI_CONNECTED, wifi.hasConnectedClient());
    statusLED.setIndicator(StatusLED::Indicator::ALARM_TRIGGERED, alarmManager.isRinging());
    statusLED.setIndicator(StatusLED::Indicator::ALARM_ARMED, alarmManager.isArmed());
    statusLED.setIndicator(StatusLED::Indicator::BUTTON_HELD, button.isHeld());
    statusLED.setIndicator(StatusLED::Indicator::TIME_SYNCED, systemClock.isSynchronized() && systemClock.isSynchronizedSince() <= 30);
    statusLED.setIndicator(StatusLED::Indicator::LORAWAN_TX, lorawan.getLastLoRaTwMs() > 0 && (millis() - lorawan.getLastLoRaTwMs() < 30000));
}

// Handle the button press and hold events, including reboot on long press
void Station::handleButton()
{
    constexpr unsigned long START_WIFI          = 5000;
    constexpr unsigned long REBOOT_HOLD_TIME_MS = 10000;

    button.update();

    if (button.wasPressed())
    {
        if (buzzer.isActive())
        {
            alarmManager.dismiss();
        }
        else
        {
            screen.interact();
        }
    }

    if (button.isHeld())
    {
        if (buttonPressStartMs == 0)
        {
            buttonPressStartMs    = millis();
            buttonWifiTriggered   = false;
            buttonRebootTriggered = false;
        }

        unsigned long holdTime = millis() - buttonPressStartMs;

        if (!buttonWifiTriggered && !wifi.isInitialized() && holdTime >= START_WIFI)
        {
            buttonWifiTriggered = true;

            wifi.configure(deviceConfig.wifiApSSID, deviceConfig.wifiApPassword);
            setWifiCallbacks();
            wifi.begin();
            configManager.saveCommsMask(deviceConfig.enabledCommsMask | static_cast<uint8_t>(CommsBit::WIFI_BIT));

            #if DEBUG_ENABLE
            Serial.println("WiFi started");
            #endif
        }

        if (!buttonRebootTriggered && holdTime >= REBOOT_HOLD_TIME_MS)
        {
            buttonRebootTriggered = true;

            #if DEBUG_ENABLE
            Serial.println("Rebooting system due to long button press.");
            #endif

            delay(1000);
            ESP.restart();
        }
    }
    else
    {
        buttonPressStartMs    = 0;
        buttonWifiTriggered   = false;
        buttonRebootTriggered = false;
    }
}

// Handle the measurement acquisition from all enabled sensors
void Station::handleMeasurements()
{
    clearMeasurement(measurement);

    bool success = sensorManager.readAll(measurement, deviceConfig.enabledSensorsMask);

    #if DEBUG_ENABLE
    for (size_t i = 0; i < sensorManager.count(); i++)
    {
        if (!sensorManager.isEnabled(i, deviceConfig.enabledSensorsMask)) continue;

        const Sensor& sensor = sensorManager.at(i);
        Serial.println(sensorManager.lastReadSucceeded(i) ? sensor.displayValue(measurement)
                                                          : (String(sensor.getName()) + ": failed to read").c_str());
    }
    #endif

    measurement.timestamp  = systemClock.now();
    measurement.acquiredMs = millis();

    status.sensorOK = success;

    dataSize          = serialize(measurement, buffer, sizeof(buffer));
    encryptedDataSize = encrypt(buffer, dataSize, deviceConfig.aesKey, encryptedBuffer, measurement.timestamp);
}

// Handle the accumulation of measurements for later processing
void Station::handleAccumulator()
{
    accumulator.add(measurement);

    #if DEBUG_ENABLE
    accumulator.printDebugInfo();
    #endif
}

// Handle the Bluetooth communication loop, including sending data to connected clients
void Station::handleBluetooth()
{
    if (!isCommEnabled(deviceConfig.enabledCommsMask, BLUETOOTH_BIT))
    {
        status.bluetoothOK = true;
        return;
    }

    if (!bluetooth.isInitialized() || dataSize == 0)
    {
        status.bluetoothOK = false;
        return;
    }

    if (!bluetooth.hasConnectedClient())
    {
        status.bluetoothOK = true;
        return;
    }

    status.bluetoothOK = bluetooth.send(encryptedBuffer, encryptedDataSize);

    #if DEBUG_ENABLE
    if (status.bluetoothOK)
    {
        Serial.println("Data sent over Bluetooth.");
    }
    else
    {
        Serial.println("Failed to send data over Bluetooth.");
    }
    #endif
}

void Station::handleWatchdog()
{
    esp_task_wdt_reset();
}

// Restart the device once a scheduled reboot is due
void Station::handleReboot()
{
    if (rebootAtMs != 0 && millis() >= rebootAtMs)
    {
        ESP.restart();
    }
}