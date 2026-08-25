#include <Arduino.h>

// ==================== Core ====================
#include "src/core/measurement.hpp"
#include "src/core/status.hpp"
#include "src/core/pins.hpp"
#include "src/core/storage.hpp"
#include "src/core/configuration_manager.hpp"
#include "src/core/alarm_manager.hpp"
#include "src/core/clock.hpp"
#include "src/core/device_config.hpp"

// ==================== Sensors ====================
#include "src/sensors/BH1750/driver_BH1750.hpp"
#include "src/sensors/HC-SR501/driver_HC-SR501.hpp"
#include "src/sensors/SCD41/driver_SCD41.hpp"
#include "src/sensors/MAX9814/driver_MAX9814.hpp"
#include "src/sensors/FC-51/driver_FC-51.hpp"
#include "src/sensors/SW-420/driver_SW-420.hpp"
#include "src/sensors/MQ-2/driver_MQ-2.hpp"
#include "src/sensors/BMP280/driver_BMP280.hpp"
#include "src/sensors/SGP41/driver_SGP41.hpp"

// ==================== Communication ====================
#include "src/communication/data_serializer.hpp"
#include "src/communication/bluetooth/bluetooth.hpp"
#include "src/communication/wifi/wifi.hpp"
#include "src/communication/LoRaWAN/lorawan.hpp"

// ==================== Interface ====================
#include "src/interface/status_led.hpp"
#include "src/interface/display_SSD1306.hpp"
#include "src/interface/button.hpp"
#include "src/interface/buzzer.hpp"

// ==================== Debug ====================
#define DEBUG_ENABLE 1

// ==================== System ====================
extern  HardwareSerial Serial;
Clock   systemClock;
Storage storage;

// ==================== Interface ====================
StatusLED      statusLED(STATUS_LED_RED_PIN, STATUS_LED_GREEN_PIN, STATUS_LED_BLUE_PIN);
DisplaySSD1306 screen(systemClock);
Button         button(SCREEN_BUTTON_PIN);
Buzzer         buzzer(BUZZER_PIN, BuzzerType::PASSIVE);
AlarmManager   alarmManager(buzzer, systemClock);

// ==================== Communication ====================
BluetoothCommunication bluetooth;
WiFiCommunication      wifi;
//LoRaWANCommunication   lorawan;

Communication* communications[] =
{
    &bluetooth,
    &wifi,
    &lorawan
};

// ==================== Sensors ====================
BH1750Sensor  lightSensor;                        // Luminosity
HCSR501Sensor motionSensor(HC_SR501_PIN);         // Motion
SCD41Sensor   co2TempHumiSensor;                  // CO2, temperature, humidity
MAX9814Sensor soundSensor(MAX9814_PIN);           // Sound
FC51Sensor    obstacleSensor(FC_51_PIN);          // Obstacle
SW420Sensor   vibrationSensor(SW_420_PIN);        // Vibration
MQ2Sensor     gasSensor(MQ2_PIN);                 // Gas
BMP280Sensor  pressureSensor;                     // Pressure
SGP41Sensor   VocNoxSensor;                       // VOC and NOx (must be after SCD41)

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
    &VocNoxSensor // Must be after SCD41
};

// ==================== Data ====================
Measurement     measurement;
Status          status;
DeviceConfig    deviceConfig;

uint8_t buffer[BUFFER_SIZE];
size_t  dataSize = 0;

// ==================== Task timing ====================
constexpr unsigned long TASK_10_MS   = 10;
constexpr unsigned long TASK_100_MS  = 100;
constexpr unsigned long TASK_1000_MS = 1000;

unsigned long last10Ms   = 0;
unsigned long last100Ms  = 0;
unsigned long last1000Ms = 0;

// ==================== Counts ====================
int sensorCount = Sensor::getSensorCount();
int commCount   = Communication::getCommunicationCount();

// ==================== Setup and Loop ====================
void setup()
{
    initSerial();
    initInterface();
    initStorage();
    initClock();
    initAlarmManager();
    initCommunication();
    initSensors();

    #if DEBUG_ENABLE
    printDebugInfo();
    #endif

    delay(1000);
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

// =================== Initialization Functions ====================

// Initialize Serial communication 
void initSerial()
{
    Serial.begin(115200);
    while (!Serial) { delay(10); } // Wait for Serial to be ready
}

// Initialize interface components: status LED, screen, button, and buzzer
void initInterface()
{
    statusLED.begin();
    statusLED.setState(StatusLED::State::STARTING);
    statusLED.update();

    screen.begin();
    button.begin();
    buzzer.begin();
}

// Initialize storage and load or create device configuration
void initStorage()
{
    storage.begin(Storage::STORAGE_NAMESPACE, false);
    checkConfigResetOnBoot(storage);

    loadOrCreateConfig(storage, Storage::devEUIKey,              "Dev EUI",                          deviceConfig.devEui,            sizeof(deviceConfig.devEui));
    loadOrCreateConfig(storage, Storage::appEUIKey,              "App EUI",                          deviceConfig.appEui,            sizeof(deviceConfig.appEui));
    loadOrCreateConfig(storage, Storage::appKeyKey,              "App Key",                          deviceConfig.appKey,            sizeof(deviceConfig.appKey));
    loadOrCreateConfig(storage, Storage::bleNameKey,             "BLE Device Name",                  deviceConfig.bleDeviceName,     31, false);
    loadOrCreateConfig(storage, Storage::serviceUUIDKey,         "Service UUID",                     deviceConfig.serviceUUID,       36);
    loadOrCreateConfig(storage, Storage::characteristicUUIDKey,  "Characteristic UUID",              deviceConfig.characteristicUUID,36);
    loadOrCreateConfig(storage, Storage::timeSyncUUIDKey,        "Time Sync Characteristic UUID",    deviceConfig.timeSyncUUID,      36);
    loadOrCreateConfig(storage, Storage::alarmTargetUUIDKey,     "Alarm Target Characteristic UUID", deviceConfig.alarmTargetUUID,   36);
    loadOrCreateConfig(storage, Storage::wifiApSSIDKey,          "WiFi AP SSID",                     deviceConfig.wifiApSSID,        32, false);
    loadOrCreateConfig(storage, Storage::wifiApPasswordKey,      "WiFi AP Password",                 deviceConfig.wifiApPassword,    64, false);

    deviceConfig.alarmArmed         = storage.getBool(Storage::alarmArmedKey,  false);
    deviceConfig.alarmTargetEpoch   = storage.getUInt(Storage::alarmTargetKey, 0);
    deviceConfig.utcOffset          = storage.getChar(Storage::utcOffsetKey,   0);
    deviceConfig.enabledSensorsMask = static_cast<uint16_t>(storage.getUInt(Storage::enabledSensorsMaskKey, 0xFFFF));

    storage.end();
}

// Initialize system clock with UTC offset from device configuration
void initClock()
{
    systemClock.configure(deviceConfig.utcOffset);
}

// Initialize alarm manager with armed state and target epoch from device configuration
void initAlarmManager()
{
    alarmManager.begin(deviceConfig.alarmArmed, deviceConfig.alarmTargetEpoch);
    setAlarmManagerCallbacks();
}

// Initialize communication interfaces: WiFi, Bluetooth, and LoRaWAN
void initCommunication()
{
    LoRaWANCommunication::RadioPins loraWANPins = {
        .nss  = SPI_NSS_PIN,
        .rst  = SX1276_RST_PIN,
        .dio0 = SX1276_DIO0_PIN,
        .dio1 = SX1276_DIO1_PIN
    };

    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
    SPI .begin(SPI_SCK_PIN, SPI_MISO_PIN, SPI_MOSI_PIN, SPI_NSS_PIN);

    wifi     .configure(deviceConfig.wifiApSSID, 
                        deviceConfig.wifiApPassword);

    bluetooth.configure(deviceConfig.bleDeviceName, 
                        deviceConfig.serviceUUID, 
                        deviceConfig.characteristicUUID, 
                        deviceConfig.timeSyncUUID, 
                        deviceConfig.alarmTargetUUID);
    //lorawan.configure(...) //TODO meme principe que les autres (confgure)

    lorawan = LoRaWANCommunication(loraWANPins, deviceConfig.devEui, deviceConfig.appEui, deviceConfig.appKey);
    lorawan.setPayloadBuilder([](uint8_t* buf, uint8_t maxLen) -> uint8_t
    {
        return static_cast<uint8_t>(serialize(measurement, buf, maxLen));
    });
    lorawan.setAutoUplinkInterval(60); // Send data every 60 seconds

    for (int i = 0; i < commCount; i++)
    {
        communications[i]->begin();
    }

    setWifiCallbacks();
    setBluetoothCallbacks();
    //setLoRaWANCallbacks(); //TODO meme principe que les autres (setCallbacks)
}

// Initialize all sensors based on the enabled sensors mask in the device configuration
void initSensors()
{
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
}

// =================== Handler Functions ====================

// Handle the alarm manager update
void handleAlarmManager()
{
    alarmManager.update();
}

// Handle the screen update with the latest measurement data
void handleScreen()
{
    screen.update(measurement);
}

// Handle the buzzer update
void handleBuzzer()
{
    buzzer.update();
}

// Handle the sound sensor update if it is enabled in the device configuration
void handleSoundSensor()
{
    if (!((deviceConfig.enabledSensorsMask >> 3) & 0x01)) return; 
    soundSensor.update();
}

// Handle the WiFi communication loop
void handleWifi()
{
    if (wifi.isInitialized())
    {
        wifi.loop();
    }
}

// Handle the LoRaWAN communication loop
void handleLoRaWAN()
{
    if (lorawan.isInitialized())
    {
        lorawan.loop();
    }
}

// Handle the status LED update based on the current system status and indicators
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

// Handle the button press and hold events, including reboot on long press
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
// Handle the measurement acquisition from all enabled sensors
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

// Handle the Bluetooth communication loop, including sending data to connected clients
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

// =================== Callback Setup Functions ====================

// Set up the callback for the alarm manager to save changes to storage and update the device configuration
void setAlarmManagerCallbacks()
{
    alarmManager.onAlarmChanged([](bool armed, uint32_t targetEpoch)
    {
        storage.begin(Storage::STORAGE_NAMESPACE, false);
        storage.putBool(Storage::alarmArmedKey, armed);
        storage.putUInt(Storage::alarmTargetKey, targetEpoch);
        storage.end();

        deviceConfig.alarmArmed       = armed;
        deviceConfig.alarmTargetEpoch = targetEpoch;
    });
}

// Set up the callbacks for WiFi communication to handle configuration saving and sensor information requests
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

// Set up the callbacks for Bluetooth communication to handle time synchronization and alarm target updates
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

// =================== Debug Functions ====================
void printDebugInfo()
{
    Serial.println("========  HomeStation Firmware  ========");
    Serial.print("Number of sensors: ");
    Serial.println(sensorCount);

    Serial.println(bluetooth.isInitialized()
        ? "Bluetooth started. Device Name: " + String(deviceConfig.bleDeviceName)
        : "Bluetooth not started.");

    Serial.println(wifi.isInitialized()
        ? "WiFi AP started. IP: " + WiFi.softAPIP().toString()
        : "WiFi AP not started.");

    Serial.println(lorawan.isInitialized() ? "LoRaWAN used." : "LoRaWAN not used.");

    lorawan.onJoined([]()  { Serial.println("LoRaWAN : Join reussi !"); });
    lorawan.onDownlink([](uint8_t port, const uint8_t* data, uint8_t length)
    {
        Serial.printf("Downlink recu sur port %d\n", port);
    });

    Serial.println("--- I2C Scan ---");
    for (uint8_t addr = 1; addr < 127; addr++)
    {
        Wire.beginTransmission(addr);
        if (Wire.endTransmission() == 0)
        {
            Serial.print("(I2C) Found peripheral at 0x");
            Serial.println(addr, HEX);
        }
    }

    Serial.println("--- Memory ---");
    Serial.print("devEui: ");
    for (int i = 0; i < 8; i++) { Serial.print(deviceConfig.devEui[i], HEX); if (i < 7) Serial.print(":"); }
    Serial.println();
    Serial.print("appEui: ");
    for (int i = 0; i < 8; i++) { Serial.print(deviceConfig.appEui[i], HEX); if (i < 7) Serial.print(":"); }
    Serial.println();
    Serial.print("appKey: ");
    for (int i = 0; i < 16; i++) { Serial.print(deviceConfig.appKey[i], HEX); if (i < 15) Serial.print(":"); }
    Serial.println();
    Serial.print("bleDeviceName: ");      Serial.println(deviceConfig.bleDeviceName);
    Serial.print("serviceUUID: ");        Serial.println(deviceConfig.serviceUUID);
    Serial.print("characteristicUUID: "); Serial.println(deviceConfig.characteristicUUID);
    Serial.print("timeSyncUUID: ");       Serial.println(deviceConfig.timeSyncUUID);
    Serial.print("alarmTargetUUID: ");    Serial.println(deviceConfig.alarmTargetUUID);
    Serial.print("alarmArmed: ");         Serial.println(deviceConfig.alarmArmed ? "true" : "false");
    Serial.print("alarmTargetEpoch: ");   Serial.println(deviceConfig.alarmTargetEpoch);
    Serial.print("utcOffset: ");          Serial.println(deviceConfig.utcOffset);
    Serial.print("enabledSensorsMask: "); Serial.println(deviceConfig.enabledSensorsMask, BIN);
    Serial.println("========================================");
}