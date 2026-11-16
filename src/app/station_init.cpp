/**
 * @file    station_init.cpp
 * @brief   Initialization functions of the Station.
 * @author  Lucas Pichon
 * @date    2026-11-16
 */

#include "station.hpp"

#include <Wire.h>
#include <SPI.h>
#include <esp_task_wdt.h>

using namespace station_cst;

// Initialize Serial communication
void Station::initSerial()
{
    Serial.begin(115200);
    while (!Serial) { delay(10); } // Wait for Serial to be ready
}

// Initialize interface components: status LED, screen, button, and buzzer
void Station::initInterface()
{
    statusLED.begin();
    statusLED.setState(StatusLED::State::STARTING);
    statusLED.update();

    screen.begin();
    button.begin();
    buzzer.begin();
}

// Load the device configuration from storage (defaults for missing entries)
void Station::initConfig()
{
    configManager.load();
}

// Initialize system clock with UTC offset from device configuration
void Station::initClock()
{
    systemClock.configure(deviceConfig.utcOffset);
}

// Initialize alarm manager with armed state and target epoch from device configuration
void Station::initAlarmManager()
{
    alarmManager.begin(deviceConfig.alarmArmed, deviceConfig.alarmTargetEpoch);
    setAlarmManagerCallbacks();
}

// Initialize communication interfaces: WiFi, Bluetooth, and LoRaWAN
void Station::initCommunication()
{
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
    SPI .begin(SPI_SCK_PIN, SPI_MISO_PIN, SPI_MOSI_PIN, SPI_NSS_PIN);

    if (isCommEnabled(deviceConfig.enabledCommsMask, BLUETOOTH_BIT))
    {
        bluetooth.configure(deviceConfig.bleDeviceName,
                            deviceConfig.serviceUUID,
                            deviceConfig.characteristicUUID,
                            deviceConfig.timeSyncUUID,
                            deviceConfig.alarmTargetUUID,
                            deviceConfig.wifiControlUUID,
                            deviceConfig.doorStateUUID);
        setBluetoothCallbacks();
        bluetooth.begin();
    }

    if (isCommEnabled(deviceConfig.enabledCommsMask, LORAWAN_BIT))
    {
        lorawanPtr = &lorawan; // Pointer to the LoRaWANCommunication instance for LMIC callbacks
        lorawan.configure(SPI_NSS_PIN, SX1276_RST_PIN, SX1276_DIO0_PIN, SX1276_DIO1_PIN,
                          deviceConfig.devEui, deviceConfig.appEui, deviceConfig.appKey,
                          [this](uint8_t* buf, uint8_t maxLen) -> uint8_t
                          {
                              if (accumulator.empty()) return 0;

                              Measurement avg = accumulator.average();
                              size_t len = serialize(avg, buf, maxLen);
                              return static_cast<uint8_t>(len);
                          },
                          LORAWAN_AUTO_UPLINK_INTERVAL_S,
                          LORAWAN_FIRST_UPLINK_UPTIME_S);
        setLoRaWANCallbacks();
        lorawan.begin();
    }

    if (isCommEnabled(deviceConfig.enabledCommsMask, WIFI_BIT))
    {
        wifi.configure(deviceConfig.wifiApSSID, deviceConfig.wifiApPassword);
        setWifiCallbacks();
        wifi.begin();
    }
}

// Initialize all sensors based on the enabled sensors mask in the device configuration
void Station::initSensors()
{
    pressureSensor.setAltitude(deviceConfig.altitudeM);

    sensorManager.begin(deviceConfig.enabledSensorsMask);

    #if DEBUG_ENABLE
    for (size_t i = 0; i < sensorManager.count(); i++)
    {
        if (!sensorManager.isEnabled(i, deviceConfig.enabledSensorsMask)) continue;

        Serial.print(sensorManager.at(i).isInitialized() ? "OK : " : "Error: Initialization failed for sensor: ");
        Serial.println(sensorManager.at(i).getName());
    }
    #endif
}

// Initialize the watchdog timer to automatically reset the ESP if it becomes unresponsive.
void Station::initWatchdog()
{
    constexpr int WATCHDOG_TIMEOUT_S = 30;
    esp_task_wdt_init(WATCHDOG_TIMEOUT_S, true);
    esp_task_wdt_add(NULL);
}