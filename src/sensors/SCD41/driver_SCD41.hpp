#pragma once

#include <SensirionI2cScd4x.h>
#include <Wire.h>
#include "../sensor.hpp"

class SCD41Sensor : public Sensor
{
    private:
        SensirionI2cScd4x sensor;
        float lastTemperature = NAN;
        float lastHumidity = NAN;
        float lastCO2 = NAN;

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
    // TODO: Si le capteur est débranché on rentre quand meme dans le !dataReady, il faudait aller dans if de l'erreur
    uint16_t co2;
    float temperature;
    float humidity;

    bool dataReady = false;
    sensor.getDataReadyStatus(dataReady);
    if (!dataReady)
    {
        m.co2 = lastCO2;
        m.temperature = lastTemperature;
        m.humidity = lastHumidity;
        return true;
    }

    int16_t error = sensor.readMeasurement(
        co2,
        temperature,
        humidity
    );
    
    if (error != 0)
    {
        return false;
    }

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