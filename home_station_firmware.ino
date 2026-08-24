#include <Arduino.h>

#include "src/core/measurement.hpp"
#include "src/core/status.hpp"
#include "src/core/pins.hpp"
#include "src/core/storage.hpp"
#include "src/core/configuration_manager.hpp"
#include "src/core/alarm_manager.hpp"
#include "src/core/clock.hpp"
#include "src/core/device_config.hpp"
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

Clock systemClock;

// Memory handle
Storage storage;

// Declaration of communcation interfaces
extern HardwareSerial Serial;
BluetoothCommunication bluetooth;
WiFiCommunication wifi;
//ajout de lorawan

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

Communication* communications[] = 
{
    &bluetooth,
    &wifi,
    &lorawan
};

int sensorCount = Sensor::getSensorCount();
int commCount = sizeof(communications) / sizeof(communications[0]);

// Constants for task timing
constexpr unsigned long TASK_10_MS   = 10;
constexpr unsigned long TASK_100_MS  = 100; 
constexpr unsigned long TASK_1000_MS = 1000; 

// Timestamps for the last execution of tasks
unsigned long last10Ms   = 0;
unsigned long last100Ms  = 0;
unsigned long last1000Ms = 0; 

// Declaration of structures to hold data and status
Measurement measurement;
Status status;
DeviceConfig deviceConfig;

uint8_t buffer[BUFFER_SIZE];
size_t dataSize = 0;

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

    storage.begin(Storage::STORAGE_NAMESPACE, false);
    checkConfigResetOnBoot(storage);
    loadOrCreateConfig(storage, Storage::devEUIKey, "Dev EUI", deviceConfig.devEui, sizeof(deviceConfig.devEui));
    loadOrCreateConfig(storage, Storage::appEUIKey, "App EUI", deviceConfig.appEui, sizeof(deviceConfig.appEui));
    loadOrCreateConfig(storage, Storage::appKeyKey, "App Key", deviceConfig.appKey, sizeof(deviceConfig.appKey));
    loadOrCreateConfig(storage, Storage::bleNameKey, "BLE Device Name", deviceConfig.bleDeviceName, 31, false);
    loadOrCreateConfig(storage, Storage::serviceUUIDKey, "Service UUID", deviceConfig.serviceUUID, 36);
    loadOrCreateConfig(storage, Storage::characteristicUUIDKey, "Characteristic UUID", deviceConfig.characteristicUUID, 36);
    loadOrCreateConfig(storage, Storage::timeSyncUUIDKey, "Time Sync Characteristic UUID", deviceConfig.timeSyncUUID, 36);
    loadOrCreateConfig(storage, Storage::alarmTargetUUIDKey, "Alarm Target Characteristic UUID", deviceConfig.alarmTargetUUID, 36);
    loadOrCreateConfig(storage, Storage::wifiApSSIDKey, "WiFi AP SSID", deviceConfig.wifiApSSID, 32, false);
    loadOrCreateConfig(storage, Storage::wifiApPasswordKey, "WiFi AP Password", deviceConfig.wifiApPassword, 64, false);

    bool alarmArmed = storage.getBool(Storage::alarmArmedKey, false);
    uint32_t alarmTargetEpoch = storage.getUInt(Storage::alarmTargetKey, 0);
    deviceConfig.utcOffset = storage.getChar(Storage::utcOffsetKey, 0);
    deviceConfig.enabledSensorsMask = static_cast<uint16_t>(storage.getUInt(Storage::enabledSensorsMaskKey, 0xFFFF));
    storage.end();

    systemClock.configure(deviceConfig.utcOffset);
    wifi.configure(deviceConfig.wifiApSSID, deviceConfig.wifiApPassword);
    bluetooth.configure(deviceConfig.bleDeviceName, deviceConfig.serviceUUID, deviceConfig.characteristicUUID, deviceConfig.timeSyncUUID, deviceConfig.alarmTargetUUID);
    //lorawan.configure(...)
    lorawan = LoRaWANCommunication(loraWANPins, deviceConfig.devEui, deviceConfig.appEui, deviceConfig.appKey);    //TODO meme principe que les autres (confgure)
    lorawan.setPayloadBuilder([](uint8_t* buf, uint8_t maxLen) -> uint8_t
    {
        return static_cast<uint8_t>(serialize(measurement, buf, maxLen));
    });
    lorawan.setAutoUplinkInterval(60); // Send data every 60 seconds

    setAlarmManagerCallbacks();
    setWifiCallbacks();
    setBluetoothCallbacks();
    //setLorawanCallbacks()

    alarmManager.begin(alarmArmed, alarmTargetEpoch);
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
    SPI.begin(SPI_SCK_PIN, SPI_MISO_PIN, SPI_MOSI_PIN, SPI_NSS_PIN);
    for (int i = 0; i < commCount; i++)
    {
        communications[i]->begin();
    }

    for(int i = 0; i < sensorCount; i++)
    {
        if (!((deviceConfig.enabledSensorsMask >> i) & 0x01)) continue; 

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

    #if DEBUG_ENABLE
    Serial.println("========  HomeStation Firmware  ========");
    Serial.print("Number of sensors: ");
    Serial.println(sensorCount);

    if (bluetooth.isInitialized())
    {
        Serial.print("Bluetooth started. Device Name: ");
        Serial.println(deviceConfig.bleDeviceName);
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
        Serial.print(deviceConfig.devEui[i], HEX);
        if (i < 7) Serial.print(":");
    }
    Serial.println();
    Serial.print("appEui: ");
    for (int i = 0; i < 8; i++)
    {
        Serial.print(deviceConfig.appEui[i], HEX);
        if (i < 7) Serial.print(":");
    }
    Serial.println();
    Serial.print("appKey: ");
    for (int i = 0; i < 16; i++)
    {
        Serial.print(deviceConfig.appKey[i], HEX);
        if (i < 15) Serial.print(":");
    }
    Serial.println();
    Serial.print("bleDeviceName: ");
    Serial.println(deviceConfig.bleDeviceName);
    Serial.print("serviceUUID: ");
    Serial.println(deviceConfig.serviceUUID);
    Serial.print("characteristicUUID: ");
    Serial.println(deviceConfig.characteristicUUID);
    Serial.print("timeSyncUUID: ");
    Serial.println(deviceConfig.timeSyncUUID);
    Serial.print("alarmTargetUUID: ");
    Serial.println(deviceConfig.alarmTargetUUID);
    Serial.print("alarmArmed: ");
    Serial.println(alarmArmed ? "true" : "false");
    Serial.print("alarmTargetEpoch: ");
    Serial.println(alarmTargetEpoch);
    Serial.print("utcOffset: ");
    Serial.println(deviceConfig.utcOffset);
    Serial.print("enabledSensorsMask: ");
    Serial.println(deviceConfig.enabledSensorsMask, BIN);

    Serial.println("========================================");
    #endif

    delay(500);
}


void loop()
{
    unsigned long now = millis();

    // Task executed every iteration
    handleLoRaWAN();
    handleWifi();

    // Task executed every 10 ms
    if (now - last10Ms >= TASK_10_MS)
    {
        last10Ms = now;

        handleSoundSensor();
    }

    // Task executed every 100 ms
    if (now - last100Ms >= TASK_100_MS)
    {
        last100Ms = now;

        handleAlarmManager();
        handleScreen();
        handleBuzzer();
        handleButton();
        handleLED();
    }

    // Task executed every 1000 ms
    if (now - last1000Ms >= TASK_1000_MS)
    {
        last1000Ms = now;
        
        status.lorawanOK = lorawan.isInitialized() && lorawan.isJoined();

        handleMeasurements();
        handleBluetooth();
    }
}

void handleAlarmManager()
{
    alarmManager.update();
}

void handleScreen()
{
    screen.update(measurement);
}

void handleBuzzer()
{
    buzzer.update();
}

void handleSoundSensor()
{
    if (!((deviceConfig.enabledSensorsMask >> 3) & 0x01)) return; 
    soundSensor.update();
}

void handleWifi()
{
    if (wifi.isInitialized())
    {
        wifi.loop();
    }
}

void handleLoRaWAN()
{
    if (lorawan.isInitialized())
    {
        lorawan.loop();
    }
}

void handleLED()
{
    statusLED.update();

    if (!status.sensorOK && !status.lorawanOK)
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

    statusLED.setIndicator(StatusLED::Indicator::BLUETOOTH_CONNECTED, bluetooth.hasConnectedClient());
    statusLED.setIndicator(StatusLED::Indicator::WIFI_CONNECTED, wifi.hasConnectedClient());
    statusLED.setIndicator(StatusLED::Indicator::ALARM_TRIGGERED, alarmManager.isRinging());
    statusLED.setIndicator(StatusLED::Indicator::ALARM_ARMED, alarmManager.isArmed());
    statusLED.setIndicator(StatusLED::Indicator::BUTTON_HELD, button.isHeld());
    statusLED.setIndicator(StatusLED::Indicator::TIME_SYNCED, systemClock.isSynchronized() && systemClock.isSynchronizedSince() <= 30);
}

void handleButton()
{
    static unsigned long pressStartMs    = 0;
    static bool          rebootTriggered = false;
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
        if (pressStartMs == 0)
        {
            pressStartMs = millis(); 
        }
        else if (!rebootTriggered && millis() - pressStartMs >= REBOOT_HOLD_TIME_MS)
        {
            rebootTriggered = true;
            #if DEBUG_ENABLE
            Serial.println("Bouton maintenu 10s : reboot du systeme.");
            #endif
            delay(1000);
            ESP.restart();
        }
    }
    else
    {
        pressStartMs    = 0;
        rebootTriggered = false;
    }
}

void handleMeasurements()
{
    clearMeasurement(measurement);

    bool success = true;
    measurement.timestamp = systemClock.now();
    for(int i = 0; i < sensorCount; i++)
    {
        bool sensorEnabled = (deviceConfig.enabledSensorsMask >> i) & 0x01;
        if (!sensorEnabled) continue;

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
    
    status.sensorOK = success;

    dataSize = serialize(measurement, buffer, sizeof(buffer));
}

void handleBluetooth()
{
    bool success;
    if (!bluetooth.isInitialized() || dataSize == 0)
    {
        status.bluetoothOK = false; // vraie erreur
        return;
    }

    if (!bluetooth.hasConnectedClient())
    {
        status.bluetoothOK = true; // pas d'erreur, juste personne de connecté
        return;
    }

    success = bluetooth.send(const_cast<uint8_t*>(buffer), dataSize);

    #if DEBUG_ENABLE
    if (success) Serial.println("Data sent over Bluetooth.");
    #endif

    status.bluetoothOK = success;
}

void setAlarmManagerCallbacks()
{
    alarmManager.onAlarmChanged([](bool armed, uint32_t targetEpoch)
    {
        storage.begin(Storage::STORAGE_NAMESPACE, false);
        storage.putBool(Storage::alarmArmedKey, armed);
        storage.putUInt(Storage::alarmTargetKey, targetEpoch);
        storage.end();
    });
}

void setWifiCallbacks()
{
   wifi.setConfigTarget(&deviceConfig);

    wifi.setSensorInfoProvider([]() -> String
    {
        String json = "[";
        for (int i = 0; i < sensorCount; i++)
        {
            if (i > 0) json += ",";
            json += "\"" + String(sensors[i]->getName()) + "\"";
        }
        json += "]";
        return json;
    });


   wifi.setOnConfigSaved([](const DeviceConfig& newDeviceConfig) 
   {
        storage.begin(Storage::STORAGE_NAMESPACE, false);

        if (memcmp(newDeviceConfig.devEui, deviceConfig.devEui, sizeof(newDeviceConfig.devEui)) != 0)
        {
            storage.putBytes(Storage::devEUIKey, newDeviceConfig.devEui, sizeof(newDeviceConfig.devEui));
            memcpy(deviceConfig.devEui, newDeviceConfig.devEui, sizeof(deviceConfig.devEui));
        }

        if (memcmp(newDeviceConfig.appEui, deviceConfig.appEui, sizeof(newDeviceConfig.appEui)) != 0)
        {
            storage.putBytes(Storage::appEUIKey, newDeviceConfig.appEui, sizeof(newDeviceConfig.appEui));
            memcpy(deviceConfig.appEui, newDeviceConfig.appEui, sizeof(deviceConfig.appEui));
        }

        if (memcmp(newDeviceConfig.appKey, deviceConfig.appKey, sizeof(newDeviceConfig.appKey)) != 0)
        {
            storage.putBytes(Storage::appKeyKey, newDeviceConfig.appKey, sizeof(newDeviceConfig.appKey));   
            memcpy(deviceConfig.appKey, newDeviceConfig.appKey, sizeof(deviceConfig.appKey));
        }
            
        if (newDeviceConfig.bleDeviceName != deviceConfig.bleDeviceName)
        {
            storage.putString(Storage::bleNameKey, newDeviceConfig.bleDeviceName);
            deviceConfig.bleDeviceName = newDeviceConfig.bleDeviceName;
        }

        if (newDeviceConfig.serviceUUID != deviceConfig.serviceUUID)
        {
            storage.putString(Storage::serviceUUIDKey, newDeviceConfig.serviceUUID);
            deviceConfig.serviceUUID = newDeviceConfig.serviceUUID;
        }

        if (newDeviceConfig.characteristicUUID != deviceConfig.characteristicUUID)
        {
            storage.putString(Storage::characteristicUUIDKey, newDeviceConfig.characteristicUUID);
            deviceConfig.characteristicUUID = newDeviceConfig.characteristicUUID;
        }

        if (newDeviceConfig.timeSyncUUID != deviceConfig.timeSyncUUID)
        {
            storage.putString(Storage::timeSyncUUIDKey, newDeviceConfig.timeSyncUUID);
            deviceConfig.timeSyncUUID = newDeviceConfig.timeSyncUUID;
        }

        if (newDeviceConfig.alarmTargetUUID != deviceConfig.alarmTargetUUID)
        {
            storage.putString(Storage::alarmTargetUUIDKey, newDeviceConfig.alarmTargetUUID);
            deviceConfig.alarmTargetUUID = newDeviceConfig.alarmTargetUUID;
        }

        if (newDeviceConfig.wifiApSSID != deviceConfig.wifiApSSID)
        {
            storage.putString(Storage::wifiApSSIDKey, newDeviceConfig.wifiApSSID);
            deviceConfig.wifiApSSID = newDeviceConfig.wifiApSSID;
        }

        if (newDeviceConfig.wifiApPassword != deviceConfig.wifiApPassword)
        {
            storage.putString(Storage::wifiApPasswordKey, newDeviceConfig.wifiApPassword);
            deviceConfig.wifiApPassword = newDeviceConfig.wifiApPassword;
        }

        if (newDeviceConfig.utcOffset != deviceConfig.utcOffset)
        {
            storage.putChar(Storage::utcOffsetKey, newDeviceConfig.utcOffset);
            deviceConfig.utcOffset = newDeviceConfig.utcOffset;
        }

        if (newDeviceConfig.enabledSensorsMask != deviceConfig.enabledSensorsMask)
        {
            storage.putUInt(Storage::enabledSensorsMaskKey, static_cast<uint32_t>(newDeviceConfig.enabledSensorsMask));
            deviceConfig.enabledSensorsMask = newDeviceConfig.enabledSensorsMask;
        }
    
        storage.end();
    });
}

void setBluetoothCallbacks()
{
    bluetooth.setTimeSyncCallback([](uint32_t epoch)
    {
        systemClock.sync(epoch);
    });

    bluetooth.setAlarmTargetCallback([](uint32_t targetEpoch)
    {
        alarmManager.setAlarm(targetEpoch);
    });
}