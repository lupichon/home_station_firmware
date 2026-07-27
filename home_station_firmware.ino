#include <Arduino.h>

#include "src/core/measurement.hpp"
#include "src/core/pins.hpp"
#include "src/sensors/BH1750/driver_BH1750.hpp"
#include "src/sensors/HC-SR501/driver_HC-SR501.hpp"
#include "src/sensors/SCD41/driver_SCD41.hpp"
#include "src/sensors/MAX9814/driver_MAX9814.hpp"
#include "src/communication/data_serializer.hpp"
#include "src/communication/bluetooth/bluetooth.hpp"
//#include "src/communication/wifi/wifi.hpp"

#define DEBUG_ENABLE 1

// Declaration of communcation interfaces
extern HardwareSerial Serial;
BluetoothCommunication bluetooth("HomeStation");
//WiFiCommunication wifi("your_ssid", "your_password");

// Declaration of all the sensors used in the project
BH1750Sensor  lightSensor;                   // Luminosity sensor
HCSR501Sensor motionSensor(HC_SR501_PIN);    // Motion sensor
SCD41Sensor   co2TempHumiSensor;             // CO2, temperature and humidity sensor
MAX9814Sensor soundSensor(MAX9814_PIN);      // Sound sensor

// Array of pointers to the sensors used in the project
Sensor* sensors[] =
{
    &lightSensor,
    &motionSensor,
    &co2TempHumiSensor,
    &soundSensor
};

int sensorCount = Sensor::getSensorCount();

void setup()
{
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

    for(int i = 0; i < sensorCount; i++)
    {
        if(!sensors[i]->begin())
        {
            #if DEBUG_ENABLE
                Serial.print("Error: Initialization failed for sensor: ");
                Serial.println(sensors[i]->getName());
            #endif
            // TODO: Faire clignoter une LED en cas de probleme d'initialisation d'un capteur
            // Ou  mieux pour que le reste fonctionne ça sera de juste allumer une LED 
            // Mais de ne pas tomber dans cette boucle infinie et de continuer à lire les autres capteurs quand meme
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


void loop()
{
    // Create a Measurement object to hold the readings from the sensors
    Measurement measurement;

    // Read data from each sensor and store it in the Measurement object
    for(int i = 0; i < sensorCount; i++)
    {
        Sensor& sensor = *sensors[i];

        if(sensor.isInitialized())
        {
            sensor.read(measurement);

            #if DEBUG_ENABLE
                Serial.println(sensor.displayValue(measurement));
            #endif
        }
    }

    // Serialize the Measurement object into a byte buffer
    uint8_t buffer[BUFFER_SIZE];
    size_t dataSize = serialize(measurement, buffer, sizeof(buffer));

    if (dataSize > 0)
    {
        // Send the measurement data over Bluetooth if the Bluetooth interface is initialized and a least one client is connected
        if (bluetooth.isInitialized() && bluetooth.hasConnectedClient())
        {
            bluetooth.send(buffer, dataSize);

            #if DEBUG_ENABLE
                Serial.println("Data sent over Bluetooth.");
            #endif
        }
    }
    
    delay(1000);
}