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
        void read(Measurement& m) override;

    protected:
        float measuredValue(const Measurement& m) const override;
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


inline void SCD41Sensor::read(Measurement& m)
{
    uint16_t co2;
    float temperature;
    float humidity;

    int16_t error = sensor.readMeasurement(
        co2,
        temperature,
        humidity
    );

    if (error == 0)
    {
        m.co2 = co2;
        m.temperature = temperature;
        m.humidity = humidity;
    }
}


inline float SCD41Sensor::measuredValue(const Measurement& m) const
{
    return m.co2;
}
