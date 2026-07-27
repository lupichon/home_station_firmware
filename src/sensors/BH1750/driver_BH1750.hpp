#pragma once

#include <BH1750.h>
#include "../sensor.hpp"

class BH1750Sensor : public Sensor
{
    private:
        BH1750 sensor;

    public:
        BH1750Sensor();

        bool begin() override;
        bool read(Measurement& m) override;
        const char* displayValue(const Measurement& m) const override;
};

inline BH1750Sensor::BH1750Sensor()
    : Sensor("BH1750", "lux")
{
}

inline bool BH1750Sensor::begin()
{
    initialized = sensor.begin(BH1750::CONTINUOUS_HIGH_RES_MODE, 0x23, &Wire);
    return initialized;
}

inline bool BH1750Sensor::read(Measurement& m)
{
    float value = sensor.readLightLevel();

    if (value < 0)
    {
        return false;
    }
    
    m.luminosity = value;
    return true;
}

inline const char* BH1750Sensor::displayValue(const Measurement& m) const
{
    static char buffer[64];

    snprintf(
        buffer,
        sizeof(buffer),
        "%s: %.2f %s",
        getName(),
        m.luminosity,
        getUnit()
    );

    return buffer;
}