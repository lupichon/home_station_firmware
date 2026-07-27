#include <Arduino.h>

#include "src/core/measurement.hpp"
#include "src/core/pins.hpp"
#include "src/sensors/BH1750/driver_BH1750.hpp"
#include "src/sensors/HC-SR501/driver_HC-SR501.hpp"
#include "src/sensors/SCD41/driver_SCD41.hpp"
#include "src/sensors/MAX9814/driver_MAX9814.hpp"
#include "src/sensors/FC-51/driver_FC-51.hpp"
#include "src/communication/data_serializer.hpp"
#include "src/communication/bluetooth/bluetooth.hpp"
//#include "src/communication/wifi/wifi.hpp"
#include "src/interface/status_led.hpp"

#define DEBUG_ENABLE 1

// Declaration of communcation interfaces
extern HardwareSerial Serial;
BluetoothCommunication bluetooth("HomeStation");
//WiFiCommunication wifi("your_ssid", "your_password");

// Declaration of elements for the interface with the user (LEDs, buttons, screens, etc.)
StatusLED statusLED(STATUS_LED_RED_PIN, STATUS_LED_GREEN_PIN, STATUS_LED_BLUE_PIN);

// Declaration of all the sensors used in the project
BH1750Sensor  lightSensor;                   // Luminosity sensor
HCSR501Sensor motionSensor(HC_SR501_PIN);    // Motion sensor
//SCD41Sensor   co2TempHumiSensor;             // CO2, temperature and humidity sensor
MAX9814Sensor soundSensor(MAX9814_PIN);      // Sound sensor
FC51Sensor    obstacleSensor(FC_51_PIN);     // Obstacle sensor

// Array of pointers to the sensors used in the project
Sensor* sensors[] =
{
    &lightSensor,
    &motionSensor,
    //&co2TempHumiSensor,
    &soundSensor,
    &obstacleSensor
};

int sensorCount = Sensor::getSensorCount();

// Constants for task timing
constexpr unsigned long TASK_100_MS  = 100; 
constexpr unsigned long TASK_1000_MS = 1000; 

// Timestamps for the last execution of tasks
unsigned long last1000Ms = 0; 
unsigned long last100Ms  = 0;

void setup()
{
    statusLED.begin();
    statusLED.setState(StatusLED::State::STARTING);
    statusLED.update(); 

    Serial.begin(115200);
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
    bluetooth.begin();
    //wifi.begin();

    #if DEBUG_ENABLE
    {
        while (!Serial)
        {
            delay(10);
        }

        Serial.println("========  HomeStation Firmware  ========");
        Serial.print("Number of sensors: ");
        Serial.println(sensorCount);

        if (bluetooth.isInitialized())
        {
            Serial.println("Bluetooth used.");
        }
        else
        {
            Serial.println("Bluetooth not used");
        }

        // if (wifi.isConnected())
        // {
        //     Serial.println("WiFi used.");
        // }
        // else
        // {
        //     Serial.println("WiFi not used");
        // }

        Serial.println("========================================");
    }
    #endif

    int failedSensors = 0;
    for(int i = 0; i < sensorCount; i++)
    {
        if(!sensors[i]->begin())
        {
            failedSensors++;
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

    if (failedSensors == 0)
    {
        statusLED.setState(StatusLED::State::OK);
    }
    else if (failedSensors < sensorCount)
    {
        statusLED.setState(StatusLED::State::WARNING_COMMUNICATION);
    }
    else
    {
        statusLED.setState(StatusLED::State::ERROR);
    }
}


void loop()
{
    unsigned long now = millis();

    // Task executed every 100 ms
    if (now - last100Ms >= TASK_100_MS)
    {
        last100Ms = now;
        
        statusLED.update();
    }

    // Task executed every 1000 ms
    if (now - last1000Ms >= TASK_1000_MS)
    {
        last1000Ms = now;
        Measurement measurement;
        
        bool successR = readSensors(measurement);  
        bool successB = sendBluetooth(measurement);
        updateStatus(successR, successB);
    }
}

bool readSensors(Measurement& measurement)
{
    bool success = true;
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
    }
    return success;
}

bool sendBluetooth(const Measurement& measurement)
{
    bool success = true;
    uint8_t buffer[BUFFER_SIZE];
    size_t dataSize = serialize(measurement, buffer, sizeof(buffer));

    if (dataSize > 0 && bluetooth.isInitialized() && bluetooth.hasConnectedClient())
    {
        success = bluetooth.send(buffer, dataSize);

        #if DEBUG_ENABLE
        if (success)
        {
            Serial.println("Data sent over Bluetooth.");
        }
        #endif
    }

    return success;
}

void updateStatus(bool sensorsOK, bool bluetoothOK)
{
    if (!sensorsOK && !bluetoothOK)
    {
        statusLED.setState(StatusLED::State::ERROR);
    }
    else if (!sensorsOK)
    {
        statusLED.setState(StatusLED::State::WARNING_SENSOR);
    }
    else if (!bluetoothOK)
    {
        statusLED.setState(StatusLED::State::WARNING_COMMUNICATION);
    }
    else
    {
        statusLED.setState(StatusLED::State::OK);
    }
}