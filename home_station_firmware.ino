#include <Arduino.h>

#include "src/core/measurement.hpp"
#include "src/core/sensor.hpp"
#include "src/core/pins.hpp"
#include "src/sensors/BH1750/driver_BH1750.hpp"
#include "src/sensors/HC-SR501/driver_HC-SR501.hpp"

#define DEBUG_ENABLE 1

// Declaration of the Serial object for debugging
extern HardwareSerial Serial;

// Declaration of all the sensors used in the project
BH1750Sensor  lightSensor;                   // Luminosity sensor
HCSR501Sensor motionSensor(HC_SR501_PIN);    // Motion sensor

// Array of pointers to the sensors used in the project
Sensor* sensors[] =
{
    &lightSensor,
    &motionSensor   
};

int sensorCount = Sensor::getSensorCount();

void setup()
{
    if (DEBUG_ENABLE)
    {
        Serial.begin(115200);
        while (!Serial) 
        { 
            delay(10); // Wait for Serial to be ready
        }
    }

    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);

    for(int i = 0; i < sensorCount; i++)
    {
        if(!sensors[i]->begin())
        {
            if(DEBUG_ENABLE)
            {
                Serial.print("Error: Initialization failed for sensor: ");
                Serial.println(sensors[i]->getName());
            }
            // TODO: Faire clignoter une LED en cas de probleme d'initialisation d'un capteur
            // Ou  mieux pour que le reste fonctionne ça sera de juste allumer une LED 
            // Mais de ne pas tomber dans cette boucle infinie et de continuer à lire les autres capteurs quand meme
        }
        else
        {
            if (DEBUG_ENABLE)
            {
                Serial.print("OK : ");
                Serial.println(sensors[i]->getName());
            }
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

            if (DEBUG_ENABLE)
            {
                Serial.println(sensor.displayValue(measurement));
            }
        }
    }
    
    delay(1000);
}