/**
 * @file    station.cpp
 * @brief   Station construction, setup/loop entry points and small helpers.
 * @author  Lucas Pichon
 * @date    2026-11-16
 */

#include "station.hpp"

using namespace station_cst;

// ============================================================
// Construction
// ============================================================

Station::Station()
    : screen(ssd1306, screenLayout, systemClock),
      ledRed(STATUS_LED_RED_PIN, 0),
      ledGreen(STATUS_LED_GREEN_PIN, 1),
      ledBlue(STATUS_LED_BLUE_PIN, 2),
      rgbLed(ledRed, ledGreen, ledBlue),
      statusLED(rgbLed),
      button(SCREEN_BUTTON_PIN),
      buzzer(BUZZER_PIN, BuzzerType::PASSIVE),
      alarmManager(buzzer, systemClock),
      motionSensor(HC_SR501_PIN),
      soundSensor(MAX9814_PIN),
      obstacleSensor(FC_51_PIN),
      vibrationSensor(SW_420_PIN),
      gasSensor(MQ2_PIN),
      vocNoxSensor(co2TempHumiSensor),
      sensors{
          &lightSensor,
          &motionSensor,
          &co2TempHumiSensor,
          &soundSensor,
          &obstacleSensor,
          &vibrationSensor,
          &gasSensor,
          &pressureSensor,
          &vocNoxSensor,
          &doorSensor
      },
      sensorManager(sensors, SENSOR_COUNT),
      configManager(storage, deviceConfig),
      configApi(deviceConfig)
{
}

// ============================================================
// Entry points
// ============================================================

void Station::setup()
{
    initWatchdog();
    initInterface();
    initSerial();
    initConfig();
    initClock();
    initAlarmManager();
    initCommunication();
    initSensors();

    #if DEBUG_ENABLE
    printDebugInfo();
    #endif

    delay(1000);
}

void Station::loop()
{
    unsigned long now = millis();

    // Task executed every iteration
    handleWatchdog();
    handleReboot();
    handleLoRaWAN();
    handleWifi();
    handleSensorsUpdate();

    // Task executed every 10 ms
    if (task10Ms.due(now))
    {

    }

    // Task executed every 100 ms
    if (task100Ms.due(now))
    {
        handleAlarmManager();
        handleScreen();
        handleBuzzer();
        handleButton();
        handleLED();
    }

    // Task executed every 1000 ms
    if (task1000Ms.due(now))
    {
        if (isLiveDataRequested())
        {
            handleMeasurements();
        }
        handleBluetooth();
    }

    // Task executed every 60000 ms
    if (task60000Ms.due(now))
    {
        if (isCommEnabled(deviceConfig.enabledCommsMask, LORAWAN_BIT))
        {
            if (isMeasurementStale(now))
            {
                handleMeasurements();
            }
            handleAccumulator();
        }
    }
}

// ============================================================
// Helpers
// ============================================================

// Check if the last measurement is stale based on the current time
bool Station::isMeasurementStale(unsigned long now)
{
    constexpr unsigned long MEASUREMENT_MAX_AGE_MS = 1500; // 1.5 seconds
    return (now - measurement.acquiredMs) > MEASUREMENT_MAX_AGE_MS;
}

// Check if live data is requested from any communication interface or the screen is active
bool Station::isLiveDataRequested()
{
    return (isCommEnabled(deviceConfig.enabledCommsMask, BLUETOOTH_BIT) && bluetooth.hasConnectedClient())
        || (isCommEnabled(deviceConfig.enabledCommsMask, WIFI_BIT)      && wifi.hasConnectedClient())
        || !screen.isSleeping()
        || DEBUG_ENABLE;
}

// Restart after a delay, so that the HTTP response has time to be sent
void Station::scheduleReboot(unsigned long delayMs)
{
    rebootAtMs = millis() + delayMs;
}