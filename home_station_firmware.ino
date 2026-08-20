#include <Arduino.h>

#include "src/core/measurement.hpp"
#include "src/core/pins.hpp"
#include "src/core/storage.hpp"
#include "src/core/configuration_manager.hpp"
#include "src/core/alarm_manager.hpp"
#include "src/core/clock.hpp"
#include "src/sensors/BH1750/driver_BH1750.hpp"
#include "src/sensors/HC-SR501/driver_HC-SR501.hpp"
#include "src/sensors/SCD41/driver_SCD41.hpp"
#include "src/sensors/MAX9814/driver_MAX9814.hpp"
#include "src/sensors/FC-51/driver_FC-51.hpp"
#include "src/sensors/SW-420/driver_SW-420.hpp"
#include "src/sensors/MQ-2/driver_MQ-2.hpp"
#include "src/sensors/BMP280/driver_BMP280.hpp"
#include "src/sensors/SGP41/driver_SGP41.hpp"
#include "src/communication/data_serializer.hpp"
#include "src/communication/bluetooth/bluetooth.hpp"
#include "src/communication/wifi/wifi.hpp"
#include "src/communication/LoRaWAN/lorawan.hpp"
#include "src/interface/status_led.hpp"
#include "src/interface/display_SSD1306.hpp"
#include "src/interface/button.hpp"
#include "src/interface/buzzer.hpp"

#define DEBUG_ENABLE 1

constexpr const char* STORAGE_NAMESPACE = "HomeStation";

Clock systemClock;

// Memory handle
Storage storage;

// Declaration of communcation interfaces
extern HardwareSerial Serial;
BluetoothCommunication bluetooth;
WiFiCommunication wifi;

// Declaration of elements for the interface with the user (LEDs, buttons, screens, etc.)
StatusLED statusLED(STATUS_LED_RED_PIN, STATUS_LED_GREEN_PIN, STATUS_LED_BLUE_PIN);
DisplaySSD1306 screen(systemClock);
Button button(SCREEN_BUTTON_PIN);
Buzzer buzzer(BUZZER_PIN, BuzzerType::PASSIVE);
AlarmManager alarmManager(buzzer, systemClock);

// Declaration of all the sensors used in the project
BH1750Sensor  lightSensor;                   // Luminosity sensor
HCSR501Sensor motionSensor(HC_SR501_PIN);    // Motion sensor
SCD41Sensor   co2TempHumiSensor;             // CO2, temperature and humidity sensor
MAX9814Sensor soundSensor(MAX9814_PIN);      // Sound sensor
FC51Sensor    obstacleSensor(FC_51_PIN);     // Obstacle sensor
SW420Sensor   vibrationSensor(SW_420_PIN);      // Vibration sensor
MQ2Sensor     gasSensor(MQ2_PIN);               // Gas sensor
BMP280Sensor  pressureSensor;                    // Pressure sensor
SGP41Sensor   VocNoxSensor;                  // Air quality sensor (VOC and NOx)


// Array of pointers to the sensors used in the project
Sensor* sensors[] =
{
    &lightSensor,
    &motionSensor,
    &co2TempHumiSensor,
    &soundSensor,
    &obstacleSensor,
    &vibrationSensor,
    &gasSensor,
    &pressureSensor,
    &VocNoxSensor       // Important: this sensor must be placed after the SCD41 (it needs temp and humidity)
};

int sensorCount = Sensor::getSensorCount();

// Constants for task timing
constexpr unsigned long TASK_10_MS   = 10;
constexpr unsigned long TASK_100_MS  = 100; 
constexpr unsigned long TASK_1000_MS = 1000; 

// Timestamps for the last execution of tasks
unsigned long last10Ms   = 0;
unsigned long last100Ms  = 0;
unsigned long last1000Ms = 0; 

// Declaration of the measurement structure to hold sensor data
Measurement measurement;

void setup()
{
    Serial.begin(115200);
    while (!Serial) { delay(10); } // Wait for Serial to be ready
    delay(1000);

    statusLED.begin();
    statusLED.setState(StatusLED::State::STARTING);
    statusLED.update(); 
    screen.begin();
    button.begin();
    buzzer.begin();

    LoRaWANCommunication::RadioPins loraWANPins = {
        .nss  = SPI_NSS_PIN,
        .rst  = SX1276_RST_PIN,
        .dio0 = SX1276_DIO0_PIN,
        .dio1 = SX1276_DIO1_PIN
    };

    uint8_t appEui[8];
    uint8_t devEui[8];
    uint8_t appKey[16];
    String bleDeviceName;
    String serviceUUID, characteristicUUID, timeSyncUUID, alarmTargetUUID;
    String wifiApSSID, wifiApPassword; 

    storage.begin(STORAGE_NAMESPACE, false);
    checkConfigResetOnBoot(storage);
    loadOrCreateConfig(storage, "devEui", "Dev EUI", devEui, sizeof(devEui));
    loadOrCreateConfig(storage, "appEui", "App EUI", appEui, sizeof(appEui));
    loadOrCreateConfig(storage, "appKey", "App Key", appKey, sizeof(appKey));
    loadOrCreateConfig(storage, "bleName", "BLE Device Name", bleDeviceName, 31, false);
    loadOrCreateConfig(storage, "serUUID", "Service UUID", serviceUUID, 36);
    loadOrCreateConfig(storage, "charUUID", "Characteristic UUID", characteristicUUID, 36);
    loadOrCreateConfig(storage, "tSynUUID", "Time Sync Characteristic UUID", timeSyncUUID, 36);
    loadOrCreateConfig(storage, "alTaUUID", "Alarm Target Characteristic UUID", alarmTargetUUID, 36);
    loadOrCreateConfig(storage, "apSSID", "WiFi AP SSID", wifiApSSID, 32, false);
    loadOrCreateConfig(storage, "apPass", "WiFi AP Password", wifiApPassword, 64, false);

    bool alarmArmed = storage.getBool("alarmArmed", false);
    uint32_t alarmTargetEpoch = storage.getUInt("alarmTarget", 0);
    int8_t utcOffset = storage.getChar("utcOffset", 0);
    storage.end();

    systemClock.configure(utcOffset);
    wifi.configure(wifiApSSID, wifiApPassword);
    bluetooth.configure(bleDeviceName, serviceUUID, characteristicUUID, timeSyncUUID, alarmTargetUUID);

    lorawan = LoRaWANCommunication(loraWANPins, devEui, appEui, appKey);    //TODO meme principe que les autres (confgure)
    lorawan.setPayloadBuilder([](uint8_t* buf, uint8_t maxLen) -> uint8_t
    {
        return static_cast<uint8_t>(serialize(measurement, buf, maxLen));
    });
    lorawan.setAutoUplinkInterval(60); // Send data every 60 seconds

    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
    SPI.begin(SPI_SCK_PIN, SPI_MISO_PIN, SPI_MOSI_PIN, SPI_NSS_PIN);
    bluetooth.begin();
    lorawan.begin();
    wifi.begin();

    alarmManager.begin(alarmArmed, alarmTargetEpoch);
    alarmManager.onAlarmChanged([](bool armed, uint32_t targetEpoch)
    {
        storage.begin(STORAGE_NAMESPACE, false);
        storage.putBool("alarmArmed", armed);
        storage.putUInt("alarmTarget", targetEpoch);
        storage.end();
    });

    #if DEBUG_ENABLE
    Serial.println("========  HomeStation Firmware  ========");
    Serial.print("Number of sensors: ");
    Serial.println(sensorCount);

    if (bluetooth.isInitialized())
    {
        Serial.print("Bluetooth started. Device Name: ");
        Serial.println(bleDeviceName);
    }
    else
    {
        Serial.println("Bluetooth not started.");
    }

    if (wifi.isInitialized())
    {
        Serial.print("WiFi AP started. IP: ");
        Serial.println(WiFi.softAPIP());
    }
    else
    {
        Serial.println("WiFi AP not started.");
    }

    if (lorawan.isInitialized())
    {
        Serial.println("LoRaWAN used.");
    }
    else
    {
        Serial.println("LoRaWAN not used");
    }

    lorawan.onJoined([]()
    {
        Serial.println("LoRaWAN : Join reussi !");
    });

    lorawan.onDownlink([](uint8_t port, const uint8_t* data, uint8_t length)
    {
        Serial.printf("Downlink recu sur port %d\n", port);
    });

    for(int i = 0; i < sensorCount; i++)
    {
        if(!sensors[i]->begin())
        {
            #if DEBUG_ENABLE
                Serial.print("Error: Initialization failed for sensor: ");
                Serial.println(sensors[i]->getName());
            #endif
        }
        else
        {
            #if DEBUG_ENABLE
                Serial.print("OK : ");
                Serial.println(sensors[i]->getName());
            #endif
        }
    }

    for (uint8_t addr = 1; addr < 127; addr++)
    {
        Wire.beginTransmission(addr);
        if (Wire.endTransmission() == 0)
        {
            Serial.print("(I2C) Found peripheral at 0x");
            Serial.println(addr, HEX);
        }
    }

    Serial.println("Memory loaded : ");
    Serial.print("devEui: ");
    for (int i = 0; i < 8; i++)
    {
        Serial.print(devEui[i], HEX);
        if (i < 7) Serial.print(":");
    }
    Serial.println();
    Serial.print("appEui: ");
    for (int i = 0; i < 8; i++)
    {
        Serial.print(appEui[i], HEX);
        if (i < 7) Serial.print(":");
    }
    Serial.println();
    Serial.print("appKey: ");
    for (int i = 0; i < 16; i++)
    {
        Serial.print(appKey[i], HEX);
        if (i < 15) Serial.print(":");
    }
    Serial.println();
    Serial.print("bleDeviceName: ");
    Serial.println(bleDeviceName);
    Serial.print("serviceUUID: ");
    Serial.println(serviceUUID);
    Serial.print("characteristicUUID: ");
    Serial.println(characteristicUUID);
    Serial.print("timeSyncUUID: ");
    Serial.println(timeSyncUUID);
    Serial.print("alarmTargetUUID: ");
    Serial.println(alarmTargetUUID);
    Serial.print("alarmArmed: ");
    Serial.println(alarmArmed ? "true" : "false");
    Serial.print("alarmTargetEpoch: ");
    Serial.println(alarmTargetEpoch);
    Serial.print("utcOffset: ");
    Serial.println(utcOffset);

    Serial.println("========================================");
    #endif

    delay(500);
}


void loop()
{
    unsigned long now = millis();

    // Task executed every iteration
    if (lorawan.isInitialized())
    {
        lorawan.loop();
    }
    
    if (wifi.isInitialized())
    {
        wifi.loop();
    }

    // Task executed every 10 ms
    if (now - last10Ms >= TASK_10_MS)
    {
        last10Ms = now;

        soundSensor.update();
    }

    // Task executed every 100 ms
    if (now - last100Ms >= TASK_100_MS)
    {
        last100Ms = now;

        handle_button();
        handle_bluetooth();
        alarmManager.update();
        statusLED.update();
        button.update();
        screen.update(measurement);
        buzzer.update();
    }

    // Task executed every 1000 ms
    if (now - last1000Ms >= TASK_1000_MS)
    {
        last1000Ms = now;

        MeasurementStatus status = handle_measurements();
        status.lorawanOK = lorawan.isInitialized() && lorawan.isJoined();
        
        updateLedStatus(status);
    }
}

bool readSensors(Measurement& measurement)
{
    bool success = true;
    measurement.timestamp = systemClock.now();
    for(int i = 0; i < sensorCount; i++)
    {
        Sensor& sensor = *sensors[i];

        if(sensor.isInitialized())
        {
            bool successSensor = sensor.read(measurement);
            success &= successSensor;

            #if DEBUG_ENABLE
            Serial.println(sensor.displayValue(measurement));
            #endif
        }
        else
        {
            success = false;
        }
    }
    return success;
}

bool sendBluetooth(const uint8_t* buffer, size_t dataSize)
{
    if (!bluetooth.isInitialized() || dataSize == 0)
    {
        return false; // vraie erreur
    }

    if (!bluetooth.hasConnectedClient())
    {
        return true; // pas d'erreur, juste personne de connecté
    }
    bool success = bluetooth.send(const_cast<uint8_t*>(buffer), dataSize);

    #if DEBUG_ENABLE
    if (success) Serial.println("Data sent over Bluetooth.");
    #endif

    return success;
}

void updateLedStatus(MeasurementStatus status)
{
    if (!status.sensorOK && !status.bluetoothOK && !status.lorawanOK)
    {
        statusLED.setState(StatusLED::State::ERROR);
    }
    else if (!status.sensorOK)
    {
        statusLED.setState(StatusLED::State::WARNING_SENSOR);
    }
    else if (!status.bluetoothOK || !status.lorawanOK)
    {
        statusLED.setState(StatusLED::State::WARNING_COMMUNICATION);
    }
    else
    {
        statusLED.setState(StatusLED::State::OK);
    }
}

void handle_button()
{
    if (button.wasPressed())
    {
        if (buzzer.isActive())
        {
            alarmManager.dismiss(); 
        }
        else
        {
            screen.buttonPressed();
        }
    }
}

void handle_bluetooth()
{
    if (bluetooth.hasNewTimeSync())
    {
        systemClock.sync(bluetooth.consumeTimeSync());
    }
    if (bluetooth.hasNewAlarmTarget())
    {
        alarmManager.setAlarm(bluetooth.consumeAlarmTarget());
    }
}

MeasurementStatus handle_measurements()
{
    clearMeasurement(measurement);
    bool successR = readSensors(measurement);

    uint8_t buffer[BUFFER_SIZE];
    size_t dataSize = serialize(measurement, buffer, sizeof(buffer));

    bool successB = sendBluetooth(buffer, dataSize);

    return { successR, successB, false }; 
}