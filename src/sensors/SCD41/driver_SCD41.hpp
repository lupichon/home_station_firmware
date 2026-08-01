#pragma once

#include <SensirionI2cScd4x.h>
#include <Wire.h>
#include "../sensor.hpp"

//TODO: Calibrer le capteur ?? Les ppm semblent haute (2500 dans ma chambre)

class SCD41Sensor : public Sensor
{
    private:
        SensirionI2cScd4x sensor;
        float lastTemperature;
        float lastHumidity;
        uint16_t lastCO2;
        unsigned long lastSuccessfulReadTime;

    public:
        SCD41Sensor();

        bool begin() override;
        bool read(Measurement& m) override;
        const char* displayValue(const Measurement& m) const;
};


inline SCD41Sensor::SCD41Sensor()
    : Sensor("SCD41"), lastTemperature(NAN), lastHumidity(NAN), lastCO2(NAN), lastSuccessfulReadTime(millis())
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
    bool dataReady = false;
    uint16_t error = sensor.getDataReadyStatus(dataReady);

    if (error != 0)
    {
        return false;
    }

    if (!dataReady)
    {
        if (millis() - lastSuccessfulReadTime > 30000)
        {
            return false; // Trop longtemps sans données
            // A voir plus tard si il faut reset le capteur
        }

        m.co2         = lastCO2;
        m.temperature = lastTemperature;
        m.humidity    = lastHumidity;
        return true;
    }

    // dataReady == true
    uint16_t co2      = 0;
    float temperature = 0.0f;
    float humidity    = 0.0f;

    error = sensor.readMeasurement(co2, temperature, humidity);

    if (error != 0)
    {
        return false;
    }

    m.co2         = lastCO2         = co2;
    m.temperature = lastTemperature = temperature;
    m.humidity    = lastHumidity    = humidity;
    lastSuccessfulReadTime          = millis();
    return true;
}

inline const char* SCD41Sensor::displayValue(const Measurement& m) const
{
    static char buffer[128];

    snprintf(
        buffer,
        sizeof(buffer),
        "%s: CO2=%u ppm | Temp=%.2f C | Hum=%.2f %%",
        getName(),
        m.co2,
        m.temperature,
        m.humidity
    );

    return buffer;
}