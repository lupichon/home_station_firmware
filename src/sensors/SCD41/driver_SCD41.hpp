#pragma once

#include <SensirionI2cScd4x.h>
#include <Wire.h>
#include "../sensor.hpp"

//TODO: Calibrer le capteur ?? Les ppm semblent haute (2500 dans ma chambre)

class SCD41Sensor : public Sensor
{
    private:
        SensirionI2cScd4x sensor;
        float lastTemperature = NAN;
        float lastHumidity = NAN;
        uint16_t lastCO2 = NAN;
        int noDataCount = 0;

    public:
        SCD41Sensor();

        bool begin() override;
        bool read(Measurement& m) override;
        const char* displayValue(const Measurement& m) const;
};


inline SCD41Sensor::SCD41Sensor()
    : Sensor("SCD41")
{
}


inline bool SCD41Sensor::begin()
{
    sensor.begin(Wire, 0x62);

    sensor.stopPeriodicMeasurement(); // Stop si déjà en cours
    delay(500);                        // Attendre que le capteur soit prêt

    int16_t error = sensor.startPeriodicMeasurement();

    initialized = (error == 0);

    return initialized;
}


inline bool SCD41Sensor::read(Measurement& m)
{
    uint16_t co2;
    float temperature;
    float humidity;

    int16_t error = sensor.readMeasurement(co2, temperature, humidity);

    if (error == 527)   
    {
        // Pas de nouvelle mesure, on renvoie les dernières valeurs
        noDataCount++;
        if (noDataCount > 20)
        {
            return false;
        }

        m.co2 = lastCO2;
        m.temperature = lastTemperature;
        m.humidity = lastHumidity;
        return true;
    }

    if (error != 0) 
    {
        return false;
    }

    noDataCount = 0;
    m.co2 = lastCO2 = co2;
    m.temperature = lastTemperature = temperature;
    m.humidity = lastHumidity = humidity;

    return true;
}

inline const char* SCD41Sensor::displayValue(const Measurement& m) const
{
    static char buffer[128];

    snprintf(
        buffer,
        sizeof(buffer),
        "%s: CO2=%.0f ppm | Temp=%.2f C | Hum=%.2f %%",
        getName(),
        m.co2,
        m.temperature,
        m.humidity
    );

    return buffer;
}