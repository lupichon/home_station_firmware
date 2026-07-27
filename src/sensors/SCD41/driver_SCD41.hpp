#pragma once

#include <SensirionI2cScd4x.h>
#include <Wire.h>
#include "../sensor.hpp"

// TO DO: A TESTER QUAND LA CAPTEUR SERA SOUDé

class SCD41Sensor : public Sensor
{
    private:
        SensirionI2cScd4x sensor;

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

    int16_t error = sensor.startPeriodicMeasurement();

    initialized = (error == 0);

    return initialized;
}


inline bool SCD41Sensor::read(Measurement& m)
{
    uint16_t co2;
    float temperature;
    float humidity;

    int16_t error = sensor.readMeasurement(
        co2,
        temperature,
        humidity
    );

    if (error != 0)
    {
        return false;
    }

    m.co2 = co2;
    m.temperature = temperature;
    m.humidity = humidity;

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