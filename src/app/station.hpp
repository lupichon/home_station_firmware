/**
 * @file    station.hpp
 * @brief   Assembles every module of the HomeStation firmware. Owns all the
 *          objects and wires them together (the only place allowed to know
 *          every module).
 * @author  Lucas Pichon
 * @date    2026-11-16
 */

#pragma once

#include <Arduino.h>

// ==================== Sensors ====================
#include "sensors/sensor_manager.hpp"
#include "sensors/BH1750/sensor.hpp"
#include "sensors/HC-SR501/sensor.hpp"
#include "sensors/SCD41/sensor.hpp"
#include "sensors/MAX9814/sensor.hpp"
#include "sensors/FC-51/sensor.hpp"
#include "sensors/SW-420/sensor.hpp"
#include "sensors/MQ-2/sensor.hpp"
#include "sensors/BMP280/sensor.hpp"
#include "sensors/SGP41/sensor.hpp"
#include "sensors/reed_switch/sensor.hpp"

// ==================== Communication ====================
#include "communication/bluetooth/bluetooth.hpp"
#include "communication/wifi/wifi.hpp"
#include "communication/wifi/config_api.hpp"
#include "communication/wifi/update_api.hpp"
#include "communication/LoRaWAN/lorawan.hpp"
#include "communication/data_serializer.hpp"
#include "communication/communications_bit.hpp"

// ========================= UI ==========================
#include "ui/screen/controller.hpp"
#include "ui/screen/SSD1306/driver.hpp"
#include "ui/screen/layout/layout_128x64.hpp"
#include "ui/led/led.hpp"
#include "ui/led/rgb_led.hpp"
#include "ui/led/status_led.hpp"
#include "ui/button.hpp"
#include "ui/buzzer.hpp"

// ==================== board ===================
#include "board/pins.hpp"

// ================== Platform ==================
#include "platform/storage/storage.hpp"
#include "platform/clock.hpp"
#include "platform/crypto.hpp"

// =================== Model ====================
#include "model/measurement.hpp"
#include "model/status.hpp"

// =================== Alarm ====================
#include "alarm/alarm_manager.hpp"

// =================== Config ===================
#include "config/config_manager.hpp"

// ==================== Utils ===================
#include "utils/periodic.hpp"

// ==================== Debug ====================
// Can be overridden from platformio.ini with -DDEBUG_ENABLE=1
#ifndef DEBUG_ENABLE
#define DEBUG_ENABLE 0
#endif

// ============================================================
// Constants
// ============================================================

namespace station_cst
{
    constexpr size_t SENSOR_COUNT = 10;

    // Task periods
    constexpr unsigned long TASK_10_MS    = 10;
    constexpr unsigned long TASK_100_MS   = 100;
    constexpr unsigned long TASK_1000_MS  = 1000;
    constexpr unsigned long TASK_60000_MS = 60000;

    // LoRaWAN
    constexpr unsigned long LORAWAN_AUTO_UPLINK_INTERVAL_S = 600;
    constexpr unsigned long LORAWAN_FIRST_UPLINK_UPTIME_S  = 1800;
}

// ============================================================
// Station class definition
// ============================================================

class Station
{
    // ── Public interface ────────────────────────────────────────────────────
    public:
        Station();

        // The station holds pointers/references to its own members
        Station(const Station&) = delete;
        Station& operator=(const Station&) = delete;

        /** @brief Initialize every module (call once from Arduino setup()). */
        void setup();

        /** @brief Run one iteration of the main loop (call from Arduino loop()). */
        void loop();

    // ── Private methods ─────────────────────────────────────────────────────
    private:
        // ---- Helpers (station.cpp) ----
        bool isMeasurementStale(unsigned long now);
        bool isLiveDataRequested();
        void scheduleReboot(unsigned long delayMs);

        // ---- Initialization (station_init.cpp) ----
        void initSerial();
        void initInterface();
        void initConfig();
        void initClock();
        void initAlarmManager();
        void initCommunication();
        void initSensors();
        void initWatchdog();

        // ---- Callbacks (station_callbacks.cpp) ----
        void setAlarmManagerCallbacks();
        void setWifiCallbacks();
        void setBluetoothCallbacks();
        void setLoRaWANCallbacks();

        // ---- Periodic tasks (station_tasks.cpp) ----
        void handleWatchdog();
        void handleReboot();
        void handleAlarmManager();
        void handleScreen();
        void handleBuzzer();
        void handleSensorsUpdate();
        void handleWifi();
        void handleLoRaWAN();
        void handleLED();
        void handleButton();
        void handleMeasurements();
        void handleAccumulator();
        void handleBluetooth();

        // ---- Debug (station_debug.cpp) ----
        #if DEBUG_ENABLE
        void printDebugInfo();
        #endif

    // ── Private members ─────────────────────────────────────────────────────
    private:
        // ---- System ----
        Clock   systemClock;
        Storage storage;

        // ---- UI ----
        SSD1306          ssd1306;
        Layout128x64     screenLayout;
        ScreenController screen;
        LED              ledRed;
        LED              ledGreen;
        LED              ledBlue;
        RgbLED           rgbLed;
        StatusLED        statusLED;
        Button           button;
        Buzzer           buzzer;
        AlarmManager     alarmManager;

        // ---- Communication ----
        BluetoothCommunication bluetooth;
        WiFiCommunication      wifi;
        LoRaWANCommunication   lorawan;

        // ---- Sensors ----
        BH1750Sensor     lightSensor;          // Luminosity
        HCSR501Sensor    motionSensor;         // Motion
        SCD41Sensor      co2TempHumiSensor;    // CO2, temperature, humidity
        MAX9814Sensor    soundSensor;          // Sound
        FC51Sensor       obstacleSensor;       // Obstacle
        SW420Sensor      vibrationSensor;      // Vibration
        MQ2Sensor        gasSensor;            // Gas
        BMP280Sensor     pressureSensor;       // Pressure
        SGP41Sensor      vocNoxSensor;         // VOC and NOx, compensated by the SCD41
        ReedSwitchSensor doorSensor;           // Door state, received over Bluetooth

        Sensor*       sensors[station_cst::SENSOR_COUNT];
        SensorManager sensorManager;

        // ---- Data ----
        Measurement            measurement{};
        MeasurementAccumulator accumulator;
        Status                 status{};
        DeviceConfig           deviceConfig;
        ConfigManager          configManager;
        ConfigApi              configApi;
        UpdateApi              updateApi;

        uint8_t buffer[BUFFER_SIZE] = {};
        size_t  dataSize = 0;

        uint8_t encryptedBuffer[AES_IV_SIZE + BUFFER_SIZE] = {};
        size_t  encryptedDataSize = 0;

        // ---- Scheduling ----
        Periodic task10Ms    {station_cst::TASK_10_MS,    0};
        Periodic task100Ms   {station_cst::TASK_100_MS,   0};
        Periodic task1000Ms  {station_cst::TASK_1000_MS,  0};
        Periodic task60000Ms {station_cst::TASK_60000_MS, 0};

        unsigned long rebootAtMs = 0;   // millis() at which to restart, 0 when no restart is pending

        // ---- Button state ----
        unsigned long buttonPressStartMs    = 0;
        bool          buttonWifiTriggered   = false;
        bool          buttonRebootTriggered = false;
};