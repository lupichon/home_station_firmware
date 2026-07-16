#include <Arduino.h>

#include "src/core/measurement.hpp"
#include "src/core/sensor.hpp"

#include "src/sensors/BH1750/driver_BH1750.hpp"

// Declaration of the Serial object for debugging
extern HardwareSerial Serial;

// Declaration of all the sensors used in the project
BH1750Sensor lightSensor;     // Luminosity sensor


// Array of pointers to the sensors used in the project
Sensor* sensors[] =
{
    &lightSensor
};


const int sensorCount = sizeof(sensors) / sizeof(sensors[0]);

void setup()
{
    Serial.begin(115200);

    Wire.begin();


    for(int i = 0; i < sensorCount; i++)
    {
        if(!sensors[i]->begin())
        {
            Serial.print("Erreur initialisation ");
            Serial.println(sensors[i]->getName());

            while(1)
            {
                // TODO: Faire clignoter une LED en cas de probleme d'initialisation d'un capteur
                // Ou  mieux pour que le reste fonctionne ça sera de juste allumer une LED 
                // Mais de ne pas tomber dans cette boucle infinie et de continuer à lire les autres capteurs quand meme
                delay(1000);
            }
        }
        else
        {
            Serial.print("OK : ");
            Serial.println(sensors[i]->getName());
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
      sensors[i]->read(measurement);
      Serial.println(sensors[i]->displayValue(measurement));
    }

    delay(1000);
}